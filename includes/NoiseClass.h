// NoiseClass.h

#ifndef _NOISECLASS
#define _NOISECLASS

#include "PointClass.h"
#include "ListClass.h"
#include "Perlin.h"

#define LARGE_TBL

#ifdef LARGE_TBL
const int PERMSIZE=64*1024*32;
#else
const int PERMSIZE=1024*32;
#endif
//const int PERMSIZE=2048*64;
const int PMASK=PERMSIZE-1;

class Noise;
class Point;
class NoiseFactor;
class Color;

extern Noise TheNoise;

extern double Noise3(Point);
extern double Random(Point);
extern double Random(double);
extern double Random(double,double);
extern double Random(double,double,double);
extern double Random(double,double,double,double);
extern double SRand();
extern double Rand();
extern int IntRand();
extern void setRandSeed(int);
extern double getRandValue();

const double fmx=(double)(1<<28);

#define RANDOM(x)		(Noise3((x)*fmx)+0.5)
#define SRANDOM(x)		(2.0*Noise3((x)*fmx))

extern int perm[];
extern double rands[];
extern int lastn;
#define PERM(x) 	perm[((int)(x))&PMASK]
#define RAND(x) 	(rands[PERM((x)+lastn)])

#define URAND(x) 	(rands[PERM((x)+lastn)]+0.5)
#define LRAND 	    (rands[PERM((lastn++)+lastn)]+0.5)

#define PRAND(p)  (rands[PERM(p.z+PERM(p.y+PERM(p.x)))+lastn])

// 3rd order smoothstep    f'=0 @x=1, x=0
#define SCURVE1(t) (t*t*(3.0 - 2.0*t) )
// 5th order smoothstep    f'=f''=0 @x=1, x=0
#define SCURVE2(t) (t*t*t*(t*t *6.0-t*15.0+10.0) )
// 3rd order polynomial    f''=0 @ x=1 f'=0 @x=0
#define GCURVE1(t) (0.5*t*t*(3-t))

enum {		// distortion opcodes
	NTWIST,
	NTHETA,
	NPHI,
	NRADIUS,
	NY,
	NXY,
	NHT,
};

enum {		// NTypes

	RO0      = 0x00000000, // default
	RO1      = 0x00000001,
	RO2      = 0x00000002,
	RO3      = 0x00000003,
	RO4      = 0x00000004,
	RO5      = 0x00000005,
	ROFF     = 0x00000007,

	GRADIENT = 0x00000100, //default
	VORONOI  = 0x00000200,
	SIMPLEX  = 0x00000300,
	NTYPES   = 0x00000300,

	UNS      = 0x00001000,
	SQR      = 0x00002000,
	NEG      = 0x00004000,
	NABS     = 0x00008000,
	SCALE    = 0x00010000,
	SS       = 0x00020000,
	SS2      = 0x00040000,
	TA       = 0x00080000, // time animation
	V2       = 0x00100000,
	V4       = 0x00200000,
	VR       = 0x00400000,
	NOREC    = 0x01000000,
	NLOD     = 0x02000000,
	NNORM    = 0x04000000,
	FMODE    = V2|V4|VR,    // fractal test method

	FS       = 0x08000000,  // shader noise
	TX       = 0x10000000,  // 3D tex
	BP       = 0x20000000,  // bump map

	T3D      = 0x40000000,  // 3D tex
	GEOM     = 0x80000000,  // geopmetry shader

	NOPTS    = 0xfffff000,
};

inline double smoothstep(double a, double b, double x)
{
	if(x<a) return a;
	if(x>b) return b;
	x=(x-a)/(b-a);
	return (x*x*(3-2*x));
}

inline double smoothstep(double _a, double _b, double x, double _min, double _max)
{
	double a,b,mn,mx;
	if(_b>_a){
		a=_a;b=_b;mn=_min;mx=_max;
	}
	else{
		a=_b;b=_a;mn=_max;mx=_min;
	}
	if(x<a)
		return mn;
	else if(x>b)
		return mx;
	else
		x=(x-a)/(b-a);
	x=x*x*(3-2*x);
	return mn+(mx-mn)*x;
}

inline double rampstep(double a, double b, double x)
{
	if(x<a) return 0;
	if(x>b) return 1;
	x=(x-a)/(b-a);
	return x;
}

