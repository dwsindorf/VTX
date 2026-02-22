// PointClass.h

#ifndef _POINTCLASS
#define _POINTCLASS
#include <math.h>
#include "defs.h"
#define RNDERR 1e-12
#define INTERR 2
#define LDIFF(x,y)(fabs((x)-(y)))
#define DEQ(x,y) (fabs((x)-(y))<RNDERR)
#define LEQ(x,y) (abs((x)-(y))<INTERR)
class Point
{
public:
	double x,y,z;

	Point()						{	x=y=z=0.0;}
	Point(double *v)			{	x=v[0];y=v[1];z=v[2];}
	Point(double a)				{	x=y=z=a;}

	Point(double a,double b,double c)
	                            {	x=a;
									y=b;
									z=c;
								}
	Point operator+(const Point& p) const {return Point(x+p.x, y+p.y, z+p.z);}
	Point operator-(const Point& p) const {return Point(x-p.x, y-p.y, z-p.z);}
	Point operator+(Point p)	{ return Point(x+p.x,y+p.y,z+p.z);}
	Point operator+(double s)	{ return Point(x+s,y+s,z+s);}
	Point operator-(Point p)	{ return Point(x-p.x,y-p.y,z-p.z);}
	Point operator-(double s)	{ return Point(x-s,y-s,z-s);}
	Point operator*(Point p)	{ return Point(x*p.x,y*p.y,z*p.z);}
	Point operator*(double s)	{ return Point(x*s,y*s,z*s);}
	Point operator/(Point p)	{ return Point(x/p.x,y/p.y,z/p.z);}
	Point operator/(double s)	{ return Point(x/s,y/s,z/s);}
	int operator<(Point&p)		{ return (z<p.z);}
	int operator>(Point&p)		{ return (z>p.z);}
	int operator==(Point&p)		{ return DEQ(x,p.x)&&DEQ(y,p.y)&&DEQ(z,p.z);}
	int operator!=(Point&p)		{ return !(*this==p);}
	double operator[](int i) const	{ return ((i==0)?x:((i==1)?y:z));}
	double dot(const Point&p)	{ return (x*p.x+y*p.y+z*p.z);}
	Point cross(const Point&p)	{ return Point(y*p.z-z*p.y,z*p.x-x*p.z,x*p.y-y*p.x);}
	double length() const		{ return sqrt(x*x+y*y+z*z);}
	double magnitude() const	{ return x*x+y*y+z*z;}
	double distance(const Point p) const	{ Point tmp=p-*this;
								  return tmp.length();
								}
	Point normalize() const     { return (*this)/length();}
	Point floor()               { return Point(::floor(x),::floor(y),::floor(z));}
	Point fract()               { Point p=floor(); return *this-p;}

	Point trunc()               { return Point(::trunc(x),::trunc(y),::trunc(z));}
	Point absval()              { return Point(::fabs(x),::fabs(y),::fabs(z));}
	Point normal(Point p2,Point p3);
	Point rectangular();
	Point spherical();
	void print();
	void print(char *m);

	Point  mm(double *m){
		Point p;
		p.x=(x*m[0]+y*m[4]+z*m[8]+m[12]);
		p.y=(x*m[1]+y*m[5]+z*m[9]+m[13]);
		p.z=(x*m[2]+y*m[6]+z*m[10]+m[14]);
		return p;
	}
	Point OrthoNormal(Point v) {
	  double g = v.z>0?1.0:-1.0;
	  double h = v.z + g;
	  return Point(g - v.x*v.x/h, -v.x*v.y/h, -v.x);
	}
	double mx(double *m)		{  return x*m[0]+y*m[4]+z*m[8]+m[12];}
	double my(double *m)		{  return x*m[1]+y*m[5]+z*m[9]+m[13];}
	double mz(double *m)		{  return x*m[2]+y*m[6]+z*m[10]+m[14];}
	double mf(double *m)		{  return x*m[3]+y*m[7]+z*m[11]+m[15];}
	double *values()            {  return &x;}
	double maxval()  const      {  double f=x>y?x:y;return f>z?f:z;}
	double minval()  const      {  double f=x<y?x:y;return f<z?f:z;}
	int intersect_sphere(Point, Point, double r, double &, double &);
	friend std::ostream& operator<<(std::ostream& os, const Point& b);
};

