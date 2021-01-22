// ########## begin clouds.frag #########################
#include "common.h"
#include "utils.h"

uniform bool fbo_read;
uniform bool fbo_write;
uniform sampler2DRect FBOTex1;

uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;

uniform float zfar;
uniform float znear;
uniform float zmax;

uniform bool test1;
uniform bool test2;
uniform bool top;


//########## 3D noise section #########################

#if NVALS >0
#include "noise_funcs.frag"
#endif

// ########## Texture section #########################
//
#if NTEXS >0
#include "tex_funcs.frag"
#endif

//-------------------------------------------------------------

// ########## main section #########################
void main(void) {
 //   vec3 normal=normalize(Normal.xyz);
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif
#if PASS == 0

#ifdef NCC
	color.a=0;
	NOISE_COLOR(NCC);
#endif
	gl_FragData[0]=color;

#elif PASS == 1
#define FRONT (gl_FrontFacing)
 	if(top){
		if(HT<0)
			discard;
	}
	else{
		if(HT>0)
			discard;
		//normal=-normal;
	}

 	float depth=DEPTH;
 	//float depth=gl_FragCoord.z;

	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // previous fragment color
	//vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // previous fragment depth (.r)
	float sat=0.0;
	float smax=zmax;
	float density=0;
	if(FRONT){
		float dmax=texture2DRect(FBOTex2, gl_FragCoord.xy).r;
		float dd=dmax-depth;
		dd=clamp(dd,0.0,1.0);
#ifdef SCATTER
		v1= Vertex1.xyz;
	    density=SCATTER;
	    smax=density*1.0e-5;
	    density=clamp(density,0.0,1.0);
#endif
#ifdef NCC
#define DENSITY density
		color=NCC;
#endif
		float ds=dd*(zfar-znear);
		sat=lerp(ds,0,smax,0,1);

		sat=smoothstep(0.6,0.7,color.a*sat);
		
#ifdef BLEND		
		gl_FragData[0].rgb=color.rgb;
		gl_FragData[0].a=sat;
#else
		gl_FragData[0].rgb=mix(fcolor1.rgb,color.rgb,sat);
		gl_FragData[0].a=sat+fcolor1.a;
#endif
		gl_FragData[1].r=depth;
	}
	else{
		gl_FragData[0]=fcolor1;
		gl_FragData[1].r=depth;
#ifdef BLEND
		gl_FragData[0].a=0;
		gl_FragData[1].a=1.0;
#endif
	}
#elif PASS==2
    vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy);
  	gl_FragData[0]=fcolor1;

#endif
}
// ########## end clouds.frag #########################
