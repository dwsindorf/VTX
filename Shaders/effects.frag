#include "utils.h"

#define PI		3.14159265359

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;
#define SHADOWTEX FBOTex3

uniform vec4 Haze;
uniform float haze_grad;
uniform float haze_zfar;	// no longer used by the HAZE block below (see it for why) - left declared in case anything else still binds it
uniform float haze_min;
uniform float haze_max;
uniform float haze_ampl;

uniform vec4 Fog;
uniform float fog_zfar;
uniform float fog_znear;

uniform float fog_ampl;

//uniform float reflection;
//uniform float water_dpr;

uniform float ice_reflection;
uniform float water_reflection;

uniform float ice_dpr;
uniform float water_dpr;

uniform float feet;
uniform float ws1;
uniform float ws2;
uniform float dh;
uniform float dv;
uniform float zn;
uniform float zf;
uniform float fov;
uniform float tb;
uniform vec4 Shadow;

//#undef RAYTRACE
//#undef REFLECT

#define VFOG    fcolor2.a
#define TYPE    fcolor2.r
#define DEPTH   fcolor2.g
#define RDP     fcolor2.b

#define AMBIENT fcolor1.a

#ifdef HAZE
// haze amount at eye distance "depth"
float hazeAmount(float depth){
	const float HAZE_GRAD_SCALE=0.1;
	float g=(1.0-haze_grad)*HAZE_GRAD_SCALE;
	float x1=(depth-haze_min)/max(haze_max-haze_min,feet);
	x1=max(x1,0.0);
	return haze_ampl*Haze.a*(1.0-exp(-6.0*g*x1));
}
#endif

#if defined(WATER) && defined(REFLECT) && defined(RAYTRACE)
//------------------------------------------------------------------
// Screen-space reflections
//
// auximage.frag (water pass) projects each water pixel's reflected ray
// and stores the far end of the segment in FBOTex4:
//   .xy  NDC end point (R's vanishing point)
//   .z   window depth at the end point
//   .w   lit sky color, packed 8 bits/channel (needs RGBA32F FBOs) -
//        last-resort fallback when no rendered sky can be found
// The segment is clipped to the screen and marched in pixel space. Window
// depth is linear in screen space along a projected line, so the ray's
// depth at parameter t is mix(zw0,zw1,t), directly comparable to the
// depth buffer.
//------------------------------------------------------------------
#define SSR_MAX_STEPS  96     // coarse steps along the on-screen ray
#define SSR_MIN_STRIDE 1.0    // minimum step (pixels)
#define SSR_REFINE     6      // binary search steps after a crossing
#define SSR_THICK      0.02   // depth slack for the previous step (fraction of depth)
#define SSR_EPS        0.002  // depth noise tolerance (fraction of depth)
#define SSR_OCC_STEPS  12     // sideways search steps around an occluded end point
#define SSR_OCC_STRIDE 3.0    // pixels per search step

// window depth -> eye distance (for the relative tolerance tests)
float ssrLin(float zw){
	float q=ws2*zw+ws1;
	return q>0.0?1.0/q:1.0e30;   // beyond zfar
}
float ssrDepth(vec2 p, out float type){
	vec4 g=texture2DRect(FBOTex2,p);
	type=g.r;
	return ssrLin(g.g);
}
vec3 ssrUnpack(float v){
	float r=floor(v/65536.0); v-=r*65536.0;
	float g=floor(v/256.0);   v-=g*256.0;
	return vec3(r,g,v)/255.0;
}
// interleaved gradient noise: per-pixel start jitter hides stride banding
float ssrNoise(vec2 p){
	return fract(52.9829189*fract(dot(p,vec2(0.06711056,0.00583715))));
}
// texel center of a ray point, clamped to the screen (FBO textures are GL_LINEAR)
vec2 ssrPix(vec2 p, vec2 lo, vec2 hi){
	return floor(clamp(p,lo,hi))+0.5;
}

// color of a reflected pixel, its eye distance (-1 for sky/background,
// which is never hazed) and its alpha (AMBIENT, used for its haze color)
vec3 ssrFetch(vec2 p, out float zhit, out float hamb){
	vec4 c=texture2DRect(FBOTex1,p);
	float type;
	float z=ssrDepth(p,type);
	zhit=type<0.5?-1.0:z;
	hamb=c.a;
	return c.rgb;
}

