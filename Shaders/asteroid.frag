#include "common.h"
#include "utils.h"

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3
#define DEPTH   gl_FragCoord.z
uniform vec3  center;
uniform vec4  Diffuse;
uniform vec4  Specular;
uniform vec4  Ambient;
uniform vec4  Shadow;
uniform float twilite_min;
uniform float twilite_max;
uniform float twilite_dph;
uniform bool  lighting;
uniform float hdr_min;
uniform float hdr_max;

varying vec3 Normal;
varying vec3 EyeDirection;
varying vec3 EyePos;
varying vec3 WorldNormal;

#ifndef LMODE
#define LMODE 0
#endif

#ifndef NLIGHTS
#define NLIGHTS 0
#endif

#if NLIGHTS > 0
vec4 setLighting(vec3 BaseColor, vec3 normal) {
    vec3 eye = normalize(EyeDirection);

#ifdef HDR
    // Match lighting.frag: use bump amplitude to modulate exposure,
    // then apply hemispherical ambient (top_shading) based on surface normal.
    float bumpamp = dot(Normal, bump);
    float exposure = 1.0 - hdr_min * smoothstep(0.0, 1.0, bumpamp / max(hdr_max, 0.001));
    exposure = max(exposure, 0.0);
    float RdotN = dot(normalize(WorldNormal), normal);
    float top_shading = 0.5 * exposure * RdotN + 0.5 + 0.5 * exposure;
#else
    float RdotN = dot(normalize(WorldNormal), normal);
    float top_shading = 0.5 * RdotN + 0.5;
#endif

    vec3 ambient = Ambient.rgb * Ambient.a * BaseColor * top_shading;

    vec3  diffuse  = vec3(0.0);
    vec3  specular = vec3(0.0);
    float shine    = gl_FrontMaterial.shininess;

#ifdef SHADOWS
    float shadow         = texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
    float shadow_diffuse = mix(1.0 - Shadow.a, 1.0, shadow);
#else
    float shadow_diffuse = 1.0;
#endif

    for (int i = 0; i < NLIGHTS; i++) {
        if (gl_LightSource[i].position.w == 0.0)
            continue;

        // Light direction in eye space
        vec3  ldir      = normalize(gl_LightSource[i].position.xyz);
        float LdotN     = (dot(ldir, normal));
        float intensity = 1.0 / gl_LightSource[i].constantAttenuation / float(NLIGHTS);

        diffuse += Diffuse.rgb * gl_LightSource[i].diffuse.rgb * LdotN * intensity;

        if (LdotN > 0.0) {
#if LMODE == 0
            vec3  H   = normalize(ldir + eye);
            float sdp = max(dot(normal, H), 0.0);
#elif LMODE == 1
            vec3  R   = reflect(-ldir, normal);
            float sdp = max(dot(R, eye), 0.0);
#else
            vec3  R     = reflect(-ldir, normal);
            float phong = max(dot(R, eye), 0.0);
            vec3  H     = normalize(ldir + eye);
            float ogl   = max(dot(normal, H), 0.0);
            float sdp   = 0.5 * phong + 0.5 * ogl;
#endif
            specular += Specular.rgb * intensity * pow(sdp, shine) * Specular.a;
        }
    }
    diffuse = mix(diffuse, Shadow.rgb, 1.0 - shadow_diffuse);
    return vec4(ambient
              + diffuse * BaseColor * Diffuse.a * shadow_diffuse
              + specular * shadow_diffuse, 0.0);
}
#endif

#if NTEXS > 0
#include "tex_funcs.frag"
#endif

void main() {
#ifdef COLOR
    vec4 color = Color;
#else
    vec4 color = vec4(1.0);
#endif
    vec3 normal = normalize(Normal);
    bump = vec3(0.0);

#if NTEXS > 0
#include "set_tex.frag"
#endif

#if NBUMPS > 0
    normal = normalize(normal + bump);
#endif
    float illumination = 0.0;

#if NLIGHTS > 0
    if (lighting) {
        vec4 c       = setLighting(color.rgb, normal);
        color.rgb    = c.rgb;
        illumination = c.a;
    }
#endif
    gl_FragData[0] = vec4(clamp(color.xyz, 0.0, 1.0), 1.0);
    // gl_FragData[1].r = ss flag: >2 means terrain pixel (enables haze/fog/HDR effects).
    // Asteroid doesn't currently support atmospheric effects (haze, fog, sky),
    // so write ss=0 to avoid postproc HDR exposure correction and effects-shader
    // processing.  Set to 4 (like planetoid terrain) when asteroid atmosphere is added.
    gl_FragData[1] = vec4(0.0, DEPTH, 0.0, color.a);
    
}