class Point2D
{
public:
	double x,y;

	Point2D()						{	x=y=0.0;}
	Point2D(double *v)			{	x=v[0];y=v[1];}

	Point2D(double a)				{	x=y=a;}
	Point2D(double a,double b)
	                            {	x=a;
									y=b;
								}
	Point2D(Point p)
	                            {	x=p.x;
									y=p.y;
								}
	Point2D operator+(Point2D p)	{ return Point2D(x+p.x,y+p.y);}
	Point2D operator+(double s)	{ return Point2D(x+s,y+s);}
	Point2D operator-(Point2D p)	{ return Point2D(x-p.x,y-p.y);}
	Point2D operator-(double s)	{ return Point2D(x-s,y-s);}
	Point2D operator*(Point2D p)	{ return Point2D(x*p.x,y*p.y);}
	Point2D operator*(double s)	{ return Point2D(x*s,y*s);}
	Point2D operator/(Point2D p)	{ return Point2D(x/p.x,y/p.y);}
	Point2D operator/(double s)	{ return Point2D(x/s,y/s);}
	int operator==(Point2D&p)		{ return DEQ(x,p.x)&&DEQ(y,p.y);}
	int operator!=(Point2D&p)		{ return !(*this==p);}
	double operator[](int i)	{ return i==0?x:y;}
	double dot(Point2D&p)			{ return (x*p.x+y*p.y);}
	double length()				    { return sqrt(x*x+y*y);}
	double magnitude()			{ return x*x+y*y;}

	Point2D normalize()           { return (*this)/length();}
	Point2D floor()               { return Point2D(::floor(x),::floor(y));}
	Point2D fract()                { Point2D p=floor(); return *this-p;}

	Point2D trunc()               { return Point2D(::trunc(x),::trunc(y));}
	Point2D absval()              { return Point2D(::fabs(x),::fabs(y));}
	Point2D normal(Point p2,Point p3);
	void print();
	double *values()            {  return &x;}
	double maxval()             {  return x>y?x:y;}
	double minval()             {  return x<y?x:y;}
	Point2D  mm4(double *m){
		Point2D p;
		p.x=(x*m[0]+y*m[2]);
		p.y=(x*m[1]+y*m[3]);
		return p;
	}
};

class Point4DL
{
public:
	int x,y,z,w;
	Point4DL() {x=y=z=w=0;}
	Point4DL(Point4DL&p){
	    x=p.x;y=p.y;z=p.z;w=p.w;
	}
	Point4DL(int a, int b, int c, int d){
	    x=a;y=b;z=c;w=d;
	}
	bool almost_equal(Point4DL&p)   { return (LDIFF(x,p.x)+ LDIFF(y,p.y) + LDIFF(z,p.z))<INTERR;}
	bool equals(Point4DL p)         { return (x==p.x)&&(y==p.y)&&(z==p.z)&&(w==p.w);}
	bool operator==(Point4DL&p)		{ return equals(p);}
	bool operator!=(Point4DL&p)		{ return !equals(p);}
};

class Point4D
{
public:
	double x,y,z,w;

	Point4D()						{	x=y=z=w=0.0;}
	Point4D(double *v)			{	x=v[0];y=v[1];z=v[2];w=v[3];}

