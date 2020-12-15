// ImageMgr.h

#ifndef _IMAGECLASS
#define _IMAGECLASS

#include "GLheaders.h"
#include "defs.h"
#include "ColorClass.h"
#include "ListClass.h"

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
	INOPTS 		= 0x0000007f,

    // TNtexture options

	MODULATE    = 0x00000000,   //
	DECAL		= 0x00000080, 	// style options
	BLEND       = 0x00000180,
	REPLACE     = 0x00000100, 	// style mask
	STYLE       = REPLACE|DECAL|BLEND|MODULATE, 	// style mask
	MMAP        = 0x00000000, 	// GL_LINEAR_MIPMAP_LINEAR  (default)
	INTERP      = 0x00000200, 	// GL_LINEAR
	NEAREST     = 0x00000400,   // GL_NEAREST
	INTRP_MASK  = MMAP|NEAREST|INTERP, 	// interp mask
	REPEAT      = 0x00000000, 	// wrap options
	BORDER      = 0x00000800, 	// border flag (GL_CLAMP)
	TEX         = 0x00000000,   // default
	BUMP	    = 0x00001000, 	// bump map flag
	HMAP   	    = 0x00002000,
	NTEX        = 0x00004000,   // texture turned off
	RGBA        = 0x00010000, 	// generate texture alpha from intensity
	SEXPR       = 0x00020000, 	// S expression in argument list
	AEXPR       = 0x00040000, 	// T expression in argument list
	TXOPTS	    = 0x0007ffA0, 	// texture opts mask

    // ImageMgr file options

	INUSE       = 0x00100000,   // image is in use
	NEWIMG      = 0x00200000,   // image is new
	CHANGED     = 0x00400000,   // image is modified

	T1D	        = 0x01000000, 	// 1D flag
	T2D	        = 0x02000000, 	// 2D flag
	DIMS 		= T1D|T2D,
	BMP    	    = 0x04000000,
	JPG     	= 0x08000000,
	BANDS   	= 0x10000000,
	IMAGE   	= 0x20000000,
	SPX     	= 0x40000000,
	IMPORT   	= 0x80000000,
	MAP   	    = 0x80000002,
	IMOPTS 		= 0xff000000,
	IOPTS 	    = TXOPTS|INOPTS,
	IFTYPE 		= BUMP|TEX|SPX,
	SPXTYPE 	= BANDS|IMAGE,
	IMTYPE 	    = IMPORT|MAP,
	ALLI    	= IFTYPE|SPXTYPE|IMTYPE|BUMP|HMAP|TEX|T1D|T2D

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
	Image(int opts, int h, int w,double rh,double rw, TNode  *b);
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
	int alpha()				{ return opts.flags.type==RGBA_DATA ? opts.flags.alpha:0;}
	int alpha_image()       { return opts.flags.alpha_image;}
	int txtype() 			{ return (height==1 || width==1)?GL_TEXTURE_1D:GL_TEXTURE_2D;}
	int tx1d()				{ return (height==1 || width==1)?1:0;}
	int tx2d()				{ return (height>1 && width>1)?1:0;}
	void addAlphaToImage(GLubyte *a);

	double aspect()         { return (double)height/(double)width;}
	int size()              { return height*width;}

	Color color(int,double);
	Color color(int,double,double);
	Color p2c(int index);

	Image *clone();
};

//************************************************************
// ImageSym class
//************************************************************
class ImageSym {
public:
	int 			   info;
	Image 			   *image;
	char			   *text;
	char			   *istring;
	char *name()		{ return (char*)text;}
	int  cmp(char *s)	{ return strcmp(text,s);}
	ImageSym(char *t, Image *i);
	ImageSym(char *t, int i);
	ImageSym(ImageSym *i);
	ImageSym(int m, char *t, Image *i,char *s);
	~ImageSym();
	void setName(char *t);
	void setIstring(char *t);
	void print();
};

//************************************************************
// ImageReader class
//************************************************************
class ImageReader
{
    enum  {
		BUILDING   	= 0x1000
	};

	int   flags;
	void addImage(char *f, int m, char *is, Image *im);
public:
	NameList<ImageSym*>   images;

	ImageReader();
	~ImageReader();

	int getFileInfo(char *f);
	int getFileInfo(char *f,char *dir);
	void hashName(char *f, int &m, char *fn);
	void unhashName(char *f, int &m, char *fn);
	ImageSym *getImageInfo(char *f);
	void printImageInfo(char *f);

	void set_building(int i)     { BIT_SET(flags,BUILDING,i);}
	int building()               { return flags&BUILDING;}

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

	char *readSpxFile(char *f);
	int saveSpxFile(char *name, char *buff);

	void save(char *f, Image *i);
	void save(char *f, Image *i, TNinode *);
	void imagedir(char *d);
	void getImagePath(char *f,char *d);
	void printImageInfo(int);
	void makeImagelist();
	void clear_flags();
};

extern ImageReader images;
#endif

