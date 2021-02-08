

varying vec4 Color;
//varying vec4 Data;

// ########## main section #########################
void main(void) {
	gl_FragData[0] = Color;
	//gl_FragData[1] = Data;
}
