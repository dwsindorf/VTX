
// ########## begin noise_funcs.h #########################
uniform sampler3D noise3DTexture;
uniform float fmax;

//vec3 v1;
vec3 v3;
vec3 df;

#ifndef PI
#define PI		3.14159265359
#endif
#define GRADIENT 1
#define VORONOI  2
#define SIMPLEX  3

#ifdef N3D
// classic Perlin noise using a 3D texture 
#define FSCALE 0.03125
#ifdef VNOISE
vec4 noise3D(vec3 p) {
	return (texture3D (noise3DTexture, FSCALE*p) - 0.5); 
}
#else
float noise3D(vec3 p) {
	return 2.0 * (texture3D (noise3DTexture, FSCALE*p ).r - 0.5); 
}
#endif
#else
#define FSCALE 1.0
#include "noise.h"
#endif

#define GCURVE1(t) (0.5*t*t*(3.0-t))
#define DCURVE1(t) 1.5*t*(2.0-t)
#define SCURVE1(t) (t*t*(3.0 - 2.0*t) )

struct noise_info {
	float fact;
	float delta;
	float orders;
	float freq;
	float H;
	float L;
	float bias;
	float ampl;
	float offset;
	float smoothing;
	float clamp;
	float logf;
	float scale;
	float seed;
	bool sqr;
	bool invert;
	bool absval;
	bool uns;
	bool animate;

	int vnoise;
	
};

uniform float rseed;

uniform noise_info nvars[NVALS];

#define VMAX 0.5

float noise_fade=0.0;
float last_seed=0;
float current_seed=0;

#ifdef NOISE_DP
// ---- double precision noise point (see Noise()) ----
// dp_on false: the point is DP_BASE+(v1-Vertex1) (unwarped v1, or v1 offset
//              by a small delta for bump normals)
// dp_on true:  a warp (twist, storms, vortex) has set the point in vdp
// warps compute their angles and rotations in double so the warped point
// keeps the full precision. GLSL has no double sin/cos: dsincos() below
#define DP_BASE (dvec3(VertexO)+dvec3(Vertex2.xyz))
dvec3 vdp=dvec3(0.0LF);
bool dp_on=false;
dvec3 dp_point(){ return dp_on?vdp:DP_BASE+dvec3(v1-Vertex1.xyz); }
void dp_set(dvec3 p){ vdp=p; dp_on=true; v1=vec3(p); }
#define DPI     3.14159265358979323846LF
#define DPI_2   1.57079632679489661923LF
// double sin and cos: quadrant reduction + Taylor series (|r|<=pi/4, error ~1e-14)
void dsincos(double x, out double s, out double c){
	double q=floor(x/DPI_2+0.5LF);
	double r=x-q*DPI_2;
	double r2=r*r;
	double sr=r*(1.0LF+r2*(-1.0LF/6.0LF+r2*(1.0LF/120.0LF+r2*(-1.0LF/5040.0LF
			+r2*(1.0LF/362880.0LF+r2*(-1.0LF/39916800.0LF+r2*(1.0LF/6227020800.0LF)))))));
	double cr=1.0LF+r2*(-0.5LF+r2*(1.0LF/24.0LF+r2*(-1.0LF/720.0LF+r2*(1.0LF/40320.0LF
			+r2*(-1.0LF/3628800.0LF+r2*(1.0LF/479001600.0LF+r2*(-1.0LF/87178291200.0LF)))))));
	int iq=int(mod(q,4.0LF));
	if(iq==0){ s=sr;  c=cr;  }
	else if(iq==1){ s=cr;  c=-sr; }
	else if(iq==2){ s=-sr; c=-cr; }
	else { s=-cr; c=sr; }
}
#endif

