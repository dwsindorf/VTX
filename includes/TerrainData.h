// TerrainData.h

#ifndef _TERRAIN_DATA
#define _TERRAIN_DATA
#include "NoiseClass.h"
#include "PointClass.h"
#include "ColorClass.h"
#include "NodeIF.h"

class ExprMgr;
class TerrainMgr;
class TNode;
class TNarg;
class Object3D;
class TNroot;
class Image;
class TNtexture;
class TNsubr;
class TNvar;
class Texture;
class Sprite;
class Plant;
class TerrainProperties;
class TNnoise;
class TNcolor;
class TNdensity;
class TNpoint;
class TNfractal;
class TNclouds;
class TNrocks;
class PlaceObj;

extern int    tindex, mindex;

#define RANDID 1.237e-66
#define DEFAULT_SHINE   5
#define DEFAULT_ALBEDO  0.2
#define DEFAULT_SPECULAR  0.2
#define DEFAULT_AMBIENT  0.02

#define S0 Tstack[tindex]
#define S1 Tstack[tindex-1]
#define S2 Tstack[tindex-2]

#define PINIT S0.p.z=S0.p.x=S0.p.y=0
#define CINIT S0.c=BLACK
#define SINIT S0.InitS();S0.s=0;
#define TINIT S0.InitT();
#define INIT  S0.InitAll();

#define SPUSH tindex++; SINIT;
#define SPOP tindex--

enum {
	HT2PASS     = 0x00000001,    // requires 2 ht passes
	LOWER       = 0x00000002,    // dual surface layer (td.lower2 valid)
	CLRTEXS     = 0x00000004,    // clear previous textures
	INMARGIN    = 0x00000008,    // in margin flag
	CFIRST      = 0x00000010,    // first craters flag
	SFIRST      = 0x00000020,    // first slope flag
	FFIRST      = 0x00000040,    // first erode flag
	FVALUE      = 0x00000080,    // fractal flag
	FNOREC      = 0x00000100,    // fractal flag (no recalc)
	EVALUE      = 0x00000200,    // sediment flag
	DVALUE      = 0x00000400,    // depth flag
	TEXFLAG     = 0x00000800,    // texture flag
	WATERFLAG   = 0x00001000,    // terrain has water
	COLORFLAG   = 0x00002000,    // terrain has color
	SNOISEFLAG  = 0x00004000,    // shader noise
	GNOISEFLAG  = 0x00008000,    // shader noise
	NOISE1D     = 0x00010000,     // shader 1D noise
	HIDDEN      = 0x00020000,    // invisible node
	COMPLEX     = 0x00040000,    // complex expression
	MULTILAYER  = 0x00080000,    // multilayer terrain
	ROCKLAYER   = 0x00100000,    // rocks terrain
	ROCKBODY    = 0x00200000,    // rocks terrain
	INEDGE      = 0x00400000,    // edge flag
};

enum {   //Terrain and NodeData type flags
	WATER          = 0x00,  // water id flag
	ID0            = 0x01,  // first terrain id
	IDMAX          = 0xff   // max terrain id
};

#define MAX_TDATA 	 5
#define MAX_FDATA 	 4
#define MAX_NOISE 	 4
#define TZBAD       -100
#define TZVALID(t)   t.p.z>TZBAD

//************************************************************
// Class TerrainData
//************************************************************
typedef struct tdata {
	unsigned int  type		: 8;	 // type code
	unsigned int  flags	: 16;	 // misc flags
} tdata;

typedef union tdu {
	struct tdata	s;
	int       		l;
} tdu;


class TerrainData
{
protected:
	enum {
		COLOR_FLAG  	= 0x0001,
		POINT_FLAG  	= 0x0002,
		SCALE_FLAG  	= 0x0004,
		CONST_FLAG  	= 0x0008,
		STRING_FLAG 	= 0x0010,
		IMAGE_FLAG 		= 0x0020,
		HARD_FLAG 		= 0x0040,
		INACTIVE_FLAG   = 0x0080
	};
	tdu info;
public:
	static Image       *image;  	// image pointer (TNImage)
	static double       albedo;  	// albedo multiplier
	static double       shine;  	// shine multiplier
	static Color		emission;  	// emission color
	static Color		ambient;  	// ambient color
	static Color		diffuse;  	// diffuse color
	static Color		specular;  	// specular color
	static double       density; 	// density (fog ..)
	static double       ocean; 	    // water-ice
	static double       erosion; 	// erosion
	static double       sediment;	// sediment
	static double       clarity; 	// max water clarity
	static double       rock;    	// rock
	static double       depth;    	// depth
	static double       margin;    	// depth
	static double       height;    	// ht
	static double       level;      // cell size level
    static double       extent;     // cell size extent (pixels)
	static TNclouds    *clouds;
	static TerrainData  lower;   	// lower surface level
	static double       texht;	    // height map texture
	static double       fracval[MAX_FDATA];    // fractal ht

