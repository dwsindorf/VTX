// Placements.h

#ifndef _PLACEMENTS
#define _PLACEMENTS

#include "TerrainMgr.h"
#include "ImageMgr.h"
#include "Util.h"

//#define DEBUG_CRATERS
//#define TEST_SPRITES
//#define TEST_PLANTS
//#define TEST_CRATERS
#define TEST_ROCKS

//#define DEBUG_PMEM         // turn on for memory usage
// FIXME: need to base hash code on placement type
class PlacementMgr;

enum {
    PID 		= 0x00000fff,
	CRATERS 	= 0x00001000,
    ROCKS   	= 0x00002000,
    CLOUDS   	= 0x00003000,
    SPRITES   	= 0x00004000,
    PLANTS   	= 0x00005000,
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
    MAXHT    	= 0x10000000,
    CNORM    	= 0x20000000,
    FINAL   	= 0x80000000
};


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
	static int chits,cvisits,crejects;
	static int nhits,nmisses,nvisits,nrejects;
	static int cmade,cfreed;
	static int vtests,pts_fails,dns_fails;
	static int place_visits,place_hits,place_misses;

	static void reset(){
		chits=cvisits=crejects=0;
		nhits=nmisses=nvisits=nrejects=0;
		cmade=cfreed=0;
		vtests=pts_fails=dns_fails=0;
		place_visits=place_hits=place_misses=0;
	}
	static void exec();
};


class Placement
{
protected:
public:
	Point4DL  	point;   // hash point
	Point4D     center;	 // surface projected point
	double		radius;  // radius
	int			hid;     // hash id
	int			type;    // type id
	int			users;
	place_flags_u flags;

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
};


typedef struct place_mgr_flags {
	unsigned int  margin	 : 1;	// set margin bit
	unsigned int  first	     : 1;	// first mgr in pass
	unsigned int  joined	 : 1;	// joined mgr in pass
	unsigned int  offset	 : 1;	// offset valid flag
	unsigned int  finalizer  : 1;	// indicates static object
	unsigned int  debug	     : 1;	// debug
	unsigned int  unused     : 27;	// unassigned bits
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

public:
	
	Placement  **hash;
	double			size;
	double 			roff;
	double 			roff2;
	double 			render_ptsize;
	double 			adapt_ptsize;
	Point4D			mpt;
	Point4D			offset;
	int 			hashsize;	
	int index;
	
	PlacementStats Stats;

	void free_htable();
	Placement *next();
	void ss();
	
	int getHashsize(){
		return hashsize;
	}
	void setHashsize(int n){
		if(hash && hashsize !=n){
			free_htable();
			delete hash;
			hash=0;
			hashsize=n;
			init();
		}
	}
	LinkedList<Placement*> list;

	int set_ntest(int i)		{ return i?BIT_OFF(options,NNBRS):BIT_ON(options,NNBRS);}
	bool sizetest()				{ return options & MINSIZE?true:false;}
	int ntest()					{ return options & NNBRS?0:1;}
	int lod()					{ return options & NOLOD?0:1;}
	virtual bool testColor();
	virtual bool testDensity();
	void set_testColor(bool b)	{ BIT_SET(options,COLOR_TEST,b);}
	void set_testDensity(bool b){ BIT_SET(options,DENSITY_TEST,b);}
	bool test()  				{ return testColor()||testDensity();}
	bool is3D()                 { return options&MC3D;}
	void set3D(bool b)          { BIT_SET(options,MC3D,b);}
	void set_margin(int i)   	{ flags.s.margin=i;}
	int margin()				{ return flags.s.margin;}
	void set_debug(int i)   	{ flags.s.debug=i;}
	int debug()				    { return flags.s.debug;}

	void set_first(int i)   	{ flags.s.first=i;}
	int first()				    { return flags.s.first;}
	void set_joined(int i)   	{ flags.s.joined=i;}
	int joined()				{ return flags.s.joined;}
	void set_offset_valid(int i){ flags.s.offset=i;}
	int offset_valid()		    { return flags.s.offset;}
	void set_finalizer(int i)   { flags.s.finalizer=i;}
	int finalizer()		        { return flags.s.finalizer;}
	void set_id(int i)          { type=i&PID;}
	int get_id()				{ return type&PID;}
	int get_class()				{ return type&PLACETYPE;}

	int		type;    // type id
	int     options;
	int		id;
	int		instance;
	int     lvl;
	double  maxsize;
	int     levels;
	double	mult;
	double	level_mult;
	double  density;
	TNode   *dexpr;
	double  base;

	double  ht;
	double  msize;

	PlacementMgr(int,int);
	PlacementMgr(int);
	virtual ~PlacementMgr();

	virtual void reset();
	virtual void init();
	virtual void eval();
	virtual void dump();
	virtual bool valid();

	virtual Placement *make(Point4DL&,int);

	friend class Placement;
};

//************************************************************
// Class TNplacements
//************************************************************
class TNplacements : public TNbase
{
protected:
public:
    PlacementMgr *mgr;
	TNplacements(int t, TNode *l, TNode *r, TNode *b);
	virtual ~TNplacements();
	
	virtual void reset();
	virtual void eval();
	virtual void init();
	virtual void save(FILE *);
	virtual int typeValue()			{ return ID_PLACED;}
	virtual void setProperties(NodeIF *n);
	virtual void valueString(char *);
	virtual int optionString(char *);
	virtual void addToken(LinkedList<TNode*>&l);
	virtual void set_id(int i);
	virtual int get_id();
	void set_flip(int i)   	    { if(i)BIT_ON(type,FLIP); else BIT_OFF(type,FLIP); }
	int flip()				    { return type & FLIP;}
	bool is3D()   				{ return type & MC3D;}
	void set3D(bool b)          { BIT_SET(type,MC3D,b);}

};

#endif


