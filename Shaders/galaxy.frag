#include "utils.h"

uniform bool fbo_write;
uniform sampler2D startex;
uniform sampler2D dusttex;

uniform float color_mix;
varying vec4 Color;
varying float distance;
varying vec4 galaxyVars;

#define ROWS 2.0
#define INVROWS 0.5
#define INDX galaxyVars.z

vec2 sprite(int index){
    vec2 l_uv=gl_PointCoord.xy;
	if(galaxyVars.w<0.33)
		l_uv.x=1-l_uv.x;
	else if(galaxyVars.w>0.66)
		l_uv.y=1-l_uv.y;

    if(galaxyVars.x || galaxyVars.y){
        // apply random reflection
		// apply sprite rotation by rotating image in sub-cell
		float cc =galaxyVars.x;
		float ss = galaxyVars.y;
		vec2 mat01 = vec2(cc, -ss);
		vec2 mat02 = vec2(ss, cc);
	    vec2 st_centered = l_uv * 2.0 - 1.0;
		vec2 st_rotated = vec2(dot(st_centered,mat01),dot(st_centered,mat02));
		l_uv = st_rotated * 0.5 + 0.5;
	}

    float x=index/2.0-floor(index/2.0);
    float y=0.5*floor(index/2.0);
	// offset to sprite top-left corner in image
	vec2 pt3=l_uv*0.5+vec2(x,y);
	return pt3;
}
// ----------- program entry -----------------
void main(void) {
float alpha;
	float bias =lerp(distance,7.0,9.0,-2.0,1.0);
    vec2 l_uv;
    vec4 texcol;
    if(INDX<4){
       l_uv=sprite(INDX);
       texcol=texture2D(startex,l_uv,bias);
    }
    else{
        l_uv=sprite(INDX-4);
        texcol=texture2D(dusttex,l_uv,bias);
	}
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
