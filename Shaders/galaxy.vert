#include "utils.h"

varying vec4 Color;

//uniform float pointsize;

varying float distance;

void main(void) {
	gl_Position = ftransform();
	Color=gl_Color;
#ifndef FGSTARS
	vec4 position = gl_ModelViewProjectionMatrix * gl_Vertex;
	distance=0.2e-4*sqrt(length(position));
	//float size=pointsize*min(distance,6.0);
#ifdef INSIDE
	//size*=3;
	//Color.a=1.2*Color.a/distance;
#endif
	//gl_PointSize=size;
#endif
}
