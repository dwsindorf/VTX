// Raster.h

#ifndef _RASTER_H
#define _RASTER_H

#include "SceneClass.h"
#include "GLheaders.h"
#include <math.h>

class MapNode;
class MapData;

enum{ // rtype flags
	NORMALS	        = 0x01,
	AUXIMAGE	    = 0x02,
	SHADING			= 0x03,
	BUMPS		    = 0x04,
	SHADERS		    = 0x05,
};

enum{
	FIRST_LIGHT    	= 0x0000,
	LAST_LIGHT		= 0x0001,
	ALL_LIGHTS		= 0x0002
};

enum{
	FILTER_TYPE1    = 0x0000,
	FILTER_TYPE2	= 0x0001,
	FILTER_TYPE3	= 0x0002,
	FILTER_TYPE     = 0x0003,
	FILTER_EDGES    = 0x0010,
	FILTER_BG       = 0x0020,
	FILTER_STRETCH  = 0x0040,
	FILTER_SHOW     = 0x0080,
	FILTER_DEPTH    = 0x0100,
	FILTER_NORM     = 0x0200,
	FILTER_CLEAN    = 0x0400,
	FILTER_ANISO    = 0x0800,
	FILTER_DEFAULTS = FILTER_TYPE2|FILTER_EDGES|FILTER_NORM|FILTER_CLEAN|FILTER_ANISO
};

class RasterMgr
{
protected:
	static double BS;
	static double BF;

	enum{ // options
		FMAX			= 0x3F,
		FWATER			= 0x80,
		FZDIFF			= 0x40
	};
	enum{ // options
		REFLECTIONS	    = 0x00000001,
		WATERDEPTH		= 0x00000002,
		WATERMOD		= 0x00000004,
		WATERSHOW		= 0x00000008,
		WDEFAULTS       = REFLECTIONS|WATERDEPTH|WATERMOD,

		HAZE			= 0x00000010,
		FOG				= 0x00000020,
		FDEFAULTS       = HAZE|FOG,

		DOSHADOWS		= 0x00000040,
		SOFT_EDGES      = 0x00000080,
		SINGLE_VIEW		= 0x00000100,
		DPTEST			= 0x00000200,
		BGSHADOWS	    = 0x00000400,

		BUMPMAPS	    = 0x00000800,
		RASTTEXS	    = 0x00001000,
		MAX_VIEW	    = 0x00002000,
		FAR_VIEW	    = 0x00004000,
		DRAW_NVIS	    = 0x00008000,
		DOHDR	        = 0x00010000,
		USE_SHADERS	    = 0x00020000,
		DRAW_LINES	    = 0x00040000,
		DEBUG_SHADOWS	= 0x00100000,

		SDEFAULTS       = DPTEST|SOFT_EDGES|BGSHADOWS|MAX_VIEW,
		NEEDS_AUXBUF    = REFLECTIONS|FOG,
		NEEDS_PIXBUF    = DOSHADOWS|WATERDEPTH|REFLECTIONS|HAZE|FOG|WATERMOD,
		NEEDS_ZBUF1     = WATERDEPTH|SHADOWS|FOG|HAZE,
		NEEDS_ZBUF2     = WATERDEPTH,
		DEFAULTS		= WDEFAULTS|FDEFAULTS|SDEFAULTS|DOHDR
	};


	typedef struct rmdata {
		unsigned int  surface  : 2;
		unsigned int  twopass  : 1;
		unsigned int  fogpass  : 1;
		unsigned int  bumptexs : 1;
		unsigned int  hazepass : 1;
		unsigned int  idmode   : 1;
		unsigned int  active   : 1;
		unsigned int  rtype    : 4;
		unsigned int  lights   : 2;	// light option
		unsigned int  light	   : 3;	// current light
		unsigned int  accum    : 1;
		unsigned int  shadows  : 1;
		unsigned int  unused   : 13;
	} rmdata;

	rmdata flags;
	int options;
	void resize_idtbl(int);
	void alloc_idtbl(int);
	void init_image();
	void boxFilter();
	void pointFilter();
	void circleFilter();

