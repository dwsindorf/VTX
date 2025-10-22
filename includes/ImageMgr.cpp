// ImageMgr.cpp

#include "ImageMgr.h"
#include "FileUtil.h"
#include "Util.h"
#include "TerrainClass.h"
#include "AdaptOptions.h"
#include "ModelClass.h"
#include "SceneClass.h"
#include "ImageClass.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TEST
//#define USE_WX_BITMAP

ImageReader images; // global image manager

extern double Rand();
extern double Hscale;
extern double Red,Green,Blue,Alpha,Theta,Phi;

extern GLubyte *readJpegFile(char *path,int &w, int &h, int &c);
extern GLubyte *readBmpFile(char *path,int &w, int &h, int &c);
extern GLubyte *readPngFile(char *path,int &w, int &h, int &c);
extern bool writeBmpFile(int w, int h,void *data, char *path, bool);
extern bool writePngFile(int w, int h,void *data,void *adata,char *path,bool);

//#define DEBUG_IMAGES
//#define DEBUG_IMAGE_INFO

int icnt1=0;
int icnt2=0;

static char *ColorToRGB(int width, int height, unsigned char *pxls){
	int n=width*height;
	char *rptr=calloc(n,3);
	for(int i=0;i<n;i++){
		rptr[i*3]=pxls[i*4];
		rptr[i*3+1]=pxls[i*4+1];
		rptr[i*3+2]=pxls[i*4+2];
	}
	return rptr;
}

static char *AlphaToRGB(int width, int height, unsigned char *bits){
	int n=width*height;
	char *rptr=calloc(n,3);
	for(int i=0;i<n;i++){
		GLubyte aval=bits[i*4+3];
		rptr[i*3]=aval;
		rptr[i*3+1]=aval;
		rptr[i*3+2]=aval;
	}
	return rptr;
}
// image test routines
Color *test1d(int h)
{
	static Color tcols[]={
		Color(1.0,0.0,0.0),
		Color(1.0,1.0,0.0),
		Color(0.0,1.0,0.0),
		Color(0.0,1.0,1.0),
		Color(0.0,0.0,1.0),
		Color(1.0,0.0,1.0),
		Color(1.0,1.0,1.0),
		Color(1.0,1.0,0.0)
	};

	int nmax=sizeof(tcols)/sizeof(Color);
	int n=h>=nmax?nmax:h;

	Color *color=0;

	MALLOC(n,Color,color);
	for(int i=0;i<n;i++){
		color[i]=tcols[i];
	}
	return color;
}

Color *test2d(int h, int w, Color c)
{
	static Color tcols[]={
		Color(1.0,0.0,0.0),
		Color(1.0,1.0,0.0),
		Color(0.0,1.0,0.0),
		Color(0.0,1.0,1.0),
		Color(0.0,0.0,1.0),
		Color(1.0,0.0,1.0),
		Color(1.0,1.0,1.0),
		Color(1.0,1.0,0.0)
	};

	Color *color=0;

	MALLOC(h*w,Color,color);
	for(int i=0;i<h;i++){
		Color c1=tcols[i];
		for(int j=0;j<w;j++){
			color[i*h+j]=c1.blend(c,Rand());
		}
	}
	return color;
}

//************************************************************
// ImageSym class
//************************************************************
ImageSym::ImageSym(char *t, Image *i)
{
	image=i;
	text=istring=path=0;
	setName(t);
}

ImageSym::ImageSym(char *t, uint i)
{
	image=0;
	text=istring=path=0;
	setName(t);
	info=i;
}

ImageSym::ImageSym(int m, char *t, Image *i, char *s, char *p)
{
	image=i;
	info=m;
	text=istring=path=0;
	setName(t);
	setIstring(s);
	setPath(p);
}

ImageSym::ImageSym(ImageSym *is)
{
	image=0;
	text=istring=path=0;
	info=is->info;
	setName(is->text);
	setIstring(is->istring);
	setPath(is->path);
}

ImageSym::~ImageSym()
{
	FREE(text);
	FREE(istring);
    DFREE(image);
    DFREE(path);
}
void ImageSym::setName(char *t)
{
	FREE(text);
	if(t){
		MALLOC(strlen(t)+1,char,text);
		strcpy(text,t);
	}
}
void ImageSym::setIstring(char *s)
{
	FREE(istring);
	if(s){
		MALLOC(strlen(s)+1,char,istring);
		strcpy(istring,s);
	}
}
void ImageSym::setPath(char *t)
{
	FREE(path);
	if(t){
		MALLOC(strlen(t)+1,char,path);
		strcpy(path,t);
	}
}
//-------------------------------------------------------------
// ImageSym::print   print info
//-------------------------------------------------------------
void ImageSym::infoString(char *tmp)
{
	tmp[0]=0;
    switch(info&IMTYPE){
    case MAP:
    	sprintf(tmp+strlen(tmp),"%s","MAP");
    	break;
    case HTMAP:
     	sprintf(tmp+strlen(tmp),"%s","HTMAP");
     	break;
    case IMPORT:
    	sprintf(tmp+strlen(tmp),"%s","IMPORT");
    	break;
    case SPX:
    	sprintf(tmp+strlen(tmp),"%s","SPX");
		if(info&TMP)
			sprintf(tmp+strlen(tmp),"%s","|TMP");
		if(info&BANDS)
			sprintf(tmp+strlen(tmp),"%s","|BANDS");
		else if(info&IMAGE)
			sprintf(tmp+strlen(tmp),"%s","|IMAGE");
		if(info&T1D)
			sprintf(tmp+strlen(tmp),"%s","|1D");
		else
			sprintf(tmp+strlen(tmp),"%s","|2D");
		if(info&HMAP)
			sprintf(tmp+strlen(tmp),"%s","|HMAP");
		if(info&BUMP)
			sprintf(tmp+strlen(tmp),"%s","|BUMP");
   	break;
    case SPRITE:
    	sprintf(tmp+strlen(tmp),"%s","SPRITE");
    	break;
    case BRANCH:
    	sprintf(tmp+strlen(tmp),"%s","BRANCH");
    	break;
    case LEAF:
    	sprintf(tmp+strlen(tmp),"%s","LEAF");
    	break;
    }

    if(info & TILED){
    	uint rows=0;
    	uint cols=0;
    	ImageReader::getImageDims(info,cols,rows);
    	sprintf(tmp+strlen(tmp),"|%dx%d",cols,rows);
    } 
    switch(info&FTYPE){
      	case BMP:
      		if(info & ACHNL)
         		sprintf(tmp+strlen(tmp),"%s","|BMPA");
      		else
      			sprintf(tmp+strlen(tmp),"%s","|BMP");
      	break;
      	case JPG:sprintf(tmp+strlen(tmp),"%s","|JPG");break;
      	case PNG:sprintf(tmp+strlen(tmp),"%s","|PNG");break;
    }
 }
