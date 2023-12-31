#include "attributes.h"
#include "attributes.vert"

varying vec4 EyeDirection;
varying vec4 Normal;
varying vec4 Position;

varying vec4 Color;

varying vec4 PlantVars;
varying vec4 Constants1;

void main(void) {
	//EyeDirection=-(gl_ModelViewMatrix * gl_Vertex); // do view rotation
	//gl_Position = ftransform();
	
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;
	EyeDirection=-(gl_ModelViewMatrix * vertex); // do view rotation
	
	float offset=0.0005*CommonAttributes1.r;  // screen space
	Normal.xyz = gl_NormalMatrix * gl_Normal;
	
	if(gl_Vertex.w<0.5){
    	gl_Position = vec4(ps.x-0.5*offset,ps.y,ps.z,1);
    }
    else if(gl_Vertex.w<1.5){
    	gl_Position = vec4(ps.x+0.5*offset,ps.y,ps.z,1);
    }
    else if(gl_Vertex.w<2.5){
    	gl_Position = vec4(ps.x+offset,ps.y,ps.z,1);
    }
    else{
    	gl_Position = vec4(ps.x-offset,ps.y,ps.z,1);
    }
	//Position = EyeDirection;

	PlantVars=TextureAttributes;
	Constants1=CommonAttributes1;
	Color=gl_Color;

}

