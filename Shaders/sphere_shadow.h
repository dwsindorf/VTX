#ifndef _SPHERE_SHADOW_H
#define _SPHERE_SHADOW_H
// ########## begin sphere_shadow.h #########################
// Analytic shadows of spheres (planets, moons) - OrbitalClass analytic_shadows
// shadow_sph: per sphere xyz center (eye space), w radius (scene units)
// For each sphere the ray from the point toward the sun passes its center
// at distance d: full shadow inside the radius (umbra), with a penumbra the
// sun's apparent size wide. A small or distant sphere whose penumbra is
// wider than itself never fully hides the sun (antumbra, partial shadow).
#ifndef MAX_SHADOW_SPHERES
#define MAX_SHADOW_SPHERES 8
#endif
uniform float shadow_sph[MAX_SHADOW_SPHERES*4];
uniform int   shadow_nsph;
uniform float shadow_sun;   // sun angular radius (radians)

// light passing (1 lit, 0 full shadow) at P (eye space) toward the sun (L unit)
float sphereShadow(vec3 P, vec3 L){
	float s=1.0;
	for(int i=0;i<MAX_SHADOW_SPHERES;i++){
		if(i>=shadow_nsph)
			break;
		vec3 c=vec3(shadow_sph[4*i],shadow_sph[4*i+1],shadow_sph[4*i+2]);
		float R=shadow_sph[4*i+3];
		float t=dot(c-P,L);
		if(t<=0.0)              // the sphere is behind the point (away from the sun)
			continue;
		float d=length(P+t*L-c);
		float w=max(t*shadow_sun,1e-4*R);
		s*=smoothstep(R-w,R+w,d);
	}
	return s;
}
// ########## end sphere_shadow.h #########################
#endif
