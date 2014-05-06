
#include "h264dec.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
# include <stdint.h>

#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
//#include "libavfilter/avfilter.h"

#include "sps_pps.h"


void hello()
{
	printf("hello!Load libh264dec.so success!\n");
}

//最大支持16路解码
#define max_channel 16
//const int max_channel = 16;
//H264解码结构
typedef struct
{
	//使用标志
	int8_t bUsed;
	//初始化成功标志
	int8_t bInitOK;
	//解码器
	AVCodec *pCodec;
	AVCodecContext *pCodecCtx;
	struct SwsContext* sws_ctx;
	//YUV帧结构
	AVFrame *pFrameYUV;
	//RGB帧结构
	AVFrame *pFrameRGB;
	//数据包
	AVPacket packet;
	//RGB缓冲区
	uint8_t* prgbBuf;
	//rgb缓冲区长度
	int rgbbuflen;
	//反交错处理标志 0不处理 1处理
	//uint8_t  dtlFlag;
	//是否处理YC伸张
	//uint8_t  fullscale;
	//图像宽度和高度
	int nImageWidth;
	int nImageHeight;
}CH264Decode;

CH264Decode g_CH264Decodes[max_channel];


pthread_mutex_t g_mutex;

//函数说明：动态库资源分配
//返回值：1--成功  -1--失败
int h264dec_init()
{
	pthread_mutex_init(&g_mutex, NULL);
	av_register_all();
	//avfilter_register_all();
	printf("h264dec_init()--success!\n");
	return 1;
}


int h264dec_uninit()
{
	pthread_mutex_destroy(&g_mutex);
	printf("h264dec_uninit()--success!\n");
	return 1;
}


int h264dec_create()
{
	CH264Decode *pCH264Decode = NULL;
	pthread_mutex_lock(&g_mutex);
	int i = 0;
	for (; i < max_channel; ++i)
	{
		pCH264Decode = &g_CH264Decodes[i];
		if (0 == pCH264Decode->bUsed)
		{
			pCH264Decode->bUsed = 1;
			break;
		}
	}

	//没有找到可用解码器
	if (i == max_channel)
	{
		goto exit;
	}

	if (NULL == pCH264Decode->pCodec)
	{
		pCH264Decode->pCodec = avcodec_find_decoder (AV_CODEC_ID_H264);
		if (NULL == pCH264Decode->pCodec)
		{
			goto exit;
		}
	}

	if (NULL == pCH264Decode->pCodecCtx)
	{
		pCH264Decode->pCodecCtx = avcodec_alloc_context3(pCH264Decode->pCodec);
		if (NULL == pCH264Decode->pCodecCtx)
		{
			goto exit;
		}
	}
	// 通知解码器我们能够处理截断的bit流－－ie，
	// bit 流帧边界可以在包中
	if(pCH264Decode->pCodec->capabilities&CODEC_CAP_TRUNCATED)
		pCH264Decode->pCodecCtx->flags|= CODEC_FLAG_TRUNCATED;
	//打开解码器(非线程安全)
	if(avcodec_open2(pCH264Decode->pCodecCtx, pCH264Decode->pCodec, NULL) < 0)
	{
		//关闭解码器
		goto exit;
	}
	//分配YUV缓冲区
	if (NULL == pCH264Decode->pFrameYUV)
	{
		pCH264Decode->pFrameYUV = av_frame_alloc();
		if (NULL == pCH264Decode->pFrameYUV)
		{
			goto exit;
		}
	}
	//分配rgb缓冲区
	if (NULL == pCH264Decode->pFrameRGB)
	{
		pCH264Decode->pFrameRGB = av_frame_alloc();
		if (NULL == pCH264Decode->pFrameRGB)
		{
			goto exit;
		}
	}

	//反交错处理标志
	//pCH264Decode->dtlFlag = 1;
	//是否处理伸张
	//pCH264Decode->fullscale = 0;
	//初始化成功标志
	pCH264Decode->bInitOK = 1;

	printf("h264dec_create()--success!\n");
	return i;

exit:
	av_frame_free(&pCH264Decode->pFrameYUV);
	avcodec_close(pCH264Decode->pCodecCtx);
	av_free(pCH264Decode->pCodecCtx);
	pthread_mutex_unlock(&g_mutex);
	printf("h264dec_create()--failed!\n");
	return -1;
}

