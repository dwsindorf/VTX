// ########## begin planetoid.frag #########################
//#extension GL_ARB_gpu_shader_fp64 : enable
//#define DBLSTEST
//#define COLOR
#include "common.h"
#include "utils.h"

uniform vec3 center;
uniform vec3 object;

uniform bool fbo_read;
uniform bool fbo_write;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

uniform float rscale;

#define SHADOWTEX FBOTex3

uniform float hdr_min;
uniform float hdr_max;

const float m2f=5280.0;


// ########## Lighting section #########################

#ifndef NLIGHTS
#define NLIGHTS 0
#endif

uniform float haze_grad;
uniform float haze_zfar;
uniform float fog_vmin;
uniform float fog_vmax;

uniform vec4 Haze;

uniform float sky_alpha;
uniform bool lighting;
uniform float twilite_min;
uniform float twilite_max;
uniform float twilite_dph; // dot product between point-center light-point at horizon
uniform float shadow_darkness;
varying vec4 EyeDirection;
varying vec4 Normal;


uniform vec4 Diffuse;   // for some reason can't seem to set & use gl_FrontMaterial.diffuse ??
uniform vec4 Shadow;

uniform vec4 WaterColor1;
uniform vec4 WaterColor2;
uniform vec4 IceColor1;
uniform vec4 IceColor2;
uniform float water_clarity;
uniform float ice_clarity;


#ifdef GRID
vec2 spherical(vec3 rec)
{
	float t,p,r;
	r=length(rec);
	t=asin(rec.y/r)*DPR;
	p=atan(-rec.z,rec.x)*DPR;
	return vec2(t,p);
}
uniform vec4 phi_color;
uniform vec4 theta_color;
uniform float grid_spacing;
vec3 addGrid(vec3 color){
	vec3 pm=(Vertex1-0.5)*2;
	vec2 p=spherical(pm);
    //vec2 p = vec2(THETA,PHI)*180;
    vec2 f  = abs(fract (p/grid_spacing));
    vec2 df = 4*fwidth(p/grid_spacing);
    vec2 g = smoothstep(-df,df , f);
    g *=1.0-g;
    g = min(2.0*g,1.0);
    	
  	vec3 rcol=mix(color,theta_color.rgb,g.x);  
    rcol=mix(rcol,phi_color.rgb,g.y);
     
	return rcol;		
}
#endif
#ifdef CONTOURS
uniform vec4 contour_color;
uniform float contour_spacing;

vec3 addContours(vec3 color){
	float y =m2f*HT;
	float f  = abs(fract (y * 1e6/contour_spacing));
	float dy = 4*fwidth(y * 1e6/contour_spacing);
	float g = smoothstep(-dy,dy , f);
	g *=1.0-g;
	g = min(2*g,1.0);
 	vec3 col=mix(color,contour_color,g);
	return vec3(col.rgb);
}
#endif
//
//########## Lighting section #########################

#if NLIGHTS >0
#include "lighting.frag"
#endif


//########## 3D noise section #########################

#if NVALS >0
#include "noise_funcs.h"
#endif

// ########## Texture section #########################

#if NTEXS >0
#include "tex_funcs.frag"
#endif
//
//#if NTEXS >0
//	TEX_VARS
//#endif
// ########## main section #########################
void main(void) {
	amplitude = 1.0;
	g=0.0; 	

#ifdef NOTILE
    warmup();
#endif
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif
    vec3 normal=normalize(Normal.xyz);
    bump=vec3(0.0); 
#ifdef NCC
	NOISE_COLOR(NCC);
#endif

#if NTEXS >0
#include "set_tex.frag"
#endif
    float illumination = 0.0;
#if NLIGHTS >0
	color.a=1.0;
    if(lighting){
		vec4 c=setLighting(color.rgb,normal,bump);
    	color.rgb=c.rgb;
    	illumination=c.a;//sqrt(c.a);
    }
#endif
#ifdef SHADOWS
    //float shadow=1.0-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
    //color.rgb=mix(color.rgb,Shadow.rgb,shadow*Shadow.a);
    //color.rgb = gl_FragCoord.xyz;
    //color.rgb=vec3(shadow,0,0);
#endif    
    float depth=gl_FragCoord.z;
    vec3 eye = normalize(EyeDirection.xyz);
    float reflect1=dot(normal,eye); // reflection angle
#ifdef VIEWOBJ
    float vfog=DENSITY*lerp(HT,fog_vmin,fog_vmax,1,0);
	gl_FragData[1]=vec4(Constants1.g,depth,reflect1,vfog);
#else  // moons 
	gl_FragData[1]=vec4(0,illumination,0.05,0.0); // set first component (type) to 0 so reflections work
#endif
#ifdef VIEWOBJ
#ifdef CONTOURS
    color.rgb=addContours(color.rgb);
#endif 
#ifdef GRID
    color.rgb=addGrid(color.rgb);
#endif   
#endif

	gl_FragData[0] = color;
	//gl_FragData[0] = abs(normalize(Normal));
	//float ht=Constants1.x/rscale+1.0;
	//gl_FragData[0] = vec4(ht*0.5,0,0,1);

}
// ########## end planetoid.frag #########################
