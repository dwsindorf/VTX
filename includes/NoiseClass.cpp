#include "NoiseClass.h"
#include "NoiseFuncs.h"

#include "ColorClass.h"
#include "Perlin.h"
#include "Util.h"
#include <ctime>
#include <math.h>


//#define OLDPERLIN
//#define NOISE_CACHE
//#define LONGLONG // slower but allows freqs>2e9

//#define DEBUG_NOISE_CACHE
#define DEBUG_NOISE_EVAL

//#define DEBUG_FACTORS
//#define DEBUG_MINMAX


int noise_hits=0;
int noise_misses=0;
int noise_visits=0;
int gradient_visits=0;
double gradient_time=0;
double gradient_time_per_octave=0;

Noise TheNoise;
int	  RandSeed=1;

extern double Height;

double limits_time=0;
const double delta=1e-4;

static int init_flag=1;

int perm[PERMSIZE];
double rands[PERMSIZE];
int lastn=0;

#define MAXSTATES 16

static int state=0;
static int domain=0;
static double norm_value,norm_offset;

typedef struct State {
	double vec[4];
	int   status;
	double values[NOISESIZE];
}
 State;

#define ROFF0 0.0000
#define ROFF1 1.2340
#define ROFF2 0.7124
#define ROFF3 2.3124
#define ROFF4 3.1516
#define ROFF5 1.7916

double Noise::offsets[OFFSETS]={ROFF0,ROFF1,ROFF2,ROFF3,ROFF4,ROFF5};

static State states[MAXSTATES][OFFSETS];
MinMax nvals,norms;

#define VALUE(i)    states[state][domain].values[i]
#define STATUS      states[state][domain].status
#define POINT       states[state][domain].vec
#define P0          states[state][0].vec
#define INIT_STATE  domain=0; \
					states[state][0].status=0; \
					states[state][1].status=0; \
					states[state][2].status=0; \
					states[state][3].status=0; \
					states[state][4].status=0; \
					states[state][5].status=0;

#define NPUSH		state++
#define NPOP		state--

#ifdef LONGLONG
const double MAX_NOISE_FREQ=2e12;
#else
const double MAX_NOISE_FREQ=2e9;
#endif

static int idum=1;
static int iff=0;
const double rmax=32767;
const double invrmax=1/rmax;

static int s_seed=1;

//-------------------------------------------------------------
// Srand()	    set portable random number generator seed
//-------------------------------------------------------------
void setRandSeed(int seed)
{
	idum=seed;
	iff=0;
}
//-------------------------------------------------------------
// getRandSeed()	    return a non-psuedo random value
//-------------------------------------------------------------
double getRandValue()
{
	time_t ttime = std::time(0);
	tm *local_time = localtime(&ttime);
	double seed=Rand()+local_time->tm_min/60.0+local_time->tm_sec/60.0;
	return seed;
}

const int M  = 714025;
const int IA = 1366;
const int IC = 150889;

//-------------------------------------------------------------
// Rand()	portable random number generator
//          values range from 0.0 to 1.0
//-------------------------------------------------------------
double Rand()
{
	static double iy,ir[98];

	int j;

	if(iff==0){
		iff=1;
		if((idum=(IC-idum)%M) <0)
			idum=-idum;
		for(j=1;j<=97;j++) {
			idum=(IA*idum+IC)%M;
			ir[j]=idum;
		}
		idum=(IA*idum+IC)%M;
		iy=idum;
	}
	j=(int)(1+97.0*iy/M);
	iy=ir[j];
	idum=(IA*idum+IC)%M;
	ir[j]=idum;
	return iy/M;
}

//-------------------------------------------------------------
// SRand()	portable random number generator
//          values range from -1.0 to 1.0
//-------------------------------------------------------------
double SRand()
{
	return 2*(Rand()-0.5);
}

//-------------------------------------------------------------
// IntRand()	portable random number generator value
//-------------------------------------------------------------
int IntRand()
{
	return (int)(rmax*Rand());
}

