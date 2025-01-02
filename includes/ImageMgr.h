// ImageMgr.h

#ifndef _IMAGECLASS
#define _IMAGECLASS

#include "GLheaders.h"
#include "defs.h"
#include "ColorClass.h"
#include "ListClass.h"
#include "FileUtil.h"
#include "Util.h"

class TNarg;
class TNode;
class TNinode;

//************************************************************
// Image class
//************************************************************
enum {
	RGBA_DATA    = 0,    	// char data
	RGB_DATA	 = 1,    	// char data
	FLOAT_DATA   = 2     	// float data
};

enum  {
    // TNbands, TNimage types

	PMAP   		= 0x00000001,
	SMAP  		= 0x00000002,
	CMAP  		= 0x00000003,
	TILE  		= 0x00000000,
	IMAP 		= 0x00000003,

    // TNbands, TNimage image generation options

	INVT    	= 0x00000004,   // invert image colors
	GRAY    	= 0x00000008,   // grayscale image
	ACHNL    	= 0x00000010,   // add alpha channel
	NORM    	= 0x00000020,   // normalize intensity
	CLAMP       = 0x00000040,   // don't wrap bands color spline
	REFLECT     = 0x00000080,   // add reflect colors from end
	INOPTS 		= 0x000000ff,

    // TNtexture options

	MODULATE    = 0x00000000,   //
	DECAL		= 0x00000100, 	// style options
	BLEND       = 0x00000200,
	REPLACE     = 0x00000300, 	// replace
	STYLE       = MODULATE|DECAL|BLEND|REPLACE, 	// style mask
	MMAP        = 0x00000000, 	// GL_LINEAR_MIPMAP_LINEAR  (default)
	INTERP      = 0x00000400, 	// GL_LINEAR
	NEAREST     = 0x00000800,   // GL_NEAREST
	INTRP_MASK  = MMAP|INTERP|NEAREST, 	// interp mask
	REPEAT      = 0x00000000, 	// wrap options
	BORDER      = 0x00001000, 	// border flag (GL_CLAMP)
	TEX         = 0x00002000,   // default
	BUMP	    = 0x00004000, 	// bump map flag
	HMAP   	    = 0x00008000,
	TBIAS       = 0x00010000,   // random flip
	RANDOMIZE   = 0x00020000,   // random flip
	SEXPR       = 0x00040000, 	// S expression in argument list
	AEXPR       = 0x00080000, 	// T expression in argument list
	TXOPTS	    = 0x000fffA0, 	// texture opts mask

    // ImageMgr file options

	INUSE       = 0x00100000,   // image is in use
	NEWIMG      = 0x00200000,   // image is new
	CHANGED     = 0x00400000,   // image is modified

	BANDS   	= 0x10000000,
	IMAGE   	= 0x20000000,
	T1D	        = 0x40000000, 	// 1D flag
	T2D	        = 0x80000000, 	// 2D flag
	BMP    	    = 0x00000100,
	JPG     	= 0x00000200,
	PNG     	= 0x00000300,
	TILED 	    = 0x04000000,
	IMCOLS      = 0x0000000f,
	IMROWS      = 0x000000f0,
	SPX     	= 0x01000000,
	IMPORT   	= 0x02000000,
	MAP   	    = 0x03000000,
	SPRITE   	= 0x04000000,
	BRANCH      = 0x05000000,
	LEAF        = 0x06000000,
	DIMS 		= T1D|T2D,
	IOPTS 	    = TXOPTS|INOPTS,
	FTYPE       = BMP|JPG|PNG,
	IFTYPE 		= BUMP|TEX|HMAP,
	SPXTYPE 	= BANDS|IMAGE,
	IMTYPE 	    = 0x07000000,
	ALLI    	= IFTYPE|SPXTYPE|IMTYPE|BUMP|T1D|T2D

};

class Image
{
	typedef struct idata {
	unsigned int  type		 : 2;	// type code
	unsigned int  accessed	 : 1;	// type code
	unsigned int  newimg	 : 1;	// type code
	unsigned int  changed	 : 1;	// type code
	unsigned int  keep 	     : 1;	// type code
	unsigned int  alpha 	 : 1;	// type code
	unsigned int  alpha_image: 1;	// type code
	unsigned int  global 	 : 1;	// type code
    } idata;

