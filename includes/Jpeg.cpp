/*
 * jpeg.c
 *
 * jpeg conversion routines
  */

#include <stdarg.h>
#include "GLheaders.h"
#include <stdio.h>

#include <setjmp.h>
#include "SceneClass.h"
#include "Bitmap.h"

extern void  restore_context();
#ifndef WXWIN
void write_JPEG_file (char * filename, GLubyte *image, int width, int height,double quality);
int read_JPEG_file (char * filename, GLubyte **image, int &width, int &height, int &comps);

extern "C" {
#include <jpeglib.h>
}
#endif
//************************************************************
//write image to the frame buffer
//************************************************************
void writePixels(GLubyte *image, int image_width, int image_height)
{
	int viewport[4];	// Current viewport
    glGetIntegerv(GL_VIEWPORT, viewport);

 	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	double inset=5;
	double y1=-1,x1=-1;
	double ys,xs;

	glClearColor(1.0f,1.0f,0.0f,1.0f);
 	glClear(GL_COLOR_BUFFER_BIT);

	x1=-1+inset/image_width;
	y1=-1+inset/image_height;

	glRasterPos4d(x1,y1,0,1);

	xs=((double)viewport[2]-inset)/(double)image_width;
	ys=((double)viewport[3]-inset)/(double)image_height;
	glPixelZoom((float)xs,(float)ys);

	glDrawPixels(image_width,image_height,GL_RGB, GL_UNSIGNED_BYTE,image);

	glPixelZoom(1,1);
	glRasterPos4d(-1,-1,0,1);
	glEnable(GL_DEPTH_TEST);
}

//************************************************************
// get image from frame buffer
//************************************************************
GLubyte *readPixels(int &image_width, int &image_height) {
	int bitsize; 		// Total size of bitmap
	int viewport[4]; 	// Current viewport
	GLubyte *bits; 		// RGB bits
	GLubyte *rgb = 0; 	// row-inverted RGB bits
	GLubyte *from, *to; // RGB looping vars
	int i, j, row;

	glGetIntegerv(GL_VIEWPORT, viewport);

	image_width = viewport[2];
	image_height = viewport[3];
	row = image_width * 3;
	bitsize = row * image_height; // Size of bitmap, aligned

	if ((bits = (GLubyte *) calloc(bitsize, 1)) == NULL) {
		return 0;
	};
	if ((rgb = (GLubyte *) calloc(bitsize, 1)) == NULL) {
		FREE(bits);
		return 0;
	};

	// Read pixels from the framebuffer

	glFinish();
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glReadBuffer(GL_BACK);

	glReadPixels(0, 0, image_width, image_height, GL_RGB, GL_UNSIGNED_BYTE,
			bits);

	// row invert image

	from = bits + (image_height - 1) * row;
	to = rgb;
	for (i = 0; i < image_height; i++) {
		for (j = 0; j < row; j++)
			to[j] = from[j];
		from -= row;
		to += row;
	}

	FREE(bits);
	return rgb;
}

#ifndef WXWIN
//************************************************************
// read a BMP file
//************************************************************
void showBmpFile(char *path){

    BITMAPINFO  *info=0;

    GLubyte     *bits = (GLubyte*)LoadDIBitmap(path,&info);
	if (!bits || !info) {
		return;
	};

	GLubyte  *pxls=(GLubyte*)BitmapToPixels(info,bits,3);

	int width=info->bmiHeader.biWidth;
	int height=info->bmiHeader.biHeight;

	if (pxls) {
		writePixels(pxls, width, height);
		FREE(pxls);
	};

	FREE(info);
	FREE(bits);
}

//************************************************************
// write a BMP file
//************************************************************
void writeBmapFile(char *fullpath)
{
    void        *bits;
    BITMAPINFO  *info;

	bits = ReadDIBitmap(&info);
	if (!bits)
		return;
	SaveDIBitmap(fullpath, info, bits);
	FREE(info);
	FREE(bits);
}
//************************************************************
//write jpeg file
//************************************************************
void writeJpegFile(char *filename,double jpeg_quality)
{
	int image_width;
	int image_height;
	GLubyte *rgb=readPixels(image_width, image_height);
	if(rgb){
		write_JPEG_file (filename, rgb, image_width,image_height,jpeg_quality);
		FREE(rgb);
	}
}

//************************************************************
//read jpeg file
//************************************************************
void showJpegFile(char *filename)
{
	int width=0;
	int height=0;
	int comps=0;

    GLubyte *image=0;
    if(read_JPEG_file(filename,&image,width,height,comps)){
    	writePixels(image,width,height);
    	FREE(image);
    }
}
//******************* JPEG COMPRESSION  *******************
void write_JPEG_file (char * filename, GLubyte *image, int width, int height, double quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;
	JSAMPROW row_pointer[1]; 		// pointer to GLubyte row[s]
	int row_stride; 				// physical row width in image buffer
	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);
	if ((outfile = fopen(filename, "wb")) == NULL) {
		printf("can't open %s\n", filename);
		return;
	}
	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = width; 		// image width and height, in pixels
	cinfo.image_height = height;
	cinfo.input_components = 3; 	// # of color components per pixel
	cinfo.in_color_space = JCS_RGB; // colorspace of input image
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);

	jpeg_start_compress(&cinfo, TRUE);
	row_stride = width * 3; 		// JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = & image[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);
}

//******************** JPEG DECOMPRESSION  *******************
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

/*
 * routine for JPEG decompression.  We assume that the source file name
 * is passed in.  We want to return 1 on success, 0 on error.
 */

int read_JPEG_file (char * filename, GLubyte **image, int &width, int &height, int &comps)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FILE * infile;  // source file
	int row_stride; // physical row width in output buffer

	if ((infile = fopen(filename, "rb")) == NULL)
		return 0;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return 0;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width * cinfo.output_components;

	int oldsize=width*height*comps;
	int newsize=cinfo.output_width*cinfo.output_height*cinfo.output_components;
	if(newsize>oldsize || *image==0) {
		FREE(*image);
		MALLOC(newsize,GLubyte,*image);
		if(image==0)
		return 0;
	}
	width=cinfo.output_width;
	height=cinfo.output_height;
	newsize=row_stride*cinfo.output_height;
	comps=cinfo.output_components;

	while (cinfo.output_scanline < cinfo.output_height) {
		GLubyte *tmp[1];
		// invert image for right-side up view
		tmp[0]=*image+(cinfo.output_height-cinfo.output_scanline-1)*row_stride;
		// jpeg_read_scanlines expects an array of pointers to scanlines.
		(void) jpeg_read_scanlines(&cinfo, tmp, 1);
	}
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);
	return newsize;
}
GLubyte *readJpegFile(char *path,int &width, int &height, int &comps){
	GLubyte *pxls=0;
	
	return read_JPEG_file (path, pxls, width, height, comps);
}

#endif