//-------------------------------------------------------------
//RandSum()	return sum of octaves (random noise)
//-------------------------------------------------------------
static double RandSum(int nargs, double *args)
{
	static double iy,ir[98];
	static int flag=0,id=1;

	double result=0,val,bias;
	int start=(int)args[0];
	int orders=(int)args[1];
	unsigned int i,n;

	n=start+orders;

	bias=nargs>2?args[2]:1.0;

	if(n<=0)
		return 0;
	if(flag==0){
		flag=1;
		if((id=(IC-id)%M) <0)
			id=-id;
		for(i=1;i<=97;i++) {
			id=(IA*id+IC)%M;
			ir[i]=id;
		}
		id=(IA*id+IC)%M;
		iy=id;
	}

	for(i=start;i<n; i++){
		val=ir[(int)(1+97.0*i/M)]/M;
		if(i%2)
			result += bias*val;
		else
			result += val;
	}
	return result;
}

//-------------------------------------------------------------
// Random()	return random number
//-------------------------------------------------------------
double Random(double x, double y, double z,double t)
{
	int n;

	n=PERM(x);
	n=PERM(y+n);
	n=PERM(z+n);
	lastn=n;
	n=PERM(t+n);
	return rands[n];
}

//-------------------------------------------------------------
// Random()	return random number
//-------------------------------------------------------------
double Random(double x, double y, double z)
{
	int n;

	n=PERM(x);
	n=PERM(y+n);
	n=PERM(z+n);
	lastn=n;
	return rands[n];
}

//-------------------------------------------------------------
// Random()	return random number
//-------------------------------------------------------------
Point Random(Point pnt)
{
	int n;
	Point p=pnt*43758.5453;
	p.x=PERM(p.x);
	p.y=PERM(p.y);
	p.z=PERM(p.z);
	return p;
}

//-------------------------------------------------------------
// Random()	return random number
//-------------------------------------------------------------
double Random(double x, double y)
{
	int n;

	n=PERM(y);
	n=PERM(x+n);
	lastn=n;
	return rands[n];
}

//-------------------------------------------------------------
// Random()	return random number
//-------------------------------------------------------------
double Random(double x)
{
	return rands[PERM(x+lastn)];
}
//-------------------------------------------------------------
// spline()	   spine curve function
//-------------------------------------------------------------
Color  cspline(double x, int n, Color *colors)
{
    int i,j;
    Color c;
    double *knots;
    double v;
    MALLOC(n,double,knots);
    for(i=0;i<4;i++){
    	for(j=0;j<n;j++){
    	    knots[j]=colors[j][i];
    	}
    	v=spline(x,n,knots);
    	c(i,v);
    }
    ::free(knots);
    return c;
}

//-------------------------------------------------------------
// spline()	   spine curve function
//-------------------------------------------------------------
double  spline(double x, int n, double *v)
{
	double c[4];
	int nspans,span;
	double *base,*knot;

	static double cr[4][4]={
		{-0.5, 1.5, -1.5, 0.5},
		{1.0, -2.5, 2.0, -0.5},
		{-0.5, 0.0, 0.5, 0.0},
		{0.0, 1.0, 0.0, 0.0}};
	x=clamp(x,0,1);
	switch(n){
	case 0:
	    return 0;
	case 1:
	    return v[0];
	case 2:
	    return x*v[1]+(1-x)*v[0];
	case 3:
	    c[2]=v[0];  					// c
	    c[1]=2*v[2]+2*v[0]-4*v[1];  	// b
	    c[0]=4*v[1]-3*v[0]-v[2];  	    // b
	    return c[0]*x+c[1]*x*x+c[2];    // ax+bx*x+c

	default:
	    MALLOC(n+4,double,base);        // need to double-up first and
		base[0]=v[0];					// last knot to constrain curve to
		for(int i=0;i<n;i++)            // pass through end points
		    base[i+1]=v[i];
		base[n+1]=v[n-1];

		knot=base;
		n+=2;
		nspans=n-3;
		x*=nspans;
		span=(int)x;
		if(span>=n-4)
			span=n-4;
		x-=span;
		knot+=span;
		c[3]=cr[0][0]*knot[0]+cr[0][1]*knot[1]+cr[0][2]*knot[2]+cr[0][3]*knot[3];
		c[2]=cr[1][0]*knot[0]+cr[1][1]*knot[1]+cr[1][2]*knot[2]+cr[1][3]*knot[3];
		c[1]=cr[2][0]*knot[0]                 +cr[2][2]*knot[2];
		c[0]=                 cr[3][1]*knot[1];
		x=((c[3]*x+c[2])*x+c[1])*x+c[0];
		::free(base);
		return x;
	}
}