std::string ImageSym::fullPath(){
	std::string pstring=basePath()+name();
	switch(info & FTYPE){
	case BMP: return pstring+".bmp";
	case JPG: return pstring+".jpg";
	case PNG: return pstring+".png";
	}
	return pstring;
}
std::string ImageSym::nameString(){
	return std::string(text);
}
std::string ImageSym::basePath(){
	return std::string(path);
}
std::string ImageSym::namePath(){
	return basePath()+nameString();
}
std::string ImageSym::parentDir(){
	std::string str=basePath();
	size_t pos = str.find_last_of(File.separator);
	str=str.substr(0,pos);
	pos = str.find_last_of(File.separator);
	std::string sstr=str.substr(pos+1, std::string::npos);
	return sstr;
}
std::string ImageSym::infoString(){
	char buff[MAXSTR];
	infoString(buff);
	return std::string(buff);
}
std::string ImageSym::toString(){
	char tmp[MAXSTR];
	sprintf(tmp,"%-20s %-30s %s",nameString().c_str(),infoString().c_str(),fullPath().c_str());
	return std::string(tmp);
}
//-------------------------------------------------------------
// ImageSym::print   print info
//-------------------------------------------------------------
void ImageSym::print()
{
    std::string tmp=toString();
    printf("%s\n",tmp.c_str());
}

//************************************************************
// Image class
//************************************************************
Image::~Image()
{
	FREE(data);
}
Image::Image()

{
	data=0;
}
Image::Image(RGBColor *b, int w, int h)
{
	data=(void *)b;
	opts.l=0;
	set_type(RGB_DATA);
	set_keep(1);
	height=h;
	width=w;
}
Image::Image(Color *b, int w, int h)
{
	data=(void *)b;
	opts.l=0;
	set_type(RGBA_DATA);
	set_keep(1);
	height=h;
	width=w;
}
Image::Image(float *b, int w, int h)
{
	data=(void *)b;
	opts.l=0;
	set_type(FLOAT_DATA);
	set_keep(1);
	height=h;
	width=w;
}

