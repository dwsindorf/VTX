#version 430 compatibility 
#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable
// notes:
// 1. second shader in pipeline
// 2. input from vertex shader
// 3. output to fragment shader
// 

#include "utils.h"

varying in vec4 Color_G[];
varying out vec4 Color;
varying out vec4 Data;

uniform mat4 modelViewProjectionMat;

varying in vec4 Vertex1_G[];
vec4 Vertex1;

uniform vec3 pv;
uniform float freqmip;
uniform float MinHt;
uniform float MaxHt;
uniform float rscale;

vec4 gv=vec4(0);
float g=0;

#if NVALS >0
#include "noise_funcs.h"
#endif
#ifdef TESSLVL
#include "geometry_funcs.h"
#endif

//######################################################

const int DSIZE=(TESSLVL+1)*(TESSLVL+2)/2;
vec4 vdata[DSIZE];
 
void ProduceVertex(int i){
	gl_Position=vdata[i];
	EmitVertex();
}

void main(void) {
#ifdef COLOR
	Color = Color_G[0]; // id mapped into color at pivot vertex
#endif
    int index=0;
	float t=1;
	float del=1.0/TESSLVL;
	for(int it = 0; it <= TESSLVL; it++,t-=del){
		int nums = it+1 ;
		float s=0;
		for(int is = 0; is < nums; is++,s+=del) {
		    Vertex1=s*(Vertex1_G[2]-Vertex1_G[0]) + t*(Vertex1_G[1]-Vertex1_G[0])+Vertex1_G[0];
			vec4 p=s*(gl_PositionIn[2]-gl_PositionIn[0]) + t*(gl_PositionIn[1]-gl_PositionIn[0])+gl_PositionIn[0];
			CALC_ZNOISE(NPZ);
			vdata[index]=gl_ModelViewProjectionMatrix * p;
			index++;
		}
	}
	Color.a=g;
    PRODUCE_INDEXED_VERTICES;
}