//************************************************************
// Class NoiseFactor
//************************************************************
class NoiseFactor {
	double 			H;
	double 			L;
public:
    int             maxorder;
	double 			exps[NOISESIZE];
	double 			freqs[NOISESIZE];
	bool equal(double h, double l) { return (H==h) && (l==L);}
	NoiseFactor(double h, double l);

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// class NoiseFactor
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
NoiseFactor::NoiseFactor(double h, double l)
{
    H=h;
    L=l;
    int i;
    double f=1.0;
	//printf("H %g L %g\n",H,L);
	for (i=0; i<NOISESIZE; i++) {
	    //if(f>MAX_NOISE_FREQ)
	    //    break;
		freqs[i]=f;
		exps[i] = pow( f, -H );
		//printf("%-2d %-12g %-12g %g\n",i,freqs[i],exps[i],exps[i]*freqs[i]);
		f *= L;
	}
	if(i==NOISESIZE)
	    i=NOISESIZE-1;
	maxorder=i;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// class Noise
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
NoiseFactor *Noise::factors[MAXNFACTORS];
//double Noise::dflt_L=1.9873215;
double Noise::dflt_L=2.0;
double Noise::dflt_H=0.5;
double Noise::min_order=0;
double Noise::max_order=32;
double Noise::rseed=0;
double Noise::offset=0.5;
double Noise::scale=0.5;
double Noise::aspect=1;
double Noise::theta=0;
double Noise::phi=0;
double Noise::shift=0;
double Noise::start_time=0;
int Noise::nfact=0;
int Noise::_mode=0;
int Noise::flags=0;
int Noise::_ntype=GRADIENT;

Noise::Noise()
{
	set_mode(EVAL);
	init();
}
Noise::~Noise()
{
	for(int i=0;i<MAXNFACTORS;i++){
	    if(factors[i]){
	        delete factors[i];
	        factors[i]=0;
	    }
    }
}
//-------------------------------------------------------------
// Noise::set()	set 4D point
//-------------------------------------------------------------
void Noise::set(double a,double b, double c, double d)
{
	INIT_STATE;
	POINT[0]=a;
	POINT[1]=b;
	POINT[2]=c;
	POINT[3]=d;
}

//-------------------------------------------------------------
// Noise::set()	set 3D point
//-------------------------------------------------------------
void Noise::set(Point &p)
{
	INIT_STATE;
	POINT[0]=p.x;
	POINT[1]=p.y;
	POINT[2]=p.z;
	POINT[3]=0;
}

//-------------------------------------------------------------
// Noise::set()	set 4D point
//-------------------------------------------------------------
void Noise::set(Point4D &p)
{
	INIT_STATE;
	POINT[0]=p.x;
	POINT[1]=p.y;
	POINT[2]=p.z;
	POINT[3]=p.w;
}

//-------------------------------------------------------------
// Noise::push() push 3D point
//-------------------------------------------------------------
void Noise::push(Point &p)
{
	NPUSH;
	INIT_STATE;
	POINT[0]=p.x;
	POINT[1]=p.y;
	POINT[2]=p.z;
	POINT[3]=0;
}

//-------------------------------------------------------------
// Noise::push() push 4D  point
//-------------------------------------------------------------
void Noise::push(Point4D &p)
{
	NPUSH;
	INIT_STATE;
	POINT[0]=p.x;
	POINT[1]=p.y;
	POINT[2]=p.z;
	POINT[3]=p.w;
}

//-------------------------------------------------------------
// Noise::pop()	pop point
//-------------------------------------------------------------
void Noise::pop()
{
	NPOP;
	domain=0;
}

//-------------------------------------------------------------
// void set_factors(double h,double l) set current noise factors
//-------------------------------------------------------------
int Noise::set_factors(double h,double l)
{
    nfact=NFACTORMASK&(int)(h*101+l*23);
    if(!factors[nfact] || !factors[nfact]->equal(h,l)){
        if(factors[nfact])
            delete factors[nfact];
        factors[nfact]=new NoiseFactor(h,l);
#ifdef DEBUG_FACTORS
        NoiseFactor *nf=factors[nfact];
        int maxl=nf->maxorder;
        double maxf=nf->freqs[maxl-1];
        printf("new nfact H %g L %g id %-4d maxl %-2d maxf %g\n",h,l,nfact,maxl,maxf);
#endif
    }
    return factors[nfact]->maxorder;
}

//-------------------------------------------------------------
// Point4D get_point() return current noise vector
//-------------------------------------------------------------
Point4D Noise::get_point()
{
   Point4D p=Point4D(
    	POINT[0],
    	POINT[1],
    	POINT[2],
    	POINT[3]);
   return p;
}
//-------------------------------------------------------------
//void clearCache()
//-------------------------------------------------------------
void  Noise::resetLimits(){

}
//-------------------------------------------------------------
//void reset()
//-------------------------------------------------------------
void  Noise::resetStats(){
	noise_hits=0;
	noise_misses=0;
	noise_visits=0;
	gradient_visits=0;
	gradient_time=0;
	gradient_time_per_octave=0;
	start_time=clock();
}
//-------------------------------------------------------------
//void showStats()
//-------------------------------------------------------------
void  Noise::showStats(){
	char buff[256];
#ifdef DEBUG_NOISE_EVAL
	double build_time=(double)(clock() - start_time)/CLOCKS_PER_SEC;

	int n=gradient_visits;
	double gtm=gradient_time;
	double gtmp=gradient_time_per_octave;
	char tstr[256];
	if(noise3D())
		sprintf(tstr,"3D ");
	else
		sprintf(tstr,"4D ");
	switch(ntype()){
	    case VORONOI:
	    	strcat(tstr,"Voronoi");
	    	break;
	    case SIMPLEX:
	    	strcat(tstr,"Simplex");
	    	break;
	    case GRADIENT:
	    default:
	    	strcat(tstr,"Perlin");
	}
	sprintf(buff,"%s MultiNoise calls:%d time total:%2.1f s per call:%2.1f u per octave:%2.1f u",
			tstr,n,build_time,1e6*gtm/n,1e6*gtmp/n);
	cout<<buff<<endl;
#endif
#ifdef DEBUG_NOISE_CACHE
	sprintf(buff,"Noise calls:%d cache hits:%d percent:%2.1f",
			noise_visits,noise_hits,(100.0*noise_hits)/noise_visits);
	cout<<buff<<endl;

#endif
}
//-------------------------------------------------------------
// double factor(int i) return noise value
//-------------------------------------------------------------
double Noise::factor(int i)
{
	return factors[nfact]->exps[i];
}

//-------------------------------------------------------------
// double amplitude(int i) return noise amlitude
//-------------------------------------------------------------
double Noise::amplitude(int i)
{
	//return factors[nfact]->exps[i]*factors[nfact]->freqs[i];
	return factors[nfact]->exps[i];
}

//-------------------------------------------------------------
// double frequency(int i) return noise frequency
//-------------------------------------------------------------
double Noise::frequency(int i)
{
	return factors[nfact]->freqs[i];
}

//-------------------------------------------------------------
// Noise::set_limits()	set limits values
//-------------------------------------------------------------
void Noise::set_limits(double v1, double v2)
{
    min_order=v1;
    max_order=v2;
}

//-------------------------------------------------------------
// Noise::set_limits()	return limits values
//-------------------------------------------------------------
void Noise::get_limits(double &v1, double &v2)
{
    v1=min_order;
    v2=max_order;
}

//-------------------------------------------------------------
// Noise::limits()	return maximum value
//-------------------------------------------------------------
void Noise::get_minmax(double &v1, double &v2,int type,int n, double *args)
{
#ifdef DEBUG_MINMAX
	double start=clock();
#endif
	const int VSTEPS=200;
	const int VCYCS=3;
	const double VFACT=PI;
    const double dv=2.0/VSTEPS;

    int oldmode=mode();
    set_mode(MINMAX);
    v2=-lim;
    v1=lim;
    int i,j;
    double val=0;
    for(i=0;i<VCYCS;i++){
   		norm_value=0.0;
		norm_offset=VFACT*i;
		for(j=0;j<VSTEPS;j++){
	        val=eval(type,n,args);
	    	v2=val>v2?val:v2;
	    	v1=val<v1?val:v1;
			norm_value+=dv;
		}
    }
    double ma,mb;
    if(type & UNS){ // scale min to max 0 to 2
		ma=1.0/(v2-v1);
		mb=v1*ma;
    }
    else{  // scale min to max -1 to 1
		ma=2.0/(v2-v1);
		mb=1-v2*ma; // this should be correct
		//mb=v1*ma+1;
    }
    double exp=0;
    if(type & SCALE){
        exp=amplitude((int)args[0]);
        ma*=exp;
        mb*=exp;
    }
#ifdef DEBUG_MINMAX
    double tm=(double)(clock() - start)/CLOCKS_PER_SEC;
    limits_time+=tm;
    cout<<"min:"<<v1<<" max:"<<v2<<" exp:"<<exp<<" ma:"<<ma<<" mb:"<<mb<<endl;
#endif
    v1=ma;
    v2=mb;
	set_mode(oldmode);
}

//-------------------------------------------------------------
// Noise::init()	make constant arrays
//-------------------------------------------------------------
void Noise::init()
{
	int i,j,k;
	if(init_flag){
		state=0;
		set(0,0,0,0);

		for(i=0;i<MAXNFACTORS;i++)
		    factors[i]=0;
		set_factors(dflt_H,dflt_L);

		int mod=PERMSIZE>>1;

		for(i=0;i<PERMSIZE;i++){
			perm[i]=i;
			rands[i]=(Rand()-0.5);
		}

		while (--i) {
			k = perm[i];
			perm[i] = perm[j = IntRand() % mod];
			perm[j] = k;
		}
		//printf("n %d\n",nfact);
		//init_noise(RandSeed);
		init_flag=0;
	}
}

//-------------------------------------------------------------
// double value(int i) return noise value
//-------------------------------------------------------------
double Noise::value(int i)
{
	if(!init_flag)
		init();
	double freq;
	int type=ntype();

	if(mode()==MINMAX){
	    freq=factors[nfact]->freqs[i];
	    double fv=norm_value*freq+norm_offset;
	    switch(type){
	    case VORONOI:
	    	return VoronoiMinMax(fv);
	    case SIMPLEX:
	    	return SimplexMinMax(fv);
	    default:
	        return NoiseMinMax(fv);
	    }
	}
	int m=1<<i;
	double v[4];
#ifdef DEBUG_NOISE_CACHE
	noise_visits++;
#endif
#ifdef NOISE_CACHE
	if((m & STATUS)==0){
#endif
		STATUS|=m;
#ifdef NOISE_CACHE
		noise_misses++;
#endif
		freq=factors[nfact]->freqs[i];
		v[0]=POINT[0]*freq;
		v[1]=POINT[1]*freq;
		v[2]=POINT[2]*freq;
		if(noise4D()){
			v[3]=POINT[3]*freq;
			switch(type){
			case VORONOI:
				VALUE(i)=Voronoi4D(v);
				break;
			case SIMPLEX:
				VALUE(i)=Simplex4D(v);
				break;
			case GRADIENT:
			default:
				VALUE(i)=Noise4D(v);
			}
		}
		else{
			switch(type){
			case VORONOI:
				VALUE(i)=Voronoi3D(v);
				break;
			case SIMPLEX:
				VALUE(i)=Simplex3D(v);
				break;
			case GRADIENT:
			default:
				VALUE(i)=Noise3D(v);
			}
		}
#ifdef NOISE_CACHE
    }
#ifdef DEBUG_NOISE_CACHE
	else
	    noise_hits++;
#endif
#endif
	return VALUE(i);
}

//-------------------------------------------------------------
// Noise::eval()	return value
//-------------------------------------------------------------
double Noise::eval(int type,int n, double *args)
{
    domain=type & ROFF;
    int nt=ntype();
    if(domain || rseed){
		shift=offsets[domain]+rseed;
	    POINT[0]=P0[0]+shift;
	    POINT[1]=P0[1]+shift;
	    POINT[2]=P0[2]+shift;
	    POINT[3]=P0[3]+shift;
	}
	else
		shift=rseed;
 	double val=0;
 	set_ntype(type&NTYPES);
	val=multinoise(type,n,args);
    if(type & ROFF)
    	domain=0;
	return val;
}

//-------------------------------------------------------------
// Noise::distort()	distort point coordinates
//-------------------------------------------------------------
void Noise::distort(int type,double distortion){
    double t=RPD*theta;
    double p=RPD*phi;
    double r=scale;
    double a=r;
	double off=scale+shift;
	double x=POINT[0]-off;
	double y=POINT[1]-off;
	double z=POINT[2]-off;
	double angle,rad,s,c;
	Point pt;

	switch(type){
	case NTWIST:
		pt=Point(x,z,y);
		pt=pt.normalize();
		rad=(pt.x*pt.x+pt.y*pt.y);
		angle=rad*distortion*2*PI;
		s=sin(angle);
		c=cos(angle);
		x=(pt.x*c-pt.y*s);
		y=(pt.x*s+pt.y*c);
		z=pt.z;
		break;
	case NTHETA:
		p=RPD*P180(phi);
		t=RPD*P360(theta+DPR*distortion);
		if(noise3D())
			a=r*cos(p);
		x=-a*cos(t);
		z=a*sin(t);
		y=r*sin(p);
		break;
	case NPHI:
		t=RPD*P360(theta);
		p=RPD*P180(phi+DPR*distortion);
		if(noise3D())
			a=r*cos(p);
		x=-a*cos(t);
		z=a*sin(t);
		y=r*sin(p);
		break;
	case NRADIUS:
		a=distortion;
		x*=a;
		y*=a;
		z*=a;
		//off+=5;
		break;
	case NY:
		y*=distortion;
		off+=distortion;
		break;
	case NXY:
		x*=distortion;
		y*=distortion;
		off+=distortion;
		break;
	}

	if(noise3D()){
	    Point pt=Point(x,y,z);
	    pt=pt+off;
	    push(pt);
	}
	else{
	    Point4D pt=Point4D(x,y,z,-a*cos(p));
	    pt=pt+off;
	    push(pt);
	}
}

//-------------------------------------------------------------
// Noise::gradient()	return sum of octaves (fBm noise)
//-------------------------------------------------------------
double Noise::multinoise(int type, int nargs, double *args)
{
	const double VMAX=0.5;
	if(!init_flag)
		init();
#ifdef DEBUG_NOISE_EVAL
	double start_time=clock();
#endif

	double H=dflt_H;
	double L=dflt_L;

    double result,signal,weight,bias,gain,val=0,fact;

	bias=nargs>2?args[2]:1.0;  // homogeneity

    if(nargs>3)
    	H=args[3];
    if(nargs>4)
    	L=args[4];

	int max=set_factors(H,L);

	int start=(int)args[0];
	double frac1=args[0]-start;
	int orders=(int)(args[1]+frac1);
	double frac2=args[1]-orders;
	if(start>max)
		start=max;
	if(orders+start>max)
	    orders=max-start;
	int end=start+orders;
    int i=start;

	if(frac2>0)
		end++;
	double last_val=0;

	if(mode()==GETLIMS){
	    double mn=start;
	    double mx=start+orders+frac2;
	    //printf("gradient %g %g\n",mn,mx);
	    min_order=mn<min_order?mn:min_order;
	    max_order=mx>max_order?mx:max_order;
	}

    result=0;
    gain=1-bias;
    weight=1;

    bool nabs=type & NABS;
	double clip=nargs>6?args[6]:1.0;
	clip*=VMAX;
	double x,rmin=0;
    if(nargs>7){
    	rmin=args[7]*VMAX;
    	if(!nabs)
    		rmin *=2;
    }
	for(i=start;i<end;i++){
		signal=value(i);
		fact=factor(i);
		if(frac1>0){
			signal=signal*(1-frac1)+value(i+1)*frac1;
			fact=fact*(1-frac1)+factor(i+1)*frac1;
		}
		signal=signal>clip?clip:signal;
		if(nabs){
		    if(signal<0)
		        signal=-signal;
		    if(rmin && signal<rmin){
		    	x=signal/rmin;
			    signal=rmin*GCURVE1(x);
		    }
		    signal=VMAX-signal;
		    signal*=2;
		}
		else{
		    signal=VMAX+signal;
		    if(rmin && signal<rmin){
		    	x=signal/rmin;
			    signal=rmin*GCURVE1(x);
		    }
		}
		if(signal<0)
		    signal=0;
		last_val=result;
	    val=signal*weight*fact;
		result += val;
		if(weight<=0)
		    break;

		weight*=signal*gain+bias;
		weight=clamp(weight,0,1);
	}
	if(frac2>0.0)
		result=last_val+frac2*val;
	if(type & SQR){
		if(type & UNS)
			result=result*result;
		else
			result=result*fabs(result);
	}
	if(type & NEG)
	    result=-result;
#ifdef DEBUG_NOISE_EVAL
    double tm=(double)(clock() - start_time)/CLOCKS_PER_SEC;
    gradient_time += tm;
    gradient_time_per_octave += tm/(i-start);
    gradient_visits++;
#endif
	return result;
}


double Noise::NoiseMinMax(double v) {
	return Perlin::minmax(v);
}
double Noise::Noise1D(double v) {
	return Perlin::noise1(v);
}
double Noise::Noise2D(double *v){
	return Perlin::noise2(v);}
double Noise::Noise3D(double *v){
	return Perlin::noise3(v);
}
double Noise::Noise4D(double *v){
	return Perlin::noise4(v);
}

//-------------------------------------------------------------
// Noise::Voronoi1D() used only to determine min max values of function
//-------------------------------------------------------------
double Noise::VoronoiMinMax(double x){
	return Voronoi::minmax(x);
}

//-------------------------------------------------------------
// Noise::Voronoi1D() used only to determine min max values of function
//-------------------------------------------------------------
double Noise::Voronoi1D(double x){
	return Voronoi::noise(x);
}

//-------------------------------------------------------------
// Noise::Voronoi2D() 2d Voronoi noise
//-------------------------------------------------------------
double Noise::Voronoi2D(double *d) {
	return Voronoi::noise(d[0],d[1]);
}

//-------------------------------------------------------------
// Noise::Voronoi3D() 3d Voronoi noise
//-------------------------------------------------------------
double Noise::Voronoi3D(double *d){
	return Voronoi::noise(d[0],d[1],d[2]);
}

//-------------------------------------------------------------
// Noise::Voronoi4D() 4d Voronoi noise
//-------------------------------------------------------------
double Noise::Voronoi4D(double *d){
	return Voronoi::noise(d[0],d[1],d[2],d[3]);
}

//-------------------------------------------------------------
// Noise::SimplexMinMax() used only to determine min max values of function
//-------------------------------------------------------------
double Noise::SimplexMinMax(double x){
	return Simplex::minmax(x);
}

//-------------------------------------------------------------
// Noise::Simplex1D() 1D Simplex noise
//-------------------------------------------------------------
double Noise::Simplex1D(double x){
	return Simplex::noise(x);
}
//-------------------------------------------------------------
// Noise::Simplex2D() 2D Simplex noise
//-------------------------------------------------------------
double Noise::Simplex2D(double *d){
	return Simplex::noise(d[0],d[1]);
}
//-------------------------------------------------------------
// Noise::Simplex3D() 3D Simplex noise
//-------------------------------------------------------------
double Noise::Simplex3D(double *d){
	return Simplex::noise(d[0],d[1],d[2]);
}
//-------------------------------------------------------------
// Noise::Simplex3D() 4D Simplex noise
//-------------------------------------------------------------
double Noise::Simplex4D(double *d){
	return Simplex::noise(d[0],d[1],d[2],d[3]);
}

//-------------------------------------------------------------
// Noise::random()	return sum of octaves (random noise)
//-------------------------------------------------------------
double Noise::random(int options,int nargs, double *args)
{
	return RandSum(nargs, args);
}
