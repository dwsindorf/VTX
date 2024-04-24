#include "attributes.h"
#include "attributes.vert"

varying vec4 Color;
varying vec4 TexVars;
varying vec3 Normal;
varying vec3 Pnorm;

void main(void) {
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;
	
	float topx=CommonAttributes1.r;
	float topy=CommonAttributes1.g;
	float botx=CommonAttributes1.b;
	float boty=CommonAttributes1.a;
	
	float dx2=topx;
	float dx1=botx;
	
	TexVars=TextureAttributes;
	Normal=gl_NormalMatrix * gl_Normal;
	Color=gl_Color;

	if(gl_Vertex.w<0.5){ // 0 lines
		Pnorm.xyz = gl_NormalMatrix * vec3(dx1,0,0);
		gl_Position = vec4(ps.x,ps.y,ps.z,1);
	}
	else if(gl_Vertex.w<1.5){ // 1
	    Pnorm.xyz = gl_NormalMatrix * vec3(dx2,0,0);
	    gl_TexCoord[0].xy=vec2(0,0);
    	gl_Position = vec4(ps.x-topx,ps.y-topy,ps.z,1); // top-left  
    }     	  
    else if(gl_Vertex.w<2.5){  // 2
     	Pnorm.xyz = gl_NormalMatrix * vec3(-dx2,0,0);
     	gl_TexCoord[0].xy=vec2(0,1);	
    	gl_Position = vec4(ps.x+topx,ps.y+topy,ps.z,1); // top-right  
    } 	  
    else if(gl_Vertex.w<3.5){  // 3
        Pnorm.xyz = gl_NormalMatrix * vec3(-dx1,0,0);
        gl_TexCoord[0].xy=vec2(1,1);
    	gl_Position = vec4(ps.x+botx,ps.y+boty,ps.z,1);  // bot-right  
    } 	
    else  if(gl_Vertex.w<4.5) {  //4
    	Pnorm.xyz = gl_NormalMatrix * vec3(dx1,0,0);
    	gl_TexCoord[0].xy=vec2(0,1);
    	gl_Position = vec4(ps.x-botx,ps.y-boty,ps.z,1);  // bot-left 
    }
}

