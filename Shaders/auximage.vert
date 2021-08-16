
attribute vec4 CommonAttributes1;

varying vec4 Constants1;

varying vec4 EyeDirection;
varying vec4 Normal;
void main(void) {
	gl_Position = ftransform();
	Constants1=CommonAttributes1;
	
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex);
	Normal.xyz = gl_NormalMatrix * gl_Normal;
}

