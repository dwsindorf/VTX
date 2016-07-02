#include "attributes.h"
#include "attributes.vert"
#include "textures.h"
#include "common.h"

varying vec4 EyeDirection;
varying vec4 Normal;

#if PASS == 1
uniform float zfar;
#endif

void main(void) {
	// note: original vertex coords are in eye space (0,0,0=eye) but not rotated for view

	// vec4 Position1=gl_Vertex-vec4(object,0); // model xyz fixed
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_Position = ftransform();
#if PASS == 1
	gl_Position.z = gl_Position.z * gl_Position.w / zfar;
#endif
	//EyeDirection.w=Attributes1.y; // and EyeDirection vectors

	Normal.xyz = gl_NormalMatrix * gl_Normal;

#include "set_common.vert"
#include "set_tex.vert"
#include "set_attributes.vert"

}
