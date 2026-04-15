#version 120

uniform mat4 smat;
varying vec4 ShadowCoord;
varying vec4 EyeDirection;
varying vec4 Normal;

void main() {
    vec3 pos     = gl_Vertex.xyz;
    vec4 eyePos4 = gl_ModelViewMatrix * vec4(pos, 1.0);
    gl_Position  = gl_ProjectionMatrix * eyePos4;
    ShadowCoord  = smat * eyePos4;
    EyeDirection = eyePos4;              // eye-space position as direction
    Normal       = vec4(gl_Normal, 0.0); // not used without _NORMALS_ but must be declared
}
