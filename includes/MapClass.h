// MapClass.h

#ifndef _MAPCLASS
#define _MAPCLASS

#include "GLheaders.h"
#include "SceneClass.h"
#include "Util.h"
#include <math.h>

// Class Map

#define PLVLS   64
#define TIDS	256
class MapData;
class MapNode;
class TerrainProperties;
class Triangle;
	enum{
		SHADER_LISTS = 0,
		RASTER_LISTS = 1,
		NORMAL_LISTS = 2,
		ZVAL_LISTS = 3,
		LISTTYPES = 4,
		MAXLISTS=32,
	};

class Map
{
protected:
	enum {	// map status flags
		VCHKED		=	0x00000001,	// visibility is up to date
		BCHKED	    =	0x00000002,	// bounds is up to date
		IDMAP       =	0x00000004,	// ids used for tests
		FIXEDGRID  	=	0x00000008,	// initializing grid in make
		NEEDADAPT  	=	0x00000010,	// adapt needed
		VISTEXS     =	0x00000020,	// visible textures
		ALPHATEXS   =	0x00000080,	// some texures are transparent
		WATERPASS   =	0x00000100,	// two surfaces (visible water)
		VISVFOG     =	0x00000200,	// visible vfog
		VISBMPS     =	0x00000400,	// visible bumpmaps
		TRANSPARANT =	0x00000800,	// set transparancy flag
		TEXTURES    =	0x00001000, // textures enabled in pass
		LIGHTING    =	0x00002000,	// lighting enabled in pass
		EROSION     =	0x00004000, // has erosion attributes
		COLORS      =	0x00008000, // enable colors
		LEAFCYCLE   =	0x00010000, // set leaf cycle
		MASK        =	0x00020000, // mask bit
		MAKE_LISTS  =	0x00040000, // make triangle list
		RENDER_LISTS  =	0x00080000, // render triangle list
		SORT        =	0x00100000, // sort triangle list
	    RENDER_BTOF =	0x00200000, // render using triangle list back to front
		VISIDS      =	0x00400000, // generate vis box from id pass
		ENDADAPT    =	0x00800000, // set before running last ids pass
		GEOMETRY    =	0x01000000, // geometery flag;
		MULTILAYER  =	0x02000000,  // multilayer flag
		SPRITES     =	0x04000000	// transparancy flag

	};

	friend class MapNode;
	friend class MapLink;
	friend class NodeData;
	MapNode *npole;
	MapNode *spole;
	MapNode *last;
	int mcount,mcreated,mdeleted;
	
    double _conv;

	static int tesslevel;

	unsigned int grid_;
	void render_ids_();
	void reset();
	void reset_texs();
	void render_texs();
	void render_sprites();

	void render_bumps();
	void render_water();
	void make_triangle_lists();
	void clearLists();

public:
	MapNode *current;

	int callLists[LISTTYPES][MAXLISTS];
    bool tid_lists[LISTTYPES];
	Bounds  vbounds;
	Bounds  rbounds;
	int     texpass;
	Texture *texture;
	int ntexs;
    int tid;
    int tids;
    
    static LinkedList<Triangle*> triangle_list;
	ValueList<Triangle*> triangles;

	TerrainProperties *tp;

	static bool use_call_lists;
	static bool use_triangle_lists;
	static int maxtesslevel;


	void enablelists(bool s);

	int    idcnts[TIDS];
	int     visid(int);

	void    render_ids();
	void    setvis_ids();
	
	void set_conv(double f)     {_conv=f;}
	double conv()               {return _conv;}

	void	set_grid(int i)		{ grid_=i;}
	unsigned int grid_size()	{ return grid_;}
	int		fixed_grid()		{ return grid_ ? 1 : 0;}

	void	set_sort(int i) 	{ BIT_SET(flags,SORT,i);}
	int	    sort()			 	{ return (flags & SORT)?1:0;}

	void	set_make_lists(int i) 	{ BIT_SET(flags,MAKE_LISTS,i);}
	int	    make_lists()		{ return (flags & MAKE_LISTS)?1:0;}

	void	set_render_lists(int i) 	{ BIT_SET(flags,RENDER_LISTS,i);}
	int	    render_lists()		{ return (flags & RENDER_LISTS)?1:0;}

	void	set_render_ftob()   { BIT_SET(flags,RENDER_BTOF,0);}
	int	    render_ftob()	    { return (flags & RENDER_BTOF)?0:1;}

	void	set_render_btof()   { BIT_SET(flags,RENDER_BTOF,1);}
	int	    render_btof()	    { return (flags & RENDER_BTOF)?1:0;}

	void	set_multilayer()   { BIT_SET(flags,MULTILAYER,1);}
	int	    multilayer()	    { return (flags & MULTILAYER)?1:0;}

	int     render_triangles();

	void	set_mask(int i)	    { BIT_SET(flags,MASK,i);}
	int	    mask()				{ return (flags & MASK)?1:0;}

	void	set_leaf_cycle(int i)  { BIT_SET(flags,LEAFCYCLE,i);}
	int	    leaf_cycle()		{ return (flags & LEAFCYCLE)?1:0;}

	void	set_lighting(int i) { BIT_SET(flags,LIGHTING,i);}
	int	    lighting_enabled()	{ return (flags & LIGHTING)?lighting:0;}

	void	set_alpha(int i)	{ BIT_SET(flags,ALPHATEXS,i);}
	int	    alpha()				{ return (flags & ALPHATEXS)?1:0;}

