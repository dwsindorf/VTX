#include "utils.h"
#include "common.h"

#if NVALS >0
attribute vec4 Position1;
#include "noise_funcs.frag"
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
    //Vertex1.xyz=normalize(p.xyz+pv); 
	//Vertex1.xyz=Vertex1.xyz*0.5+0.5;
    
#ifdef NPZ
    SET_ZNOISE(NPZ);
    df=gv.yzw;
    Normal.xyz=normalize(Normal.xyz-0.1*gl_NormalMatrix *df);
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

