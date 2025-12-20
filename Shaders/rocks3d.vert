#version 120

uniform float night_ligting;
varying vec3 Normal;
varying vec3 EyeDir;
varying vec3 VertexColor;
varying vec3 WorldPos;
varying vec3 TemplatePos;  // ADD THIS
attribute vec3 templatePosition;  // Attribute location 3

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
     
    // Normal in eye space
    Normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Eye direction (position in eye space)
    EyeDir = (gl_ModelViewMatrix * gl_Vertex).xyz;
    TemplatePos = templatePosition;  // Pass through
    
    WorldPos = gl_Vertex.xyz;
    
    VertexColor = gl_Color.rgb;
}