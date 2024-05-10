#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

varying in vec4 Color_G[];
varying in vec4 Normal_G[];
varying in vec4 Constants1[];
varying in vec4 Constants2[];
varying in vec4 TexVars_G[];

varying out vec4 Color;
varying out vec4 TexVars;
varying out vec3 Normal;
varying out vec3 Pnorm;

uniform float norm_scale;

#define PI		3.14159265359
// spline (WIP) 
//  o Smooth branch curves and add possible speedup (fewer opengl vertexes needed)
//  - current scheme uses quadratic interpolation from last 3 points to generate intermediate sub-sections
//  - TODO: move implementation to geometry shader 
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
// Leaf Support (DONE)
//  o For transparent leafs a rectangle billboard is rendered
//  o For non-transparent leafs a "diamond" billboard is rendered

// Shader width calculation (vs Opengl)
//  - need to pass in width at top and bottom (w1,w2)
//  - can calculate angle of current vector by subtracting input points (p2,p1)
//  - but also need vec3 of previous point (p0) to calculate offset angle for bottom
//  - so for shader to work need to pass in 3 vs 2 points (GL_TRIANGLE vs GL_LINE)
//  o test code
//    vec2 v=(ps1.xy-ps2.xy); // note: reversed bot-top !
//    v=normalize(v);
//	  float a=atan2(v.y,v.x);		 	
//	  float x = -sin(a);  // same calculation as in opengl code
//	  float y = cos(a);	
//	  vec2 vv=vec2(topy,topx); // could pass in single width float value;
//	  float l=length(vv);	 	
// 	  topx=(l*x); // this seems to kinda work but see gaps in branch segments
// 	  topy=(l*y);

#define LINE 0
#define BRANCH 1
#define LEAF   2
#define SPLINE 3

// draw a line
void emitLine(){
 	float nscale=1e-5;//TexVars.r;
 
    // need at least 3 vertexes for line strip
    Pnorm=gl_NormalMatrix*vec3(nscale,0,0);
    gl_Position.xyz = gl_PositionIn[1].xyz; // top
    gl_Position.w=1;
    EmitVertex();
    EmitVertex();
    
    gl_Position.xyz = gl_PositionIn[0].xyz; // top
    gl_Position.w=1;
    EmitVertex();
    
	EndPrimitive();
 }
 
// draw a leaf
void emitLeaf(){
	vec3 ps1=gl_PositionIn[0].xyz;	
	vec3 ps2=gl_PositionIn[1].xyz;
	float topx=Constants1[0].r;
	float topy=Constants1[0].g;
	int colmode=TexVars_G[0].g+0.1; // transparenct flag
	int rectmode=colmode & 4;
	
	vec2 va=vec2(topx,topy);
    vec2 v2=normalize(va); 
    vec3 pc=vec3(gl_PositionIn[1].xyz-gl_PositionIn[0].xyz);
    vec3 vw=vec3(topx,topy,gl_PositionIn[1].z);   
    Pnorm=0.1*normalize(cross(pc,vw));
   
   if(rectmode){
       vec2 vtop=vec2(topx,topy);
    
	   gl_TexCoord[0].xy=vec2(0,0);
	   gl_Position = vec4(ps1.xy-vtop,ps1.z,1);  // bot-left 
	   EmitVertex();
	    
	   gl_TexCoord[0].xy=vec2(1,0);
	   gl_Position = vec4(ps1.xy+vtop,ps1.z,1);  // bot-right 
	   EmitVertex(); 
	   
	   gl_Position = vec4(ps2.xy-vtop,ps2.z,1); // top-left
	   gl_TexCoord[0].xy=vec2(0,1);
	   EmitVertex();
	   
	   gl_Position = vec4(ps2.xy+vtop,ps2.z,1); // top-right
	   gl_TexCoord[0].xy=vec2(1,1);
	   EmitVertex();
   }
   else{ 
   		vec4 pa=0.5*(gl_PositionIn[0]+gl_PositionIn[1]);
     
	    gl_Position = vec4(ps1.xy,ps1.z,1);   // bottom
	    gl_TexCoord[0].xy=vec2(0.5,0);
	    EmitVertex();
	      
	    gl_TexCoord[0].xy=vec2(1,0.5);
	    gl_Position = vec4(pa.xy+va,pa.z,1);  // right
	    EmitVertex(); 
	    
	    gl_Position = vec4(pa.xy-va,pa.z,1);  // left
	    gl_TexCoord[0].xy=vec2(0,0.5);
	    EmitVertex();
	    
	    gl_TexCoord[0].xy=vec2(0.5,1);
	    gl_Position = vec4(ps2.xy,ps2.z,1);  // top 
	    EmitVertex();
    }  
    EndPrimitive();
 }
 
