#include "utils.h"
#include "scattering.h"
#ifndef NLIGHTS
#define NLIGHTS 1
#endif

uniform float sky_grad;
uniform float haze_grad;
uniform float twilite_min;
uniform float twilite_max;
uniform float day_grad;
uniform float dpmax;
uniform float dpmin;
uniform float dht;
uniform float density;
uniform vec3 center;

uniform vec4 Haze;
uniform vec4 Halo;
uniform vec4 Twilite;
uniform vec4 Night;
uniform vec4 Sky;

// Physically-derived scattering inputs (see scattering.h) - additive,
// gated by scatter_mix (Sky::scatter_mix, 0 by default so existing
// scenes are unaffected until this is dialed in on the Sky tab).
uniform float pressure;
uniform float ghg_fraction;
uniform float scatter_mix;
uniform float scatter_sharpen;

uniform vec3 pv;
varying vec4 EyeDirection;
varying vec4 Normal;
uniform bool fbo_read;
uniform bool fbo_write;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;

uniform float twilite_dph; // dot product between point-center light-point at horizon

// ----------- program entry -----------------
void main(void) {
	float illumination = 0.0;
	float alpha = 0.0;
	float a=0.0;
	vec3 eye = normalize(EyeDirection.xyz);
	vec3 normal = -normalize(EyeDirection.xyz+center);

	// moved up from below the light loop (was computed just before the
	// dpEV>dpmin alpha-gradient test) so it's available for the scattering
	// airmass estimate inside the loop too - 'center' is available from
	// the start, so this has no effect on the original alpha-gradient math
	vec3 ec = normalize(center);
	float dpEV=dot(-eye, ec);

	vec4 color=vec4(1,1,1,1);
#if NLIGHTS>0	
	color=Night; // start with night color
#else
	color=Sky;
	illumination=1;
#endif

	for(int i=0;i<NLIGHTS;i++){
		vec3 light      = normalize(gl_LightSource[i].position.xyz+EyeDirection.xyz);

		float LdotR    = dot(light,normal); // for horizon band calculation
		float ampl     = 1.0/gl_LightSource[i].constantAttenuation;

		float mx=lerp(LdotR, twilite_dph+twilite_min, twilite_dph+twilite_max, 0.0, Sky.a);
		color.rgb=mix(color.rgb,Sky.rgb,mx);

		// night-side darken all colors
		//mx=lerp(LdotR, twilite_dph+twilite_min,twilite_dph+twilite_max, 1-Night.a, 0.0);
		//color.rgb=mix(color.rgb,vec3(0.0),mx);

 		illumination+=max(lerp(LdotR,  twilite_dph+twilite_min, twilite_dph+twilite_max, 0.0, 1.0),illumination);
		alpha=max(lerp(LdotR, twilite_dph+2*twilite_min, twilite_dph+twilite_min, 0.0, 1.0),alpha);
//		vec3 H = normalize((light+eye)*0.5);
//		float sdp   = max(0.0, dot(normal,H));
#if LMODE == 0 // phong-blinn: uses half-vector (this is the openGL light model)
			vec3 H = normalize((light + eye)*0.5);
			float sdp   = max(0.0, dot(normal,H));
#elif LMODE == 1 // phong: uses light vector reflected about normal
			vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
			float sdp   = max(0.0, dot(R, eye));
#else // combined mode (ogl seems better at low angles phong better at higher angles)
			vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
			float sdp1   = max(0.0, dot(R, eye));
			vec3 H = normalize((light + eye)*0.5);
			float sdp2   = max(0.0, dot(normal,H));
			//float sdp=min(sdp1,sdp2);
			float sdp=(sdp1+sdp2)*0.5;
#endif

		float pf    = pow(sdp, 10.0*day_grad);
		color.rgb = mix(color.rgb,Halo.rgb,pf);

		// ---- physically-derived Rayleigh+Mie scattering (additive) ----
		// dpEV (computed above, before this loop) is the shader's own
		// existing "looking at the disk center" (dpEV~1, short path) vs
		// "looking at the limb/beyond" (dpEV<=0, long path) proxy - reused
		// here as a stand-in for atmospheric path length rather than
		// deriving new ray/sphere geometry.
		if(scatter_mix>0.0){
			float airmass=1.0+8.0*(1.0-clamp(dpEV,0.0,1.0));
			float cosTheta=dot(-eye,light);
			// day_grad (the "Halo" Sharpen slider) doubles as scatterColor()'s
			// tightness control now - see sky.back.frag for the reasoning.
			vec3 scat=scatterColor(cosTheta,airmass,pressure,ghg_fraction,
					day_grad,scatter_sharpen,gl_LightSource[i].diffuse.rgb);
			// scale by 'density' (Sky tab "Opacity") - see sky.back.frag,
			// same reasoning: scatterColor() alone doesn't know how thin
			// or thick this particular sky's atmosphere actually is.
			// also scale by 'ampl' - the same distance/luminosity brightness
			// factor (Light::getAttenuation(), from the star's temperature
			// and orbital distance) already used for the specular term just
			// above, so the scattering glow responds to how close/bright
			// the star actually is instead of always being full-strength.
			color.rgb+=scatter_mix*density*ampl*scat;
		}
	}
	color.a=1;

#if NLIGHTS>0
	if (dpEV>dpmin) // equator to horizon (planet) gradient
		a=lerp(dpEV, dpmin, 1.0, density, pow(density,10*sky_grad) );		
	else // horizon to space gradient (top side)
		a=lerp(dpEV, dpmax, dpmin, 0, density);
#else
	float dpn=dot(eye, normal); // angle between normal and eye direction
	if (dpEV>dpmin) // equator to horizon (planet) gradient
		a=pow(1-dpn, 2*sky_grad);		
	else // horizon to space gradient (top side)
		a=lerp(dpEV, dpmax, dpmin, 0, density);
		a*=Sky.a;
#endif
	color.a=a*lerp(density,0.0,1.0,alpha,1.0);
	color.rgb*=illumination;
	//color=vec4(a,0,0,1);
	color=clamp(color,0.0,1.0);
    	
	gl_FragData[0]=color;
	gl_FragData[1]=texture2DRect(FBOTex2, gl_FragCoord.xy); // FBO properties (background)

}