Image::Image(int opts, int h, int w, TNode *value, Image *grad)
{
	Color *colors=0;
	FColor *fcolors=0;
	FColor  fmax(-lim,-lim,-lim);
	FColor  fmin(lim,lim,lim);
	double rh=0.5,rw=0.5;
	
	Color *gcolors=0;
	int gsize=0;

	//Noise::resetStats();
	set_alpha_image(0);

	int lod=Adapt.lod();
	Adapt.set_lod(0);
	images.set_building(1);

	double hscale=Hscale;
	Hscale=0.025; // to normalize craters

	int cimage=0;
	int cnorm=0;
	int i,j;
	double x,y,dx,dy,x1=0,y1=0;
	double vx=0,vy=0,vz=0,vw=0;
	int aflag=0;
	double mx=-lim;
	double mn=lim;
	int norm=opts&NORM;
	int invt=opts&INVT;
	int gray=opts&GRAY;
	bool gradient=!gray && (grad!=0);
	//bool gradient=grad!=0;

	int achnl=((opts&ACHNL) && (opts&T1D))||(opts&BUMP);
	TNode *rvalue=0;
	TNode *gvalue=0;
	TNode *bvalue=0;
	TNode *avalue=0;

	//if(achnl)
	//	aflag=1;

	value->init();
	value->eval();
	if(S0.cvalid())
	    cimage=1;
	if(cimage && norm && value->typeValue()==ID_COLOR){
	    cnorm=1;
	    TNcolor &vc=*((TNcolor*)value);
	    rvalue=vc[0];
	    gvalue=vc[1];
	    bvalue=vc[2];
	    avalue=vc[3];
	    MALLOC(h*w,FColor,fcolors);
	}
	if(gradient){
		gsize=grad->size()-1;
		gcolors=(Color *)grad->data;
	}
	CALLOC(h*w,Color,colors);
	double b=0.5;
	TheNoise.set3D();
	TheNoise.offset=b;
	TheNoise.scale=rw;
	TheNoise.aspect=rw/rh;

	int mtype=opts & IMAP;
	//if(mtype==0)
	//    mtype=TILE;
	switch(mtype){
	default:
	case PMAP:        // planar mapping (decal)
		dy=2*PI*rh/h;
		dx=2*PI*rw/w;
	 	vz=1.0;
		break;
	case SMAP:       // spherical mapping
	    dx=360.0/w;
		dy=180.0/h;
		b=0;
		break;
	case CMAP:	      // cylindrical mapping (tiles in x)
		//TheNoise.set4D();
	    dx=360.0/w;
		dy=2*PI*rh/h;
		y1=-PI*rh;
		break;
	case TILE:	      // 4D mapping (tiles in x and y)
		TheNoise.set4D();
	    dx=360.0/w;
	    dy=360.0/h;
		break;
	}
	Point pt;

	for(y=y1,j=0;j<h;j++,y+=dy){
		switch(mtype){
		case TILE:
	        vy=rh*SIN(RPD*y);
	        vw=-rh*COS(RPD*y);
	        break;
		default:
		 	vy=y;
	        break;
	    }

		//TheNoise.phi=y-180;
		TheNoise.phi=y;
		//Phi=y/360-0.5;
		Phi=0.5*(y-180);

		for(x=x1,i=0;i<w;i++,x+=dx){
			switch(mtype){
			case SMAP:
				pt=Point(x,y+90,rh);
				pt=pt.rectangular();
				vx=pt.x;
				vy=pt.y;
				vz=pt.z;
				break;
			case CMAP:
			case TILE:
				vx=-rw*COS(RPD*x);
				vz=rw*SIN(RPD*x);
				break;
			case PMAP:
			default:
			    vx=x;
				break;
			}
			TheNoise.theta=x;
			Theta=x;
			TheNoise.set(vx+b,vy+b,vz+b,vw+b);
			INIT;
			CurrentScope->revaluate();
			if(cimage){
			    double a=1.0;
			    if(cnorm){
			        FColor fc;
			        rvalue->eval();
			        fc.set_red(S0.s);
			        Red=S0.s;
			        gvalue->eval();
			        fc.set_green(S0.s);
			        Green=S0.s;
			        bvalue->eval();
			        fc.set_blue(S0.s);
			        Blue=S0.s;
					if(gray)
						fc=fc.grayscale();

			        if(avalue){
			       	 	avalue->eval();
			        	fc.set_alpha(S0.s);
			        	a=S0.s;
						if(S0.s!=1)
					    	aflag=1;
			        }
			    	fc.set_alpha(a);
			        fcolors[j*w+i]=fc;
			        fmax.cmax(fc);
			        fmin.cmin(fc);
			    }
			    else{
					value->eval();
					a=S0.c.alpha();
					if(a!=1){
				    	aflag=1;
					}
					if(gray)
				   	 	S0.c=S0.c.grayscale();
			    	if(invt)
			        	S0.c=S0.c.complement();
			    	S0.c.set_alpha(a);
					colors[j*w+i]=S0.c;
				}
			}
			else{
				value->eval();
				double f;
				if(S0.s==RANDID)
					f=Rand();
				else
					f=S0.s;
				if(invt)
				    f=1-f;
				if(norm){
					colors[j*w+i].cd.f=(float)f;
					mn=f<mn?f:mn;
					mx=f>mx?f:mx;
				}
				else{
					if(gradient){
						int index=f*gsize;
						double g=f*gsize-index;
						if(index<gsize)
							colors[j*w+i]=gcolors[index].blend(gcolors[index+1],g);
						else
							colors[j*w+i]=gcolors[index];
					}
					else{
						if(achnl)
							colors[j*w+i]=Color(f,f,f,f);
						else
							colors[j*w+i]=Color(f,f,f);
					}
				}
			}
		}
	}
	if(cimage && cnorm){
	    //fmax.print();
	    //fmin.print();
		// scale all colors to fit between 0 and 1
	    double max=fmax.maxcomp(); // max color component
	    double min=fmin.mincomp(); // min color component
	    fmax=FColor(max,max,max);
	    fmin=FColor(min,min,min);
	    fmax.rescale(fmin);
	    //fmax.print();
		for(j=0;j<h;j++){
			for(i=0;i<w;i++){
			    int n=j*w+i;
			    double a=0.0;
			    if(aflag)
			        a=fcolors[n].alpha();
			    FColor fc=fcolors[n]*fmax-fmin;
			    Color c=Color(fc.red(),fc.green(),fc.blue());
				//if(gray)
				//   	c=c.grayscale();
			    if(invt)
			        c=c.complement();
			    c.set_alpha(a);
			    colors[n]=c;
			}
		}
		DFREE(fcolors);
	}
	else if(!cimage && norm){
	    double ma=1;
	    double mb=0;
	    //printf("min %g max %g\n",mx,mn);
	    if(mx != mn){
			ma=1/(mx-mn);
			mb=mn*ma;
		}
	 	for(j=0;j<h;j++){
			for(i=0;i<w;i++){
				double v=colors[j*w+i].cd.f;
				double f=ma*v-mb;
				f=clamp(f,0,1);
				if(gradient){
					int index=f*gsize;
					double g=f*gsize-index;
					if(index<gsize)
						colors[j*w+i]=gcolors[index].blend(gcolors[index+1],g);
					else
						colors[j*w+i]=gcolors[index];
				}
				else{
					if(achnl)
						colors[j*w+i]=Color(f,f,f,f);
					else
						colors[j*w+i]=Color(f,f,f);
				}
			}
		}
	}
	//Noise::showStats();
	TheNoise.set3D();
	TheNoise.offset=0.5;
	TheNoise.scale=0.5;
	TheNoise.aspect=1;

    height=h;
    width=w;
	set_alpha(aflag);
	data=(void *)colors;
	set_type(RGBA_DATA);
	set_accessed(0);
	set_keep(1);
	set_global(1);
	Hscale=hscale;
	Adapt.set_lod(lod);
	images.set_building(0);
}

Color Image::aveColor(){
	GLubyte      *c=(GLubyte*)data;
	FColor ave=FColor(0,0,0);
	int dim=comps();
	for (int i = 0; i < height; i++){
		for (int j = 0; j< width ; j++) {
		    int index=i*width*dim+j*dim;
			FColor fc=FColor(c[index],c[index+1],c[index+2]);
			ave=ave+fc;
		}
	}
	ave=ave/size()/255.0;
	return Color(ave.red(),ave.green(),ave.blue());
}
//-------------------------------------------------------------
// Image::addAlphaToImage()    get alpha from aux. RGB image
//-------------------------------------------------------------
void Image::addAlphaToImage(GLubyte *a)
{
	Color      *c=(Color*)data;
    RGBColor   *rgb=(RGBColor*)a;

    set_alpha_image(1);
    set_alpha(1);

	for (int i = 0; i < height; i++){
		for (int j = 0; j< width ; j++) {
		    int index=i*width+j;
		    double aval=rgb[index].intensity();
			c[index].set_alpha(aval);
			//c[index].print();

		}
	}
}

//-------------------------------------------------------------
// Image::clone()      make deep copy
//-------------------------------------------------------------
Image *Image::clone()
{
	Image *im=new Image();
	int i,n=width*height;
	*im=*this;
	switch(type()){
	case RGB_DATA:
		RGBColor *r1,*r2;
		MALLOC(n,RGBColor,r2);
		r1=(RGBColor*)data;
		r2=(RGBColor*)im->data;
		for(i=0;i<n;i++)
			r2[i]=r1[i];
		break;
	case RGBA_DATA:
		Color *c1,*c2;
		MALLOC(n,Color,c2);
		c1=(Color*)data;
		c2=(Color*)im->data;
		for(i=0;i<n;i++)
			c2[i]=c1[i];
		break;
	case FLOAT_DATA:
		float *f1,*f2;
		MALLOC(n,float,f2);
		f1=(float*)data;
		f2=(float*)im->data;
		for(i=0;i<n;i++)
			f2[i]=f1[i];
		break;
	}
	return im;
}

