// Effects

#ifndef _EFFECTS_H
#define _EFFECTS_H

#include "Raster.h"
class SurfacePoint;

#define WATER_UNKNOWN 0xffffffff

class EffectsMgr : public RasterMgr
{
	GLuint shadowMapFBO;
	GLuint depthTextureId;
	GLuint jitterTextureId;
	GLuint shadow_map_height;
	GLuint shadow_map_width;
public:
    static char *shadow_vert;
    static char *shadow_geom;
    static char *shadow_defs;
	void enableShadowMap(bool b);
    static void setShadowProgram(const char*vert,const char*geom, char *defs){
    	shadow_vert=vert;
     	shadow_geom=geom;
     	shadow_defs=defs;
   }
	GLuint shadow_map_ratio;

	// sky view reflections (see Scene::render_skyview)
	int    skyview_enable;   // allow the extra sky view render
	int    skyview_ids;      // 1: extra id pass so terrain in the sky view is adapted
	                         //    like visible terrain (sharper, costs adapt time)
	                         // 0: draw it at the coarse hidden level (default)
	double skyview_margin;   // extra coverage above the flat-water mirror view (degrees)
	double skyview_max;      // max sky view vertical fov (degrees)
	bool   skyview_valid;    // sky view rendered for this frame
	int    skyview_minpix;   // min visible water pixels (last frame) to use the sky view
	GLuint water_query;      // occlusion query: water pixels drawn by the auximage pass
	bool   water_query_pending;
	GLuint water_pixels;     // result of the last completed water query (WATER_UNKNOWN: none yet)
	bool   skyview_water(bool wait=false); // enough water was visible in the last frame
	bool   skyview_wanted(); // reflections need tracing & view is compatible
	bool   update_skyview(); // compute View::sky_* for the current view, true if needed
	void create_jitter_lookup(int size, int samples_u, int samples_v);
	void initShadowTexture();
	EffectsMgr();
	~EffectsMgr();
	void render();
	void apply();

	void init_render();

	void setProgram(int);
	void renderBgShadows();
	void applyBgShadows();
	void renderFgShadows();
	
	void applyShadows();
	void init_shadows();
 	void render_shadows();
 	void setShadowProj(bool b);
 	bool shadowProj() { return shadow_proj;}
};
extern EffectsMgr Raster;
#endif

