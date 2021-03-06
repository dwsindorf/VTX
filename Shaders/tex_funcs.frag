// ########## begin tex_funcs.frag #########################
// TODO: for 1d textures, pass in S expression for evaluation in shader
#include "attributes.h"
#include "textures.h"

struct tex2d_info {
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
	bool  t1d;           // 1d texture
	bool  randomize;     // randomized texture
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

#define BIAS vec2(tex2d[tid].bias,0.0)
#define NOATTR 1.0
#define SET_ATTRIB(ATTR) \
	attrib = ATTR;

 //   slope_bias=tex2d[i].slope_bias*Tangent.z;
 //   slope_bias=tex2d[i].slope_bias*1e7*fwidth(HT);

#define INIT_TEX(i,COORDS) \
  	tid = i; \
	coords = COORDS; \
	scale=tex2d[i].scale; \
	amplitude = clamp(attrib,0.0,1.0); \
	logf=tex2d[i].logf; \
	last_color=color; \
	alpha = tex2d[i].texamp; \
	alpha_bias=Tangent.w-logf-colormip+tex2d[i].bias; \
	alpha_fade = lerp(alpha_bias,-6.0,1.0,0.0,1.0); \
	alpha_fade *= lerp(alpha_bias,10,17,1.0,0.0); \
	dlogf=tex2d[i].dlogf; \
	last_bump=bump; \
	last_bmpht=bmpht; \
	bump_ampl = tex2d[i].bumpamp; \
    bump_delta = tex2d[i].bump_delta; \
	bump_bias=(bmpht)*tex2d[i].bump_bias; \
	slope_bias = tex2d[i].slope_bias; \
    phi_bias=tex2d[i].phi_bias*pow(abs(PHI-0.5),2); \
    height_bias=HT*tex2d[i].height_bias;\
    slope_bias=tex2d[i].slope_bias*(Tangent.z+Normal.w-0.5); \
    env_bias=phi_bias+bump_bias+height_bias+slope_bias;\
	if(tex2d[i].t1d) { \
	    coords.x+=env_bias; \
	} \
    else { \
        alpha+=env_bias; \
        bump_ampl+=env_bias; \
        amplitude+=env_bias; \
        amplitude=clamp(amplitude,0,1); \
    }

#define SET_TEX1D(X) \
	coords.x += (X);  // optional offset

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
		cmix = alpha_fade*amplitude*lerp(alpha,1.0,2.0,tval.a*alpha,1.0);
		
#define SET_COLOR \
		last_color=color; \
		color.rgb=mix(color,tval, cmix); \
		color.a=cmix;
		
#define SET_BUMP \
		bump*=1.0-tval.a*tex2d[tid].bump_damp*bump_max; \
		s=coords.x; \
		t=coords.y; \
		orders_bump=tex2d[tid].orders_bump; \
		ds=vec2(s+orders_bump,t); \
		dt=vec2(s,t+orders_bump); \
		tc2.x=textureTile(tid,ds,texmip).a-tval.a; \
		tc2.y=textureTile(tid,dt,texmip).a-tval.a; \
		tc=vec3(tc2, 0.0); \
	    last_bump=bump; \
	    last_bmpht=bmpht; \
	    bump_fade = lerp(Tangent.w-logf-freqmip,-4.0,1.0,0.0,1.0); \
	    bump_fade *= lerp(Tangent.w-logf-freqmip,3.0,10.0,1.0,0.0); \
	    bump_max=max(bump_max,bump_fade); \
		bump += bump_max*amplitude*bump_ampl*trans_mat*tc; \
		bmpht += b+amplitude*(tval.a-0.5)*bump_ampl*orders_delta;
		
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
 	vec2 tc2; \
	float s,t; \
	vec3 tc; \
	vec2 ds,dt; \
    mat3 model=mat3(gl_ModelViewMatrix); \
    vec3 tangent=normalize(vec3(Tangent.x,Tangent.y,0.0)); \
    vec3 binormal=normalize(cross(tangent, normal)); \
    mat3 trans_mat=model*transpose(mat3(tangent, binormal, normal));

// ########## end tex_funcs.frag #########################
