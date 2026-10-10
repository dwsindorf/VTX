#include "utils.h"
#include "scattering.h"
#include "sunlight.h"
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
uniform vec4 HazeLit;   // Haze rgb darkened for time of day (= terrain haze color)
uniform vec4 Halo;
uniform vec4 Twilite;
uniform vec4 Scatter;
uniform vec4 Night;
uniform vec4 Sky;

// Physically-derived scattering inputs (see scattering.h) - additive,
// gated by scatter_mix (Sky::scatter_mix, 0 by default so existing
// scenes are unaffected until this is dialed in on the Sky tab).
uniform float pressure;
uniform float ghg_fraction;
//uniform float scatter_mix;
uniform float scatter_grad;
uniform float sun_tau;     // Sky::sunTau()
uniform float sun_redden;  // Sky::redden (0 = off)

uniform vec3 pv;
varying vec4 EyeDirection;
varying vec4 Normal;
varying vec4 Constants1;

uniform bool fbo_read;
uniform bool fbo_write;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;


#define HT Constants1.x
//#define DEPTH Constants1.y
#define PHI Constants1.z

// ----------- program entry -----------------
void main(void) {
	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // FBO image (background)
	vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // FBO properties (background)

	float illumination = fcolor2.b;
	float alpha = 0.0;
	float ha=0.0,sa=0.0,sd=0.0,aa=0.0,a=0.0;
	vec3 haze=HazeLit.rgb;  // match the terrain haze (effects.frag)
	vec3 eye = -normalize(EyeDirection.xyz);
	vec3 radius = -normalize(EyeDirection.xyz+center);

	float specular = 0.0;

	ha=sqrt(Halo.a);
	vec4 color=Night; // start with night color
	vec3 ec = normalize(center);                // eye-center
	float dpv=dot(eye,ec); // angle between center and eye direction

	float dp=dot(ec, radius); // angle between radius and eye direction
	float h=lerp(dht,0.0,1.0,-1.0,-0.3);
	aa=lerp(dp, dpmax+h, 1.5*dpmax, 0.0, 1.0); // horizon to space gradient
	float d=lerp(dp, dpmax-2.0*dpmin, dpmax, 0.0, 1.0);
	//color.a=d;
	vec4 band=vec4(0.0);  // sunrise/sunset horizon glow (sunsetBand)
	vec4 core=vec4(0.0);  // sun disk color/weight, re-applied on top of the band
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

 		illumination+=max(lerp(LdotR,  twilite_dph+twilite_min, twilite_dph+twilite_max, 0.0, 1.0),illumination);
		alpha=max(lerp(LdotR, twilite_dph+2*twilite_min, twilite_dph+twilite_min, 0.0, 1.0),alpha);
		//alpha*=alpha;
		//alpha=lerp(density,0.3,0.8,alpha,1);

		color.rgb=color.rgb*sd+vec3(0.5*sd);
		float pf=0.0;
		// sunlight color after its path through the atmosphere, using the
		// sun's elevation at the viewer (up = away from the planet center)
		vec3 up=-ec;                                       // viewer's up
		vec3 sdir=normalize(gl_LightSource[i].position.xyz);
		float sunSin=dot(sdir,up);
		vec3 sun_tint=sunTint(sunSin,sun_tau,sun_redden);
		{   // horizon band: view and sun directions projected onto the horizon plane
			float viewSin=dot(eye,up);
			vec3 eh=eye-up*viewSin;
			vec3 sh=sdir-up*sunSin;
			float le=length(eh), ls=length(sh);
			float cosAz=(le>1e-5 && ls>1e-5)?dot(eh,sh)/(le*ls):1.0;
			vec4 b=sunsetBand(viewSin,sunSin,cosAz,dot(eye,sdir),sun_tau,sun_redden);
			if(b.a>band.a)
				band=b;
		}
		light = normalize(gl_LightSource[i].position.xyz-pv);
		if(LdotR>twilite_min){ // do specular for day side only
			float dpLE=dot(light, eye); // angle between vertex and light to eye direction
			dpLE=clamp(dpLE, 0.0, 1.0); // only positive angles
			pf =0.8*pow(dpLE,2000.0*day_grad);
			specular += ampl*pf;
			// sun disk stays bright yellow-white (see sunCoreTint)
			core = vec4(Halo.rgb*sunCoreTint(sun_tint)*gl_LightSource[i].specular.rgb,clamp(specular*ha,0.0,1.0));
			color.rgb = mix(color.rgb,core.rgb,core.a);
			haze=mix(haze.rgb,Halo.rgb*sun_tint,Halo.a*pow(dpLE,10.0*day_grad)*sqrt(haze_grad)); // darken with night
            float scatter_mix=Scatter.a;
			// ---- physically-derived Rayleigh+Mie scattering (additive) ----
			if(scatter_mix>0.0){
				float dayFade=smoothstep(twilite_min,twilite_min+0.05,LdotR);
				float airmass=1.0+8.0*(1.0-clamp(aa,0.0,1.0));
				float cosTheta=dot(eye,light);
				vec3 scatter_color=mix(Scatter.rgb,gl_LightSource[i].diffuse.rgb,0.5)*sun_tint;
				vec3 scat=scatterColor(cosTheta,airmass,pressure,ghg_fraction,
						day_grad,scatter_grad,scatter_color);
				color.rgb+=scatter_mix*dayFade*density*ampl*scat;
			}
		}
	}
	float rmp = lerp(dht,0.0,0.2,0.1*illumination,0.0);
	//sa=pow(aa+rmp, 40.0*sky_grad);  // sharpen sky gradient
	sa=(1.0-dht)*pow(aa+0.1*illumination, 2.0*sky_grad);  // sharpen sky gradient
	color.a=alpha*sa;
	float ca=length(color.rgb);
	float da=lerp(density,0.25,1,ca,2.0);

	color.a*=da;

	color.a=clamp(color.a,0.0,1.0);
	//	color.a=max(ha,alpha*sa); // fade sky alpha with ht
	//color.rgb=vec3(0.0,0.0,1.0);//
	// sunrise/sunset horizon band and sun disk go in BEFORE haze, so haze
	// covers them like it covers everything else (haze=1, sharpen=0 must
	// still give a uniform fog). Applied after color.a is set so they don't
	// change the sky opacity.
	color.rgb=mix(color.rgb,band.rgb,band.a);
	// the additive scattering glow can push the sky around the sun past 1,
	// which clipped to white and left the (<=1) sun disk darker than the
	// sky next to it. Soft-compress each channel above a knee so the sky
	// approaches SKY_MAX (headroom for the disk) and very bright sky goes
	// pale like an overexposed photo. Values below the knee are unchanged.
	// (Scaling all channels by the max instead kept the hue, which turned
	// an over-bright sky a dull brown.)
	const float SKY_KNEE=0.7;
	const float SKY_MAX=0.92;
	vec3 over=max(color.rgb-SKY_KNEE,0.0);
	color.rgb=min(color.rgb,SKY_KNEE)+(SKY_MAX-SKY_KNEE)*(1.0-exp(-over/(SKY_MAX-SKY_KNEE)));
	// sun disk on top of the band; never darker than the sky behind it
	color.rgb=mix(color.rgb,max(color.rgb,core.rgb),core.a);
