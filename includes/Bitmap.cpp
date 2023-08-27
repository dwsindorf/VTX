// Bitmap.cpp

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "Bitmap.h"

// ".bmp" files created under windows use LITTLE ENDEAN
// format for short and long data storage types
// bytes must be swapped end-to-end to read into a BIG ENDEAN os.

//#define HDRSIZE  sizeof(BITMAPFILEHEADER)

#define HDRSIZE  14 // unix alignment forces size=16

static SHORT BM=('M'<<8)|'B'; // wintel byte swap

inline void SWAP2(BYTE  *src)
{
	BYTE b=src[1];
	src[1]=src[0];
	src[0]=b;
}

inline void SWAP4(BYTE  *src)
{
	BYTE b=src[3];
	src[3]=src[0];
	src[0]=b;
	b=src[2];
	src[2]=src[1];
	src[1]=b;
}
#define READ2 \
		if(fread(src+index, 1, 2, fp)<1) \
			return 1; \
		SWAP2(src+index); \
		index+=2

#define WRITE2 \
	    tmp[1]=src[index]; \
	    tmp[0]=src[index+1]; \
		if(fwrite(tmp, 1, 2, fp)<1) \
			return 1; \
		index+=2

#define READ4 \
		if(fread(src+index, 1, 4, fp)<1) \
			return 1; \
		SWAP4(src+index); \
		index+=4

#define WRITE4 \
	    tmp[3]=src[index]; \
	    tmp[2]=src[index+1]; \
	    tmp[1]=src[index+2]; \
	    tmp[0]=src[index+3]; \
		if(fwrite(tmp, 1, 4, fp)<1) \
			return 1; \
		index+=4

/*
 * Read the file header and any following bitmap information...
 */
#ifndef BIG_ENDEAN
int ReadFileHeader(BITMAPFILEHEADER *header, FILE *fp)
{
	fread(&(header->bfType), 1, 2, fp);
    fread(&(header->bfSize), 1, HDRSIZE-2, fp);
	return 0; /* header OK */
}

#else  // UNIX
int ReadFileHeader(BITMAPFILEHEADER *header,FILE *fp)
{
  BYTE *src=(BYTE*)header;
  int index=0;
  header->bfType=0;

  READ2;	// 2: bfType ('BM')

  src=(BYTE*)(&(header->bfSize));
  index=0;

  READ4;	// 4: bfSize
  READ2;	// 4: bfReserved1
  READ2;	// 4: bfReserved2
  READ4;	// 4: bfOffBits
  return 0;
}
#endif

#ifndef BIG_ENDEAN
int WriteFileHeader(BITMAPFILEHEADER *header, FILE *fp)
{
    fwrite(&(header->bfType), 1, 2, fp);
    fwrite(&(header->bfSize), 1, HDRSIZE-2, fp);
    return 0;
 }
#else  // UNIX
int WriteFileHeader(BITMAPFILEHEADER *header,FILE *fp)
{
  BYTE *src=(BYTE*)header;
  int index=0;
  BYTE tmp[4];
  WRITE2;	// 2: bfType
  src=(BYTE*)(&(header->bfSize));
  index=0;
  WRITE4;	// 4: bfSize
  WRITE2;	// 2: bfReserved1
  WRITE2;	// 2: bfReserved2
  WRITE4;	// 4: bfOffBits

  return 0;
}
#endif

#ifndef BIG_ENDEAN
int ReadInfoHeader(BITMAPINFO *info,FILE *fp, int infosize)
{
  	if (fread(info, 1, infosize, fp) < (unsigned)infosize)
  		return 1;
  	return 0;
}
#else // UNIX
int ReadInfoHeader(BITMAPINFO *info,FILE *fp,int infosize)
{
  BYTE *src=(BYTE*)info;
  int index=0;
  READ4;   	// 4: biSize
  READ4;	// 4: biWidth
  READ4;	// 4: biHeight
  READ2;	// 2: biPlanes
  READ2;	// 2: biBitCount
  READ4;	// 4: biCompression
  READ4;	// 4: biSizeImage
  READ4;	// 4: biXPelsPerMeter
  READ4;	// 4: biYPelsPerMeter
  READ4;	// 4: biClrUsed
  READ4;	// 4: biClrImportant
  return 0;
}
#endif

