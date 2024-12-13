// ########## begin tex_funcs.frag #########################
// TODO: for 1d textures, pass in S expression for evaluation in shader
#include "attributes.h"
#include "textures.h"

struct tex2d_info {
	float bumpval;
	float bumpamp;
	float texamp;
	float bump_delta;
	float bump_damp;
	float orders_delta;  // scale multiplier for multi-orders
	float orders_atten;  // attenuation factor
	float orders_bump;
	float scale;         // texture scale
	float bias;   		 // texture bias 
	float logf;    		 // scale factor
	float dlogf;         // delta scale factor
	float orders; 		 // number of orders to add
	float phi_bias; 	 // latitude bias
	float height_bias;   // height bias
	float bump_bias;     // bump bias
	float slope_bias; 	 // slope bias
	float near_bias; 	 // low frequency bias bias
	float far_bias; 	 // high frequency bias bias
	float tilt_bias;     // tilt bias	
	bool  t1d;           // 1d texture
	bool  randomize;     // randomized texture	
	bool  seasonal;      // seasonal	
};
uniform tex2d_info tex2d[NTEXS];
uniform sampler2D samplers2d[NTEXS];

#include "tile_funcs.frag"

vec4 textureTile(int id, in vec2 uv , float mm)
{
#ifdef NOTILE
   if(tex2d[id].randomize)
       return textureNoTile(id, samplers2d[id], uv,mm);
#endif
   return texture(samplers2d[id], uv,mm);
}


float phiFunc(int id){
	//float p=PI*(PHI-0.5);
	//float tf=sin(p+tex2d[id].tilt_bias);
	//return tf*tf;
	if(tex2d[id].seasonal)
		return SFACT;
	else
		return PHI;
}
#define BIAS vec2(tex2d[tid].bias,0.0)
#define NOATTR 1.0
#define SET_ATTRIB(ATTR) \
	v1= Vertex1.xyz; \
	attrib = ATTR;

#define INIT_TEX(i,COORDS) \
  	tid = i; \
	coords = COORDS; \
	scale=tex2d[i].scale; \
	amplitude = clamp(attrib,0.0,1.0); \
	logf=tex2d[i].logf; \
	last_color=color; \
	alpha = tex2d[i].texamp; \
	alpha_bias=Tangent.w-logf-colormip+tex2d[i].bias; /* larger with higher frequencies */  \
	alpha_fade = lerp(alpha_bias,-6.0,1.0,tex2d[i].far_bias,1.0); /* fade high frequencies with distance */ \
	alpha_fade *= lerp(alpha_bias,10,17,1.0,tex2d[i].near_bias); /* fade low frequencies with closeness */ \
	dlogf=tex2d[i].dlogf; \
	last_bump=bump; \
	last_bmpht=bmpht; \
	bump_ampl = tex2d[i].bumpamp; \
    bump_delta = tex2d[i].bump_delta; \
	bump_bias=bmpht*tex2d[i].bump_bias; \
    phi_bias=tex2d[i].phi_bias*phiFunc(i); \
    height_bias=HT*tex2d[i].height_bias;\
    slope_factor=abs(tex2d[i].slope_bias); \
    slope_bias=2*lerp(8*slope_factor*Tangent.z,0,1,-tex2d[i].slope_bias,tex2d[i].slope_bias); \
    env_bias=phi_bias+height_bias+bump_bias+slope_bias;\
	if(tex2d[i].t1d) { \
	    coords.x+=env_bias; \
	} \
    else { \
        alpha+=env_bias; \
        bump_ampl*=1+max(env_bias,-1); \
        amplitude*=1+max(env_bias,-1); \
        amplitude=clamp(amplitude,0,1); \
        bump_ampl*=amplitude; \
    }

//#define SET_TEX1D(X) \
//	coords.x += (X)/scale;  // optional offset