    typedef union idu {
    	idata	     flags;
    	int          l;
     } idu;

public:
	void    *data;
	int		width;		// texture width
	int		height;		// texture height
	idu     opts;

	Image(RGBColor *b, int h, int w);
	Image(Color *b, int h, int w);
	Image(float  *b, int h, int w);
	//Image(int opts, int h, int w,double rh,double rw, TNode  *b);
	Image(int opts, int h, int w, TNode  *b, Image *grad);

	Image();

	~Image();

	int valid()				{ return data ? 1:0;}
	void set_global(int t) 	{ opts.flags.global=t;}
	void set_type(int t) 	{ opts.flags.type=t;}
	void set_accessed(int t){ opts.flags.accessed=t;}
	int accessed() 			{ return opts.flags.accessed;}
	void set_changed(int t) { opts.flags.changed=t;}
	int changed() 			{ return opts.flags.changed;}
	void set_newimg(int t)  { opts.flags.newimg=t;}
	int newimg() 			{ return opts.flags.newimg;}
	void set_keep(int t) 	{ opts.flags.keep=t;}
	int global() 			{ return opts.flags.global;}
	int keep() 				{ return opts.flags.keep;}
	int type() 				{ return opts.flags.type;}
	int	comps()				{ switch(type()){
								case RGBA_DATA:	return 4;
								case RGB_DATA:	return 3;
									default:	return 1;
							  }
							}
	int	gltype()			{ switch(type()){
								case RGBA_DATA:	return GL_RGBA;
								case RGB_DATA:	return GL_RGB;
									default:	return 0;
							  }
							}
	void set_alpha(int i)	{ opts.flags.alpha=i;}
	void set_alpha_image(int i)	{ opts.flags.alpha_image=i;}
	int alpha()				{ return opts.flags.type==RGBA_DATA ? tx1d()&&opts.flags.alpha:0;}
	int alpha_image()       { return opts.flags.alpha_image;}
	int txtype() 			{ return (height==1 || width==1)?GL_TEXTURE_1D:GL_TEXTURE_2D;}
	int tx1d()				{ return (height==1 || width==1)?1:0;}
	int tx2d()				{ return (height>1 && width>1)?1:0;}

	double aspect()         { return (double)height/(double)width;}
	int size()              { return height*width;}
	
	void addAlphaToImage(GLubyte *a);

	FColor color(int,double); //1d
	FColor color(int,double,double);  //2d
	double value(int,double,double);  //2d

	double lerp(double s, double e, double t){return s+(e-s)*t;}
	double blerp(double c00, double c10, double c01, double c11, double tx, double ty){
	    return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
	}
 	double cubic( double v0, double v1, double v2, double v3, double frac);

	double p2v(int index);
	double p2v(double r, double c);

	Color aveColor();

	Color p2c(int index);
	Color p2c(double r, double c);

	Image *clone();
};

//************************************************************
// ImageSym class
//************************************************************
class ImageSym {
public:
	uint 			   info;
	Image 			   *image;
	char			   *text;
	char			   *istring;
	char			   *path;
	char *name()		{ return (char*)text;}
	int  cmp(char *s)	{ return strcmp(text,s);}
	ImageSym(char *t, Image *i);
	ImageSym(char *t, uint i);
	ImageSym(ImageSym *i);
	ImageSym(int m, char *t, Image *i,char *s,char *p);
	~ImageSym();
	void setName(char *t);
	void setIstring(char *t);
	void setPath(char *t);
	void print();
	void infoString(char *);
};

//************************************************************
// ImageReader class
//************************************************************
class ImageReader
{
    enum  {
		BUILDING   	  = 0x1000,
		NEED_REBUILD  = 0x2000
	};

	int   flags;
	void addImage(char *f, int m, char *is, Image *im, char *p);

public:
	NameList<ImageSym*>   images;

	ImageReader();
	~ImageReader();

