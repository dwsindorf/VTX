

uniform sampler2D samplers2d[NSPRITES];

varying vec4 index;

vec4 color;
//-------------------------------------------------------------

// ########## main section #########################
void main(void) {
    vec2 l_uv=gl_PointCoord.xy;
    int i=index.x+0.1;

 	color=texture2D(samplers2d[i],l_uv);
 
 	gl_FragData[0]=color;
}
// ########## end clouds.frag #########################
