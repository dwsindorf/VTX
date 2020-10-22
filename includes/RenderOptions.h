// RenderOptions.h

#ifndef _RENDER_OPTIONS
#define _RENDER_OPTIONS

#include "defs.h"

#define MAXSTATES  16
typedef struct flagsdata {
	unsigned int  mode	    : 16;	// render mode
	unsigned int  showface	: 2;	// face
	unsigned int  markers	: 2;	// markers
	unsigned int  showsed	: 2;	// show sediment
	unsigned int  unused	: 4;	// unassigned
} flagsdata;

typedef struct renderdata {
	unsigned int  fogsel	: 2;	// fog options
	unsigned int  colors	: 4;	// colors test mode
	unsigned int  water	    : 2;	// water mode
	unsigned int  display	: 7;	// info select
	unsigned int  valid	    : 1;	// unassigned
	unsigned int  lmode	    : 2;	// light mode
	unsigned int  unused	: 14;	// options
} renderdata;

enum{ //light mode
	PHONG_BLINN		= 0x00,
	PHONG			= 0x01,
	MIXED			= 0x02
	};

enum{ //showface
	FRONT			= 0x01,
	BACK			= 0x02
	};

enum{
	SHOW_BOUNDS		= 0x0001,
	SHOW_REFS		= 0x0002
	};

enum{
	SHOW_SHADER		= 0x0001,
	SHOW_SOLID		= 0x0002,
	SHOW_LINES		= 0x0004,
	SHOW_POINTS		= 0x0008,
	SHOW_ZVALS		= 0x0010,
	SHOW_IDS		= 0x0020,
	SHOW_NORMALS	= 0x0040,
	SHOW_RASTER		= 0x0080,
	SHOW_FRONT		= SHOW_SOLID|SHOW_POINTS|SHOW_LINES|SHOW_SHADER,
	SHOW_BUFFER		= SHOW_ZVALS|SHOW_IDS|SHOW_RASTER|SHOW_NORMALS
	};

enum{  //colors
	CSIZE			= 0x01,
	CNODES			= 0x02,
	VNODES	    	= 0x03,
	LNODES			= 0x04,
	ENODES			= 0x05,
	MNODES			= 0x06,
	RNODES			= 0x07,
    SNODES          = 0x08
	};

enum{  //fogsel
	LINEAR			= 0x00,
	EXP				= 0x01,
	EXP2			= 0x02,
	POW			    = 0x03
	};

enum{  // info displayay options
	OBJINFO			= 0x01,
	MAPINFO			= 0x02,
	OCTINFO			= 0x04,
	CRTRINFO        = 0x08,
	TRNINFO         = 0x10,
	RASTINFO		= 0x20,
	NODEINFO		= 0x40,
	ALLINFO         = 0x7f
	};

class RenderOptions
{
enum{
	INVALID_POINTS   = 0x1,
	INVALID_NORMALS  = 0x2,
	INVALID_TEXTURES = 0x4
};

enum{ // flags
	SMOOTH		    = 0x00000001,
	LIGHTSHADING	= 0x00000002,
	DEALIAS	    	= 0x00000004,
	TWILITE	    	= 0x00000008,
	HAZE			= 0x00000010,
	FOG	    	    = 0x00000020,
	TEXTURES		= 0x00000040,
	AVENORMS		= 0x00000080,
	WATERSHOW		= 0x00000100,
	WATERLINES		= 0x00000200,
	BUMPMAPS		= 0x00000400,
	DRAW_NVIS		= 0x00000800,
	STARTEX		    = 0x00001000,
	SPECTEX		    = 0x00002000,
	RAYTRACE		= 0x00004000,
	MULTITEXS		= 0x00008000,
	GEOMETRY		= 0x00010000,
	DEFAULTS        = SMOOTH
					|LIGHTSHADING
					|HAZE
					|FOG
					|TWILITE
					|TEXTURES
					|BUMPMAPS
					|WATERSHOW
					|SPECTEX
					|RAYTRACE
					|MULTITEXS
					|STARTEX
					|AVENORMS
					|DEALIAS
					|GEOMETRY
	};

