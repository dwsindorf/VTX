#include "clouds.lighting.frag"

#if NVALS >0
#include "noise_funcs.frag"
#endif
uniform sampler2D sprites;
uniform vec3 offset;

uniform bool textures;

uniform float INVROWS;

// ########## main section #########################
void main(void) {
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif
    vec3 normal=normalize(Normal.xyz);

    vec2 l_uv=INVROWS*(gl_TexCoord[0].xy);

	vec4 texcol=texture2D(sprites,l_uv);
	if(textures)
		color.a*=texcol.a;
    if(lighting)
    	color.rgb=setLighting(color.rgb,normal);

	gl_FragData[0]=color;
	gl_FragData[1]=vec4(0,0,0,1);
	gl_FragData[2]=vec4(0,0,0,1);

}
// ########## end clouds.frag #########################
