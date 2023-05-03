#ifndef VTXIMAGEMGR_H_
#define VTXIMAGEMGR_H_
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class VtxImageMgr
{
protected:
public:
	static void showBmpFile(char *path);
	static void writeBmpFile(char *path);
	static void writeJpegFile(char *path,double quality);
	static void showJpegFile(char *path);
	static bool writeBmpFile(int w, int h,void *data, char *path, bool static_data);
	static bool writePngFile(int w, int h,void *data, void *adata,char *path, bool static_data);

	static GLubyte *readImageFile(int type,char *path,int &width, int &height, int &comps);
};
#endif /*VTXIMAGEMGR_H_*/
