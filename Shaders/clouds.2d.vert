#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

varying vec4 EyeDirection;
varying vec4 Normal;

void main(void) {
	gl_Position = ftransform();
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	Normal.xyz = gl_NormalMatrix * gl_Normal;

#include "set_common.vert"
#include "set_tex.vert"
#include "set_attributes.vert"

}