#ifndef BIG_ENDEAN
int WriteInfoHeader(BITMAPINFO *info, FILE *fp,int infosize)
{
  if (fwrite(info, 1, infosize, fp) < (unsigned)infosize)
    {
       fclose(fp);
       return 1;
    }
	return 0;
 }
#else // UNIX
int WriteInfoHeader(BITMAPINFO *info,FILE *fp,int infosize)
{
  BYTE *src=(BYTE*)info;
  int index=0;
  BYTE tmp[4];
  WRITE4;   // 4: biSize
  WRITE4;	// 4: biWidth
  WRITE4;	// 4: biHeight
  WRITE2;	// 2: biPlanes
  WRITE2;	// 2: biBitCount
  WRITE4;	// 4: biCompression
  WRITE4;	// 4: biSizeImage
  WRITE4;	// 4: biXPelsPerMeter
  WRITE4;	// 4: biYPelsPerMeter
  WRITE4;	// 4: biClrUsed
  WRITE4;	// 4: biClrImportant
  return 0;
}
#endif

/*
 * 'makeBitmapInfo()' - initialize BITMAPINFO structure.
 *
 */
BITMAPINFO	*makeBitmapInfo(int w, int h)
{
	BITMAPINFO	*info;
  	if ((info = (BITMAPINFO *)malloc(sizeof(BITMAPINFO))) == NULL){
    	return (NULL);
  	}
  	info->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  	info->bmiHeader.biWidth         = w;
  	info->bmiHeader.biHeight        = h;
  	info->bmiHeader.biPlanes        = 1;
  	info->bmiHeader.biBitCount      = 24;
  	info->bmiHeader.biCompression   = BI_RGB;
  	info->bmiHeader.biSizeImage     = 0;
  	info->bmiHeader.biXPelsPerMeter = 2952; /* 75 DPI */
  	info->bmiHeader.biYPelsPerMeter = 2952; /* 75 DPI */
  	info->bmiHeader.biClrUsed       = 0;
  	info->bmiHeader.biClrImportant  = 0;
  	return info;
}

/*
 * 'LoadDIBitmap()' - Load a DIB/BMP file from disk.
 *
 * Returns a pointer to the bitmap if successful, NULL otherwise...
 */

void *LoadDIBitmap(char	*path,	/* I - File to load */
             BITMAPINFO	**info)		/* O - Bitmap information */
{
	FILE           *fp;			/* Open file pointer */
	void           *bits;		/* Bitmap pixel bits */
	unsigned int    bitsize,	/* Size of bitmap */
                	infosize;	/* Size of header information */
	BITMAPFILEHEADER header;	/* File header */

  	// Try opening the file; use "rb" mode to read this *binary* file.

	if ((fp = fopen(path, "rb")) == NULL){
 		printf("Couldn't open bitmap file: %s\n",path);
    	return (NULL);
    }

  	// Read the file header and any following bitmap information...

  	if(ReadFileHeader(&header,fp)){
 		printf("Couldn't read bitmap file header: %s\n",path);
   	    fclose(fp);
      	return NULL;
    }

  	if (header.bfType != 'MB')	// Check for BM reversed...
  	if (header.bfType != 'BM')	// Check for BM normal...
  	{
 	 	printf("Not a bitmap file: %x %s\n",header.bfType,path);
   		fclose(fp);
    	return (NULL);
  	};

  	infosize = header.bfOffBits - HDRSIZE;

	if ((*info = (BITMAPINFO *)malloc(infosize)) == NULL){
		printf("Couldn't allocate memory for bitmap info %s\n",path);
		fclose(fp);
    	return (NULL);
 	};

	if(ReadInfoHeader(*info,fp,infosize)){
		printf("Couldn't read the bitmap info header: %s\n",path);
    	free(*info);
    	fclose(fp);
    	return (NULL);
  	}

  	// Now that we have all the header info read in, allocate memory for the
  	// bitmap and read *it* in...

	if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
    	bitsize = ((*info)->bmiHeader.biWidth *(*info)->bmiHeader.biBitCount + 7) / 8 *abs((int)((*info)->bmiHeader.biHeight));

  	if ((bits = malloc(bitsize)) == NULL) {
		printf("Couldn't allocate memory for bits: %s\n",path);
        free(*info);
    	fclose(fp);
    	return (NULL); // Couldn't allocate memory - return NULL!
  	};

  	if (fread(bits, 1, bitsize, fp) <  (size_t)bitsize) {
		printf("Error reading bitmap data: %s\n",path);
    	free(*info);
    	free(bits);
    	fclose(fp);
    	return (NULL); // Couldn't read bitmap - free memory and return NULL!
  	};

 	// OK, everything went fine - return the allocated bitmap...

  	fclose(fp);
  	return (bits);
}