	uint getFileInfo(char *f);
	uint getImageInfo(char *f,char *dir);
	uint getTiledImageInfo(char *f,char *dir);
	uint getFileInfo(char *n,char *d);
	void hashName(char *f, int &m, char *fn);
	void unhashName(char *f, int &m, char *fn);
	ImageSym *getImageInfo(char *f);
	void printImageInfo(char *f);
	void showImageInfo();
	void set_building(int i)     { BIT_SET(flags,BUILDING,i);}
	int building()               { return flags&BUILDING;}

	void invalidate()            { BIT_ON(flags,NEED_REBUILD);}
	void validate()              { BIT_OFF(flags,NEED_REBUILD);}

	int invalid()               { return flags&NEED_REBUILD;}

	void getImageInfo(int, LinkedList<ImageSym*> &list);

	void free();
	Image *find(char *f);
	void remove(char *f);
	void removeAll(char *f);
	Image *load(char *f,int mode);
	Image *load(char *f,TNinode *);
	Image *open(char *f);
	Image *open(char *f,char *dir);
	Image *openBmpFile(char *name,char *path);
	Image *openJpgFile(char *name,char *path);
	Image *openPngFile(char *name,char *path);

	char *readSpxFile(char *f);
	int saveSpxFile(char *name, char *buff);

	void save(char *f, Image *i);
	void save(char *f, Image *i, TNinode *);
	void imagedir(char *d);
	void getImagePath(char *f,char *d);
	void printImageInfo(int);
	void makeImagelist();
	void addImages(char *base);
	void addTiledImages(char *base);
	void clear_flags();
	static void getImageDims(char *s,uint &cols,uint &rows);
	static void getImageDims(uint info, uint &cols,uint &rows);
	static void setImageDims(uint &info, uint cols,uint rows);
};

extern ImageReader images;

class ImageMgr {
protected:
	char base_dir[512];
public:	
	ValueList<FileData*> image_dirs;	
	
	virtual void setImageBaseDir(){}
	void setImageBaseDir(char *b){
		strcpy(base_dir,b);
		getImageDirs();
	}
	char *getImageBaseDir(){
		return base_dir;
	}	
	void getImageDirs(){
	   if(image_dirs.size>0)
		   image_dirs.free();
		File.getDirectoryList(getImageBaseDir(),image_dirs);
		image_dirs.sort();		
	}
	
	static char *Istr;
	static char *Bstr;

};
class ImageInfo {
protected:
	char image_file[64];
	char image_dir[64];
	
	ImageMgr *image_mgr;
public:
	uint image_cols;
	uint image_rows;
	uint image_center;
	
	Image *image;

	ImageInfo(){
		image_file[0]=0;
		image_dir[0]=0;
		image_cols=0;
		image_rows=0;
		image_center=0;
		image_mgr=0;
		image=0;
	}
	void setImageName(char *s){
		strcpy(image_file,s);
	}
	int getImageCenter(){return image_center;}
	void setImageCenter(uint i){image_center=i;}
	int getImageRows(){return image_rows;}
	int getImageCols() {return image_cols;}
	void setImageMgr(ImageMgr *m){image_mgr=m;}
	ImageMgr *getImageMgr(){return image_mgr;}
	
	void getImageDims(uint &cols,uint &rows){
		rows=image_rows;
		cols=image_cols;
	}
	void getImageDims(char *s,uint &cols,uint &rows){
		int i=0,j=0;
		sscanf(s,"%dx%d",&i,&j);
		rows=j;
		cols=i;
	}
	void setImage(char *name);
	bool getImageFilePath(char*name,char *dir);
	char *getBaseDir(){
		if(!image_mgr)
			return "..";
		return image_mgr->getImageBaseDir();
	}
	void getImageDirPath(char *dir,char *path){
		if(dir)
			sprintf(path,"%s/%s",getBaseDir(),dir);
		else
			sprintf(path,"%s",getBaseDir());		
	}
	char *getImageFile() { return image_file;}
	char *getImageDir()  { return image_dir;}
	bool imageFileExists(char *name,char *base);


};

#endif

