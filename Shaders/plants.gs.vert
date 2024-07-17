
attribute vec4 CommonAttributes1;
attribute vec4 CommonAttributes2;
attribute vec4 TextureAttributes;

varying vec4 Normal_G;
varying vec4 Color_G;
varying vec4 Constants1;
varying vec4 P0;
varying vec4 TexVars_G;

void main(void) {
	Normal_G.xyz = gl_NormalMatrix * gl_Normal;
	Color_G=gl_Color;
	Constants1=CommonAttributes1;
	TexVars_G=TextureAttributes;
    gl_Position=gl_Vertex;
    P0=CommonAttributes2;
 	
}

