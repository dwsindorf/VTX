
//  Renderer.h

#ifndef _RENDERER
#define _RENDERER

//#include "LightClass.h"
#include "ObjectClass.h"
//#include "OrbitalClass.h"
#include "ViewClass.h"
#include "GLSupport.h"
#include <time.h>

enum {
    DRAFT,
    NORMAL,
    HIGH,
    BEST
};

typedef struct renderer_options {
	unsigned int  keep_vars		: 1;	// keep variables
	unsigned int  keep_children    : 1;	// keep child nodes
} renderer_options;

class Renderer : public View, public GLSupport
{
protected:
	View       *last_view;
	void set_ambient();
	renderer_options ropts;
public:
	ObjectMgr  *objects;
	Color       backcolor;
	double      ambient;
	double      ambient_min;
	double      ambient_bias;
	double 		tex_mip;
	double 		color_mip;
	double 		bump_mip;
	double 		freq_mip;
	int         render_quality;
	int         generate_quality;
	int         small_width,small_ht;
	int         med_width,med_ht;
	int         big_width,big_ht;
	int         pan_width,pan_ht;
	int         wide_width,wide_ht;
	int         custom_width,custom_ht;
	int         current_size;
	int         current_width,current_ht;
	int			hborder;
	int			wborder;

	void set_keep_variables(int i)		{ ropts.keep_vars=i;}
	int keep_variables()				{ return ropts.keep_vars;}
	void set_keep_children(int i)		{ ropts.keep_children=i;}
	int keep_children()					{ return ropts.keep_children;}
    
	Renderer();
	~Renderer();

	void erase();
	void show_points();
	void show_lines();
	void show_solid();
	void reset_view();
	void set_ambient(double f);
	void set_glfog(double,double,double,Color);
	void resize(int w, int h);
	void set_size(int i);
	int get_size();
	void get_size(int &w, int &h);
	void set_size(int i,int,int);
	void get_size(int i,int &w,int &h);
	virtual void free();
	virtual void init();
	virtual void init_view();
	virtual void reset();
	virtual void gl_init();
	virtual void render();
	virtual void adapt();
	virtual void rebuild();
	virtual void rebuild_all();
	virtual void render_objects();
	virtual void set_quality(int i) 	{ render_quality=i;}
};
#endif