//-------------------------------------------------------------
// Image::color()      return interpolated color (1d)
//-------------------------------------------------------------
FColor Image::color(int opts, double s)
{
    s=fmod(s,1);
    double col=s*width;
    int n1=floor(col);
    double f=col-floor(col);
    int n2=n1+1;
    FColor col1=p2c(n1);
    FColor col2=p2c(n2);
    return col1.mix(col2,f);
}


//-------------------------------------------------------------
// Image::color()      get nearest pixel Color
//-------------------------------------------------------------
double Image::p2v(int indx){
	if(indx<0)
		return 0;
 	if(type()==RGB_DATA && comps()==3){
 		RGBColor c=*((RGBColor*)data+indx);
 		return c.intensity();
	}
 	if(type()==RGBA_DATA && comps()==4){
 		Color c=*((Color*)data+indx);
 		return c.intensity();
 	}
 	return 0.0;

}

//-------------------------------------------------------------
// Image::color()      get pixel Color
//-------------------------------------------------------------
Color Image::p2c(int indx){
 	if(type()==RGB_DATA && comps()==3){
 		RGBColor c=*((RGBColor*)data+indx);
 		Color col=Color(c.red(),c.green(),c.blue(),1);
 		return col;
	}
 	if(type()==RGBA_DATA && comps()==4){
 		Color c=*((Color*)data+indx);
 		Color col=Color(c.red(),c.green(),c.blue(),1);
 		return col;
 	}
 	return Color();
 }


//-------------------------------------------------------------
// Image::color()      get nearest pixel amplitude
//-------------------------------------------------------------
double Image::p2v(double x, double y){
    double r=floor(y*height);
    double c=floor(x*width);
  	c=c>=width?c-width:c;
	r=r>=height?r-height:r;
	c=c<0?width+c:c;
	r=r<0?height+r:r;

    double indx=r*width+c;
	return p2v(indx);
}

//-------------------------------------------------------------
// Image::color()      get nearest pixel Color
//-------------------------------------------------------------
Color  Image::p2c(double x, double y){
    double r=floor(y*height);
    double c=floor(x*width);
  	c=c>=width?c-width:c;
	r=r>=height?r-height:r;
	c=c<0?width+c:c;
	r=r<0?height+r:r;

    double indx=r*width+c;
	return p2c(indx);
}

//-------------------------------------------------------------
// Image::color() return interpolated image color (2d)
// 1) using Color causes step like artifacts for some reason
//    - difference in blend/mix functions ?
// 2) result is flat-sided because of linear interpolation in box only
//   - might be able to improve using 3 or 4 point smoothing function
//     from adjacent box pixels
//-------------------------------------------------------------
double Image::value(int opts, double x, double y)
{
	x=fmod(x,1.0);
	y=fmod(y,1.0);

    if(opts==GL_NEAREST)
    	return p2v(x,y);

    // linear interpolation

	double dw=1.0/width,dh=1.0/height; //  pixel step
	double row=y*height;
	double col=x*width;
	double f1=col-floor(col);
	double f2=row-floor(row);

	if(opts==GL_LINEAR)
		return blerp(p2v(x,y),p2v(x+dw,y),p2v(x,y+dh),p2v(x+dw,y+dh),f1,f2);

	// bi-cubic interpolation (3rd order polynomial)

#define pval(i,j) p2v(x+dw*i,y+dh*j)
//#define FORWARD
#ifdef FORWARD // interpolate using only forward pixels
	double x1 = cubic(pval(0,0), pval(1,0), pval(2,0), pval(3,0), f1 );
	double x2 = cubic(pval(0,1), pval(1,1), pval(2,1), pval(3,1), f1 );
	double x3 = cubic(pval(0,2), pval(1,2), pval(2,2), pval(3,2), f1 );
	double x4 = cubic(pval(0,3), pval(1,3), pval(2,3), pval(3,3), f1 );
#else // interpolate using backwards pixels
	double x1 = cubic(pval(-1,-1), pval(0,-1), pval(1,-1), pval(2,-1), f1 );
	double x2 = cubic(pval(-1,0), pval(0,0), pval(1,0), pval(2,0), f1 );
	double x3 = cubic(pval(-1,1), pval(0,1), pval(1,1), pval(2,1), f1 );
	double x4 = cubic(pval(-1,2), pval(0,2), pval(1,2), pval(2,2), f1 );
#endif
	double y1 = cubic( x1, x2, x3, x4, f2 );
	return y1;
}

double Image::cubic(double v0, double v1, double v2, double v3, double frac){
	double A = 0.5 * (v3 - v0) + 1.5 * (v1 - v2);
	double B = 0.5 * (v0 + v2) - v1 - A;
	double C = 0.5 * (v2 - v0);
	double D = v1;
    return ((A * frac + B) * frac + C) * frac + D;
}

//-------------------------------------------------------------
// Image::color() return interpolated image color (2d)
// 1) using Color causes step like artifacts for some reason
//    - difference in blend/mix functions ?
// 2) result is flat-sided because of linear interpolation in box only
//   - might be able to improve using 3 or 4 point smoothing function
//     from adjacent box pixels
//-------------------------------------------------------------
FColor Image::color(int opts, double x, double y)
{
	x=fmod(x,1.0);
	y=fmod(y,1.0);

	double dw=1.0/width,dh=1.0/height; //  pixel step
	double row=y*height;
	double col=x*width;
	double f1=col-floor(col);
	double f2=row-floor(row);

    if(opts==GL_NEAREST)
    	return p2c(x,y);

	FColor col1=p2c(x,y);
	FColor col2=p2c(x+dw,y);
	FColor col3=p2c(x,y+dh);
	FColor col4=p2c(x+dw,y+dh);

	FColor mix1=col1.mix(col2,f1); // base row midpoint
	FColor mix2=col3.mix(col4,f1); // previous row midpoint
	FColor mix3=mix1.mix(mix2,f2);

	return mix3;
}

