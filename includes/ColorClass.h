// ColorClass.h

#include "GLheaders.h"

#include <stddef.h>
#include <stdio.h>

#ifndef _COLORCLASS
#define _COLORCLASS

#define	CMAX	 255
#define	INVCMX	 (double)(1.0/CMAX)

//************************************************************
// RGBColor class
//************************************************************
class RGBColor
{
protected:
public:
	GLubyte r,g,b;
	RGBColor() {}
	RGBColor(double c1,double c2,double c3)
									{	set(c1,c2,c3);}

	GLubyte clamp(double f)				{	return f<=0.0 ? (GLubyte)0 :
										(f>=1.0 ? (GLubyte)CMAX:(GLubyte)(f*CMAX));
									}
	void set(double c1,double c2,double c3)
									{	r=clamp(c1);
										g=clamp(c2);
										b=clamp(c3);
									}
	void set_red(double a)			{	r=clamp(a);}
	void set_green(double a)		{	g=clamp(a);}
	void set_blue(double a)			{	b=clamp(a);}
	double red()					{	return (double)(r*INVCMX);}
	double green()					{	return (double)(g*INVCMX);}
	double blue()					{	return (double)(b*INVCMX);}
	int operator==(RGBColor c)		{	return r!=c.r?0:g!=c.g?0:b!=c.b?0:1;}
	int operator!=(RGBColor c)		{	return !(*this==c);}
	RGBColor complement()				{	return RGBColor(1.0f-red(),1.0f-green(),1.0f-blue());}
	RGBColor blend(RGBColor b, double f);
	double intensity()				{	return (INVCMX/3.0)*(r+g+b);}
	unsigned char pack332()			{	unsigned char c;
										c =  r&0xe0;
										c |= (g&0xe0)>>3;
										c |= b>>6;
										return c;
									}
	void unpack332(unsigned char c) {   r=c&0xe0;
										g=(c<<3)&0xe0;
										b=c<<6;
									}
	unsigned int pack655()			{	unsigned int c;
										c =  r&0xfc00;
										c |= (g&0xf800)>>6;
										c |= b>>3;
										return c;
									}
	void unpack655(unsigned int c)  {   r=c&0xfc00;
										g=(c<<6)&0xf800;
										b=c<<3;
									}
	RGBColor lighten(double f);
	RGBColor darken(double f);
	void print();
	class Color color();
};

//************************************************************
// Color class
//************************************************************
typedef struct cdc {
	unsigned int  r  : 8;
	unsigned int  g  : 8;
	unsigned int  b  : 8;
	unsigned int  a  : 8;
} cdc;

typedef union cdu {
	cdc			 c;
	int          l;
	float        f;
} cdu;

class Color
{
public:
	static double min_brightness;
	static double min_contrast;

	cdu cd;
	Color()							{	cd.c.a=CMAX; }
	Color(char *);
	Color(unsigned int l)			{	cd.l=l; }
	Color(double c1,double c2,double c3)
									{	cd.c.r=clamp(c1);
										cd.c.g=clamp(c2);
										cd.c.b=clamp(c3);
										cd.c.a=CMAX;
									}
	Color(double c1,double c2,double c3, double c4)
									{	cd.c.r=clamp(c1);
										cd.c.g=clamp(c2);
										cd.c.b=clamp(c3);
										cd.c.a=clamp(c4);
									}
	void set(double c1,double c2,double c3, double c4)
									{	cd.c.r=clamp(c1);
										cd.c.g=clamp(c2);
										cd.c.b=clamp(c3);
										cd.c.a=clamp(c4);
									}
	void set(GLubyte c1,GLubyte c2,GLubyte c3, GLubyte c4)
									{	cd.c.r=c1;
										cd.c.g=c2;
										cd.c.b=c3;
										cd.c.a=c4;
									}
	void set(GLubyte c1,GLubyte c2,GLubyte c3)
									{	cd.c.r=c1;
										cd.c.g=c2;
										cd.c.b=c3;
										cd.c.a=CMAX;
									}

