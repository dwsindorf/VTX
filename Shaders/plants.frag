

#include "utils.h"

#if NTEXS >0
uniform sampler2D samplers2d[NTEXS];
#endif

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3
#define LINE_MODE   8

uniform vec4 Diffuse;
uniform vec4 Ambient;
uniform vec4 Shadow;
uniform vec4 Haze;
uniform float haze_zfar;
uniform float haze_ampl;
uniform float haze_grad;
uniform float ws1;
uniform float ws2;
uniform float bump_ampl;
uniform float bump_delta;
uniform float norm_scale;
uniform bool lighting;

#define DEPTH   gl_FragCoord.z
#define TEXID   TexVars.b
#define LINE    TexVars.w

varying vec4 Color;
varying vec3 Pnorm;
varying vec3 Normal;
varying vec4 TexVars;

vec3 test;

#define AVE(v) (v.x+v.y+v.z)
vec3 getNormal(){
	vec3 bn=norm_scale*Pnorm;
	vec3 normal=Normal-bn;
	if(LINE)
		return normal;
	
#ifdef BUMPS
	int texid=TEXID;
	float delta=bump_delta;
	vec2 l_uv=gl_TexCoord[0].xy;
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
	test=normalize(tc);
#endif
	return normal;
}
vec3 setLighting(vec3 BaseColor) {
	vec3 diffuse = vec3(0, 0, 0);
    vec3 normal=getNormal();
	for(int i=0;i<NLIGHTS;i++){
		vec3 light= normalize(gl_LightSource[i].position.xyz);
		float LdotN= dot(light,normal);// for day side diffuse lighting
		float f=max(LdotN,0.0);	
		float amplitude = 1.0/gl_LightSource[i].constantAttenuation;
		float lpn       = f*amplitude;
		diffuse        += Diffuse.rgb*gl_LightSource[i].diffuse.rgb*lpn;
	}
	vec3 TotalDiffuse = BaseColor*diffuse*Diffuse.a+Ambient*Ambient.a;
	return TotalDiffuse;
}

//-------------------------------------------------------------

// ########## main section #########################
void main(void) {
	vec4 color = Color;
#if NTEXS >0
	int texid=TEXID;
	int copt=TexVars.g;
	
	if(texid>=0){
		vec2 l_uv=gl_TexCoord[0].xy;
  		vec4 tcolor=texture2D(samplers2d[texid],l_uv);
  		if(copt>0){    // new color = blend color and texture where texture.a >0
  			vec3 ncolor=mix(color.rgb,tcolor.rgb,tcolor.a); 
  			if(copt>1) // new color = blend color and ncolor where color.a >0
				color.rgb=mix(ncolor.rgb,color.rgb,color.a); 
			else // no alpha in color
				color.rgb=ncolor.rgb;				
		}
		else // texture only
			color.rgb=tcolor.rgb;		
	}
#endif
	color.a=1;
	if(lighting)
    	color.rgb=setLighting(color.rgb);
#ifdef SHADOWS
     float shadow=1.0-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
     color.rgb=mix(color.rgb,color.rgb,Shadow.rgb,shadow*Shadow.a);
#endif  
 
#ifdef HAZE
	float z=DEPTH; // depth buffer
	float depth=1.0/(ws2*z+ws1); // distance
	float d=haze_grad==0.0?1e-4:lerp(depth,0.0,haze_grad*haze_zfar,0.0,1.0); // same as in effects.frag
	float h=haze_ampl*Haze.a*pow(d,8.0*haze_grad); // same as in effects.frag
	color.rgb=mix(color.rgb,Haze.rgb,h);
#endif 
//color=vec4(test.x,test.y,test.z,1);
 	gl_FragData[0]=color;
	gl_FragData[1]=vec4(0,DEPTH,0,color.a); // set type to 0 to bypass second haze correction in effects.frag

}
// ########## end plants.frag #########################
