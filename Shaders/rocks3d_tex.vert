#version 120

#include "textures.h"
#include "attributes.vert"
#include "common.h"

uniform float night_ligting;
uniform float textureScale;
uniform float wscale;
varying vec3 Normal;
varying vec3 EyeDirection;
varying vec3 WorldPos;
varying vec3 TemplatePos;
varying vec3 WorldNormal;
attribute vec4 templatePosition;  // Attribute location 3
attribute vec4 faceNormal;        // Attribute location 4

void main() {

    float depth=templatePosition.w;
    float dfactor = 0.5 * wscale / depth;
    float max_orders = log2(dfactor);
    
    gl_Position = ftransform();
     
    // Normal in eye space
    Normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Eye direction (position in eye space)
    EyeDirection = -(gl_ModelViewMatrix * gl_Vertex).xyz;
    TemplatePos = templatePosition.xyz;  // Pass through
    
    Tangent.w = max_orders;
    
     WorldPos = templatePosition.xyz;
     WorldNormal=normalize(WorldPos);
#ifdef COLOR
     Color = gl_Color;
#endif

#if NTEXS >0
	for(int i=0;i<NTEXS;i++){
		gl_TexCoord[i] = vec4(templatePosition.xyz, 1.0);
	}
	WorldNormal.xyz = faceNormal.xyz;//gl_TexCoord[0].xyz;
	Tangent.z=abs(faceNormal.w);
#endif    
}