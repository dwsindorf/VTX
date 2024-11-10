#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

#include "utils.h"

varying in vec4 Color_G[];
varying in vec4 Normal_G[];
varying in vec4 Constants1[];
varying in vec4 ImageVars_G[];
varying in vec4 P0[];
varying in vec4 TexVars_G[];

varying out vec4 Color;
varying out vec4 TexVars;
varying out vec4 ImageVars;
varying out vec3 Normal;
varying out vec4 Pnorm;

vec4 Pos0,Pos1,Pos2;

#define PI		3.14159265359

#define LINE   0
#define BRANCH 1
#define LEAF   2
#define SPLINE 3

vec4 project(vec4 pnt){
	vec4 vertex=vec4(pnt.xyz,1.0);
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	return vec4(proj.xyz/proj.w,pnt.w);
}

// given two points p1 and p2 create a vector out
// that is perpendicular to (p2-p1)
vec3 createPerp(vec3 p1, vec3 p2)
{
  vec3 invec = normalize(p2 - p1);
  vec3 ret = cross( invec, vec3(1.0, 0.0, 0.0) );
  if ( length(ret) == 0.0 )
     ret = cross( invec, vec3(0.0, 1.0, 0.0) );
  return ret;
}

vec3 OrthoNormalVector(vec3 v) {
  float g = v.z>0?1.0:-1.0;
  float h = v.z + g;
  return vec3(g - v.x*v.x/h, -v.x*v.y/h, -v.x);
}

void projectVertex(vec3 v){
   vec4 p=vec4(v,1.0);
   gl_Position = project(p);
   EmitVertex();  
}

void emitVertex(vec3 v){
   vec4 p=vec4(v,1.0);
   gl_Position = p;
   EmitVertex();  
}
void produceVertex(vec3 v){
   vec4 p=vec4(v,1.0);
   gl_Position = project(p);
   EmitVertex();  
}

void produceTxVertex(vec2 tx,vec3 v){
  gl_TexCoord[0].xy=tx;
  emitVertex(v);
}

void produceTVertex(vec2 tx,vec3 v){
  gl_TexCoord[0].xy=tx;
  produceVertex(v);
}
// draw a line
void emitLine(){
    emitVertex(Pos2);
    emitVertex(Pos1); 
    emitVertex(Pos2);
 }
#define MAT_MUL
vec4 bezier(float t, vec4 P0, vec4 P1, vec4 P2, vec4 P3){
#ifdef MAT_MUL
   mat4 M=mat4(1,0,0,0,
               -3,3,0,0,
               3,-6,3,0,
               -1,3,-3,1
               );               
   mat4 P=mat4(P0,P1,P2,P3);
   vec4 X=vec4(1,t,t*t,t*t*t);
   mat4 MP=mul(P,M);
   return (MP*X);
#else
   return (P0 + (P1*3 -3*P0)*t + (P2*3 -6*P1 +3*P0)*t*t + (P3 -3*P2 +3*P1 -P0)*t*t*t);
#endif
}


