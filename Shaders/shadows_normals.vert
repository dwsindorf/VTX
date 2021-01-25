#include "utils.h"
#include "common.h"

//#define _NORMALS

#if NVALS >0
attribute vec4 Position1;
#include "noise_funcs.h"
#endif

#ifdef TESSLVL
#include "geometry_funcs.h"
#endif

varying vec4 ShadowCoord;

varying vec4 Normal;
varying vec4 EyeDirection;
uniform bool lighting;

uniform mat4 smat;

void main(void) {
    vec4 p=gl_Vertex;
    Normal.xyz = gl_NormalMatrix * gl_Normal;
    
#if NVALS >0
    Vertex1=Position1; 
    
#ifdef NPZ
    SET_ZNOISE(NPZ);
    df=gv.yzw;
    //Normal.xyz=normalize(Normal.xyz-5e5*gl_NormalMatrix *df);
#endif
    vec4 VertexPosition=(gl_ModelViewMatrix * p);
    ShadowCoord=smat * VertexPosition;
	EyeDirection=-(gl_ModelViewMatrix * p);
#else
    vec4 VertexPosition=(gl_ModelViewMatrix * gl_Vertex);
    ShadowCoord=smat * VertexPosition;
	EyeDirection=-(gl_ModelViewMatrix * gl_Vertex);
	
    gl_Position=ftransform();
#endif
}

