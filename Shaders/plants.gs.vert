
attribute vec4 CommonAttributes1;
attribute vec4 TextureAttributes;

varying vec4 EyeDirection_G;
varying vec4 Norm_G;
varying vec4 Color_G;
varying vec4 Factors;
varying vec4 TexVars;

void main(void) {
	
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);
	EyeDirection_G=-(gl_ModelViewMatrix * vertex);
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;

	gl_Position = vec4(ps,1);
	Norm_G.xyz = gl_NormalMatrix * gl_Normal.xyz;

	Color_G=gl_Color;
	Factors=CommonAttributes1;	
	TexVars=TextureAttributes;
	
}

