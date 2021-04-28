
// ########## begin clouds.frag #########################
#include "common.h"
#include "utils.h"

#ifndef LMODE
#define LMODE 0
#endif

// Phong shading variables

uniform bool lighting;
uniform float twilite_min;
uniform float twilite_max;
uniform float twilite_dph; // dot product between point-center light-point at horizon
uniform float sky_ht;
uniform float eye_ht;

varying vec4 EyeDirection;
varying vec4 Normal;

uniform vec3 center;

uniform vec4 Haze;
uniform vec4 Halo;
uniform float haze_grad;
uniform float haze_zfar;
uniform float day_grad;

uniform float dpmax;
uniform float dpmin;
//
// Phong Lighting model
//

uniform vec4 Diffuse;   // for some reason can't seem to set & use gl_FrontMaterial.diffuse ??

vec3 setLighting(vec3 BaseColor, vec3 n) {
	vec3 normal  = normalize(n);
	vec3 eye = normalize(EyeDirection.xyz);
	vec3 radius = -normalize(EyeDirection.xyz+center);
	vec3 ambient = vec3(1, 1, 1);
	vec3 diffuse = vec3(0, 0, 0);
	vec3 specular = vec3(0, 0, 0);
	vec3 emission = vec3(0, 0, 0);
	float RdotN   = abs(dot(radius,normal));   // constant top & bottom lighting (starglow, groundglow)
	float top_shading  = 0.5*RdotN+0.5;   // models hemispherical lighting (light from all angles)
	//float top_shading  = RdotN;   // models single light source from above or below
	vec3 ec = normalize(center);          // viewdir-center
	float dp=dot(ec, radius); // angle between radius and viewdir direction
	float transmission=Constants1.z;

    //vec3 topcolor=gl_FrontMaterial.ambient.rgb*(top_shading*gl_FrontMaterial.ambient.a);
    vec3 topcolor=gl_FrontMaterial.ambient.rgb*top_shading;
	ambient        = topcolor;
	emission = gl_FrontMaterial.emission.rgb;
	float illumination=length(emission);

	//eye=normalize(-eye);
	vec3 haze=Haze.rgb;
	float halo=Halo.a;

	for(int i=0;i<NLIGHTS;i++){
		vec3 light      = normalize(gl_LightSource[i].position.xyz+EyeDirection.xyz);
		float LdotN     = dot(light,normal);// for day side diffuse lighting
		float LdotR     = dot(light,radius); // for horizon band calculation
		float horizon   = lerp(LdotR,twilite_dph+twilite_min,twilite_dph+twilite_max,0.0,1.0); // twilite band
		float amplitude = 1.0/gl_LightSource[i].constantAttenuation;
		float lpn       = LdotN*amplitude*horizon;
		diffuse        += Diffuse.rgb*gl_LightSource[i].diffuse.rgb*max(lpn,top_shading*gl_FrontMaterial.ambient.a);
		illumination   += horizon;
		//if(LdotN>0.0){ // top side
			vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
			float sdp   = max(0.0, dot(R, eye));
			float pf    = amplitude*horizon*pow(sdp, gl_FrontMaterial.shininess);
			specular   += gl_LightSource[i].specular.rgb * pf;
		//}
#ifdef BACK
		if(day_grad>0.0 && LdotR>twilite_min){ // mix in star color
			float dpLE=dot(light, -eye); // angle between vertex and light to viewdir direction
			dpLE=clamp(dpLE, 0.0, 1.0); // only positive angles
			float pf =pow(dpLE,100.0*day_grad);
			transmission*=pow(dpLE,10.0*day_grad);
		}
#endif
	}
	vec3 TotalDiffuse = diffuse * BaseColor * Diffuse.a;
	vec3 TotalSpecular = specular.rgb;
	vec3 TotalEmission = emission.rgb;
	vec3 TotalAmbient = ambient.rgb * BaseColor;
	vec3 TotalColor=TotalAmbient + TotalDiffuse + TotalEmission+TotalSpecular;
	illumination=clamp(illumination,0.0,1.0);
#ifdef BACK
#ifdef HAZE
	// horizon+fog height to zenith gradient
	//dp=max(dp,0.0);
	//float s=lerp(dp, dpmax-2.0*dpmin, dpmax-dpmin, 0.0, 1.0);
	float s=lerp(dp, dpmax-2.0*dpmin, dpmin, 0.0, 1.0);
	s=clamp(s,1e-5,1.0);  // for some reason pow(x,..) causes black pixels if x<=0
	float dv=pow(s,4.0*haze_grad);
	// eye to max distance gradient	
	//float d=min(DEPTH/haze_zfar,1.0);
	float dh=1.0;//pow(d,4.0*haze_grad);
	float h=Haze.a*dv*dh;
	transmission*=lerp(haze_grad,0.0,0.2,0.0,1.0);
	TotalColor.rgb=mix(TotalColor.rgb,haze.rgb*illumination,h);
#endif
	TotalColor.rgb=mix(TotalColor.rgb,Halo.rgb*illumination,transmission);
#endif

	return TotalColor;
}
// ########## end clouds.frag #########################
