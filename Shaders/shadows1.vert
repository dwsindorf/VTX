#include "utils.h"
#include "common.h"

#if NVALS >0
attribute vec4 Position1;
#include "noise_funcs.frag"
#endif

varying vec4 ShadowCoord;

varying vec4 Normal;
varying vec4 EyeDirection;

uniform mat4 smat;

void main(void) {
    vec4 p=gl_Vertex;
#if NVALS >0
    Vertex1=Position1; 
#ifdef NPZ
    SET_ZNOISE(NPZ);
#endif	
#else
    gl_Position=ftransform();
#endif
    vec4 VertexPosition=(gl_ModelViewMatrix * gl_Vertex);
    ShadowCoord=smat * VertexPosition;
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex);
	Normal.xyz = gl_NormalMatrix * gl_Normal;
}

