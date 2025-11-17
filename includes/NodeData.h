// NodeData.h

#ifndef _NODEDATA
#define _NODEDATA

#include "GLheaders.h"
#include "TerrainData.h"
#include "TextureClass.h"
#include "MapClass.h"
#include <math.h>

//#define DP_DIMS
#define DPTEXTURES
//---------------------------------------------------------------------

//#define HASH_POINTS
//#define HASH_NORMALS


#define CELLSLOPE(x,v) { \
    static double lastz=0; \
    if(mdcnt>2) { \
	     double dt,dp; \
	     double z1=mapdata[0]->x; \
	     double z2=mapdata[1]->x; \
	     double z3=mapdata[2]->x; \
	     double p1=mapdata[0]->lp.span(mapdata[1]->lp); \
	     double t1=mapdata[0]->lt.span(mapdata[1]->lt); \
	     if(mdcnt==4){ \
		     double z4=mapdata[3]->x; \
		     double s1=mapdata[0]->span(mapdata[1]); \
		     double s2=mapdata[2]->span(mapdata[3]); \
		     if(s1 && s2){  \
		     	dt=fabs(z1-z2)/s1; \
		     	dp=fabs(z3-z4)/s2; \
		     	lastz=sqrt(dt*dt+dp*dp); \
		     } \
	     } \
	     else if(mdcnt==3){ \
		 	 double p2=mapdata[2]->lp.span(mapdata[0]->lp); \
		 	 double t2=mapdata[2]->lt.span(mapdata[0]->lt); \
		     if(p1==0 && t1 && p2){  \
			 	dt=fabs(z2-z1)/t1; \
			 	dp=fabs(z3-0.5*(z1+z2))/p2; \
		     	lastz=sqrt(dt*dt+dp*dp); \
		     } \
		     else if(p1 && t2) { \
			 	dp=fabs(z1-z2)/p1; \
			 	dt=fabs(z3-0.5*(z1+z2))/t2; \
		     	lastz=sqrt(dt*dt+dp*dp); \
		     } \
	     } \
    } \
    v=lastz; \
 }

class MapData;
class MapNode;
class Map;

// global flags


enum {
	GRT       	= 0,    // right group neighbor
	GUP       	= 1,    // up group neighbor
	GLT    		= 2,    // left group neighbor
	GDN    		= 3     // down group neighbor
};

typedef struct idc {
#ifndef BIG_ENDEAN
	GLubyte  blue;
	GLubyte  green;
	GLubyte  red;
	GLubyte  alpha;
#else
	GLubyte  alpha;
	GLubyte  red;
	GLubyte  green;
	GLubyte  blue;
#endif
} idc;

typedef struct llong {
#ifndef BIG_ENDEAN
	uint         low;
	uint		 high;
#else
	uint		 high;
	uint         low;
#endif
} llong;

typedef union fdu {
	double		 d;
    llong        l;
#ifdef __LP64__
    MapData      *p;
#endif
} fdu;


typedef union idu {
	idc			 c;
	int         l;
} idu;

enum {
	D32_LONG,
	D32_FLOAT,
	D32_COLOR,
	D32_PTR
};
typedef union d32 {
	int     l;
    double   d;
	unsigned long    ul;
    MapData *md;
} d32;

typedef struct Hdata {
	void  *adrs;
	Point			point;
}		Hdata;

class HashTable
{
	int debug;
public:
	Hdata *hdata;
	int size;
	int mask;
	static int hits;
	static int visits;
	static int misses;
	HashTable(int n) {
		resize(n);
		debug=0;
	}
	HashTable(int n,int d) {
		resize(n);
		debug=d;
	}
	~HashTable(){
		delete []hdata;
		hdata=0;
	}
	static void reset(){hits=visits=misses=0;}
	void resize(int n){
		if(hdata)
			delete []hdata;
		hdata=new Hdata[n];
		size=n;
		mask=size-1;
		init();
	}
	void init(){
		for(int i=0;i<size;i++)
			hdata[i].adrs=0;
	}
	void remove(int id){
		if(hdata)
			hdata[id & mask].adrs=0;
	}
	void set_debug(int i){  debug=i;}
	Hdata *hash(int id, void *x){
	   int adr=id&mask;
#ifdef DEBUG_HASH
		if(debug){
			visits++;
			if(hdata[adr].adrs==x)
				hits++;
		}
#endif
		return hdata+adr;
	}
};