//************************************************************
// ImageReader class
//************************************************************
ImageReader::ImageReader()
{
    flags=NEED_REBUILD;

}
ImageReader::~ImageReader()
{
	images.free();
}

//-------------------------------------------------------------
// ImageReader::free   free image buffers
//-------------------------------------------------------------
void ImageReader::free()
{
//	images.free();
}

//-------------------------------------------------------------
// ImageReader::printImageInfo   print file info
//-------------------------------------------------------------
void ImageReader::printImageInfo(int info)
{
	LinkedList<ImageSym*> list;

	getImageInfo(info,list);
	list.ss();
	ImageSym *is;
	while((is=list++))
	    is->print();
    list.free();
}

//-------------------------------------------------------------
// ImageReader::clear_flags   reset flags
//-------------------------------------------------------------
void ImageReader::clear_flags()
{
	ImageSym *is;
	for(int i=0;i<images.size;i++){
		is=images[i];
		if(is->image){
			is->image->set_accessed(0);
			is->image->set_newimg(0);
		}
	}
}

//-------------------------------------------------------------
// ImageReader::getFileInfo   get file info
//-------------------------------------------------------------
uint ImageReader::getImageInfo(char *name, char *dir)
{
    char path[512];
    uint info=0;

	sprintf(path,"%s%s.spx",dir,name);
	if(File.fileExists(path))
		info|=SPX;

	sprintf(path,"%s%s.bmp",dir,name);
	if(File.fileExists(path))
		info|=BMP;

	sprintf(path,"%s%s.jpg",dir,name);
	if(File.fileExists(path))
		info|=JPG;
	
	sprintf(path,"%s%s.png",dir,name);
	if(File.fileExists(path))
		info|=PNG;
	return info;
}

void ImageReader::getImageDims(uint info, uint &cols,uint &rows){
	cols=info&IMCOLS;
	rows=(info&IMROWS)>>4;
}
void ImageReader::setImageDims(uint &info, uint cols,uint rows){
	MSK_SET(info,IMCOLS,cols);
	MSK_SET(info,IMROWS,rows<<4);
}

void ImageReader::setImageDims(uint &info, char *s){
	uint rows=0;
	uint cols=0;
	sscanf(s,"%dx%d",&cols,&rows);
	setImageDims(info,cols,rows);
}
void ImageReader::getImageDims(uint info, char *s){
	uint rows=0;
	uint cols=0;
	getImageDims(info, cols,rows);
	char tmp[32];
	sprintf(s,"%dx%d",cols,rows);
}

//-------------------------------------------------------------
// ImageReader::getFileInfo   get file info
//-------------------------------------------------------------
uint ImageReader::getTiledImageInfo(char *name, char *dir)
{
	ValueList<FileData*> image_dirs;	
	File.getDirectoryList(dir,image_dirs);
	char sdir[256];  // subdir
	char path[512];
	
	uint info=0;
	for(int i=0;i<image_dirs.size;i++){
		strcpy(sdir,image_dirs[i]->name());
		sprintf(path,"%s%s%s",dir,image_dirs[i]->name(),File.separator);
		info=getImageInfo(name,path); // type
		if(info){
			setImageDims(info,sdir);
			sprintf(dir,"%s",path);
			break;
		}
	}
	return info;

}
//-------------------------------------------------------------
// ImageReader::getFileInfo   get file info
//-------------------------------------------------------------
uint ImageReader::getFileInfo(char *name, char *dir)
{
    uint info=0;
//	cout <<name<<endl;

  	File.getBitmapsDir(dir);
   	info=getImageInfo(name,dir);
   	if(info)
   		return info;

   	File.getImportsDir(dir);
	info=getImageInfo(name,dir);
   	if(info){
   		info |=IMPORT;
   		return info;
   	}
	File.getMapsDir(dir);
	info=getImageInfo(name,dir);
	if(info){
		info|=MAP;
		return info;
	}
	File.getHmapsDir(dir);
	info=getImageInfo(name,dir);
	if(info){
		info|=HTMAP;
		return info;
	}
	File.getSpritesDir(dir);
	info=getTiledImageInfo(name,dir);
   	if(info){
   		info |=SPRITE;
   		return info;
   	}
   	File.getBranchesDir(dir);
	info=getTiledImageInfo(name,dir);
   	if(info){
   		info |=BRANCH;
   		return info;
   	}
   	File.getLeavesDir(dir);
	info=getTiledImageInfo(name,dir);
   	if(info){
   		info |=LEAF;
   		return info;
   	}
   	return 0;

}

uint ImageReader::getFileInfo(char *name){
	char dir[512];
	dir[0]=0;
	uint info=getFileInfo(name, dir);
	return info;
}

ImageSym *ImageReader::imageInfo(char *name){
	ImageSym *is=images.inlist(name);
	return is;
}
//-------------------------------------------------------------
// ImageReader::getImageInfo   get info for saved image
//-------------------------------------------------------------
ImageSym *ImageReader::getImageInfo(char *name)
{
	char dir[MAXSTR];
	dir[0]=0;
    char *spx=0;
	std::string tmp(name);
	size_t pos=tmp.find("_alpha");
	if(pos !=std::string::npos){
		return 0;
	}
	uint info=getFileInfo(name,dir);
	if(info & SPX){
		spx=readSpxPath(name,dir);
		if(spx){
		    info |=SPX;
	    	TNinode *n=(TNinode*)TheScene->parse_node(spx);
	   		if(n){
	   		     info |=n->itype();
	   		     if(n->tmp())
	   		    	info|=TMP;
	   		     if(n->t1d())
	   		         info|=T1D;
	   		     else
	   		         info|=T2D;
	   		     if(n->hmapActive())
	   		    	info|=HMAP;
	   		    	 
	   		     info|=TEX;
				// if(n->alpha())
				//	info|=BMPA;
	   		     delete n;
	        }
	    }
	}
	ImageSym *is=new ImageSym(info,name,0,spx,dir);
	FREE(spx);
	return is;
}

