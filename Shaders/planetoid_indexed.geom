#version 430 compatibility 
#extension GL_EXT_geometry_shader4 : enable

// notes:
// 1. second shader in pipeline
// 2. input from vertex shader
// 3. output to fragment shader
// 

#include "utils.h"
#include "common.h"

#define HT Constants1.x

#define GEOM

varying in vec4 Normal_G[];
varying in vec4 Constants_G[];
varying in vec4 Color_G[];
#if NVALS >0
varying in vec4 Vertex1_G[];
#endif
#if NTEXS >0
varying in vec4 Tangent_G[];
#endif

//########## 3D noise section #########################

uniform bool lighting;
uniform float rscale;
uniform float nscale;

float delta=0;

#define PI		3.14159265359
#define RT2		1.414213562373

#if NVALS >0
#include "noise_funcs.h"
#endif

#ifdef TESSLVL
#include "geometry_funcs.h"
#endif

//######################################################

varying in vec4 Attributes_G[][2];

// forward to fragment shader 
varying out vec4 Color;
varying out vec4 Normal;
varying out vec4 EyeDirection;

uniform vec3 center;

varying out vec4 attributes[2];

const int DSIZE=(TESSLVL+1)*(TESSLVL+2)/2;
vec4 vdata[DSIZE][4];

void ProduceVertex(int i){
    float s=vdata[i][2].x;
    float t=vdata[i][2].y;
	Vertex1=vdata[i][0];
	
	vec4 p=vdata[i][1];
	g=vdata[i][2].z;

	gl_Position=gl_ModelViewProjectionMatrix * p;
    Normal=vdata[i][3];
	EyeDirection=-(gl_ModelViewMatrix * p); // do view rotation
	
	Constants1=s*(Constants_G[2]-Constants_G[0]) + t*(Constants_G[1]-Constants_G[0])+Constants_G[0];	
	HT+=rscale*g;
	
#ifdef COLOR
if(newcell)
    Color=vec4(1,0,0,1);
else
	Color=s*(Color_G[2]-Color_G[0]) + t*(Color_G[1]-Color_G[0])+Color_G[0];
#endif
#if NTEXS >0
	Tangent=s*(Tangent_G[2]-Tangent_G[0])+t*(Tangent_G[1]-Tangent_G[0])+Tangent_G[0];
	for(int i=0;i<NTEXS;i++)	
		gl_TexCoord[i]=s*(gl_TexCoordIn[2][i]-gl_TexCoordIn[0][i])+t*(gl_TexCoordIn[1][i]-gl_TexCoordIn[0][i])+gl_TexCoordIn[0][i];
#endif
	for(int i=0;i<2;i++)
		attributes[i]=s*(Attributes_G[2][i]-Attributes_G[0][i]) + t*(Attributes_G[1][i]-Attributes_G[0][i])+Attributes_G[0][i];
	EmitVertex();
}

// new scheme uses array to fetch data from vertexes used more than once
// - reduces number noise calculations from 24 to 15 for tesslvl=4
// TODO: calculate normals using cross product vs noise lookup
//
void main(void) {
	float t=1;
	float del=1.0/TESSLVL;
	for(int it = 0,index=0; it <= TESSLVL; it++,t-=del){ // row = triangle strip
		int nums = it+1 ;
		float s=0;
		for(int is = 0; is < nums; is++,s+=del) { // triangles in strip
		    Vertex1=s*(Vertex1_G[2]-Vertex1_G[0]) + t*(Vertex1_G[1]-Vertex1_G[0])+Vertex1_G[0];
			vdata[index][0]=Vertex1;		    
			vec4 p=s*(gl_PositionIn[2]-gl_PositionIn[0]) + t*(gl_PositionIn[1]-gl_PositionIn[0])+gl_PositionIn[0];
			CALC_ZNOISE(NPZ);
//#if NLIGHTS >0
			Normal=s*(Normal_G[2]-Normal_G[0]) + t*(Normal_G[1]-Normal_G[0])+Normal_G[0];
			CALC_ZNORMAL(NPZ);
			vdata[index][3]=Normal;
//#endif
			vdata[index][1]=p;
			vdata[index][2].x=s;
		    vdata[index][2].y=t;
		    vdata[index][2].z=g;
			index++;
		}
	}
    PRODUCE_INDEXED_VERTICES;
}
