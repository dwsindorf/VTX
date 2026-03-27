#version 120

#include "textures.h"
#include "attributes.vert"
#include "common.h"

uniform float night_ligting;
uniform float textureScale;
uniform float wscale;
uniform vec4 mpoint;
varying vec3 Normal;
varying vec3 EyeDirection;
varying vec3 EyePos;
varying vec3 TemplatePos;
varying vec3 WorldNormal;
attribute vec4 templatePosition;
attribute vec4 faceNormal;

// Per-instance attributes
attribute vec3 rockEyeCenter;
attribute float rockSize;
attribute float rockRval;

// Per-frame rotation uniforms (same for all rocks)
uniform vec3 rockRight;
uniform vec3 rockForward;
uniform vec3 rockUp;

#if NTEXS > 0
struct tex2d_info {
    float bumpval;
    float bumpamp;
    float texamp;
    float bump_delta;
    float bump_damp;
    float orders_delta;
    float orders_atten;
    float orders_bump;
    float scale;
    float bias;
    float logf;
    float dlogf;
    float orders;
    float phi_bias;
    float height_bias;
    float bump_bias;
    float slope_bias;
    float near_bias;
    float far_bias;
    float tilt_bias;
    bool  randomize;
    bool  seasonal;
    bool  t1d;
    bool  triplanar;
    bool  t3d;
    int   instance;
    int   active;
};
uniform tex2d_info tex2d[NTEXS];
#endif

void main() {
    // Transform local rock space → world-relative-to-eye
    vec3 localPos = gl_Vertex.xyz;
    vec3 pos = rockEyeCenter
             + (localPos.x * rockRight
             +  localPos.y * rockForward
             +  localPos.z * rockUp) * rockSize;

    // Apply ModelView rotation + Projection
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(pos, 1.0);

    // Transform normal through same rotation
    vec3 worldNormal = normalize(
        gl_Normal.x * rockRight +
        gl_Normal.y * rockForward +
        gl_Normal.z * rockUp);
    Normal = normalize(gl_NormalMatrix * worldNormal);

    // Eye direction in eye space
    vec4 eyePos4 = gl_ModelViewMatrix * vec4(pos, 1.0);
    EyeDirection = -eyePos4.xyz;

    // Depth for LOD calculations
    float depth = length(eyePos4.xyz);
    float dfactor = 10.0 * wscale / depth;
    Tangent.w = log2(dfactor);

    // templatePos with per-instance rval offset for texture variation
    vec3 tp = templatePosition.xyz + rockRval;
    TemplatePos = tp;

    EyePos = normalize(mpoint.xyz);
    WorldNormal = normalize(tp);

#ifdef COLOR
    Color = gl_Color;
#endif

#if NTEXS > 0
    for(int i = 0; i < NTEXS; i++) {
        float scale = tex2d[i].scale * textureScale;
        vec3 coords = tp * scale;
        if(tex2d[i].t1d)
            coords += tex2d[i].bias;
        gl_TexCoord[i] = vec4(coords, depth);
    }
    WorldNormal = faceNormal.xyz;
    float slope = dot(worldNormal, rockUp);
    Tangent.z = abs(slope);
    
#endif
}