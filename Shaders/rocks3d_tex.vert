#version 120

#include "attributes.h"
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
//#include "set_tex.vert"
//#include "set_common.vert"
#ifdef TX0
    gl_TexCoord[0] = vec4(0.4*templatePosition, 1.0);
#endif
#ifdef TX1
    gl_TexCoord[1] = vec4(templatePosition, 1.0);
#endif

    WorldNormal.xyz = gl_Normal;
    
}