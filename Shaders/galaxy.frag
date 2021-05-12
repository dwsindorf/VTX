#include "utils.h"

uniform sampler2D startex;

uniform float color_mix;
varying vec4 Color;
varying float distance;
varying vec4 galaxyVars;

const float ROWS=4.0;
const float COLS=4.0;

const float INVROWS=1.0/ROWS;
const float INVCOLS=1.0/COLS;

#define INDX galaxyVars.z

vec2 sprite(float index){
    vec2 l_uv=gl_PointCoord.xy;
    vec2 offset=vec2(INVCOLS,INVROWS);
  
   // apply random reflection
 
	if(galaxyVars.w<0.33)
		l_uv.x=1-l_uv.x;
	else if(galaxyVars.w>0.66)
		l_uv.y=1-l_uv.y;
 
    if(galaxyVars.x || galaxyVars.y){
		// apply sprite rotation by rotating image in sub-cell
		float cc =galaxyVars.x;
		float ss = galaxyVars.y;
		vec2 mat01 = vec2(cc, -ss);
		vec2 mat02 = vec2(ss, cc);
	    vec2 st_centered = l_uv * 2.0 - 1.0;
		vec2 st_rotated = vec2(dot(st_centered,mat01),dot(st_centered,mat02));
		l_uv = st_rotated * 0.5 + 0.5;
	}

    float x=INVCOLS*floor(index*INVROWS);   
    float y=INVROWS*floor(index*INVCOLS);
    
	// offset to sprite top-left corner in image
	l_uv=l_uv*offset;
	vec2 pt3=l_uv+vec2(x,y);
	
	return pt3;
}
// ----------- program entry -----------------
void main(void) {
float alpha;
	float bias =lerp(distance,7.0,9.0,-2.0,1.0);
    
    vec2 l_uv=sprite(INDX);
    vec4 texcol=texture2D(startex,l_uv);
       
	vec4 color=Color;
	alpha=texcol.a;
	color.a=alpha*Color.a;
	float f=color_mix*texcol.a*texcol.a;
	color.rgb=color.rgb*(1.0-f)+vec3(1.0)*f;
    color.a*=length(color);
	gl_FragData[0] = color;
#ifdef INSIDE_SKY
	alpha =Color.a;
	gl_FragData[1] = vec4(0.0,0.0,0.0,alpha);
#else
	gl_FragData[1] = vec4(0.0);
#endif
}