	GLubyte clamp(double f)			{	return f<=0.0 ? (GLubyte)0 :
										(f>=1.0 ? (GLubyte)CMAX:(GLubyte)(f*CMAX));
									}
	void get_values(double *rgba)	{	rgba[0]=red();
										rgba[1]=green();
										rgba[2]=blue();
										rgba[3]=alpha();
									}
	void get_values(float *rgba)	{	rgba[0]=red();
										rgba[1]=green();
										rgba[2]=blue();
										rgba[3]=alpha();
									}
	void set_red(double c)			{	cd.c.r=clamp(c);}
	void set_green(double c)		{	cd.c.g=clamp(c);}
	void set_blue(double c)			{	cd.c.b=clamp(c);}
	void set_alpha(double c)		{	cd.c.a=clamp(c);}
	void set_rb(GLubyte c)			{	cd.c.r=c;}
	void set_gb(GLubyte c)			{	cd.c.g=c;}
	void set_bb(GLubyte c)			{	cd.c.b=c;}
	void set_ab(GLubyte c)			{	cd.c.a=c;}
	GLubyte rb()					{   return (GLubyte)cd.c.r;}
	GLubyte gb()					{   return (GLubyte)cd.c.g;}
	GLubyte bb()					{   return (GLubyte)cd.c.b;}
	GLubyte ab()					{   return (GLubyte)cd.c.a;}
	double red()					{	return (double)(cd.c.r*INVCMX);}
	double green()					{	return (double)(cd.c.g*INVCMX);}
	double blue()					{	return (double)(cd.c.b*INVCMX);}
	int hasAlpha()					{   return cd.c.a==CMAX?0:1;}
	double alpha()					{	return cd.c.a==CMAX?1.0:(double)(cd.c.a*INVCMX);}
	Color operator+(Color c)	    {   return Color(red()+c.red(),green()+c.green(),blue()+c.blue());}
	Color operator+(double s)	    {   return Color(red()+s,green()+s,blue()+s);}
	Color operator-(Color c)	    {   return Color(red()-c.red(),green()-c.green(),blue()-c.blue());}
	Color operator-(double s)	    {   return Color(red()-s,green()-s,blue()-s);}
	Color operator*(Color c)	    {   return Color(red()*c.red(),green()*c.green(),blue()*c.blue());}
	Color operator*(double s)	    {   return Color(red()*s,green()*s,blue()*s);}
	Color operator/(Color c)	    {   return Color(red()/c.red(),green()/c.green(),blue()/c.blue());}
	Color operator/(double s)	    {   return Color(red()/s,green()/s,blue()/s);}
	int operator==(Color c)			{	return cd.l==c.cd.l?1:0;}
	int operator!=(Color c)			{	return !(*this==c);}
	double operator[](int i)	    {   switch(i){
	                                    default:
									    case 0: return red();
									    case 1: return green();
									    case 2: return blue();
									    case 3: return alpha();
										}
	                                }

	double operator()(int i,double d) {   switch(i){
	                                    default:
									    case 0: set_red(d);
									    case 1: set_green(d);
									    case 2: set_blue(d);
									    case 3: set_alpha(d);
										}
										return d;
	                                }
	Color complement()				{	return Color(1.0f-red(),1.0f-green(),1.0f-blue());}
	Color grayscale()				{	double g=intensity(); return Color(g,g,g);}
	double intensity()				{	return (INVCMX/3.0)*(cd.c.r+cd.c.g+cd.c.b);}
	Color cmax(Color c)				{   return intensity()>c.intensity()? *this:c;}
	Color cmin(Color c)				{   return intensity()<c.intensity()? *this:c;}
	Color blend(Color b, double f);
	Color lighten(double f);
	Color darken(double f);
	void  floatArray(float *f);
	double difference(Color c);
	RGBColor rgb();
	void print();
	unsigned char pack332()			{	GLubyte c;
										c =  (GLubyte)(cd.c.r&0xe0);
										c |= (cd.c.g&0xe0)>>3;
										c |= cd.c.b>>6;
										return c;
									}
	void unpack332(unsigned char c) {   cd.c.r=c&0xe0;
										cd.c.g=(c<<3)&0xe0;
										cd.c.b=c<<6;
									}
	unsigned int pack655()			{	unsigned int c;
										c =  cd.c.r&0xfc00;
										c |= (cd.c.g&0xf800)>>6;
										c |= cd.c.b>>3;
										return c;
									}
	void unpack655(unsigned int c)  {   cd.c.r=c&0xfc00;
										cd.c.g=(c<<6)&0xf800;
										cd.c.b=c<<3;
									}
	unsigned int pack()			{   return cd.l;}
	void unpack(int l)				{   cd.l=l;}
	void toString(char *);
	double brightness();
	double contrast(Color c);
	Color mix(Color c,double t);
	Color contrast_color(Color fg,double ampl);
	Color color()               { return Color(red(),blue(),green(),1.0);}
	Color intensify(double f);
};

