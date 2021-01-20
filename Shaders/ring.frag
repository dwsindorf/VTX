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

vec4 setLighting(vec4 BaseColor, vec3 n) {
//TODO - develop lighting model for rings
   return BaseColor;
}
#if NVALS >0
#include "noise_funcs.frag"Color.a
#endif

// ########## Texture section #########################

#if NTEXS >0
#include "tex_funcs.frag"
#endif

// ########## main section #########################
void main(void) {
    vec3 normal=normalize(Normal.xyz);
    bump=vec3(0.0); 
 
	vec4 color=Emission;

#if NTEXS >0
#include "set_tex.frag"
#endif
   // float illumination = 0;
    float illumination = Emission.a;
	color.a=Emission.a;
    if(lighting){
		vec4 c=setLighting(color,normal);
    	color.rgb=c.rgb;
    	illumination=c.a;
    }

#ifdef SHADOWS
    float shadow=texture2DRect(SHADOWTEX, gl_FragCoord.xy).r; // data texture
    color.rgb=mix(color.rgb,Shadow.rgb,Shadow.a*(1.0-shadow));
#endif

	gl_FragData[0] = color;
	gl_FragData[0].a=illumination;
	//if (fbo_write){
		gl_FragData[1]=vec4(Constants.g,illumination,0.0,0);
		gl_FragData[2]=vec4(0,0,0,1);
	//}
}
