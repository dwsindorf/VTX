#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

varying in vec4 Color_G[];
varying in vec4 Normal_G[];
varying in vec4 Constants1[];
varying in vec4 TexVars_G[];

varying out vec4 Color;
varying out vec4 TexVars;
varying out vec3 Normal;
varying out vec3 Pnorm;

uniform float norm_scale;

#define PI		3.14159265359
// Hermite spline (TODO) 
// To smooth branch curves and add possible speedup (fewer opengl vertexes needed) 
//  o pre-generate 4x4 spline matrix M (pass in via fixed mat3 constant or uniform)
//    - M = [2,-2,1,1][-3,3,-2,-1][0,0,1,0][1,0,0,0]
//  o for each line segment pass in two points (P0,P1) and 2 vectors (V0,V1) 
//    - in next line segment P0,V0 assigned to P1,V1 of previous segment (in opengl)
//  o angle calculation needed at top and bottom for width 
//    - bottom angle obtained from angle between v1 and v0
//    - assume top angle 90 degrees from top vector/point in screen space PI/2-atan2(dv1.y,dv1.x)
//    - also need width values for top and bottom (also passed in)
//  o generate a set of n intermediate points for each spline segment (n>=1)
//   - use TxMxPV to calculate points T=[t^3,t^2,t,1] M=hermite matrix(fixed), PV=[P0,P1,V0,V1)
//   - may be able to parametize "t" 0..1 (e.g 0,1/4,1/2,3/4) 
//   - use intermediate points and last point to draw multiple polygon segments (like below)
//   - for each new "t" point V(t) = P(t)-P(t-1) W(t) = pi/2-atan2(dv1.y,dv1.x)
//   - construct polygon using W(t) and W(t-1)
//  o Note: only need to create spline for main branches 
//   - for forked side branches start a new spline curve at branching start point
//  o more work to do to generate width and normals for spline seqments 
//   - pass in width values for top and bottom anchor points
//   - for each calclated point use linear interpolation from anchor points
// Bumpmap textures (DONE)
//  o In fragment shader constuct dx and dy vectors from texture
//   - read parameters from uniform float variables 
//     bump_delta: small svalue (e.g. 1.0/image_width)
//     bump_ampl: how much intensity to apply (TBD)
//   - calculate ave intensity of texture:
//     vec2 l_uv=gl_TexCoord[0].xy;
//     vec4 tval=texture2D(samplers2d[texid],l_uv);
//     float tva=(tval.x+tval.y+tval.z)/3.0;
//   - construct 2d delta vectors in x and y by fetching texture after applying a small bump_delta
//	   vec2 ds=vec2(l_uv.x+bump_delta,l_uv.y); 
//     vec2 dt=vec2(l_uv.x,l_uv.y+bump_delta);
//     vec4 tcs=texture2D(samplers2d[texid],ds);
//     vec4 tct=texture2D(samplers2d[texid],dt);
//     float tsa=(tcs.x+tcs.y+tcs.z)/3.0;
//     float tta=(tct.x+tct.y+tct.z)/3.0;
//  o construct delta vector 
//	 - vec3 tc=vec3(tsa-tva,tta-tva, 0.0);
//  o calculate bump intensity my multipying tc with TX
//   - vec3 bv=bump_ampl*TX*tc;
//  o calculate lighting contribution from bump
//   - vec3 bmp  = -gl_NormalMatrix*bv;
//   - vec3 normal  = normalize(Normal.xyz+bmp);
//   - use normal (vs Normal) in lighting calculation
//  o possible speedup by generating a separate intensity texture in opengl ?
//   - could use pixel intensity vs average values calculated in shader
// Leaf Support
//  o For leafs change shape so that instead of a rectangle, a "diamond" pattern is rendered ?
//  o Add option to render Texture as a point sprite 
// Shader width calculation (vs Opengl)
//  - need to pass in width at top and bottom (w1,w2)
//  - can calculate angle of current vector by subtracting input points (p2,p1)
//  - but also need vec3 of previous point (p0) to calculate offset angle for bottom
//  - so for shader to work need to pass in 3 vs 2 points (GL_TRIANGLE vs GL_LINE)
//  o test code
//    vec2 v=(ps1.xy-ps2.xy); // note: reversed bot-top !
//    v=normalize(v);
//	  double a=atan2(v.y,v.x);		 	
//	  float x = -sin(a);  // same calculation as in opengl code
//	  float y = cos(a);	
//	  vec2 vv=vec2(topy,topx); // could pass in single width float value;
//	  float l=length(vv);	 	
// 	  topx=(l*x); // this seems to kinda work but see gaps in branch segments
// 	  topy=(l*y);


 // draw a line
void emitLine(){
 	float nscale=TexVars.r;
 
    // need at least 3 vertexes for line strip
    Pnorm=gl_NormalMatrix*vec3(nscale,0,0);
    gl_Position = gl_PositionIn[1]; // top
    EmitVertex();
    EmitVertex();
    
    gl_Position = gl_PositionIn[0]; // bottom
    EmitVertex();
    
	EndPrimitive();
 }
 
// draw a polygon
void emitRectangle(){

	vec3 ps1=gl_PositionIn[0].xyz;	
	vec3 ps2=gl_PositionIn[1].xyz;
	
	float top_offset=gl_PositionIn[1].w;
	float bot_offset=gl_PositionIn[0].w;
 	
	float topx=Constants1[0].r;
	float topy=Constants1[0].g;
	float botx=Constants1[0].b;
	float boty=Constants1[0].a;
	
	vec2 vtop=vec2(topx,topy);
    vec2 vbot=vec2(botx,boty);

    vec2 v2=normalize(vec2(topx,topy));
    vec2 v1=normalize(vec2(botx,boty));
    
    vec2 top_left=vtop*(1+top_offset);
    vec2 top_right=vtop*(1-top_offset);
    vec2 bot_left=vbot*(1+bot_offset);
    vec2 bot_right=vbot*(1-bot_offset);
    
 	Pnorm=vec3(v2,0);
    gl_Position = vec4(ps2.xy-top_left,ps2.z,1); // top-left
    gl_TexCoord[0].xy=vec2(0,0);
    EmitVertex();
   
    Pnorm=vec3(-v2,0);
    gl_Position = vec4(ps2.xy+top_right,ps2.z,1); // top-left
    gl_TexCoord[0].xy=vec2(1,0);
    EmitVertex();
        
    Pnorm=vec3(v1,0);
    gl_TexCoord[0].xy=vec2(0,1);
    gl_Position = vec4(ps1.xy-bot_left,ps1.z,1);  // bot-left 
    EmitVertex();
    
    Pnorm=vec3(-v1,0);
    gl_TexCoord[0].xy=vec2(1,1);
    gl_Position = vec4(ps1.xy+bot_right,ps1.z,1);  // bot-left 
    EmitVertex(); 
 
    EndPrimitive();
}

void main(void) {
    if(length(gl_PositionIn[1]-gl_PositionIn[0])>2)
    	return;
 	Color=Color_G[0];
	Normal.xyz=Normal_G[0].xyz;
	TexVars=TexVars_G[0];
    int mode=TexVars_G[0].w;
    
    if(mode<0.1)
    	emitLine();
    else
    	emitRectangle();
 
}

