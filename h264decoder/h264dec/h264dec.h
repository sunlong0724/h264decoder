#ifndef _H_H264DEC_H_
#define _H_H264DEC_H_

typedef struct
{
	unsigned char  *ph264buf;   //[in] h264缓冲区
	int h264buflen;      		//[in] h264缓冲区长度
	int isfinished;     		//[out]该帧数据是否解码完成
	unsigned char *pyuvbuf;    	//[out]yuv缓冲区
	int yuvbuflen;       		//[in/out]yuv缓冲区长度
	unsigned char *prgbbuf;     //[out]rgb缓冲区
	int rgbbuflen;        		//[in/out]rgb缓冲区长度
	int decwidth;         		//[out]解码出的数据宽
	int decheight;        		//[out]解码出的数据高

}SH264decParams;

void hello();
int h264dec_init();
int h264dec_uninit();

int h264dec_create();
int h264dec_destory(int decObjIdx);

//int h264dec_yuv(int decObjIdx, unsigned char *ph264buf, int h264buflen,int* isfinished, unsigned char *pyuvbuf, int yuvbuflen);
int h264dec(int decObjIdx, SH264decParams* pSH264decParams);


#endif