	static Array<TerrainProperties*>properties;
	static Array<PlaceObj*> plants;
	static Array<PlaceObj*> sprites;
	static int sid;
	static int fid;
	static int rid;

	static TerrainData  *data[MAX_TDATA]; // second surface data
	static TerrainData  zlevel[MAX_TDATA]; // zordered highest values
	static int          zlevels;
	static int datacnt;
	static int flags;
	static int tids;
	static int sids;
	static int rids;
	static int fids;
	static int pass;

	static TerrainProperties *tp;

	static void clr_flags()        { flags=0;}
	static void set_flag(int i)    { BIT_ON(flags,i);}
	static void clr_flag(int i)    { BIT_OFF(flags,i);}
	static int get_flag(int i)		{ return flags&i;}

	static Point	rectangular(double t, double p);

	void insert_strata(TerrainData &);
	void begin();
	void end();

	void set_ivalid()	 	{ BIT_ON(info.s.flags,IMAGE_FLAG);}
	void clr_ivalid()		{ BIT_OFF(info.s.flags,IMAGE_FLAG);}
	int ivalid()		    { return info.s.flags & IMAGE_FLAG;}

	void set_cvalid()	 	{ BIT_ON(info.s.flags,COLOR_FLAG);}
	void clr_cvalid()		{ BIT_OFF(info.s.flags,COLOR_FLAG);}
	int cvalid()		    { return info.s.flags & COLOR_FLAG;}

	void set_strvalid()	 	{ BIT_ON(info.s.flags,STRING_FLAG);}
	void clr_strvalid()		{ BIT_OFF(info.s.flags,STRING_FLAG);}
	int strvalid()		    { return info.s.flags & STRING_FLAG;}

	void set_pvalid()		{ BIT_ON(info.s.flags,POINT_FLAG);}
	void clr_pvalid()		{ BIT_OFF(info.s.flags,POINT_FLAG);}
	int pvalid()		    { return info.s.flags & POINT_FLAG;}

	void set_svalid()		{ BIT_ON(info.s.flags,SCALE_FLAG);}
	void clr_svalid()		{ BIT_OFF(info.s.flags,SCALE_FLAG);}
	int svalid()		    { return info.s.flags & SCALE_FLAG;}

	void set_constant()		{ BIT_ON(info.s.flags,CONST_FLAG);}
	void clr_constant()		{ BIT_OFF(info.s.flags,CONST_FLAG);}
	int constant()		    { return info.s.flags & CONST_FLAG;}

	void set_hardness()		{ BIT_ON(info.s.flags,HARD_FLAG);}
	void clr_hardness()		{ BIT_OFF(info.s.flags,HARD_FLAG);}
	int hardness()		    { return info.s.flags & HARD_FLAG;}

	void set_inactive()		{ BIT_ON(info.s.flags,INACTIVE_FLAG);}
	void clr_inactive()		{ BIT_OFF(info.s.flags,INACTIVE_FLAG);}
	int inactive()		    { return info.s.flags & INACTIVE_FLAG;}

	int type()				{ return info.s.type;}
	void set_type(int i)     { info.s.type=i;}

	int id()                { return (info.s.type&IDMAX)-ID0;}
	void set_id(int i)      { info.s.type=(i&IDMAX)+ID0;}
	void next_id()          { set_id(tids++);}
	TerrainProperties		*add_id();
	static void add_texture(Texture *);
	static void add_sprite(Sprite *);
	static void add_plant(Plant *);
	static void add_fractal(TNfractal *);

	static void add_TNnoise(TNnoise *);
	static void add_TNcolor(TNcolor *);
	static void add_TNdensity(TNdensity *);
	static void add_TNpoint(TNpoint *);
	static void add_TNclouds(TNclouds *);
	void set_water()    	{ info.s.type=WATER;}
	int water()          	{ return info.s.type==WATER;}


	Point   p;				// Point value
	Color   c;				// Color value
	double  s;				// Scale value
	char *string;           // string pointer

	void InitT()			{ datacnt=0;}
	void InitI()			{ image=0;}
	void InitS()			{ info.l=0;set_id(0);}
	void InitC()			{ c=WHITE;}
	void InitP()			{ p.z=p.x=p.y=0;}
	void InitAll()			{ info.l=0;
							  s=p.z=p.x=p.y=0;
							  c=WHITE;
							  set_id(0);
							}
	void reset(){
		InitAll();
		datacnt=0;
		tids=0;
		mindex=0;
		tindex=0;
		image=0;
		clouds=0;
		texht=0;
		margin=0;
		depth=0;
		density=0;
		ocean=0;
	}
	void init(){
		reset();
		clr_flags();
		for(int i=0;i>MAX_FDATA;i++)
			fracval[i]=0;
		level=0;
	}
	TerrainData() {InitAll();}
	void copy(TerrainData &t2)
	{
		p=t2.p;
		c=t2.c;
		s=t2.s;
		info.l=t2.info.l;
	}
	void eval_properties();
	friend class TerrainProperties;
};

