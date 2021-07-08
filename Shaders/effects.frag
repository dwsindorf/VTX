#include "utils.h"

#define PI		3.14159265359

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3

uniform vec4 Haze;
uniform float haze_grad;
uniform float haze_zfar;
uniform float haze_ampl;

uniform vec4 Fog;
uniform float fog_zfar;
uniform float fog_znear;

uniform float fog_ampl;

uniform float reflection;
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

#define VFOG    fcolor2.a
#define TYPE    fcolor2.r
#define DEPTH   fcolor2.g
#define RDP     fcolor2.b

#define AMBIENT fcolor1.a

void main(void) {
	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // image
	vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // Params
	vec3 color = fcolor1.rgb;
	float h,d;
#ifdef SHADOW_TEST
	gl_FragData[0]=texture2DRect(SHADOWTEX, gl_FragCoord.xy);
	return;
#endif
	if(TYPE>0.5) { // TYPE=0 for sky or background
		float z=DEPTH;
		float depth=1.0/(ws2*z+ws1);
#ifdef HAZE
		d=lerp(depth,0.0,haze_zfar,0.0,1.0);
		h=haze_ampl*Haze.a*pow(d,1.5*haze_grad);
#endif
#ifdef WATER
		if(TYPE<1.8){ // TYPE=1 for water
			float ry=RDP;
			float rmod=lerp(ry,0.0,water_dpr,1.0,0.0);
#ifdef REFLECT
			float y1=gl_FragCoord.y*dv;
			float y=y1+4*ry;

			vec2 lu=vec2(gl_FragCoord.x,y/dv);
			//vec3 rcolor=texture2DRect(FBOTex1, lu);            // default: pixel at top edge
			vec3 rcolor=texture2DRect(FBOTex1, gl_FragCoord.xy).rgb; // default: pixel start point
#ifdef RAYTRACE
			// TODO: 1. improve speed by creating lut texture for trig. exprs.
			//       2. fix artifact when row is not sky
			//       3. fix depth precision artifact
			//       4. fix ave-normals artifacts
			//       5. fix striping artifact at low angles
			//       6. when view is tilted (e.g. turning) reflection is still vertical (should be slanted)
			float maxy=1-y1-dv;
			float steps=maxy/dv;

			float z1=depth;
			y=gl_FragCoord.y+1.0;
			float astep=dv*fov;
			float theta=PI-2.0*acos(RDP);
			int last_water=0;
			for(int i=0;i<steps;i++){
				float alpha=astep*i;
				float beta=theta-alpha;
				float t=texture2DRect(FBOTex2, vec2(gl_FragCoord.x,y+i)).r;
				if(t>1.5 || t< 0.5)
				    last_water=i;
				if(t==0){ // first hit = sky
					float z2=texture2DRect(FBOTex2, vec2(gl_FragCoord.x,y+i+2)).r;
					if(z2!=0) //2nd hit !=sky (hole?)
						continue; // try next higher pixel if 2 hits in a row we're done
					rcolor=texture2DRect(FBOTex1, vec2(gl_FragCoord.x,y+2*last_water)).rgb; // pick a point same distance above horizon
					break;
				}
				float z=texture2DRect(FBOTex2, vec2(gl_FragCoord.x,y+i)).g;
				float zb=1.0/(ws2*z+ws1);// zbuffer depth at y
				float zr=z1*(cos(alpha)+sin(alpha)/tan(beta)); // depth of reflected ray at y
				float dz=zb/zr;
				if(dz<0.999){
					rcolor=texture2DRect(FBOTex1, vec2(gl_FragCoord.x,y+i)).rgb;
					break;
				}
			}
			
#endif // RAYTRACE
			color=mix(color,rcolor.xyz,reflection*rmod);
#endif // REFLECT
#ifdef HAZE
#ifdef FOG
			vec3 rfog=Fog.rgb;
#else
			vec3 rfog=Haze.rgb;
#endif // FOG
			color=mix(color,rfog*AMBIENT,rmod*h);
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
	//gl_FragData[0]=vec4(h,0,0,1.0);	
	
    gl_FragData[1]=fcolor2;
}

