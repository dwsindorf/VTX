// Placements.h

#ifndef _PLACEMENTS_
#define _PLACEMENTS_

//#pragma once

//#include "TerrainMgr.h"
#include "ImageMgr.h"
#include "Util.h"

//#define DEBUG_CRATERS
#define TEST_SPRITES
#define TEST_PLANTS
#define TEST_CRATERS
#define TEST_ROCKS

#define PRINT_PLACEMENT_TIMING
//#define PRINT_PLACEMENT_STATS

//#define DEBUG_PMEM         // turn on for memory usage

class PlacementMgr;
class PlaceData;
class TNarg;

enum {
    PID 		= 0x00000fff,
	CRATERS 	= 0x00001000,
    ROCKS   	= 0x00002000,
    CLOUDS   	= 0x00003000,
    SPRITES   	= 0x00004000,
    PLANTS   	= 0x00005000,
    MCROCKS   	= 0x00006000,
    PLACETYPE   = 0x0000f000,
	
    FLIP        = 0x00010000,
    MINSIZE     = 0x00020000,
    MC3D  	    = 0x00040000,
	
	COLOR_TEST  = 0x00100000,
	DENSITY_TEST= 0x00200000,
	
    NOLOD   	= 0x01000000,
    NNBRS   	= 0x02000000,
    MAXA    	= 0x04000000,
    MAXB    	= 0x08000000,
    MXHT    	= 0x10000000,
    CNORM    	= 0x20000000,
    FINAL   	= 0x80000000
};

class SData {
public:
    double v;
    double f;
    int l;
    int h;
    double value()   { return v;}
};

#define SDATA_SIZE 256

typedef struct place_flags {
	unsigned int  users	     : 8;	// users
	unsigned int  valid	     : 1;	// radius >0
	unsigned int  active	 : 1;	// used
	unsigned int  unused     : 21;	// unassigned bits
} place_flags;

typedef union place_flags_u {
	place_flags			s;
	int         		l;
} place_flags_u;

class PlacementStats : public StatCollector {
public:
	static int chits,cvisits,crejects,cchained;
	static int nhits,nmisses,nvisits,nrejects,nchained;
	static int cmade,cfreed;
	static int vtests,pts_fails,dns_fails,dns_pass;
	static int lod_visits,lod_hits,lod_fails;

	static void reset(){
		chits=cvisits=crejects=cchained=0;
		nhits=nmisses=nvisits=nrejects=nchained=0;
		cmade=cfreed=0;
		vtests=pts_fails=dns_fails=dns_pass=0;
		lod_visits=lod_hits=lod_fails=0;
	}
	static void exec();
	static void dump();
};


class PlaceObj
{
protected:
	int    id;
public:

	int    type;
	int    layer;
	bool   valid;
	TNode  *expr;
	PlaceObj(int t, TNode *e);

    virtual void eval()  {};  
    virtual void print() {};
    virtual bool setProgram() { return false;}
    virtual bool initProgram() {return false;} 
    virtual PlacementMgr *mgr() { return nullptr;}
    
	int get_id()				{ return id;}
	void set_id(int i)			{ id=i;}
	bool isValid()				{ return valid;}
	void setIsValid(bool i)		{ valid=i;}

};

class PlaceObjMgr
{
protected:
	Array<PlaceObj*> objs;
public:
	static bool shadow_mode;
	PlaceObjMgr();
	virtual void collect(){}
	virtual void eval();
	virtual void reset()  { objs.reset();}
	virtual void free() {}
	virtual bool setProgram(){return false;}
	virtual void render() {}
	virtual int layer() { return objs.size>0?objs[0]->layer:0;}
	virtual int placements() { return 0;}
	virtual const char *name() { return 0;}
	virtual bool supports_shadows() { return false;}
	virtual void render_zvals() {}
	virtual void render_shadows(){}

	int objects() { return objs.size;}
	
	void addObject(PlaceObj *obj) { objs.add(obj);}
	void rmoveObject(PlaceObj *obj) { objs.remove(obj);}
	
};

class Placement
{
public:
	Point4DL  	point;   // hash point
	Point4D     center;	 // surface projected point
	double		radius;  // radius
	int			hid;     // hash id
	int			type;    // type id
	int			lvl;     // level
	int         layer;   // terrain layer
	int			users;
	double 		ht;
	double 		aveht;
	double 		wtsum;
	double 		dist;
	double      pts;
	double      rval;
	int 		visits;
	int 		hits;
	int 		instance;
    Point       vertex;
    Point       normal;
	PlacementMgr *mgr;
	place_flags_u flags;
	
	Placement* next;  // ⭐ ADD THIS - chain for collisions

	Placement(PlacementMgr&, Point4DL&,int);

	virtual bool set_terrain(PlacementMgr &mgr);
	virtual void dump();
	virtual void reset();
	int get_id()				{ return type&PID;}
	int get_class()				{ return type&PLACETYPE;}
	void setActive(bool b)		{ flags.s.active=b;}
	bool active()               { return flags.s.active;}
	bool is3D()                 { return type&MC3D;}
	void set3D(bool b)          { BIT_SET(type,MC3D,b);}
	void setVertex();
	bool isEqual(Point4DL &p, int type, int lvl, int inst, int layer);
};

class PlaceData
{
public:
	int    type;
	double dist;
	double radius;
	double ht;
	double pts;
	double rval;
	int instance;
	Point vertex;
	Point normal;
	
