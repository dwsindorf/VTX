// ########## Lighting section #########################
#include "common.h"
#include "utils.h"

//#ifndef NLIGHTS
//#define NLIGHTS 1
//#endif
// Phong shading variables

uniform bool lighting;
uniform float mipbias;
uniform float emission;
uniform vec3 center;

uniform bool fbo_read;
uniform bool fbo_write;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3

varying vec4 EyeDirection;
varying vec4 Normal;
varying vec4 Color;
uniform vec4 Shadow;
uniform vec4 Emission;
uniform vec4 Diffuse;

uniform float twilite_min;
uniform float twilite_max;
uniform float twilite_dph; // dot product between point-center light-point at horizon

//
//Lighting model
//

vec3 setLighting(vec4 BaseColor) {
	vec3 diffuse = vec3(0, 0, 0);
	//float transmission=Constants1.z;

	for(int i=0;i<NLIGHTS;i++){
		float amplitude = 1.0/gl_LightSource[i].constantAttenuation;
		diffuse        += Diffuse.rgb*gl_LightSource[i].diffuse.rgb*amplitude;
	}
	vec3 TotalDiffuse = diffuse * BaseColor * Diffuse.a;
	return vec3(TotalDiffuse);
}
#if NVALS >0
#include "noise_funcs.h"
#endif

// ########## Texture section #########################

#if NTEXS >0
#include "tex_funcs.frag"
#endif

// ########## main section #########################
void main(void) {
 	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // FBO image (background)
    vec4 color=fcolor1;
#if NTEXS >0
#include "set_tex.frag"
#endif
	vec3 emission = gl_FrontMaterial.emission.rgb;
	float illumination=length(emission);
	//color.a*=Emission.a;
    if(lighting){
		vec3 c=setLighting(color);
    	color.rgb=c.rgb*emission.rgb;
    }
    //color.rgb=mix(color.rgb,fcolor1.rgb,1-Emission.a);
#ifdef SHADOWS
    float shadow=texture2DRect(SHADOWTEX, gl_FragCoord.xy).r; // data texture
    color.rgb=mix(color.rgb,Shadow.rgb,Shadow.a*(1.0-shadow));
    
#endif
	gl_FragData[0] = color;
	//gl_FragData[0] = vec4(Constants1.g,0,0,1);
	//gl_FragData[0].a=illumination;
	//gl_FragData[1]=vec4(Constants1.g,pow(color.a,4)*illumination,0.0,0);
	//gl_FragData[0].a=1;

	//gl_FragData[1]=vec4(0,fcolor1.g,0,0);
	
}