//释放解码对象分配的资源
int h264dec_destory(int decObjIdx)
{
	if (decObjIdx < 0 || decObjIdx > max_channel)
	{
		printf("h264dec_destory()--failed!\n");
		return -1;
	}
	CH264Decode *pCH264Decode = NULL;
	pCH264Decode =  &g_CH264Decodes[decObjIdx];
	if (pCH264Decode != NULL)
	{
		if (pCH264Decode->sws_ctx != NULL)
		{
			sws_freeContext(pCH264Decode->sws_ctx);
			pCH264Decode->sws_ctx = NULL;
		}

		if (pCH264Decode->prgbBuf != NULL)
		{
			av_free(pCH264Decode->prgbBuf);
			pCH264Decode->prgbBuf = NULL;

		}


		//释放存放rgb的AVFrame
		if (pCH264Decode->pFrameRGB != NULL)
		{
			av_frame_free(&pCH264Decode->pFrameRGB);
		}
		//释放存放yuv的AVFrame
		if (pCH264Decode->pFrameYUV != NULL)
		{
			av_frame_free(&pCH264Decode->pFrameYUV);
		}
		//关闭并释放解码器
		avcodec_close(pCH264Decode->pCodecCtx);
		av_free(pCH264Decode->pCodecCtx);

		//恢复初始化状态
		memset(pCH264Decode, 0, sizeof(CH264Decode));
	}

	printf("h264dec_destory()--success!\n");
	return 1;
}

//从h264缓冲区中解析出宽高
int h264dec_getInfo(int decObjIdx, uint8_t *ph264buf, int h264buflen, int* width, int* height)
{
	if (decObjIdx < 0 || decObjIdx > max_channel)
	{
		return -1;
	}
	if (NULL == ph264buf || h264buflen <= 0 )
	{
		return -2;
	}

	uint8_t *p = ph264buf;
	CH264Decode *pCH264Decode = &g_CH264Decodes[decObjIdx];

	int i = 0;
	int j = 0;
	int spsStartCodeFlag = 0;
	int spsEndCodeFlag = 0;
	for (; i < 1024*10; ++i)
	{
		//定位SPS起始位置
		if(p[i]==0x00 && p[i+1]==0x00 && p[i+2]==0x00 && p[i+3]==0x01 && p[i+4]==0x67)
		{
			j = i;
			spsStartCodeFlag = 1;
		}
		if (spsStartCodeFlag == 1)
		{
			if(p[i]==0x00 && p[i+1]==0x00 && p[i+2]==0x00 && p[i+3]==0x01 && p[i+4]==0x68)
			{
				spsEndCodeFlag = 1;
				break;
			}
		}
	}

	if (i == 1024*10 || spsEndCodeFlag == 0 || spsStartCodeFlag == 0)
	{
		return -3;
	}


	p = p + j + 5;

	uint8_t spsbuf[512] = {0};
	memcpy(&spsbuf, p, i-j);

	get_bit_context get_bit_context = {0};
	get_bit_context.buf = spsbuf;
	get_bit_context.buf_size = i-j;

	SPS sps = {0};
	h264dec_seq_parameter_set(&get_bit_context, &sps);
	pCH264Decode->nImageHeight = (sps.pic_height_in_map_units_minus1+1)*16;
	pCH264Decode->nImageWidth  = (sps.pic_width_in_mbs_minus1+1)*16;


	return 1;
}

