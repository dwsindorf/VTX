// ########## begin cloud_haze.h #########################
// Sky haze for clouds. The effects pass (effects.frag) hazes terrain by
// distance but skips sky pixels (TYPE 0), and all cloud types are drawn
// into sky pixels, so the clouds apply the same haze themselves.
// Same formula as effects.frag hazeAmount(); uniforms set in
// CloudLayer::setProgram() (ch_ampl is 0 when haze is off).
#ifndef _CLOUD_HAZE_H
#define _CLOUD_HAZE_H
#include "sunlight.h"
uniform vec3  ch_color;   // haze color, lit for the time of day (Raster.haze_color)
uniform float ch_min;     // haze_min (scene units)
uniform float ch_max;     // haze_max (scene units)
uniform float ch_grad;    // haze_grad
uniform float ch_ampl;    // haze strength (Raster.haze_hf * haze alpha), 0 = off

// haze amount (0..1) at eye distance "depth" (scene units)
float cloudHaze(float depth){
	if(ch_ampl<=0.0)
		return 0.0;
	float g=(1.0-ch_grad)*0.1;
	float x1=max((depth-ch_min)/max(ch_max-ch_min,1e-12),0.0);
	return clamp(ch_ampl*(1.0-exp(-6.0*g*x1)),0.0,1.0);
}

// Sun disk behind volume clouds (the 12:28 version; 2D and 3D clouds use
// plain blending). The sky draws the disk much brighter than the sky, so
// behind a cloud only T^CLOUD_SUN_K of it gets through and the rest is
// replaced by the sky around the disk (clouds.volume.frag skyAroundSun:
// a ring of samples just outside the disk).
#define CLOUD_SUN_K   15.0
#define CLOUD_SUN_LUM 0.93    // disk: brighter than the sky's limit (sky.back.frag SKY_MAX 0.92)
uniform float sun_disk;   // sun angular radius (radians), Sky::setSunDiskVars
uniform float sun_pix;    // angular size of a pixel (radians)
float sunDiskW(vec3 c){
	return smoothstep(CLOUD_SUN_LUM,CLOUD_SUN_LUM+0.05,max(c.r,max(c.g,c.b)));
}
#endif
// ########## end cloud_haze.h #########################
