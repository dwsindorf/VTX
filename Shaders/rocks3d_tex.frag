//#version 120

#include "common.h"
#include "utils.h"

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3

varying vec3 Normal;
varying vec4 WorldNormal;

varying vec3 EyeDirection;
varying vec3 WorldPos;


uniform vec4 Diffuse;
uniform vec4 Ambient;
uniform vec4 Shadow;

uniform float twilite_min;
uniform float twilite_max;

uniform float night_lighting;
uniform bool lighting;

#define DEPTH   gl_FragCoord.z
vec3 setLighting(vec3 BaseColor) {
    vec3 normal = normalize(Normal);
    vec3 eye = normalize(-EyeDirection);
    vec3 radius = normalize(EyeDirection);
    
    // Hemispherical ambient lighting
    float RdotN = dot(radius, normal);
        
    float top_shading = 0.5 * RdotN + 0.5;
    
    // Reduce ambient boost for darker shadows (was 3.0)
    vec3 ambient = BaseColor * Ambient.rgb * Ambient.a * top_shading * 2.0;
    vec3 diffuse = vec3(0.0);
    
    for (int i = 0; i < NLIGHTS; i++) {
        if (gl_LightSource[i].position.w == 0.0)
            continue;
        vec3 lpos=normalize(gl_LightSource[i].position.xyz);
        float dl=-dot(lpos,WorldPos.rgb);
        dl=pow(max(dl,0),0.5);
            
        vec3 light = normalize(gl_LightSource[i].position.xyz + EyeDirection);
        float LdotN = dot(light, normal);
        
        // Keep some compression but less aggressive (use power 0.8 instead of sqrt=0.5)
        LdotN = pow(max(LdotN, 0.0), 0.8);
        
        float intensity = dl / gl_LightSource[i].constantAttenuation / float(NLIGHTS);
        float lpn = LdotN * intensity;
        
        diffuse += Diffuse.rgb * gl_LightSource[i].diffuse.rgb * lpn;
    }
    
    // Increase diffuse contribution for brighter lit areas (was 0.7)
    vec3 TotalDiffuse = BaseColor * diffuse * Diffuse.a * 0.85;
    
    return ambient + TotalDiffuse;
 
}
#if NTEXS >0
#include "tex_funcs.frag"
#endif

void main() {
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif

#if NTEXS >0
#include "set_tex.frag"
#endif
 
#if NBUMPS > 0
        // Apply accumulated bump to normal
        Normal = normalize(Normal + bump);
#endif
       
#if NLIGHTS >0
	if(lighting){
		vec3 c=setLighting(color.rgb);
    	color.rgb=c.rgb;
    }
#endif
     
#ifdef SHADOWS
     float shadow=1.0-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
     color.rgb=mix(color.rgb,Shadow.rgb,shadow*Shadow.a); 
#endif  
	vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // Params
   // color.rgb=vec3(Tangent.z,0,0);
   
    gl_FragData[0]=vec4(color.xyz,1);
    gl_FragData[1]=vec4(4,DEPTH,0,color.a); 

    //gl_FragData[1]=vec4(fcolor2); // pass thru
    
}