// notes:
// 1. second shader in pipeline
// 2. input from vertex shader
// 3. output to fragment shader
// 

#include "utils.h"
#ifdef COLOR
varying in vec4 Color_G[];
varying out vec4 Color;
#endif

varying in vec4 Vertex1_G[];
varying out vec4 Vertex1;

#define PI		3.14159265359

uniform vec3 pv;
uniform float freqmip;

vec4 gv=vec4(0);
float g=0;

#if NVALS >0
#include "noise_funcs.frag"
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
	SET_ZNOISE(NPZ);
#endif
	EmitVertex();
}


void main(void) {
#ifdef COLOR
	Color = Color_G[0]; // id mapped into color at pivot vertex
#endif
#if TESSLVL >0	
	int numLayers = TESSLVL;
	float dt = 1.0 / float( numLayers );
	float t_top = 1.0;
	for( int it = 0; it < numLayers; it++ ){
		float t_bot = t_top - dt;
		float smax_top = 1.0 - t_top;
		float smax_bot = 1.0 - t_bot;
		int nums = it + 1;
		float ds_top = smax_top / float(nums - 1 );
		float ds_bot = smax_bot / float(nums);
		float s_top = 0.0;
		float s_bot = 0.0;
		int j=0;
		for( int is = 0; is < nums; is++ ,j++)
		{
			ProduceVertex(s_bot, t_bot);
			ProduceVertex(s_top, t_top);
			s_top += ds_top;
			s_bot += ds_bot;
		}
		ProduceVertex(s_bot, t_bot);
		EndPrimitive();
		t_top = t_bot;
		t_bot -= dt;
	}
#else
	ProduceVertex(0);
	ProduceVertex(1);
	ProduceVertex(2);
#endif
}
