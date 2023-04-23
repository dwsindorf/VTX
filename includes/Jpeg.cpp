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

extern void  restore_context();

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
	glReadBuffer(GL_FRONT);

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

