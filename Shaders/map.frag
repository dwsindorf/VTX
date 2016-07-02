#version 120
#ifdef COLOR
varying vec4 Color;
#endif
// ########## main section #########################
void main(void) {
#ifdef COLOR
	gl_FragData[0] = Color;
#endif
}