// zw0 = window depth of this water pixel
// zhit/hamb: see ssrFetch (zhit<0: reflection is sky, no extra haze)
vec3 traceReflection(float zw0, out float zhit, out float hamb){
	zhit=-1.0;
	hamb=1.0;
	vec4 rd=texture2DRect(FBOTex4,gl_FragCoord.xy);
	vec2 vsize=vec2(1.0/dh,1.0/dv);
	vec3 skyFallback=ssrUnpack(rd.w);
	vec2 s0=gl_FragCoord.xy;
	vec2 s1=(rd.xy*0.5+0.5)*vsize;
	float zw1=rd.z;
	vec2 d=s1-s0;
	float dlen=length(d);
	if(dlen<1.0)
		return skyFallback;

	// clip to the screen. Mostly-vertical rays are only ended by the top or
	// bottom edge: past a side edge x is clamped and the ray slides up the
	// edge column (rays near the sides lean outward and would otherwise
	// leave the screen before seeing anything).
	vec2 lo=vec2(0.5);
	vec2 hi=vsize-0.5;
	float tx=1.0;
	float ty=1.0;
	if(d.x>0.0)      tx=(hi.x-s0.x)/d.x;
	else if(d.x<0.0) tx=(lo.x-s0.x)/d.x;
	if(d.y>0.0)      ty=(hi.y-s0.y)/d.y;
	else if(d.y<0.0) ty=(lo.y-s0.y)/d.y;
	bool slide=abs(d.y)>=abs(d.x);
	float tmax=slide?min(ty,1.0):min(min(tx,ty),1.0);
	tmax=max(tmax,0.0);

	float stride=max(SSR_MIN_STRIDE,dlen*tmax/float(SSR_MAX_STEPS));
	float dt=stride/dlen;
	float t=1.5/dlen+dt*ssrNoise(gl_FragCoord.xy); // skip own pixel, jitter start
	float tprev=0.0;
	float zprev=ssrLin(zw0);
	bool hit=false;
	bool sawSky=false;
	vec3 lastSky=skyFallback;

	for(int i=0;i<SSR_MAX_STEPS;i++){
		if(t>tmax)
			break;
		vec2 p=ssrPix(s0+d*t,lo,hi);
		float type;
		float zs=ssrDepth(p,type);
		float zr=ssrLin(mix(zw0,zw1,t));
		if(type<0.5){         // sky or background (moons etc)
			lastSky=texture2DRect(FBOTex1,p).rgb;
			sawSky=true;
		}
		else if(type>2.1){    // land (water/ice is never a hit: grazing rays skim it)
			// hit if the ray crosses the surface during this step: behind it
			// now, in front of it at the previous step. A ray already farther
			// than the surface is passing behind it (e.g. a ridge silhouette).
			if(zr>zs*(1.0+SSR_EPS) && zprev<zs*(1.0+SSR_THICK)){
				hit=true;
				break;
			}
		}
		tprev=t;
		zprev=zr;
		t+=dt;
	}

	// Depth along the ray grows like 1/(1-t) toward the vanishing point, so
	// a distant shore can be crossed after the last sample. The end point is
	// effectively at infinity: if it is on screen and on land the crossing
	// lies in [last sample, end]; if on far water, use that pixel.
	// But the end point may be covered by something in front of the ray
	// (a tree, a near rock): then what the ray would see is hidden there, so
	// search sideways (across the ray) for the nearest pixel that isn't -
	// gaps between branches usually expose the shore or sky behind.
	if(!hit && tmax>=1.0){
		float type;
		vec2 pe=ssrPix(s1,lo,hi);
		float zse=ssrDepth(pe,type);
		bool occluded=type>0.5 && zse*(1.0+SSR_THICK)<zprev;
		if(!occluded){
			if(type>2.1){
				hit=true;
				t=1.0;
			}
			else if(type>0.5)
				return ssrFetch(pe,zhit,hamb);
		}
		else{
			vec2 side=normalize(vec2(-d.y,d.x));
			for(int k=1;k<=SSR_OCC_STEPS;k++){
				for(int sgn=-1;sgn<=1;sgn+=2){
					vec2 q=ssrPix(s1+side*(float(sgn*k)*SSR_OCC_STRIDE),lo,hi);
					float zq=ssrDepth(q,type);
					if(type<0.5 || zq*(1.0+SSR_THICK)>=zprev)
						return ssrFetch(q,zhit,hamb);          // sky, or a surface behind the ray
				}
			}
			// nothing visible nearby: fall through to the sky estimate
		}
	}

	if(!hit){
		// The end point is R's vanishing point: on screen, the last sky pixel
		// passed is the reflected sky. Off screen the rendered sky nearest the
		// exit is still a far better match than the lit WaterSky color (which
		// differs a lot from the real sky, so blending toward it made large
		// areas of water jump between the two as the view moved).
		if(sawSky)
			return lastSky;
		// nothing but land/plants along the way: look for sky along the top
		// edge, outward from the column where the ray left
		vec2 pe=ssrPix(s0+d*tmax,lo,hi);
		for(int k=0;k<SSR_OCC_STEPS*2;k++){
			for(int sgn=-1;sgn<=1;sgn+=2){
				vec2 q=ssrPix(vec2(pe.x+float(sgn*k)*SSR_OCC_STRIDE*2.0,hi.y),lo,hi);
				float type;
				ssrDepth(q,type);
				if(type<0.5)
					return texture2DRect(FBOTex1,q).rgb;
			}
		}
		return skyFallback;
	}

	// refine the crossing
	float ta=tprev;
	float tb=t;
	for(int j=0;j<SSR_REFINE;j++){
		float tm=0.5*(ta+tb);
		float type;
		float zs=ssrDepth(ssrPix(s0+d*tm,lo,hi),type);
		float zr=ssrLin(mix(zw0,zw1,tm));
		if(type>2.1 && zr>zs*(1.0+SSR_EPS))
			tb=tm;
		else
			ta=tm;
	}
	return ssrFetch(ssrPix(s0+d*tb,lo,hi),zhit,hamb);
}
#endif