	flagsdata	flags;
	renderdata	info;
	int renderflags;
	int valid;
	double _hscale;
	double _haze;
	double _fog;
	double _blend;
	double _darken;
	Color  _haze_color;
	Color  _fog_color;
	double _image_quality;
	int mode_state[MAXSTATES];
	int modeptr;
	int render_state[MAXSTATES];
	int renderptr;
public:
	RenderOptions()             {  set_defaults() ; }
	void set_defaults()         { renderflags=DEFAULTS;
                                  info.fogsel=EXP;
                                  info.lmode=PHONG_BLINN;
                                  info.colors=0;
                                  info.display=0;
                                  valid=0;
                                  _hscale=1;
                                  _haze=1.0;
                                  _haze_color=Color(1.0,1.0,1.0);
                                  _fog_color=Color(1.0,1.0,1.0);
                                  _fog=1.0;
                                  _blend=0.5;
                                  _darken=0.5;
                                  _image_quality=95.0;
                                  flags.mode=SHOW_SHADER;
                                  flags.showface=FRONT;
                                  flags.markers=SHOW_REFS;
                                  flags.showsed=0;
                                  modeptr=0;
                                }
	void pushmode(int newstate) {
		mode_state[modeptr]=flags.mode;
		if(modeptr<MAXSTATES-1)
			modeptr++;
		flags.mode=newstate;
	}
	void popmode() {
		if(modeptr>0)
			modeptr--;
		flags.mode=mode_state[modeptr];
	}
	void push() {
		render_state[renderptr]=renderflags;
		if(renderptr<MAXSTATES-1)
			renderptr++;
	}
	void pop() {
		if(renderptr>0)
			renderptr--;
		renderflags=render_state[renderptr];
	}
	void set_image_quality(double q) { _image_quality=q;}
	double image_quality() 		{ return _image_quality;}
	void set_display(int c)     { BIT_ON(info.display,c);}
	void clr_display(int c)     { BIT_OFF(info.display,c);}
	int  display(int c)			{ return info.display & c?1:0;}

	void set_dealias(int c)		{ BIT_SET(renderflags,DEALIAS,c);}
	int  dealias()				{ return renderflags & DEALIAS?1:0;}

	void set_raytrace(int c)	{ BIT_SET(renderflags,RAYTRACE,c);}
	int  raytrace()				{ return renderflags & RAYTRACE?1:0;}

	void set_multitexs(int c)	{ BIT_SET(renderflags,MULTITEXS,c);}
	int  multitexs()			{ return renderflags & MULTITEXS?1:0;}

	void set_startex(int c)		{ BIT_SET(renderflags,STARTEX,c);}
	int  startex()				{ return renderflags & STARTEX?1:0;}

	void set_geometry(int c)	{ BIT_SET(renderflags,GEOMETRY,c);}
	int  geometry()				{ return renderflags & GEOMETRY?1:0;}

	void set_spectex(int c)		{ BIT_SET(renderflags,SPECTEX,c);}
	int  spectex()				{ return renderflags & SPECTEX?1:0;}

	void set_lighting(int c)	{ BIT_SET(renderflags,LIGHTSHADING,c);}
	int  lighting()				{ return renderflags & LIGHTSHADING?1:0;}

	void set_water_show(int c)	{ BIT_SET(renderflags,WATERSHOW,c);}
	int  show_water()			{ return renderflags & WATERSHOW?1:0;}

	void set_water_lines(int c)	{ BIT_SET(renderflags,WATERLINES,c);}
	int  water_lines()			{ return renderflags & WATERLINES?1:0;}

	int  haze()					{ return renderflags & HAZE?1:0;}
	void set_haze(int c)		{ BIT_SET(renderflags,HAZE,c);}

	int  fog()					{ return renderflags & FOG?1:0;}
	void set_fog(int c)			{ BIT_SET(renderflags,FOG,c);}

	int  twilite()				{ return renderflags & TWILITE?1:0;}
	void set_twilite(int c)		{ BIT_SET(renderflags,TWILITE,c);}

	int  bumps()				{ return renderflags & BUMPMAPS?1:0;}
	void set_bumps(int c)		{ BIT_SET(renderflags,BUMPMAPS,c);}

	int  draw_nvis()		    { return renderflags & DRAW_NVIS?1:0;}
	void set_draw_nvis(int c)	{ BIT_SET(renderflags,DRAW_NVIS,c);}

	int  avenorms()				{ return renderflags & AVENORMS?1:0;}
	void set_avenorms(int c)	{ if(c != avenorms())
									invalidate_normals();
									BIT_SET(renderflags,AVENORMS,c);
								}
	int  linfog()				{ return info.fogsel==LINEAR?1:0;}
	void set_linfog(int c)		{ info.fogsel=c?LINEAR:EXP;}

	int  textures()				{ return renderflags & TEXTURES?1:0;}
	void set_textures(int c)	{ BIT_SET(renderflags,TEXTURES,c);}

