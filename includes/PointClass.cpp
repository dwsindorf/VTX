// PointClass.cpp

#include "PointClass.h"
#include "defs.h"

double TWOPI=2.0*PI;
double INV2PI=1.0/TWOPI;

const double PIBY2=PI/2.0;
const double DTOR=TWOPI/360.0;

const int  SINE_LUT_SIZE=4096*4;
const double SINE_LUT_STEP=SINE_LUT_SIZE/TWOPI;

static double lut[SINE_LUT_SIZE];
static int sin_lut_flag=0;

//-------------------------------------------------------------
//make_lut	build the sine look-up table (one time only)
//-------------------------------------------------------------
void make_lut()
{
	if(sin_lut_flag)
		return;
	cout << "building sin LUT"<<endl;
    for (int i = 0; i < SINE_LUT_SIZE; ++i) {
        double angle = (TWOPI * i) / SINE_LUT_SIZE;
        lut[i] = sin(angle);
    }
	sin_lut_flag=1;
}

//-------------------------------------------------------------
// lsin(): sin lut approximation to sin()
//-------------------------------------------------------------
double lsin(double t)
{	
	make_lut();
    // Wrap x into [0, TWO_PI)
    double x = fmod(t, TWOPI);
    if (x < 0) x += TWOPI;

    // Scale to table index
    double index = (x *INV2PI) * SINE_LUT_SIZE;
    int f0 = (int)index;
    int f1 = (f0 + 1) % SINE_LUT_SIZE;
    double frac = index - f0;

    // Linear interpolation
    double ls=lut[f0] * (1.0 - frac) + lut[f1] * frac;
	return ls;
}
//-------------------------------------------------------------
// lcos(): sin lut approximation to cos()
//-------------------------------------------------------------
double lcos(double t)
{
	make_lut();
	return lsin(t+PIBY2);
}

//************************************************************
// Point class
//************************************************************
//-------------------------------------------------------------
// Point::normal()	surface normal
//-------------------------------------------------------------
Point Point::normal(Point p2, Point p3)	
{	
	Point p,v1,v2;
	double l;
	p=*this;
	v1=p2-p;
	v2=p3-p;
	p=v2.cross(v1);
	l=p.length();
	return l ? p*(1/l) : p;
}

//-------------------------------------------------------------
// Point::rectangular()	spherical-to-cartesion conversion
//-------------------------------------------------------------
Point Point::rectangular()
{
	double t=RPD*x, p=RPD*y;
	double f;
	f=z*COS(p);
	return Point(-f*COS(t),z*SIN(p),f*SIN(t));
}

inline void cartesianToSpherical(double x, double y, double z,
		double& r, double& t, double& p) {
    r =sqrt(x * x + y * y + z * z);
    if (r > 1e-8) 
        t = acos(z / r);        // inclination
    else 
        t = 0.0f;
    p = atan2(y, x);              // azimuth
}
//-------------------------------------------------------------
// Point::spherical()	 cartesion-to-spherical conversion
//-------------------------------------------------------------
Point Point::spherical()
{
	double t,p,r;
	r=sqrt(x*x+y*y+z*z);
	t=asin(y/r)*DPR;
	p=atan2(-z,x)*DPR;
	return Point(t,p,r);
}

//-------------------------------------------------------------
// Point::print()	 print value
//-------------------------------------------------------------
void Point::print()
{
	print("");
}

//-------------------------------------------------------------
// Point::print()	 print value
//-------------------------------------------------------------
void Point::print(char *m)
{
    double px=x;
    double py=y;
    double pz=z;
    int r=5;
    const char *s="";
    if(length()>LY){
        px/=LY;
        py/=LY;
        pz/=LY;
        r=1;
        s="ly";
    }
	printf("%s%-15g %-15g %-4.1f %s ",m,round(px,r),round(py,r),pz,s);
}

//-------------------------------------------------------------
// Point::intersect_sphere()	 sphere intersection
//-------------------------------------------------------------
int Point::intersect_sphere(Point pr, Point ps, double rad, double &d1, double &d2)
{
    Point p=*this;
    Point r=pr-p;
    Point s=p-ps;
    
    double a=r.magnitude(); 
    double b=2*(r.x*s.x+r.y*s.y+r.z*s.z);
    double c=s.magnitude()-rad*rad;
    
    double d=b*b-4*a*c;
    if(d<0)
        return -1;  // no intersection
    if(d==0)
        return 0;   // tangent
    d=sqrt(d);
        
    d1=(-b-d)/2/a;
    d2=(-b+d)/2/a;
    
    return 1;
}
//************************************************************
// Point4D class
//************************************************************
//-------------------------------------------------------------
// Point4D::rectangular()	spherical-to-cartesion conversion
//-------------------------------------------------------------
Point4D Point4D::rectangular()
{
	double t=RPD*x, p=RPD*y;
	return Point4D(-w*COS(t),z*SIN(p),w*SIN(t),-z*COS(p));
}

//-------------------------------------------------------------
// Point4D::print()	 print value
//-------------------------------------------------------------
void Point4D::print()
{
    int r=5;
	printf("%-15g %-15g %-15g %-15g\n",
		round(x,r),round(y,r),round(z,r),round(w,r));
}

//************************************************************
// LPoint class
//************************************************************

uint	LTheta::MAXBIT		= 31;
uint	LTheta::MAX		    = 1<<MAXBIT;
uint	LTheta::MASK		= MAX-1;
double	LTheta::INV_MAX		= 1.0/(double)MAX;
double	LTheta::SCALE		= (double)MAX/360.0;
double	LTheta::INV_SCALE	= 1.0/SCALE;

uint	LPhi::MAXBIT		= 30;
uint	LPhi::MAX		    = 1<<MAXBIT;
double	LPhi::INV_MAX		= 1.0/(double)MAX;
uint	LPhi::MASK		    = MAX-1;
double	LPhi::SCALE			= (double)MASK/180.0;
double	LPhi::INV_SCALE		= 1.0/SCALE;

uint	LRad::MAX			= 0x40000000;
double	LRad::INV_MAX		= 1.0/(double)MAX;
uint	LRad::MASK			= MAX-1;
double  LRad::SCALE			= (double)MAX;
double  LRad::INV_SCALE		= 1.0/SCALE;

//-------------------------------------------------------------
// LPoint::rectangular()	spherical-to-cartesion conversion
//-------------------------------------------------------------
Point  LPoint::rectangular()	
{	
	double xr=TWOPI*t.lvalue()/t.MAX; // radians
	double yr=PI*p.lvalue();
	double zr=r.INV_SCALE*r.lvalue();
	double f;
	f=zr*COS(yr);
	return Point(-f*COS(xr),zr*SIN(yr),f*SIN(xr));
}
