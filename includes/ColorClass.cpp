// LightClass.cpp

#include "defs.h"
#include "ColorClass.h"
#include "Util.h"

//#include "NoiseClass.h"

#include <math.h>

#define MAX(a,b) a>b?a:b
#define MIN(a,b) a<b?a:b

double Color::min_brightness=0.5;
double Color::min_contrast=0.4;

static double clamp(double x, double l, double m) {
	if(x<l)
		return l;
	if(x>m)
		return m;
	return x;
}

// RGBColor class

RGBColor RGBColor::blend(RGBColor c, double f)
{
	f=::clamp(f,0,1.0);
	RGBColor tmp;
	tmp.set_red(f*c.red()+(1-f)*red());
	tmp.set_green(f*c.green()+(1-f)*green());
	tmp.set_blue(f*c.blue()+(1-f)*blue());
	return tmp;
}

RGBColor RGBColor::lighten(double f)
{
	return blend(RGBColor(1,1,1),f);
}

RGBColor RGBColor::darken(double f)
{
	return blend(RGBColor(0,0,0),f);
}

void RGBColor::print()
{
	printf("RGBColor(%g,%g,%g)\n",red(),green(),blue());
}

Color RGBColor::color()
{
	return Color(red(),blue(),green(),1.0);
}

// Color class

Color::Color(char *s)
{
    char buff[256];
    strcpy(buff,s);
 	float c1=1.0f;
	float c2=1.0f;
	float c3=1.0f;
	float c4=1.0f;
	char *cptr=strtok(buff,"(");
	if(strcmp(cptr,"Color"))
	    printf("Color syntax error %s",cptr);
	cptr=strtok(0,",");
	if(cptr)
        sscanf(cptr,"%g",&c1);
	cptr=strtok(0,",");
	if(cptr)
        sscanf(cptr,"%g",&c2);
	cptr=strtok(0,",)");
	if(cptr)
        sscanf(cptr,"%g",&c3);
	cptr=strtok(0,")");
	if(cptr)
        sscanf(cptr,"%g",&c4);

	cd.c.r=clamp(c1);
	cd.c.g=clamp(c2);
	cd.c.b=clamp(c3);
	if(c4==1)
    	cd.c.a=CMAX;
    else
    	cd.c.a=clamp(c4);
    //print();
}

void Color::toString(char *s)
{
	if(hasAlpha())
		sprintf(s,"Color(%0.3f,%0.3f,%0.3f,%0.3f)",red(),green(),blue(),alpha());
	else
		sprintf(s,"Color(%0.3f,%0.3f,%0.3f)",red(),green(),blue());
}

void  Color::floatArray(float *f)
{
	f[0]=(float)red();
	f[1]=(float)green();
	f[2]=(float)blue();
	f[3]=(float)alpha();
}

Color Color::blend(Color c, double f)
{	Color tmp;
    tmp.cd.c.r=lerp(f,0.0,1.0,cd.c.r,c.cd.c.r);
    tmp.cd.c.g=lerp(f,0.0,1.0,cd.c.g,c.cd.c.g);
    tmp.cd.c.b=lerp(f,0.0,1.0,cd.c.b,c.cd.c.b);
    tmp.cd.c.a=lerp(f,0.0,1.0,cd.c.a,c.cd.c.a);
	return tmp;
}
Color Color::lighten(double f)
{
	return blend(Color(1,1,1),f);
}

Color Color::darken(double f)
{
	return blend(Color(0,0,0),f);
}

double Color::brightness(){
	return ((red()*0.299) + (green()*0.587) + (blue()*0.114));
}

double Color::contrast(Color c){
	double diff=MAX(red(), c.red()) - MIN(red(), c.red())
	+ MAX(green(), c.green()) - MIN(green(), c.green())
	+ MAX(blue(), c.blue()) - MIN(blue(), c.blue());
	return diff/3.0;
}

double Color::difference(Color c)
{
//	double db=fabs(brightness()-c.brightness())/Color::min_brightness;
//	double dc=fabs(contrast(c))/Color::min_contrast;
//	return MAX(db,dc);
	double rr,gg,bb,aa;
	rr=red()-c.red();
	gg=green()-c.green();
	bb=blue()-c.blue();
	aa=alpha()-c.alpha();
	return 0.5*sqrt(rr*rr+gg*gg+bb*bb+aa*aa);
}

Color Color::mix(Color c,double t){
	FColor c1=c;
	FColor c2=(*this);
	c1=c1*t;
	c2=c2*(1.0-t);
	FColor c3=c1+c2;
	return Color(c3.red(),c3.green(),c3.blue(),c3.alpha());
}

