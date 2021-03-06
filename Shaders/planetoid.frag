// ########## begin planetoid.frag #########################
//#extension GL_ARB_gpu_shader_fp64 : enable
//#define DBLSTEST
//#define COLOR
#include "common.h"
#include "utils.h"

#ifdef TEST4
//#define N3D
#endif
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
#define NLIGHTS 1
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


#ifndef LMODE
#define LMODE 0
#endif

uniform vec4 Diffuse;   // for some reason can't seem to set & use gl_FrontMaterial.diffuse ??
uniform vec4 Shadow;

#ifdef GRID
uniform vec4 phi_color;
uniform vec4 theta_color;
uniform float grid_spacing;
vec3 addGrid(vec3 color){
    vec2 p = vec2(THETA,PHI)*180;
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
//Lighting model
//
#if NLIGHTS >0
vec4 setLighting(vec3 BaseColor, vec3 n, vec3 b) {
	vec3 bmp  = -gl_NormalMatrix*b;
	vec3 eye = normalize(EyeDirection.xyz);
	vec3 radius = -normalize(EyeDirection.xyz+center);

	vec3 normal  = normalize(n+bmp);
	vec3 ambient = vec3(0.0, 0.0, 0.0);
	vec3 emission = vec3(0.0, 0.0, 0.0);
	vec3 diffuse = vec3(0.0, 0.0, 0.0);
	vec3 specular = vec3(0.0, 0.0, 0.0);
#ifdef HDR
	float bumpamp=dot(n,bmp);
    float exposure = 1.0-hdr_min*smoothstep(0.0,1.0,bumpamp/hdr_max);
	exposure = max(exposure,0.0);
	exposure*=exposure;	
	float RdotN   = dot(radius,normal); // for top lighting on nightside
	float top_shading  = 0.5*exposure*RdotN+0.5+0.5*exposure;  
#else
	float RdotN   = dot(radius,normal); // for top lighting on nightside
	float top_shading  = 0.5*RdotN+0.5;  // models hemispherical lighting (light from all angles)
#endif
	//float top_shading  = RdotN*RdotN*RdotN; // models a single light source from above (a bit more dramatic)
    vec3 topcolor=gl_FrontMaterial.ambient.rgb*(top_shading*gl_FrontMaterial.ambient.a);
	ambient = topcolor;
	emission = gl_FrontMaterial.emission.rgb;
	float illumination = gl_FrontMaterial.emission.a;
#ifdef SHADOWS
    float shadow=texture2DRect(SHADOWTEX, gl_FragCoord.xy).r; // data texture
    float shadow_diffuse=lerp(Shadow.a,0.0,1.0,1.0,shadow);
    float shadow_specular=shadow;//lerp(shadow,0.2,1.0,0.0,shadow);
#else
	float shadow_diffuse=1.0;
	float shadow_specular=1.0;
#endif


	for(int i=0;i<NLIGHTS;i++){
	    if(gl_LightSource[i].position.w==0.0)
	    	continue;
		vec3 light      = normalize(gl_LightSource[i].position.xyz+EyeDirection.xyz);
		float LdotN     = dot(light,normal);  // for day side diffuse lighting
		float LdotR     = dot(light,radius ); // for horizon band calculation
		float horizon   = lerp(LdotR,twilite_dph+twilite_min,twilite_dph+twilite_max,0.0,1.0); // twilite band
		float intensity = 1.0/gl_LightSource[i].constantAttenuation;
		float lpn       = LdotN*intensity*horizon;
		diffuse        += Diffuse.rgb*gl_LightSource[i].diffuse.rgb*max(lpn,top_shading*gl_FrontMaterial.ambient.a);
		//illumination   += 2.0*LdotN*intensity*horizon;
		illumination   += horizon;
		ambient+=topcolor*(3.0*horizon);
        if(LdotN>-0.5){
			float damp = lerp(LdotN,-0.5,0.0,0.0,1.0); // prevents false specular highlites on backfaces
#if LMODE == 0 // phong-blinn: uses half-vector (this is the openGL light model)
			vec3 H = normalize((light + eye)*0.5);
			float sdp   = max(0.0, dot(normal,H));
#elif LMODE == 1 // phong: uses light vector reflected about normal
			vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
			float sdp   = max(0.0, dot(R, eye));
#else // combined mode (ogl seems better at low angles phong better at higher angles)
			vec3 R = reflect(-light,normal);  // normalize(((2.0*normal )*LdotN)-light);
			float phong   = max(0.0, dot(R, eye));
			vec3 H = normalize((light + eye)*0.5);
			float ogl   = max(0.0, dot(normal,H));
			float sdp=(0.5*phong+0.5*ogl);
#endif
			float pf        = damp*intensity*horizon*pow(sdp, gl_FrontMaterial.shininess);
			
			specular       += gl_LightSource[i].specular.rgb * pf*shadow_specular;
			//illumination   += pf*gl_LightSource[i].specular.a;
        }
	}
	diffuse.rgb=mix(diffuse.rgb,Shadow.rgb,1.0-shadow_diffuse);
	vec3 TotalEmission = emission.rgb * BaseColor;
	vec3 TotalAmbient = ambient.rgb * BaseColor;
	vec3 TotalDiffuse = diffuse.rgb * BaseColor*Diffuse.a*shadow_diffuse;
	vec3 TotalSpecular = specular.rgb;
	//illumination=clamp(illumination,0.0,1.0);
	return vec4(TotalAmbient +TotalEmission + TotalDiffuse + TotalSpecular,illumination);
}
#endif
//########## 3D noise section #########################

#if NVALS >0
#include "noise_funcs.h"
#endif

// ########## Texture section #########################

#if NTEXS >0
#include "tex_funcs.frag"
#endif

// ########## main section #########################
void main(void) {
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
    float shadow=1.0-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
    color.rgb=mix(color.rgb,Shadow.rgb,shadow*Shadow.a);
#endif    
    float depth=gl_FragCoord.z;
    vec3 eye = normalize(EyeDirection.xyz);
    float reflect1=dot(normal,eye); // reflection angle
#ifdef VIEWOBJ
    float vfog=DENSITY*lerp(HT,fog_vmin,fog_vmax,1,0);
	gl_FragData[1]=vec4(Constants1.g,depth,reflect1,vfog);
#else  // moons
	gl_FragData[1]=vec4(0,illumination,0.0,0.0);
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
	//gl_FragData[0] = vec4(bmpht,0,0,1);//color;
	


}
// ########## end planetoid.frag #########################
