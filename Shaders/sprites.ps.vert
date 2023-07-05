#include "attributes.h"
#include "attributes.vert"

varying vec4 EyeDirection;
varying vec4 Normal;
varying vec4 Position;

uniform float INVROWS;
uniform float ROWS;

varying vec4 SpriteVars;
varying vec4 Constants1;

void main(void) {
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_Position = ftransform();
	
	Normal.xyz = gl_NormalMatrix * gl_Normal;
	
	Position = EyeDirection;

	gl_PointSize = TextureAttributes.z;
	SpriteVars=TextureAttributes;
	Constants1=CommonAttributes1;

}