void main(void) {
	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // image
	vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // Params
	vec3 color = fcolor1.rgb;
	float h,d,q=0,v=0;
#ifdef SHADOW_TEST
	gl_FragData[0]=texture2DRect(SHADOWTEX, gl_FragCoord.xy);
	return;
#endif
	if(TYPE>0.6) { // TYPE=0 for sky or background
		float z=DEPTH;
		float depth=1.0/(ws2*z+ws1);
#ifdef HAZE
		h=hazeAmount(depth);
#endif
#ifdef WATER
		if(TYPE<2.1){ // TYPE=1 for water, 2 for ice, 3+ for land
			float ry=RDP;
			float wf=lerp(TYPE,1,2,0,1);
			
			float reflection=mix(water_reflection,ice_reflection,wf);
			float dpr=mix(water_dpr,ice_dpr,wf);
			
			float rmod=lerp(ry,0.0,dpr,1.0,0.0);

#ifdef REFLECT
			vec3 rcolor=fcolor1.rgb; // default: no reflection (pixel itself)
#ifdef RAYTRACE
			float zhit,hamb;
			rcolor=traceReflection(z,zhit,hamb);
#ifdef HAZE
			// The reflected image is pre-haze, and this pixel is hazed below only
			// for the eye->water distance. Give a reflected surface the same total
			// haze as viewing it directly (eye->water->surface ~ its own depth):
			// layers combine as 1-(1-h1)(1-h), so pre-apply h1 here.
			if(zhit>0.0){
				float ht=hazeAmount(zhit);
				float h1=clamp(1.0-(1.0-ht)/max(1.0-h,1.0e-3),0.0,1.0);
				rcolor=mix(rcolor,Haze.rgb*hamb,h1);
			}
#endif
#endif // RAYTRACE
			color=mix(color,rcolor,reflection*rmod);
#endif // REFLECT
#ifdef HAZE
#ifdef FOG
			vec3 rfog=Fog.rgb;
#else
			vec3 rfog=Haze.rgb;
#endif // FOG
			color=mix(color,rfog,rmod*h);
			//color=mix(color,rfog*AMBIENT,rmod*h);
			
#endif // HAZE
		}
#endif // WATER

#ifdef FOG
    vec3 fog=Fog.rgb;
	float f=Fog.a*VFOG*lerp(depth,fog_znear,fog_zfar,0.0,fog_ampl);
#ifdef SHADOWS
    float shadow=1-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r; // data texture
    // darken fog in shadow
    // - lessen effect for increasing fog density otherwize shadow
    //   seems to punch through fog when viewed in direction of light
    // - TODO: need a fog "luminosity" factor to control all of this
    fog=mix(fog,vec3(0.0),0.5*shadow*(1-f)*(1-f));
#endif
	color=mix(color,fog.rgb*AMBIENT,f);
#endif

#ifdef HAZE
	color=mix(color,Haze.rgb*AMBIENT,h);
#endif
 	}
		
	gl_FragData[0]=vec4(color,1.0);	
	//gl_FragData[0]=vec4(TYPE,0,0,1.0);	
	
    gl_FragData[1]=fcolor2;
}