	Point4D(double a,double b,double c)
								{	x=a;
									y=b;
									z=c;
									w=0;
								}
	Point4D(double a,double b,double c,double d)
								{	x=a;
									y=b;
									z=c;
									w=d;
								}
	Point4D(Point &p)
								{	x=p.x;
									y=p.y;
									z=p.z;
									w=0;
								}
	Point4D(Point4DL &p)        {   x=p.x;
									y=p.y;
									z=p.z;
									w=p.w;
	                            }
	Point4D operator+(Point4D p)	{ return Point4D(x+p.x,y+p.y,z+p.z,w+p.w);}
	Point4D operator+(double s)		{ return Point4D(x+s,y+s,z+s,w+s);}
	Point4D operator-(Point4D p)	{ return Point4D(x-p.x,y-p.y,z-p.z,w-p.w);}
	Point4D operator-(double s)		{ return Point4D(x-s,y-s,z-s,w-s);}
	Point4D operator*(Point4D p)	{ return Point4D(x*p.x,y*p.y,z*p.z,w*p.w);}
	Point4D operator*(double s)		{ return Point4D(x*s,y*s,z*s,w*s);}
	int operator==(Point4D&p)		{ return DEQ(x,p.x)&&DEQ(y,p.y)&&DEQ(z,p.z)&&DEQ(w,p.w);}
	//int operator!=(Point4D&p)		{ return !(*this==p);}
	int operator!=(Point4D&p)		{ return (!DEQ(x,p.x)||!DEQ(y,p.y)||!DEQ(z,p.z)||!DEQ(w,p.w));}
	double operator[](int i)		{ return (&x)[i];}
	double length()					{ return sqrt(x*x+y*y+z*z+w*w);}
	double magnitude()				{ return x*x+y*y+z*z+w*w;}
	double distance(Point4D p)		{ Point4D tmp=p-*this;
								  		return tmp.length();
									}
	Point4D normalize()           	{ return (*this)*(1.0/length());}
	Point4D rectangular();
	double *values()                { return &x;}
	Point4D absval()                { return Point4D(::fabs(x),::fabs(y),::fabs(z),::fabs(w));}
	double maxval()                 { double f=x>y?x:y;f=f>z?f:z;return f>w?f:w;}
	double minval()                 { double f=x<y?x:y;f=f<z?f:z;return f<w?f:w;}
	Point4D floor()                 { return Point4D(::floor(x),::floor(y),::floor(z),::floor(w));}
	Point4D fract()                 { Point4D p=floor(); return *this-p;}

	double dot(Point4D&p)			{ return (x*p.x+y*p.y+z*p.z+w*p.w);}
	Point toPoint()                 { return Point(x,x,z);}	
	void clear()					{x=y=z=w=0;}
	void print();
	friend std::ostream& operator<<(std::ostream& os, const Point4D& b);
};

class LTheta
{
	uint l;
public:
	static uint MAXBIT;
	static uint MAX;
	static uint MASK;
	static double INV_MAX;
	static double SCALE;
	static double INV_SCALE;
	LTheta()				{   l=0;}
	LTheta(double a)		{	set(a);}
	LTheta(uint a)			{	set(a);}

	void set(double a)		{	l=(uint)(SCALE*a);}
	void set(uint a)		{	l=a;}
	double dvalue()			{	return INV_SCALE*l;}
	double dvalue(uint x)	{	return INV_SCALE*x;}
	double norm()			{	return INV_MAX*l;}
	uint lvalue()			{	return l;}
	uint lvalue(double x)	{	return l=(uint)(SCALE*x);}

	LTheta operator+(LTheta&p)	{ return LTheta(MASK&(l+p.l));}
	LTheta operator-(LTheta&p)	{ return LTheta(MASK&(l-p.l));}
	double operator+(double p)	{ return INV_SCALE*(MASK&(l+(uint)(SCALE*p)));}
	double operator-(double p)	{ return INV_SCALE*(MASK&(l-(uint)(SCALE*p)));}
	int operator<(LTheta&p)		{ return (l<p.l);}
	int operator<=(LTheta&p)	{ return (l<=p.l);}
	int operator>(LTheta&p)		{ return (l>p.l);}
	int operator>=(LTheta&p)	{ return (l>=p.l);}
	double span(LTheta &t)
	{
	    double d1=l*INV_MAX;
	    if(d1>0.5) d1=1-d1;
	    double d2=t.l*INV_MAX;
	    if(d2>0.5) d2=1-d2;
	    return 2*fabs(d1-d2);
	}
	friend class LPoint;
};

class LPhi
{
	uint l;
public:
	static uint MAXBIT;
	static uint MAX;  // must be 1/2 MAX
	static uint MASK;
	static double INV_MAX;
	static double SCALE;
	static double INV_SCALE;

	LPhi()					{   l=0;}
	LPhi(double a)			{	set(a);}
	LPhi(uint a)			{	set(a);}

