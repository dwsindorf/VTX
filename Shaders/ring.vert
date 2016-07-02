#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

varying vec4 Color;
uniform vec3 object;
varying vec4 Normal;
varying vec4 EyeDirection;
//uniform vec4 Emission;

void main(void) {

	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_Position = ftransform();

	Normal.xyz = gl_NormalMatrix * gl_Normal;
	Normal.w = TextureAttributes.x;     // tangent packed into unused 4th component of Normal
	EyeDirection.w=TextureAttributes.y; // and EyeDirection vectors

	//Color=Emission;

#include "set_common.vert"
#include "set_tex.vert"
#include "set_attributes.vert"

}
