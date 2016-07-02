varying vec4 Color;

// ----------- program entry -----------------
void main(void) {
	//gl_FragColor=Color;
	vec4 color=Color;
	color.a=1.0;
	gl_FragData[0] = color;
}

