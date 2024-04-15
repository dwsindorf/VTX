
#include "common.h"
#include "utils.h"

varying vec3 Norm;
varying vec3 pnorm;
varying vec4 EyeDirection;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3

uniform vec4 Diffuse;
uniform vec4 Shadow;
uniform vec4 Haze;
uniform float haze_zfar;
uniform float haze_ampl;
uniform float haze_grad;
uniform float ws1;
uniform float ws2;

uniform bool lighting;

#define DEPTH   gl_FragCoord.z

varying vec4 Color;

vec3 setLighting(vec3 BaseColor) {
	vec3 diffuse = vec3(0, 0, 0);
    
	for(int i=0;i<NLIGHTS;i++){
		vec3 light      = normalize(gl_LightSource[i].position.xyz);
		float LdotN     = dot(light,Norm.xyz);// for day side diffuse lighting
		//float Ldotn     = dot(light,pnorm.xyz);// for day side diffuse lighting
        float f=max(LdotN,0.0);
		float amplitude = 1.0/gl_LightSource[i].constantAttenuation;
		float lpn       = f*amplitude;
		lpn=clamp(lpn,0.0,1.0);
	
		diffuse        += Diffuse.rgb*gl_LightSource[i].diffuse.rgb*lpn;
	}
	vec3 TotalDiffuse = diffuse * BaseColor * Diffuse.a;
	return TotalDiffuse;
}


//-------------------------------------------------------------

// ########## main section #########################
void main(void) {
	vec4 color =Color;
	 color.rgb=setLighting(color.rgb);
    
 	gl_FragData[0]=color;
	gl_FragData[1]=vec4(0,DEPTH,0,color.a); // set type to 0 to bypass second haze correction in effects.frag

}
// ########## end plants.frag #########################
