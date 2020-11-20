//#version 120
#include "attributes.h"
#include "textures.h"
#include "attributes.vert"

varying vec4 EyeDirection_G;
varying vec4 Normal_G;
varying vec4 Constants_G;
varying vec4 Factors;
varying vec4 Color_G;

// converts input vertices from model space to screen space
// and passes them to the geometry shader which creates a
// screen space rectangle (billboard)

void main(void) {
	//gl_Position = gl_Vertex;
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);
	EyeDirection_G=-(gl_ModelViewMatrix * vertex);

	// convert to screen space
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;

	gl_Position = vec4(ps,1);
	Normal_G.xyz = gl_NormalMatrix * gl_Normal;

#ifdef COLOR
	Color_G=gl_Color;
#endif

	Constants_G=CommonAttributes;

	Factors=TextureAttributes;

}

