
#include "common.h"
#include "utils.h"

uniform sampler2D samplers2d[NSPRITES];

varying vec4 index;
varying vec4 Normal;
varying vec4 EyeDirection;
varying vec4 Position;

uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;

#define SHADOWTEX FBOTex3

uniform vec4 Diffuse;
uniform vec4 Shadow;
uniform vec4 Haze;
uniform float haze_zfar;
uniform float haze_ampl;
uniform float haze_grad;
uniform float ws1;
uniform float ws2;

#define DEPTH   gl_FragCoord.z

vec3 setLighting(vec3 BaseColor) {
	vec3 diffuse = vec3(0, 0, 0);

	for(int i=0;i<NLIGHTS;i++){
		vec3 light      = normalize(gl_LightSource[i].position.xyz+EyeDirection.xyz);
		float LdotN     = dot(light,Normal.xyz);// for day side diffuse lighting
		float amplitude = 1.0/gl_LightSource[i].constantAttenuation;
		float lpn       = LdotN*amplitude;
	
		diffuse        += Diffuse.rgb*gl_LightSource[i].diffuse.rgb*lpn;
	}
	vec3 TotalDiffuse = diffuse * BaseColor * Diffuse.a;
	return TotalDiffuse;
}

//-------------------------------------------------------------

// ########## main section #########################
void main(void) {
	vec4 color;

    vec2 l_uv=gl_PointCoord.xy;
    int i=index.x+0.1;
    if(index.y>0.1){
       l_uv.x=1.0-l_uv.x;
       //l_uv.y=1-l_uv.y; //  test mode (flip upside down for better visibility)
    }
  
 	color=texture2D(samplers2d[i],l_uv);
 	color.rgb=setLighting(color.rgb);
 	//color.rgb=mix(color.rgb,Haze.rgb,gl_FragCoord.z);
#ifdef SHADOWS
     float shadow=1.0-texture2DRect(SHADOWTEX, gl_FragCoord.xy).r;
     color.rgb=mix(color.rgb,color.rgb,Shadow.rgb,shadow*Shadow.a);
#endif  
    //float depth=gl_FragCoord.z;

#ifdef HAZE
// issue
// see a dark border around sprites when haze is applied
//    - seemed to be caused by edge between alpha <1 and alpha=1
// observation
//    - can sharpen edge by raising alpha by a small power (so ~0->1)
//    - when haze is large dark border is reduced
//    - but this causes visible artifact in general (sprites dont blend well with the background)
//  work-around
//    - apply alpha power as a function of haze value
//    - improved works most haze effects but still not perfecr for all cases
	float z=DEPTH; // depth buffer
	float depth=1.0/(ws2*z+ws1); // distance
	float d=haze_grad==0?1e-4:lerp(depth,0.0,haze_grad*haze_zfar,0.0,1.0); // same as in effects.frag
	float h=haze_ampl*Haze.a*pow(d,8*haze_grad); // same as in effects.frag
	// works for haze factor > ~0.1
	color.rgb=mix(color.rgb,Haze.rgb,h);
	// improves result for haze factor <0.1 but creates edge artifacts at mid distances
	float p=lerp(h,0.0,0.8,1,0.001); // hack !
	color.a=pow(color.a,p);
#endif
    
 	gl_FragData[0]=color;
	gl_FragData[1]=vec4(0,DEPTH,0,color.a); // set type to 0 to bypass second haze correction in effects.frag

}
// ########## end clouds.frag #########################
