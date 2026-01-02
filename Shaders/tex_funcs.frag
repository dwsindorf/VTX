// ########## begin tex_funcs.frag #########################
// TODO: for 1d textures, pass in S expression for evaluation in shader
#include "attributes.h"
#include "textures.h"

extern vec3 object;
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
	bool  randomize;     // randomized texture	
	bool  seasonal;      // seasonal	
	bool  t1d;           // 1d texture
	bool  triplanar;     // triplanar mapping	
	bool  t3d;           // 3d mapping	
};
uniform tex2d_info tex2d[NTEXS];
uniform sampler2D samplers2d[NTEXS];
// static variables
#define TEX_VARS \
	int tid=0; \
	float orders_delta = 1.0;  \
	float alpha = 1.0;  \
	float phi = PHI-0.5;  \
	float cmix = 1.0;  \
    float attrib=0.0; \
    vec4 coords; \
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
    vec3 last_bump=vec3(0.0);

#define BUMP_VARS \
	float tva; \
    mat3 model=mat3(gl_ModelViewMatrix); \
    vec3 tangent=normalize(vec3(Tangent.x,Tangent.y,0.0)); \
    vec3 binormal=normalize(cross(tangent, Normal)); \
    mat3 trans_mat=model*transpose(mat3(tangent, binormal, Normal));

#if NTEXS >0
TEX_VARS;
#endif
#if NBUMPS >0
BUMP_VARS;
#endif

float phiFunc(int id){
	if(tex2d[id].seasonal)
		return SFACT;
	else
		return EQU;
}
#define SHARPEN
//#define USE_DOMINANT

#include "tile_funcs.frag"

#if NTEXS >0

// Triplanar mapping function

vec4 triplanarMap(int id, vec4 pos, float mm)
{
 	sampler2D samp=samplers2d[id];
 	 			
    vec3 N = normalize(WorldNormal.xyz);
    vec3 V = pos.xyz; 

    vec3 blendWeights = abs(N);
#ifdef SHARPEN
    blendWeights = pow(blendWeights, vec3(6.0)); // Increase power for sharper transition
#endif
    // Calculate blend weights based on normal direction
    blendWeights = blendWeights / (blendWeights.x + blendWeights.y + blendWeights.z);
    // Sample texture from 3 orthogonal planes
    vec3 xProj = texture2D(samp, V.yz,mm).rgb;
    vec3 yProj = texture2D(samp, V.xz,mm).rgb;
    vec3 zProj = texture2D(samp, V.xy,mm).rgb;
    vec3 blended=xProj*blendWeights.x +yProj*blendWeights.y +zProj*blendWeights.z;    
    // Blend the three samples
    return vec4(blended,1.0);
}

vec4 textureTile(int id, in vec4 coords , float mm)
{
#ifdef NOTILE
   	if(tex2d[tid].randomize)
       	return triplanarNoTile(tid, coords,mm);
#endif
	return triplanarMap(tid,coords,mm);
}

vec4 textureTile(int id, in vec2 uv , float mm)
{
#ifdef NOTILE
   if(tex2d[id].randomize)
       return textureNoTile(id, uv,mm);
#endif
	return texture2D(samplers2d[id], uv,mm);
}

vec4 getTex(int tid, vec4 coords, float mm){
	if(tex2d[tid].triplanar)
		return textureTile(tid,coords,mm);
	else
		return textureTile(tid, coords.xy,mm);
}
#if NBUMPS >0
vec3 getBump2d(int tid, vec4 coords,float mm){
	float s=coords.x;
	float t=coords.y;
	orders_bump=tex2d[tid].orders_bump;
	vec2 ds=vec2(s+orders_bump,t);
	vec2 dt=vec2(s,t+orders_bump);
	vec4 tcs=textureTile(tid,ds,mm);
	vec4 tct=textureTile(tid,dt,mm);
	float tsa=(tcs.x+tcs.y+tcs.z)/3.0;
	float tta=(tct.x+tct.y+tct.z)/3.0;
	return vec3(tsa-tva,tta-tva,0.0);
}
vec3 getBumpTriplanar(int id, vec4 coords, float mm) {
    vec3 N = normalize(WorldNormal);
    
    vec3 blendWeights = abs(N);
    blendWeights = blendWeights / (blendWeights.x + blendWeights.y + blendWeights.z);
    
    vec3 V = coords.xyz;  // Already scaled
    float bump_delta = tex2d[id].orders_bump;  // Use the actual bump delta
    
    // ========== X Projection (YZ plane) ==========
    vec2 uv_x = V.yz;
    vec4 tc_x = textureTile(id, uv_x, mm);
    vec4 tcs_x = textureTile(id, uv_x + vec2(bump_delta, 0.0), mm);
    vec4 tct_x = textureTile(id, uv_x + vec2(0.0, bump_delta), mm);
    
    float tva_x = (tc_x.x + tc_x.y + tc_x.z) / 3.0;
    float tsa_x = (tcs_x.x + tcs_x.y + tcs_x.z) / 3.0;
    float tta_x = (tct_x.x + tct_x.y + tct_x.z) / 3.0;
    
    vec3 bump_x = vec3(tsa_x - tva_x, tta_x - tva_x, 0.0);
    // Reorient to world space: YZ plane means normal along X
    vec3 worldBump_x = vec3(0.0, bump_x.x, bump_x.y);
    
    // ========== Y Projection (XZ plane) ==========
    vec2 uv_y = V.xz;
    vec4 tc_y = textureTile(id, uv_y, mm);
    vec4 tcs_y = textureTile(id, uv_y + vec2(bump_delta, 0.0), mm);
    vec4 tct_y = textureTile(id, uv_y + vec2(0.0, bump_delta), mm);
    
    float tva_y = (tc_y.x + tc_y.y + tc_y.z) / 3.0;
    float tsa_y = (tcs_y.x + tcs_y.y + tcs_y.z) / 3.0;
    float tta_y = (tct_y.x + tct_y.y + tct_y.z) / 3.0;
    
    vec3 bump_y = vec3(tsa_y - tva_y, tta_y - tva_y, 0.0);
    // Reorient to world space: XZ plane means normal along Y
    vec3 worldBump_y = vec3(bump_y.x, 0.0, bump_y.y);
    
    // ========== Z Projection (XY plane) ==========
    vec2 uv_z = V.xy;
    vec4 tc_z = textureTile(id, uv_z, mm);
    vec4 tcs_z = textureTile(id, uv_z + vec2(bump_delta, 0.0), mm);
    vec4 tct_z = textureTile(id, uv_z + vec2(0.0, bump_delta), mm);
    
    float tva_z = (tc_z.x + tc_z.y + tc_z.z) / 3.0;
    float tsa_z = (tcs_z.x + tcs_z.y + tcs_z.z) / 3.0;
    float tta_z = (tct_z.x + tct_z.y + tct_z.z) / 3.0;
    
    vec3 bump_z = vec3(tsa_z - tva_z, tta_z - tva_z, 0.0);
    // Already in world space for XY plane
    
    // Blend the three bump vectors
    vec3 blendedBump = worldBump_x * blendWeights.x +
                       worldBump_y * blendWeights.y +
                       bump_z * blendWeights.z;
    
    return blendedBump;
}