inline double rampstep(double _a, double _b, double x, double _min, double _max)
{
	double a,b,mn,mx;
	if(_b>_a){
		a=_a;b=_b;mn=_min;mx=_max;
	}
	else{
		a=_b;b=_a;mn=_max;mx=_min;
	}
	if(x<a)
		return mn;
	else if(x>b)
		return mx;
	else
		x=(x-a)/(b-a);

	return mn+(mx-mn)*x;
}

inline double scal(double x, double mn, double mx)
{
	return mn+(mx-mn)*x;
}

inline double clamp(double x, double l, double m)
{
	if(x<l)
		return l;
	if(x>m)
		return m;
	return x;
}

inline double tmod(double x)
{
	int n=(int)x;
	x-=n;
	if(x<0)
		x+=1;
	return x;
}

inline double umod(double a,double b)
{
	int n=(int)(a/b);
	a-=n*n;
	if(a<0)
		a+=b;
	return a;
}

extern double  spline(double x, int n, double *knot);
extern Color  cspline(double x, int n, Color *knot);


//************************************************************
// Class NoiseSym
//************************************************************
class NoiseSym {
public:
	char			    text[256];
	char *name()		{ return text;}
	int  cmp(char *s)		{ return strcmp(text,s);}
	double vmin,vmax,a,b;
	NoiseSym(char *t,double mn,double mx, double ma, double mb)
	{
	    strcpy(text,t);
	    vmin=mn;vmax=mx;a=ma;b=mb;
	}
};

#define MAXNFACTORS 256
#define NFACTORMASK 255
enum {
   EVAL     = 0x00,
   MINMAX   = 0x01,
   GETLIMS  = 0x02,
   SETLIMS  = 0x03
};
#define NOISESIZE 40
#define OFFSETS 6

//************************************************************
// Class Noise
//************************************************************
class Noise
{
protected:
	static int flags;
	static int _mode;
	static int _ntype;
	static NoiseFactor *factors[MAXNFACTORS];

   enum {
       PT4D     = 0x01
    };
    static void   init();
    static double multinoise(int options, int n, double *s);
    //static double random(int options,int n, double *s);
    static double NoiseMinMax(double v);
    static double Noise1D(double v);
    static double Noise2D(double *v);
    static double Noise3D(double *v);
    static double Noise4D(double *v);
    static double VoronoiMinMax(double v);
    static double Voronoi1D(double x);
    static double Voronoi2D(double *pnt);
    static double Voronoi3D(double *pnt);
    static double Voronoi4D(double *pnt);
    static double SimplexMinMax(double v);
    static double Simplex1D(double x);
    static double Simplex2D(double *pnt);
    static double Simplex3D(double *pnt);
    static double Simplex4D(double *pnt);

public:

    static char *ntypes[];
    static char *nopts[];
    
    static std::string getNtype(double f);
    static std::string getNopts(double f);

    
 	static double dflt_L;
	static double dflt_H;
	static double max_order;
	static double min_order;
	static double offsets[OFFSETS];

	static double offset;
	static double scale;
	static double maxampl;
	static double aspect;
	static double theta;
	static double phi;
	static double shift;
	static double rseed;
	static int nfact;
	static double start_time;


	Noise();
	~Noise();
	static void set3D()  	{ BIT_OFF(flags,PT4D);}
	static void set4D()  	{ BIT_ON(flags,PT4D);}
	static int noise4D()    { return flags&PT4D?1:0;}
	static int noise3D()    { return flags&PT4D?0:1;}

	static void set_mode(int i)	{ _mode=i;}
	static int mode()           { return _mode;}
	static void set_ntype(int i)	{ _ntype=i;}
	static int ntype()           { return _ntype;}

	static void push(Point &p);
	static void push(Point4D &p);
	static void pop();
	static void set(Point &p);
	static void set(Point4D &p);
	static void set(double, double, double ,double);
	static double eval(int options,int n, double *s);
	static double value(int i);
	static double amplitude(int i);
	static double frequency(int i);
	static double factor(int i);
    static Point4D get_point();
    static void set_limits(double v1, double v2);
    static void get_limits(double &v1, double &v2);
    static int set_factors(double h,double l);
    static void get_minmax(double &v1, double &v2,int options,int n, double *s);
    static void distort(int type,double ampl);
    static void resetLimits();
    static void resetStats();
    static void showStats();
};

#endif

