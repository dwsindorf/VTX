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
varying out vec4 Pnorm;

#define PI		3.14159265359
// spline (DONE) 
//  o Smooth branch curves and add possible speedup (fewer opengl vertexes needed)
// Bumpmap textures (DONE)
// Leaf Support (DONE)
//  o For transparent leafs a rectangle billboard is rendered
//  o For non-transparent leafs a "diamond" billboard is rendered
// Shader width calculation (vs Opengl)
//  - need to pass in width at top and bottom (w1,w2)
//  - can calculate angle of current vector by subtracting input points (p2,p1)
//  - but also need vec3 of previous point (p0) to calculate offset angle for bottom
//  - so for shader to work need to pass in 3 vs 2 points (GL_TRIANGLE vs GL_LINE)

#define LINE 0
#define BRANCH 1
#define LEAF   2
#define SPLINE 3

// draw a line
void emitLine(){
 	float nscale=1e-5;//TexVars.r;
 
    // need at least 3 vertexes for line strip
    Pnorm.xyz=gl_NormalMatrix*vec3(nscale,0,0);
    Pnorm.w=0;
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
	float topx=Constants1[0].r;
	float topy=Constants1[0].g;
	int colmode=TexVars_G[0].g+0.1; // transparenct flag
	int rectmode=colmode & 4;
	
    vec3 vw=vec3(topx,topy,gl_PositionIn[1].z);   
    float ps=Constants1[0].g; // size
    vec4 v=ps*normalize(gl_PositionIn[1]-gl_PositionIn[0]); 
    vec4 pa=gl_PositionIn[0]+v;// end
 
 	float ta=Constants1[0].r+PI/2; // position angle
	float cc=cos(ta);
	float ss=sin(ta);    	   		   
	mat2  M=ps*mat2(cc,ss,-ss,cc);
	
	vec2 tang=M*vec2(-1,-1)-M*vec2(1,-1);
	vec3 t=normalize(vec3(tang,pa.z));
    Pnorm.xyz=normalize(cross(pa,t));
    Pnorm.w=0.01;
         
    if(rectmode){ // use a rectangle (for transparent textures)
       float w=2*TexVars.r;
    
  	   gl_Position = vec4(pa.xy+M*vec2(-w,-1),pa.z,1);   // bottom-left   
	   gl_TexCoord[0].xy=vec2(0,0);
	   EmitVertex();

	   gl_Position = vec4(pa.xy+M*vec2(w,-1),pa.z,1);   // bot-right
	   gl_TexCoord[0].xy=vec2(1,0);
	   EmitVertex(); 
	   
	   gl_Position = vec4(pa.xy+M*vec2(-w,1),pa.z,1);   // top-left	    
	   gl_TexCoord[0].xy=vec2(0,1);
	   EmitVertex();
	   
	   gl_Position = vec4(pa.xy+M*vec2(w,1),pa.z,1);   // top-right	    
	   gl_TexCoord[0].xy=vec2(1,1);
	   EmitVertex();
   }
   else { // use a diamond shape for solid textures or color only
       float w=TexVars.r;
    
 	   gl_Position = vec4(pa.xy+M*vec2(0,-1),pa.z,1);   // bottom       
	   gl_TexCoord[0].xy=vec2(0.5,0);
	   EmitVertex();
	   
	   gl_Position = vec4(pa.xy+M*vec2(0.9*w,-0.8),pa.z,1);
	   gl_TexCoord[0].xy=vec2(1,0.25);
	   EmitVertex(); 

	   gl_Position = vec4(pa.xy+M*vec2(-0.9*w,-0.8),pa.z,1);
	   gl_TexCoord[0].xy=vec2(0,0.25);
	   EmitVertex(); 
	      
	   gl_Position = vec4(pa.xy+M*vec2(w,-0.5),pa.z,1);
	   gl_TexCoord[0].xy=vec2(1,0.5);
	   EmitVertex(); 
	    
	   gl_Position = vec4(pa.xy+M*vec2(-w,-0.5),pa.z,1);
	   gl_TexCoord[0].xy=vec2(0,0.5);
	   EmitVertex();

	   gl_Position = vec4(pa.xy+M*vec2(0.7*w,-0.25),pa.z,1);
	   gl_TexCoord[0].xy=vec2(1,0.5);
	   EmitVertex(); 
	    
	   gl_Position = vec4(pa.xy+M*vec2(-0.7*w,-0.25),pa.z,1);
	   gl_TexCoord[0].xy=vec2(0,0.5);
	   EmitVertex();
	    
	   gl_Position = vec4(pa.xy,pa.z,1);   // top
	   gl_TexCoord[0].xy=vec2(0.5,1);
	   EmitVertex();
    }  
    EndPrimitive();
 }

