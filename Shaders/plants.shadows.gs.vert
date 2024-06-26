
attribute vec4 CommonAttributes1;
attribute vec4 CommonAttributes2;
attribute vec4 TextureAttributes;


varying vec4 Constants1;
varying vec4 P0;
varying vec4 TexVars_G;


vec4 project(vec4 pnt){
	vec4 vertex=vec4(pnt.xyz,1.0);
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	return vec4(proj.xyz/proj.w,pnt.w);
}
void main(void) {
	Constants1=CommonAttributes1;
    gl_Position=gl_Vertex;
    P0=CommonAttributes2;
    TexVars_G=TextureAttributes;
    
}

