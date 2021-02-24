
attribute vec4 CommonAttributes1;

varying vec4 EyeDirection;
varying vec3 Normal;
varying vec4 Constants1;

void main(void) {
    //gl_Position = gl_ModelViewProjectionMatrix * (gl_Vertex*10000);
	gl_Position = ftransform();
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	gl_TexCoord[0]=gl_MultiTexCoord0;
	Normal.xyz = gl_NormalMatrix * gl_Normal;
	Constants1=CommonAttributes1;
}