float reset(){
	v1=Vertex1.xyz;
#ifdef NOISE_DP
	dp_on=false;
#endif
	return 0.0;
}
#ifdef NOISE_DP
float corialis(float a){
	dvec3 vt=(dp_point()-0.5LF)*2.0LF;
	double x=vt.x;
	double y=vt.z;
	double z=vt.y;
	double r=x*x+y*y;
	double s,c;
	dsincos(double(a)*2.0LF*DPI*r,s,c);
	vt=dvec3(x*c-y*s,x*s+y*c,z);
	dp_set(vt*0.5LF+0.5LF);
	return 0.0;
}
#else
float corialis(float a){
    vec3 vt=(v1-0.5)*2.0;
    
	float x=vt.x;
	float y=vt.z;
	float z=vt.y;
	float r=(x*x+y*y);
	float angle=a*2.0*PI*r;
	float s=sin(angle);
	float c=cos(angle);
	vt=vec3(x*c-y*s,x*s+y*c,z);
	v1=vt*0.5+0.5;
	return 0.0;
}
#endif

#define twist(a,b) (corialis(a)+b+reset())

// ---- storms(k,size,expr): wind the noise point around storm centers ----
// (StormTable in NoiseClass.h; uniforms set in TerrainData.cpp).
// Rotation about each center c by k*storm_s[i]*smoothstep(f),
// f=1-d/(size*R) inside the storm radius. TNstorms::valueString always
// emits the 3 argument form. v1 is restored afterwards (so storms() can be nested
// around twist(), but not inside it: twist's reset() would undo it).
// Note: like twist(), only v1 is warped (v2 is the close-range coordinate).
#ifndef MAX_STORMS
#define MAX_STORMS 24
#endif
uniform vec4  storm_c[MAX_STORMS];   // xyz center (unit), w radius (chord)
uniform float storm_s[MAX_STORMS];   // signed strength (radians)
uniform int   storm_n;
vec3 storm_save;
#ifdef NOISE_DP
dvec3 storm_save_d;
bool  storm_save_on;
float storm_push(){ storm_save=v1; storm_save_d=vdp; storm_save_on=dp_on; return 0.0; }
float storm_pop(){ v1=storm_save; vdp=storm_save_d; dp_on=storm_save_on; return 0.0; }
float stormwarp(float k, float size){
	dvec3 vt=(dp_point()-0.5LF)*2.0LF;
	double l=length(vt);
	if(l<=0.0LF || k==0.0 || size<=0.0)
		return 0.0;
	for(int i=0;i<MAX_STORMS;i++){
		if(i>=storm_n)
			break;
		dvec3 c=dvec3(storm_c[i].xyz);
		double R=double(storm_c[i].w)*double(size);
		double d=length(vt/l-c);
		if(d<R){
			double f=1.0LF-d/R;
			double a=double(k)*double(storm_s[i])*f*f*(3.0LF-2.0LF*f);
			double sn,cs;
			dsincos(a,sn,cs);
			vt=vt*cs+cross(c,vt)*sn+c*dot(c,vt)*(1.0LF-cs);
		}
	}
	dp_set(vt*0.5LF+0.5LF);
	return 0.0;
}
#else
float storm_push(){ storm_save=v1; return 0.0; }
float storm_pop(){ v1=storm_save; return 0.0; }
float stormwarp(float k, float size){
	vec3 vt=(v1-0.5)*2.0;
	float l=length(vt);
	if(l<=0.0 || k==0.0 || size<=0.0)
		return 0.0;
	for(int i=0;i<MAX_STORMS;i++){
		if(i>=storm_n)
			break;
		vec3 c=storm_c[i].xyz;
		float R=storm_c[i].w*size;
		float d=length(vt/l-c);
		if(d<R){
			float f=1.0-d/R;
			float a=k*storm_s[i]*f*f*(3.0-2.0*f);
			float cs=cos(a);
			float sn=sin(a);
			vt=vt*cs+cross(c,vt)*sn+c*dot(c,vt)*(1.0-cs);
		}
	}
	v1=vt*0.5+0.5;
	return 0.0;
}
#endif
#define storms(k,s,b) (storm_push()+stormwarp(k,s)+(b)+storm_pop())

