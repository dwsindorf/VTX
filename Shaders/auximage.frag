#include "utils.h"

#define HT Constants.x
#define DENSITY Constants.z

varying vec4 Color;
varying vec4 Constants;
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

uniform vec4 WaterDepth;
uniform vec4 WaterSky;
uniform vec4 WaterTop;
uniform float clarity;
uniform float saturation;
uniform float reflection;
uniform float water_dpr;
uniform float dpm;
uniform float cmix;
uniform float ws1;
uniform float ws2;
uniform vec4 Shadow;
uniform float fog_vmin;
uniform float fog_vmax;

uniform float feet;

//
//Lighting model
//

void main(void) {
	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // FBO image (background)
	vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // data texture

	float ambient = 0.0;
	float specular = 0.0;
	float diffuse = 0.0;

	vec3 Diffuse = vec3(0.0, 0.0, 0.0);
	vec3 Specular = vec3(0.0, 0.0, 0.0);
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
		horizon   += lerp(LdotR,twilite_dph+twilite_min,twilite_dph+twilite_max,0.0,1.0); // twilite band

		ambient   += horizon;

#ifdef WATER
		float LdotN     = dot(light,normal);
		float amplitude = 1.0/gl_LightSource[i].constantAttenuation;
		float lpn       = LdotN*amplitude*horizon;
		//diffuse+=LdotN;
		diffuse+=max(lpn,top_shading*gl_FrontMaterial.ambient.a);
		Diffuse        += gl_LightSource[i].diffuse.rgb*diffuse;
		//Diffuse=gl_LightSource[i].diffuse.rgb*diffuse;
		//if(LdotN>-0.5){
#if LMODE == 0 // phong-blinn: uses half-vector (this is the openGL light model)
		vec3 H = normalize((light + eye)*0.5);
		float sdp   = dot(normal,H);//max(0.0, dot(normal,H));
#elif LMODE == 1 // phong: uses light vector reflected about normal
		vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
		float sdp   = dot(R, eye);//max(0.0, dot(R, eye));
#else // combined mode (ogl seems better at low angles phong better at higher angles)
		vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
		float phong   = dot(R, eye);//max(0.0, dot(R, eye));
		vec3 H = normalize((light + eye)*0.5);
		float ogl   = dot(normal,H);//max(0.0, dot(normal,H));
		float sdp=max(phong,ogl);
#endif
		specular    = pow(max(sdp,0.0), gl_FrontMaterial.shininess);
		Specular    = gl_LightSource[i].specular.rgb*specular*shadow_specular;
#endif
	}
#ifdef WATER
	float reflect1=dot(normal,eye); // reflection angle
	float depth=gl_FragCoord.z; // water
	float z=fcolor2.g;  // land
	float z2=1.0/(ws2*z+ws1);
	float z1=1.0/(ws2*depth+ws1);
	float dz=(z2-z1);
	
	//float dz=ws1*(depth-z);
	dz=dz<0.0?0.0:dz;
	float f=lerp(dz,0.0,clarity,0.0,1.0);
	float type=Constants.g;
#ifdef SKY
	vec3 DepthColor=WaterDepth.rgb*horizon;
	vec3 color=mix(fcolor1.rgb,DepthColor,f); // add depth color
	float dpr=0.5*water_dpr;
	float fmix=lerp(reflect1,dpr,dpm,cmix,0);
	float rmod=lerp(reflect1,0,dpr,reflection,0);
	float cmod=(1-rmod)*saturation;
	fmix*=fmix;
	float f1=1-fmix; // surface
	float f2=fmix*cmod;
	float f3=rmod;

	vec3 TopColor=Diffuse*WaterTop.rgb+Ambient*WaterTop.rgb;
	vec3 SkyColor=Diffuse*WaterSky.rgb+Ambient*WaterSky.rgb;

	color=color.rgb*f1+TopColor*f2+SkyColor.rgb*f3+Specular;
	color.rgb=mix(color.rgb,vec3(0),1-shadow_diffuse);
#else
    vec3 color=mix(fcolor1.rgb,WaterDepth.rgb,f); // add depth color
#endif	
	color=clamp(color,0.0,1.0);
	gl_FragData[0] = vec4(color,1.0);
	float ht=HT+0.001*fcolor2.a;	// add a touch of underwater terrain
	
	float vfog=DENSITY*lerp(ht,fog_vmin,fog_vmax,1,0);
#else // no water
	float reflect1 = fcolor2.b;
	float type=fcolor2.r;
	float depth=fcolor2.g;
	float vfog=fcolor2.a;
	gl_FragData[0] = fcolor1;
#endif
	gl_FragData[0].a=1;
	//gl_FragData[0] = vec4(z,0,0,1.0);
	gl_FragData[1] = vec4(type,depth,reflect1,vfog);
}

