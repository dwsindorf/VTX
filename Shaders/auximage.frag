#include "utils.h"

#define HT Constants1.x
#define DENSITY Constants1.z

varying vec4 Color;
varying vec4 Constants1;
varying vec4 EyeDirection;
varying vec4 Normal;


uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;
#define SHADOWTEX FBOTex3

uniform float twilite_min;
uniform float twilite_max;
uniform float twilite_dph; // dot product between point-center light-point at horizon
uniform vec3 center;
uniform vec4 WaterSky;

uniform vec4 WaterColor2;
uniform vec4 WaterColor1;
uniform float water_reflection;
uniform float water_saturation;
uniform float water_clarity;
uniform float water_mix;
uniform float water_dpr;

uniform vec4 IceColor2;
uniform vec4 IceColor1;
uniform float ice_reflection;
uniform float ice_saturation;
uniform float ice_clarity;
uniform float ice_mix;
uniform float ice_dpr;

//uniform float water_dpr;
uniform float dpm;
uniform float ws1;
uniform float ws2;
uniform vec4 Shadow;
uniform float fog_vmin;
uniform float fog_vmax;

uniform float feet;
uniform float dh;           // 1/viewport width
uniform float dv;           // 1/viewport height
uniform float water_shine;  // water tab Shine (0-100)
uniform float ice_shine;
uniform float water_albedo; // water tab Albedo (specular amount 0-1)
uniform float ice_albedo;
uniform float sun_tau;     // Sky::sunTau()  (0 = no reddening)
uniform float sun_redden;  // Sky::redden

#include "sunlight.h"

//
//Lighting model
//

