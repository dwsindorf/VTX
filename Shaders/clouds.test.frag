
#include "clouds.lighting.frag"

// ########## main section #########################
void main(void) {
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif
    vec3 normal=normalize(Normal.xyz);

    if(lighting)
    	color.rgb=setLighting(color.rgb,normal);
	gl_FragData[0]=color;
}
// ########## end clouds.frag #########################