//-------------------------------------------------------------
// ImageReader::getImages  return list of images of specified type
//-------------------------------------------------------------
void ImageReader::getImageInfo(int mode, LinkedList<ImageSym*> &list)
{
	ImageSym *is;
	bool test=false;
	
	for(int i=0;i<images.size;i++){
		is=images[i];
	    uint info=is->info;
	    if((mode&IMTYPE) != (info &  IMTYPE))
	    	continue;

		switch(mode&IMTYPE){
		case SPX:
			{
				if(!(info & SPX))
					continue;
				if(mode & TMP){
					if(!(info & TMP))
						continue;
					break;
				}
				switch(mode&SPXTYPE){
				case IMAGE:
					if((info&SPXTYPE) != IMAGE)
						continue;
					break;
				case BANDS:
					if((info&SPXTYPE) != BANDS)
						continue;
					break;
				}
			    int dmode=mode&(T1D|T2D);
				if(dmode && dmode != (T1D|T2D)){
					if((mode & T1D) && !(info & T1D))
						continue;
					else if((mode & T2D) && !(info & T2D))
						continue;
				}
				if((mode & HMAP) && !(info & HMAP))
					continue;
			}
			break;
		case LEAF:
		case SPRITE:
		case BRANCH:
			if((mode & IMDIMS) && ((mode & IMDIMS) != (info & IMDIMS)))
			 	continue;
			break;
		}
		ImageSym *nis=new ImageSym(is);
		list.add(nis);
	}
}


//-------------------------------------------------------------
// Scene::delete_tmpfiles() tmp files
//-------------------------------------------------------------
//void ImageReader::check_tmpfiles()
//{
//	LinkedList<ImageSym *> list;
//	images.getImageInfo(TMP|SPX, list);
//    tmp_files=list.size;
//    list.free();    	
//}

//-------------------------------------------------------------
// ImageReader::removeAll      remove an image + all image files
//-------------------------------------------------------------
void ImageReader::removeAll(char *name)
{
 	ImageSym *is=images.inlist(name);
	if(is)
		images.remove(is); // deletes is
	char path[256];
 	getImagePath(name,path);
   	remove(path);
}

//-------------------------------------------------------------
// ImageReader::remove      remove an image
//-------------------------------------------------------------
void ImageReader::remove(char *fn)
{
   	char path[256];
 	sprintf(path,"%s%s",fn,File.ext);
	File.deleteFile(path);
 	sprintf(path,"%s.bmp",fn);
	File.deleteFile(path);
 	sprintf(path,"%s_alpha.bmp",fn);
	File.deleteFile(path);
 	sprintf(path,"%s_bump.bmp",fn);
	File.deleteFile(path);
 	sprintf(path,"%s.jpg",fn);
	File.deleteFile(path);
 	sprintf(path,"%s.jpeg",fn);
	File.deleteFile(path);
 	sprintf(path,"%s_alpha.jpg",fn);
	File.deleteFile(path);
 	sprintf(path,"%s.png",fn);
	File.deleteFile(path);

}

//-------------------------------------------------------------
// ImageReader::find      find  an image
//-------------------------------------------------------------
Image *ImageReader::find(char *f)
{
	if(f && strlen(f)){
		ImageSym *is=images.inlist(f);
		if(is && is->image)
			return 	is->image;
	}
	return 0;
}

//-------------------------------------------------------------
// ImageReader::imagedir get imagedir
//-------------------------------------------------------------
void ImageReader::imagedir(char *dir)
{
	char base[MAXSTR];
  	File.getBaseDirectory(base);
	File.makeSubDirectory(base,File.bitmaps,dir);
}

//-------------------------------------------------------------
// ImageReader::makeImagelist make image list
//-------------------------------------------------------------
void ImageReader::addImages(char *dir){
	NameList<ModelSym*>flist;
	char sdir[MAXSTR];
	ModelSym* sym;
	ImageSym *is;
	
	File.getFileNameList(dir,"*.bmp",flist);
	File.getFileNameList(dir,"*.jpg",flist);
	File.getFileNameList(dir,"*.png",flist);

	while((sym=flist++)){
		if(!images.inlist(sym->name())){
			is=getImageInfo(sym->name());
			if(is){
				images.add(is);
				//images.sort();
			}
		}
	}
	images.sort();
	flist.free();
}

//-------------------------------------------------------------
// ImageReader::makeImagelist make image list
//-------------------------------------------------------------
void ImageReader::addTiledImages(char *dir){
	ValueList<FileData*> image_dirs;
	NameList<ModelSym*>flist;
	ModelSym* sym;
	ImageSym *is;

	char path[MAXSTR];
	char sdir[256];

	uint rows=0;
	uint cols=0;

	File.getDirectoryList(dir,image_dirs);
	for(int i=0;i<image_dirs.size;i++){
		strcpy(sdir,image_dirs[i]->name());
		sprintf(path,"%s%s%s",dir,sdir,File.separator);
		File.getFileNameList(path,"*.bmp",flist);
		File.getFileNameList(path,"*.jpg",flist);
		File.getFileNameList(path,"*.png",flist);
		while((sym=flist++)){
			char *name=sym->name();
			if(!images.inlist(name)){
				is=getImageInfo(name);
				getImageDims(sdir,cols,rows);
				setImageDims(is->info,cols,rows);
				images.add(is);
			}	
		}
		images.sort();
		flist.free();
	}
}

//-------------------------------------------------------------
// ImageReader::makeImagelist make image list
//-------------------------------------------------------------
void ImageReader::makeImagelist()
{
    if(invalid()){
    	images.free();
    	static TimeIt timer;
    	timer.start();

		char sdir[MAXSTR];

		File.getSpritesDir(sdir);
	  	addTiledImages(sdir);

	  	File.getBranchesDir(sdir);
	  	addTiledImages(sdir);

	  	File.getLeavesDir(sdir);
	  	addTiledImages(sdir); 	

	  	File.getBitmapsDir(sdir);
	  	addImages(sdir);

	  	File.getImportsDir(sdir);
	  	addImages(sdir);
	  	
	  	File.getMapsDir(sdir);
	  	addImages(sdir);

	  	File.getHmapsDir(sdir);
	  	addImages(sdir);

		//images.sort();
	  	
		timer.showTime("makeImagelist");
		cout<<"number of images="<<images.size<<endl;
#ifdef DEBUG_IMAGE_INFO
		showImageInfo();
#endif
		validate();
	}
}

