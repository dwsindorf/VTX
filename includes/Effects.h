// Effects

#ifndef _EFFECTS_H
#define _EFFECTS_H

#include "Raster.h"

class EffectsMgr : public RasterMgr
{
	GLuint shadowMapFBO;
	GLuint depthTextureId;
	GLuint jitterTextureId;
	GLuint shadow_map_height;
	GLuint shadow_map_width;
public:

	void enableShadowMap(bool b);

	GLuint shadow_map_ratio;
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

