#include <stdio.h>
#include <string.h>
#include "h264dec.h"

#include "BitMapFile.h"

int main()
{
	printf("main\n");
	//hello_shared();
	hello();
	h264dec_init();
	h264dec_uninit();
	int objIdx = h264dec_create();


#define H264BUFSIZE 100*1024*2 //200K
#define YUVBUFSIZE 100*1024*20 //2M
#define RGBBUFSIZE 100*1024*20*2 //4M
	unsigned char* ph264Buf = new unsigned char[H264BUFSIZE];
	memset((void*)ph264Buf, 0, H264BUFSIZE);

	unsigned char* pyuvBuf = new unsigned char[YUVBUFSIZE];
		memset((void*)pyuvBuf, 0, YUVBUFSIZE);
	SH264decParams sh264decParams = {0};
	sh264decParams.ph264buf = ph264Buf;
	sh264decParams.h264buflen = H264BUFSIZE;
	sh264decParams.isfinished = -1;
	sh264decParams.pyuvbuf = pyuvBuf;
	sh264decParams.yuvbuflen = YUVBUFSIZE;
	sh264decParams.prgbbuf = new unsigned char[RGBBUFSIZE];
	sh264decParams.rgbbuflen = RGBBUFSIZE;


	FILE *fp = NULL;
	fp = fopen("./testi.h264", "rb");
	int readsize = 0;
	readsize = fread((void*)ph264Buf, 1, H264BUFSIZE, fp);
	fclose(fp);
	fp = NULL;
	int retVal = 0;
	if (readsize > 0)
	{
		int isfinished = -1;
		//int h264dec_yuv(int decObjIdx, unsigned char *ph264buf, int h264buflen,int* isfinished, unsigned char *pyuvbuf, int yuvbuflen);
		retVal = h264dec(objIdx, &sh264decParams);
		if (retVal > 0 && 0 != isfinished)
		{
			FILE *fp = NULL;
			fp = fopen("./testi.yuv", "wb");
			fwrite(sh264decParams.pyuvbuf, 1, sh264decParams.yuvbuflen, fp);
			fclose(fp);
		}
	}

	h264dec_destory(objIdx);

	construct_bitmap_file(sh264decParams.prgbbuf, sh264decParams.rgbbuflen, sh264decParams.decwidth, sh264decParams.decheight);

	delete[] ph264Buf;
	ph264Buf = NULL;
	delete[] pyuvBuf;
	pyuvBuf = NULL;
	delete[] sh264decParams.prgbbuf;
	sh264decParams.prgbbuf = NULL;

	return 0;
}
