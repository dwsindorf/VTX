// notes:
// 1. second shader in pipeline
// 2. input from vertex shader
// 3. output to fragment shader
// 

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

varying in vec4 Normal_G[];
varying in vec4 Constants_G[];
varying in vec4 Color_G[];
#if NVALS >0
varying in vec4 Vertex1_G[];
varying out vec4 Vertex1;
#endif
#if NTEXS >0
varying in vec4 Tangent_G[];
varying out vec4 Tangent;
#endif

#define HT Constants1.x

//########## 3D noise section #########################
uniform float freqmip=0;
uniform float bump_delta;
uniform float bump_ampl;
uniform bool lighting;

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
varying out vec4 Constants1;

uniform vec3 center;
uniform vec3 pv;

varying out vec4 attributes[2];

void ProduceVertex(float s, float t){
	g=0;
#if NVALS >0
	Vertex1=s*(Vertex1_G[2]-Vertex1_G[0]) + t*(Vertex1_G[1]-Vertex1_G[0])+Vertex1_G[0];
#endif
	// ---- Alternative calculation from gl_PositionIn ------------
	//Vertex1.xyz=normalize(p.xyz+pv); 
	//Vertex1.xyz=Vertex1.xyz*0.5+0.5;
	Normal=s*(Normal_G[2]-Normal_G[0]) + t*(Normal_G[1]-Normal_G[0])+Normal_G[0];  // ave normal
	vec4 p=s*(gl_PositionIn[2]-gl_PositionIn[0]) + t*(gl_PositionIn[1]-gl_PositionIn[0])+gl_PositionIn[0];
#ifdef NPZ
    CALC_ZNOISE(NPZ);
    CALC_ZNORMAL(NPZ);
    gl_Position=gl_ModelViewProjectionMatrix * p;
#endif
	EyeDirection=-(gl_ModelViewMatrix * p); // do view rotation
	
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
	Constants1=s*(Constants_G[2]-Constants_G[0]) + t*(Constants_G[1]-Constants_G[0])+Constants_G[0];
	HT+=g; // water not included
	for(int i=0;i<2;i++)
		attributes[i]=s*(Attributes_G[2][i]-Attributes_G[0][i]) + t*(Attributes_G[1][i]-Attributes_G[0][i])+Attributes_G[0][i];
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
