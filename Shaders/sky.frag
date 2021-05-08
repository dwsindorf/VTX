#include "utils.h"
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
uniform vec3 viewpoint;
uniform vec4 Haze;
uniform vec4 Halo;
uniform vec4 Twilite;
uniform vec4 Night;
uniform vec4 Sky;

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
	float ha=0.0,sa=0.0,sd=0.0,aa=0.0,a=0.0;
	vec3 haze=Haze.rgb;
	vec3 viewdir = -normalize(EyeDirection.xyz);
	vec3 radius = -normalize(EyeDirection.xyz+center);

	float specular = 0.0;

	ha=sqrt(Halo.a);
	vec4 color=Night; // start with night color

	vec3 ec = normalize(center);                 // viewdir-center
	float dp=dot(ec, radius); // angle between radius and viewdir direction
	//float h=min(1.0,dht)-1.0;
	float h=lerp(dht,0.0,1.0,-1.0,-0.3);
	aa=lerp(dp, dpmax+h, dpmax, 0.0, 1.0); // horizon to space gradient
	float d=lerp(dp, dpmax-2.0*dpmin, dpmax, 0.0, 1.0);
	sd=pow(d, sky_grad);  // darken sky with azimuth
	float VdotR;
#ifndef BACKSIDE
	vec3 C = -center;
	vec3 B = EyeDirection.xyz;
	vec3 A = C-B;
	VdotR=dot(normalize(A),normalize(C));
	//mx*=lerp(VdotR,-1,0,0,1);
	//mx*=1-VdotR;//lerp(VdotR,0.0,-0.2,0,mx);
#endif

	for(int i=0;i<NLIGHTS;i++){
		vec3 light = normalize(gl_LightSource[i].position.xyz - EyeDirection.xyz);

		float LdotR    = dot(light,radius); // for horizon band calculation
		float ampl     = 1.0/gl_LightSource[i].constantAttenuation;

// 		illumination+=lerp(LdotR, twilite_dph+twilite_min, twilite_dph+twilite_max, 0.0, 1.0);
//		alpha=lerp(LdotR, twilite_dph+twilite_min, twilite_dph, 0.0, 1.0);

		// mix in twilite color
		float mx=lerp(LdotR, twilite_dph+twilite_min, twilite_dph, 0.0, Twilite.a);

		mx*=1-VdotR;

		color.rgb=mix(color.rgb,Twilite.rgb,mx);
		// mix in sky color

		mx=lerp(LdotR, twilite_dph+twilite_min, twilite_dph+twilite_max, 0.0, Sky.a);
		color.rgb=mix(color.rgb,Sky.rgb,mx);

		// night-side darken all colors
		mx=lerp(LdotR, twilite_dph+twilite_min,twilite_dph+twilite_max, 1-Night.a, 0.0);
		color.rgb=mix(color.rgb,vec3(0.0),mx);


 		illumination+=max(lerp(LdotR,  twilite_dph+twilite_min, twilite_dph+twilite_max, 0.0, 1.0),illumination);
		alpha=max(lerp(LdotR, twilite_dph+2*twilite_min, twilite_dph+twilite_min, 0.0, 1.0),alpha);
//
//		float mx=lerp(LdotR, twilite_min-twilite_max, 0, 0.0, Twilite.a);
//		color.rgb=mix(color.rgb,Twilite.rgb,mx);
//		mx=lerp(LdotR, -twilite_max, 0.0, 0.0, Sky.a);
//		color.rgb=mix(color.rgb,Sky.rgb,mx);
#ifdef BACKSIDE // darken inside sky with azimuth
		color.rgb=color.rgb*sd+vec3(0.5*sd);
#endif
		float pf=0.0;
		light = normalize(gl_LightSource[i].position.xyz-pv);
		if(LdotR>twilite_min){ // do specular for day side only
			float dpLE=dot(light, viewdir); // angle between vertex and light to viewdir direction
			dpLE=clamp(dpLE, 0.0, 1.0); // only positive angles
			pf =0.8*pow(dpLE,100.0*day_grad);
			specular += ampl*pf;
			color.rgb = mix(color.rgb,Halo.rgb*gl_LightSource[i].specular.rgb,specular*ha);
			haze=mix(haze.rgb,Halo.rgb,Halo.a*pow(dpLE,10.0*day_grad)*sqrt(haze_grad)); // darken with night
		}
	}
	color.a=1;

	//sa=(1-dht)*pow(aa+0.1*illumination, 2.0*sky_grad);  // sharpen sky gradient
#ifdef BACKSIDE // inside sky
	float rmp = lerp(dht,0.0,0.2,0.1*illumination,0.0);
	sa=pow(aa+rmp, 10.0*sky_grad);  // sharpen sky gradient
	float hg=lerp(haze_grad,0,1.0,0.1,10);
	ha=Haze.a*pow(aa,hg*haze_grad); // sharpen haze gradient

	color.a=max(ha,alpha*sa); // fade sky alpha with ht
	color.a=clamp(color.a,0.0,1.0);

	haze=mix(haze.rgb,Night.a*vec3(0.0),(1.0-alpha)); // darken with night

	color.rgb=mix(color.rgb,haze.rgb,ha); // combine sky and haze colors
	if (fbo_read) {
		vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // FBO image (background)
		vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // FBO properties (background)
		// reduce opacity of sky in front of luminous objects
		// notes:
		// - opacity of background objects encoded in fcolor2.g
		// - shine of background objects encoded in fcolor2.r
		// - reduce shine-through as atmospheric density and haze increases

		a = color.a*(1.0-fcolor2.r*(1.0-ha)*(1.0-density)); // moons: enhance shine-thru of day side
		color.rgb= mix(fcolor1.rgb,color.rgb,a);
		color.a=1;

	}
#else  // outside

	float dpEV=dot(viewdir, ec); // angle between center and eye direction
	if (dpEV<=dpmin){ // horizon to space
		a=lerp(dpEV, dpmax, dpmin, 0.0, 1.0);
		//a=pow(a, 1.0-density+2*sky_grad); // sharpen sky gradient
		a=pow(a, 2*sky_grad); // sharpen sky gradient
	}
	else{            //  equator to horizon
		a=lerp(dpEV, dpmin, 1.0, density, pow(density,1.0+10.0*sky_grad) );
	}
	color.a=a*lerp(density,0.0,1.0,alpha,1.0);
	ha=Haze.a*(1.0-density)*pow(a,0.2+2.0*haze_grad); // sharpen haze gradient
	color.rgb=mix(color.rgb,haze.rgb,ha); // combine sky and haze colors

#endif
	//color=vec4(vec3(VdotR),1.0);

	gl_FragData[0]=color;
}

