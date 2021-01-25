
#include "clouds.lighting.frag"
// ########## begin clouds.frag #########################

#if NVALS >0
#include "noise_funcs.h"
#endif
uniform sampler2D sprites;
varying vec4 CloudVars;
uniform bool textures;
uniform vec3 offset;

uniform bool backfacing;

//-------------------------------------------------------------
#ifdef PS
vec2 sprite(float index){
    vec2 l_uv=gl_PointCoord.xy;
    // apply random reflection
	if(CloudVars.w<0.33)
		l_uv.x=1-l_uv.x;
	else if(CloudVars.w>0.66)
		l_uv.y=1-l_uv.y;

	// apply sprite rotation by rotating image in sub-cell
	float cc =CloudVars.x;
	float ss = CloudVars.y;
	vec2 mat01 = vec2(cc, -ss);
	vec2 mat02 = vec2(ss, cc);
    vec2 st_centered = l_uv * 2.0 - 1.0;
	vec2 st_rotated = vec2(dot(st_centered,mat01),dot(st_centered,mat02));
	l_uv = st_rotated * 0.5 + 0.5;

    int y=index/ROWS;
    int x=index-ROWS*y;
	// offset to sprite top-left corner in image
	vec2 pt3=l_uv+vec2(x,y);

	return pt3/ROWS;
}
#endif
// ########## main section #########################
void main(void) {
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif
    vec3 normal=normalize(Normal.xyz);
#ifdef V3D
#ifdef BB
    vec2 l_uv=offset.z*(gl_TexCoord[0].xy+vec2(offset.x,offset.y));
#else
    vec2 l_uv=sprite(CloudVars.z);
#endif
	vec4 texcol=texture2D(sprites,l_uv);
	if(textures)
	color.a*=texcol.a;
#endif

#ifdef NCC
#ifdef V3D
	v1= Vertex1.xyz;
    vec4 ncolor=NCC;
 	color =ncolor*color;
#else
    NOISE_COLOR(NCC)
#endif
#endif

    if(lighting)
    	color.rgb=setLighting(color.rgb,normal);
    //float ah=lerp(Constants.w, 0, sky_ht, 0.0, 1.0); // ht
    //color.rgb=vec3(ah,0,0);
	gl_FragData[0]=color;
	//gl_FragData[0]=vec4(DEPTH,0,0,1.0);
	//if(backfacing)
		//discard;
	//	gl_FragData[0]=vec4(0,0,1,0.15);
	//else
		//discard;
	//	gl_FragData[0]=vec4(1,0,0,0.15);
	//gl_FragData[0]=Color;

}
// ########## end clouds.frag #########################
