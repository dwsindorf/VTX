#include "utils.h"
#include "common.h"

#if NVALS >0
attribute vec4 Position1;
#include "noise_funcs.h"
#endif

void main(void) {

#if NVALS >0
    Vertex1=Position1;
   
#ifdef NPZ
    vec4 p=gl_Vertex;
    CALC_ZNOISE(NPZ);
    gl_Position=gl_ModelViewProjectionMatrix * p;
#endif
#else
    gl_Position=ftransform();
#endif
#ifdef COLOR
	Color=gl_Color;
#endif

}
