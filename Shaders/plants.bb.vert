#include "attributes.h"
#include "attributes.vert"

varying vec4 EyeDirection;
varying vec4 Normal;
varying vec4 Position;
attribute vec4 vpoint;
varying vec4 Color;

//attribute vec4 CommonAttributes2;

varying vec4 PlantVars;
varying vec4 Constants1;

void main(void) {
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);//-CommonAttributes2; //-TheScene->vpoint
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;
	EyeDirection=-(gl_ModelViewMatrix * vertex); // do view rotation
	
	float offset=CommonAttributes1.r;  // screen space
	float taper=CommonAttributes1.g;  // screen space
	Normal.xyz = gl_NormalMatrix * gl_Normal;
	
	if(gl_Vertex.w<0.5){
    	gl_Position = vec4(ps.x-taper*offset,ps.y,ps.z,1); // top-left
    }
    else if(gl_Vertex.w<1.5){
    	gl_Position = vec4(ps.x+taper*offset,ps.y,ps.z,1); // top-right
    }
    else if(gl_Vertex.w<2.5){
    	gl_Position = vec4(ps.x+offset,ps.y,ps.z,1);  // bot-right
    }
    else{
    	gl_Position = vec4(ps.x-offset,ps.y,ps.z,1);
    }
	//Position = EyeDirection;

	PlantVars=TextureAttributes;
	Constants1=CommonAttributes1;
	Color=gl_Color;

}

