// Placements.h

#ifndef _PLACEMENTS
#define _PLACEMENTS

#include "TerrainMgr.h"
#include "ImageMgr.h"

//#define DEBUG_PMEM         // turn on for memory usage
// FIXME: need to base hash code on placement type
class PlacementMgr;

enum {
    PID 		= 0x00000fff,
	CRATERS 	= 0x00001000,
    ROCKS   	= 0x00002000,
    CLOUDS   	= 0x00004000,
    SPRITES   	= 0x00008000,
    PLACETYPE   = 0x0000f000,

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
	unsigned int  unused     : 22;	// unassigned bits
} place_flags;

typedef union place_flags_u {
	place_flags			s;
	int         		l;
} place_flags_u;

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
	
};


typedef struct place_mgr_flags {
	unsigned int  margin	 : 1;	// set margin bit
	unsigned int  first	     : 1;	// first mgr in pass
	unsigned int  joined	 : 1;	// joined mgr in pass
	unsigned int  offset	 : 1;	// offset valid flag
	unsigned int  finalizer  : 1;	// indicates static object
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

	static Placement  **hash;
    void find_neighbors(Placement *);

public:
	//static	int		last_id;
	double			size;
	double 			roff;
	double 			roff2;
	Point4D			mpt;
	Point4D			offset;
	static int index;
	static int hits;
	static int hashsize;
	static LinkedList<Placement*> list;
	virtual bool valid(){ return true;}

	int set_ntest(int i)		{ return i?BIT_OFF(options,NNBRS):BIT_ON(options,NNBRS);}
	int ntest()					{ return options & NNBRS?0:1;}
	int lod()					{ return options & NOLOD?0:1;}
	void set_margin(int i)   	{ flags.s.margin=i;}
	int margin()				{ return flags.s.margin;}
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

	static void free_htable();

	int		type;    // type id
	int     options;
	int		id;
	int     lvl;
	double  maxsize;
	int     levels;
	double	mult;
	double  density;
	TNode   *dexpr;
	double  base;

	double  ht;
	double  msize;

	PlacementMgr(int);
	virtual ~PlacementMgr();

	virtual void reset();
	virtual void init();
	virtual void eval();
	virtual void dump();
	virtual Placement *make(Point4DL&,int);
	static Placement *next();
	static void ss();
	static void end();

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

};

#endif