	int idsize;
	int idcnt;
	int dcnt;
	MapNode	**idtbl;
	int ftype;

public:
	enum {RENDERPGM,EFFECTSPGM,POSTPROCPGM,SHADOW_ZVALS,SHADOWS,SHADOWS_FINISH};

	static GLdouble vpmat[16];
	static GLdouble sproj[16];
	static GLdouble smat[16];
	static GLdouble vmat[16];
	static GLdouble ismat[16];
	static GLdouble vproj[16];
	static GLdouble ivproj[16];
	static GLdouble ivpmat[16];
	RasterMgr();
	~RasterMgr();
    void set_defaults();
	int filter_type()				{ return ftype&FILTER_TYPE;}
	void set_filter_type(int i)		{ BIT_OFF(ftype,FILTER_TYPE);ftype|=i;}
	void set_filter_edges(int i)	{ BIT_SET(ftype,FILTER_EDGES,i);}
	int filter_edges()				{ return (ftype&FILTER_EDGES)?1:0;}
	void set_clean_edges(int i)		{ BIT_SET(ftype,FILTER_CLEAN,i);}
	int clean_edges()				{ return (ftype&FILTER_CLEAN)?1:0;}
	void set_filter_bg(int i)		{ BIT_SET(ftype,FILTER_BG,i);}
	int filter_bg()					{ return (ftype&FILTER_BG)?1:0;}
	void set_filter_stretch(int i)	{ BIT_SET(ftype,FILTER_STRETCH,i);}
	int filter_stretch()			{ return (ftype&FILTER_STRETCH)?1:0;}
	void set_filter_aniso(int i)	{ BIT_SET(ftype,FILTER_ANISO,i);}
	int filter_aniso()				{ return (ftype&FILTER_ANISO)?1:0;}
	void set_filter_show(int i)		{ BIT_SET(ftype,FILTER_SHOW,i);}
	int filter_show()				{ return (ftype&FILTER_SHOW)?1:0;}
	void set_filter_depth(int i)	{ BIT_SET(ftype,FILTER_DEPTH,i);}
	int filter_depth()				{ return (ftype&FILTER_DEPTH)?1:0;}
	void set_filter_normal(int i)	{ BIT_SET(ftype,FILTER_NORM,i);}
	int filter_normal()				{ return (ftype&FILTER_NORM)?1:0;}
	void show_filter_mask();
	void make_filter_mask();

	double filter_color_ampl;
	double filter_normal_ampl;
	double filter_normal_min;
	double filter_normal_max;
	double filter_depth_max;
	double filter_depth_min;

	int surface;
	int bmppass;
	Point lpts[8]; // light points

	Color  water_color2;
	Color  water_color1;
	double water_level;
	double water_mix;
	double water_clarity;
	double water_reflect;
	double water_specular;
	double water_shine;
	double water_dpr;
	double water_dpm;

	Color  haze_color;
	double haze_value;
	double haze_min;
	double haze_max;
	double haze_grad;
	double haze_zfar;

	Color  fog_color;
	double fog_value;
	double fog_min;
	double fog_max;
	double fog_vmin;
	double fog_vmax;

	Color  sky_color;

	double twilite_value;
	double twilite_max;
	double twilite_min;
	double twilite_dph;

	Point center;

	double sky_alpha;

	double bump_ampl;
	double bump_shift;
	double bump_bias;

	double shadow_darkness;
	double shadow_dscale;
	double shadow_randval;
	double shadow_zmin;
	double shadow_zmax;
	double shadow_zfactor;
	double shadow_dpmin;
	double shadow_dpmax;

	double shadow_vsteps;
	double shadow_vsteps_bg;
	double shadow_vbias;
	double shadow_vbias_bg;
	double shadow_vshift;
	double shadow_vshift_bg;
	double shadow_fov;
	double shadow_fov_bg;
	double shadow_dov;
	double shadow_dov_bg;

	int	   shadow_edge_min;
	int	   shadow_edge_width;
	int	   shadow_vcnt;
	int    shadow_test;
	bool   shadow_proj;

