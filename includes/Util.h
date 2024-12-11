// Util.h

#ifndef _UTIL
#define _UTIL

#include "PointClass.h"
#include <math.h>
#include <iostream>
#include <list>

const double lim=1e33;

class Averager {
	double total;
	int aves;
	std::list<double> vals;
public:
	int cnt;
	Averager(int num_aves){
		aves=num_aves;
		total=0;
	}
	void reset(){
		vals.clear();
		cnt=0;
		total=0;
	}
	double getAve(double d){
		if(vals.size()==aves){
			total-=vals.front();
			vals.pop_front();
			cnt++;
		}
		vals.push_back(d);
		total+=d;
		return total/vals.size();
	}
};
class MinMax
{
public:
	double min,max,sum,ss;
	int cnt;
	MinMax()	  			{ reset();}
	void reset()  			{ min=lim; max=-lim;cnt=0;sum=0;ss=0;}
	void eval(double v)     {
		min=v<min?v:min;
		max=v>max?v:max;
		cnt++;
		sum+=v;
		ss+=v*v;
	}
	double ave()             { return cnt?sum/cnt:0;}
	double stdev() {
	    if(!cnt)
	        return 0;
	    double a=ave();
	    return sqrt(ss/cnt-a*a);
	}
	void print(const char *c, double f){
		if(cnt){
			printf("%-10s cnt %d min %2.1f max %2.1f ave %2.1f sd %1.3f\n",
			    c,cnt,f*min,f*max,f*ave(),stdev());
	    }
	}
	void print(const char *c){
		print(c,1.0);
	}
	void print(double f){
		print("minmax",f);
	}
	void print(){ print(1.0);}
};

const int BPOINTS=10;

#define BMIN b[3]
#define BMAX b[5]

class Bounds
{
int _valid;
public:
	Point b[10];
	double zn,zf;
	Bounds()	  			{ reset();}
	int valid() 			{ return _valid;}
	void set_valid(int i)   { _valid=1;}
	void reset()  			{
	    BMIN.x=BMIN.y=BMIN.z=lim;
	    BMAX.x=BMAX.y=BMAX.z=-lim;
	    _valid=0;
	}
	void eval()      {
		eval(b[0]);
		eval(b[1]);
		eval(b[2]);
		eval(b[4]);
		eval(b[6]);
		eval(b[7]);
	}
	void eval(Point p)      {
	    if(p.x<BMIN.x) BMIN.x=p.x;
	    if(p.y<BMIN.y) BMIN.y=p.y;
	    if(p.z<BMIN.z) BMIN.z=p.z;
	    if(p.x>BMAX.x) BMAX.x=p.x;
	    if(p.y>BMAX.y) BMAX.y=p.y;
	    if(p.z>BMAX.z) BMAX.z=p.z;
	    _valid=1;
	}
	int size()              { return BPOINTS;}
	Point bmin() 		    { return BMIN;}
	Point bmax() 		    { return BMAX;}
	Point center()			{ return b[8];}
	Point normal()			{ return b[9];}
	Point *box() 			{ return b;}
	Point *make()      		{
	    b[0].x=BMIN.x; b[0].y=BMAX.y; b[0].z=BMIN.z;
	    b[1].x=BMAX.x; b[1].y=BMAX.y; b[1].z=BMIN.z;
	    b[2].x=BMAX.x; b[2].y=BMIN.y; b[2].z=BMIN.z;
	    b[4].x=BMIN.x; b[4].y=BMAX.y; b[4].z=BMAX.z;
	    b[6].x=BMAX.x; b[6].y=BMIN.y; b[6].z=BMAX.z;
	    b[7].x=BMIN.x; b[7].y=BMIN.y; b[7].z=BMAX.z;
	    Point p1,p2;
	    int i;
	    for(i=0;i<4;i++)
	        p1=p1+b[i];
	    for(i=4;i<8;i++)
	        p2=p2+b[i];
	    b[9]=(p2-p1)/4;
	    b[8]=(p1+p2)/8;
	    b[9]=b[9].normalize();
	    return b;
	}
	double depth(){
	    return b[4].distance(b[0]);
	}
	double height(){
	    return b[0].distance(b[3]);
	}
	double width(){
	    return b[4].distance(b[5]);
	}
	double aspect(){
	    return depth()/width();
	}
	void scale(double s){
	    for(int i=0;i<10;i++)
	        b[i]=b[i]*s;
	}
	Point heightEdge(){
	    return b[3]-b[0];
	}
	Point widthEdge(){
	    return b[5]-b[4];
	}
	Point depthEdge(){
	    return b[4]-b[0];
	}
	double extent(){
	    double d=depth();
	    double d1=width();
	    d=d1>d?d1:d;
	    d1=height();
	    d=d1>d?d1:d;
	    return d;
	}
	void dump(){
	    printf("  zn  %-12g zf %-12g ex %g\n",zn,zf,extent());
	    printf("   h  %-12g  w %-12g  d %g\n",height(),width(),depth());
#ifdef PRINT_ALL
	    printf("b[0]  %-12g    %-12g    %g\n",b[0].x,b[0].y,b[0].z);
	    printf("b[1]  %-12g    %-12g    %g\n",b[1].x,b[1].y,b[1].z);
	    printf("b[2]  %-12g    %-12g    %g\n",b[2].x,b[2].y,b[2].z);
	    printf("b[3]  %-12g    %-12g    %g\n",b[3].x,b[3].y,b[3].z);
	    printf("b[4]  %-12g    %-12g    %g\n",b[4].x,b[4].y,b[4].z);
	    printf("b[5]  %-12g    %-12g    %g\n",b[5].x,b[5].y,b[5].z);
	    printf("b[6]  %-12g    %-12g    %g\n",b[6].x,b[6].y,b[6].z);
	    printf("b[7]  %-12g    %-12g    %g\n",b[7].x,b[7].y,b[7].z);
	    printf("cntr  %-12g    %-12g    %g\n",b[8].x,b[8].y,b[8].z);
	    printf("norm  %-12g    %-12g    %g\n",b[9].x,b[9].y,b[9].z);
#endif
	}
};

inline double lerp(double x, double xmin, double xmax, double ymin, double ymax){
	x=x<xmin?xmin:x;
	x=x>xmax?xmax:x;
	double a=(ymax-ymin)/(xmax-xmin);
	double b=ymin-a*xmin;
	double y=a*x+b;
	return y;
}

inline void d2f(double doubleValue, float  &floatHigh, float &floatLow, double scale) {
    if (doubleValue >=  0.0){
        double doubleHigh = floor(doubleValue / scale) * scale;
        floatHigh = (float)doubleHigh;
        floatLow = (float)(doubleValue - doubleHigh);
    }
    else {
        double doubleHigh = floor(-doubleValue / scale) * scale;
        floatHigh = (float)-doubleHigh;
        floatLow = (float)(doubleValue + doubleHigh);
    }
}


inline std::string gets(char *s[], int n, double r) {
	int nt=(int)(n*r);
	nt=nt>n-1?n-1:nt;
	return std::string(s[nt]);
}


#endif

