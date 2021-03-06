// ObjectClass.h

#ifndef _OBJECTCLASS
#define _OBJECTCLASS

#include "GLglue.h"
#include "PointClass.h"
#include "ColorClass.h"
#include "ListClass.h"
#include "NodeIF.h"
#include <math.h>

class Map;
class MapData;
class TerrainData;
class Bounds;

enum{	// visibility bits
	OFFSCREEN	   = 0x0,
	INSIDE	       = 0x1,
	OUTSIDE	       = 0x2
};

enum{  // visibility bits
	SEXCL          	= 0x01, // exclude for scale
	PEXCL          	= 0x02, // exclude for select
	SHEXCL			= 0x04, // exclude for shadows
	ALLEXCL        	= 0x0f  // all excludes
};

enum{  // flag bits
	BGOBJ	       	= 0x02, // far objects flag
	CCLIP	       	= 0x04  // clip children flag
};

enum{  // group bits
	FG1	       	= 0x0,  // furthest foreground object
	FG0	       	= 0x1,  // nearest foreground object
	BG0	       	= 0x2,  // nearest background object
	BG1	       	= 0x3,  // normal background object
	BG2	   		= 0x4,  // environment background object
	BG3	   		= 0x5,  // furthest local background object
	BG4	   		= 0x6,  // background objects
	BG5	   		= 0x7,  // background objects
	BG6	   		= 0x8,  // background objects
	BGS         = 0x9,  // background shadow passTN_TYPES
	FGS         = 0xa,  // foreground shadow pass
	FGR         = 0xb,  // outside raster pass
	BGFAR       = BG6,  // backdrop objects
	BGMAX       = BG5,	// furthest std bg objects
	BGMIN       = BG0,	// nearest std bg objects
	FGMIN       = FG0,	// nearest fg objects
	FGMAX       = FG1	// furthest fg objects
};


typedef struct objstruct {
	unsigned int s      : 4;  // selection
	unsigned int g      : 4;  // group type
	unsigned int r      : 2;  // render side
	unsigned int v      : 4;  // visibility
	unsigned int x      : 4;  // excludes
	unsigned int f      : 4;  // flags
	unsigned int a      : 4;  // constructor args
} objstruct;

typedef union objflags {
	objstruct	 s;
	int          l;
} objflags;

class Object3D : public NodeIF
{
enum{
	SELECTED	   = 0x1,
	INCLUDED	   = 0x2,
	EXCLUDED	   = 0x4
};
enum{
	LGROUP	    = 0x1,
	VGROUP	    = 0x2,
	SGROUP	    = 0x4,
	NGROUP	    = 0x8
};

protected:
public:
	Point		point;			// 3D world coordinates
	double		size;			// world scale factor
	objflags    status;

	Object3D()					 { size=0.0;status.l=0;}
	Object3D(double s) : size(s) { status.l=0;}
	virtual ~Object3D();

	double value()		 { return size;} // for sorting

	// shader functions

	virtual void loadProgram()		{}
	virtual void loadVars()			{}
	virtual bool isObject()         { return false;}
	virtual bool hasTexture()       { return false;}
	virtual void setActiveTex(unsigned int i){ }
	virtual bool tex1D()			{ return false;}
	virtual bool force_adapt()     { return false;}

	//virtual bool isShell()          { return false;}

