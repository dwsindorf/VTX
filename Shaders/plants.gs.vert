
attribute vec4 CommonAttributes1;
attribute vec4 TextureAttributes;

varying vec4 Normal_G;
varying vec4 Color_G;
varying vec4 Factors;
varying vec4 TexVars_G;

void main(void) {
	
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;

	gl_Position = vec4(ps,1);
	Normal_G.xyz = gl_NormalMatrix * gl_Normal;

	Color_G=gl_Color;
	Factors=CommonAttributes1;	
	TexVars_G=TextureAttributes;
	
}