//************************************************************
// MapData class
//************************************************************

#define ZSIZE  flags.s.dims
#define FSIZE  flags.s.fchnls
#define LSIZE  flags.s.links
#define ESIZE  flags.s.evals
#define MSIZE  flags.s.mdata
#define DSIZE  flags.s.density
#define WSIZE  flags.s.ocean

#define BDSIZE 1
#define TDSIZE 1

#define CSTART LSIZE
#define DSTART LSIZE+flags.s.colors
#define WSTART DSTART+DSIZE
#define ZSTART WSTART+WSIZE
#define FSTART ZSTART+ZSIZE
#define MSTART FSTART+FSIZE
#define ESTART MSTART+MSIZE

#define TSTART ESTART+ESIZE
//
//    mpdata bit map
//  31            24  23            16  15            8  7             0
//  - - - - - - - -   - - - - - - - -   - - - - - - - -  - - - - - - - -
//  B B B B B B B B   T T T T T T T T   T T T T T G G E  E F Y X Z D C L
//  5 5 5 5 5 5 5 5   5 5 5 5 5 5 5 5   4 4 4 4 4 4 4 4  3 3 3 3 2 2 1 0
//
//  B B B B B B B B   T T T T T T T T   T T T G G E E F  F X X Z Z D C L    DB_DIMS
//  5 5 5 5 5 5 5 5   5 5 5 5 5 5 5 5   4 4 4 4 4 4 4 4  3 3 3 3 2 2 1 0
//  __LP64__
//  B B B B B B B B   T T T T T T T T   T T T T G G E E  F Y X Z D C L L
//  5 5 5 5 5 5 5 5   5 5 5 5 5 5 5 5   4 4 4 4 4 4 4 4  3 3 3 3 2 2 1 0
//
//  B B B B B B B B   T T T T T T T T   T T G G E E F F  X X Z Z D C L L   DB_DIMS
//  5 5 5 5 5 5 5 5   5 5 5 5 5 5 5 5   4 4 4 4 4 4 4 4  3 3 3 3 2 2 1 0
typedef struct mpdata {
	unsigned int  type	    : 8;	// type code
	unsigned int  textures	: 1;	// textures (flag)
	unsigned int  bumpmaps	: 1;	// bumpmaps (flag)
	unsigned int  links     : 1;	// link to more MapData (0..1)
	unsigned int  colors	: 1;	// colors (0..1)
	unsigned int  dims	    : 2;	// z, x, y (0..2)
	unsigned int  density	: 1;	// fog/density (0..1)
	unsigned int  ocean	    : 1;	// water-ice
	unsigned int  fchnls	: 3;	// fractal data
	unsigned int  evals	    : 2;	// erosion data (0..2)
	unsigned int  tstart	: 4;	// texture data start
	unsigned int  normal	: 1;	// normal flag
	unsigned int  hidden	: 1;	// hidden flag
	unsigned int  margin	: 1;	// margin flag
	unsigned int  hmaps	    : 1;	// hmap flag
	unsigned int  mdata	    : 1;	// margin flag
	unsigned int  edge	    : 1;	// edge flag
	unsigned int  rock	    : 1;	// rock layer
} mpdata;


typedef union mpdu {
	struct mpdata	s;
	int       		l;
} mpdu;