//-------------------------------------------------------------
// ImageReader::getImagePath   return path to file (sans ext)
//-------------------------------------------------------------
void ImageReader::getImagePath(char *name,char *dir)
{
	imagedir(dir);
   	strcat(dir,File.separator);
   	strcat(dir,name);
}

//-------------------------------------------------------------
// ImageReader::load   read an image from a file (TNimage.init)
//-------------------------------------------------------------
Image *ImageReader::load(char *f,TNinode *n)
{
    int info=n->opts;
    char name[256];
    strcpy(name,f);
    ImageSym *is=getImageInfo(name);
	char buff[4096];
	buff[0]=0;

    // see if spx string has changed
	if(TheScene->keep_variables()){
		TheScene->set_keep_variables(0);
		n->valueString(buff);
		TheScene->set_keep_variables(1);
	}
	else
		n->valueString(buff);

	strcat(buff,"\n");

	if(is->istring && strcmp(is->istring,buff)!=0){
#ifdef DEBUG_IMAGES
	printf("%-20s SPX CHANGED - REBUILD NEEDED\n %s %s \n",name,buff,is->istring);
#endif
		delete is;
		return 0;      // changed: builder needs to rebuild
	}
    
	Image *image=open(name);
	addImage(name,is->info,buff,image,is->path);

	delete is;
	return image;
}

//-------------------------------------------------------------
// ImageReader::load      read an image from a file
//-------------------------------------------------------------
Image *ImageReader::load(char *f,int mode)
{
    char name[512];
    //copyName(f,name);
    strcpy(name,f);

    Image *image=find(name);
    if(image)
		return image;
    image=open(name);
    if(image){
		ImageSym *is=getImageInfo(name);
		if(is){
			addImage(is->text,is->info,is->istring,image,is->path);	
			delete is;
		}
    }
    return image;
}

//-------------------------------------------------------------
// ImageReader::save      save an image to a file
//-------------------------------------------------------------
void ImageReader::save(char *f, Image *image, TNinode *n)
{
    char buff[4096];
    buff[0]=0;
	if(TheScene->keep_variables()){
		TheScene->set_keep_variables(0);
		n->valueString(buff);
		TheScene->set_keep_variables(1);
	}
	else
		n->valueString(buff);
    int info=n->opts;
    char name[256];
    strcpy(name,f);
	saveSpxFile(name,buff);
    save(name,image);
    ImageSym *is=ImageReader::getImageInfo(name);
    if(is)
    	addImage(name,info,buff,image,is->path);
}

//-------------------------------------------------------------
// ImageReader::addImage      add image to image list
//-------------------------------------------------------------
void ImageReader::addImage(char *name, int m, char *vstr, Image *im, char *p)
{
    ImageSym *is=images.inlist(name);
	if(is){
		DFREE(is->image);
		is->info=m;
		is->image=im;
		is->setIstring(vstr);
		is->setName(name);
		is->setPath(p);
	}
	else{
		is=new ImageSym(m,name,im,vstr,p);
		images.add(is);
		//images.sort();
	}
}

//-------------------------------------------------------------
// ImageReader::saveSpxFile      save an spx file
//-------------------------------------------------------------
int ImageReader::saveSpxFile(char *name, char *buff)
{
	char path[256];

   	getImagePath(name,path);

   	strcat(path,File.ext);

	FILE *fp=fopen(path,"wb");
	if(!fp)
	    return 0;
 	fprintf(fp,"%s\n",buff);
	fclose(fp);
    return 1;
}

//-------------------------------------------------------------
// ImageReader::readSpxFile      read an spx file
//-------------------------------------------------------------
char *ImageReader::readSpxPath(char *name, char *dir)
{
	char path[512];
	strcpy(path,dir);
   	strcat(path,name);
    return readSpxFile(name, path);
}
//-------------------------------------------------------------
// ImageReader::readSpxFile      read an spx file
//-------------------------------------------------------------
char *ImageReader::readSpxFile(char *name)
{
	char path[512];
	getImagePath(name,path);
    return readSpxFile(name, path);
}
//-------------------------------------------------------------
// ImageReader::readSpxFile      read an spx file
//-------------------------------------------------------------
char *ImageReader::readSpxFile(char *name, char *path)
{
//	char path[256];
//	getImagePath(name,path);
	FILE *fp=0;

	char *buff=0;

	strcat(path,File.ext);
	fp=fopen(path,"rb");
	if(!fp)
		return 0;
	fseek( fp, 0, SEEK_END);
	int size=ftell( fp);
	fseek( fp, 0, SEEK_SET);
	MALLOC(size+1,char,buff);
	int i=0;
	char c;
	while((c=fgetc(fp))!=EOF)
		buff[i++]=c;
	buff[i]=0;

	fclose(fp);
	return buff;
}

//-------------------------------------------------------------
// ImageReader::open read an image from a file
//-------------------------------------------------------------
//#define TIME_OPEN
Image *ImageReader::open(char *name)
{
	char dir[512];
	Image *im=0;
	dir[0]=0;
#ifdef TIME_OPEN
	static TimeIt timer;
	timer.start();
#endif
	ImageSym *is=imageInfo(name);
	if(is){
		strcat(dir,is->namePath().c_str());
		im=open(name, dir);	
#ifdef TIME_OPEN
	timer.showTime("open");
#endif
	}
//	else
//		cout<<"Image not Found "<<name<<endl;
//	if(!im)
//		cout<<"Image not opened "<<name<<endl;
		
	return im;
}

//-------------------------------------------------------------
// ImageReader::open read an image from a file
//-------------------------------------------------------------
Image *ImageReader::open(char *name, char *path)
{
	Image *image=openBmpFile(name,path);
	if(image)
		return image;
	image=openJpgFile(name,path);
	if(image)
		return image;
	image=openPngFile(name,path);
	if(image)
		return image;
	return 0;
}

