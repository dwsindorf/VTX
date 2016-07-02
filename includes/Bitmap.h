#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <stdio.h>
#include <stdlib.h>
#include "GLheaders.h"

/*
 * Include necessary headers.
 */
#ifdef WIN32

#  include <errno.h>
#  include <windows.h>
#  include <wingdi.h>
#  include <GL/gl.h>

extern int	PrintDIBitmap(HWND owner, BITMAPINFO *info, void *bits);

#else   // UNIX

#undef INTGR
#undef SHORT
#undef BYTE

#define INTGR  unsigned int
#define SHORT  unsigned short
#define BYTE   unsigned char
	
/* constants for the biCompression field */
#define BI_RGB        0
#define BI_RLE8       1
#define BI_RLE4       2
#define BI_BITFIELDS  3

typedef struct tagBITMAP
  {
    INTGR	bmType;
    INTGR	bmWidth;
    INTGR	bmHeight;
    INTGR	bmWidthBytes;
    SHORT	bmPlanes;
    SHORT	bmBitsPixel;
    INTGR	bmBits;
  } BITMAP;

typedef struct tagRGBTRIPLE {
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFOHEADER { // file size=40 bytes
    INTGR	biSize;
    INTGR	biWidth;
    INTGR	biHeight;
    SHORT	biPlanes;
    SHORT	biBitCount;
    INTGR	biCompression;
    INTGR	biSizeImage;
    INTGR	biXPelsPerMeter;
    INTGR	biYPelsPerMeter;
    INTGR	biClrUsed;
    INTGR	biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    //RGBQUAD             bmiColors[1];
} BITMAPINFO;

typedef struct tagBITMAPFILEHEADER {  // file size=14 bytes
    SHORT	bfType;
    INTGR	bfSize;
    SHORT   bfReserved1; 
    SHORT   bfReserved2; 
    INTGR	bfOffBits;
 } BITMAPFILEHEADER;

#endif  // UNIX LINUX

/* common prototypes */

extern GLubyte	*ConvertRGB(BITMAPINFO *info, void *bits);
extern void	*ReadDIBitmap(BITMAPINFO **info);
extern void *LoadDIBitmap(char *path,BITMAPINFO **info);
extern int	SaveDIBitmap(char *path, BITMAPINFO *info, void *bits);
extern void *BitmapToPixels(BITMAPINFO *info, void *bits, int mode);
extern void *PixelsToBitmap(BITMAPINFO *info, void *bits, int mode);
extern BITMAPINFO *makeBitmapInfo(int w, int h);

 
#endif
