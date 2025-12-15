#version 120

#include "utils.h"

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3

varying vec3 Normal;
varying vec3 EyeDir;

uniform vec4 Diffuse;
uniform vec4 Ambient;
uniform vec4 Shadow;

uniform float night_lighting;

#define DEPTH   gl_FragCoord.z

vec3 setLighting(vec3 BaseColor) {
    vec3 normal = normalize(Normal);
    vec3 eye = normalize(-EyeDir);
    vec3 radius = normalize(EyeDir);
    
    // Hemispherical ambient lighting
    float RdotN = dot(radius, normal);
        
    float top_shading = 0.5 * RdotN + 0.5;
    
    // Reduce ambient boost for darker shadows (was 3.0)
    vec3 ambient = BaseColor * Ambient.rgb * Ambient.a * top_shading * 2.0;
    vec3 diffuse = vec3(0.0);
    
    for (int i = 0; i < NLIGHTS; i++) {
        if (gl_LightSource[i].position.w == 0.0)
            continue;
            
        vec3 light = normalize(gl_LightSource[i].position.xyz + EyeDir);
        float LdotN = dot(light, normal);
        
        // Keep some compression but less aggressive (use power 0.8 instead of sqrt=0.5)
        LdotN = pow(max(LdotN, 0.0), 0.8);
        
        float intensity = night_lighting / gl_LightSource[i].constantAttenuation / float(NLIGHTS);
        float lpn = LdotN * intensity;
        
        diffuse += Diffuse.rgb * gl_LightSource[i].diffuse.rgb * lpn;
    }
    
    // Increase diffuse contribution for brighter lit areas (was 0.7)
    vec3 TotalDiffuse = BaseColor * diffuse * Diffuse.a * 0.85;
    
    return ambient + TotalDiffuse;
    //return vec3(night_lighting,0,0);
}

void main() {
    vec3 rockColor = vec3(0.6, 0.5, 0.4);
    vec3 color = setLighting(rockColor);
    
#ifdef SHADOWS
     float shadow=1.0-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
     color.rgb=mix(color.rgb,Shadow.rgb,shadow*Shadow.a); 
#endif  

    gl_FragData[0]=vec4(color, 1.0);
    gl_FragData[1]=vec4(2,DEPTH,1,1.0); // 
    
    //gl_FragColor[0] = vec4(night_lighting,0,0,1.0);
}