//-------------------------------------------------------------
// ImageReader::openPngFile read an image from a file
//-------------------------------------------------------------
Image *ImageReader::openPngFile(char *name,char *path)
{
	Image *image=0;
	char cpath[256];

 	sprintf(cpath,"%s.png",path);

 	FILE *fp=fopen(cpath, "rb");
	if (fp == NULL){
		return 0;
	}

	fclose(fp);

	int width=0;
	int height=0;
	int comps=0;
	GLubyte *pxls=readPngFile(cpath,width,height,comps);
	//cout<<"reading png image "<<path<<" comps="<<comps<<endl;
    if(pxls){
		image=new Image((Color*)pxls,width,height);
		image->set_global(1);
		if(comps==4)
			image->set_alpha_image(1);
		else
			image->set_alpha_image(0);
    }
	return image;
}
//-------------------------------------------------------------
// ImageReader::openJpgFile read an image from a file
//-------------------------------------------------------------
Image *ImageReader::openJpgFile(char *name,char *path)
{
	Image *image=0;
	char cpath[256];

 	sprintf(cpath,"%s.jpg",path);

 	FILE *fp=fopen(cpath, "rb");
	if (fp == NULL){
		sprintf(cpath,"%s.jpeg",path);
		fp=fopen(cpath, "rb");
		if (fp == NULL){
		    return 0;
		}
	}

	fclose(fp);

	int width=0;
	int height=0;
	int comps=0;
	GLubyte *pxls=readJpegFile(cpath,width,height,comps);
    if(pxls){
		image=new Image((Color*)pxls,width,height);
		image->set_global(1);
		if(comps==4)
			image->set_alpha_image(1);
		else
			image->set_alpha_image(0);
    }  
	return image;	
}

void ImageReader::showImageInfo(){
 	for(int i=0;i<images.size;i++){
		cout << images[i]->toString().c_str()<<endl;
	}
}
//-------------------------------------------------------------
// ImageReader::printImageInfo   print file info
//-------------------------------------------------------------
void ImageReader::printImageInfo(char *f)
{
    ImageSym *is=getImageInfo(f);
    is->print();
    delete is;
}

//-------------------------------------------------------------
// ImageReader::openBmpFile read an image from a file
//-------------------------------------------------------------
Image *ImageReader::openBmpFile(char *name,char *path)
{
	//BITMAPINFO	*info=0;

	void *bits;
	void *pxls;
	Image *image=0;
	int aflag=1;
	int comps=0;
	int w=0;
	int h=0;

	char cpath[256];
	char apath[256];

 	sprintf(cpath,"%s.bmp",path);

 	FILE *fp=fopen(cpath, "rb");
	if (fp == 0){
	    return 0;
	}
	fclose(fp);
#ifdef DEBUG_IMAGES
	printf("%-20s READING IMAGE %s\n","ImageReader",name);
#endif

 	sprintf(apath,"%s_alpha.bmp",path);

	fp=fopen(apath, "rb");
	if (fp == NULL)
	    aflag=0;
	else
		fclose(fp);

	pxls=readBmpFile(cpath,w,h,comps);

	if(!pxls){
		printf("Couldn't create image for %s\n",name);
    	return 0;
	}
	if(!aflag){
		bits=ColorToRGB(w,h,(unsigned char*)pxls);
		image=new Image((RGBColor*)bits,w,h);
		image->set_global(1);
		image->set_alpha_image(0);
		FREE(pxls);
		return image;
	}
	image=new Image((Color*)pxls,w,h);
	image->set_global(1);
	image->set_alpha_image(1);

	bits=readBmpFile(apath,w,h,comps);
	if(bits){
		Color *a=(Color*)(bits);
		Color *c=(Color*)(pxls);
		for(int i=0;i<h*w;i++){
			double aval=a[i].intensity();
			c[i].set_alpha(aval);
		}
		FREE(bits);
		image->set_alpha(1);
	}
	return image;
}

//-------------------------------------------------------------
// ImageReader::save      save an image to a file
//-------------------------------------------------------------
void ImageReader::save(char *f, Image *image)
{
	char path[256];

#ifdef DEBUG_IMAGES
	printf("%-20s SAVING IMAGE %s %s\n","ImageReader",image->alpha()?"RGBA":"RGB",f);
#endif

	getImagePath(f,path);
   	strcat(path,".bmp");
   	
   	int h=image->height;
   	int w=image->width;
   	
   	FILE *fp=fopen(path, "wb");
   	if (fp == NULL)
   		return;
	void *pxls=ColorToRGB(w,h,(unsigned char*)image->data);
	if(!writeBmpFile(w, h,pxls, path,false)){
		cout<<"failed to save bmp file:"<<path<<endl;
		return;
	}
	getImagePath(f,path);
	strcat(path,"_alpha.bmp");

	if(image->alpha()){
		pxls=AlphaToRGB(w,h,(unsigned char*)image->data);
		writeBmpFile(w, h,pxls, path,false);
	}
	else
	   File.deleteFile(path);
}

void ImageReader::getImageDims(char *s,uint &cols,uint &rows){
	int i=0,j=0;
	sscanf(s,"%dx%d",&i,&j);
	rows=j;
	cols=i;
}

void ImageInfo::setImage(char *name){
	if(strcmp(name,image_file)){
		setImageName(name);
		char path[512];
		if(getImageFilePath(image_file,path)){
			if(image)
				delete image;
			image=images.open(image_file,path);
		}
	}
}

bool ImageInfo::getImageFilePath(char *name,char *dir){
	if(image_mgr==0)
		return false;
	image_rows=1;
	image_cols=1;

	ImageSym *is=images.getImageInfo(name);
	strcpy(dir,is->namePath().c_str());
	if(is->info & TILED){
		std::string str=is->parentDir();
		char *pd=str.c_str();
		strcpy(image_dir,pd);
		ImageReader::getImageDims(pd,image_cols,image_rows);
	}
//	else if((is->info & IMTYPE)==IMPORT)
//		strcpy(image_dir,ImageMgr::Istr);
//	else if((is->info & IMTYPE)==SPX)
//		strcpy(image_dir,ImageMgr::Bstr);
	return true;

}
