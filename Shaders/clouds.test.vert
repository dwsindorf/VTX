varying vec4 EyeDirection;
varying vec4 Normal;
varying vec4 Color;

void main(void) {
	gl_Position = ftransform();
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	Normal.xyz = gl_NormalMatrix * gl_Normal;

	Color=gl_Color;
}

