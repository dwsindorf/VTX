// ########## begin clouds.volume.frag #########################
// Ray-marched volumetric clouds (CloudLayer::volume()).
//
// Each pixel's view ray is marched through the shell between the cloud
// base and cloud top spheres (centered on the planet). At each step a
// cloud density is evaluated and lit by a march toward the sun out to the
// cloud top (Beer-Lambert transmittance, multiple scattering octaves,
// two-lobe Henyey-Greenstein phase function), sky light occluded by the
// cloud above, and sunlight bounced off the ground onto the base. The
// result is composited over the scene image (FBOTex1).
//
// All distances are in km, planet-centered, in the cloud layer's object
// frame (so the clouds turn with the layer).
//
// Density model:
// The layer's expression is used if it has pixel noise (vertex noise is
// evaluated on the CPU and never reaches this shader):
//   Point Z (NPZ)   the cloud SHAPE: evaluated at each 3D sample point (the
//                   same 0..1 planet coordinates as the 2D map, so at the
//                   cloud base it's the same pattern, and it changes with
//                   height above that). Heights are measured from the cloud
//                   base, so the layer Height only lifts the clouds. Z<=0 is clear air; Z=1/VOL_ZGAIN
//                   (0.1) is full density at mid height. Higher Z reaches
//                   further up (taller clouds), as Z sets the sprite stack
//                   height for sprite clouds. Coverage is added to Z*10.
//                   Replaces the built-in shape noise (Freq then only sets
//                   the erosion scale and the top height variation).
//   Color alpha     a 2D coverage mask (as for 2D clouds) if it has pixel
//   (NCC)           noise (NCC_NOISE), evaluated per column; multiplies the
//                   density.
//   Color RGB       tint (albedo, clamped 0..1). The Color expression is
//                   always compiled in for volume clouds (TNcolor). Without
//                   pixel noise it's evaluated at each lit sample with HT =
//                   height in the layer, 0 at the base to 1/VOL_ZGAIN (0.1)
//                   at the top - the same range as Z - so Color(1+4*HT,...)
//                   brightens the cloud tops as for sprites. With noise it's
//                   taken from the per column evaluation.
// Without NPZ the built-in shape is used:
//   coverage  Color alpha (NCC) or a built-in low frequency noise; plus
//             Coverage
//   shape     Perlin fBm shaped by inverted Worley fBm (billows), with a
//             larger scale clustering noise
// Both:
//   tops      each cloud's top is Height*(1+HtVar*n) above the base (n: a
//             0..1 noise, so HtVar=0: all the same height, 1: up to twice
//             as tall). The cloud's shape (flat base, rounded top, erosion)
//             is measured against its own top, so tall clouds are stretched
//             copies of short ones
//   erosion   high frequency Worley: billowy near the top, wispy near the
//             base (Erosion; 0 = off)
//
// Passes (CloudLayer::drawVolume):
//   default      sky pixels (FBOTex2.r==0) only; terrain pixels are drawn
//                after the effects pass (haze, fog, water)
//   VOL_TERRAIN  sky and terrain pixels (no effects pass follows)
//   VOL_POST     terrain pixels only, over the effects image (FBOTex3/4,
//                written to attachments 0/1 for postproc)
// Terrain pixels: the ray stops at the terrain (FBOTex2.g window depth), so
// clouds are drawn in front of terrain, and terrain inside a cloud is
// partly hidden.

#include "utils.h"
#include "sunlight.h"
uniform float sun_tau;
uniform float sun_redden;
#include "cloud_haze.h"

#ifndef PI
#define PI 3.14159265359
#endif

// VOL_DEBUG (0 = normal output):
//   1: which path each pixel takes
//      blue   not a sky pixel (FBOTex2.r>0.5) - left as is
//      green  sky pixel, but the ray misses the cloud shell
//      red    ray segment through the shell found (brighter = longer)
//      yellow tint added where any cloud density was found on the ray
//   2: height of the eye in the shell: red=below base, green=inside,
//      blue=above top (whole screen one color)
//   3: coverage (weather) at the middle of each ray (grey scale)
//   4: Color tint (RGB at the cloud mid height); magenta if the Color
//      expression isn't compiled in (no NCC)
//   5: terrain pixels (drawn in the pass that handles them): terrain
//      distance in green (brighter = nearer, 0..50 km), red if the ray
//      reaches the cloud layer before the terrain (clouds can be in front),
//      blue if the depth gives no valid distance
//   7: raw density at one point per ray (the middle of its segment through
//      the layer), no marching or lighting: stripes here come from the
//      density/noise itself, not from the ray steps
//   8: as 7 but the Z noise only (0.5+5*Z), without the height profile
//   9: lighting only: the light (S) at the first cloud sample on each ray,
//      no accumulation (stripes here: the lighting; none: the march)
//   6: cloud opacity found along each ray (red = 1-T, before haze) and
//      green = sky haze amount at the first cloud; terrain and sky pixels
//      in the passes that draw them
#define VOL_DEBUG 0

// Point Z value that gives full coverage is 1/VOL_ZGAIN
#define VOL_ZGAIN 10.0

