// ########## begin halo.frag #########################
#include "common.h"
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
	//float d=lerp(dp, dpmax-2.0*dpmin, dpmax, 0.0, 1.0);
	float d=lerp(dp, dpmax, dpmin, 0, 1);

	float f=pow(1-d,gradient+0.5);
	color=f*outer+(1.0-f)*inner;
	color.a*=f;
	
	if (fbo_write){
		// r = amount of corona color to mix with sky
		// g = increase in sky opacity
		// a = FBO transparency
		vec4 data2=vec4(0,0.0,0.0,1.0);
		gl_FragData[1] = data2;
		gl_FragData[2]=vec4(0,0,0,0);
	}
	color=clamp(color,0.0,1.0);
	//gl_FragData[1]=texture2DRect(FBOTex2, gl_FragCoord.xy); // FBO properties (background)	
	gl_FragData[0]=color;
	
}
// ########## end halo.frag #########################
