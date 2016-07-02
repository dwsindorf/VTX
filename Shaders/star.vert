
#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

varying vec4 EyeDirection;

void main(void) {

	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_Position = ftransform();

	//EyeDirection.w=TextureAttributes.y; // and EyeDirection vectors

#ifdef COLOR
	Color=gl_Color;
#endif

#include "set_common.vert"

#include "set_tex.vert"
#include "set_attributes.vert"

}