	void	set_textures(int i) { BIT_SET(flags,TEXTURES,i);}
	int	    textures()		    { return (flags & TEXTURES)?1:0;}

	void	set_colors(int i)	{ BIT_SET(flags,COLORS,i);}
	int	    colors()		    { return (flags & COLORS)?1:0;}

	void	set_waterpass(int i)	{ BIT_SET(flags,WATERPASS,i);}
	int	    waterpass()		    { return (flags & WATERPASS)?1:0;}

	void	set_fog(int i)		{ BIT_SET(flags,VISVFOG,i);}
	int	    fog()		    	{ return (flags & VISVFOG)?1:0;}

	void	set_visbumps(int i)	{ BIT_SET(flags,VISBMPS,i);}
	int	    visbumps()		    { return (flags & VISBMPS)?1:0;}

	void	set_vistexs(int i)	{ BIT_SET(flags,VISTEXS,i);}
	int	    vistexs()		    { return (flags & VISTEXS)?1:0;}

	void	set_erosion(int i)	{ BIT_SET(flags,EROSION,i);}
	int	    erosion()		    { return (flags & EROSION)?1:0;}

	void	set_transparant(int i){ BIT_SET(flags,TRANSPARANT,i);}
	int		transparant()		{ return (flags & TRANSPARANT)?1:0;}

	void	set_sprites(int i)	{ BIT_SET(flags,SPRITES,i);}
	int		sprites()			{ return (flags & SPRITES)?1:0;}

	void	set_vchecked()		{ BIT_ON(flags,VCHKED);}
	void	clr_vchecked()		{ BIT_OFF(flags,VCHKED);}
	int		vchecked()			{ return flags & VCHKED;}

	void	set_bchecked()		{ BIT_ON(flags,BCHKED);}
	void	clr_bchecked()		{ BIT_OFF(flags,BCHKED);}
	int		bchecked()			{ return flags & BCHKED;}

	void	set_need_adapt()	{ BIT_ON(flags,NEEDADAPT);}
	void	clr_need_adapt()	{ BIT_OFF(flags,NEEDADAPT);}
	int	    need_adapt()		{ return flags & NEEDADAPT;}

	void	set_idmap(int i)	{ BIT_SET(flags,IDMAP,i);}
	int	    idmap()		    	{ return (flags & IDMAP)?1:0;}

	void	set_visids(int i)	{ BIT_SET(flags,VISIDS,i);}
	int	    visids()		    { return (flags & VISIDS)?1:0;}

	void	set_end_adapt(int i)	{ BIT_SET(flags,ENDADAPT,i);}
	int	    end_adapt()		    { return (flags & ENDADAPT)?1:0;}

	int     geometry()          { return (flags & GEOMETRY)?1:0;}
	void	set_geometry(int i)	{ BIT_SET(flags,GEOMETRY,i);}


	int		size;
	int		links;
	int 	cycles;
	int 	vnodes;

	int	    flags;
	int     frontface;
	int     lighting;
    bool 	idtest;

	double  zn,zf;
	double	symmetry;
	double	radius;
	double	smax,smin;		// cell size limits
	double	dmax,dmin;		// distance limits
	double	hscale,hmin,hmax,hrange;
	double  resolution;
	double  gmax;		// color gradient diff threshold
	double  cmax;		// curvature threshold (sharp)
	double  cmin;		// curvature threshold (smooth)
	double  minext;		// minimum cell extent (pixels) in adapt pass

	Object3D   *object;

	Map(double r);
	~Map();
	virtual Point point(double t, double p, double r);
	virtual void  set_scene();
	virtual void  set_resolution();

	MapNode        *root_node() { return npole; }
	MapNode        *active_node() { return last; }
	MapNode        *current_node() { return current; }
	void            clrflags()  { flags=0;}
	double          cell_size();
	void            visit(void (MapNode::*f) ());
	void            visit(void (*f) (MapNode *));
	void            visit_all(void (MapNode::*f) ());
	void            visit_all(void (*f) (MapNode *));
	void            make();
	void            find_limits();
	void            free();
	void            render();
	void            render_lines();
	void            render_points();
	void            render_solid();
	void            render_shaded();
	void            render_as_point();
	void            render_bounds();
	void            render_refs();
	void            render_surface();
	void            render_raster();
	void            render_zvals();
	void            shadow_normals();
	void            render_triangle_list();
	void            render_select();
	void            make_current();
	void            adapt();
	void            vischk(bool b);
	void            make_visbox();
	void 			init_tables();
	void 			set_face_distance();
	double          height(double t, double p);
	double          elevation(double t, double p);
	MapNode        *locate(double t, double p);
	MapNode        *makenode(MapNode * parent, uint t, uint p);
	MapNode        *makenode(MapNode * parent, double t, double p);
	void            get_info();
	void 			invalidate_normals();
	void 			validate_normals();
	bool 			setProgram();
	static int      tessLevel();
	static int      setTessLevel(int n);
	bool            setGeometryDefs();
	bool            setGeometryPrgm();
	bool            hasGeometry();

};

class RingMap : public Map
{
public:
	double width;
	RingMap(double r, double w) : Map(r) {lighting=0;width=w;}

	Point point(double t, double p, double r);
	void set_scene();
};
extern Map	*TheMap;
extern double   ptable[];

#define CELLSIZE(i) PI*TheMap->radius*ptable[i]

#endif

