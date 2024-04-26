#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

varying in vec4 Color_G[];
varying in vec4 Normal_G[];
varying in vec4 Factors[];
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

 // draw a line
void emitLine(){
 
    // need at least 3 vertexes for line strip
    Pnorm=gl_NormalMatrix*vec3(1e-7,0,0);
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
		
	float topx=Factors[0].r;
	float topy=Factors[0].g;
	float botx=Factors[0].b;
	float boty=Factors[0].a;
	
	float dx2=topx;
	float dx1=botx;
    
	Pnorm=gl_NormalMatrix*vec3(dx2,0,0);
    gl_Position = vec4(ps2.x-topx,ps2.y-topy,ps2.z,1); // top-left
    gl_TexCoord[0].xy=vec2(0,0);
    EmitVertex();
   
    Pnorm=gl_NormalMatrix*vec3(-dx2,0,0);
    gl_Position = vec4(ps2.x+topx,ps2.y+topy,ps2.z,1); // top-right  
    gl_TexCoord[0].xy=vec2(1,0);
    EmitVertex();
        
    Pnorm=gl_NormalMatrix*vec3(dx1,0.0,0);
    gl_TexCoord[0].xy=vec2(0,1);
    gl_Position = vec4(ps1.x-botx,ps1.y-boty,ps1.z,1);  // bot-left 
    EmitVertex();
    
    Pnorm=gl_NormalMatrix*vec3(-dx1,0.0,0);
    gl_TexCoord[0].xy=vec2(1,1);
    gl_Position = vec4(ps1.x+botx,ps1.y+boty,ps1.z,1);  // bot-right 
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
    	emitRectangle();
    else
   		emitLine();
}

