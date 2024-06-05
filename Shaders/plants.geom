#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

varying in vec4 Color_G[];
varying in vec4 Normal_G[];
varying in vec4 Constants1[];
varying in vec4 P0[];
varying in vec4 TexVars_G[];

varying out vec4 Color;
varying out vec4 TexVars;
varying out vec3 Normal;
varying out vec4 Pnorm;

vec4 Pos0,Pos1,Pos2;

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

#define LINE   0
#define BRANCH 1
#define LEAF   2
#define SPLINE 3
#define THREED 4

// draw a line
void emitLine(){
 	float nscale=1e-5;//TexVars.r;
 
    // need at least 3 vertexes for line strip
    Pnorm.xyz=gl_NormalMatrix*vec3(nscale,0,0);
    Pnorm.w=0;
    gl_Position.xyz = Pos2.xyz; // top
    gl_Position.w=1;
    EmitVertex();
    EmitVertex();
    
    gl_Position.xyz = Pos1.xyz; // top
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
	
    vec3 vw=vec3(topx,topy,Pos2.z);   
    float ps=Constants1[0].g; // size
    vec4 v=normalize(Pos2-Pos1); 
    vec4 pa=Pos1+ps*v;// end
    
    float a = atan2(v.y, v.x);       
    float ta = a-PI/2;
    
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
    //EndPrimitive();
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

   vec4 p1=Pos1;
   vec4 p2=Pos2;
   vec4 c=Constants1[0];
   vec4 p0=Pos0;
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

void drawCone(vec4 pnt0, vec4 pnt1, vec4 pnt2, vec4 c)
{
   vec4 p1,p2,proj;

   float r1=5e-9*c.x;
   float r2=5e-9*c.y;
   float t1=c.z;
   float t2=c.w;
   Pnorm.w=0.02;
   vec3 axis1 = pnt1.xyz - pnt0.xyz;
   vec3 axis2 = pnt2.xyz - pnt1.xyz;

   vec3 tx1 = createPerp( pnt1.xyz, pnt0.xyz );
   vec3 ty1 = cross( normalize(axis1), tx1 );

   vec3 tx2 = createPerp( pnt2.xyz, pnt1.xyz );
   vec3 ty2 = cross( normalize(axis2), tx2 );
   int segs = 8;
   float f=1.0 /(segs-1);
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
      Pnorm.xyz=n1.xyz;
      
      gl_TexCoord[0].xy=vec2(a,0);

      p1.xyz = pnt1.xyz+r1*n1;
      proj = gl_ModelViewProjectionMatrix * vec4(p1.xyz, 1.0);
      gl_Position =vec4(proj.xyz/proj.w,1);
      //gl_Position = project(p1);
      EmitVertex();
      
      gl_TexCoord[0].xy=vec2(a,1);
      Pnorm.xyz=n2.xyz;
      p2.xyz = pnt2.xyz + r2*n2;
      proj = gl_ModelViewProjectionMatrix * vec4(p2.xyz, 1.0);
      gl_Position = vec4(proj.xyz/proj.w,1);
      //gl_Position = proj;
      //gl_Position = project(p2);
      EmitVertex();
        
   }
   EndPrimitive();   
   
}
// draw a branch as a spline

void emit3dSpline(){

	vec4 c=Constants1[0];
	vec4 p0=P0[0];
	vec4 p1=gl_PositionIn[0];
	vec4 p2=gl_PositionIn[1];
	float r1=c.r;
	float r2=c.g;
 	vec4 d;

	int nv=2;
	float ds=0.5/nv;
	float s=0.5;
    vec4 s0,s1,s2;
    s0=p0;
	float delta=1.0/nv;
	for(int i=0;i<nv;i++){
		float f1=i*delta;
		float f2=(i+1)*delta;
		d.x=(1-f1)*r1+f1*r2;
		d.y=(1-f2)*r1+f2*r2;
		d.z=f1;
		d.w=f2;
		
		s1=spline(s,p0,p1,p2);
		s2=spline(s+ds,p0,p1,p2);
		drawCone(s0,s1,s2,d);
		s0=s1;
		s+=ds;
	}	
}

void emit3d(){
   Pnorm.w=0.025;
   vec4 c=Constants1[0];
   vec4 p0=P0[0];
   vec4 p1=gl_PositionIn[0];
   vec4 p2=gl_PositionIn[1];
   vec4 d;
   d.x=c.r;
   d.y=c.g;
   d.z=0;
   d.w=1;
   drawCone(p0,p1,p2,d);
   //drawCone(p1,p2,1e-9,1e-9);
   
}
void main(void) {
    Pos0=project(P0[0]);
    Pos1=project(gl_PositionIn[0]);
    Pos2=project(gl_PositionIn[1]);
    if(length(Pos2-Pos1)>2)
    	return;
 	Color=Color_G[0];
	Normal.xyz=Normal_G[0].xyz;
	TexVars=TexVars_G[0];
    int mode=TexVars_G[0].w+0.1;
    
    if(mode==LINE)
    	emitLine();
    else if(mode==LEAF)
    	emitLeaf();
#ifdef TEST
    else if(mode==BRANCH)
    	emit3d();
    else
    	emit3dSpline();
#else
    else if(mode==BRANCH)
        emitBranch(); 
    else
        emitSpline(); 
#endif
}

