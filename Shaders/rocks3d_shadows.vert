#version 120

#include "common.h"

// Per-instance attributes
attribute vec3 rockEyeCenter;
attribute float rockSize;
attribute float rockRval;

// Per-frame rotation uniforms
uniform vec3 rockRight;
uniform vec3 rockForward;
uniform vec3 rockUp;

uniform mat4 smat;
varying vec4 ShadowCoord;
varying vec4 EyeDirection;
varying vec4 Normal;

void main() {
    vec3 localPos = gl_Vertex.xyz;
    vec3 pos = rockEyeCenter
             + (localPos.x * rockRight
             +  localPos.y * rockForward
             +  localPos.z * rockUp) * rockSize;

    vec4 eyePos4 = gl_ModelViewMatrix * vec4(pos, 1.0);
    
    gl_Position = gl_ProjectionMatrix * eyePos4;
    ShadowCoord = smat * eyePos4;
    EyeDirection = -eyePos4;

    vec3 worldNormal = normalize(
        gl_Normal.x * rockRight +
        gl_Normal.y * rockForward +
        gl_Normal.z * rockUp);
    Normal.xyz = normalize(gl_NormalMatrix * worldNormal);
}