// ---- vortex(k,size,expr), spots(size): gas giant vortices ----
// (VortexTable in NoiseClass.h; uniforms set in TerrainData.cpp)
// vortex: rotate v1 within east-west ellipses (closed ovals), like storms()
// spots:  sum of oval tones (1 over the core, falling to 0 over the outer
//         20%: a sharp rim, so the shift doesn't sweep through several
//         band colors as rings), unwarped point
#define MAX_VORTS 16
uniform vec4 vort_c[MAX_VORTS];   // xyz center (unit), w north-south radius
uniform vec4 vort_e[MAX_VORTS];   // xyz local east (unit), w east-west aspect
uniform vec4 vort_p[MAX_VORTS];   // x signed strength, y spot tone
uniform int  vort_n;
vec3 vort_save;
#ifdef NOISE_DP
dvec3 vort_save_d;
bool  vort_save_on;
float vort_push(){ vort_save=v1; vort_save_d=vdp; vort_save_on=dp_on; return 0.0; }
float vort_pop(){ v1=vort_save; vdp=vort_save_d; dp_on=vort_save_on; return 0.0; }
float vortwarp(float k, float size){
	dvec3 vt=(dp_point()-0.5LF)*2.0LF;
	double l=length(vt);
	if(l<=0.0LF || k==0.0 || size<=0.0)
		return 0.0;
	dvec3 u=vt/l;
	for(int i=0;i<MAX_VORTS;i++){
		if(i>=vort_n)
			break;
		dvec3 c=dvec3(vort_c[i].xyz);
		double w=dot(u,c);
		if(w<=0.0LF)
			continue;
		dvec3 e=dvec3(vort_e[i].xyz);
		dvec3 nn=cross(c,e);
		double asp=double(vort_e[i].w);
		double es=dot(u,e)/asp;
		double no=dot(u,nn);
		double d=sqrt(es*es+no*no);
		double R=double(vort_c[i].w)*double(size);
		if(d<R){
			double f=1.0LF-d/R;
			double a=double(k)*double(vort_p[i].x)*f*f*(3.0LF-2.0LF*f);
			double sn,cs;
			dsincos(a,sn,cs);
			double es2=es*cs-no*sn;
			double no2=es*sn+no*cs;
			u=normalize(es2*asp*e+no2*nn+w*c);
		}
	}
	dp_set(u*l*0.5LF+0.5LF);
	return 0.0;
}
#else
float vort_push(){ vort_save=v1; return 0.0; }
float vort_pop(){ v1=vort_save; return 0.0; }
float vortwarp(float k, float size){
	vec3 vt=(v1-0.5)*2.0;
	float l=length(vt);
	if(l<=0.0 || k==0.0 || size<=0.0)
		return 0.0;
	vec3 u=vt/l;
	for(int i=0;i<MAX_VORTS;i++){
		if(i>=vort_n)
			break;
		vec3 c=vort_c[i].xyz;
		float w=dot(u,c);
		if(w<=0.0)
			continue;
		vec3 e=vort_e[i].xyz;
		vec3 nn=cross(c,e);
		float asp=vort_e[i].w;
		float es=dot(u,e)/asp;
		float no=dot(u,nn);
		float d=length(vec2(es,no));
		float R=vort_c[i].w*size;
		if(d<R){
			float f=1.0-d/R;
			float a=k*vort_p[i].x*f*f*(3.0-2.0*f);
			float cs=cos(a), sn=sin(a);
			float es2=es*cs-no*sn;
			float no2=es*sn+no*cs;
			u=normalize(es2*asp*e+no2*nn+w*c);
		}
	}
	v1=u*l*0.5+0.5;
	return 0.0;
}
#endif
#define vortex(k,s,b) (vort_push()+vortwarp(k,s)+(b)+vort_pop())
float spots(float size){
	vec3 u=normalize((Vertex1.xyz-0.5)*2.0);
	float v=0.0;
	for(int i=0;i<MAX_VORTS;i++){
		if(i>=vort_n)
			break;
		vec3 c=vort_c[i].xyz;
		if(dot(u,c)<=0.0)
			continue;
		vec3 e=vort_e[i].xyz;
		vec3 nn=cross(c,e);
		float es=dot(u,e)/vort_e[i].w;
		float no=dot(u,nn);
		float d=length(vec2(es,no))/(vort_c[i].w*size);
		if(d<1.0)
			v+=vort_p[i].y*(1.0-smoothstep(0.8,1.0,d));   // flat core, narrow rim
	}
	return v;
}
// Texture A (alpha) expression: band texture faded out inside the ovals
float spotmask(float size, float depth){
	return 1.0-depth*clamp(abs(spots(size)),0.0,1.0);
}

