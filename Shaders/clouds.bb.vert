
#include "attributes.h"
#include "attributes.vert"
#include "common.h"

varying vec4 EyeDirection;
varying vec4 Normal;

uniform float INVROWS;
uniform float ROWS;

void main(void) {
	//gl_Position = ftransform();

	vec4 vertex=vec4(gl_Vertex.xyz,1.0);

	// convert to screen space
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	vec3 ps = proj.xyz/proj.w;

//	vec4 position=gl_ModelViewProjectionMatrixInverse * vec4(ps,1.0);
//	position /=position.w;
//	EyeDirection=-(gl_ModelViewMatrix * position);

	EyeDirection=-(gl_ModelViewMatrix * vertex); // do view rotation

	Normal.xyz = gl_NormalMatrix * gl_Normal;

#ifdef COLOR
	Color=gl_Color;
#endif
	//vec3 ps=gl_PositionIn[0].xyz;

	float pa=TextureAttributes.z; // random position angle
	float pr=TextureAttributes.w; // position offset from center

	float cosp=pr*cos(pa);
	float sinp=pr*sin(pa);

	ps.xy+=vec2(cosp,sinp);

	float ta=TextureAttributes.x;
	float ts=TextureAttributes.y;

	float d=CommonAttributes1.w;

	float ty=floor(d*INVROWS);
	float tx=floor(d-ROWS*ty);

	float cosa=ts*cos(ta);
	float sina=ts*sin(ta);//1-cosa;

	Constants1=CommonAttributes1;
    if(gl_Vertex.w<0.5){
    	gl_Position = vec4(ps.x+sina,ps.y+cosa,ps.z,1);
    	gl_TexCoord[0].xy=vec2(tx,ty);
    }
    else if(gl_Vertex.w<1.5){
    	gl_Position = vec4(ps.x+cosa,ps.y-sina,ps.z,1);
    	gl_TexCoord[0].xy=vec2(1.0+tx,ty);
    }
    else if(gl_Vertex.w<2.5){
    	gl_Position = vec4(ps.x-sina,ps.y-cosa,ps.z,1);
    	gl_TexCoord[0].xy=vec2(1.0+tx,1.0+ty);
    }
    else{
    	gl_Position = vec4(ps.x-cosa,ps.y+sina,ps.z,1);
    	gl_TexCoord[0].xy=vec2(tx,1.0+ty);
    }
}