/*
 * 'SaveDIBitmap()' - Save a bitmap to a DIB/BMP file on disk.
 *
 * Returns 0 if successful, non-zero otherwise...
 */

int SaveDIBitmap(char   *filename,	/* I - File to save to */
	         BITMAPINFO *info,		/* I - Bitmap information */
             void       *bits)		/* I - Bitmap pixel bits */
{
	FILE				*fp;		/* Open file pointer */
  	unsigned int		size,		/* Size of file */
						infosize,	/* Size of bitmap info */
						bitsize;	/* Size of bitmap pixels */
  	BITMAPFILEHEADER	header;		/* File header */

  	// Try opening the file; use "wb" mode to write this *binary* file.

  	if ((fp = fopen(filename, "wb")) == NULL)
    	return (-1);

	if (info->bmiHeader.biSizeImage == 0){	// Figure out the bitmap size
       bitsize = (info->bmiHeader.biWidth *
               info->bmiHeader.biBitCount + 7)
               /8*abs((int)(info->bmiHeader.biHeight));

	}
  	else
    	bitsize = info->bmiHeader.biSizeImage;

  	infosize = sizeof(BITMAPINFOHEADER);

  	switch (info->bmiHeader.biCompression) {
    case BI_BITFIELDS :
         infosize += 12;			// Add 3 RGB doubleword masks
         if (info->bmiHeader.biClrUsed == 0)
	 	 break;
    case BI_RGB :
        if (info->bmiHeader.biBitCount > 8 &&
            info->bmiHeader.biClrUsed == 0)
	  	break;
    case BI_RLE8 :
    case BI_RLE4 :
        if (info->bmiHeader.biClrUsed == 0)
          	infosize += (1 << info->bmiHeader.biBitCount) * 4;
		else
          	infosize += info->bmiHeader.biClrUsed * 4;
		break;
  	};
  	size = HDRSIZE + infosize + bitsize;

  	// Write the file header, bitmap information, and bitmap pixel data...

  	header.bfType      = (SHORT)BM;
  	header.bfSize      = size;
  	header.bfReserved1 = 0;
  	header.bfReserved2 = 0;
  	header.bfOffBits   = HDRSIZE + infosize;

  	if(WriteFileHeader(&header, fp))
      	return -1;
  	if(WriteInfoHeader(info, fp,infosize))
      	return -1;

  	if (fwrite(bits, 1, bitsize, fp) <  (size_t)bitsize) {
     	fclose(fp);
    	return (-1);
  	};
  	// OK, everything went fine - return...

  	fclose(fp);
  	return (0);
}

/*
 * 'ReadDIBitmap()' - Read the current OpenGL viewport into a
 *                    24-bit RGB bitmap.
 *
 * Returns the bitmap pixels if successful and NULL otherwise.
 */

