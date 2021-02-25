#include "utils.h"

#define JITTER
#define JITTER_TEST
//#define LDRTEST

#define SAMPLES_COUNT 64
#define SAMPLES_COUNT_DIV2 32
#define INV_SAMPLES_COUNT (1.0f/SAMPLES_COUNT);

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform vec3 center;
uniform vec3 Light;

varying vec4 EyeDirection;
varying vec4 Normal;

#ifdef USING_PROJ
uniform sampler2DRectShadow ShadowMap;
#else
uniform sampler2DRect ShadowMap;
#endif

uniform sampler3D JitterMap;

varying vec4 ShadowCoord;
uniform float shadowmap_ratio;
uniform float fwidth;
uniform int view;
uniform int views;
uniform int shadow_test;
uniform float xmax,ymax;
uniform float zmin,zmax;
uniform float shadow_intensity;
uniform int light_index;

uniform float rscale;


#ifdef USING_PROJ
#define LOOKUP(OFFSET) shadow2DRectProj(ShadowMap, ShadowCoord + vec4((OFFSET)/shadowmap_ratio, 0, 0)).w;
#else
#define LOOKUP(OFFSET) texture2DRect(ShadowMap,sc.st+(OFFSET)).z < sc.z? 0.0 : 1.0;
#endif

void main(void) {
#ifdef SHADOW_TEST
   if(shadow_test==3){
        float d=gl_FragCoord.z;
        gl_FragData[0]=vec4(pow(d,2),0,0,1);
        return;
    }
#endif
 	float shadow = 0.0;
	float x,y;
	int i,j;
    float flag1=0;
    float flag2=1;   
    float blur=fwidth*views;
    // reset "already processed" flag on last view for next light
    float fview=(view+1.0)/views;
	if(fview>0.99)
		fview=0;
    
    vec4 lastview=texture2DRect(FBOTex3, gl_FragCoord.xy); // shadow accumulator
    if(lastview.z>0.1){ // if already processed return last view result
    	gl_FragData[0]=vec4(lastview.rg,fview,1);
    	return;
    }
    vec4 sc = ShadowCoord/ShadowCoord.w ; // transform pixel to light view
    // discard if transformed pixel is offscreen in light view
    if(sc.x>xmax || sc.x<0)
    	discard;
    if(sc.y>ymax || sc.y<0)
    	discard;
    	
   // if(sc.z>zmax || sc.z<zmin)
   // 	onscreen=0;
	vec3 light   = normalize(Light+EyeDirection.xyz);
#ifdef LDRTEST
	vec3 radius  = -normalize(EyeDirection.xyz+center);
	float LdotR  = dot(light,radius); // for horizon band calculation
	if(LdotR<-0.2)
		shadow=0.0;
	else if(LdotR>0.7)
		shadow=1.0;
	else{
#endif
	    flag2=0;
#ifdef _NORMALS_
		vec3 normal  = normalize(Normal.xyz);
		float LdotN  = dot(light,normal);
#endif
#ifdef JITTER
	    vec3 jcoord=vec3(gl_FragCoord.xy,0);
#ifdef JITTER_TEST
		for(i=0;i<4;i++){
			vec4 offset=texture3D(JitterMap,jcoord)*blur;
			jcoord.z+=1.0/SAMPLES_COUNT_DIV2;
			shadow += LOOKUP(offset.xy);
			shadow += LOOKUP(offset.zw);
		}
	    shadow/=8;
	    if(abs((shadow-1.0)*shadow) >= 0.1){
	        flag1=1;
	        shadow /= 8.0;
		    for(i=0;i<SAMPLES_COUNT-4;i++){
				vec4 offset=texture3D(JitterMap,jcoord)*blur;
				jcoord.z+=1.0/SAMPLES_COUNT_DIV2;
				shadow += LOOKUP(offset.xy);
				shadow += LOOKUP(offset.zw);
			}
			shadow /= SAMPLES_COUNT;
		}
#else
	   for(i=0;i<SAMPLES_COUNT;i++){
			vec4 offset=texture3D(JitterMap,jcoord)*blur;
			jcoord.z+=1.0/SAMPLES_COUNT_DIV2;
			shadow += LOOKUP(offset.xy);
			shadow += LOOKUP(offset.zw);
		}
		shadow /= SAMPLES_COUNT;
#endif // JITTER_TEST   
#else  // NOT JITTER
		float size=8;
		int limit=(size-1)*blur/2.0;
		for (i = 0 ; i <size ; i++){
		    y=-limit+i*blur;
			for (j = 0 ; j <size ; j++){
			    x=-limit+j*blur;		
				shadow += LOOKUP(vec2(x,y));
			}
		}
		size-=1;
		shadow /= size*size ;
#endif
#ifdef _NORMALS_
		float dp=lerp(LdotN,-0.3,0,0.0,1);
		shadow*=dp;
#endif
		shadow=clamp(shadow,0.0,1.0);
#ifdef LDRTEST
	}
#endif
	shadow*=shadow_intensity; // normalize all lights intensity 
	shadow+=lastview.r;
	//shadow=clamp(shadow,0.0,1.0);
#ifdef SHADOW_TEST
    if(shadow_test==1)
    	gl_FragData[0]=vec4(shadow,fview,fview,1);
    else if(shadow_test==2)
    	gl_FragData[0]=vec4(shadow,flag1,fview,1);
    else
#endif
	   gl_FragData[0]=vec4(shadow,0,fview,1);
    	   
}

