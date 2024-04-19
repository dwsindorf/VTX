

#include "utils.h"

#if NTEXS >0
uniform sampler2D samplers2d[NTEXS];
#endif
varying vec4 Normal;
varying vec4 pnorm;
varying vec4 TexVars;

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
		vec3 light= normalize(gl_LightSource[i].position.xyz);		
		float LdotN= dot(light,Normal.xyz);// for day side diffuse lighting
		float f=max(LdotN,0.0);
		if(TexVars.w>0){
			float Ldotn = dot(light,pnorm.xyz);// for branch lighting
        	f*=Ldotn;
        }
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
#if NTEXS >0
	int i=TexVars.b;
	if(i>=0){
		vec2 l_uv=gl_TexCoord[0].xy;
  		color=texture2D(samplers2d[i],l_uv);
	}
#endif
	if(lighting)
    	color.rgb=setLighting(color.rgb);
#ifdef SHADOWS
     float shadow=1.0-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
     color.rgb=mix(color.rgb,color.rgb,Shadow.rgb,shadow*Shadow.a);
#endif  
 
#ifdef HAZE
	float z=DEPTH; // depth buffer
	float depth=1.0/(ws2*z+ws1); // distance
	float d=haze_grad==0.0?1e-4:lerp(depth,0.0,haze_grad*haze_zfar,0.0,1.0); // same as in effects.frag
	float h=haze_ampl*Haze.a*pow(d,8.0*haze_grad); // same as in effects.frag
	color.rgb=mix(color.rgb,Haze.rgb,h);
#endif
    
 	gl_FragData[0]=color;
	gl_FragData[1]=vec4(0,DEPTH,0,color.a); // set type to 0 to bypass second haze correction in effects.frag

}
// ########## end plants.frag #########################