void *ReadDIBitmap(BITMAPINFO **info)		/* O - Bitmap information */
{
	unsigned int    bitsize,	/* Total size of bitmap */
                	width;		/* Aligned width of a scanline */
	int             i, j;
	int             viewport[4];	/* Current viewport */
	void           *bits;		/* RGB bits */
	GLubyte        *rgb,		/* RGB looping var */
               	 	temp;		/* Temporary var for swapping */

 /*
  * Grab the current viewport...
  */

  glGetIntegerv(GL_VIEWPORT, viewport);

 /*
  * Allocate memory for the header and bitmap...
  */

  if ((*info = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER))) == NULL)
  {
   /*
    * Couldn't allocate memory for bitmap info - return NULL...
    */

    return (NULL);
  };

  width   = viewport[2] * 3;		/* Real width of scanline */
  width   = (width + 3) & ~3;		/* Aligned to 4 bytes */
  bitsize = width * viewport[3];	/* Size of bitmap, aligned */
  if ((bits = calloc(bitsize, 1)) == NULL)
  {
   /*
    * Couldn't allocate memory for bitmap pixels - return NULL...
    */

    free(*info);
    return (NULL);
  };

 /*
  * Read pixels from the framebuffer...
  */

  glFinish();				/* Finish all OpenGL commands */
  glPixelStorei(GL_PACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

  glReadPixels(0, 0, viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE,bits);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);

 /*
  * Swap red and blue for the bitmap...
  */

  for (i = 0; i < viewport[3]; i ++)
    for (j = 0, rgb = ((GLubyte *)bits) + i * width;
         j < viewport[2];
	 j ++, rgb += 3)
    {
      temp   = rgb[0];
      rgb[0] = rgb[2];
      rgb[2] = temp;
    };

 /*
  * Finally, initialize the bitmap header information...
  */

  (*info)->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  (*info)->bmiHeader.biWidth         = viewport[2];
  (*info)->bmiHeader.biHeight        = viewport[3];
  (*info)->bmiHeader.biPlanes        = 1;
  (*info)->bmiHeader.biBitCount      = 24;
  (*info)->bmiHeader.biCompression   = BI_RGB;
  (*info)->bmiHeader.biSizeImage     = bitsize;
  (*info)->bmiHeader.biXPelsPerMeter = 2952; /* 75 DPI */
  (*info)->bmiHeader.biYPelsPerMeter = 2952; /* 75 DPI */
  (*info)->bmiHeader.biClrUsed       = 0;
  (*info)->bmiHeader.biClrImportant  = 0;

  return (bits);
}

/*
 * 'ConvertRGB()' - Convert a DIB/BMP image to 24-bit RGB pixels.
 *
 * Returns an RGB pixel array if successful and NULL otherwise.
 */

GLubyte *ConvertRGB(BITMAPINFO *info,	/* I - Original bitmap information */
           void       *bits)		    /* I - Original bitmap pixels */
{
	unsigned int             i, j,		/* Looping vars */
	                bitsize,	/* Total size of bitmap */
	                width;		/* Aligned width of bitmap */
	GLubyte        *newbits;	/* New RGB bits */
	GLubyte        *from, *to;	/* RGB looping vars */
 /*
  * Allocate memory for the RGB bitmap...
  */

  width   = 3 * info->bmiHeader.biWidth;
  width   = (width + 3) & ~3;
  bitsize = width * info->bmiHeader.biHeight;
  if ((newbits = (GLubyte *)calloc(bitsize, 1)) == NULL)
    return (NULL);

 /*
  * Copy the original bitmap to the new array, converting as necessary.
  */

  switch (info->bmiHeader.biCompression)
  {
    case BI_RGB :
        if (info->bmiHeader.biBitCount == 24)
	{
         /*
          * Swap red & blue in a 24-bit image...
          */

          for (i = 0; i < info->bmiHeader.biHeight; i ++)
	    for (j = 0, from = ((GLubyte *)bits) + i * width,
	             to = newbits + i * width;
		 j < info->bmiHeader.biWidth;
		 j ++, from += 3, to += 3)
            {
              to[0] = from[2];
              to[1] = from[1];
              to[2] = from[0];
            };
	};
	break;
    case BI_RLE4 :
    case BI_RLE8 :
    case BI_BITFIELDS :
        break;
  };

  return (newbits);
}