// coverage is evaluated every VOL_WSTRIDE steps and interpolated
// (the layer's noise expression can be expensive)
#define VOL_WSTRIDE 2

// with Z (NPZ) shape: octaves dropped for the light march samples
#define VOL_LIGHT_LOD 2.0

// with Z (NPZ) shape: height of the base rounding as a fraction of the
// cloud top (0.06 of the layer is the minimum: a flat base). Dense cores
// still reach the flat base; weaker parts curve up from it.
#define VOL_BASE_ROUND 0.25

// with Z (NPZ) shape: where Z is weak the base lifts by up to this
// fraction of the cloud top (dense cores reach the flat base)
#define VOL_BASE_VAR 0.3

// with Z (NPZ) shape: denser parts reach higher. A point's local cloud top
// (as a fraction of its column's top) rises from VOL_TOP_MIN where Z just
// makes cloud to 1 where Z*VOL_ZGAIN+Cover reaches VOL_TOP_CR, so the tops
// follow the 3D noise (domes) rather than the column top (a flat lid)
#define VOL_TOP_MIN 0.35
#define VOL_TOP_CR  2.0
// density fades in over the top/base band where the height profile is
// below this (strong Z no longer gives a hard edge at the profile limit)
#define VOL_EDGE_SOFT 0.3

// saturation of the sky color used for the ambient (sky) light. The light
// under a cloud is mostly scattered sunlight and ground bounce, much
// greyer than the sky itself (1 = full sky color)
#define VOL_SKY_SAT 0.3

// ---- lighting ----
// sun light march: VOL_LIGHT_STEPS samples toward the sun, out to where
// the sun ray leaves the cloud top (at most VOL_LIGHT_MAX km), finer near
// the sample point
#define VOL_LIGHT_STEPS 6
#define VOL_LIGHT_MAX 4.0
// multiple scattering octaves (Hillaire 2016): octave i has extinction
// VOL_MS_A^i, energy VOL_MS_B^i and phase anisotropy VOL_MS_C^i - the
// later octaves reach deep into the cloud with a broad phase function
#define VOL_MS_OCTAVES 3
#define VOL_MS_A 0.4
#define VOL_MS_B 0.55
#define VOL_MS_C 0.5
// overall sunlight scale (Diffuse on the Lighting tab scales it too)
#define VOL_SUN_GAIN 0.35
// sky light: occluded by the cloud above (2 sample march straight up);
// the sky light is diffuse, so it's attenuated less than direct sun
#define VOL_AMB_SIGMA 0.25
// sunlight bounced off the ground onto the cloud base (ground albedo)
#define VOL_GROUND 0.25

// distance fade (aerial perspective, vol_haze km): distant clouds keep at
// least this much of their own color, so near the sun they don't turn into
// the bright sunrise glow behind them (the sky haze, cloud_haze.h, still
// fades them fully when haze is on)
#define VOL_FADE_MIN 0.4

// ---- banding at cloud edges ----
// VOL_JITTER: per pixel start offset of the view ray samples
//   0: interleaved gradient noise (smooth, but its regular pattern showed
//      as fine stripes along cloud edges)
//   1: hashed white noise (fine grain instead of a pattern; sine-free hash -
//      fract(sin(x)*43758) loses precision on many GPUs and repeats in rows)
#define VOL_JITTER 0
// samples at a cloud edge (entering from clear air, or thin density) are
// split into VOL_EDGE_SUB density sub-samples over the step interval, so
// the edge isn't quantized to the step grid (0 or 1: off)
// VOL_LIGHT_JITTER: random offset of the light march samples per pixel
// (breaks up light banding, but adds speckle)
#define VOL_LIGHT_JITTER 0
#define VOL_EDGE_SUB 4
#define VOL_EDGE_D   0.3     // density below which a sample counts as edge

varying vec2 ScreenUV;

// ---- the layer's expression: Point Z (NPZ, TNpoint::initProgram) and
// Color (NCC, TNcolor::initProgram) ----
// noise_funcs.h reads Vertex1 (normally a varying set from the map
// vertexes); here it's a global set per sample point.
// VOL_NPZ   Z shape (needs pixel noise)
// VOL_NCC   Color alpha coverage mask (needs pixel noise in the Color)
// VOL_TINT  Color RGB tint
#if defined(NVALS)
#if NVALS>0
#ifdef NPZ
#define VOL_NPZ
#endif
#if defined(NCC) && defined(NCC_NOISE)
#define VOL_NCC
#endif
#endif
#endif
#ifdef NCC
#define VOL_TINT
#endif
#if defined(VOL_NPZ) || defined(VOL_TINT)
#define VOL_NOISE
#endif

