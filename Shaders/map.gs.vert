
// notes:
// 1. first shader in pipeline
// 2. output to geometry shader

attribute vec4 Position1;

varying vec4 Vertex1_G;

#ifdef COLOR
varying vec4 Color_G;
#endif
void main(void) {
	gl_Position=gl_Vertex;
#ifdef COLOR
	Color_G=gl_Color;
#endif
	Vertex1_G=Position1;
}
