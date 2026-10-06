
uniform float water_shine;
uniform float ice_shine;
uniform float water_specular;
uniform float ice_specular;
uniform float albedo;
uniform float glow;
uniform float sun_tau;     // Sky::sunTau() - 0 if no sky (no reddening)
uniform float sun_redden;  // Sky::redden   - 0 = off

#include "sunlight.h"


#ifndef LMODE
#define LMODE 0
#endif


vec4 setLighting(vec3 BaseColor, vec3 n, vec3 b) {
	vec3 bmp  = -gl_NormalMatrix*b;
	vec3 eye = normalize(EyeDirection.xyz);
	vec3 radius = -normalize(EyeDirection.xyz+center);

	vec3 normal  = normalize(n+bmp);
	vec3 ambient = vec3(0.0, 0.0, 0.0);
	vec3 emission = vec3(0.0, 0.0, 0.0);
	vec3 diffuse = vec3(0.0, 0.0, 0.0);
	vec3 specular = vec3(0.0, 0.0, 0.0);
#ifdef HDR
	float bumpamp=dot(n,bmp);
    float exposure = 1.0-hdr_min*smoothstep(0.0,1.0,bumpamp/hdr_max);
	exposure = max(exposure,0.0);
	float RdotN   = dot(radius,normal); // for top lighting on nightside
	float top_shading  = 0.5*exposure*RdotN+0.5+0.5*exposure;  
#else
	float RdotN   = dot(radius,normal); // for top lighting on nightside
	float top_shading  = 0.5*RdotN+0.5;  // models hemispherical lighting (light from all angles)
#endif
	//float top_shading  = RdotN*RdotN*RdotN; // models a single light source from above (a bit more dramatic)
    vec3 topcolor=gl_FrontMaterial.ambient.rgb*(top_shading*gl_FrontMaterial.ambient.a);
	ambient = topcolor;
	emission = gl_FrontMaterial.emission.rgb;
	// Base term only (self-emission, 0 for any ordinary reflective body
	// like a moon/planet) - the phase-dependent reflected-light term is
	// accumulated per-light below.
	float illumination = gl_FrontMaterial.emission.a;
	// Captured BEFORE the water/ice block below reassigns 'albedo' to
	// water_specular/ice_specular for the specular highlight - shine-
	// through brightness should reflect the surface's real albedo, not
	// that specular-reflectivity substitute (mixing the two broke water:
	// it's tuned for how tight/bright a highlight looks, not for how much
	// diffuse sunlight the surface reflects overall).
	float base_albedo = albedo;
#ifdef SHADOWS
    float shadow=texture2DRect(SHADOWTEX, gl_FragCoord.xy).r; // data texture
    float shadow_diffuse=lerp(Shadow.a,0.0,1.0,1,shadow);
    float shadow_specular=shadow;//lerp(shadow,0.2,1.0,0.0,shadow);
#else
	float shadow_diffuse=1.0;
	float shadow_specular=1.0;
#endif

    float shine=gl_FrontMaterial.shininess;
    if(Constants1.g>=0 && Constants1.g<2.1){
	    if(Constants1.g>1.1){
		   shine=ice_shine;
		   albedo=ice_specular;
		}
	    else {
	       shine=water_shine;
	       albedo=water_specular;
	    }
	 }
    
	for(int i=0;i<NLIGHTS;i++){
	    if(gl_LightSource[i].position.w==0.0)
	    	continue;
		vec3 light      = normalize(gl_LightSource[i].position.xyz+EyeDirection.xyz);
		float LdotN     = dot(light,normal);  // for day side diffuse lighting
		float LdotR     = dot(light,radius ); // for horizon band calculation
		float horizon   = lerp(LdotR,twilite_dph+twilite_min,twilite_dph+twilite_max,0.0,1.0); // twilite band
		float intensity = 1.0/gl_LightSource[i].constantAttenuation/NLIGHTS;
		float lpn       = LdotN*intensity*horizon;
		vec3 tint       = sunTint(LdotR,sun_tau,sun_redden); // sunrise/sunset reddening
		// tint only the direct (sunlit) term, not the hemispherical floor,
		// so the night side doesn't pick up the horizon red
		diffuse        += Diffuse.rgb*gl_LightSource[i].diffuse.rgb*max(tint*lpn,vec3(top_shading*gl_FrontMaterial.ambient.a));
		ambient+=topcolor*(3.0*horizon);
		// Reflected-light illumination for shine-through (moons/planets
		// viewed through the sky): unlike 'diffuse' above this has NO
		// top_shading/ambient floor, so it genuinely falls to ~0 on the
		// unlit side of the terminator instead of staying visible - that's
		// what makes a crescent read as a crescent instead of a uniformly
		// lit disc. Uses base_albedo (the real surface albedo, captured
		// above before water/ice locally override 'albedo' for specular);
		// horizon reuses the existing twilight-band falloff so the
		// shine-through edge softens the same way the visible surface
		// shading does.
		illumination += base_albedo*horizon*max(LdotN,0.0);
        if(LdotN>-0.5){
			float damp = lerp(LdotN,-0.5,0.0,0.0,1.0); // prevents false specular highlites on backfaces
#if LMODE == 0 // phong-blinn: uses half-vector (this is the openGL light model)
			vec3 H = normalize((light + eye)*0.5);
			float sdp   = max(0.0, dot(normal,H));
#elif LMODE == 1 // phong: uses light vector reflected about normal
			vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
			float sdp   = max(0.0, dot(R, eye));
#else // combined mode (ogl seems better at low angles phong better at higher angles)
			vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
			float phong   = max(0.0, dot(R, eye));
			vec3 H = normalize((light + eye)*0.5);
			float ogl   = max(0.0, dot(normal,H));
			float sdp=(0.5*phong+0.5*ogl);
#endif
			float pf        = damp*intensity*horizon*pow(sdp, shine);
			
			// glints (e.g. sun on water) stay bright yellow like the disk
			specular       += gl_LightSource[i].specular.rgb * sunCoreTint(tint) * pf*shadow_specular*albedo;
        }
	}
	diffuse.rgb=mix(diffuse.rgb,Shadow.rgb,1.0-shadow_diffuse);
	vec3 TotalEmission = emission.rgb * BaseColor;
	vec3 TotalAmbient = ambient.rgb * BaseColor;
	vec3 TotalDiffuse = diffuse.rgb * BaseColor*Diffuse.a*shadow_diffuse*glow;
	vec3 TotalSpecular = specular.rgb;

	return vec4(TotalAmbient +TotalEmission + TotalDiffuse + TotalSpecular,illumination);

}


