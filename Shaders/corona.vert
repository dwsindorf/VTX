
#include "attributes.vert"
#include "common.h"

varying vec3 VertexPosition;

void main(void) {
	VertexPosition=-(gl_ModelViewMatrix * gl_Vertex).xyz;
	gl_Position = ftransform();
	
	Constants1=CommonAttributes1;
#include "set_common.vert"
}

