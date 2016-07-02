#include "utils.h"
#ifndef NLIGHTS
#define NLIGHTS 1
#endif

uniform float sky_grad;
uniform float haze_grad;
uniform float haze_zfar;
uniform float twilite_min;
uniform float twilite_max;
uniform float twilite_dph; // dot product between point-center light-point at horizon
uniform float day_grad;
uniform float dpmax;
uniform float dpmin;
uniform float dht;
uniform float density;
uniform vec3 center;
uniform vec3 viewpoint;
uniform vec4 Haze;
uniform vec4 Halo;
uniform vec4 Twilite;
uniform vec4 Night;
uniform vec4 Sky;

uniform vec3 pv;
varying vec4 EyeDirection;
varying vec3 Normal;
varying vec4 Constants;

uniform bool fbo_read;
uniform bool fbo_write;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;


#define HT Constants.x
#define DEPTH Constants.y
#define PHI Constants.z

// ----------- program entry -----------------
void main(void) {
	float illumination = 0.0;
	float alpha = 0.0;
	float ha=0.0,sa=0.0,sd=0.0,aa=0.0,a=0.0;
	vec3 haze=Haze.rgb;
	vec3 eye = -normalize(EyeDirection.xyz);
	vec3 radius = -normalize(EyeDirection.xyz+center);

	float specular = 0.0;

	ha=sqrt(Halo.a);
	vec4 color=Night; // start with night color

	vec3 ec = normalize(center);                // eye-center
	float dpv=dot(eye,ec); // angle between center and eye direction

	float dp=dot(ec, radius); // angle between radius and eye direction
	float h=lerp(dht,0.0,1.0,-1.0,-0.3);
	aa=lerp(dp, dpmax+h, dpmax, 0.0, 1.0); // horizon to space gradient
	float d=lerp(dp, dpmax-2.0*dpmin, dpmax, 0.0, 1.0);
	//color.a=d;
	for(int i=0;i<NLIGHTS;i++){
		vec3 light = normalize(gl_LightSource[i].position.xyz - EyeDirection.xyz);

		float LdotR    = dot(light,radius); // for horizon band calculation
		float ampl     = 1.0/gl_LightSource[i].constantAttenuation;

		// mix in twilite color
		float horizon  = lerp(LdotR,twilite_dph+twilite_min,twilite_dph+twilite_max,0.0,1.0); // twilite band
		float mx=lerp(LdotR, twilite_dph+twilite_min, twilite_dph, 0.0, Twilite.a);

		color.rgb=mix(color.rgb,Twilite.rgb,mx);
		// mix in sky color

		mx=lerp(LdotR, twilite_dph+twilite_min, twilite_dph+twilite_max, 0.0, Sky.a);
		color.rgb=mix(color.rgb,Sky.rgb,mx);

		sd=pow(d, sky_grad);  // darken sky with azimuth
	    color.rgb=mix(color.rgb,color.rgb*0.75,1.0-sd);

		// night-side darken all colors
		mx=lerp(LdotR, twilite_dph+twilite_min,twilite_dph+twilite_max, 1.0-Night.a, 0.0);
		color.rgb=mix(color.rgb,vec3(0.0),mx);

 		illumination+=horizon;//max(lerp(LdotR,  twilite_dph+twilite_min, twilite_dph+twilite_max, 0.0, 1.0),illumination);
		alpha=max(lerp(LdotR, twilite_dph+2.0*twilite_min, twilite_dph+twilite_min, 0.0, 1.0),alpha);

		//color.rgb=color.rgb*sd+vec3(0.5*sd);

		float pf=0.0;
		light = normalize(gl_LightSource[i].position.xyz-pv);
		if(LdotR>twilite_min){ // do specular for day side only
			float dpLE=dot(light, eye); // angle between vertex and light to eye direction
			dpLE=clamp(dpLE, 0.0, 1.0); // only positive angles
			pf =0.8*pow(dpLE,100.0*day_grad);
			specular += ampl*pf;
			color.rgb = mix(color.rgb,Halo.rgb*gl_LightSource[i].specular.rgb,specular*ha);
			haze=mix(haze.rgb,Halo.rgb,Halo.a*pow(dpLE,10.0*day_grad)*sqrt(haze_grad)); // darken with night
		}
	}
	float rmp = lerp(dht,0.0,0.2,0.1*illumination,0.0);
	sa=pow(aa+rmp, 10.0*sky_grad);  // sharpen sky gradient
	sa=(1.0-dht)*pow(aa+0.1*illumination, 2.0*sky_grad);  // sharpen sky gradient
	color.a=alpha*sa;
	color.a=clamp(color.a,0.0,1.0);
	//	color.a=max(ha,alpha*sa); // fade sky alpha with ht
	//color.rgb=vec3(0.0,0.0,1.0);//
#ifdef HAZE
	float s=lerp(dp, dpmax-2.0*dpmin, dpmax, 0.0, 1.0);
	//d=min(DEPTH/haze_zfar,1.0);
	float dv=pow(s,2.0*haze_grad);
	//float dh=1.0;//pow(d,haze_grad);
	h = Haze.a*dv; // haze transparancy
	//h=clamp(h,0.0,1.0);
	color.rgb=mix(color.rgb,haze.rgb*illumination,h);
#endif

	if (fbo_read) {
		vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // FBO image (background)
		vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // FBO properties (background)
		float shine_thru=fcolor2.r;
		shine_thru=clamp(shine_thru,0.0,1.0);

		// reduce opacity of sky in front of luminous objects
		// notes:
		// - opacity of background objects encoded in fcolor2.g
		// - shine of background objects encoded in fcolor2.r
		// - reduce shine-through as atmospheric density and haze increases

		a = color.a*(1.0-shine_thru*(1.0-h)*(1.0-density)); // moons: enhance shine-thru of day side
		a=clamp(a,0.0,1.0);
		color.rgb= mix(fcolor1.rgb,color.rgb,a);
	}
	gl_FragData[0]=color;
	gl_FragData[1]=texture2DRect(FBOTex2, gl_FragCoord.xy); // FBO properties (background)
	
	//gl_FragData[1]=vec4(0);
	//gl_FragData[1]=vec4(0,0,0,1);
	//gl_FragData[2]=vec4(0,0,0,1);
}

