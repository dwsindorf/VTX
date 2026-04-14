#version 120

uniform mat4 smat;
varying vec4 ShadowCoord;

void main() {
    vec3 pos = gl_Vertex.xyz;
    vec4 eyePos4 = gl_ModelViewMatrix * vec4(pos, 1.0);
    gl_Position = gl_ProjectionMatrix * eyePos4;
    ShadowCoord = smat * eyePos4;
}
