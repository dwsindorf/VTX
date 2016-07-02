varying vec4 Color;

void main(void) {
	gl_Position = ftransform();	
	
	Color=gl_Color;
}

