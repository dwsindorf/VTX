#include "utils.h"

#if NTEXS >0
uniform sampler2D samplers2d[NTEXS];
#endif

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3


uniform vec4 Diffuse;
uniform vec4 Ambient;
uniform vec4 Shadow;
uniform vec4 Haze;
uniform float haze_zfar;
uniform float haze_ampl;
uniform float haze_grad;
uniform float ws1;
uniform float ws2;
//uniform float bump_ampl;
uniform float bump_delta;
uniform float norm_scale;
uniform bool lighting;
uniform float twilite_min;
uniform float twilite_max;
uniform float fog_vmin;
uniform float fog_vmax;

#define DEPTH   gl_FragCoord.z
#define TEXID   TexVars.z

#define LINE    0
#define BRANCH  1
#define LEAF    2
#define SPLINE  3
#define MODE    (int)(TexVars.w+0.1)

varying vec4 Color;
varying vec4 Pnorm;
varying vec3 Normal;
varying vec4 TexVars;
varying vec4 ImageVars;
varying vec2 FogVars;
// FogVars comes from a dedicated attribute (FogAttributes, see Plants.h/.cpp)
// rather than Constants1 - Constants1 is fully packed with branch radius/taper
// data for plants. Macros named to match planetoid.frag/common.h's convention.
#define HT      FogVars.x
#define DENSITY FogVars.y

vec3 test;


vec2 sprite(vec2 l_uv){
    
    float cols=ImageVars.x;  
    float rows=ImageVars.y;
     
    vec2 scale=vec2(cols,rows);
         
    float x=ImageVars.z;
    float y=ImageVars.w;
	vec2 pt3=l_uv+vec2(x,y);
	
	pt3/=scale;
    	
	return pt3;
}
#define AVE(v) (v.x+v.y+v.z)
vec3 getNormal(float nscale){
	vec3 bn=gl_NormalMatrix*nscale*Pnorm.xyz;
	vec3 normal=Normal-bn;
	int texid=TEXID;
	if(MODE==LINE || texid<0)
		return normal;
#ifdef BUMPS
    float bump_ampl=Pnorm.w;
	float delta=bump_delta;
	vec2 l_uv=sprite(gl_TexCoord[0].xy);
	vec4 tval=texture2D(samplers2d[texid],l_uv);
	float tva=AVE(tval);
	vec2 ds=vec2(l_uv.x+delta,l_uv.y);
	vec2 dt=vec2(l_uv.x,l_uv.y+delta);
	vec4 tcs=texture2D(samplers2d[texid],ds);
	vec4 tct=texture2D(samplers2d[texid],dt);
	float tsa=AVE(tcs);
	float tta=AVE(tct);
	vec3 tc=vec3(tsa-tva,tta-tva, 0.0);
	
	vec3 tangent=vec3(0,1,0);	
	vec3 binormal=cross(tangent,normal);
	mat3 trans_mat=mat3(tangent, binormal, normal);
	vec3 bv=(bump_ampl/delta)*trans_mat*(tc);
	normal += bv;
#endif
	return normal;
}
// Low-sun tuning:
//  WRAP      - wrap (half-Lambert style) lighting: lets the side turned away
//              from a low sun still get some light (sky fill / scatter)
//              instead of hard-clamping to ambient-only. 0 = old behavior.
//  LEAF_XMIT - fraction of light transmitted through backlit leaves.
#define WRAP      0.35
#define LEAF_XMIT 0.5

vec3 setLighting(vec3 BaseColor) {
	vec3 diffuse = vec3(0, 0, 0);
	float nscale=TexVars.x;
	vec3 up=normalize(Normal);
 	for(int i=0;i<NLIGHTS;i++){
 		vec3 light= normalize(gl_LightSource[i].position.xyz);
 		float LdotR= dot(light,up);// sun elevation (horizon gate only)
		float night_lighting = lerp(LdotR,twilite_min,twilite_max,0.0,1.0); // twilite band
		// was: getNormal(nscale*day_lighting) with day_lighting -> 0.5 in
		// daylight, which halved the cylinder (side) normal exactly when the
		// sun is above the horizon, so at low sun angles the result was
		// dominated by dot(light,up) ~ sin(elevation) -> dark trunks.
 		vec3 normal=normalize(getNormal(nscale));
 		float LdotN= dot(light,normal);// includes fake normal and bumpmap
		if(MODE==LEAF && LdotN<0.0)
			LdotN=-LdotN*LEAF_XMIT;   // backlit leaf: light passes through
		LdotN=(LdotN+WRAP)/(1.0+WRAP); // wrap lighting
 		float intensity = 1.0/gl_LightSource[i].constantAttenuation/NLIGHTS;
		float lpn       = LdotN*intensity*night_lighting;
		lpn=clamp(lpn,0,5.0);
		diffuse        += Diffuse.rgb*gl_LightSource[i].diffuse.rgb*lpn;
	}
	// ambient now modulates the texture (was added as a flat color, which is
	// why unlit bark rendered as a uniform near-black silhouette)
	vec3 TotalDiffuse = BaseColor*(diffuse*Diffuse.a+Ambient.rgb*Ambient.a);
	return TotalDiffuse;
}

//-------------------------------------------------------------

// ########## main section #########################
void main(void) {
	//vec4 color = vec4(pow(DEPTH,15.0),0,0,1);//Color;
	vec4 color = Color;
	int texid=TEXID;
	int copt=TexVars.y+0.1;
	int aopt=copt&4;
	copt=copt&3;
#if NTEXS >0
 	if(texid>=0){
		vec2 l_uv=sprite(gl_TexCoord[0].xy);
  		vec4 tcolor=texture2D(samplers2d[texid],l_uv);
  		if(copt>0){
  			if(copt>1){ // color has alpha: it modulates texture vs color directly
   				color.rgb=mix(tcolor.rgb,color.rgb,color.a);
  			}
			else{ // no alpha in color: color shows through where texture is clear
				color.rgb=mix(color.rgb,tcolor.rgb,tcolor.a);
				//color.rgb=ncolor.rgb;
			}
  		    //if(aopt) // leaf
  		       color.a=tcolor.a;
		}
		else // texture only
			color=tcolor;
		if(MODE != LEAF)
		   color.a=1;	
	}
	else
		color.a=1;
#endif
	//if(!aopt)
	//	color.a=1;
	if(lighting)
    	color.rgb=setLighting(color.rgb);
#ifdef SHADOWS
     float shadow=1.0-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
     color.rgb=mix(color.rgb,Shadow.rgb,shadow*Shadow.a); 
#endif  
 
	// Haze is handled once, downstream, by the deferred effects.frag pass
	// (same as rocks/terrain) - it used to also be applied here per-fragment,
	// which both double-applied it and (via color.a=pow(color.a,p)) corrupted
	// this fragment's blended alpha.
 	gl_FragData[0]=color;
  	float vfog=DENSITY*lerp(HT,fog_vmin,fog_vmax,1,0);
  	gl_FragData[1]=vec4(4,DEPTH,0,vfog);
  	//gl_FragData[1]=vec4(0.1,DEPTH,0,1);
 	//gl_FragData[1]=vec4(fcolor2); // pass thru

}
// ########## end plants.frag #########################