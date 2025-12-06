#version 120

varying vec3 Normal;
varying vec3 EyeDir;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    // Normal in eye space
    Normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Eye direction (position in eye space)
    EyeDir = (gl_ModelViewMatrix * gl_Vertex).xyz;
}