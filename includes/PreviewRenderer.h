
//  PreviewRenderer.h

#ifndef _PVRENDERER
#define _PVRENDERER

#include "LightClass.h"
#include "ObjectClass.h"
#include "OrbitalClass.h"
#include "Renderer.h"
#include "NodeIF.h"
#include "MapClass.h"
#include <time.h>

class PreviewObject;

enum{	// vflags bits
	AUTOZOOM		= 0x00000001,
	ENABLE_LIGHT	= 0x00000002,
	LEFT_LIGHT		= 0x00000004,
	PVOBJECT		= 0x00000008,
	OVERHEAD		= 0x00000010,
	CHANGED_MAP		= 0x10000000,
	DEFAULTS        = AUTOZOOM|ENABLE_LIGHT|OVERHEAD
};
class PreviewRenderer : public Renderer
{
protected:
	int rflags;
	PreviewObject   *object;
	NodeIF          *original_node;
	TNode           *root;
	Point			lpoint;
public:	
	PreviewRenderer();
	~PreviewRenderer();
	int set_preview_object(NodeIF *n);
	NodeIF *get_preview_object();
	void set_changed_map(int i)			{ BIT_SET(rflags,CHANGED_MAP,i);}
	int changed_map()					{ return rflags & CHANGED_MAP;}
	void show_preview_object(int i)		{ BIT_SET(rflags,PVOBJECT,i);}
	int shows_preview_object()			{ return rflags & PVOBJECT;}
	void set_lighting(int i)			{ BIT_SET(rflags,ENABLE_LIGHT,i);}
	int lighting()						{ return rflags & ENABLE_LIGHT;}
	void set_light(Point p)				{ lpoint=p;}
	void save_state();
	void set_state();
	void restore_state();
	void set_object_size(double f);
	void render();

	void change_view(int,double);

	void set_oblique_view();
	void set_overhead_view();
	void init_view();
	void set_light();
	void set_quality(int);
};

class PreviewMap : public Map
{
protected:
public:	
	PreviewMap(double r) : Map(r){}
	void set_scene();
	void set_resolution();
};

class PreviewObject : public ObjectNode
{
protected:
public:	
	PreviewMap *map;
	NodeIF     *tnode;
	PreviewObject();
	~PreviewObject();
	TerrainMgr *getMgr();
	void set_surface(TerrainData &d); 
	void set_preview_object(NodeIF *n);
	NodeIF *get_preview_object();
	void rebuild();
	void render();
	double resolution();
};
#endif