vec3 getBump(int tid, vec4 coords,float mm){
	if(tex2d[tid].triplanar)
		return getBumpTriplanar(tid,coords,mm);
	else
		return getBump2d(tid,coords,mm);
}
#endif

#endif

#define BIAS vec2(tex2d[tid].bias,0.0)
#define NOATTR 1.0
#define SET_ATTRIB(ATTR) \
	v1= Vertex1.xyz; \
	attrib = ATTR;

#define INIT_TEX(i,COORDS) \
  	tid = i; \
	scale=tex2d[i].scale; \
	coords = COORDS; \
	coords*= tex2d[i].t3d?scale*1e-7:1.0; \
	amplitude = clamp(attrib,0.0,1.0); \
	logf=tex2d[i].logf; \
	last_color=color; \
	alpha = tex2d[i].texamp; \
	alpha_bias=Tangent.w-logf-colormip+tex2d[i].bias; /* larger with higher frequencies */  \
	alpha_fade = lerp(alpha_bias,-3.0,1.0,tex2d[i].far_bias,1.0); /* fade high frequencies with distance */ \
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

#define BGN_ORDERS \
	tex_orders=min(tex2d[tid].orders, Tangent.w-logf-freqmip+0.5); \
	tex_n=int(tex_orders); \
	tex_n=max(tex_orders,1); \
	tex_rem=tex_orders-float(tex_n); \
	tex_n=tex_rem>0.0?tex_n+1:tex_n; \
	orders_delta=1.0; \
	for(int i=0;i<tex_n;i++) {	
		
#define APPLY_TEX \
	    offset = vec2(g*tex2d[tid].texamp*tex2d[tid].scale); \
	    coords.xy+=offset; \
	    tval=getTex(tid,coords,texmip); \
		alpha = tex2d[tid].texamp; \
		cmix = clamp(alpha_fade*amplitude*tval.a*alpha,0,1); 
 		
#define SET_COLOR \
		last_color=color; \
		color.rgb=mix(color,tval, cmix); \
		color.a=cmix;

#define SET_BUMP \
	    tva=(tval.x+tval.y+tval.z)/3.0;\
		bump*=1.0-tva*tex2d[tid].bump_damp*bump_max; \
		vec3 tc=getBump(tid,coords,texmip); \
	    last_bump=bump; \
	    last_bmpht=bmpht; \
	    bump_fade = lerp(Tangent.w-logf-freqmip,-4.0,1.0,0.0,1.0); \
	    bump_fade *= lerp(Tangent.w-logf-freqmip,3.0,10.0,1.0,0.0); \
	    bump_max=max(bump_max,bump_fade); \
	    if(tex2d[tid].t3d)  \
	        bump += bump_max*bump_ampl*tc; \
	    else  \
	        bump += bump_max*bump_ampl*trans_mat*tc; \
	    \
		bmpht += b+(tva-0.5)*bump_ampl*orders_delta;
	
#define NEXT_ORDER \
		orders_delta /= tex2d[tid].orders_delta; \
	    coords *= tex2d[tid].orders_delta; \
		amplitude *=tex2d[tid].orders_atten; \
		bump_ampl *=amplitude; \
		logf+=dlogf;

#define END_ORDERS \
	} \
	if(tex_rem>0.0){ \
		color=mix(last_color,color,tex_rem); \
		bump=mix(last_bump,bump,tex_rem); \
		bmpht=mix(last_bmpht,bmpht,tex_rem); \
	} \

// ########## end tex_funcs.frag #########################