void emitRectangle(vec4 p1,vec4 p2, vec4 c, vec4 tx){
	vec3 ps1=p1.xyz;	
	vec3 ps2=p2.xyz;
	
	float bot_offset=p1.w;
	float top_offset=p2.w;
 	
	float topx=c.r;
	float topy=c.g;
	float botx=c.b;
	float boty=c.a;
	
	vec2 vtop=vec2(topx,topy);
    vec2 vbot=vec2(botx,boty);

    vec2 v2=normalize(vec2(topx,topy));
    vec2 v1=normalize(vec2(botx,boty));
    
    vec2 top_left=vtop*(1+top_offset);
    vec2 top_right=vtop*(1-top_offset);
    vec2 bot_left=vbot*(1+bot_offset);
    vec2 bot_right=vbot*(1-bot_offset);
  
    Pnorm=vec3(v1,0);
    gl_TexCoord[0].xy=vec2(0,tx.w);
    gl_Position = vec4(ps1.xy-bot_left,ps1.z,1);  // bot-left 
    EmitVertex();
    
    Pnorm=vec3(-v1,0);
    gl_TexCoord[0].xy=vec2(1,tx.w);
    gl_Position = vec4(ps1.xy+bot_right,ps1.z,1);  // bot-right 
    EmitVertex(); 
   
 	Pnorm=vec3(v2,0);
    gl_TexCoord[0].xy=vec2(0,tx.y);
    gl_Position = vec4(ps2.xy-top_left,ps2.z,1); // top-left
    EmitVertex();
   
    Pnorm=vec3(-v2,0);
    gl_TexCoord[0].xy=vec2(1,tx.y);
    gl_Position = vec4(ps2.xy+top_right,ps2.z,1); // top-right
    EmitVertex();
    EndPrimitive();

}

// draw a polygon
void emitBranch(){
   vec4 p1=gl_PositionIn[0];
   vec4 p2=gl_PositionIn[1];
   vec4 c=Constants1[0];
   emitRectangle(p1,p2,c,vec4(0,0,0,1));
}

mat3 m=mat3(2,-3,1,-4,4,0,2,-1,0);
vec3 spline(float x, vec3 p0, vec3 p1, vec3 p2){
  //mat3 p=mat3(p0,p1,p2);
  //mat3 r=m*p;
  //return r[0]*x*x+r[1]*x+r[2];
  vec3 c=p0;
  vec3 b=p1*4.0-p0*3.0-p2;
  vec3 a=p2*2.0+p0*2.0-p1*4.0;
  return a*x*x+b*x+c;
}

// draw a spline
void emitSpline(){
    vec3 p1=gl_PositionIn[0].xyz;
    vec3 p2=gl_PositionIn[1].xyz;    
   
 	float bot_offset=gl_PositionIn[0].w;
  	float top_offset=gl_PositionIn[1].w;
  	
    vec3 p0=Constants2[0].xyz;
   
    float topx=Constants1[0].r;
	float topy=Constants1[0].g;
	float botx=Constants1[0].b;
	float boty=Constants1[0].a;
	
    vec4 c=Constants1[0];
	int nv=6;
	float ds=0.5/nv;
	float s=0.5;

	float delta=1.0/nv;
	vec2 bot=vec2(botx,boty);
	vec2 top=vec2(topx,topy);
	for(int i=0;i<nv;i++){
		float f1=i*delta;
		float f2=(i+1)*delta;
		vec2 xy1=(1-f1)*bot+f1*top;
		vec2 xy2=(1-f2)*bot+f2*top;
		float off1=(1.0-f1)*bot_offset+f1*top_offset;
		float off2=(1.0-f2)*bot_offset+f2*top_offset;
		c=vec4(xy2, xy1);
		vec3 t1=spline(s,p0,p1,p2);
		vec3 t2=spline(s+ds,p0,p1,p2);
		vec4 tx=vec4(0,f1,0,f2);
		
		emitRectangle(vec4(t1,off1),vec4(t2,off2),c,tx);
		
		s+=ds;
	}
	
}
void main(void) {
    if(length(gl_PositionIn[1]-gl_PositionIn[0])>2)
    	return;
 	Color=Color_G[0];
	Normal.xyz=Normal_G[0].xyz;
	TexVars=TexVars_G[0];
    int mode=TexVars_G[0].w+0.1;
    
    if(mode==LINE)
    	emitLine();
    else if(mode==BRANCH)
    	emitBranch();
    else if(mode==LEAF)
    	emitLeaf();
    else
        emitSpline(); 
}