	double shadow_vstep;
	double shadow_vleft;
	double shadow_vright;
	double shadow_vsize;
	double shadow_vmin;
	double shadow_vmax;
	double shadow_zrange;
	double shadow_vzn;
	double shadow_vzf;
	Color  shadow_color;
	double shadow_value;
	double shadow_intensity;
	double shadow_blur;

	double s_zmin,s_zmax,s_rval,s_dpmin,s_dpmax;

	double blend_factor;
	double darken_factor;

	double hdr_min;
	double hdr_max;

	int do_water;
	int do_depth;
	int do_reflect;
	int do_shadows;
	int do_bumps;
	int do_fog;
	int do_vfog;
	int do_haze;
	int do_image;
	int do_edges;
	int do_hdr;
	int do_shaders;

	Color blend_color;

	void set_render_type(int i)     { flags.rtype=i;}
	int render_type()				{ return flags.rtype;}
	bool auximage()				    { return flags.rtype&(AUXIMAGE|SHADERS)?true:false;}
	void set_render_bumps()         { flags.rtype=BUMPS;}

	void set_options(int i)			{ options=i;}
	void set_filter(int i)			{ ftype=i;}
	int get_options()				{ return options;}

	int  pix_buf()					{ return options & NEEDS_PIXBUF?1:0;}
	int  aux_buf()					{ return options & NEEDS_AUXBUF?1:0;}
	int  aux_zbuf()					{ return options & NEEDS_ZBUF1?1:0;}
	int  pix_zbuf()					{ return options & NEEDS_ZBUF2?1:0;}

	int  hdr()				        { return options & DOHDR?1:0;}
	void set_hdr(int c)		        { BIT_SET(options,DOHDR,c);}

	int  lines()				    { return options & DRAW_LINES?1:0;}
	void set_lines(int c)		    { BIT_SET(options,DRAW_LINES,c);}

	int  use_shaders()				{ return options & USE_SHADERS?1:0;}
	void set_use_shaders(int c)		{ BIT_SET(options,USE_SHADERS,c);}

	int  debug_shadows()			{ return options & DEBUG_SHADOWS?1:0;}
	void set_debug_shadows(int c)	{ BIT_SET(options,DEBUG_SHADOWS,c);}

	int  textures()				    { return options & RASTTEXS?1:0;}
	void set_textures(int c)		{ BIT_SET(options,RASTTEXS,c);}

	int  reflections()				{ return options & REFLECTIONS?1:0;}
	void set_reflections(int c)		{ BIT_SET(options,REFLECTIONS,c);}

	int  water_depth()				{ return options & WATERDEPTH?1:0;}
	void set_water_depth(int c)		{ BIT_SET(options,WATERDEPTH,c);}

	int show_water()				{ return options & WATERSHOW?1:0;}
	void set_show_water(int c)		{ BIT_SET(options,WATERSHOW,c);}

	int  shading()				    { return options & SHADING?1:0;}
	void set_shading(int c)		    { BIT_SET(options,SHADING,c);}

	int  bumps()				    { return options & BUMPMAPS?1:0;}
	void set_bumps(int c)			{ BIT_SET(options,BUMPMAPS,c);}

	int  shadows()				    { return options & DOSHADOWS?1:0;}
	void set_shadows(int c)		    { BIT_SET(options,DOSHADOWS,c);}

	int  haze()				        { return options & HAZE?1:0;}
	void set_haze(int c)			{ BIT_SET(options,HAZE,c);}

	int  fog()				        { return options & FOG?1:0;}
	void set_fog(int c)				{ BIT_SET(options,FOG,c);}

	int  water_modulation()			{ return options & WATERMOD?1:0;}
	void set_water_modulation(int c){ BIT_SET(options,WATERMOD,c);}

	int  draw_nvis()				{ return options & DRAW_NVIS?1:0;}
	void set_draw_nvis(int c)		{ BIT_SET(options,DRAW_NVIS,c);}

	void set_all()					{ flags.surface=3;}
	void set_top()					{ flags.surface=1;}
	void set_bottom()				{ flags.surface=2;}
	int all()						{ return flags.surface==3?1:0;}
	int top()						{ return flags.surface==1?1:0;}
	int bottom()					{ return flags.surface==2?1:0;}

