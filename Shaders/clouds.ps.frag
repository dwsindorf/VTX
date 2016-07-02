
#include "clouds.lighting.frag"
//########## 3D noise section #########################

uniform sampler2D sprites;
uniform float ROWS;
uniform bool textures;

varying vec4 CloudVars;

//-------------------------------------------------------------

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

// ########## main section #########################
void main(void) {
#ifdef COLOR
	vec4 color=Color;
#else
	vec4 color=vec4(1.0);
#endif
    vec3 normal=normalize(Normal.xyz);

    vec2 l_uv=sprite(CloudVars.z);
	vec4 texcol=texture2D(sprites,l_uv);
	if(textures)
		color.a*=texcol.a;

    if(lighting)
    	color.rgb=setLighting(color.rgb,normal);
	gl_FragData[0]=color;
}
// ########## end clouds.frag #########################
