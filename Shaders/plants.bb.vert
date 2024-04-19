#include "attributes.h"
#include "attributes.vert"

varying vec4 Normal;
varying vec4 Color;
varying vec4 pnorm;
varying vec4 TexVars;

void main(void) {
	vec4 vertex=vec4(gl_Vertex.xyz,1.0);//-CommonAttributes2; //-TheScene->vpoint
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;
	
	float topx=CommonAttributes1.r;
	float topy=CommonAttributes1.g;
	float botx=CommonAttributes1.b;
	float boty=CommonAttributes1.a;
	
	float nscale=TextureAttributes.w;
	float dx2=nscale*topx;
	float dx1=nscale*botx;
	
	TexVars=TextureAttributes;
		
	Normal.xyz = gl_NormalMatrix * gl_Normal;
	
	vec3 norm;

#ifdef DRAW_TRIANGLES
	
	if(gl_Vertex.w<0.5){ // 0 lines
		gl_Position = vec4(ps.x,ps.y,ps.z,1);
	}
	else if(gl_Vertex.w<1.5){ // 1
	    norm=vec3(dx2,0,1);
	    gl_TexCoord[0].xy=vec2(0,0);
    	gl_Position = vec4(ps.x-topx,ps.y-topy,ps.z,1); // top-left  
    }     	  
    else if(gl_Vertex.w<2.5){  // 2
     	norm=vec3(-dx2,0,1);
     	gl_TexCoord[0].xy=vec2(0,1);	
    	gl_Position = vec4(ps.x+topx,ps.y+topy,ps.z,1); // top-right  
    } 	  
    else if(gl_Vertex.w<3.5){  // 3
        norm=vec3(-dx1,0,1);
        gl_TexCoord[0].xy=vec2(1,1);
    	gl_Position = vec4(ps.x+botx,ps.y+boty,ps.z,1);  // bot-right  
    } 	
    else  if(gl_Vertex.w<4.5) {  //4
    	norm=vec3(dx1,0,1);
    	gl_TexCoord[0].xy=vec2(0,1);
    	gl_Position = vec4(ps.x-botx,ps.y-boty,ps.z,1);  // bot-left 
    }
    norm=normalize(norm);
    pnorm.xyz=gl_NormalMatrix * norm;
#else
	gl_Position = vec4(ps.x,ps.y,ps.z,1); // lines only
#endif
	Color=gl_Color;

}

