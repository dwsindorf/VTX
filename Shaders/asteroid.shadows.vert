// asteroid_shadows.vert
#version 120

uniform mat4 smat;
varying vec4 ShadowCoord;

void main() {
   pos = gl_Vertex.xyz;  // already in eye-relative world space
   vec4 eyePos4 = gl_ModelViewMatrix * vec4(pos, 1.0);
   gl_Position = gl_ProjectionMatrix * eyePos4;
   ShadowCoord = smat * eyePos4;
   EyeDirection = -eyePos4;

   gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
    
}