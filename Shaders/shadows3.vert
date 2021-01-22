
#include "utils.h"
#include "common.h"

#if NVALS >0
attribute vec4 Position1;
#include "noise_funcs.frag"
#endif

void main(void) {
#if NVALS >0
    Vertex1=Position1;
   
#ifdef NPZ
    vec4 p=gl_Vertex;
    SET_ZNOISE(NPZ);
#endif
#else
    gl_Position=ftransform();
#endif
}

