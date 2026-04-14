#version 120

#include "textures.h"
#include "attributes.vert"
#include "common.h"

uniform float wscale;
uniform float textureScale;
uniform vec4 mpoint;        // asteroid world position (for EyePos horizon calc)

varying vec3 Normal;
varying vec3 EyeDirection;
varying vec3 EyePos;
varying vec3 TemplatePos;
varying vec3 WorldNormal;

attribute vec4 templatePosition;
attribute vec4 faceNormal;

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
    // Vertices already in eye-relative world space — no instancing transform needed
    vec3 pos  = gl_Vertex.xyz;
    vec3 norm = gl_Normal;

    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(pos, 1.0);

    vec4 eyePos4   = gl_ModelViewMatrix * vec4(pos, 1.0);
    EyeDirection   = -eyePos4.xyz;
    Normal         = normalize(gl_NormalMatrix * norm);

    // LOD depth factor (used by texture LOD calculations)
    float depth    = length(eyePos4.xyz);
    float dfactor  = 10.0 * wscale / depth;
    Tangent.w      = log2(dfactor);

    // Template position drives texture coordinates and WorldNormal
    vec3 tp   = templatePosition.xyz;
    TemplatePos   = tp;
    EyePos        = normalize(mpoint.xyz);   // direction to asteroid center from eye
    WorldNormal   = normalize(tp);           // outward direction in object space

	vec3 wn = normalize(tp);  // templatePosition normalized = outward surface direction
	vec3 up = vec3(0.0, 1.0, 0.0);
	if (abs(dot(wn, up)) > 0.9)
	    up = vec3(1.0, 0.0, 0.0);  // avoid degeneracy at poles
	vec3 tangent  = normalize(cross(wn, up));
	vec3 binormal = normalize(cross(wn, tangent));
	
	// Transform tangent to eye space and store in Tangent.xy
	vec3 eyeTangent = normalize(mat3(gl_ModelViewMatrix) * tangent);
	Tangent.x = eyeTangent.x;
	Tangent.y = eyeTangent.y;
	
#ifdef COLOR
    Color = gl_Color;
#endif

#if NTEXS > 0
    for (int i = 0; i < NTEXS; i++) {
        float scale  = tex2d[i].scale * textureScale;
        vec3  coords = tp * scale;
        if (tex2d[i].t1d)
            coords += tex2d[i].bias;
        gl_TexCoord[i] = vec4(coords, depth);
    }
    WorldNormal = faceNormal.xyz;   // face normal for slope-based texture blending
    float slope = dot(norm, up);    
    Tangent.z = abs(slope);
#endif
}
