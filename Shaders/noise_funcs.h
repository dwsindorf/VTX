
// ########## begin noise_funcs.h #########################
uniform sampler3D noise3DTexture;
uniform float fmax;

vec3 v1;
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
	float ma;
	float mb;
	bool sqr;
	bool invert;
	bool absval;
	bool uns;
	int vnoise;
	
};

uniform noise_info nvars[NVALS];
uniform float rscale;

#define VMAX 0.5

float noise_fade=0.0;

float reset(){
	v1=Vertex1.xyz;
	return 0.0;
}
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

#define twist(a,b) (corialis(a)+b+reset())

// multi-order procedural 3d noise
vec4 Noise(int index) {
	noise_info info=nvars[index];

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
	for(int i=0;i<n;i++) {
		float df=f/fmax;
        float m=smoothstep(0.1,1.75,df);
        switch(info.vnoise){
        case SIMPLEX:
        	P1=simplex3d(v1*f);
	        break;
        case VORONOI:
        	P1=voronoi3d(v1*f);
	        break;
        case GRADIENT:
        default:
	        P1=noise3D(v1*f);
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
