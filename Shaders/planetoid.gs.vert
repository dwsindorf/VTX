
// notes:
// 1. first shader in pipeline
// 2. output to geom shader

#include "textures.h"
#include "attributes.vert"

// modified from common.h 
#if NVALS >0
varying vec4 Vertex1_G;
#endif
#if NTEXS >0
varying vec4 Tangent_G;
#endif

varying vec4 EyeDirection_G;
varying vec4 Normal_G;
varying vec4 Constants_G;
varying vec4 Color_G;
varying vec4 Attributes_G[2];

void main(void) {
	EyeDirection_G=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_Position=gl_Vertex;
	Normal_G.xyz = gl_NormalMatrix * gl_Normal;

#ifdef COLOR
	Color_G=gl_Color;
#endif

#if NTEXS >0
	Tangent_G=TextureAttributes;
#endif

#if NVALS >0
	Vertex1_G=Position1;
#endif
	Constants_G=CommonAttributes;
	
	Attributes_G[0]=Attributes3;
	Attributes_G[1]=Attributes4;

#include "set_tex.vert"

}
