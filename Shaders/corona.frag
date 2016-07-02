// ########## begin clouds.frag #########################
#include "utils.h"

varying vec3 VertexPosition;

uniform float dpmax;
uniform float dpmin;
uniform float gradient;
uniform bool fbo_read;
uniform bool fbo_write;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;

uniform vec4 outer;  // outer color
uniform vec4 inner;  // inner color
uniform vec3 center;

void main(void) {
	vec4 color;
	vec3 viewdir = -normalize(VertexPosition.xyz);
	vec3 radius = normalize(-VertexPosition-center); // vertex-center
    vec3 ec = normalize(center);                 // eye-center

	float dp=dot(radius, ec);
	float d=lerp(dp, dpmax-2.0*dpmin, dpmax, 0.0, 1.0);

	float f=pow(d,10.0*gradient);
	color=f*inner+(1.0-f)*outer;
	color.a*=f;
	if (fbo_write){
		// r = amount of corona color to mix with sky
		// g = increase in sky opacity
		// a = FBO transparency
		//vec4 data2=vec4(f*f*0.1,1.0,0.0,color.a);
		//vec4 data2=vec4(f*f*f*0.1,1.0,0.0,1.0);
		vec4 data2=vec4(f*f*f*0.1,0.0,0.0,1.0);
		gl_FragData[1] = data2;
		gl_FragData[2]=vec4(0,0,0,1);
	}
	color=clamp(color,0.0,1.0);
	gl_FragData[0]=color;//vec4(1.0,1.0,1.0,color.a);//color;
}
// ########## end clouds.frag #########################