// branches  
void emitRectangle(vec4 p1,vec4 p2, vec4 c, vec4 tx){
	
	float bot_offset=p1.w;
	float top_offset=p2.w;
 	
	float topx=c.r;
	float topy=c.g;
	float botx=c.b;
	float boty=c.a;
	
	vec2 vtop=vec2(topx,topy);
    vec2 vbot=vec2(botx,boty);
 
    vec2 top_left=vtop*(1+top_offset);
    vec2 top_right=vtop*(1-top_offset);
    vec2 bot_left=vbot*(1+bot_offset);
    vec2 bot_right=vbot*(1-bot_offset);
    
    vec2 v2=normalize(vec2(topx,topy));
    vec2 v1=normalize(vec2(botx,boty));
  
    Pnorm.xyz=vec3(v1,0);
    gl_TexCoord[0].xy=vec2(0,tx.w);
    gl_Position = vec4(p1.xy-bot_left,p1.z,1);  // bot-left 
    EmitVertex();
    
    Pnorm.xyz=vec3(-v1,0);
    gl_TexCoord[0].xy=vec2(1,tx.w);
    gl_Position = vec4(p1.xy+bot_right,p1.z,1);  // bot-right 
    EmitVertex(); 
   
 	Pnorm.xyz=vec3(v2,0);
    gl_TexCoord[0].xy=vec2(0,tx.y);
    gl_Position = vec4(p2.xy-top_left,p2.z,1); // top-left
    EmitVertex();
   
    Pnorm.xyz=vec3(-v2,0);
    gl_TexCoord[0].xy=vec2(1,tx.y);
    gl_Position = vec4(p2.xy+top_right,p2.z,1); // top-right
    EmitVertex();
    EndPrimitive();
}

vec4 calcOffsets(vec4 p0,vec4 p1,vec4 p2, vec4 c){
	
	float w2=c.g;     // top width
	float w1=c.r;     // bottom width
     
    vec3 v1=p1.xyz-p0.xyz;
    vec3 v2=p2.xyz-p1.xyz;
    
    float a1 = atan2(v1.y, v1.x);       
	float a2 = atan2(v2.y, v2.x);
	
    float x1 = -sin(a1);
	float y1 = cos(a1);
    float x2 = -sin(a2);
	float y2 = cos(a2);
     	
	float botx = x1 * w1;
	float boty = y1 * w1;	
	float topx = x2 * w2;
	float topy = y2 * w2;

	vec4 cc=vec4(topx,topy,botx,boty);
	return cc;
}

// draw a branch as a polygon
void emitBranch(){
   Pnorm.w=0.025;

   vec4 p1=gl_PositionIn[0];
   vec4 p2=gl_PositionIn[1];
   vec4 c=Constants1[0];
   vec4 p0=Constants2[0];
   vec4 cc=calcOffsets(p0,p1,p2,c);
   emitRectangle(p1,p2,cc,vec4(0,0,0,1));
 }

mat3 m=mat3(2,-3,1,-4,4,0,2,-1,0);
vec4 spline(float x, vec4 p0, vec4 p1, vec4 p2){
  vec4 c=p0;
  vec4 b=p1*4.0-p0*3.0-p2;
  vec4 a=p2*2.0+p0*2.0-p1*4.0;
  return a*x*x+b*x+c;
}

// draw a branch as a spline
void emitSpline(){
    Pnorm.w=0.025;
   
    vec4 p1=gl_PositionIn[0];
    vec4 p2=gl_PositionIn[1];       
    vec4 p0=Constants2[0];
 
    vec4 cc=calcOffsets(p0,p1,p2,Constants1[0]);
    
    float topx=cc.r;
	float topy=cc.g;
	float botx=cc.b;
	float boty=cc.a;
	
	int nv=8;
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
		vec4 c=vec4(xy2, xy1);
		vec4 t1=spline(s,p0,p1,p2);
		vec4 t2=spline(s+ds,p0,p1,p2);
		vec4 tx=vec4(0,f1,0,f2);
		emitRectangle(t1,t2,c,tx);	
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
    else if(mode==LEAF)
    	emitLeaf();
    else if(mode==BRANCH)
    	emitBranch();
    else
        emitSpline(); 
}