#ifdef VOL_NOISE
vec4 Vertex1=vec4(0.5,0.5,0.5,0.0);
vec4 Vertex2=vec4(0.0);
vec4 Constants1=vec4(0.0);
vec3 v1;
float logf=0.0;
float RED=1.0;
float GREEN=1.0;
float BLUE=1.0;
float ALPHA=1.0;
float bmpht=0.0;
uniform float freqmip;
#define BMPHT bmpht
#define SLOPE 0.0
#define HT Constants1.x
#define FHT Constants1.x
#define DENSITY Constants1.z
#define SFACT Constants1.a
#define PHI Vertex1.y
#define THETA Vertex1.x
#define LAT abs(PHI)
#define EQU abs(PHI-0.5)
#define HEMI (2.0*smoothstep(-0.15,0.15,PHI-0.5)-1.0)
#define X Vertex1.x
#if NVALS>0
#ifdef NOISE_DP
// double precision noise point (noise_funcs.h): set with Vertex1 by
// volPoint() from the eye relative sample position, so the large eye
// position doesn't round the noise point (blocks in close views)
#define VOL_DP
dvec3 dp_base=dvec3(0.5LF);
#define DP_BASE dp_base
#endif
#include "noise_funcs.h"
#endif
#endif
vec3 vol_ctint=vec3(1.0);   // tint from the last column (Color with noise)

#ifdef VOL_POST
uniform sampler2DRect FBOTex3;   // effects pass image
uniform sampler2DRect FBOTex4;   // effects pass params (as FBOTex2)
#define SCENE_TEX FBOTex3
#define PARAM_TEX FBOTex4
#else
uniform sampler2DRect FBOTex1;   // scene image
uniform sampler2DRect FBOTex2;   // scene params (r = type, 0 = sky; g = window depth)
#define SCENE_TEX FBOTex1
#define PARAM_TEX FBOTex2
#endif
// terrain distance: eye depth = 1/(ws2*z+ws1) (scene units, as effects.frag)
uniform float vol_ws1;
uniform float vol_ws2;
uniform vec3  vol_fwd;           // view axis (unit), layer frame

// camera (km)
uniform vec3 vol_eye;
uniform vec3 vol_ray00;          // corner ray directions: lower left
uniform vec3 vol_ray10;          // lower right
uniform vec3 vol_ray01;          // upper left
uniform vec3 vol_ray11;          // upper right

uniform vec3 vol_sun;            // unit vector toward the sun
uniform vec3 vol_sun_color;      // light color * Diffuse
uniform vec3 vol_sky_color;      // sky color
uniform vec3 vol_amb_color;      // Ambient color * strength
uniform float vol_amb_base;      // ambient at the cloud base (1 at the top)
uniform vec3 vol_emission;       // Radiance
uniform float vol_fwd_g;         // forward scattering lobe g (Shine)
uniform float vol_fwd_w;         // forward scattering lobe weight (Specular)

uniform float vol_rg;            // ground radius (km)
uniform float vol_rb;            // layer level radius (km): the layer Height (Object tab)
uniform float vol_rt;            // highest cloud top radius (km): level+depth/2+hvar
uniform float vol_depth;         // cloud thickness (km, Height on the Volume tab), centered on the level
uniform float vol_far;           // max ray length (km)
uniform vec3  vol_hts;           // heights above the ground (km): eye, lowest cloud base, highest top

uniform float vol_cov;           // coverage bias
uniform float vol_freq;          // shape noise frequency (1/km)
uniform float vol_wfreq;         // built-in coverage noise frequency (1/km)
uniform float vol_sigma;         // extinction (1/km) at density 1
uniform float vol_efreq;         // erosion frequency multiplier
uniform float vol_estr;          // erosion strength
uniform float vol_hvar;          // cloud height variation (km): tops up to hvar higher, bases
                                 // down to hvar below the level (independently per cloud)
uniform float vol_haze;          // aerial perspective distance (km)
uniform vec3 vol_haze_color;     // air color (Sky haze_lit: lit for the time of day)
uniform int   vol_steps;         // view ray steps
uniform float vol_seed;          // noise offset (per layer)
uniform float vol_wlog;          // noise LOD: log2(0.25*wscale) (km)
uniform float vol_km2scene;      // scene units per km (for the sky haze)


// ---------------- noise ----------------
// octaves are rotated against each other to hide the lattice
const mat3 vol_rot=mat3( 0.00, 0.80, 0.60,
                        -0.80, 0.36,-0.48,
                        -0.60,-0.48, 0.64);
