// notes:
// 1. second shader in pipeline
// 2. input from vertex shader
// 3. output to fragment shader
// 

#include "utils.h"

varying in vec4 Vertex1_G[];
varying out vec4 Vertex1;

#define PI		3.14159265359

uniform vec3 pv;
uniform float freqmip;

uniform float rscale;


varying vec4 Normal;
vec4 gv=vec4(0);
float g=0;

#if NVALS >0
#include "noise_funcs.h"
#endif
#ifdef TESSLVL
#include "geometry_funcs.h"
#endif

//######################################################

#if TESSLVL >0
void ProduceVertex(float s, float t){
#else
void ProduceVertex(int i){
#endif
#if TESSLVL >0	
	Vertex1=s*(Vertex1_G[2]-Vertex1_G[0]) + t*(Vertex1_G[1]-Vertex1_G[0])+Vertex1_G[0];
	vec4 p=s*(gl_PositionIn[2]-gl_PositionIn[0]) + t*(gl_PositionIn[1]-gl_PositionIn[0])+gl_PositionIn[0];
#else
	Vertex1=Vertex1_G[i];
	vec4 p=gl_PositionIn[i];
#endif	
    // displace using noise function
#ifdef NPZ
	CALC_ZNOISE(NPZ);
	gl_Position=gl_ModelViewProjectionMatrix * p;
#endif
	EmitVertex();
}

void main(void) {
#if TESSLVL >0	
	PRODUCE_VERTICES;
#else
	ProduceVertex(0);
	ProduceVertex(1);
	ProduceVertex(2);
#endif
}