//************************************************************
// FColor class
//************************************************************
class FColor
{
public:
	float  rgba[4];
	void   set(Color s)				{	rgba[0]=(float)s.red();
										rgba[1]=(float)s.green();
										rgba[2]=(float)s.blue();
										rgba[3]=(float)s.alpha();
									}
	FColor(double r, double g, double b)
									{	rgba[0]=(float)r;
										rgba[1]=(float)g;
										rgba[2]=(float)b;
										rgba[3]=1.0f;
									}
	FColor(double r, double g, double b, double a)
									{	rgba[0]=(float)r;
										rgba[1]=(float)g;
										rgba[2]=(float)b;
										rgba[3]=(float)a;
									}
	FColor(const FColor &s)			{	rgba[0]=s.rgba[0];rgba[1]=s.rgba[1];rgba[2]=s.rgba[2];rgba[3]=s.rgba[3];}
	FColor(Color s)					{	set(s);}
	FColor()						{	rgba[0]=rgba[1]=rgba[2]=0.0f;
										rgba[3]=1.0f;
									}
	double red()					{	return rgba[0];}
	double green()					{	return rgba[1];}
	double blue()					{	return rgba[2];}
	double alpha()					{	return rgba[3];}
	void set_red(double c)			{	rgba[0]=(float)c;}
	void set_green(double c)		{	rgba[1]=(float)c;}
	void set_blue(double c)			{	rgba[2]=(float)c;}
	void set_alpha(double c)		{	rgba[3]=(float)c;}

	FColor operator+(FColor c)	    {   return FColor(red()+c.red(),green()+c.green(),blue()+c.blue());}
	FColor operator+(double s)	    {   return FColor(red()+s,green()+s,blue()+s);}
	FColor operator-(FColor c)	    {   return FColor(red()-c.red(),green()-c.green(),blue()-c.blue());}
	FColor operator-(double s)	    {   return FColor(red()-s,green()-s,blue()-s);}
	FColor operator*(FColor c)	    {   return FColor(red()*c.red(),green()*c.green(),blue()*c.blue());}
	FColor operator*(double s)	    {   return FColor(red()*s,green()*s,blue()*s);}
	FColor operator/(FColor c)	    {   return FColor(red()/c.red(),green()/c.green(),blue()/c.blue());}
	FColor operator/(double s)	    {   return FColor(red()/s,green()/s,blue()/s);}

	double intensity()				{	return (1.0/3.0)*(red()+green()+blue());}
	FColor complement()				{	return FColor(1.0f-red(),1.0f-green(),1.0f-blue());}
	FColor grayscale()				{	double g=intensity(); return FColor(g,g,g);}
	void operator=(Color s)	        {   set(s);}
	FColor blend(Color b, double f);
	FColor lighten(double f);
	FColor darken(double f);
	float *values()					{   return rgba;}
	void cmax(FColor &f);
	void cmin(FColor &f);
	void rescale(FColor &f);
	double maxcomp();
	double mincomp();
	double brightness();
	double contrast(FColor c2);
	void setmax();
	void setmin();
	void print();
	double difference(FColor c);
	FColor mix(FColor c,double t);
	FColor clamp();
	FColor contrast_color(FColor fg,double ampl);
	FColor rgb2hsv();
	FColor hsv2rgb();
	FColor intensify(double f);

	friend class Color;
};

#define BLACK		Color(0.0,0.0,0.0)
#define WHITE		Color(1.0,1.0,1.0)

#endif

