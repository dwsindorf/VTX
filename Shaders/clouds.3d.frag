#include "clouds.lighting.frag"

#if NVALS >0
#include "noise_funcs.h"
#endif
uniform sampler2D sprites;
uniform vec3 offset;

uniform bool textures;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;

uniform float INVROWS;

// ########## main section #########################
void main(void) {
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif
#ifdef NCC
	NOISE_COLOR(NCC);
#endif
    vec3 normal=normalize(Normal.xyz);

    vec2 l_uv=INVROWS*(gl_TexCoord[0].xy);

	vec4 texcol=texture2D(sprites,l_uv);
	if(textures)
		color.a*=texcol.a;
	if(color.a<0)
	    color.a=0;
    if(lighting)
    	color.rgb=setLighting(color.rgb,normal);

	gl_FragData[0]=color;
	gl_FragData[1]=texture2DRect(FBOTex2, gl_FragCoord.xy); // FBO properties (background)
	//gl_FragData[1]=vec4(0,0,0,1);
	gl_FragData[2]=vec4(0,0,0,1);

}
// ########## end clouds.frag #########################
