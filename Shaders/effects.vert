
void main(void) {
	gl_Position = vec4(gl_Vertex.xyz,1.0);
	gl_TexCoord[0].xy=gl_MultiTexCoord0.xy;
}