	virtual void animate();
	virtual void project(RECT&);
	virtual void init();        // initialize variables
	virtual void adapt();       // adapt to current conditions
	virtual void adapt_object();// adapt the object only
	virtual void render();      // draw the object and children
	virtual void render_object();// draw the object only
	virtual void init_adapt();  // adapt setup
	virtual void init_render(); // render setup
	virtual void set_lights();  // radient objects set scene lighting
	virtual void set_lighting();// non-radient objects modify lighting
	virtual void set_point();	// set point to global position
	virtual void locate();		// update point with 3D coordinates
	virtual void init_view();	// set initial view parameters
	virtual void set_view();	// update viewpoint for this pass
	virtual void set_ref();		// model to global translation/rotations
	virtual void set_scene();	// set scene view parameters
	virtual Bounds *bounds();	// set view bounds
	virtual int scale(double&, double&);// znear zfar
	virtual void save(FILE*);
	virtual void set_color(Color);
	virtual void map_color(MapData*,Color&);
	virtual Color color();
	virtual double extent();    // how many pixels are projected
	virtual void visit(void  (Object3D::*func)());
	virtual void visit(void  (*f)(Object3D*));
	virtual void visitAll(void  (Object3D::*func)());
	virtual void visitAll(void  (*f)(Object3D*));
	virtual const char *name();
	virtual int  type();
	virtual void set_surface(TerrainData&);	// set 3D attributes
	virtual double resolution();
	virtual void select();
	virtual int select_pass();
	virtual int adapt_pass();
	virtual int render_pass();
	virtual int shadow_pass();
	virtual int selection_pass();
	virtual void free();
	virtual int included();
	virtual int cells();
	virtual int cycles();
	virtual void rebuild();
 	virtual void set_geometry();
 	virtual Object3D *getObjParent();
 	virtual Map *getMap();
	virtual void draw_rect();
	virtual double depth();
	virtual double radius();
	virtual double height(double t, double p);
	virtual double far_height();
	virtual double max_height();
	virtual Point screen();
	virtual void set_focus(Point &p);
	virtual void move_focus(Point &p);
	virtual Point get_focus(void *);

	// NodeIF methods

	int typeValue()			    { return type();}
	const char *typeName()		{ return name();}
	void saveNode(FILE *f)		{ save(f);}

	void setvis(int v)			{ status.s.v=v;}
	int getvis()				{ return status.s.v;}
	int inside()				{ return status.s.v==INSIDE ? 1:0;}
	int outside()				{ return status.s.v==OUTSIDE ? 1:0;}
	int onscreen()				{ return status.s.v ? 1:0;}
	int offscreen()				{ return status.s.v ? 0:1;}

	int bgfar()					{ return (status.s.f & BGOBJ)?1:0;}
	void set_far()			    { BIT_ON(status.s.f,BGOBJ);}
	void set_near()			    { BIT_OFF(status.s.f,BGOBJ);}

	int clip_children()         { return status.s.f & CCLIP;}
	void set_clip_children()    { BIT_ON(status.s.f,CCLIP);}

	void all_include()          { BIT_OFF(status.s.x,ALLEXCL);}
	void all_exclude()          { BIT_ON(status.s.x,ALLEXCL);}

	void select_include()		{ BIT_OFF(status.s.x,PEXCL);}
	void select_exclude()		{ BIT_ON(status.s.x,PEXCL);}
	int  allows_selection()		{ return (status.s.x & PEXCL)?0:1;}

	void scale_include()		{ BIT_OFF(status.s.x,SEXCL);}
	void scale_exclude()		{ BIT_ON(status.s.x,SEXCL);}
	int  allows_scale()			{ return (status.s.x & SEXCL)?0:1;}

	void shadows_include()		{ BIT_OFF(status.s.x,SHEXCL);}
	void shadows_exclude()		{ BIT_ON(status.s.x,SHEXCL);}
	int  allows_shadows()		{ return (status.s.x & SHEXCL)?0:1;}

	void select_pass(int t);
	void clear_pass(int t);

	void set_local_group()		{ BIT_ON(status.s.g,LGROUP);}
	void clr_local_group()		{ BIT_OFF(status.s.g,LGROUP);}
	int  local_group()			{ return status.s.g&LGROUP;}

	void set_view_group()		{ BIT_ON(status.s.g,VGROUP);}
	void clr_view_group()		{ BIT_OFF(status.s.g,VGROUP);}
	int  view_group()			{ return status.s.g&VGROUP;}

	void set_near_group()		{ BIT_ON(status.s.g,NGROUP);}
	void clr_near_group()		{ BIT_OFF(status.s.g,NGROUP);}
	int  near_group()			{ return status.s.g&NGROUP;}

