#version 120

#include "utils.h"

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3

varying vec3 Normal;
varying vec3 EyeDir;
varying vec3 WorldPos;
varying vec3 VertexColor;
varying vec3 TemplatePos;

uniform vec4 Diffuse;
uniform vec4 Ambient;
uniform vec4 Shadow;
uniform float night_lighting;

uniform sampler2D rockTexture;
uniform float textureScale;
uniform float bumpScale;
uniform int useTexture;  // 0=color only, 1=texture only, 2=color*texture

// Triplanar mapping function
vec3 triplanarMapping(vec3 pos) {
    vec3 blending = abs(pos);
    blending = normalize(max(blending, 0.00001));
    float b = (blending.x + blending.y + blending.z);
    blending /= vec3(b, b, b);
    
    vec2 xaxis = pos.yz * textureScale;
    vec2 yaxis = pos.xz * textureScale;
    vec2 zaxis = pos.xy * textureScale;
    
    vec3 xColor = texture2D(rockTexture, xaxis).rgb;
    vec3 yColor = texture2D(rockTexture, yaxis).rgb;
    vec3 zColor = texture2D(rockTexture, zaxis).rgb;
    
    return xColor * blending.x + yColor * blending.y + zColor * blending.z;
}
// Simple bump mapping (uses texture luminance for height)
vec3 computeBumpedNormal(vec3 worldPos, vec3 normal) {
    vec3 blending = abs(normal);
    blending = normalize(max(blending, 0.00001));
    float b = (blending.x + blending.y + blending.z);
    blending /= vec3(b, b, b);
    
    float texelSize = 1.0 / 1024.0;
    float scale = bumpScale * 0.1;
    
    // Sample height for each projection plane
    vec2 xaxis = worldPos.yz * textureScale;
    vec2 yaxis = worldPos.xz * textureScale;
    vec2 zaxis = worldPos.xy * textureScale;
    
    // X-axis projection bump
    vec3 xBump = vec3(0, 0, 0);
    if (blending.x > 0.1) {
        float h = texture2D(rockTexture, xaxis).r;
        float hr = texture2D(rockTexture, xaxis + vec2(texelSize, 0)).r;
        float hu = texture2D(rockTexture, xaxis + vec2(0, texelSize)).r;
        xBump = vec3(0, (hr-h)*scale, (hu-h)*scale);
    }
    
    // Y-axis projection bump (similar)
    // Z-axis projection bump (similar)
    // ... (simplified for brevity)
    
    return normalize(normal + xBump * blending.x);
}

vec3 setLighting(vec3 BaseColor) {
    vec3 normal = normalize(Normal);
    
    // Hemispherical ambient
    vec3 radius = normalize(EyeDir);
    float RdotN = dot(radius, normal);
    float top_shading = 0.5 * RdotN + 0.5;
    
    vec3 ambient = BaseColor * Ambient.rgb * Ambient.a * top_shading * 2.0;
    vec3 diffuse = vec3(0.0);
    
    for (int i = 0; i < NLIGHTS; i++) {
        if (gl_LightSource[i].position.w == 0.0)
            continue;
            
        vec3 light = normalize(gl_LightSource[i].position.xyz + EyeDir);
        float LdotN = dot(light, normal);
        LdotN = pow(max(LdotN, 0.0), 0.8);
        
        float intensity = night_lighting / gl_LightSource[i].constantAttenuation / float(NLIGHTS);
        float lpn = LdotN * intensity;
        
        diffuse += Diffuse.rgb * gl_LightSource[i].diffuse.rgb * lpn;
    }
    
    vec3 TotalDiffuse = BaseColor * diffuse * Diffuse.a * 0.85;
    
    return ambient + TotalDiffuse;
}

void main() {
    vec3 rockColor;
    
    if (useTexture == 1) {
        // Texture only
        rockColor = triplanarMapping(TemplatePos);
    } else if (useTexture == 2) {
        // Color * Texture
        vec3 texColor = triplanarMapping(TemplatePos);
        rockColor = VertexColor * texColor;
    } else {
        // Color only (default)
        rockColor = VertexColor;
    }
    
    vec3 color = setLighting(rockColor);
    
#ifdef SHADOWS
    float shadow = 1.0 - texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
    color.rgb = mix(color.rgb, Shadow.rgb, shadow * Shadow.a);
#endif
 // DEBUG 3: Show the actual UV coordinates being used
    gl_FragData[0] = vec4(color, 1.0);
    gl_FragData[1] = vec4(2, gl_FragCoord.z, 1, 1.0);
}