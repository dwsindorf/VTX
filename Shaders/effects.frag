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

#if defined(WATER) && defined(REFLECT) && defined(RAYTRACE)
//------------------------------------------------------------------
// Screen-space reflections
//
// auximage.frag (water pass) projects each water pixel's reflected ray
// and stores the far end of the segment in FBOTex4:
//   .xy  NDC end point (R's vanishing point)
//   .z   window depth at the end point
//   .w   lit sky color, packed 8 bits/channel (needs RGBA32F FBOs)
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

// zw0 = window depth of this water pixel
vec3 traceReflection(float zw0){
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
	// effectively at infinity: if it is on screen and on land, the crossing
	// lies in [last sample, end]; if on far water, use that pixel.
	if(!hit && tmax>=1.0){
		float type;
		vec2 pe=ssrPix(s1,lo,hi);
		ssrDepth(pe,type);
		if(type>2.1){
			hit=true;
			t=1.0;
		}
		else if(type>0.5)
			return texture2DRect(FBOTex1,pe).rgb;
	}

	if(!hit){
		// the end point is R's vanishing point, so on screen the last sky pixel
		// is the reflected sky; off screen, blend toward the lit sky color
		vec3 sky=skyFallback;
		if(sawSky){
			float off=(1.0-tmax)*dlen;   // pixels beyond the screen edge
			sky=mix(lastSky,skyFallback,clamp(off/(0.5*vsize.y),0.0,1.0));
		}
		return sky;
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
	return texture2DRect(FBOTex1,ssrPix(s0+d*tb,lo,hi)).rgb;
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
		// Old formula: d=lerp(depth,0,haze_grad*haze_zfar,0,1);
		//              h=haze_ampl*Haze.a*pow(d,10*haze_grad);
		const float HAZE_GRAD_SCALE=0.1;
		float g=(1.0-haze_grad)*HAZE_GRAD_SCALE;
		float x1=(depth-haze_min)/max(haze_max-haze_min,feet);
		x1=max(x1,0.0);
		h=haze_ampl*Haze.a*(1.0-exp(-6.0*g*x1));
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
			rcolor=traceReflection(z);
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
