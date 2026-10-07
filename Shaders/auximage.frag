#include "utils.h"

#define HT Constants1.x
#define DENSITY Constants1.z

varying vec4 Color;
varying vec4 Constants1;
varying vec4 EyeDirection;
varying vec4 Normal;


uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;
#define SHADOWTEX FBOTex3

uniform float twilite_min;
uniform float twilite_max;
uniform float twilite_dph; // dot product between point-center light-point at horizon
uniform vec3 center;
uniform vec4 WaterSky;

uniform vec4 WaterColor2;
uniform vec4 WaterColor1;
uniform float water_reflection;
uniform float water_saturation;
uniform float water_clarity;
uniform float water_mix;
uniform float water_dpr;

uniform vec4 IceColor2;
uniform vec4 IceColor1;
uniform float ice_reflection;
uniform float ice_saturation;
uniform float ice_clarity;
uniform float ice_mix;
uniform float ice_dpr;

//uniform float water_dpr;
uniform float dpm;
uniform float ws1;
uniform float ws2;
uniform vec4 Shadow;
uniform float fog_vmin;
uniform float fog_vmax;

uniform float feet;
uniform float water_shine;  // water tab Shine (0-100)
uniform float ice_shine;
uniform float water_albedo; // water tab Albedo (specular amount 0-1)
uniform float ice_albedo;
uniform float sun_tau;     // Sky::sunTau()  (0 = no reddening)
uniform float sun_redden;  // Sky::redden

#include "sunlight.h"

//
//Lighting model
//