//h264数据流序列0x67 0x68 0x65 0x61 0x61...0x67 x68 0x65 0x61 0x61...
//             SPS  PPS   I   P     P ... SPS PPS  I    P    P  ...
//ph264buf缓冲区中的数据SPS+PPS+I或者P
//参数值：decObjIdx--解码对象索引值
//      ph264buf--h264缓冲区
//		h264buflen--h264缓冲区长度
//      isfinished--该帧数据是否解码完成
//		pyuvbuf--yuv缓冲区
//		yuvbuflen--yuv缓冲区长度
//将多个输入参赛封装进结构中去
int h264dec(int decObjIdx, SH264decParams* pSH264decParams)
{
	printf("h264dec_yuv()--here!\n");
	if (decObjIdx < 0 || decObjIdx > max_channel)
	{
		return -1;
	}
	if (NULL == pSH264decParams->ph264buf || pSH264decParams->h264buflen <= 0 )
	{
		return -2;
	}

	uint8_t *p = pSH264decParams->ph264buf;
	CH264Decode *pCH264Decode = &g_CH264Decodes[decObjIdx];
	//如果解码对象未初始化，返回失败
	if (pCH264Decode->bInitOK == 0 || pCH264Decode->bUsed == 0)
	{
		return -3;
	}

	//从h264缓冲区中解析出宽高
	if (pCH264Decode->nImageHeight == 0 || pCH264Decode->nImageWidth == 0)
	{
		if (1 != h264dec_getInfo(decObjIdx, p, pSH264decParams->h264buflen, &pCH264Decode->nImageWidth, &pCH264Decode->nImageHeight))
			return -4;
	}

	pSH264decParams->decwidth = pCH264Decode->nImageWidth;
	pSH264decParams->decheight = pCH264Decode->nImageHeight;

	if (pSH264decParams->pyuvbuf == NULL && pSH264decParams->prgbbuf == NULL)
	{
		return 1;
	}

	pCH264Decode->packet.data = p;
	pCH264Decode->packet.size = pSH264decParams->h264buflen;
	//解码
	int ret = avcodec_decode_video2(pCH264Decode->pCodecCtx, pCH264Decode->pFrameYUV, &pSH264decParams->isfinished, &pCH264Decode->packet);
	if (ret < 0)
	{
		//解码失败
		return -5;
	}
	//未解码完一帧数据
	if(pSH264decParams->isfinished == 0)
	{
		return -6;
	}
	//将解码出的yuv数据拷贝到外部缓冲区
	if (pSH264decParams->pyuvbuf != NULL && pSH264decParams->yuvbuflen > 0)
	{
		int i = 0;
		int j = 0;

		int yuvwidth = 0;
		int yuvheight = 0;
		int k = 0;
		for (; i < 3; ++i)
		{
			if (i == 0)//Y
			{
				yuvwidth = pCH264Decode->pFrameYUV->width;
				yuvheight = pCH264Decode->pFrameYUV->height;
			}
			else//U V
			{
				yuvwidth = pCH264Decode->pFrameYUV->width/2;
				yuvheight = pCH264Decode->pFrameYUV->height/2;
			}
			//从data[i]中以固定长度linesize为间隔拷贝有效长度为yuvwidth的数据到pyuvbuf中
			int blocksize = yuvheight * pCH264Decode->pFrameYUV->linesize[i];
			for (k = 0;k < blocksize;)
			{
				memcpy(&pSH264decParams->pyuvbuf[j], &pCH264Decode->pFrameYUV->data[i][k], yuvwidth);
				k+=pCH264Decode->pFrameYUV->linesize[i];
				j+=yuvwidth;
			}
		}
		pSH264decParams->yuvbuflen = j;
	}

	if (pSH264decParams->prgbbuf == NULL || pSH264decParams->rgbbuflen <= 0)
	{
		return 2;
	}

    int src_w, src_h, dst_w, dst_h;
    src_w = dst_w = pCH264Decode->nImageWidth;
    src_h = dst_h = pCH264Decode->nImageHeight;

   if (NULL == pCH264Decode->prgbBuf || 0 == pCH264Decode->rgbbuflen)
   {
	   pCH264Decode->rgbbuflen = avpicture_get_size(AV_PIX_FMT_BGR24, pCH264Decode->pCodecCtx->width,
	          		pCH264Decode->pCodecCtx->height);
	   pCH264Decode->prgbBuf = av_malloc(pCH264Decode->rgbbuflen);

	   ret = avpicture_fill((AVPicture *)(pCH264Decode->pFrameRGB), pCH264Decode->prgbBuf, AV_PIX_FMT_BGR24,
			pCH264Decode->pCodecCtx->width, pCH264Decode->pCodecCtx->height);
   }

   /* create scaling context */
    if (NULL == pCH264Decode->sws_ctx)
    {
    	pCH264Decode->sws_ctx = sws_getCachedContext(pCH264Decode->sws_ctx, src_w, src_h, pCH264Decode->pCodecCtx->pix_fmt,
    		                             dst_w, dst_h, AV_PIX_FMT_BGR24,
    		                             SWS_BICUBIC, NULL, NULL, NULL);
    	if (NULL == pCH264Decode->sws_ctx)
    	{
    		printf("sws_getCachedContext failed!\n");
    		return -7;
    	}
    }

    //避免rgb倒置,变换yuv数据的起始地址
    pCH264Decode->pFrameYUV->data[0] += pCH264Decode->pFrameYUV->linesize[0] * (pCH264Decode->pCodecCtx->height-1);
    pCH264Decode->pFrameYUV->linesize[0] *= -1;
    pCH264Decode->pFrameYUV->data[1] += pCH264Decode->pFrameYUV->linesize[1] * (pCH264Decode->pCodecCtx->height/2 - 1);;
    pCH264Decode->pFrameYUV->linesize[1] *= -1;
    pCH264Decode->pFrameYUV->data[2] += pCH264Decode->pFrameYUV->linesize[2] * (pCH264Decode->pCodecCtx->height/2 - 1);;
    pCH264Decode->pFrameYUV->linesize[2] *= -1;


    /* convert to destination format */
    sws_scale(pCH264Decode->sws_ctx, (const uint8_t * const*)pCH264Decode->pFrameYUV->data,
    		pCH264Decode->pFrameYUV->linesize, 0, src_h, pCH264Decode->pFrameRGB->data, pCH264Decode->pFrameRGB->linesize);

    //拷贝rbg缓冲区
	memcpy(pSH264decParams->prgbbuf, pCH264Decode->prgbBuf, pCH264Decode->rgbbuflen);
	pSH264decParams->rgbbuflen = pCH264Decode->rgbbuflen;

	return 3;
}


