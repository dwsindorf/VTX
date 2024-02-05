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

#define XY_OFFSETS
void main(void) {
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);//-CommonAttributes2; //-TheScene->vpoint
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;
	EyeDirection=-(gl_ModelViewMatrix * vertex); // do view rotation
	
	//float xoffset=offset*CommonAttributes1.b;
	//float yoffset=offset*CommonAttributes1.w;
	//float taper=CommonAttributes1.g;  // screen space
	
	float topx=CommonAttributes1.r;
	float topy=CommonAttributes1.g;
	float botx=CommonAttributes1.b;
	float boty=CommonAttributes1.a;
	
	float dx=topx-botx;
	//dx=dx*dx*dx;
	float dy=topy-boty;
	
	Normal.xyz = gl_NormalMatrix * gl_Normal;
	
	vec3 norm;

#ifdef DRAW_TRIANGLES
	gl_Position = vec4(ps.x,ps.y,ps.z,1);

	if(gl_Vertex.w<0.5){
	 norm=vec3(-dx,topy,ps.z);
    	gl_Position = vec4(ps.x,ps.y,ps.z,1); // top-left
    }
	else if(gl_Vertex.w<1.5){
	    norm=vec3(-dx,-dy,ps.z);
    	gl_Position = vec4(ps.x-topx,ps.y-topy,ps.z,1); // top-left  
    }     	  
    else if(gl_Vertex.w<2.5){
     	norm=vec3(dx,dy,ps.z);
    	gl_Position = vec4(ps.x+topx,ps.y+topy,ps.z,1); // top-right  
    } 	  
    else if(gl_Vertex.w<3.5){
        norm=vec3(dx,dy,ps.z);
    	gl_Position = vec4(ps.x+botx,ps.y+boty,ps.z,1);  // bot-right  
    } 	
    else{
    	norm=vec3(-dx,-dy,ps.z);
    	gl_Position = vec4(ps.x-botx,ps.y-boty,ps.z,1);  // bot-left 
    }
    norm=normalize(norm);
    Normal.xyz = gl_NormalMatrix * norm;
#else
	gl_Position = vec4(ps.x,ps.y,ps.z,1); // lines only
#endif
	//Position = EyeDirection;

	PlantVars=TextureAttributes;
	Constants1=CommonAttributes1;
	Color=gl_Color;

}