// ---- belts(offset,power): gas giant band coordinate ----
// same formula as TNbelts (TerrainNode.cpp): 0.5*(2*|PHI-0.5-offset|)^power
float belts(float offset, float power){
	float x=2.0*abs(Vertex1.y-0.5-offset);
	return 0.5*pow(x,power);
}

// ---- zonal(itcz,subtrop,midlat): latitude cloud cover bias ----
// same formula as TNzonal (TerrainNode.cpp); latitude of the unwarped point
float zonal(float itcz, float subtrop, float midlat){
	vec3 vt=normalize((Vertex1.xyz-0.5)*2.0);
	float la=degrees(asin(clamp(vt.y,-1.0,1.0)));
	float a=abs(la);
	float g1=(la-6.0)/7.0;
	float g2=(a-25.0)/9.0;
	float m=clamp((a-35.0)/20.0,0.0,1.0);
	return itcz*exp(-g1*g1)-subtrop*exp(-g2*g2)+midlat*m;
}

// multi-order procedural 3d noise
vec4 Noise(int index) {
	noise_info info=nvars[index];
	
	last_seed=rseed;
	current_seed=info.animate?info.seed:rseed;
	//else
	//	current_seed=rseed;

    float orders=min(info.orders, Vertex1.w-info.logf-freqmip);
    noise_fade = lerp(orders,-1.0,1.0,0.0,1.0);

    if(noise_fade<=0.0)
    	return vec4(0.0);

	float gval=0.0,x,y;
	float fact=info.fact;
	vec4 result=vec4(0.0);
	vec4 last_val=vec4(0.0);
	vec4 val,nvec,P1,P2;
	
	float f=info.freq;
	float gain=1.0-info.bias;
	float weight=1.0;
    float octaves=max(orders,1.0);

	int n=int(octaves);
	float rem=octaves-float(n);
	n=rem>0.0?n+1:n;
	float rmin=info.smoothing*VMAX;
	if(!info.absval)
		rmin *=2.0;
	float clip=info.clamp*VMAX;
#ifdef NOISE_DP
	// double precision noise point: pivot + offset, or the warped point
	dvec3 vd=dp_point()+double(current_seed);
	// lattice period the scaled point is reduced by (in double) before the
	// float noise call, so it stays small and keeps its fractional precision
	//   GRADIENT 256 (permTexture size, GL_REPEAT)
	//   SIMPLEX  867 (3*289: mod 289 hash, x/3 skew)
	//   VORONOI  256 (hash of the cell mod 256 in voronoi3d())
	double period=info.vnoise==SIMPLEX?867.0LF:256.0LF;
#else
	vec3 v=v1+current_seed;
#endif
	for(int i=0;i<n;i++) {
		float df=f/fmax;
        float m=smoothstep(0.1,1.75,df);
#ifdef NOISE_DP
		dvec3 qd=vd*double(f);
		if(period>0.0LF)
			qd-=period*floor(qd/period);
		vec3 vf=vec3(qd);
#else
		vec3 vf=v*f;
#endif
        switch(info.vnoise){
        case SIMPLEX:
        	P1=simplex3d(vf);
	        break;
        case VORONOI:
        	P1=voronoi3d(vf);
	        break;
        case GRADIENT:
        default:
	        P1=noise3D(vf);
        }
        nvec=P1;

		nvec.yzw*=f;
		if(nvec.x>clip+0.1*nvec.x){
			nvec.x=clip+0.1*nvec.x;
			nvec.yzw*=0.1;
		}		
		f*=info.L;
		if(info.absval) {
			if(nvec.x<0.0)
			    nvec=-nvec;
			if(rmin>0.0 && nvec.x<rmin){
		    	x=nvec.x/rmin;
			    y=DCURVE1(x);
			    nvec.yzw*=y;
				x=GCURVE1(x);
			    nvec.x=rmin*x;
			}
			nvec.x=VMAX-nvec.x;
			nvec.yzw=-nvec.yzw;
			nvec*=2;
		}
		else{
			nvec.x=VMAX+nvec.x;
		    if(rmin>0.0 && nvec.x<rmin){
		    	x=nvec.x/rmin;
			    y=DCURVE1(x);
			    nvec.yzw*=y;
			    x=GCURVE1(x);
			    nvec.x=rmin*x;
		    }
		}
		val=nvec*weight*fact;
		last_val=result;
		result += val;
		fact*=info.delta;
		if(weight<=0.0)
		    break;
		weight*=nvec.x*gain+info.bias;
		weight=clamp(weight,0.0,1.0);
	} // end for
	if(rem>0.0)
		result=last_val+rem*val;
	if(info.sqr){
		result.yzw*=2.0*abs(result.x);  // derivative of x^2 ?
		if(info.uns)
			result.x*=result.x;
		else
		    result.x*=abs(result.x);
	}
	float offset=noise_fade*info.offset;
	//if(info.absval)
	//	offset+=VMAX;
	if(info.invert)
		result=-result;
	result*=noise_fade*info.ampl;
	result.x+=offset;
	current_seed=last_seed;
	return result;	
}

