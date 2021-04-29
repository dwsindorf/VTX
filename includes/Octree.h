// Octree.h

#ifndef _OCTREE
#define _OCTREE

#include "defs.h"
#include "ObjectClass.h"
#include "ColorClass.h"
#include "NoiseClass.h"
#include "Util.h"

class OctTree;
class StarTree;
class Image;
class TNode;

//************************************************************
// OctNode class
//************************************************************
class OctNode
{
	enum {
		SPLIT=1,
		COMBINE=2
	};

protected:
	typedef struct odata {
	uint  lvl	    : 5;	   // subdivision level
	uint  slvl	    : 2;	   // subdivision level target
  	uint  tests    : 2;       // test flags
  	uint  visible  : 1;       // visibility flag
  	uint  clipped  : 1;       // visibility flag
  	uint  flags    : 2;       // class flag
 } odata;

	odata info;			// misc
	float _density;
	float _value;
	int lx,ly,lz;      // position
	OctNode **child;	// children
public:
	OctNode(uint,int,int,int);
	virtual ~OctNode();

	int  children()				{ return child?8:0;}
	uint level()				{ return info.lvl;}
	uint slevel()				{ return info.slvl+1;}
	uint order()				{ return info.lvl;}
	void set_level(uint i)		{ info.lvl=i;}
	void set_slevel(uint i)	{ info.slvl=i-1;}
	double cellsize()			{ return (double)(1<<info.lvl);}

	void set_density(double d)	{ _density=(float)d;}
	double density()			{ return _density;}
	double value();
	void setValue();

	bool isActive()				{ return !child && visible() && !clipped() && density();}

	void clr_tests()			{ info.tests=0;}
	void set_ctest()			{ BIT_ON(info.tests,COMBINE);}
	void set_stest()			{ BIT_ON(info.tests,SPLIT);}
	int ctest()					{ return info.tests & COMBINE?1:0;}
	int stest()					{ return info.tests & SPLIT?1:0;}

	void set_visible()			{ info.visible=1;}
	void clr_visible()			{ info.visible=0;}
	int visible()				{ return info.visible;}

	int clip_test();
	void set_clipped()			{ info.clipped=1;}
	void clr_clipped()			{ info.clipped=0;}
	int clipped()				{ return info.clipped;}

	void set_flags(uint d)		{ info.flags=d;}
	uint flags()				{ return info.flags;}

	OctNode *locate(Point &);
	int contains(Point &);
	void set_tests();

	void visit(void  (OctNode::*func)());
	void visit(void (*func)(OctNode*));

	Point  center();
	double size();
	virtual void split();
	virtual void combine();
	virtual void render();
	virtual void adapt();
	virtual void select();
	virtual Point randomize();
	virtual double calc_density(Point &);
	virtual Point point();

	friend class OctTree;
};

//************************************************************
// DensityNode class
//************************************************************
class DensityNode : public OctNode
{
public:
	DensityNode(uint,int,int,int);
	virtual void render();
};

//************************************************************
// StarNode class
//************************************************************
class StarNode : public DensityNode
{
public:
	StarNode(uint,int,int,int);
	void render();
	void select();
	Point point();
	int stars(Point &mpt);
	int brightest(Point &mpt);
};

//************************************************************
// OctTree class
//************************************************************
class OctTree
{
protected:

	int flags;
	OctNode *root;
	uint  levels;
	void   free();
	static Point  last_view;
	int changed_view();
public:
	Object3D   *object;
	static LinkedList<OctNode*> unsorted;
	static ValueList<OctNode*> sorted;
	static double max_value;
	static bool sort_nodes;

	int    nodes;
	int    vtype;
	int    maxnodes;
	double z1,z2;
	double units;
	double wscale;
	double gscale;
	double nscale;
	double size;
	double dispersion;
	double diffusion;
	double shape_exterior;
	double shape_interior;
	double shape_sharpness;
	double shape_solidity;
	double resolution_value;
	double resolution_scale;
	double density_value;
	double density_scale;
	double normal_ext;
	double hidden_ext;
	double smooth_ext;
	double combine_factor;
	double maxsize;
	double minsize;
	double fullsize;
	Point  origin;
	Point  selpt;

	void sortNodes();

	OctTree(double l, double s);
	~OctTree();

	void visit(void (OctNode::*f) ())	{ root->visit(f);}
	void visit(void (*f) (OctNode *))	{ root->visit(f);}
	double density()					{ return density_value*density_scale;}
	double resolution()					{ return resolution_value*resolution_scale;}
	uint level()						{ return levels;}
	virtual void make_current();
	virtual OctNode  *build(uint,int,int,int);
	virtual void scale();
    virtual int scale(double &zn, double &zf);
	virtual void normalize(Point &);
	virtual void init();
	virtual void render();
	virtual void adapt();
	virtual void select();
	virtual void rebuild();
	virtual int test_clipped(Point &p);
	virtual double test_surface(Point &p);
	virtual void init_node(OctNode *);
	virtual int inside();
	virtual Point point(OctNode *);
	virtual void draw_selpt();
	virtual const char *typeName();
};

//************************************************************
// DensityTree class
//************************************************************
class DensityTree : public OctTree
{
protected:
	enum {
		COLOR_EXPR   =0x0010,
		TEXTURE_EXPR =0x0020
	};

	double rmax,cf;
	double c1,a1,a2;
public:
	double gradient;
	double inner_radius;
	double outer_radius;
	double compression;
	double twist_angle;
	double twist_radius;
	double noise_saturation;
	double noise_cutoff;
	double noise_vertical;
	double noise_amplitude;
	double center_radius;
	double center_bias;

	double color_mix;
	double color_bias;
	Color  dcols[8];
	int ncols;

	static const char *def_noise_expr;
	static const char *def_color_list;
	static const char *def_color_expr;

	DensityTree(double l, double u);
   ~DensityTree();

   	void set_color_expr(int i)		{ BIT_SET(flags,COLOR_EXPR,i);}
	void set_texture_expr(int i)	{ BIT_SET(flags,TEXTURE_EXPR,i);}
	int color_expr()				{ return (flags & COLOR_EXPR)?1:0;}
	int texture_expr()				{ return (flags & TEXTURE_EXPR)?1:0;}

	virtual OctNode *build(uint,int,int,int);
	virtual void init_node(OctNode *);

	virtual void init();
	virtual void scale();
    virtual int scale(double &zn, double &zf);
	virtual void make_current();
	virtual void render();
	virtual double test_surface(Point &p);
	virtual int inside();
};

//************************************************************
// StarTree class
//************************************************************
class StarTree : public DensityTree
{
protected:
	enum {
		RENDER_FG=0x1000,
		RENDER_BG=0x2000
	};

public:
	static double fgpt2;
	static double bgpt2;
	static double bgscale;
	static double fgscale;

	StarTree(double l);
	int    fgpts;
	int    bgpts;
	double bgpt1;
	double fgpt1;
	double fgfar;
	double bg_density;
	double ptscale;
	double nova_density;
	double nova_size;
	double fg_random;
	double bg_random;

	void set_render_fg(int i) { BIT_SET(flags,RENDER_FG,i);}
	void set_render_bg(int i) { BIT_SET(flags,RENDER_BG,i);}
	int render_fg()				{ return (flags & RENDER_FG)?1:0;}
	int render_bg()				{ return (flags & RENDER_BG)?1:0;}
	OctNode *build(uint,int,int,int);
	void scale();
    int scale(double &zn, double &zf);
	void make_current();
	void render();
	void select();
	double test_surface(Point &p);
	void draw_selpt();
	const char *typeName();
};


#endif

