
attribute vec4 CommonAttributes;

varying vec4 Constants;

varying vec4 EyeDirection;
varying vec4 Normal;

void main(void) {
	gl_Position = ftransform();
	Constants=CommonAttributes;

	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex);
	Normal.xyz = gl_NormalMatrix * gl_Normal;
}

