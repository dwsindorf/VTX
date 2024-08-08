#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

#ifdef SHADOWS
varying vec4 ShadowCoord;
varying vec4 EyeDirection;
uniform mat4 smat;
#endif
varying in vec4 Constants1[];
varying in vec4 TexVars_G[];

varying in vec4 P0[];

vec4 Pos0,Pos1,Pos2;

float scale=6e-7;
 
#define PI		3.14159265359

#define LINE   0
#define BRANCH 1
#define LEAF   2

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
  vec3 ret = cross( invec, vec3(0.0, 0.0, 1.0) );
  if ( length(ret) == 0.0 )
     ret = cross( invec, vec3(0.0, 1.0, 0.0) );
  return ret;
}

void produceVertex(vec3 v){
   vec4 p=vec4(v,1.0);
   gl_Position = project(p);
#ifdef SHADOWS
   vec4 VertexPosition=(gl_ModelViewMatrix * p);
   ShadowCoord=smat * VertexPosition;
   EyeDirection=-(gl_ModelViewMatrix * p);
#endif   
   EmitVertex();  
}


// draw a line
void emitLine(){
    produceVertex(Pos2);
    produceVertex(Pos1); 
    produceVertex(Pos2);
}
void emitCone()
{
   vec4 c=Constants1[0];
   vec3 p1,p2;
   float r1=scale*c.x;
   float r2=scale*c.y;

   vec3 axis1 = Pos1.xyz - Pos0.xyz;
   vec3 axis2 = Pos2.xyz - Pos1.xyz;

   vec3 tx1 = createPerp(Pos1.xyz, Pos0.xyz);
   vec3 ty1 = cross(normalize(axis1), tx1 );

   vec3 tx2 = createPerp(Pos2.xyz, Pos1.xyz);
   vec3 ty2 = cross(normalize(axis2), tx2 );
   
   int segs = 4;

   float f=1.0 /(segs-1);
   float delta=1.0/segs;
   
   for(int i=0; i<segs; i++) {
      float a = i*f;
      float ca = cos(2.0 * PI*a); 
      float sa = sin(2.0 * PI*a);
      vec3 n1 = vec3(ca*tx1.x + sa*ty1.x,
                     ca*tx1.y + sa*ty1.y,
                     ca*tx1.z + sa*ty1.z );
      
      vec3 n2 = vec3(ca*tx2.x + sa*ty2.x,
                     ca*tx2.y + sa*ty2.y,
                     ca*tx2.z + sa*ty2.z );
   
      p1 = Pos1+r1*n1;   
      produceVertex(p1);

      p2 = Pos2 + r2*n2;
      produceVertex(p2);
   }
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

void emitLeaf(){
	Pos1=gl_PositionIn[0];
	Pos2=gl_PositionIn[1];

	vec3 v=normalize(Pos2.xyz-Pos1.xyz);   
	float ps=Constants1[0].w; // size
	Pos2=vec4(Pos1.xyz+ps*v,1);
 
	vec3 tx2 = cross(v, normalize(Pos2.xyz) ); // perpendicular to eye direction
	vec3 tx1 = cross(v, normalize(Pos1.xyz) );
    
    float w=ps*Constants1[0].z; // width_ratio
    
    float taper=Constants1[0].x;
    float compression=Constants1[0].y;
    
    float w1=compression*w;
    float w2=taper*w1;
    
    float b1=0.1;
    float b2=0.9;

	w1=w1*1.5;
	w2=w2*1.5;

    int nodes=6;
    vec4 t1=vec4(tx1,1.0);
    vec4 t2=vec4(tx2,1.0);
    vec4 x1=mix(Pos1,Pos2,b1);
    vec4 x2=mix(Pos1,Pos2,b2);
    vec4 s1p=x1+w1*t1;
    vec4 s1m=x1-w1*t1;
    vec4 s2p=x2+w2*t2;
    vec4 s2m=x2-w2*t2;
    
    float dt=1.0/nodes;
    float t=dt;

    produceVertex(Pos1); // bot
    for(int i=0;i<nodes-1;i++){
      produceVertex(bezier(t,Pos1,s1p,s2p,Pos2));
      produceVertex(bezier(t,Pos1,s1m,s2m,Pos2));
      t+=dt;
    }
    produceVertex(Pos2); // top      
 }
 


void main(void) {
    Pos0=P0[0];
    Pos1=gl_PositionIn[0];
    Pos2=gl_PositionIn[1];
    //if(length(project(Pos2)-project(Pos1))>2)
    //	return;
    int mode=TexVars_G[0].w+0.1;    
    if(mode==LINE)
    	emitLine();
    else if(mode==LEAF)
    	emitLeaf();
 	else
 		emitCone(); 
}


