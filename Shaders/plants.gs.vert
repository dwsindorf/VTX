attribute vec4 CommonAttributes1;
attribute vec4 CommonAttributes2;
attribute vec4 CommonAttributes3;
attribute vec4 TextureAttributes;
attribute vec4 FogAttributes;

varying vec4 Normal_G;
varying vec4 Color_G;
varying vec4 Constants1;
varying vec4 ImageVars_G;
varying vec4 P0;
varying vec4 TexVars_G;
varying vec4 EyeDirection;
varying vec4 FogVars_G;  // x=HT @ p1(this vertex), y=HT @ p2, z=density (see Plants.h/.cpp)

void main(void) {
	Normal_G.xyz = gl_NormalMatrix * gl_Normal;
	Color_G=gl_Color;
	Constants1=CommonAttributes1;
	TexVars_G=TextureAttributes;
    gl_Position=gl_Vertex;
    P0=CommonAttributes2;
    ImageVars_G=CommonAttributes3;
    FogVars_G=FogAttributes;

    EyeDirection=-(gl_ModelViewMatrix * gl_Vertex);

}