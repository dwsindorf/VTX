#ifndef _PLANET_SHADOWS_H
#define _PLANET_SHADOWS_H
// ########## begin planet_shadows.h #########################
// Analytic shadows on anything on or near a planet or moon: terrain
// (lighting.frag), rocks, plants, sprites. See OrbitalClass analytic_shadows;
// analyticShadowDefs() and analyticShadowVars() set the defines and uniforms.
#ifdef RING_SHADOW
// ---- the planet's ring shadow (analytic, OrbitalClass analytic_shadows) ----
// the ray from the point toward the sun crosses the ring plane (the
// planet's equator, through ring_center) at radius r; the light passing is
// 1-opacity there, from the ring's opacity profile (inner..outer edge).
// The profile is averaged over the sun's size at that distance (penumbra)
uniform vec3  ring_center;          // planet center (eye space)
uniform vec3  ring_pole;            // ring plane normal (eye space)
uniform float ring_r0;              // inner radius
uniform float ring_r1;              // outer radius
uniform float ring_sun;             // sun angular radius (radians)
uniform float ring_op[RING_NOP];    // opacity, inner..outer edge
float ringOpacity(float f){
	if(f<=0.0 || f>=1.0)
		return 0.0;
	float x=f*float(RING_NOP-1);
	int i=int(floor(x));
	int j=min(i+1,RING_NOP-1);
	return mix(ring_op[i],ring_op[j],x-float(i));
}
float ringShadow(vec3 P, vec3 L){
	float dn=dot(L,ring_pole);
	if(abs(dn)<1e-6)
		return 1.0;
	float t=dot(ring_center-P,ring_pole)/dn;
	if(t<=0.0)                       // the ring is behind (away from the sun)
		return 1.0;
	float r=length(P+t*L-ring_center);
	float w=ring_r1-ring_r0;
	float f=(r-ring_r0)/w;
	float df=t*ring_sun/w;           // sun radius at the ring (fraction of its width)
	float o=0.25*ringOpacity(f-df)+0.5*ringOpacity(f)+0.25*ringOpacity(f+df);
	return 1.0-o;
}
#endif

#ifdef SPHERE_SHADOWS
#include "sphere_shadow.h"
#endif

#if defined(RING_SHADOW) || defined(SPHERE_SHADOWS)
#define PLANET_SHADOWS
// light passing (1 lit, 0 full shadow) at P (eye space)
float planetShadow(vec3 P){
	if(gl_LightSource[0].position.w==0.0)
		return 1.0;
	vec3 L=normalize(gl_LightSource[0].position.xyz-P);
	float s=1.0;
#ifdef RING_SHADOW
	s*=ringShadow(P,L);
#endif
#ifdef SPHERE_SHADOWS
	s*=sphereShadow(P,L);
#endif
	return s;
}
#endif
// ########## end planet_shadows.h #########################
#endif
