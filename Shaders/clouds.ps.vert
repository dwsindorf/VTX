
#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

varying vec4 EyeDirection;
varying vec4 Normal;

uniform float INVROWS;
uniform float ROWS;

varying vec4 CloudVars;

void main(void) {
	gl_Position = ftransform();
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	Normal.xyz = gl_NormalMatrix * gl_Normal;

#ifdef COLOR
	Color=gl_Color;
#endif

	Constants1=CommonAttributes1;

	gl_PointSize = TextureAttributes.z;
	float angle=TextureAttributes.y;
	float cc = cos(angle);
	float ss = sin(angle);
	CloudVars.x=cc;
	CloudVars.y=ss;

	CloudVars.z=TextureAttributes.x;
	CloudVars.w=TextureAttributes.w;

}

