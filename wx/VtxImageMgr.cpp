
#include "GLheaders.h"
#include <stdio.h>
#include <defs.h>
#include "VtxImageMgr.h"

extern void writePixels(GLubyte *image, int image_width, int image_height);
extern GLubyte *readPixels(int &image_width, int &image_height);

void writeJpegFile(char *path,double quality){
	VtxImageMgr::writeJpegFile(path,quality);
}

void showJpegFile(char *path){
	VtxImageMgr::showJpegFile(path);
}
GLubyte *readJpegFile(char *path,int &width, int &height, int &comps){
	return VtxImageMgr::readJpegFile(path,width,height,comps);
}
//************************************************************
// read a BMP file
//************************************************************
void VtxImageMgr::showBmpFile(char *path){
	wxImage bmp(wxString(path),wxBITMAP_TYPE_BMP);
	wxImage img=bmp.Mirror(false); // need to flip vertically for ogl
	int width=img.GetWidth();
	int height=img.GetHeight();
	GLubyte *data=img.GetData();
	if(data>0)
		writePixels(data, width, height);
}

//************************************************************
// read a JPG file
//************************************************************
GLubyte *VtxImageMgr::readJpegFile(char *path,int &width, int &height, int &comps){
	wxImage bmp(wxString(path),wxBITMAP_TYPE_JPEG);
	wxImage img=bmp.Mirror(false); // need to flip vertically for ogl
	width=img.GetWidth();
	height=img.GetHeight();
	if(img.HasAlpha())
		comps=4;
	else
		comps=3;
	if(img.IsOk()){
		// need a copy since img data will be deleted on call exit
		GLubyte *data=img.GetData();
		GLubyte *pxls;
		int size=width*height*comps;
		MALLOC(size,GLubyte,pxls);
		for(int i=0;i<size;i++)
			pxls[i]=data[i];
		return pxls;
	}
	return 0;
}

//************************************************************
// write a BMP file
//************************************************************
void VtxImageMgr::writeBmpFile(char *path)
{
	int width=0;
	int height=0;
	GLubyte *data=readPixels(width,height);
	wxImage bmp(width,height,data);
	bmp.SaveFile(wxString(path), wxBITMAP_TYPE_BMP);
}

//************************************************************
// write a jpeg file
//************************************************************
void VtxImageMgr::writeJpegFile(char *path,double quality)
{
	int width=0;
	int height=0;
	GLubyte *data=readPixels(width,height);
	wxImage img(width,height,data);
	img.SetOption(wxString("quality"), (int)quality);
	img.SaveFile(wxString(path), wxBITMAP_TYPE_JPEG);
}

//************************************************************
// read a jpeg file
//************************************************************
void VtxImageMgr::showJpegFile(char *path)
{
	wxImage bmp(wxString(path),wxBITMAP_TYPE_JPEG);
	wxImage img=bmp.Mirror(false); // need to flip vertically for ogl
	int width=img.GetWidth();
	int height=img.GetHeight();
	GLubyte *data=img.GetData();
	if(data>0)
		writePixels(data, width, height);
}