vec3 hash33(vec3 p){
	p=fract(p*vec3(0.1031,0.1030,0.0973));
	p+=dot(p,p.yxz+33.33);
	return fract((p.xxy+p.yxx)*p.zyx);
}
// gradient noise, about [-0.7,0.7]
float gnoise(vec3 p){
	vec3 i=floor(p);
	vec3 f=p-i;
	vec3 u=f*f*f*(f*(f*6.0-15.0)+10.0);
	float n000=dot(hash33(i+vec3(0,0,0))*2.0-1.0,f-vec3(0,0,0));
	float n100=dot(hash33(i+vec3(1,0,0))*2.0-1.0,f-vec3(1,0,0));
	float n010=dot(hash33(i+vec3(0,1,0))*2.0-1.0,f-vec3(0,1,0));
	float n110=dot(hash33(i+vec3(1,1,0))*2.0-1.0,f-vec3(1,1,0));
	float n001=dot(hash33(i+vec3(0,0,1))*2.0-1.0,f-vec3(0,0,1));
	float n101=dot(hash33(i+vec3(1,0,1))*2.0-1.0,f-vec3(1,0,1));
	float n011=dot(hash33(i+vec3(0,1,1))*2.0-1.0,f-vec3(0,1,1));
	float n111=dot(hash33(i+vec3(1,1,1))*2.0-1.0,f-vec3(1,1,1));
	return mix(mix(mix(n000,n100,u.x),mix(n010,n110,u.x),u.y),
	           mix(mix(n001,n101,u.x),mix(n011,n111,u.x),u.y),u.z);
}
// distance to the nearest feature point (0 at the points, ~1 max)
float worley(vec3 p){
	vec3 c=floor(p);
	vec3 f=p-c;
	float dmin=9.0;
	for(int x=-1;x<=1;x++)
	for(int y=-1;y<=1;y++)
	for(int z=-1;z<=1;z++){
		vec3 o=vec3(float(x),float(y),float(z));
		vec3 d=o+hash33(c+o)-f;
		dmin=min(dmin,dot(d,d));
	}
	return sqrt(dmin);
}
float fbm_g(vec3 p,int oct){
	float s=0.0,a=1.0,n=0.0;
	for(int i=0;i<5;i++){
		if(i>=oct) break;
		s+=a*gnoise(p);
		n+=a; a*=0.5; p=vol_rot*p*2.03+13.1;
	}
	return s/n;
}
float fbm_w(vec3 p,int oct){
	float s=0.0,a=1.0,n=0.0;
	for(int i=0;i<3;i++){
		if(i>=oct) break;
		s+=a*worley(p);
		n+=a; a*=0.5; p=vol_rot*p*2.03+7.7;
	}
	return s/n;
}
float remap(float x,float a,float b,float c,float d){
	return c+(x-a)*(d-c)/max(b-a,1e-5);
}

#ifdef VOL_NOISE
// set the noise point (Vertex1, and its double form for NOISE_DP)
// v: noise point (0..1 planet frame), w: noise level of detail
#ifdef VOL_DP
void volPoint(dvec3 v, float w){
	dp_base=v;
	Vertex1=vec4(vec3(v),w);
}
#else
void volPoint(vec3 v, float w){
	Vertex1=vec4(v,w);
}
#endif
#endif

// ---------------- coverage (per column) ----------------
// q: sample position relative to the eye (km), dist: distance from the eye (km)
// returns (coverage 0..1, cloud base and top heights above the layer level, km)
vec3 column(vec3 q, float dist){
	float wc;
	vec3 n=normalize(vol_eye+q);   // unit direction from the planet center
#ifdef VOL_NOISE
#ifdef VOL_DP
	volPoint(normalize(dvec3(vol_eye)+dvec3(q))*0.5LF+0.5LF,vol_wlog-log2(max(dist,0.01)));
#else
	volPoint(n*0.5+0.5,vol_wlog-log2(max(dist,0.01)));
#endif
	Constants1=vec4(0.0);
#endif
#ifdef VOL_NPZ
	// 2D mask only (the shape comes from Z in cloudDensity)
#ifdef VOL_NCC
	v1=Vertex1.xyz;
	RED=1.0;GREEN=1.0;BLUE=1.0;ALPHA=1.0;
	vec4 c=NCC;
	wc=clamp(c.a,0.0,1.0);
	vol_ctint=clamp(c.rgb,0.0,1.0);
#else
	wc=1.0;
#endif
#elif defined(VOL_NCC)
	v1=Vertex1.xyz;
	RED=1.0;GREEN=1.0;BLUE=1.0;ALPHA=1.0;
	vec4 c=NCC;
	wc=clamp(c.a+vol_cov,0.0,1.0);
	vol_ctint=clamp(c.rgb,0.0,1.0);
#else
	vec3 wp=n*vol_rg*vol_wfreq;
	wc=clamp(0.5+vol_cov+0.9*fbm_g(wp+vol_seed*0.37,3),0.0,1.0);
#endif
	// cloud base and top relative to the layer level (Object tab Height):
	// centered on it, Height (Volume tab) thick, and Ht Var (km) stretches
	// each cloud by random amounts (two independent noises a few km
	// across): the top up to Ht Var higher, the base up to Ht Var lower
	vec3 hp=n*vol_rg*vol_freq*0.25+vol_seed*0.71;
	float ht=clamp(0.5+1.2*fbm_g(hp,2),0.0,1.0);
	float hb=clamp(0.5+1.2*fbm_g(hp*1.13+17.3,2),0.0,1.0);
	return vec3(wc,-0.5*vol_depth-vol_hvar*hb,0.5*vol_depth+vol_hvar*ht);
}

#ifdef VOL_NPZ
// the layer's Point Z expression at a 3D point (q: relative to the eye, km);
// dist: distance from the eye (noise level of detail), lod: octaves to drop
float zfield(vec3 q, float dist, float lod){
	// measured from the cloud base: the base maps to the ground radius
	float w=vol_wlog-log2(max(dist,0.01))-lod;
#ifdef VOL_DP
	// (no double divisions: they're very slow. The float 0.5/vol_rg is the
	// same for every sample, so its rounding doesn't matter)
	dvec3 p=dvec3(vol_eye)+dvec3(q);
	double ir=inversesqrt(dot(p,p));
	double r=dot(p,p)*ir;
	dvec3 pb=p*((double(vol_rg)+(r-double(vol_rb)))*ir);
	volPoint(pb*double(0.5/vol_rg)+0.5LF,w);
#else
	vec3 p=vol_eye+q;
	float r=length(p);
	vec3 pb=p*((vol_rg+(r-vol_rb))/r);
	volPoint(pb/(2.0*vol_rg)+0.5,w);
#endif
	v1=Vertex1.xyz;
	Constants1=vec4(0.0);
	vec4 zv=vec4(NPZ);
	return zv.x;
}
#endif

