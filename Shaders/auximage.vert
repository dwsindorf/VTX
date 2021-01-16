#include "utils.h"
#include "common.h"

#define _BUMPS_
#if NVALS >0
uniform bool lighting;

attribute vec4 Position2;
attribute vec4 Position1;
#include "noise_funcs.frag"
#endif

attribute vec4 CommonAttributes;

varying vec4 EyeDirection;
varying vec4 Normal;
varying vec4 Params;

void main(void) {
	Params=CommonAttributes;

#if NVALS >0
    Vertex1=Position1;
    Vertex2=Position2;
#ifdef NPZ
    vec4 p=gl_Vertex;
    SET_ZNOISE(NPZ);
    Params.r+=g;
	Normal.xyz=normalize(gl_NormalMatrix * gl_Normal - gl_NormalMatrix *df);
	
#endif

#else
	Normal.xyz = gl_NormalMatrix * gl_Normal;
    gl_Position=ftransform();
#endif	
    EyeDirection=-(gl_ModelViewMatrix * gl_Vertex);
}

