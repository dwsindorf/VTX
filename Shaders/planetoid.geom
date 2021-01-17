// notes:
// 1. second shader in pipeline
// 2. input from vertex shader
// 3. output to fragment shader
// 

//#version 120
//#extension GL_EXT_geometry_shader4 : enable

#include "utils.h"

float logf=0;
float nbamp=0;
vec4 gv=vec4(0);
float amplitude = 1.0;
vec3 bump;
float g=0;
float b=0;
bool newcell=false;

#define GEOM

//#ifdef TEST4
//#define N3D
//#endif

#define _BUMPS_

varying in vec4 Normal_G[];
varying in vec4 EyeDirection_G[];
varying in vec4 Constants_G[];
varying in vec4 Color_G[];
#if NVALS >0
varying in vec4 Vertex1_G[];
varying in vec4 Vertex2_G[];
varying out vec4 Vertex1;
varying out vec4 Vertex2;
#endif
#if NTEXS >0
varying in vec4 Tangent_G[];
varying out vec4 Tangent;
#endif

#define HT Constants.x

//########## 3D noise section #########################
uniform float freqmip=0;
uniform float bump_delta;
uniform float bump_ampl;
uniform bool lighting;


float delta=0;

#define PI		3.14159265359
#define RT2		1.414213562373

#if NVALS >0
#include "noise_funcs.frag"
#endif
//######################################################

varying in vec4 Attributes_G[][2];

// forward to fragment shader 
varying out vec4 Color;
varying out vec4 Normal;
varying out vec4 EyeDirection;
varying out vec4 Constants;

uniform int  tessLevel;
uniform vec3 center;
uniform vec3 pv;
varying out vec4 attributes[2];

void ProduceVertex(float s, float t){
    float amp=1.0e-6; // Seems about right but need to use ht scale factor
#if NVALS >0
	Vertex1=s*(Vertex1_G[2]-Vertex1_G[0]) + t*(Vertex1_G[1]-Vertex1_G[0])+Vertex1_G[0];
	Vertex2=s*(Vertex2_G[2]-Vertex2_G[0]) + t*(Vertex2_G[1]-Vertex2_G[0])+Vertex2_G[0];
#endif
	vec4 p=s*(gl_PositionIn[2]-gl_PositionIn[0]) + t*(gl_PositionIn[1]-gl_PositionIn[0])+gl_PositionIn[0];
	// ---- Alternative calculation from gl_PositionIn ------------
	//Vertex1.xyz=normalize(p.xyz+pv); 
	//Vertex1.xyz=Vertex1.xyz*0.5+0.5;
	vec3 bump;
	vec4 normal=s*(Normal_G[2]-Normal_G[0]) + t*(Normal_G[1]-Normal_G[0])+Normal_G[0];  // ave normal
	float g1=0;
#ifdef NPZ
	SET_NOISE(NPZ);
	g1=2.0*g; // seems to align scaling better with per vertex ht
	g=0;
#endif
	if (lighting){
		normal.xyz=normalize(normal.xyz-gl_NormalMatrix *df);
	}
	
	Normal.xyz=normalize(normal.xyz);
	vec3 v=p.xyz+pv;  // move from eye to model reference
	v=normalize(v)*g1; // displace along vector from vertex to object center
	p.xyz+=amp*v;
	
	gl_Position=gl_ModelViewProjectionMatrix * p;
	
	//EyeDirection=-(gl_ModelViewMatrix * p);
	EyeDirection=s*(EyeDirection_G[2]-EyeDirection_G[0])+t*(EyeDirection_G[1]-EyeDirection_G[0])+EyeDirection_G[0];
	
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
	Constants=s*(Constants_G[2]-Constants_G[0]) + t*(Constants_G[1]-Constants_G[0])+Constants_G[0];	
#ifdef NPZ
	HT+=g1;
#endif
	for(int i=0;i<2;i++)
		attributes[i]=s*(Attributes_G[2][i]-Attributes_G[0][i]) + t*(Attributes_G[1][i]-Attributes_G[0][i])+Attributes_G[0][i];
	EmitVertex();
}
// #vertexes = (tessLevel+1)*(tessLevel+2)
void main(void) {
	int numLayers = tessLevel;
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
}