// ---------------- cloud density ----------------
// q: position relative to the eye (km), cw: column(), dist: distance from
// the eye (km). returns 0..1
float cloudDensity(vec3 q, vec3 cw, float dist, bool detail){
	vec3 p=vol_eye+q;
	float wc=cw.x;
	if(wc<=0.0)
		return 0.0;
	float r=length(p);
	float hk=r-vol_rb-cw.y;         // height above this cloud's base (km)
	float thick=cw.z-cw.y;          // its base to top (km)
	if(hk<0.0 || hk>=thick)
		return 0.0;
	float h=hk/thick;               // fraction of this cloud's own top
	vec3 ps=p+vol_seed;
#ifdef VOL_NPZ
	// height profile: flat base, rounded top
	float top=1.0;
	float z=zfield(q,dist,detail?0.0:VOL_LIGHT_LOD);
	float cr=VOL_ZGAIN*z+vol_cov;
	if(cr<=0.0)
		return 0.0;
	// base: lifted where Z is weak (VOL_BASE_VAR), then rounded
	// (VOL_BASE_ROUND)
	float lift=VOL_BASE_VAR*top*clamp(1.5-cr,0.0,1.0);
	float hbase=VOL_BASE_ROUND*top;
	// local top: denser parts reach higher (domes that follow the noise)
	float tl=top*mix(VOL_TOP_MIN,1.0,smoothstep(0.0,VOL_TOP_CR,cr));
	float g=clamp(remap(h,lift,lift+hbase,0.0,1.0),0.0,1.0)*clamp(remap(h,tl*0.3,tl,1.0,0.0),0.0,1.0);
	if(g<=0.0)
		return 0.0;
	g=g*g*(3.0-2.0*g);
	// shape from Z: density c is needed to survive up to where the profile
	// is g, so only the densest parts reach the top; near the profile limit
	// (g < VOL_EDGE_SOFT) the density fades out instead of ending in a hard
	// edge when c is saturated
	float c=clamp(cr,0.0,1.0)*wc;
	float d=clamp((c-(1.0-g))/max(g,VOL_EDGE_SOFT),0.0,1.0);
#else
	// height profile: flat base, rounded top, taller where coverage is high
	float top=max(0.45+0.55*wc,0.12);
	float g=clamp(remap(h,0.0,0.06,0.0,1.0),0.0,1.0)*clamp(remap(h,top*0.45,top,1.0,0.0),0.0,1.0);
	if(g<=0.0)
		return 0.0;
	g=g*g*(3.0-2.0*g);
	vec3 qf=ps*vol_freq;
	// base shape: Perlin fBm (with larger scale clustering) shaped by
	// inverted Worley fBm
	float pf=0.55*gnoise(qf*0.31+3.7)+fbm_g(qf,detail?4:3)*1.1+0.5;
	float wf=1.0-fbm_w(qf*1.5,detail?3:2);
	float base=clamp(remap(pf,-(1.0-wf)*0.6,1.0,0.0,1.0),0.0,1.0);
	float d=clamp(remap(base*g,1.0-wc,1.0,0.0,1.0),0.0,1.0);
#endif
	if(d<=0.0 || !detail || vol_estr<=0.0)
		return d;
	// erosion: billowy near the top, wispy near the base
	float e=1.0-fbm_w(ps*vol_freq*vol_efreq,2);
	float hb=clamp(h*5.0,0.0,1.0);
	e=e*hb+(1.0-e)*(1.0-hb);
	return clamp(remap(d,vol_estr*e,1.0,0.0,1.0),0.0,1.0);
}

// ---------------- ray / sphere ----------------
// returns (t near, t far); t far < 0 if missed
// ray from the EYE against the sphere at height hs above the ground. Same
// as sphere(vol_eye,d,vol_rg+hs) but precise: dot(o,o)-R*R cancels two
// numbers ~4e7 km^2 (float error ~ +-4 km^2), which shifted the segment
// ends in steps with the view elevation and showed as horizontal bands.
// Uses heights from doubles and the stable form of the quadratic.
vec2 eyeShell(vec3 d, float hs){
	float he=vol_hts.x;
	float b=dot(vol_eye,d);
	float c=(he-hs)*(2.0*vol_rg+he+hs);
	float disc=b*b-c;
	if(disc<0.0)
		return vec2(1.0,-1.0);
	float s=sqrt(disc);
	float q=(b>0.0)?-(b+s):(-b+s);   // root of larger magnitude
	float t1=q;
	float t2=(abs(q)>1e-12)?c/q:0.0;
	return vec2(min(t1,t2),max(t1,t2));
}
vec2 sphere(vec3 o, vec3 d, float R){
	float b=dot(o,d);
	float c=dot(o,o)-R*R;
	float disc=b*b-c;
	if(disc<0.0)
		return vec2(1.0,-1.0);
	float s=sqrt(disc);
	return vec2(-b-s,-b+s);
}