Color Color::contrast_color(Color c,double ampl){
	FColor fg(c.red(),c.green(),c.blue());
	FColor bg(red(),green(),blue());
	double diff=fg.difference(bg);
	if(diff<1.0){
		FColor fg1=bg.mix(fg,ampl).clamp();
		double diff2=fg1.difference(bg);
		if(diff2>1)
			fg=fg1;
		else if(diff2<diff)
			fg=fg.complement();
		else
			fg=fg1.complement();
	}
	return Color(fg.red(),fg.blue(),fg.green());
}

RGBColor Color::rgb()
{
	return RGBColor(red(),blue(),green());
}

void Color::print()
{
	if(hasAlpha())
		printf("Color(%g,%g,%g,%g)\n",red(),green(),blue(),alpha());
	else
		printf("Color(%g,%g,%g)\n",red(),green(),blue());
}

// FColor class

FColor FColor::blend(Color c, double f)
{	FColor tmp;
	f=::clamp(f,0,1.0);
	tmp.set_red(f*c.red()+(1-f)*red());
	tmp.set_green(f*c.green()+(1-f)*green());
	tmp.set_blue(f*c.blue()+(1-f)*blue());
	tmp.set_alpha(f*c.alpha()+(1-f)*alpha());
	return tmp;
}
FColor FColor::lighten(double f)
{
	return blend(Color(1,1,1),f);
}

FColor FColor::darken(double f)
{
	return blend(Color(0,0,0),f);
}

double FColor::maxcomp()
{
    double v=rgba[0]>rgba[1]?rgba[0]:rgba[1];
    return v>rgba[2]?v:rgba[2];
}

double FColor::mincomp()
{
    double v=rgba[0]<rgba[1]?rgba[0]:rgba[1];
    return v<rgba[2]?v:rgba[2];
}

void FColor::setmax()
{
    float v=(float)maxcomp();
    rgba[0]=rgba[1]=rgba[2]=v;
}

void FColor::setmin()
{
    float v=(float)mincomp();
    rgba[0]=rgba[1]=rgba[2]=v;
}

void FColor::cmax(FColor &c)
{
    for(int i=0;i<4;i++)
        rgba[i]=rgba[i]>c.rgba[i]?rgba[i]:c.rgba[i];
}
void FColor::cmin(FColor &c)
{
    for(int i=0;i<4;i++)
        rgba[i]=rgba[i]<c.rgba[i]?rgba[i]:c.rgba[i];
}

void FColor::rescale(FColor &c)
{
    for(int i=0;i<3;i++){
        if(rgba[i]==c.rgba[i]){
            rgba[i]=1.0f;
            c.rgba[i]=0.0f;
        }
        else{
            rgba[i]=1.0f/(rgba[i]-c.rgba[i]);
            c.rgba[i]*=rgba[i];
        }
    }
}

double FColor::difference(FColor c)
{
	double db=fabs(brightness()-c.brightness())/Color::min_brightness;
	double dc=fabs(contrast(c))/Color::min_contrast;
	return MAX(db,dc);
}

double FColor::brightness(){
	return ((red()*0.299) + (green()*0.587) + (blue()*0.114));
}
double FColor::contrast(FColor c2){
	double diff=MAX(red(), c2.red()) - MIN(red(), c2.red())
	+ MAX(green(), c2.green()) - MIN(green(), c2.green())
	+ MAX(blue(), c2.blue()) - MIN(blue(), c2.blue());
	return diff/3.0;
}

FColor FColor::clamp(){
	double r=MAX(red(),0.0);
	r=MIN(r,1.0);
	double g=MAX(green(),0.0);
	g=MIN(g,1.0);
	double b=MAX(blue(),0.0);
	b=MIN(b,1.0);
	double a=MAX(alpha(),0.0);
	a=MIN(a,1.0);
	return FColor(r,g,b,a);
}

FColor FColor::mix(FColor c,double t){
	FColor c1,c2,c3;
	c1=c*t;
	//FColor c2=(*this)*(1.0-t);
	c2=*this;
	c2=c2*(1-t);
	c3=c1+c2;
	//c3.print();
	return c3;
}

FColor FColor::contrast_color(FColor fg,double ampl){
	FColor bg(*this);
	double diff=fg.difference(bg);
	if(diff<1.0){
		FColor fg1=bg.mix(fg,ampl).clamp();
		double diff2=fg1.difference(bg);
		if(diff2>1)
			return fg1;
		else if(diff2<diff)
			return fg.complement();
		else
			return fg1.complement();
	}
	return fg;
}

void FColor::print()
{
	if(alpha()!=1)
		printf("FColor(%g,%g,%g,%g)\n",red(),green(),blue(),alpha());
	else
		printf("FColor(%g,%g,%g)\n",red(),green(),blue());
}

