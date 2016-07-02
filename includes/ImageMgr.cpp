// ImageMgr.cpp

#include "ImageMgr.h"
#include "Bitmap.h"
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

ImageReader images; // global image manager

extern double Rand();
extern double Hscale;
extern double Red,Green,Blue,Alpha,Theta,Phi;

#define DEBUG_IMAGES

int icnt1=0;
int icnt2=0;

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
	text=istring=0;
	setName(t);
}

ImageSym::ImageSym(char *t, int i)
{
	image=0;
	text=istring=0;
	setName(t);
	info=i;
}

ImageSym::ImageSym(int m, char *t, Image *i, char *s)
{
	image=i;
	info=m;
	text=istring=0;
	setName(t);
	setIstring(s);
}

ImageSym::ImageSym(ImageSym *is)
{
	image=0;
	text=istring=0;
	info=is->info;
	setName(is->text);
	setIstring(is->istring);
}

ImageSym::~ImageSym()
{
	FREE(text);
	FREE(istring);
    DFREE(image);
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

//-------------------------------------------------------------
// ImageSym::print   print info
//-------------------------------------------------------------
void ImageSym::print()
{
    char tmp[256];
    int m=0;
    images.unhashName(name(),m,tmp);
    printf("%-10s",tmp);
    if(info&SPX){
        printf("%-5s","SPX");
        if(info&BANDS)
        	printf("%-6s","BANDS");
    	else if(info&IMAGE)
        	printf("%-6s","IMAGE");
   	 	if(info&T1D)
        	printf("%-5s","1D");
    	else
        	printf("%-5s","2D");
    	if((m&BUMP) || (info&BUMP))
        	printf("%-5s","BUMP");
    }
    if(info&BMP)
        printf("%-5s","BMP");
    else if(info&JPG)
        printf("%-5s","JPG");
    printf("\n");
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
Image::Image(int opts, int h, int w, double rh, double rw, TNode *value)
{
	Color *colors=0;
	FColor *fcolors=0;
	FColor  fmax(-lim,-lim,-lim);
	FColor  fmin(lim,lim,lim);

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
	int achnl=(opts&ACHNL)||(opts&BUMP);
	TNode *rvalue=0;
	TNode *gvalue=0;
	TNode *bvalue=0;
	TNode *avalue=0;

	if(achnl)
		aflag=1;

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
		dy=360.0/h;
		break;
	case CMAP:	      // cylindrical mapping (tiles in x)
		TheNoise.set4D();
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
	        vy=rh*sin(RPD*y);
	        vw=-rh*cos(RPD*y);
	        break;
		default:
		 	vy=y;
	        break;
	    }

		//TheNoise.phi=y-180;
		TheNoise.phi=y;
		Phi=y/360-0.5;
		for(x=x1,i=0;i<w;i++,x+=dx){
			switch(mtype){
			case SMAP:
				pt=Point(x,y,rh);
				pt=pt.rectangular();
				vx=pt.x;
				vy=pt.y;
				vz=pt.z;
				break;
			case CMAP:
			case TILE:
				vx=-rw*cos(RPD*x);
				vz=rw*sin(RPD*x);
				break;
			case PMAP:
			default:
			    vx=x;
				break;
			}
			TheNoise.theta=x;
			Theta=x/360;
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
					f=clamp(f,0,1);
					if(achnl)
						colors[j*w+i]=Color(f,f,f,f);
					else
						colors[j*w+i]=Color(f,f,f);
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
	    printf("min %g max %g\n",mx,mn);
	    if(mx != mn){
			ma=1/(mx-mn);
			mb=mn*ma;
		}
	 	for(j=0;j<h;j++){
			for(i=0;i<w;i++){
				double v=colors[j*w+i].cd.f;
				double f=ma*v-mb;
				if(achnl)
					colors[j*w+i]=Color(f,f,f,f);
				else
					colors[j*w+i]=Color(f,f,f);
			}
		}
	}

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
// Image::color()      return interpolated color
//-------------------------------------------------------------
Color Image::color(int opt, double s)
{
	Color *c=(Color*)data;
    int n1,n2,m=width-1;
    s=clamp(s,0,1);
    n1=(int)(s*m);

    if(n1>=m)
        return c[m];
    n2=n1+1;

    double f=s*m-n1;

    return c[n1].blend(c[n2],f);
}

//************************************************************
// ImageReader class
//************************************************************
ImageReader::ImageReader()
{
    flags=0;

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
	while((is=list++)>0)
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
int ImageReader::getFileInfo(char *name)
{
    char path[512];
    char dir[512];
    int info=0;

   	getImagePath(name,dir);

	sprintf(path,"%s%s",dir,File.ext);
	if(File.fileExists(path))
		info|=SPX;

	sprintf(path,"%s.bmp",dir);
	if(File.fileExists(path))
		info|=BMP;

	sprintf(path,"%s.jpg",dir);
	if(File.fileExists(path))
		info|=JPG;

	return info;
}

//-------------------------------------------------------------
// ImageReader::getImageInfo   get info for saved image
//-------------------------------------------------------------
ImageSym *ImageReader::getImageInfo(char *name)
{
    char *spx=0;
	int info=getFileInfo(name);
	if(info & SPX){
		spx=readSpxFile(name);
		if(spx){
		    info |=SPX;
	    	TNinode *n=(TNinode*)TheScene->parse_node(spx);
	   		if(n){
	   		     info |=n->itype();
	   		     if(n->t1d())
	   		         info|=T1D;
	   		     else
	   		         info|=T2D;
	   		     info|=TEX;

	   		     delete n;
	        }
	    }
	}
	ImageSym *is=new ImageSym(info,name,0,spx);
	FREE(spx);
	return is;
}

//-------------------------------------------------------------
// ImageReader::getImages  return list of images of specified type
//-------------------------------------------------------------
void ImageReader::getImageInfo(int mode, LinkedList<ImageSym*> &list)
{
	ImageSym *is;
	for(int i=0;i<images.size;i++){
		is=images[i];
	    int info=is->info;
	    int imode=mode&(IMAGE|BANDS);
	    int dmode=mode&(T1D|T2D);
	    int smode=mode&(SPX);

		if(smode && !(info & SPX)){
			continue;
		}
		if(!smode && (info & SPX)){
			continue;
		}

		if(imode && imode != (IMAGE|BANDS)){
			if((mode & IMAGE) && !(info & IMAGE))
				continue;
			if((mode & BANDS) && !(info & BANDS))
				continue;
		}
		if(dmode && dmode != (T1D|T2D)){
			if((mode & T1D) && !(info & T1D))
				continue;
			else if((mode & T2D) && !(info & T2D))
				continue;
		}
		if(is->image){
			if(is->image->accessed())
				BIT_ON(is->info,INUSE);
			if(is->image->newimg())
				BIT_ON(is->info,NEWIMG);
			if(is->image->changed())
				BIT_ON(is->info,CHANGED);
		}
		list.add(new ImageSym(is));
	}
}

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
 	sprintf(path,"%s_alpha.jpg",fn);
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
void ImageReader::makeImagelist()
{
    static int init=1;

    if(init){
		char base[256];
		char sdir[MAXSTR];

		LinkedList<ModelSym*>flist;
		ModelSym* sym;
		ImageSym *is;

		// on first call get info for all BitMaps

	  	File.getBaseDirectory(base);

		sprintf(sdir,"%s%sBitmaps",base,File.separator);
		File.getFileNameList(sdir,"*.spx",flist);
		sprintf(sdir,"%s%sTextures",base,File.separator);
		File.getFileNameList(sdir,"*.bmp",flist);
		File.getFileNameList(sdir,"*.jpg",flist);
		flist.ss();
		cout << "images.size:"<<images.size<<" flist.size:"<< flist.size << endl;
		while((sym=flist++)>0){
			if(!images.inlist(sym->name())){
				is=getImageInfo(sym->name());
				images.add(is);
				images.sort();
			}
		}
		flist.free();
		init=0;
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
// ImageReader::hashName   create a hash name
//-------------------------------------------------------------
void ImageReader::hashName(char *f,int &m,char *fn)
{
    strcpy(fn,f);
 }

//-------------------------------------------------------------
// ImageReader::hashName   create a hash name
//-------------------------------------------------------------
void ImageReader::unhashName(char *f,int &m,char *fn)
{
	strcpy(fn,f);
}

//-------------------------------------------------------------
// ImageReader::load   read an image from a file (TNimage.init)
//-------------------------------------------------------------
Image *ImageReader::load(char *f,TNinode *n)
{
    int info=n->opts;
    char name[256];
    hashName(f,info,name);
    ImageSym *is=getImageInfo(name);

    // see if spx file exits

	if(!is->istring){
		delete is;
		return 0;      // no spx file TNinode needs to build
	}

    // see if spx string has changed

	char buff[4096];

    buff[0]=0;
	if(TheScene->keep_variables()){
		TheScene->set_keep_variables(0);
		n->valueString(buff);
		TheScene->set_keep_variables(1);
	}
	else
		n->valueString(buff);

	strcat(buff,"\n");

	if(strcmp(is->istring,buff)!=0){
		delete is;
		return 0;      // changed: builder needs to rebuild
	}

	Image *image=open(name);
	addImage(name,is->info,buff,image);

	delete is;
	return image;
}

//-------------------------------------------------------------
// ImageReader::load      read an image from a file
//-------------------------------------------------------------
Image *ImageReader::load(char *f,int mode)
{
    char name[256];
    hashName(f,mode,name);
    Image *image=find(name);
    if(image)
		return image;
    image=open(name);

	ImageSym *is=getImageInfo(name);
	addImage(is->text,is->info,is->istring,image);

	delete is;
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
	hashName(f,info,name);
	saveSpxFile(name,buff);
	addImage(name,info,buff,image);
    save(name,image);
}

//-------------------------------------------------------------
// ImageReader::addImage      add image to image list
//-------------------------------------------------------------
void ImageReader::addImage(char *name, int m, char *vstr, Image *im)
{
    ImageSym *is=images.inlist(name);
	if(is){
		DFREE(is->image);
		is->info=m;
		is->image=im;
		is->setIstring(vstr);
		is->setName(name);
	}
	else{
		is=new ImageSym(m,name,im,vstr);
		images.add(is);
		images.sort();
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
char *ImageReader::readSpxFile(char *name)
{
	char path[256];
	FILE *fp=0;

	char *buff=0;

	getImagePath(name,path);
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
Image *ImageReader::open(char *name)
{
	char dir[256];
	char base[256];
  	File.getBaseDirectory(base);
  	char *d=File.separator;
	sprintf(dir,"%s%sBitmaps%s%s",base,d,d,name);

	Image *image=open(name, dir);
	if(image)
		return image;
	sprintf(dir,"%s%sTextures%s%s",base,d,d,name);
	return open(name, dir);
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
	return 0;
}

//-------------------------------------------------------------
// ImageReader::openJpgFile read an image from a file
//-------------------------------------------------------------
Image *ImageReader::openJpgFile(char *name,char *path)
{
	Image *image=0;
	extern GLubyte *readJpegFile(char *path,int &w, int &h, int &c);
	char cpath[256];

 	sprintf(cpath,"%s.jpg",path);

 	FILE *fp=fopen(cpath, "rb");
	if (fp == NULL)
	    return 0;
	fclose(fp);

	int width=0;
	int height=0;
	int comps=0;
	GLubyte *pxls=readJpegFile(cpath,width,height,comps);
    if(pxls){
		image=new Image((RGBColor*)pxls,width,height);
		image->set_global(1);
		if(comps==4)
			image->set_alpha_image(1);
		else
			image->set_alpha_image(0);
    }   
	return image;	
}

//-------------------------------------------------------------
// ImageReader::openBmpFile read an image from a file
//-------------------------------------------------------------
Image *ImageReader::openBmpFile(char *name,char *path)
{
	BITMAPINFO	*info=0;
	void *bits;
	void *pxls;
	Image *image=0;
	int aflag=1;

	char cpath[256];
	char apath[256];

 	sprintf(cpath,"%s.bmp",path);

 	FILE *fp=fopen(cpath, "rb");
	if (fp == NULL)
	    return 0;
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

	bits=LoadDIBitmap(cpath,&info);

	if(!bits){
		printf("Couldn't read image file %s\n",cpath);
		return 0;
	}

	if(info->bmiHeader.biBitCount != 24){
		FREE(bits)
		return 0;
	}

	if(aflag)
		pxls=BitmapToPixels(info,bits,4);
	else
		pxls=BitmapToPixels(info,bits,3);

	FREE(bits);
	if(!pxls){
		printf("Couldn't create image for %s\n",name);
		FREE(info);
    	return 0;
	}

	int h=info->bmiHeader.biHeight;
	int w=info->bmiHeader.biWidth;
	if(!aflag){
		image=new Image((RGBColor*)pxls,w,h);
		image->set_global(1);
		image->set_alpha_image(0);
		FREE(info);
		return image;
	}

	image=new Image((Color*)pxls,w,h);
	image->set_global(1);
	image->set_alpha_image(0);

	FREE(info);
	bits=LoadDIBitmap(apath,&info);
	if(bits){
		void *apxls=BitmapToPixels(info,bits,3);
		FREE(bits);
		if(apxls){
		    image->addAlphaToImage((GLubyte *)apxls);
		    FREE(apxls);
		}
	}
	FREE(info);
	return image;
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
// ImageReader::save      save an image to a file
//-------------------------------------------------------------
void ImageReader::save(char *f, Image *image)
{
	char path[256];

#ifdef DEBUG_IMAGES
	printf("%-20s SAVING IMAGE %s\n","ImageReader",f);
#endif

	getImagePath(f,path);
   	strcat(path,".bmp");

	BITMAPINFO	*info=makeBitmapInfo(image->width,image->height);
	if(!info)
	    return;

  	void *pxls=PixelsToBitmap(info,image->data,0);

  	SaveDIBitmap(path,info,pxls);
	::free(pxls);
	getImagePath(f,path);
   	strcat(path,"_alpha.bmp");

	if(image->alpha()){
		pxls=PixelsToBitmap(info,image->data,1);
  		SaveDIBitmap(path,info,pxls);
		::free(pxls);
	}
	else
	    File.deleteFile(path);
	FREE(info);
}
