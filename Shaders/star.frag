#include "common.h"
#include "utils.h"

uniform vec3 center;
uniform vec3 object;

uniform bool fbo_read;
uniform bool fbo_write;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;

varying vec4 EyeDirection;
varying vec4 Normal;


//########## 3D noise section #########################

#ifndef NVALS
#define NVALS 0
#endif

#if NVALS >0
#include "noise_funcs.h"
#endif

// ########## Texture section #########################

#if NTEXS >0
#include "tex_funcs.frag"
#endif

// ########## main section #########################
void main(void) {

#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif

#if NTEXS >0
#include "set_tex.frag"
#endif
	if (fbo_write){
		//vec4 data2=vec4(1.0);
		//gl_FragData[1]=data2;
		gl_FragData[2]=vec4(0,0,0,1);
	}
	color=clamp(color,0.0,1.0);
	gl_FragData[0] = color;
	gl_FragData[1]=vec4(0,1,0.0,0.0);
	//color.a=1.0;
}
