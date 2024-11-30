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

vec3 OrthoNormalVector(vec3 v) {
  float g = v.z>0?1.0:-1.0;
  float h = v.z + g;
  return normalize(vec3(g - v.x*v.x/h, -v.x*v.y/h, -v.x));
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
	float g=Constants1[0].z; //flatness
	vec3 v1=normalize(p1-p0); // branch vector
	vec3 v2=normalize(p2-p1); // leaf vector
	vec3 eye=normalize(p1);
	vec3 v=(1-g)*v1+g*eye;

    vec3 tx=normalize(cross(v, v2));

    tx=tx-p1;
 
	vec4 Pos1=vec4(p1,0);
 	vec4 Pos2=vec4(p2,0);
    float w=Constants1[0].y;

	Pnorm.xyz=-normalize(cross(v, tx ));

	Pnorm.w=0.001;//P0[0].w; // bump
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

       float s=w1*1.5;
		//w1=w1*1.5;
		//w2=w2*1.5;
         
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

      produceTVertex(vec2(0.5,0.0),Pos1); // bot
      for(int i=0;i<nodes-1;i++){
          vec4 p=bezier(t,Pos1,s1p,s2p,Pos2);
          produceTVertex(vec2(p.w/s+0.5,t),p);
          p=bezier(t,Pos1,s1m,s2m,Pos2);
          produceTVertex(vec2(p.w/s+0.5,t),p);
          t+=dt;
      }
      produceTVertex(vec2(0.5,1.0),Pos2); // top
	}
	EndPrimitive();
}

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

   vec3 tx1 = OrthoNormalVector(v1);
   vec3 ty1 = cross(v1, tx1);
   vec3 tx2 = OrthoNormalVector(v2);
   
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
      //Pnorm.xyz=-n2;
      pt2.xyz = p2 + r2*n2;
      pt2.w=1;
      gl_Position = project(pt2);
      EmitVertex();     
   }
}

void emitBranch(){
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

// draw a line
void emitLine(){
    emitVertex(Pos2);
    emitVertex(Pos1); 
    emitVertex(Pos2);
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
 	else
 		emitBranch(); 
}

