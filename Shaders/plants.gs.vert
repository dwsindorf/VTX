
attribute vec4 CommonAttributes1;

varying vec4 EyeDirection_G;
varying vec4 Normal_G;
varying vec4 Color_G;
varying vec4 Factors;

void main(void) {

	//gl_Position = gl_Vertex;
	
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);
	EyeDirection_G=-(gl_ModelViewMatrix * vertex);
	//EyeDirection_G=-gl_Vertex;
	// convert to screen space
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;

	gl_Position = vec4(ps,1);
	Normal_G.xyz = gl_NormalMatrix * gl_Normal;

	Color_G=gl_Color;
	Factors=CommonAttributes1;
	
}