	void set(double a)		{	l=(uint)(SCALE*(a+90));}
	void set(uint a)		{	l=a;}
	double dvalue()			{	return INV_SCALE*l-90.0;}
	double norm()			{	return INV_MAX*l;}
	double dvalue(uint x)	{	return INV_SCALE*x-90.0;}
	uint lvalue()			{	return l;}
	uint lvalue(double a)	{	return (uint)(SCALE*(a+90));}
	double operator+(double p)  {
									uint ll=l+(uint)(SCALE*(p));
									ll = ll>MAX?MAX-ll:ll;
									return INV_SCALE*ll-90;
								}
	double operator-(double p) {
									uint ll=l-(uint)(SCALE*(p));
									ll = ll>MAX?MAX-ll:ll;
									return INV_SCALE*ll-90;
								}
	int operator<(LPhi&p)		{ return (l<p.l);}
	int operator<=(LPhi&p)		{ return (l<=p.l);}
	int operator>(LPhi&p)		{ return (l>p.l);}
	int operator>=(LPhi&p)		{ return (l>=p.l);}
	double span(LPhi &p)
	{
	    return fabs(l*INV_MAX-p.l*INV_MAX);
	}
	friend class LPoint;
};

class LRad
{
	uint l;
public:
	static uint MAX;
	static uint MASK;
	static double INV_MAX;
	static double SCALE;
	static double INV_SCALE;

	LRad()					{   l=0;}
	LRad(double a)			{	set(a);}
	LRad(uint a)			{	set(a);}
	void set(double a)		{	l=(uint)(SCALE*a);}
	void set(uint a)		{	l=MASK&a;}
	double norm()			{	return INV_MAX*l;}
	double dvalue()			{	return INV_SCALE*l;}
	double dvalue(int x)	{	return INV_SCALE*x;}
	uint lvalue()			{	return l;}
	uint lvalue(double a)	{	return l=(uint)(SCALE*a);}

	friend class LPoint;
};

class LPoint
{
public:
	LTheta t;
	LPhi   p;
	LRad   r;

	LPoint() {  }
	LPoint(double a, double b, double c) : t(a), p(b), r(c) {}
	LPoint(uint a, uint b, double c) : t(a), p(b), r(c) {}

	void set_theta(double a)	{	t.set(a);}
	void set_phi(double a)		{	p.set(a);}
	void set_radius(double a)	{	r.set(a);}
	void set_theta(uint a)		{   t.set(a);}
	void set_phi(uint a)		{   p.set(a);}
	void set_radius(uint a)	{   r.set(a);}

	uint ltheta()				{ return t.lvalue();}
	uint lphi()				{ return p.lvalue();}
	uint lradius()				{ return r.lvalue();}

	uint ltheta(double x)		{ return t.lvalue(x);}
	uint lphi(double x)		{ return p.lvalue(x);}
	uint lradius(double x)		{ return r.lvalue(x);}

	double theta()				{ return t.dvalue();}
	double phi()				{ return p.dvalue();}
	double radius()				{ return r.dvalue();}

	double theta(int x)		{ return t.dvalue(x);}
	double phi(int x)			{ return p.dvalue(x);}
	double radius(int x)		{ return r.dvalue(x);}

	Point rectangular();
};

class FPoint
{
public:
	float x;
	float y;
	float z;

	FPoint() {  x=y=z=0.0f;}
	FPoint(double a, double b, double c) : x((float)a), y((float)b), z((float)c) {}

	void set_theta(double a)	{	x=(float)a;}
	void set_phi(double a)		{	y=(float)a;}
	void set_radius(double a)	{	z=(float)a;}

	double theta()				{ return x;}
	double phi()				{ return y;}
	double radius()				{ return z;}
	float *values()             { return &x;}
	FPoint operator+(double s)	{ return FPoint(x+s,y+s,z+s);}
	FPoint operator-(double s)	{ return FPoint(x-s,y-s,z-s);}
	FPoint operator*(double s)	{ return FPoint(x*s,y*s,z*s);}
	FPoint operator/(double s)	{ return FPoint(x/s,y/s,z/s);}

};


#endif