void main(void) {
	vec4 fcolor1=texture2DRect(FBOTex1, gl_FragCoord.xy); // FBO image (background)
	vec4 fcolor2=texture2DRect(FBOTex2, gl_FragCoord.xy); // data texture
	
    float type=Constants1.g;

	float ambient = 0.0;
	float specular = 0.0;
	float diffuse = 0.0;

	vec3 Diffuse = vec3(0.0, 0.0, 0.0);
	vec3 Specular = vec3(1.0, 1.0, 1.0);
	vec3 Ambient = vec3(0.0, 0.0, 0.0);

	vec3 eye = normalize(EyeDirection.xyz);
	vec3 radius = -normalize(EyeDirection.xyz+center);
	vec3 normal  = normalize(Normal.xyz);

	float RdotN   = dot(radius,normal); // for top lighting on nightside
	float top_shading  = 0.5*RdotN+0.5;  // models hemispherical lighting (light from all angles)

    ambient=gl_FrontMaterial.ambient.a;
    vec3 topcolor=gl_FrontMaterial.ambient.rgb*top_shading*ambient;

    Ambient = topcolor;
    float horizon=0.0;
    float illumination = gl_FrontMaterial.emission.a;
    
#ifdef SHADOWS
    float shadow=texture2DRect(SHADOWTEX, gl_FragCoord.xy).r; // data texture
    float shadow_specular=lerp(Shadow.a,0.0,1.0,1.0,shadow);
    float shadow_diffuse=shadow_specular;
#else
	float shadow_diffuse=1.0;
	float shadow_specular=1.0;
#endif
	for(int i=0;i<NLIGHTS;i++){
		vec3 light      = normalize(gl_LightSource[i].position.xyz+EyeDirection.xyz);
		float LdotR     = dot(light,radius ); // for horizon band calculation
		//horizon   += lerp(LdotR,-1,1,1.0,0.0); // twilite band
		horizon   += lerp(LdotR,twilite_dph+twilite_min,twilite_dph+twilite_max,0.0,1.0); // twilite band
		illumination   += horizon;

		ambient   += horizon;

		float LdotN     = dot(light,normal);
		float amplitude = 1.0/gl_LightSource[i].constantAttenuation;
		float lpn       = LdotN*amplitude*horizon;
		//diffuse+=LdotN;
		diffuse+=max(lpn,top_shading*gl_FrontMaterial.ambient.a);
		Diffuse        += gl_LightSource[i].diffuse.rgb*diffuse;
		//Diffuse=gl_LightSource[i].diffuse.rgb*diffuse;
		// Water/ice always use the half-vector (OGL / Blinn-Phong) model,
		// whatever the scene's lighting model: at grazing sun angles it
		// stretches the sun's glint into the long vertical "glitter path"
		// seen on real water, where Phong gives a round spot. Terrain
		// still follows the Lighting Model setting (lighting.frag).
		vec3 H = normalize((light + eye)*0.5);
		float sdp   = dot(normal,H);
		// Shine slider (0-100) -> exponent 2..1000: high values give the
		// narrow glitter column of a low sun. (gl_FrontMaterial.shininess
		// was whatever the last land layer set - the water Shine slider
		// never reached this shader.)
		float wshine = mix(water_shine,ice_shine,lerp(type,1.0,2.0,0.0,1.0));
		float expo   = 2.0+wshine*wshine*0.1;
		// sharp glint (Shine) + a broad sheen. The sheen restores the old
		// look where open water gets progressively lighter toward the sun:
		// originally this shader used gl_FrontMaterial.shininess, which was
		// left at the land's low shininess, so the whole highlight was a
		// wide lobe. Now Shine controls the glint, and this fixed low-
		// exponent lobe supplies the gradient (real water does this too -
		// wave facets spread the sun's reflection over a wide area).
		const float SHEEN_EXP=8.0;   // width of the sheen (lower = wider)
		const float SHEEN_AMT=0.35;  // brightness of the sheen vs the glint
		specular    = pow(max(sdp,0.0), expo) + SHEEN_AMT*pow(max(sdp,0.0), SHEEN_EXP);
		// no glint once the sun is below the horizon (the twilight band
		// still lights the water, but the sun itself can't be reflected)
		float sunUp = smoothstep(-0.01,0.02,LdotR);
		float walb  = mix(water_albedo,ice_albedo,lerp(type,1.0,2.0,0.0,1.0));
		// glint color follows the sun: bright yellow-white near the horizon
		// (sunCoreTint), like the disk itself
		vec3 glint  = sunCoreTint(sunTint(LdotR,sun_tau,sun_redden));
		Specular    = gl_LightSource[i].specular.rgb*glint*specular*sunUp*walb*shadow_specular;
	}
	float clarity=1;
	float wf=lerp(type,1,2,0,1);

	vec3 Color1 = mix(WaterColor1,IceColor1,wf);
	vec3 Color2 = mix(WaterColor2,IceColor2,wf);
#define TEST
#ifdef TEST
	if(type<0.5){
		float df=1.0-type;
		Color2.xyz=Color2.xyz*df;
	}
#endif	
	float reflection=mix(water_reflection,ice_reflection,wf);
	float saturation=mix(water_saturation,ice_saturation,wf);
	clarity=mix(water_clarity,ice_clarity,wf);
	float cmix=mix(water_mix,ice_mix,wf);
	float dpr=0.5*mix(water_dpr,ice_dpr,wf);

	float reflect1=dot(normal,eye); // reflection angle
	float depth=gl_FragCoord.z; // water
	float z=fcolor2.g;  // land
	float z2=1.0/(ws2*z+ws1);
	float z1=1.0/(ws2*depth+ws1);
	float dz=(z2-z1);
	dz=dz<0.0?0.0:dz;
	float f=fcolor2.r==0?1:lerp(dz,0.0,clarity,0.0,1.0); // f=1 if back surface != land
	
	// Water body shading (restored original model): deep water shows the
	// Transmit color (Color2) as its body color, blended toward the
	// Reflect color (Color1) and the sky with view angle (fmix/rmod), which
	// gives the depth/left-right shading. The "transparency only filters,
	// opaque water shows only reflection" variant was tried and removed:
	// it flattened open water to a uniform reflection color.
	vec3 TopColor=Diffuse*Color1.rgb+Ambient*Color1.rgb;
	
#ifdef SKY
	vec3 DepthColor=Color2.rgb*horizon;	
	vec3 color=mix(fcolor1.rgb,DepthColor,f); // add depth color
	float fmix=lerp(reflect1,dpr,dpm,cmix,0);
	float rmod=lerp(reflect1,0,dpr,reflection,0);
	float cmod=(1-rmod)*saturation;
	fmix*=fmix;
	float f1=1-fmix; // surface
	float f2=fmix*cmod;
	float f3=rmod;

#ifdef RAYTRACE
	// reflections are traced in effects.frag; adding the constant sky color
	// here too would tint every reflection (e.g. blue under a sunset sky)
	vec3 SkyColor0=vec3(0.0);
#else
	// no traced reflection: approximate it with the sky's base color
	vec3 SkyColor0=Diffuse*WaterSky.rgb+Ambient*WaterSky.rgb;
#endif

	color=color.rgb*f1+TopColor*f2+SkyColor0.rgb*f3+Specular;
	color.rgb=mix(color.rgb,vec3(0),1-shadow_diffuse);
#else
    vec3 color=mix(fcolor1.rgb,Color2.rgb*(0.25+horizon),f)+Specular; // add depth color 
#endif	
    //color.rgb=vec3(horizon,0,0);
	color=clamp(color,0.0,1.0);
	gl_FragData[0] = vec4(color,1.0);
	float ht=HT+fcolor2.a;
	
	float vfog=DENSITY*lerp(HT,fog_vmin,fog_vmax,1,0);
	gl_FragData[0].a=1;
	gl_FragData[1] = vec4(type,depth,reflect1,vfog);

#ifdef RAYTRACE
	// Screen-space reflection ray setup, consumed by effects.frag.
	// FBOTex4 (attachment 3, unused until the effects pass overwrites it):
	//   .xy  NDC end point of the projected reflected ray
	//   .z   window depth of the end point
	//   .w   lit sky color, packed 8 bits/channel (exact only in RGBA32F)
	// The ray is built in camera space from the fragment's window position.
	// EyeDirection can't be used: View::project() puts the full lookat (eye
	// translation included) into GL_PROJECTION, so modelview space is not
	// camera-centered, and its large coordinates lose too much float
	// precision for grazing views. gl_ProjectionMatrix = Persp*[Rot|t]; only
	// its rotation/lens terms (xyz of rows 0-3) are used here.
	{
		mat4 Pm = gl_ProjectionMatrix;
		vec3 r0 = vec3(Pm[0][0],Pm[1][0],Pm[2][0]);
		vec3 r1 = vec3(Pm[0][1],Pm[1][1],Pm[2][1]);
		vec3 r2 = vec3(Pm[0][2],Pm[1][2],Pm[2][2]);
		vec3 r3 = vec3(Pm[0][3],Pm[1][3],Pm[2][3]);
		float fx = length(r0);                // f/aspect
		float fy = length(r1);                // f
		float A  = dot(r2,-r3/length(r3));    // Persp[2][2]
		float w  = 1.0/gl_FragCoord.w;        // clip w = eye depth
		vec2 ndc = gl_FragCoord.xy*vec2(dh,dv)*2.0-1.0;
		vec3 Pc  = vec3(ndc.x*w/fx, ndc.y*w/fy, -w);   // camera-space position
		vec3 V   = normalize(Pc);
		// face normal of the water surface (mesh normals tilt near coastlines)
		vec3 N   = normalize(cross(dFdx(Pc),dFdy(Pc)));
		if(dot(N,V)>0.0)
			N=-N;
		vec3 R  = reflect(V,N);
		vec4 H0 = vec4(ndc*w, (2.0*gl_FragCoord.z-1.0)*w, w);
		vec4 dH = vec4(fx*R.x, fy*R.y, A*R.z, -R.z);
		// long ray: the end point converges to R's vanishing point
		float L = 1.0e4*w;
		if(dH.w<0.0)                          // turning toward the eye: stay in front of it
			L=min(L,0.9*H0.w/(-dH.w));
		vec4 H1 = H0+L*dH;
		vec3 SkyColor=clamp(Diffuse*WaterSky.rgb+Ambient*WaterSky.rgb,0.0,1.0);
		vec3 sc = floor(SkyColor*255.0+0.5);
		float skypack=sc.r*65536.0+sc.g*256.0+sc.b;
		gl_FragData[2]=vec4(H1.xy/H1.w,0.5*H1.z/H1.w+0.5,skypack);
		// FBOTex5 (attachment 4): camera-space reflected ray and eye depth,
		// used by effects.frag to continue the ray in the sky view
		// (camera-space origin = (ndc.x*w/fx, ndc.y*w/fy, -w))
		gl_FragData[3]=vec4(R,w);
	}
#endif
}