/*
 * 'BitmapToPixels()' - Convert a DIB/BMP image to 24-bit RGB pixels.
 *
 * Returns a pixel array if successful and NULL otherwise.
 */

void * BitmapToPixels(BITMAPINFO *info,	void *bits, int n)
{
	unsigned int         i, j,		/* Looping vars */
                bitsize,	/* Total size of bitmap */
                width;		/* Aligned width of bitmap */
	GLubyte    *newbits;	/* New RGB bits */
	GLubyte    *from, *to;	/* RGB looping vars */

	if (info->bmiHeader.biCompression != BI_RGB || info->bmiHeader.biBitCount != 24){
		printf("ERROR: Only 24 bit bitmaps are currently supported\n");
		return (NULL);
	}

  	// Allocate memory for the RGB bitmap...

	width = n * info->bmiHeader.biWidth;
	width = (width + n) & ~n;
	bitsize = width * info->bmiHeader.biHeight;
	if ((newbits = (GLubyte *) calloc(bitsize, 1)) == NULL){
		printf("Image ERROR: could not allocate pixel memory\n");
		return (NULL);
	}

 	// Copy the original bitmap to the new array, converting as necessary.
	// Swap red & blue in a 24-bit image...

	for (i = 0; i < info->bmiHeader.biHeight; i++){
		for (j = 0, from = ((GLubyte *) bits) + i*3*info->bmiHeader.biWidth,
			to = newbits + i * width;
			j < info->bmiHeader.biWidth;
			j++, from += 3, to += n)
		{
			to[0] = from[2];
			to[1] = from[1];
			to[2] = from[0];
			if(n==4)
				to[3]=255;
		}
	}
	return (newbits);
}

/*
 * 'PixelsToBitmap()' - Convert a 24-bit RGB pixel image to a DIB/BMP.
 *
 * Returns a DIB/BM array if successful and NULL otherwise.
 */
void * PixelsToBitmap(BITMAPINFO *info,	void *bits, int mode)
{
	unsigned int         i, j,		/* Looping vars */
                bitsize,	/* Total size of bitmap */
                width,		/* Aligned width of bitmap */
                w;			/* Aligned width of bitmap */
	GLubyte    *newbits;	/* New RGB bits */
	GLubyte    *from, *to;	/* RGB looping vars */

	if (info->bmiHeader.biCompression != BI_RGB || info->bmiHeader.biBitCount != 24){
		printf("ERROR: Only 24 bit bitmaps are currently supported\n");
		return (NULL);
	}

  	// Allocate memory for the RGB bitmap...

	w = 4 * info->bmiHeader.biWidth;
	width = 3 * info->bmiHeader.biWidth;
	bitsize = width * info->bmiHeader.biHeight;
	if ((newbits = (GLubyte *) calloc(bitsize, 1)) == NULL){
		printf("Image ERROR: could not allocate pixel memory\n");
		return (NULL);
	}

	// RGB mode

	if(mode==0){
		for (i = 0; i < info->bmiHeader.biHeight; i++){
			for (j = 0, from = ((GLubyte *) bits) + i * w,
				to = newbits + i * width;
				j < info->bmiHeader.biWidth;
				j++, from += 4, to += 3)
			{
				to[0] = from[2]; // Swap red & blue in a 24-bit image...
				to[1] = from[1];
				to[2] = from[0];
			}
		}
	}

	// ALPHA mode

	else{
		for (i = 0; i < info->bmiHeader.biHeight; i++){
			for (j = 0, from = ((GLubyte *) bits) + i * w,
				to = newbits + i * width;
				j < info->bmiHeader.biWidth;
				j++, from += 4, to += 3)
			{
			    GLubyte alpha=from[3];
				to[0] = alpha;
				to[1] = alpha;
				to[2] = alpha;
			}
		}
	}
	return (newbits);
}
