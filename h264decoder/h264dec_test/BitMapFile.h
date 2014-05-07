#include <stdio.h>

typedef 	long 	  DWORD;
typedef  	long      LONG;
typedef  	short     WORD;

#pragma pack (push,1) /*指定按2字节对齐*/

typedef struct _WinBMPFileHeader
{
	WORD   FileType;     /* File type, always 4D42h ("BM") */
	DWORD  FileSize;     /* Size of the file in bytes */
	WORD   Reserved1;    /* Always 0 */
	WORD   Reserved2;    /* Always 0 */
	DWORD  BitmapOffset; /* Starting position of image data in bytes */
} WINBMPFILEHEADER;

typedef struct _Win4xBitmapHeader
{
	DWORD Size;            /* Size of this header in bytes */
	LONG  Width;           /* Image width in pixels */
	LONG  Height;          /* Image height in pixels */
	WORD  Planes;          /* Number of color planes */
	WORD  BitsPerPixel;    /* Number of bits per pixel */
	DWORD Compression;     /* Compression methods used */
	DWORD SizeOfBitmap;    /* Size of bitmap in bytes */
	LONG  HorzResolution;  /* Horizontal resolution in pixels per meter */
	LONG  VertResolution;  /* Vertical resolution in pixels per meter */
	DWORD ColorsUsed;      /* Number of colors in the image */
	DWORD ColorsImportant; /* Minimum number of important colors */
	/* Fields added for Windows 4.x follow this line */

	DWORD RedMask;       /* Mask identifying bits of red component */
	DWORD GreenMask;     /* Mask identifying bits of green component */
	DWORD BlueMask;      /* Mask identifying bits of blue component */
	DWORD AlphaMask;     /* Mask identifying bits of alpha component */
	DWORD CSType;        /* Color space type */
	LONG  RedX;          /* X coordinate of red endpoint */
	LONG  RedY;          /* Y coordinate of red endpoint */
	LONG  RedZ;          /* Z coordinate of red endpoint */
	LONG  GreenX;        /* X coordinate of green endpoint */
	LONG  GreenY;        /* Y coordinate of green endpoint */
	LONG  GreenZ;        /* Z coordinate of green endpoint */
	LONG  BlueX;         /* X coordinate of blue endpoint */
	LONG  BlueY;         /* Y coordinate of blue endpoint */
	LONG  BlueZ;         /* Z coordinate of blue endpoint */
	DWORD GammaRed;      /* Gamma red coordinate scale value */
	DWORD GammaGreen;    /* Gamma green coordinate scale value */
	DWORD GammaBlue;     /* Gamma blue coordinate scale value */
} WIN4XBITMAPHEADER;

typedef struct
{
	WINBMPFILEHEADER bmpfileheader;
	WIN4XBITMAPHEADER bmpinfo;
}BMPHEADER;

int construct_bitmap_file(unsigned char *rgb24buf, int rgbbuflen, int width, int height)
{
	BMPHEADER bmpheader={0};
	bmpheader.bmpfileheader.FileType = 0x4D42;
	bmpheader.bmpfileheader.FileSize = sizeof(BMPHEADER)+rgbbuflen;
	bmpheader.bmpfileheader.Reserved1 = bmpheader.bmpfileheader.Reserved2 = 0;
	bmpheader.bmpfileheader.BitmapOffset = sizeof(BMPHEADER);

	bmpheader.bmpinfo.Size = sizeof(WIN4XBITMAPHEADER);
	bmpheader.bmpinfo.Width = width;
	bmpheader.bmpinfo.Height = height;
	bmpheader.bmpinfo.Planes = 1;
	bmpheader.bmpinfo.BitsPerPixel = 24;
	bmpheader.bmpinfo.Compression = 0;
	bmpheader.bmpinfo.SizeOfBitmap = 0;
	bmpheader.bmpinfo.HorzResolution = 0;//?
	bmpheader.bmpinfo.VertResolution = 0;//?
	bmpheader.bmpinfo.ColorsUsed = 0;
	bmpheader.bmpinfo.ColorsImportant = 0;


	FILE *fp = fopen("./1.bmp", "wb");
	fwrite((void*)(&bmpheader), 1, sizeof(BMPHEADER), fp);
	fwrite((void*)rgb24buf, 1, rgbbuflen, fp);
	fclose(fp);


	return 1;
}

int main0()
{
	FILE *fp = NULL;
	int readlen = 0;
	unsigned char tmp[100*1024*1024] = {0};
	fp = fopen("./testi.rgb", "rb");
	if ((readlen = fread(tmp, 1, sizeof(tmp), fp)) > 0)
	{
		fclose(fp);
		fp = NULL;
	}
	construct_bitmap_file(tmp, readlen, 160, 128);
	return 1;
}