class MapData
{
	static int gid;
	mpdu  flags;
	int   _id;
	d32  *data;

public:
	LTheta lt;
	LPhi   lp;
#ifndef HASH_POINTS
	Point	point_;
#endif
#ifndef HASH_NORMALS
	Point	normal_;
#endif
	void add_normal(Point p)      { normal_=normal_+p;}

#ifdef __LP64__
    void  packp(MapData *d, int i){  fdu fd;
                                   fd.p=d;
                                   data[i].l=fd.l.high;
                                   data[i+1].l=fd.l.low;
                                }
    MapData *unpackp(int i)       { fdu fd;
                                  fd.l.high=data[i].l;
                                  fd.l.low=data[i+1].l;
                                  return fd.p;
                                }
#endif
	int dims()					{ return flags.s.dims;}
	int evals()				    { return flags.s.evals;}
	int textures()				{ return flags.s.textures;}
	int bumpmaps()				{ return flags.s.bumpmaps;}
	int hidden()			    { return flags.s.hidden;}
	void set_hidden(int n)		{ flags.s.hidden=n;}
	int margin()			    { return flags.s.margin;}
	void set_margin(int n)		{ flags.s.margin=n;}
    void set_edge(int i)        { flags.s.edge=i;}
    int edge()                  { return flags.s.edge;}

	int rock()					{ return flags.s.rock;}
	void setRock(bool b)		{ flags.s.rock=b;}

	int normal_valid()			{ return flags.s.normal;}
	void set_normal_valid(int n){ flags.s.normal=n;}
	int colors()				{ return flags.s.colors;}
	int links()					{ return flags.s.links;}
	int has_density()			{ return flags.s.density;}
	void set_has_density(int n)	{ flags.s.density=n;}
	void setFchnls(int n)		{ flags.s.fchnls=n;}
	void setEvals(int n)		{ flags.s.evals=n;}
	void set_has_ocean(int n)	{ flags.s.ocean=n;}
	void setDims(int n)			{ flags.s.dims=n;}
	void setColors(int n)		{ flags.s.colors=n;}
	void setTextures(int n)		{ flags.s.textures=n;}
	void setBumpmaps(int n)		{ flags.s.bumpmaps=n;}
	void setHmaps(int n)		{ flags.s.hmaps=n;}
	void setMdata(int n)		{ flags.s.mdata=n;}

	void setLinks(int n)		{ flags.s.links=n;}
	int type()  				{ return flags.s.type; }
	int dtype()  				{ return flags.s.type==WATER?data2()->type():type(); }
 	MapData *get_surface(int t) { MapData *d=this;
 									while(d){
 	    								if(d->type()==t)
 	        								return d;
 										d=d->data2();
 									}
 									return 0;
 	                            }
	MapData *next_surface()  	{ MapData *d=data2();
	                              if(!d)
	                                  return 0;
	                              if(d->flags.s.type==WATER)
		                             return d->data2();
	                              return d;
	                            }
	void set_type(int t)		{ flags.s.type=t;}
	int water()                 { return flags.s.type==WATER;}
	int two_pass()              { MapData *d=this;
									if(flags.s.type==WATER)
	                                   d=d->data2();
	                                if(d)
	                                    d=d->data2();
	                                if(d && !d->water())
	                                    return 1;
	                                return 0;
	                            }
	int two_surface()           { MapData *d=this;
									while(d){
	    								if(d->flags.s.type==WATER)
	        								return 1;
										d=d->data2();
									}
									return 0;
								}
	MapData *surface2()			{ MapData *d=this;
									while(d){
	    								if(d->flags.s.type==WATER)
	        								return d;
										d=d->data2();
									}
									return 0;
								}
	MapData *surface1()         { if(flags.s.type==WATER)
		                             return data2();
	                              return this;
 	                            }
	void setMemory(int n,int t) {
                                n+=t;
                                flags.s.tstart=TSTART;
                                if(n){
                                    MALLOC(n,d32,data);
                                }

                                }
	int memsize();
    int multitex()             { return flags.s.textures>1 ?1:0;}

    int tstart()               { return flags.s.tstart;}
    int getTexIndex(Texture *, int &i);
    void apply_texture();

    void visit_all(void (*func)()) {
        MapData *d=this;
        while(d){
            (*func)();
            d=d->data2();
        }
    }
    void visit_all(void (MapData::*func)()) {
        MapData *d=this;
        while(d){
            (d->*func)();
            d=d->data2();
        }
    }

