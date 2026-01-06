#version 120

//#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

uniform float night_ligting;
uniform float textureScale;
varying vec3 Normal;
varying vec3 EyeDirection;
varying vec3 WorldPos;
varying vec3 TemplatePos;
varying vec4 WorldNormal;
attribute vec3 templatePosition;  // Attribute location 3
attribute vec3 faceNormal;        // Attribute location 4

void main() {
    gl_Position = ftransform();
     
    // Normal in eye space
    Normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Eye direction (position in eye space)
    EyeDirection = -(gl_ModelViewMatrix * gl_Vertex).xyz;
    TemplatePos = templatePosition;  // Pass through
    
     WorldPos = templatePosition;
#ifdef COLOR
     Color = gl_Color;
#endif

#if NTEXS >0
	for(int i=0;i<NTEXS;i++){
		gl_TexCoord[i] = vec4(templatePosition, 1.0);
	}
	WorldNormal.xyz = faceNormal;//gl_TexCoord[0].xyz;
#endif    
}