#define BGN_ORDERS \
	tex_orders=min(tex2d[tid].orders, Tangent.w-logf-freqmip+0.5); \
	tex_n=int(tex_orders); \
	tex_rem=tex_orders-float(tex_n); \
	tex_n=tex_rem>0.0?tex_n+1:tex_n; \
	orders_delta=1.0; \
	for(int i=0;i<tex_n;i++) {	
		
#define APPLY_TEX \
	    offset = vec2(g*tex2d[tid].texamp*tex2d[tid].scale); \
		tval=textureTile(tid,coords+offset,texmip); \
		alpha = tex2d[tid].texamp; \
		cmix = clamp(alpha_fade*amplitude*tval.a*alpha,0,1); 
        //cmix = alpha_fade*amplitude*lerp(alpha,1.0,2.0,tval.a*alpha,1.0);
		
#define SET_COLOR \
		last_color=color; \
		color.rgb=mix(color,tval, cmix); \
		color.a=cmix;	

#define SET_BUMP \
	    tva=(tval.x+tval.y+tval.z)/3.0;\
		bump*=1.0-tva*tex2d[tid].bump_damp*bump_max; \
		s=coords.x; \
		t=coords.y; \
		orders_bump=tex2d[tid].orders_bump; \
		ds=vec2(s+orders_bump,t); \
		dt=vec2(s,t+orders_bump); \
		tcs=textureTile(tid,ds,texmip); \
		tct=textureTile(tid,dt,texmip); \
		tsa=(tcs.x+tcs.y+tcs.z)/3.0;\
		tta=(tct.x+tct.y+tct.z)/3.0;\
		tc2.x=tsa-tva; \
		tc2.y=tta-tva; \
		tc=vec3(tc2, 0.0); \
	    last_bump=bump; \
	    last_bmpht=bmpht; \
	    bump_fade = lerp(Tangent.w-logf-freqmip,-4.0,1.0,0.0,1.0); \
	    bump_fade *= lerp(Tangent.w-logf-freqmip,3.0,10.0,1.0,0.0); \
	    bump_max=max(bump_max,bump_fade); \
		bump += bump_max*bump_ampl*trans_mat*tc; \
		bmpht += b+(tva-0.5)*bump_ampl*orders_delta;
		
#define NEXT_ORDER \
		orders_delta /= tex2d[tid].orders_delta; \
	    coords *= tex2d[tid].orders_delta; \
		amplitude *=tex2d[tid].orders_atten; \
		logf+=dlogf;

#define END_ORDERS \
	} \
	if(tex_rem>0.0){ \
		color=mix(last_color,color,tex_rem); \
		bump=mix(last_bump,bump,tex_rem); \
		bmpht=mix(last_bmpht,bmpht,tex_rem); \
	} \


#define TEX_VARS \
	int tid=0; \
	float orders_delta = 1.0;  \
	float alpha = 1.0;  \
	float phi = PHI-0.5;  \
	float cmix = 1.0;  \
    float attrib=0.0; \
    vec2 coords; \
    vec2 offset=vec2(0.0); \
 	vec4 tval; \
	vec4 tcolor; \
    float alpha_fade=0.0; \
    float delta; \
    float bump_fade=1.0; \
    float dlogf; \
    float orders_bump=1e-3; \
	float tex_orders=0.0; \
	float tex_rem=0.0; \
	float bump_max=0.0; \
	float scale=1.0; \
	float bump_bias=0; \
	float slope_bias=0; \
	float slope_factor=0; \
	float height_bias=0; \
	float phi_bias=0; \
	float env_bias=0; \
	float alpha_bias=0; \
	int tex_n=0; \
	vec4 last_color=vec4(0.0); \
	float last_bmpht; \
    vec3 last_bump=vec3(0.0); \
	amplitude = 1.0; \
	g=0.0; 	
	
#define BUMP_VARS \
	float tva; \
	float tsa; \
	float tta; \
 	vec2 tc2; \
 	vec4 tcs; \
 	vec4 tct; \
	float s,t; \
	vec3 tc; \
	vec2 ds,dt; \
    mat3 model=mat3(gl_ModelViewMatrix); \
    vec3 tangent=normalize(vec3(Tangent.x,Tangent.y,0.0)); \
    vec3 binormal=normalize(cross(tangent, normal)); \
    mat3 trans_mat=model*transpose(mat3(tangent, binormal, normal));

// ########## end tex_funcs.frag #########################
