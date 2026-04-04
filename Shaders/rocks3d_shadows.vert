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

<<<<<<< HEAD
void main() {
    vec3 localPos = gl_Vertex.xyz;
    vec3 pos = rockEyeCenter
             + (localPos.x * rockRight
             +  localPos.y * rockForward
             +  localPos.z * rockUp) * rockSize;

    vec4 eyePos4 = gl_ModelViewMatrix * vec4(pos, 1.0);
    
=======
uniform bool useInstancing;

void main() {
    vec3 pos;

    if (useInstancing) {
        vec3 localPos = gl_Vertex.xyz;
        pos = rockEyeCenter
            + (localPos.x * rockRight
            +  localPos.y * rockForward
            +  localPos.z * rockUp) * rockSize;
    } else {
        pos = gl_Vertex.xyz;  // already in eye-relative world space
    }

    vec4 eyePos4 = gl_ModelViewMatrix * vec4(pos, 1.0);
>>>>>>> test2
    gl_Position = gl_ProjectionMatrix * eyePos4;
    ShadowCoord = smat * eyePos4;
    EyeDirection = -eyePos4;

<<<<<<< HEAD
    vec3 worldNormal = normalize(
        gl_Normal.x * rockRight +
        gl_Normal.y * rockForward +
        gl_Normal.z * rockUp);
    Normal.xyz = normalize(gl_NormalMatrix * worldNormal);
=======
    vec3 norm = useInstancing ?
        normalize(gl_Normal.x * rockRight +
                  gl_Normal.y * rockForward +
                  gl_Normal.z * rockUp) :
        gl_Normal;
    Normal.xyz = normalize(gl_NormalMatrix * norm);
>>>>>>> test2
}