	void set_shadows_mode(int i)    { flags.shadows=i;set_draw_nvis(i);}
	int shadows_mode()				{ return flags.shadows;}

	void set_accumulate(int i)      { flags.accum=i;}
	int accumulate()				{ return flags.accum;}

	void set_waterpass(int i)         { flags.twopass=i;}
	int waterpass()				    { return flags.twopass;}

	void set_fogpass(int i)         { flags.fogpass=i;}
	int fogpass()				    { return flags.fogpass;}

	void set_hazepass(int i)        { flags.hazepass=i;}
	int hazepass()				    { return flags.hazepass;}

	void set_bumptexs(int i)        { flags.bumptexs=i;}
	int bumptexs()				    { return flags.bumptexs;}

	// id table functions

	void set_idmode(int c)			{ flags.idmode=c;}
	int  idmode()					{ return flags.idmode;}

	void set_data(MapNode *n)		{ if(dcnt>=idsize)
										  resize_idtbl((int)(idsize*1.25));
									  idtbl[dcnt++]=n;
									}
	MapNode *get_data(int id)		{ if(valid_id(id))
										return idtbl[id];
									  return 0;
									}
	int   set_id()					{ return idcnt=dcnt;}
	int   first_id()				{ return 1;}
	int   last_id()					{ return idcnt;}
	int   idvalues()				{ return dcnt;}
	int   invalid_id(int i)			{ return (i<0 || i>=dcnt)?1:0;}
	int   valid_id(int i)			{ return (i>=0 && i<dcnt)?1:0;}
	void  reset_idtbl();
	void  read_ids();
	void  setVisibleIDs();
	void  getLimits(double &zn, double &zf);
	MapNode *pixelID(int i, int j);

	void  modulate(Color &c);

	// shadow options

	int  lights()					{ return flags.lights;}
	void set_light(int c)	    	{ flags.light=c;}
	int  light_index()				{ return flags.light;}
	Light *light()					{ return Lights[flags.light];}
	void set_lights(int c)	    	{ flags.lights=c;}
	int  first_light()				{ return flags.lights==FIRST_LIGHT?1:0;}
	int  last_light()				{ return flags.lights==LAST_LIGHT?1:0;}
	int  all_lights()				{ return flags.lights==ALL_LIGHTS?1:0;}

	void set_single_view(int c)		{ BIT_SET(options,SINGLE_VIEW,c);}
	int  single_view()				{ return options & SINGLE_VIEW?1:0;}
	void set_dptest(int c)			{ BIT_SET(options,DPTEST,c);}
	int  dptest()					{ return options & DPTEST?1:0;}
	void set_soft_edges(int c)		{ BIT_SET(options,SOFT_EDGES,c);}
	int  soft_edges()				{ return options & SOFT_EDGES?1:0;}
	void set_bgshadows(int c)		{ BIT_SET(options,BGSHADOWS,c);}
	int  bgshadows()				{ return options & BGSHADOWS?1:0;}
	void set_maxview(int c)			{ BIT_SET(options,MAX_VIEW,c);}
	int  maxview()					{ return options & MAX_VIEW?1:0;}
	void set_farview(int c)			{ BIT_SET(options,FAR_VIEW,c);}
	int  farview()					{ return options & FAR_VIEW?1:0;}

	int  next_view();
	int  more_views();
	void set_light_view();
 	void init_view();
 	void clr_checked();
	void reset();
	void init_lights(int);
	void setView();

	void manageBuffers();
	void getZbuf(int i);
	void getPixels();
	void getAuxImage(int i);
	void drawPixels();

	void render_shading();
	void render_image();

	virtual void vertex(MapNode *);
	virtual void render();
	virtual void apply();
	virtual void applyWaterdepth();
	virtual void applyShadows();
	virtual void applyReflections();
	virtual void applyFog();
	virtual void applyFilter();

	virtual void init_render();

	virtual void init_shadows();
	virtual void renderBgShadows();
	virtual void applyBgShadows();
	virtual void renderFgShadows();
 	virtual void render_shadows();
    virtual void shadow_view();
    virtual void shadow_light();

};
//extern RasterMgr Raster;
#endif

