
#include "utils.h"
#include "common.h"

#if NVALS >0
attribute vec4 Position1;
#include "noise_funcs.h"
#endif

#ifdef TESSLVL
#include "geometry_funcs.h"
#endif

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
}

