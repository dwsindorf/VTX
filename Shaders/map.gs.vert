//#version 120
// notes:
// 1. first shader in pipeline
// 2. output to geometry shader

attribute vec4 Position2;
attribute vec4 Position1;

varying vec4 Vertex1_G;
varying vec4 Vertex2_G;

#ifdef COLOR
varying vec4 Color_G;
#endif
void main(void) {
	gl_Position=gl_Vertex;
#ifdef COLOR
	Color_G=gl_Color;
#endif
	Vertex1_G=Position1;
	Vertex2_G=Position2;
}