//************************************************************
// Class TerrainSym
//************************************************************
class TerrainSym : public ObjectSym {
enum {
	SHOW_FLAG  = 0x01,
	EVAL_FLAG  = 0x02,
	CONST_FLAG = 0x04,
	REVAL_FLAG = 0x08,
	DIRTY_FLAG = 0x10,
	HT_FLAG    = 0x20
};

    int                 flags;
public:
	const char 		   *units;
	TerrainData		    value;
	TerrainSym(const char *t, TerrainData v) : ObjectSym(t)
			{ value=v;units=0;flags=SHOW_FLAG|EVAL_FLAG;}

	void set_show(int i)	{ BIT_SET(flags,SHOW_FLAG,i);}
	int  do_show()		    { return flags&SHOW_FLAG;}

	void set_eval(int i)	{ BIT_SET(flags,EVAL_FLAG,i);}
	int  do_eval()		    { return flags&EVAL_FLAG;}

	void set_constant(int i){ BIT_SET(flags,CONST_FLAG,i);}
	int  constant()		    { return flags&CONST_FLAG;}

	void set_reval(int i)	{ BIT_SET(flags,REVAL_FLAG,i);}
	int  reval()		    { return flags&REVAL_FLAG;}

	void set_dirty(int i)	{ BIT_SET(flags,DIRTY_FLAG,i);}
	int  dirty()		    { return flags&DIRTY_FLAG;}

	void set_ht(int i)		{ BIT_SET(flags,HT_FLAG,i);}
	int  ht()		    	{ return flags&HT_FLAG;}

	void save(FILE*);
};

extern TerrainData Tstack[64];

//************************************************************
// Class TerrainProperties
//************************************************************

typedef struct tpinfo {
	unsigned int  type	    : 8;	// type flag
	unsigned int  tsize	    : 6;	// textures info size
	unsigned int  density	: 1;	// terrain contains TNdensity
	unsigned int  color	    : 1;	// terrain contains TNcolor
	unsigned int  point	    : 1;	// terrain contains TNpoint
	unsigned int  bmpht	    : 1;	// terrain contains BMPHT
	unsigned int  inactive  : 1;	// hidden layer
	unsigned int  geometry	: 1;	// use geometry shader (point noise)
	unsigned int  complex	: 1;	// complex expression
	unsigned int  rock	    : 1;	// rock layer
	unsigned int  unused	: 5;	// unassigned
} tpinfo;

typedef union tpinfo_u {
	struct tpinfo	s;
	int       		l;
} tpinfo_u;
class TerrainProperties
{
public:
	double albedo;
	double shine;
	Color glow;
	Color color;
	int callList;
	Array<Texture*> textures;
	Array<TNnoise*> noise;
	Array<TNrocks*> rocks;
	Array<PlaceObj*> sprites;
	Array<PlaceObj*> plants;
	
	TNcolor *tncolor;
	TNpoint *tnpoint;
	TNdensity *tndensity;
	tpinfo_u info;
	int id;
	int ntexs;

	static int nid;
	static int tid;

	static int pass;

	TerrainProperties(int);
	~TerrainProperties();
	int tsize() { return info.s.tsize;}

	void set_complex(bool b) { info.s.complex=b;}
	bool is_complex()  	    { return info.s.complex;}
	void set_rock(bool b)    { info.s.rock=b;}
	bool is_rock()  	    { return info.s.rock;}
	void set_geometry(bool b) { info.s.geometry=b;}
	bool has_geometry()  	{ return info.s.geometry;}
	void set_color(bool b)  { info.s.color=b;}
	bool has_color()  		{ return info.s.color;}
	void set_density(bool b) { info.s.density=b;}
	bool has_density()  	{ return info.s.density;}
	void set_bmpht(bool b)  { info.s.bmpht=b;}
	bool has_bmpht()  		{ return info.s.bmpht;}
	void set_point(bool b)  { info.s.point=b;}
	bool has_point()  		{ return info.s.point;}
	void set_inactive(bool b)  { info.s.inactive=b;}
	bool inactive()  		{ return info.s.inactive;}
	int type()				{ return info.s.type;}
	void set_type(int i)    { info.s.type=i>=0?i:0;}
	void initProgram();
	void setProgram();
	void reset();
	int passes();
	void add_sprite(Sprite *s);

};
#endif

