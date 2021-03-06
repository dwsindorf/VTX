#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

varying vec4 EyeDirection;
varying vec4 Normal;

varying vec4 CloudVars;

void main(void) {
	gl_Position = ftransform();
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	Normal.xyz = gl_NormalMatrix * gl_Normal;

	Color=gl_Color;

#if NVALS >0
	Vertex1=Position1;
#endif

#if NTEXS >0
	Tangent=TextureAttributes;
#endif
	Constants1=CommonAttributes1;
#ifdef BB
	gl_TexCoord[0]=gl_MultiTexCoord0;
#endif
#ifdef PS
	gl_PointSize = TextureAttributes.z;
	float angle=TextureAttributes.y;
	float cc = cos(angle);
	float ss = sin(angle);
	CloudVars.x=cc;
	CloudVars.y=ss;

	CloudVars.z=TextureAttributes.x;
	CloudVars.w=TextureAttributes.w;
#endif
}

