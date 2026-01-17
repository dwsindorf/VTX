#ifndef _COMMON_H
#define _COMMON_H
// ########## begin common.h #########################
#ifdef COLOR
varying vec4 Color;
#endif


//#if NVALS >0
varying vec4 Vertex1;
varying vec4 Vertex2;
//#endif
//#if NTEXS >0
varying vec4 Tangent;
//#endif

varying vec4 Constants1;

float bump_ampl=1.0;
float bump_delta=1e-6;
float logf=0.0;
float nbamp=0.0;
vec4 gv=vec4(0.0);
float amplitude = 1.0;
vec3 bump;
float g=0.0;
float b=0.0;
float nd=1.0;
bool newcell=false;
uniform vec3 pv;
vec3 v1;


uniform float texmip;
uniform float bumpmip;
uniform float colormip;
uniform float freqmip;

#define HT Constants1.x
#define FHT Constants1.x

#define DENSITY Constants1.z
//#define THETA Constants1.a
#define SFACT Constants1.a
#define PHI Vertex1.y
#define THETA Vertex1.x


#define PI		3.14159265359
#define RT2		1.414213562373

float RPD=2.0*PI/360.0;
float DPR=360.0/2.0/PI;

//const float MILES = 1.0e-5;
float bmpht=0.0;
#define BMPHT bmpht
#define SLOPE 0.0
float RED=1.0;
float GREEN=1.0;
float BLUE=1.0;
float ALPHA=1.0;

#define LAT abs(PHI)
#define EQU (1-LAT)
#define X Vertex1.x

// ########## end common.h #########################

#endif
