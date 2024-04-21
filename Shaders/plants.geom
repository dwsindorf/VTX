#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

varying in vec4 Color_G[];
varying in vec4 Normal_G[];
varying in vec4 Factors[];
varying in vec4 TexVars_G[];

varying out vec4 Normal;
varying out vec4 Color;
//varying out vec4 pnorm;
varying out vec4 TexVars;

#define PI		3.14159265359
// TODO: Hermite spline 
//  o pre-generate 4x4 spline matrix M (pass in via fixed constant uniform)
//    - M = [2,-2,1,1][-3,3,-2,-1][0,0,1,0][1,0,0,0]
//  o for each line segment pass in two points (P0,P1) and 2 vectors (V0,V1) 
//    - in next line segment P0,V0 assigned to P1,V1 of previous segment (in opengl)
//  o angle calculation needed at top and bottom for width 
//    - bottom angle obtained from angle between v1 and v0
//    - assume top angle 90 degrees from top vector/point in screen space pi/2-atan2(dv1.y,dv1.x)
//    - also need width values for top and bottom (also passed in)
//  o generate a set of n intermediate points for each spline segment (n>=1)
//   - use TxMxPV to calculate points (T=[t^3,t^2,t,1] M=hermite matrix(fixed), PV=[P0,P1,V0,V1)
//   - may be able to parametize "t" 0..1 (e.g 0,1/4,1/2,3/4) 
//   - use intermediate points and last point to draw multiple polygon segments (like below)
//   - for each new "t" point V(t) = P(t)-P(t-1) W(t) = pi/2-atan2(dv1.y,dv1.x)
//    - construct polygon using W(t) and W(t-1)
//  o Note: only need to create spline for main branches 
//   - for forked side branches start a new spline curve at branching start point
void main(void) {

	Color=Color_G[0];
	
	 vec4 pnorm;

	vec3 ps1=gl_PositionIn[0].xyz;	
	vec3 ps2=gl_PositionIn[1].xyz;
		
	float topx=Factors[0].r;
	float topy=Factors[0].g;
	float botx=Factors[0].b;
	float boty=Factors[0].a;
	
	float nscale=TexVars_G[0].w;

	TexVars=TexVars_G[0];
	
	float dx2=topx;
	float dx1=botx;
	
	vec3 norm;
    // draw a polygon
    
	norm=vec3(dx2,0,0);
    pnorm.xyz = gl_NormalMatrix * norm;
    Normal.xyz=Normal_G[0].xyz-nscale*pnorm;
    gl_Position = vec4(ps2.x-topx,ps2.y-topy,ps2.z,1); // top-left
    gl_TexCoord[0].xy=vec2(0,0);
    EmitVertex();
   
    norm=vec3(-dx2,0,0);
    pnorm.xyz = gl_NormalMatrix * norm;
    Normal.xyz=Normal_G[0].xyz-nscale*pnorm;
    gl_Position = vec4(ps2.x+topx,ps2.y+topy,ps2.z,1); // top-right  
    gl_TexCoord[0].xy=vec2(1,0);
    EmitVertex();
        
    norm=vec3(dx1,0.0,0);
    pnorm.xyz = gl_NormalMatrix * norm;
    Normal.xyz=Normal_G[0].xyz-nscale*pnorm;
    gl_TexCoord[0].xy=vec2(0,1);
    gl_Position = vec4(ps1.x-botx,ps1.y-boty,ps1.z,1);  // bot-left 
    EmitVertex();
    
    norm=vec3(-dx1,0.0,0);
    pnorm.xyz = gl_NormalMatrix * norm;
    Normal.xyz=Normal_G[0].xyz-nscale*pnorm;
    gl_TexCoord[0].xy=vec2(1,1);
    gl_Position = vec4(ps1.x+botx,ps1.y+boty,ps1.z,1);  // bot-right 
    EmitVertex(); 
 
  	EndPrimitive();
  
}

