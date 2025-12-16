#version 120

uniform float night_ligting;
varying vec3 Normal;
varying vec3 EyeDir;
varying vec3 VertexColor;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    // Normal in eye space
    Normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Eye direction (position in eye space)
    EyeDir = (gl_ModelViewMatrix * gl_Vertex).xyz;
    
    VertexColor = gl_Color.rgb;
}