    MapData *data2()            { return flags.s.links?data[0].md:0;}
    void setLink(MapData*d)     { if(flags.s.links) data[0].md=d;}
    double density()            { return flags.s.density?data[DSTART].d:0.0;}
    double ocean()              { return flags.s.ocean?data[WSTART].d:0.0;}
    double mdata()              { return flags.s.mdata>0?data[MSTART].d:1.0;}
    double depth()              { return flags.s.evals>0?data[ESTART].d:0.0;}
    double hardness()           { return flags.s.evals>0?data[ESTART].d:0.0;}
    double solid()              { return flags.s.evals>0?data[ESTART].d:0.0;}
    double sediment()           { return flags.s.evals>1?data[ESTART+1].d:0.0;}
    double Z()                  { return flags.s.dims>0?data[ZSTART].d:0.0;}
    double X()                  { return flags.s.dims>1?data[ZSTART+1].d:0.0;}
    double Y()                  { return flags.s.dims>2?data[ZSTART+2].d:0.0;}
    //double fractal()            { return flags.s.fchnls?data[FSTART].d:0.0;}
    double fractal(int i)       { return flags.s.fchnls?data[FSTART+i].d:0.0;}
    void setDensity(double f)   { if(flags.s.density) data[DSTART].d=f;}
    void setOcean(double f)     { if(flags.s.ocean) data[WSTART].d=f;}

    void setZ(double f)         { if(flags.s.dims>0)  data[ZSTART].d=f;}
    void setX(double f)         { if(flags.s.dims>1)  data[ZSTART+1].d=f;}
    void setY(double f)         { if(flags.s.dims>2)  data[ZSTART+2].d=f;}
    //void setFractal(double f)   { if(flags.s.fchnls) data[FSTART].d=f;}
    void setFractal(double f,int i){ if(flags.s.fchnls) data[FSTART+i].d=f;}
    void setMargin(double f)    { if(flags.s.mdata>0) data[MSTART].d=f;}
    void setDepth(double f)     { if(flags.s.evals>0) data[ESTART].d=f;}
    void setHardness(double f)  { if(flags.s.evals>0) data[ESTART].d=f;}
    void setSolid(double f)     { if(flags.s.evals>0) data[ESTART].d=f;}
    void setSediment(double f)  { if(flags.s.evals>1) data[ESTART+1].d=f;}
    void setColor(Color c)      { if(flags.s.colors)  data[CSTART].ul=c.pack();}
    void setTexture(double t, int &a){
        data[a].d=t;
        a+=1;
    }
    double texture(int &a) {
        double t=data[a].d;
        a+=1;
        return t;
    }
    void setBumpmap(double t, int &a){
        data[a].d=t;
        a+=1;
    }
    double bumpmap(int &a) {
        double t=data[a].d;
        a+=1;
        return t;    }

	Color  color()				{ return flags.s.colors?Color(data[CSTART].ul):WHITE;}
	//double Ht()					{ return Z()+sediment();}
    double Ht()                 { return Z();}
    void setHt(double z);

	MapData(uint t, uint p);
	MapData(MapData *p);
	~MapData();

	void	free();

	int id()						{ return _id;}
	int set_id(int i)				{ return _id=i;}
	uint ltheta()					{ return lt.lvalue();}
	uint lphi()					    { return lp.lvalue();}

	double height();
	double tbase()					{ return lt.dvalue();}
	double pbase()					{ return lp.dvalue();}

	void set_tbase(double d)		{ lt.set(d);}
	void set_pbase(double d)		{ lp.set(d);}

	double theta()					{ return lt.dvalue()+X(); }
	double phi()					{ double f=lp.dvalue()+Y();
									  return clamp(f,-90,90);
									}
	double pnorm()					{ return lp.norm();}
	double tnorm()					{ return lt.norm();}

	double span(MapData *d);
	Point point();
	Point mpoint();
	Point pvector();
	Point tvector();
	void init_terrain_data(TerrainData&,int);

	void invalidate();
	void invalidate_normal();
	void invalidate_point();
	void validate_normal();
	void setSurface();

	friend class MapNode;
};

#define MAX_NDATA 10
extern MapData *mapdata[MAX_NDATA];
extern int mdcnt,rccnt;
extern MapData *mdctr;

#endif