	void set_shadow_group()		{ BIT_ON(status.s.g,SGROUP);}
	void clr_shadow_group()		{ BIT_OFF(status.s.g,SGROUP);}
	int  shadow_group()			{ return status.s.g&SGROUP;}

	int groups()                { return status.s.g;}
	void set_groups(int i)		{ status.s.g=i;}
	void clr_groups()			{ status.s.g=0;}

	void fg_include()			{ BIT_ON(status.s.s,INCLUDED);}
	void fg_exclude()			{ BIT_ON(status.s.s,EXCLUDED);}
	int  fg_included()			{ return status.s.s&INCLUDED;}
	void set_selected()			{ BIT_ON(status.s.s,SELECTED);}
	void clr_selected()			{ BIT_OFF(status.s.s,SELECTED);}
	int  selected()				{ return status.s.s&SELECTED;}

	void set_cargs(int i)		{ status.s.a=i;}
	int cargs()					{ return status.s.a;}
};

class ObjectNode : public Object3D
{
	friend class Scene;
public:
	LinkedList<Object3D *> children;
	ObjectNode *parent;
	ObjectNode() : Object3D()				{	parent=0;}
	ObjectNode(double s) : Object3D(s)		{   parent=0;set_cargs(1);}
	ObjectNode(ObjectNode *m);
	ObjectNode(ObjectNode *m, double s);

	virtual bool isObject()        			 { return typeClass()&ID_OBJECT;}

	virtual int numChildren()				{ return children.size;}

	// NodeIF methods

	virtual NodeIF *getParent()					{ return parent;}
 	virtual Object3D *getObjParent()			{ return parent;}
	virtual void setParent(NodeIF *p)			{ parent=(ObjectNode*)p;}
	virtual bool isLeaf()						{ return false;}

	Object3D *getChild(int vtype){
    	for(int i=0;i<children.size;i++){
    		Object3D *var=children[i];
    		if(var->type()==vtype)
    			return var;
     	}
    	return 0;
    }
    bool hasChild(int vtype){
    	for(int i=0;i<children.size;i++){
    		Object3D *var=children[i];
    		if(var->type()==vtype)
    			return true;
     	}
    	return false;
    }

	virtual bool hasChildren() {return children.size?true:false;}
	virtual int getChildren(LinkedList<NodeIF*>&l);
	virtual NodeIF *addChild(NodeIF *);
	virtual NodeIF *addAfter(NodeIF *b,NodeIF *c);
	virtual NodeIF *removeChild(NodeIF *);
	virtual NodeIF *replaceChild(NodeIF *c,NodeIF *n);

	virtual ~ObjectNode();
	virtual void save(FILE*);
	virtual void visit(void  (Object3D::*func)());
	virtual void visit(void  (*f)(Object3D*));
	virtual void visitChildren(void  (Object3D::*func)());
	virtual void visitChildren(void  (*f)(Object3D*));
	virtual void visitAll(void  (Object3D::*func)());
	virtual void visitAll(void  (*f)(Object3D*));
	virtual void rebuild();
	virtual void free()					    {	children.free();}
};

class ObjectMgr
{
public:
	LinkedList<Object3D*> list;
	ObjectMgr()					{}

	int size()						{ return list.size;}
	void save(FILE*);
	void free()						{	list.free();}
	void add(Object3D *v)			{	list.add(v);}
	void remove(Object3D *v)		{	list.remove(v);}
	Object3D*  operator[](int i)	{	return list[i];}
	Object3D*  ss()					{	return list.ss();}
	Object3D*  se()					{	return list.se();}
	Object3D*  at()					{	return list.at();}
	Object3D*  first()				{	return list.first();}
	Object3D*  last()			    {	return list.last();}
	Object3D*  next()				{	return list++;}
	Object3D*  set(Object3D *v);
	void visit(void  (Object3D::*func)());
	void visit(void  (*f)(Object3D*));
	void visitAll(void  (Object3D::*func)());
	void visitAll(void  (*f)(Object3D*));
	void visitNodes(void  (*f)(NodeIF*));

};


#endif