void main(void) {
	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // FBO image (background)
	vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // data texture
	
    float type=Constants1.g;

	float ambient = 0.0;
	float specular = 0.0;
	float diffuse = 0.0;

	vec3 Diffuse = vec3(0.0, 0.0, 0.0);
	vec3 Specular = vec3(1.0, 1.0, 1.0);
	vec3 Ambient = vec3(0.0, 0.0, 0.0);

	vec3 eye = normalize(EyeDirection.xyz);
	vec3 radius = -normalize(EyeDirection.xyz+center);
	vec3 normal  = normalize(Normal.xyz);

	float RdotN   = dot(radius,normal); // for top lighting on nightside
	float top_shading  = 0.5*RdotN+0.5;  // models hemispherical lighting (light from all angles)

    ambient=gl_FrontMaterial.ambient.a;
    vec3 topcolor=gl_FrontMaterial.ambient.rgb*top_shading*ambient;

    Ambient = topcolor;
    float horizon=0.0;
    float illumination = gl_FrontMaterial.emission.a;
    
#ifdef SHADOWS
    float shadow=texture2DRect(SHADOWTEX, gl_FragCoord.xy).r; // data texture
    float shadow_specular=lerp(Shadow.a,0.0,1.0,1.0,shadow);
    float shadow_diffuse=shadow_specular;
#else
	float shadow_diffuse=1.0;
	float shadow_specular=1.0;
#endif
	for(int i=0;i<NLIGHTS;i++){
		vec3 light      = normalize(gl_LightSource[i].position.xyz+EyeDirection.xyz);
		float LdotR     = dot(light,radius ); // for horizon band calculation
		//horizon   += lerp(LdotR,-1,1,1.0,0.0); // twilite band
		horizon   += lerp(LdotR,twilite_dph+twilite_min,twilite_dph+twilite_max,0.0,1.0); // twilite band
		illumination   += horizon;

		ambient   += horizon;

		float LdotN     = dot(light,normal);
		float amplitude = 1.0/gl_LightSource[i].constantAttenuation;
		float lpn       = LdotN*amplitude*horizon;
		//diffuse+=LdotN;
		diffuse+=max(lpn,top_shading*gl_FrontMaterial.ambient.a);
		Diffuse        += gl_LightSource[i].diffuse.rgb*diffuse;
		//Diffuse=gl_LightSource[i].diffuse.rgb*diffuse;
		// Water/ice always use the half-vector (OGL / Blinn-Phong) model,
		// whatever the scene's lighting model: at grazing sun angles it
		// stretches the sun's glint into the long vertical "glitter path"
		// seen on real water, where Phong gives a round spot. Terrain
		// still follows the Lighting Model setting (lighting.frag).
		vec3 H = normalize((light + eye)*0.5);
		float sdp   = dot(normal,H);
		// Shine slider (0-100) -> exponent 2..1000: high values give the
		// narrow glitter column of a low sun. (gl_FrontMaterial.shininess
		// was whatever the last land layer set - the water Shine slider
		// never reached this shader.)
		float wshine = mix(water_shine,ice_shine,lerp(type,1.0,2.0,0.0,1.0));
		float expo   = 2.0+wshine*wshine*0.1;
		specular    = pow(max(sdp,0.0), expo);
		// no glint once the sun is below the horizon (the twilight band
		// still lights the water, but the sun itself can't be reflected)
		float sunUp = smoothstep(-0.01,0.02,LdotR);
		float walb  = mix(water_albedo,ice_albedo,lerp(type,1.0,2.0,0.0,1.0));
		// glint color follows the sun: bright yellow-white near the horizon
		// (sunCoreTint), like the disk itself
		vec3 glint  = sunCoreTint(sunTint(LdotR,sun_tau,sun_redden));
		Specular    = gl_LightSource[i].specular.rgb*glint*specular*sunUp*walb*shadow_specular;
	}
	float clarity=1;
	float wf=lerp(type,1,2,0,1);

	vec3 Color1 = mix(WaterColor1,IceColor1,wf);
	vec3 Color2 = mix(WaterColor2,IceColor2,wf);
#define TEST
#ifdef TEST
	if(type<0.5){
		float df=1.0-type;
		Color2.xyz=Color2.xyz*df;
	}
#endif	
	float reflection=mix(water_reflection,ice_reflection,wf);
	float saturation=mix(water_saturation,ice_saturation,wf);
	clarity=mix(water_clarity,ice_clarity,wf);
	float cmix=mix(water_mix,ice_mix,wf);
	float dpr=0.5*mix(water_dpr,ice_dpr,wf);

	float reflect1=dot(normal,eye); // reflection angle
	float depth=gl_FragCoord.z; // water
	float z=fcolor2.g;  // land
	float z2=1.0/(ws2*z+ws1);
	float z1=1.0/(ws2*depth+ws1);
	float dz=(z2-z1);
	dz=dz<0.0?0.0:dz;
	float f=fcolor2.r==0?1:lerp(dz,0.0,clarity,0.0,1.0); // f=1 if back surface != land
	
	// ---- transmission vs reflection ------------------------------------
	// Transmit (Color2 + its clarity/"transparency" distance) only FILTERS
	// light coming up from the bottom; it is not a surface color. Reflect
	// (Color1 + its amount) is what the surface shows instead.
	//   f = 0: clear/shallow - the bottom (fcolor1) shows through, tinted
	//   f = 1: deeper than the clarity distance (or no bottom behind it) -
	//          nothing is transmitted, the surface shows only reflection
	// Previously the deep/opaque case showed the Transmit color itself, so
	// low transparency painted the water in the transmit color (e.g. red).
	vec3 Lit   = Diffuse+Ambient;                     // light on the surface
	vec3 Under = fcolor1.rgb*mix(vec3(1.0),Color2.rgb,f)*(1.0-f);
	vec3 Refl  = Color1.rgb*Lit;                      // reflection color, lit
	// Fresnel-like: more reflection at grazing view angles (reflect1->0)
	float rmod = lerp(reflect1,0.0,max(dpr,1e-3),reflection,0.0);
#if defined(SKY) && !defined(RAYTRACE)
	// no traced reflection: approximate the reflected sky with its base color
	// (with RAYTRACE, effects.frag mixes in the actual reflected pixel)
	Refl = mix(Refl, WaterSky.rgb*Lit, rmod);
#endif
	// whatever isn't transmitted is reflected
	float R = max(rmod, f);
	vec3 color = mix(Under, Refl, R) + Specular;
#ifdef SKY
	color.rgb=mix(color.rgb,vec3(0),1-shadow_diffuse);
#endif
    //color.rgb=vec3(horizon,0,0);
	color=clamp(color,0.0,1.0);
	gl_FragData[0] = vec4(color,1.0);
	float ht=HT+fcolor2.a;
	
	float vfog=DENSITY*lerp(HT,fog_vmin,fog_vmax,1,0);
	gl_FragData[0].a=1;
	gl_FragData[1] = vec4(type,depth,reflect1,vfog);
}

