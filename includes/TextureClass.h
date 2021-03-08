// TextureClass.h

#ifndef _TEXTURECLASS
#define _TEXTURECLASS

#include "GLSupport.h"
#include "ImageMgr.h"
#include "PointClass.h"
#include "defs.h"
#include <math.h>

//#define DPTEXTURES

class TNode;
class Image;

//************************************************************
// Texture class
//************************************************************
class Texture
{
	int    options;
	bool   valid;
	GLuint id[2];
	void del();
	void set_state();

public:
	static int num_attrs;
	static int num_coords;

	Image  *timage;
	Texture(Image *i, int f, TNode *e);
	~Texture();
	double  s;
	double  t;
	double  scale;
    double  tvalue;
    double  svalue;

	double texamp;

	double bumpamp;
	double bump_mip;
	double bump_damp;
	double bias;

	double  orders_delta;
	double  orders_atten;
	double  orders;

	double hmap_amp;
	double hmap_bias;

	double phi_bias;
	double height_bias;
	double bump_bias;
	double slope_bias;

	static double mipbias;

	bool enabled;
	bool tex_active;
	bool bump_active;
	bool hmap_active;

	bool coords_active;

	bool s_data;
	bool a_data;

	int type;
	int tid;
	int cid;
	TNode  *expr;

	int style()				{ switch(options & STYLE){
								default    : return GL_MODULATE;
							    case DECAL : return GL_DECAL;
							    case BLEND : return GL_BLEND;
							    case REPLACE : return GL_REPLACE;
							   }
							}
	int intrp()				{ if(options & NEAREST)
								 return GL_NEAREST;
							  else if(options & INTERP)
							     return GL_LINEAR;
							  else
								 return GL_LINEAR_MIPMAP_LINEAR;
							}
    bool nearest()          { return intrp()==GL_NEAREST  ? true:false;}
    bool linear()           { return intrp()==GL_LINEAR ? true:false;}
    bool mipmap()              { return intrp()== GL_LINEAR_MIPMAP_LINEAR? true:false;}

	bool randomized()        { return (options & RANDOMIZE)?true:false;}
	int wrap()				{ return options & BORDER ? GL_CLAMP_TO_EDGE:GL_REPEAT;}
	int t1d()				{ return (width()==1 || height()==1)?1:0;}
	int t2d()				{ return (width()>1 && height()>1)?1:0;}
	int comps()				{ return image()->comps();}
	int gltype()			{ return image()->gltype();}
	int txtype()			{ return type;}
	int width()				{ return image()->width;}
	int height()			{ return image()->height;}
	bool alpha_enabled()	{ return options & RGBA;}
	bool normalize()	    { return options & NORM;}
	const void *pixels();
	Image *image();
	void begin();
	void end();
	void texCoords(int);
	void bumpCoords(int,double s,double t);
	void texCoords(int,double s,double t);
	void getTexCoords(double& s, double & t);
	double getTexAmpl(int mode);
	void eval();

	void enable()			{ glEnable(type); }
	void disable()			{ glDisable(type); }
	bool invalid() 	    	{ return !valid; }
	void invalidate()       { valid=false;}
	bool setProgram();
	bool initProgram();
    static void reset();


};
#endif