#ifdef HAZE
	float s=lerp(dp, dpmax-2.0*dpmin, dpmax, 0.0, 1.0);
	//d=min(DEPTH/haze_zfar,1.0);
	float dv=pow(s,2.0*haze_grad);
	//float dh=1.0;//pow(d,haze_grad);
	h = Haze.a*dv; // haze transparancy
	h=clamp(h,0.0,1.0);
	// no extra illumination scale: HazeLit already carries the day/night
	// darkening, and effects.frag applies the same color to the terrain.
	// (illumination could reach ~2 here, which is why a dense haze sky
	// went white while the terrain fog stayed gray.)
	color.rgb=mix(color.rgb,haze.rgb,h);
#endif

	//if (fbo_read) {
		float shine_thru=fcolor2.g;
		shine_thru=clamp(shine_thru,0.0,1.0);

		// reduce opacity of sky in front of luminous objects
		// notes:
		// - opacity of background objects encoded in fcolor2.g
		// - luminocity of background objects encoded in fcolor2.b
		// - reduce shine-through as atmospheric density and haze increases
		//
		float a0 = Sky.a*color.a*(1.0-shine_thru*(1.0-density)); // moons: enhance shine-thru of day side
		a = a0;
		a=lerp(density,0.5,0.9,a,Sky.a);
		a = mix(a, 1.0, h);
		a=clamp(a,0.0,1.0);
		color.rgb=mix(fcolor1.rgb,color.rgb,a);
		//color.rgb=mix(color.rgb,vec3(1.0,1.0,1.0),shine_thru*luminance);

	//}
	gl_FragData[0]=vec4(color.rgb,a);//color;
	gl_FragData[1]=texture2DRect(FBOTex2, gl_FragCoord.xy); // FBO properties (background)
}