	void set_smooth_shading(int c){ BIT_SET(renderflags,SMOOTH,c);}
	int  smooth_shading()		{ return renderflags & SMOOTH?1:0;}

	void set_colors(unsigned int c)	{ info.colors=c;}
	int  colors()				{ return info.colors;}

	void set_light_mode(unsigned int c)	{ info.lmode=c;}
	int  light_mode()			{ return info.lmode;}

	void invalidate()			{ BIT_ON(valid,INVALID_NORMALS|INVALID_POINTS|INVALID_TEXTURES);}
	void invalidate_normals()	{ BIT_ON(valid,INVALID_NORMALS);}
	void validate_normals()		{ BIT_OFF(valid,INVALID_NORMALS);}
	bool invalid_normals()		{ return valid & INVALID_NORMALS?true:false;}
	void invalidate_points()	{ BIT_ON(valid,INVALID_POINTS);}
	void validate_points()		{ BIT_OFF(valid,INVALID_POINTS);}
	bool invalid_points()		{ return valid & INVALID_POINTS?true:false;}
	void invalidate_textures()	{ BIT_ON(valid,INVALID_TEXTURES);}
	void validate_textures()	{ BIT_OFF(valid,INVALID_TEXTURES);}
	bool invalid_textures()	    { return valid & INVALID_TEXTURES?true:false;}

	double haze_value()			{ return _haze;}
	void set_haze_value(double h){ _haze=h;}
	Color haze_color()			{ return _haze_color;}

	double fog_value()			{ return _fog;}
	void set_fog_value(double h){ _fog=h;}

	double blend()				{ return _blend;}
	void set_blend(double h)	{ _blend=h;}

	double darken()				{ return _darken;}
	void set_darken(double h)	{ _darken=h;}

	int  fogmode()				{ return info.fogsel;}
	void set_fogmode(int i)		{ info.fogsel=i;}

    // Flags

	int  backface()				{ return flags.showface & BACK;}
	int  frontface()			{ return flags.showface & FRONT;}
    int  cullface()				{ return flags.showface==(FRONT|BACK)?0:1;}
    int  showall()				{ return flags.showface==(FRONT|BACK)?1:0;}
	void set_back()				{ flags.showface=BACK;}
	void set_front()			{ flags.showface=FRONT;}
	void set_both()				{ flags.showface=FRONT|BACK;}
	void set_none()				{ flags.showface=0;}
	int  showface()             { return flags.showface;}
	void set_showface(int c)    { flags.showface=c;}

	void set_bounds(int c)		{ BIT_SET(flags.markers,SHOW_BOUNDS,c);}
	void set_refs(int c)		{ BIT_SET(flags.markers,SHOW_REFS,c);}
	int  draw_refs()			{ return flags.markers & SHOW_REFS;}
	int  draw_bounds()			{ return flags.markers & SHOW_BOUNDS;}
	int  markers()              { return flags.markers;}
	void set_markers(int c)     { flags.markers=c;}

    void set_showsed(int i)     { flags.showsed=i;invalidate();}
    int showsed()				{ return flags.showsed;}
	int  mode()                 { return flags.mode;}
	void set_mode(int c)        { flags.mode=c;}
	void show_points()			{ flags.mode=SHOW_POINTS;}
	void show_lines()			{ flags.mode=SHOW_LINES;}
	void show_solid()			{ flags.mode=SHOW_SOLID;}
	void show_zvals()			{ flags.mode=SHOW_ZVALS;}
	void show_ids()				{ flags.mode=SHOW_IDS;}
	void show_raster()			{ flags.mode=SHOW_RASTER;}
	void show_normals()			{ flags.mode=SHOW_NORMALS;}
	void show_shaded()			{ flags.mode=SHOW_SHADER;}
	int  draw_lines()			{ return flags.mode==SHOW_LINES?1:0;}
	int  draw_solid()			{ return flags.mode==SHOW_SOLID?1:0;}
	int  draw_points()			{ return flags.mode==SHOW_POINTS?1:0;}
	int  draw_zvals()			{ return flags.mode==SHOW_ZVALS?1:0;}
	int  draw_ids()				{ return flags.mode==SHOW_IDS?1:0;}
	int  draw_raster()			{ return flags.mode==SHOW_RASTER?1:0;}
	int  draw_shaded()			{ return flags.mode==SHOW_SHADER?1:0;}
	int  draw_normals()			{ return flags.mode==SHOW_NORMALS?1:0;}

	int  draw_front()			{ return flags.mode & SHOW_FRONT;}
	int  draw_back()			{ return flags.mode & SHOW_BUFFER;}

};

extern	RenderOptions    Render;

#endif

