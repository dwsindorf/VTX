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

#define PI		3.14159265359

#define LINE   0
#define BRANCH 1
#define LEAF   2

vec4 project(vec4 pnt){
	vec4 vertex=vec4(pnt.xyz,1.0);
	vec4 proj=gl_ModelViewProjectionMatrix * vertex;
	return vec4(proj.xyz/proj.w,pnt.w);
}

void produceVertex(vec4 p){
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
    produceVertex(vec4(Pos2.xyz,1.0));
    EmitVertex();
    
    produceVertex(vec4(Pos1.xyz,1.0));
 }

// draw a leaf
void emitLeaf(){
	float w=TexVars_G[0].r;
	
    float ps=Constants1[0].g; // size
    vec4 v=normalize(Pos2-Pos1); 
    vec4 pa=Pos1+ps*v;// end
    
    float a = atan2(v.y, v.x);       
    float ta = a-PI/2;
    
	float cc=cos(ta);
	float ss=sin(ta);    	   		   
	mat2  M=ps*mat2(cc,ss,-ss,cc);
	             
	produceVertex(vec4(pa.xy+M*vec2(0,-1),pa.z,1));   // bottom          
	produceVertex(vec4(pa.xy+M*vec2(0.9*w,-0.8),pa.z,1));
	produceVertex(vec4(pa.xy+M*vec2(-0.9*w,-0.8),pa.z,1));   
	produceVertex(vec4(pa.xy+M*vec2(w,-0.5),pa.z,1));  
	produceVertex(vec4(pa.xy+M*vec2(-w,-0.5),pa.z,1)); 
	produceVertex(vec4(pa.xy+M*vec2(0.7*w,-0.25),pa.z,1));    
	produceVertex(vec4(pa.xy+M*vec2(-0.7*w,-0.25),pa.z,1));   
	produceVertex(vec4(pa.xy,pa.z,1));   // top
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
 
void drawCone(vec4 pnt0, vec4 pnt1, vec4 pnt2, vec4 c)
{
   vec4 p1,p2,proj;
   float scale=6e-7;
   float r1=scale*c.x;
   float r2=scale*c.y;

   vec3 axis1 = pnt1.xyz - pnt0.xyz;
   vec3 axis2 = pnt2.xyz - pnt1.xyz;

   vec3 tx1 = createPerp( pnt1.xyz, pnt0.xyz );
   vec3 ty1 = cross( normalize(axis1), tx1 );

   vec3 tx2 = createPerp( pnt2.xyz, pnt1.xyz );
   vec3 ty2 = cross( normalize(axis2), tx2 );
   
   int segs = 8;

   float f=1.0 /(segs-1);
   float delta=1.0/segs;
   
   for(int i=0; i<segs; i++) {
      float a = i*f;
      float ca = cos(2.0 * PI*a); 
      float sa = sin(2.0 * PI*a);
      vec3 n1 = vec3( ca*tx1.x + sa*ty1.x,
                     ca*tx1.y + sa*ty1.y,
                     ca*tx1.z + sa*ty1.z );
      
      vec3 n2 = vec3( ca*tx2.x + sa*ty2.x,
                     ca*tx2.y + sa*ty2.y,
                     ca*tx2.z + sa*ty2.z );
   
      p1.xyz = pnt1.xyz+r1*n1;
      p1.w=1;     
      produceVertex(p1);

      p2.xyz = pnt2.xyz + r2*n2;
      p2.w=1;
      produceVertex(p2);

   }
}

void emitBranch3d(){
   vec4 c=Constants1[0];
   vec4 p0=P0[0];
   vec4 p1=gl_PositionIn[0];
   vec4 p2=gl_PositionIn[1];
   vec4 d;
   d.x=c.r;
   d.y=c.g;
   d.z=c.b;
   d.w=c.w;
   drawCone(p0,p1,p2,d);
}
void main(void) {
    Pos0=project(P0[0]);
    Pos1=project(gl_PositionIn[0]);
    Pos2=project(gl_PositionIn[1]);
    if(length(Pos2-Pos1)>2)
    	return;
    int mode=TexVars_G[0].w+0.1;    
    if(mode==LINE)
    	emitLine();
    else if(mode==LEAF)
    	emitLeaf();
 	else
 		emitBranch3d(); 
}


