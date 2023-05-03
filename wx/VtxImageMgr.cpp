
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
	return VtxImageMgr::readImageFile(wxBITMAP_TYPE_JPEG,path,width,height,comps);
}
GLubyte *readBmpFile(char *path,int &width, int &height, int &comps){
	return VtxImageMgr::readImageFile(wxBITMAP_TYPE_BMP,path,width,height,comps);
}
GLubyte *readPngFile(char *path,int &width, int &height, int &comps){
	return VtxImageMgr::readImageFile(wxBITMAP_TYPE_PNG,path,width,height,comps);
}

bool writeBmpFile(int w, int h,void *data, char *path, bool static_data){
	 return VtxImageMgr::writeBmpFile(w,h,data,path,static_data);
}
bool writePngFile(int w, int h,void *data,void *adata,char *path,bool static_data){
	return VtxImageMgr::writePngFile(w,h,data,adata,path,static_data);
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
	if(data!=0)
		writePixels(data, width, height);
}

//************************************************************
// read a JPG file
//************************************************************
GLubyte *VtxImageMgr::readImageFile(int type,char *path,int &width, int &height, int &comps){
	wxImage bmp(wxString(path),type);
	wxImage img=bmp.Mirror(false); // need to flip vertically for ogl
	width=img.GetWidth();
	height=img.GetHeight();
	GLubyte *data=0;
	GLubyte *adata=0;

	if(img.IsOk()){
		data=img.GetData();
		if(img.HasAlpha()){
			comps=4;
			adata=img.GetAlpha();
		}
		else{
			comps=3;
		}
		// need a copy since img data will be deleted on call exit
		GLubyte *pxls;
		int size=width*height;
		CALLOC(size*4,GLubyte,pxls);
		for(int i=0;i<size;i++){
			pxls[i*4]=data[i*3];
			pxls[i*4+1]=data[i*3+1];
			pxls[i*4+2]=data[i*3+2];
			if(comps==4)
				pxls[i*4+3]=adata[i];
			else
				pxls[i*4+3]=255;
		}
		return pxls;
	}
	return 0;
}

//************************************************************
// write a PNG file from image data
//************************************************************
bool VtxImageMgr::writePngFile(int w, int h, void *data, void *adata, char *path,bool static_data)
{
	wxImage bmp(w,h,data,static_data);
	return bmp.SaveFile(wxString(path), wxBITMAP_TYPE_PNG);
}
//************************************************************
// write a BMP file from image data
//************************************************************
bool VtxImageMgr::writeBmpFile(int w, int h, void *data, char *path, bool static_data)
{
	wxImage img(w,h,data,static_data);
	return img.SaveFile(wxString(path), wxBITMAP_TYPE_BMP);
}

//************************************************************
// write a BMP file from screen buffer
//************************************************************
void VtxImageMgr::writeBmpFile(char *path)
{
	int width=0;
	int height=0;
	char *data=readPixels(width,height);
	wxImage img(width,height,data);
	img.SaveFile(wxString(path), wxBITMAP_TYPE_BMP);
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
	if(data!=0)
		writePixels(data, width, height);
}

