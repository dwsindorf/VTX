#version 120
varying vec4 Color;

// ########## main section #########################
void main(void) {
	gl_FragData[0] = Color;

}
