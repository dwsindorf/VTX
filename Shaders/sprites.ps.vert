//#version 450

#include "attributes.h"
#include "attributes.vert"

varying vec4 EyeDirection;
varying vec4 Normal;

varying vec4 index;

void main(void) {
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_Position = ftransform();
	
	Normal.xyz = gl_NormalMatrix * gl_Normal;
	
	float delta=length(EyeDirection-gl_Position);

	gl_PointSize = TextureAttributes.z;
	index=TextureAttributes;

}