float hg(float c,float g){
	float g2=g*g;
	return (1.0-g2)/(4.0*PI*pow(1.0+g2-2.0*g*c,1.5));
}

// sample spacing: finer near the start of the segment
#define STEP_LIN 0.3
float stepT(float x, float L){ return L*(STEP_LIN*x+(1.0-STEP_LIN)*x*x); }

// sine-free hash (Dave Hoskins, "Hash without Sine"): 0..1
float hash12(vec2 p){
	vec3 p3=fract(vec3(p.xyx)*0.1031);
	p3+=dot(p3,p3.yzx+33.33);
	return fract((p3.x+p3.y)*p3.z);
}

// the sky around the sun disk (cloud_haze.h): average of a ring of samples
// just outside the disk that aren't disk
vec3 skyAroundSun(vec3 c){
	float rp=max(sun_disk,2.5*sun_pix)/max(sun_pix,1e-6)+3.0;   // disk radius + edge (pixels)
	float R=2.5*rp;                                              // ring outside the disk
	vec3 sum=vec3(0.0);
	float n=0.0;
	for(int k=0;k<12;k++){
		float a=6.2831853*float(k)/12.0;
		vec3 q=texture2DRect(SCENE_TEX,gl_FragCoord.xy+R*vec2(cos(a),sin(a))).rgb;
		if(sunDiskW(q)<0.5){
			sum+=q;
			n+=1.0;
		}
	}
	return n>0.0?sum/n:min(c,vec3(CLOUD_SUN_LUM));
}