// draw a leaf
void drawLeaf(vec3 p0,vec3 p1, vec3 p2)
{   
	int colmode=TexVars_G[0].g+0.1; // transparency flag
	int rectmode=colmode & 4;
	float g=Constants1[0].z;
	vec3 v1=normalize(p1-p0);
	vec3 v2=normalize(p2-p1);
	vec3 eye=normalize(p1);
	vec3 v=(1-g)*v1+g*eye;

    vec3 tx=normalize(cross(v, v2));

    tx=tx-p1;
 
	vec4 Pos1=vec4(p1,0);
 	vec4 Pos2=vec4(p2,0);
    float w=Constants1[0].y;

	Pnorm.xyz=normalize(cross(v, tx ));
	Pnorm.w=0.01;//P0[0].w; // bump
     vec3 ty = cross(v, tx);
	if(rectmode){ // use a rectangle (for transparent textures){
		produceTVertex(vec2(0.0,0.0),Pos1-w*tx); // bot-left
		produceTVertex(vec2(1.0,0.0),Pos1+w*tx); // bot-right
		produceTVertex(vec2(0.0,1.0),Pos2-w*tx); // top-left
		produceTVertex(vec2(1.0,1.0),Pos2+w*tx); // top-right
	}
	else{ // leaf shape
	    float taper=Constants1[0].x;
	    	    	
	    float w1=w;
        float w2=taper*w;
         
        float b1=0.1;
        float b2=0.9;

		w1=w1*1.5;
		w2=w2*1.5;
        float s=w1*1.4;
        
        int nodes=6;
        vec4 t1=vec4(tx,1);
        vec4 x1=mix(Pos1,Pos2,b1);
        vec4 x2=mix(Pos1,Pos2,b2);
        vec4 s1p=x1+w1*t1;
        vec4 s1m=x1-w1*t1;
        vec4 s2p=x2+w2*t1;
        vec4 s2m=x2-w2*t1;
        
        float dt=1.0/nodes;
        float t=dt;
//#define TEST3D
#ifdef TEST3D
        int segs = 3;
        nodes=5;
	    float f=1.0 /(segs-1);
        
		tx = createPerp(p2, p1);
		ty = cross(v2, tx);
         float ws=0.5;
        f=1.0 /(segs);
  		for(int i=0; i<segs; i++) {
	      float a = i*f;
	      float f1=w1;
	      float f2=w2;
	      //Color=vec4(a,0,0,1);
	      float ca = cos(2*PI*a);
	      f1=lerp(abs(ca),0.0,1.0,w1,ws*w1);
	      f2=lerp(abs(ca),0.0,1.0,w2,ws*w2);
	      float sa = sin(2*PI*a);
	      vec3 na1 = vec3(ca*tx.x + sa*ty.x,ca*tx.y + sa*ty.y,ca*tx.z + sa*ty.z);
	      vec4 nx=vec4(na1,1);
	      s1p=x1+f1*nx;
          s2p=x2+f2*nx;
          t=0;
          // rotate curve one step
          a = (i+1)*f;
          ca = cos(2*PI*a); 
	      sa = sin(2*PI*a);
	      na1 = vec3(ca*tx.x + sa*ty.x,ca*tx.y + sa*ty.y,ca*tx.z + sa*ty.z);
	      nx=vec4(na1,1);
	      f1=lerp(abs(ca),0.0,1.0,w1,ws*w1);
	      f2=lerp(abs(ca),0.0,1.0,w2,ws*w2);
	      
          s1m=x1+f1*nx;
	      s2m=x2+f2*nx;
 	                 	                     
	      produceTVertex(vec2(0.5,0.0),Pos1); // bot
	      for(int i=0;i<nodes-1;i++){
	          vec4 p=bezier(t,Pos1,s1p,s2p,Pos2);
	          produceTVertex(vec2(p.w/s+0.5,t),p);
	          p=bezier(t,Pos1,s1m,s2m,Pos2);
	          produceTVertex(vec2(p.w/s+0.5,t),p);
	          t+=dt;
	      }
	      produceTVertex(vec2(0.5,1.0),Pos2); // top
	      EndPrimitive();
      }
 #else    
      produceTVertex(vec2(0.5,0.0),Pos1); // bot
      for(int i=0;i<nodes-1;i++){
          vec4 p=bezier(t,Pos1,s1p,s2p,Pos2);
          produceTVertex(vec2(p.w/s+0.5,t),p);
          p=bezier(t,Pos1,s1m,s2m,Pos2);
          produceTVertex(vec2(p.w/s+0.5,t),p);
          t+=dt;
      }
      produceTVertex(vec2(0.5,1.0),Pos2); // top
#endif 
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

}

// 2d only
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

// draw a branch as a polygon (2d)
void emitBranch(){
   Pnorm.w=0.01;

   vec4 p1=Pos1;
   vec4 p2=Pos2;
   vec4 c=Constants1[0];
   vec4 p0=Pos0;
   vec4 cc=calcOffsets(p0,p1,p2,c);
   emitRectangle(p1,p2,cc,vec4(0,0,0,1));
 }

//mat3 m=mat3(2,-3,1,-4,4,0,2,-1,0);
vec4 spline(float x, vec4 p0, vec4 p1, vec4 p2){
  vec4 c=p0;
  vec4 b=p1*4.0-p0*3.0-p2;
  vec4 a=p2*2.0+p0*2.0-p1*4.0;
  return a*x*x+b*x+c;
}

// draw a branch as a spline (2d only)
void emitSpline(){
 
    Pnorm.w=0.01;  
    vec4 p1=Pos1;
    vec4 p2=Pos2;       
    vec4 p0=Pos0;
 
    vec4 cc=calcOffsets(p0,p1,p2,Constants1[0]);
    
    float topx=cc.r;
	float topy=cc.g;
	float botx=cc.b;
	float boty=cc.a;
	
	int nv=4;
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

// 3d only
void drawCone(vec3 p0, vec3 p1, vec3 p2)
{
   vec4 c=Constants1[0];
   
   vec4 pt1,pt2;

   float r1=c.x;
   float r2=c.y;
   float t1=c.z;
   float t2=c.w;
   Pnorm.w=0.002;//P0[0].w;
   vec3 v1 = normalize(p1 - p0);
   vec3 v2 = normalize(p2 - p1);

	vec3 tx1=OrthoNormalVector(v1);
   //vec3 tx1 = createPerp(p1, p0);
   vec3 ty1 = cross(v1, tx1);

   vec3 tx2 = OrthoNormalVector(v2);
   //   vec3 tx2 = createPerp(p2, p1);
   
   vec3 ty2 = cross(v2, tx2);
   int dim=ImageVars_G[0].x*ImageVars_G[0].y+0.1;
   float scale=dim>1?1.0:2.0;
   
   int segs = 8;

   float f=1.0 /(segs-1);
   float p=P0[0].w;//-0.35;
   for(int i=0; i<segs; i++) {
      float a = i*f;
      float ca = cos(2.0 * PI*(a+p)); 
      float sa = sin(2.0 * PI*(a+p));
      vec3 n1 = vec3(ca*tx1.x + sa*ty1.x,
                     ca*tx1.y + sa*ty1.y,
                     ca*tx1.z + sa*ty1.z);
      
      vec3 n2 = vec3(ca*tx2.x + sa*ty2.x,
                     ca*tx2.y + sa*ty2.y,
                     ca*tx2.z + sa*ty2.z);
      Pnorm.xyz=-n1;
      
      gl_TexCoord[0].xy=vec2(scale*a,t1);

      pt1.xyz = p1+r1*n1;
      pt1.w=1;
 
      gl_Position = project(pt1);
      EmitVertex();
      
      gl_TexCoord[0].xy=vec2(scale*a,t2);
      Pnorm.xyz=-n2.xyz;
      pt2.xyz = p2 + r2*n2;
      pt2.w=1;
      gl_Position = project(pt2);
      EmitVertex();     
   }
}

void emitBranch3d(){
   vec3 p0=P0[0].xyz;
   vec3 p1=gl_PositionIn[0].xyz;
   vec3 p2=gl_PositionIn[1].xyz;
   drawCone(p0,p1,p2);
}

void emitLeaf(){
   vec3 p0=P0[0].xyz;
   vec3 p1=gl_PositionIn[0].xyz;
   vec3 p2=gl_PositionIn[1].xyz;
   drawLeaf(p0,p1,p2);
}

void main(void) {
    Pos0=project(P0[0]);
    Pos1=project(gl_PositionIn[0]);
    Pos2=project(gl_PositionIn[1]);
    
    if(length(Pos2)>2)
       return;
    if(length(Pos1)>2)
       return;   

 	Color=Color_G[0];
	Normal.xyz=Normal_G[0].xyz;
	TexVars=TexVars_G[0];
	ImageVars=ImageVars_G[0];
    int mode=TexVars_G[0].w+0.1;
    
    if(mode==LINE)
    	emitLine();
    else if(mode==LEAF)
    	emitLeaf();
#ifdef ENABLE_3D
 	else
 		emitBranch3d(); 
#else
    else if(mode==BRANCH)
        emitBranch(); 
    else
        emitSpline(); 
#endif
}

