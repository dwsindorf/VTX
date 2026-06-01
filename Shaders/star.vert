
#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

varying vec4 EyeDirection;
varying vec4 Normal;
varying vec4 WorldNormal;

void main(void) {

	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_Position = ftransform();

	Normal.xyz = gl_NormalMatrix * gl_Normal;
	Normal.w=0;

#ifdef COLOR
	Color=gl_Color;
#endif

#include "set_common.vert"
WorldNormal.xyz = gl_Normal;

#include "set_tex.vert"
#include "set_attributes.vert"

}