void main(void) {
	vec4 scene=texture2DRect(SCENE_TEX,gl_FragCoord.xy);
	vec4 params=texture2DRect(PARAM_TEX,gl_FragCoord.xy);
	gl_FragData[1]=params;
	bool terrain=params.r>0.5;
#if VOL_DEBUG==2
	{
		float re0=length(vol_eye);
		gl_FragData[0]=re0<vol_rb?vec4(1,0,0,1):(re0<=vol_rt?vec4(0,1,0,1):vec4(0,0,1,1));
		return;
	}
#endif
#if defined(VOL_POST)
	if(!terrain){                // sky: done in the main pass
		gl_FragData[0]=scene;
		return;
	}
#elif !defined(VOL_TERRAIN)
	if(terrain){                 // terrain/water: after the effects pass
#if VOL_DEBUG==1
		gl_FragData[0]=vec4(0,0,1,1);
#else
		gl_FragData[0]=scene;
#endif
		return;
	}
#endif
	vec3 dir=normalize(mix(mix(vol_ray00,vol_ray10,ScreenUV.x),
	                       mix(vol_ray01,vol_ray11,ScreenUV.x),ScreenUV.y));
	vec3 o=vol_eye;
	float he=vol_hts.x;                // eye height above the ground (km)

	// segment of the ray inside the cloud shell (first one in front)
	vec2 to=eyeShell(dir,vol_hts.z);
	vec2 ti=eyeShell(dir,vol_hts.y);
	vec2 tg=eyeShell(dir,0.0);
	float t0,t1;
	if(he<vol_hts.y){                  // below the clouds
		t0=ti.y;
		t1=to.y;
	}
	else if(he<=vol_hts.z){            // inside the cloud layer
		t0=0.0;
		t1=(ti.y>0.0 && ti.x>0.0)?ti.x:to.y;
	}
	else{                              // above the clouds
		t0=to.x;
		t1=(ti.y>0.0 && ti.x>0.0)?ti.x:to.y;
	}
	if(tg.y>0.0 && tg.x>0.0)           // stop at the ground
		t1=min(t1,tg.x);
	if(terrain){                       // stop at the terrain
		float zd=1.0/(vol_ws2*params.g+vol_ws1);   // eye depth (scene units)
		float tt=(zd/vol_km2scene)/max(dot(dir,vol_fwd),1e-3);
#if VOL_DEBUG==5
		{
			bool ok=(zd>0.0 && tt<1e7);
			float g=ok?clamp(1.0-tt/50.0,0.0,1.0):0.0;
			float rr=(ok && to.y>0.0 && max(t0,0.0)<tt && t1>max(t0,0.0))?1.0:0.0;
			gl_FragData[0]=vec4(rr,g,ok?0.0:1.0,1.0);
			return;
		}
#endif
		t1=min(t1,tt);
	}
	t0=max(t0,0.0);
	t1=min(t1,t0+vol_far);
	if(to.y<0.0 || t1<=t0){
#if VOL_DEBUG==1
		gl_FragData[0]=vec4(0,1,0,1);
#else
		gl_FragData[0]=scene;
#endif
		return;
	}
	float L=t1-t0;
	float N=float(vol_steps);
#if VOL_JITTER==1
	float jitter=hash12(gl_FragCoord.xy);
#else
	float jitter=fract(52.9829189*fract(dot(gl_FragCoord.xy,vec2(0.06711056,0.00583715))));
#endif
	bool prevEmpty=true;      // previous sample was clear air (edge sub-samples)
#if VOL_LIGHT_JITTER
	float ljit=hash12(gl_FragCoord.yx+17.31);   // light march offset
#else
	float ljit=0.5;
#endif

#if VOL_DEBUG==4
	{
#ifdef VOL_TINT
#ifdef VOL_DP
		volPoint(normalize(dvec3(o)+dvec3(dir*(t0+0.5*L)))*0.5LF+0.5LF,vol_wlog);
#else
		volPoint(normalize(o+dir*(t0+0.5*L))*0.5+0.5,vol_wlog);
#endif
		v1=Vertex1.xyz;
		Constants1=vec4(0.5/VOL_ZGAIN,0.0,0.0,0.0);
		RED=1.0;GREEN=1.0;BLUE=1.0;ALPHA=1.0;
		gl_FragData[0]=vec4(clamp(vec4(NCC).rgb,0.0,1.0),1.0);
#else
		gl_FragData[0]=vec4(1.0,0.0,1.0,1.0);
#endif
		return;
	}
#endif
#if VOL_DEBUG==7 || VOL_DEBUG==8
	{
		float tm=t0+0.5*L;
		vec3 pm=dir*tm;
		vec3 cm=column(pm,tm);
#if VOL_DEBUG==7
		float dm=cloudDensity(pm,cm,tm,true);
#elif defined(VOL_NPZ)
		float dm=clamp(0.5+5.0*zfield(pm,tm,0.0),0.0,1.0);
#else
		float dm=0.0;
#endif
		gl_FragData[0]=vec4(dm,dm,dm,1.0);
		return;
	}
#endif
#if VOL_DEBUG==3
	{
		float w=column(dir*(t0+0.5*L),t0+0.5*L).x;
		gl_FragData[0]=vec4(w,w,w,1.0);
		return;
	}
#endif

	// sun disk behind this pixel (sky pixels only)
	// only near the sun's direction: a bright sky (e.g. near-white haze at
	// the horizon) also passes the brightness test, and skyAroundSun's ring
	// then picked the darker pixels around it (the terrain below), which
	// gave the clouds there dark rims
	float sunw=terrain?0.0:sunDiskW(scene.rgb);
	if(sunw>0.0){
		float rsun=max(sun_disk,2.5*sun_pix)+3.0*sun_pix;   // disk radius + edge (radians)
		float ang=acos(clamp(dot(dir,vol_sun),-1.0,1.0));
		sunw*=1.0-smoothstep(rsun,rsun+2.0*sun_pix,ang);
	}
	vec3 sunsky=sunw>0.0?skyAroundSun(scene.rgb):scene.rgb;


	float cosang=dot(dir,vol_sun);
	// phase function per scattering octave (broader for the later ones)
	float phs[VOL_MS_OCTAVES];
	{
		float c=1.0;
		for(int k=0;k<VOL_MS_OCTAVES;k++){
			phs[k]=4.0*PI*mix(hg(cosang,-0.3*c),hg(cosang,vol_fwd_g*c),vol_fwd_w);
			c*=VOL_MS_C;
		}
	}

	// ambient (sky) light color, mostly desaturated (same luminance)
	float skyl=dot(vol_sky_color,vec3(0.299,0.587,0.114));
	vec3 skyamb=mix(vec3(skyl),vol_sky_color,VOL_SKY_SAT);

	vec3 col=vec3(0.0);
	float T=1.0;
	float tfirst=-1.0;

	// coverage at sample i0 and i0+VOL_WSTRIDE, interpolated in between
	int i0=0;
	float ta=t0+stepT(jitter/N,L);
	float tb=t0+stepT((float(VOL_WSTRIDE)+jitter)/N,L);
	vec3 cA=column(dir*ta,ta);
	vec3 cB=column(dir*tb,tb);

	for(int i=0;i<256;i++){
		if(i>=vol_steps || T<0.01)
			break;
		if(i>=i0+VOL_WSTRIDE){
			i0+=VOL_WSTRIDE;
			cA=cB;
			float tn=t0+stepT((float(i0+VOL_WSTRIDE)+jitter)/N,L);
			cB=column(dir*tn,tn);
		}
		float x=(float(i)+jitter)/N;
		float t=t0+stepT(x,L);
		float dt=L*(STEP_LIN+2.0*(1.0-STEP_LIN)*x)/N;
		vec3 cw=mix(cA,cB,float(i-i0)/float(VOL_WSTRIDE));
		vec3 q=dir*t;                // relative to the eye
		vec3 p=o+q;
		float d=cloudDensity(q,cw,t,true);
#if VOL_EDGE_SUB>1
		// cloud edge: average the density over the step interval
		if((d>0.003 && (prevEmpty || d<VOL_EDGE_D)) || (d<=0.003 && !prevEmpty)){
			float ds=0.0;
			for(int k=0;k<VOL_EDGE_SUB;k++){
				float tk=t+dt*((float(k)+0.5)/float(VOL_EDGE_SUB)-0.5);
				ds+=cloudDensity(dir*tk,cw,tk,true);
			}
			d=ds/float(VOL_EDGE_SUB);
		}
#endif
		prevEmpty=(d<=0.003);
		if(d<=0.003)
			continue;
		if(tfirst<0.0)
			tfirst=t;
		vec3 pn=normalize(p);
		float rp=length(p);
		float h=clamp((rp-vol_rb-cw.y)/(cw.z-cw.y),0.0,1.0);   // fraction of this cloud's top

		// light march toward the sun (same column coverage), out to where
		// the sun ray leaves the cloud top: thick clouds go dark below
		float Ls=clamp(sphere(p,vol_sun,vol_rt).y,0.05,VOL_LIGHT_MAX);
		float od=0.0;
		float xp=0.0;
		for(int j=0;j<VOL_LIGHT_STEPS;j++){
			float xa=float(j+1)/float(VOL_LIGHT_STEPS);
			float xm=(float(j)+ljit)/float(VOL_LIGHT_STEPS);
			float x1=Ls*xa*xa;
			od+=cloudDensity(q+vol_sun*(Ls*xm*xm),cw,t,false)*(x1-xp);
			xp=x1;
		}
		// multiple scattering octaves
		float ms=0.0;
		{
			float a=1.0,b=1.0;
			for(int k=0;k<VOL_MS_OCTAVES;k++){
				ms+=b*exp(-a*vol_sigma*od)*phs[k];
				a*=VOL_MS_A;
				b*=VOL_MS_B;
			}
		}
		// sky light, occluded by the cloud above
		float Lu=max(vol_rt-rp,0.0);
		float odu=0.5*Lu*(cloudDensity(q+pn*(0.25*Lu),cw,t,false)
		                 +cloudDensity(q+pn*(0.75*Lu),cw,t,false));
		float ambT=exp(-VOL_AMB_SIGMA*vol_sigma*odu);

		// sunlight color/strength at this point (reddening, night side)
		float sinElev=dot(pn,vol_sun);
		float day=smoothstep(-0.08,0.12,sinElev);
		vec3 sunc=vol_sun_color*sunTint(sinElev,sun_tau,sun_redden)*day;
		vec3 amb=skyamb*vol_amb_color*mix(vol_amb_base,1.0,h)*ambT*(0.03+0.97*day);
		// ground bounce: lights the base, fading out with height
		amb+=VOL_GROUND*sunc*max(sinElev,0.0)*(1.0-h)*(1.0-h)*0.25;
		vec3 tint=vec3(1.0);
#if defined(VOL_TINT)
#if defined(NCC_NOISE)
		tint=vol_ctint;
#else
		Constants1=vec4(h/VOL_ZGAIN,0.0,0.0,0.0);  // HT: height in the layer
		RED=1.0;GREEN=1.0;BLUE=1.0;ALPHA=1.0;
		tint=clamp(vec4(NCC).rgb,0.0,1.0);
#endif
#endif
		vec3 S=tint*(sunc*(ms*VOL_SUN_GAIN)+amb)+vol_emission;
#if VOL_DEBUG==9
		gl_FragData[0]=vec4(S,1.0);
		return;
#endif
		float st=exp(-vol_sigma*d*dt);
		col+=T*S*(1.0-st);
		T*=st;
	}
#if VOL_DEBUG==6
	gl_FragData[0]=vec4(1.0-T,tfirst>0.0?cloudHaze(tfirst*vol_km2scene):0.0,0.0,1.0);
	return;
#endif
	// aerial perspective: distant clouds fade into the sky behind them.
	// By day that's the scene pixel; at night the scene has stars in it,
	// so it's clamped to the air color (the stars only show where the
	// cloud itself is thin: T*scene below)
	if(tfirst>0.0){
		float f=max(exp(-tfirst/vol_haze),VOL_FADE_MIN);
		float eday=smoothstep(-0.1,0.1,dot(normalize(o),vol_sun));
		vec3 behind=terrain?scene.rgb:mix(min(scene.rgb,vol_haze_color),scene.rgb,eday);
		// the HDR sun disk must not come back through the distance fade:
		// over it, fade into the sky around the disk
		// the HDR sun disk must not come back through the distance fade:
		// over it, fade into the sky around the disk
		if(!terrain && sunw>0.0)
			behind=mix(behind,sunsky,sunw);
		col=mix((1.0-T)*behind,col,f);
		// sky haze (the terrain haze, effects.frag): the cloud fades into
		// the haze color as the terrain at that distance does
		col=mix(col,(1.0-T)*ch_color,cloudHaze(tfirst*vol_km2scene));
	}
#if VOL_DEBUG==1
	gl_FragData[0]=vec4(0.3+0.7*clamp(L/vol_far,0.0,1.0),tfirst>0.0?0.8:0.0,0,1);
	return;
#endif
	// sun disk behind the clouds (cloud_haze.h): the disk is HDR bright, so
	// (cloud opacity 1-T): T^K of the disk, the rest is the sky around it
	vec3 bg=scene.rgb;
	if(!terrain && sunw>0.0)
		bg=mix(bg,pow(T,CLOUD_SUN_K-1.0)*scene.rgb+(1.0-pow(T,CLOUD_SUN_K-1.0))*sunsky,sunw);
	gl_FragData[0]=vec4(col+T*bg,scene.a);
}
// ########## end clouds.volume.frag #########################
