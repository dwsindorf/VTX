#include "utils.h"
attribute vec4 TextureAttributes;

varying vec4 Color;

uniform float pointsize;

varying float distance;
varying vec4 galaxyVars;

void main(void) {
	gl_Position = ftransform();
	Color=gl_Color;
	vec4 position = gl_ModelViewProjectionMatrix * gl_Vertex;
	distance=0.2e-4*sqrt(length(position));
//#ifndef FGSTARS	
//	gl_PointSize = TextureAttributes.z;
//#endif
	float angle=TextureAttributes.y;
	float cc = cos(angle);
	float ss = sin(angle);
	galaxyVars.x=cc;
	galaxyVars.y=ss;
	galaxyVars.z=TextureAttributes.x;
	galaxyVars.w=TextureAttributes.w;
		
}