float Noise1D(int i) {
	vec4 v = Noise(i);
	return v.x;
}

#if NLIGHTS  >0
#ifdef VNOISE
#define SET_NOISE(func) \
	v1= Vertex1.xyz; \
 	gv = func; \
 	g=gv.x; \
 	if(lighting) { \
 	    df=gv.yzw; \
 	    df*=0.1*noise_fade*bump_ampl; \
		bump = bump + amplitude*df; \
    }
#else
#define SET_NOISE(func) \
	v1= Vertex1.xyz; \
 	gv = func; \
 	g= gv.x; \
    { \
 	    float delta=bump_delta; \
		nbamp = 0.1*noise_fade*bump_ampl/delta; \
		b = g; \
		v1 = vec3(Vertex1.x+delta,Vertex1.y,Vertex1.z);  \
 		gv = func; \
		df.x =gv.x; \
		v1 = vec3(Vertex1.x,Vertex1.y+delta,Vertex1.z); \
 		gv = func; \
		df.y =gv.x; \
		v1 = vec3(Vertex1.x,Vertex1.y,Vertex1.z+delta); \
 		gv = func; \
		df.z =gv.x; \
		df = (df-vec3(b,b,b))*(nbamp); \
		bump = bump + amplitude*df; \
    }

#endif
#else
#define SET_NOISE(func) \
	v1= Vertex1.xyz; \
 	gv = func; \
 	g=gv.x;
#endif

#define NOISE_DENSITY(func) \
	v1= Vertex1.xyz; \
    nd=func;
    
#define NOISE_COLOR(func) \
	v1= Vertex1.xyz; \
    vec4 ncolor=func; \
 	color =ncolor+color;

#define NOISE_VARS \
    df=vec3(0); \
    nbamp=0.0; \
    g=0.0; \
    b=0.0; \
    noise_fade=0.0; \
	gv=vec4(0.0);

// ########## end noise_funcs.h #########################
