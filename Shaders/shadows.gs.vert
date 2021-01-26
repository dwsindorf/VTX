
// notes:
// 1. first shader in pipeline
// 2. output to geometry shader

attribute vec4 Position1;

varying vec4 Vertex1_G;

void main(void) {
	gl_Position=gl_Vertex;
	Vertex1_G=Position1;
}
