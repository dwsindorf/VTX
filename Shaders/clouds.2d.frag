

#include "clouds.lighting.frag"
#define _BUMPS_

#if NVALS >0
#include "noise_funcs.h"
#endif

// ########## Texture section #########################

#if NTEXS >0
#include "tex_funcs.frag"
#endif

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;

// ########## main section #########################
void main(void) {
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif
    vec4 background=texture2DRect(FBOTex1, gl_FragCoord.xy); // FBO image (background)
	bump=vec3(0.0);
    vec3 normal=normalize(Normal.xyz);
#ifdef NCC
	NOISE_COLOR(NCC);
#endif

#if NTEXS >0
#include "set_tex.frag"
#endif

     if(lighting)
    	color.rgb=setLighting(color.rgb,normal-bump);

     color=clamp(color,0.0,1.0);
	 gl_FragData[0]=color;
//#ifdef BACK
	 vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // Params
	 //gl_FragData[1]=vec4(0,0,0,1);
	 gl_FragData[1]=fcolor2;
	 
//#else
//	 gl_FragData[1]=vec4(0,1,0,1);
//#endif
 //    gl_FragData[2]=vec4(0,0,0,1);
}
// ########## end clouds.frag #########################