	PlacementMgr *mgr;
	PlaceData(Placement*);
	
	virtual double value() { return dist;}
	int get_id()				{ return type&PID;}
	int get_class()				{ return type&PLACETYPE;}
};

typedef struct place_mgr_flags {
	unsigned int  margin	 : 1;	// set margin bit
	unsigned int  first	     : 1;	// first mgr in pass
	unsigned int  joined	 : 1;	// joined mgr in pass
	unsigned int  offset	 : 1;	// offset valid flag
	unsigned int  finalizer  : 1;	// indicates static object
	unsigned int  debug	     : 1;	// debug
	unsigned int  testpts	 : 1;	// test point size
	unsigned int  useaveht	 : 1;	// use average ht
	unsigned int  showstats	 : 1;	// show statistics
	unsigned int  unused     : 25;	// unassigned bits
} place_mgr_flags;

typedef union place_mgr_flags_u {
	place_mgr_flags			s;
	int         		l;
} place_mgr_flags_u;


class PlacementMgr
{
protected:
	
	place_mgr_flags_u flags;

    void find_neighbors(Placement *);
    static Placement* currentChain;  // ⭐ Add this member variable
	static int 			index;

public:
	
	static Placement  **hash;
	static SData   sdata[SDATA_SIZE];
	static ValueList<SData*> slist;
	static double size;
	static int  scnt,hits,lvl,slvl;
	static Point4D	offset;
	static Point4D	mpt;
	static double sval,cval,htval;
	static LinkedList<Placement*> list;
	static PlacementStats Stats;
	static int  trys,visits,bad_visits,bad_valid,bad_active,bad_pts,new_placements;
	static double roff,roff2;
	static int hashsize;
	static double collect_minpts,adapt_ptsize,render_ptsize;
	static int min_hits;

	int		type;    // type id
	int     options;
	int		id;
	int		instance;
	int		layer;
	
	int     levels;
	double  maxsize;
	double	mult;
	double	level_mult;	
	double  density;
	double  ht;
	double  base;
	double  slope_bias;
	double  ht_bias;
	double  lat_bias;
	double  hardness_bias;
	double  selection_bias;
	double  maxdensity;
	double  comp;
	double  drop;
	double  noise_amp;
	double  noise_expr;
	double  pts_scale;

	TNode   *dexpr;

	static void free_htable();
	
	static Placement *next();
	static Placement *next(int type);
	static void resetIterator() {
	    index = 0;
	    currentChain = nullptr;
	}
	static void printChainStats();
	static double calcDensity(double s, double m, double b, double p);
	int hashPoint(Point4DL& pc, int lvl, int id);

	int set_ntest(int i)		{ return i?BIT_OFF(options,NNBRS):BIT_ON(options,NNBRS);}
	bool sizetest()				{ return options & MINSIZE?true:false;}
	int ntest()					{ return options & NNBRS?0:1;}
	int lod()					{ return options & NOLOD?0:1;}
	void set_testColor(bool b)	{ BIT_SET(options,COLOR_TEST,b);}
	void set_testDensity(bool b){ BIT_SET(options,DENSITY_TEST,b);}
	bool test()  				{ return testColor()||testDensity();}
	bool is3D()                 { return options&MC3D;}
	void set3D(bool b)          { BIT_SET(options,MC3D,b);}
	void set_margin(int i)   	{ flags.s.margin=i;}
	int margin()				{ return flags.s.margin;}
	void set_debug(int i)   	{ flags.s.debug=i;}
	int debug()				    { return flags.s.debug;}
	void set_showstats(int i)   { flags.s.showstats=i;}
	int showstats()				{ return flags.s.showstats;}
	void set_testpts(int i)   	{ flags.s.testpts=i;}
	int testpts()				{ return flags.s.testpts;}
	void set_useaveht(int i)   	{ flags.s.useaveht=i;}
	int useaveht()				{ return flags.s.useaveht;}

	void set_first(int i)   	{ flags.s.first=i;}
	int first()				    { return flags.s.first;}
	void set_joined(int i)   	{ flags.s.joined=i;}
	int joined()				{ return flags.s.joined;}
	void set_offset_valid(int i){ flags.s.offset=i;}
	int offset_valid()		    { return flags.s.offset;}
	void set_finalizer(int i)   { flags.s.finalizer=i;}
	int finalizer()		        { return flags.s.finalizer;}
	void set_pid(int i)         { type=i&PID;}
	int get_pid()				{ return type&PID;}
	int get_class()				{ return type&PLACETYPE;}


	PlacementMgr(int);
	virtual ~PlacementMgr();
	static void resetAll();

	virtual void reset();
	virtual void init();
	virtual void eval();
	virtual void dump();
	virtual bool valid();
	virtual bool setTests();
	virtual bool testColor();   // color node for debugging
	virtual bool testDensity(); // add for detail to center to more accurately define seed position
	virtual void collect(ValueList<PlaceData*> &data); // collect list of objects
	virtual Placement *make(Point4DL&,int);
	virtual PlaceData *make(Placement*s);
	virtual void setHashcode();
	virtual void getArgs(TNarg *);
	static bool setProgram(Array<PlaceObj*> &objs);
	static void render(Array<PlaceObj*> &objs);
	friend class Placement;
};

#endif


