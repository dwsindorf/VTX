#include "utils.h"
uniform sampler2DRect FBOTex1;
uniform sampler2DRect FBOTex2;
uniform sampler2DRect FBOTex3;
uniform sampler2DRect FBOTex4;
uniform vec2 tc_offset[25];

// various filter kernels
// --- Sorbel ----------------------
//    -1 -2 -1       1 0 -1 
// H = 0  0  0   V = 2 0 -2		result = sqrt(H^2 + V^2)
//     1  2  1       1 0 -1
// 
// --- Prewitt ----------------------
//    -1 -1 -1       1 0 -1 
// H = 0  0  0   V = 1 0 -1		result = sqrt(H^2 + V^2)
//     1  1  1       1 0 -1
// 
//--- standard Laplace --------------
//   -1 -1 -1
//   -1  8 -1
//   -1 -1 -1
// 
//---  modified Laplace -------------
//   -1 -2  -1
//   -2  12 -2
//   -1 -2  -1

#ifdef FILTER1 
#define BIG_KERNEL
#endif
//#undef BIG_KERNEL

#ifdef ONEPASS
#define IMAGE FBOTex1
#define FLAGS FBOTex2
#else
#define IMAGE FBOTex3
#define FLAGS FBOTex4
#endif
uniform float color_ampl;
uniform float normal_ampl;
uniform float ws1;
uniform float ws2;
uniform float zf;
uniform float zn;
uniform float hdr_min;
uniform float hdr_max;

void main(void) {
	vec3 color = texture2DRect(IMAGE, gl_FragCoord.xy).rgb;
	vec3 edge_sum;
	float ss=texture2DRect(FLAGS, gl_FragCoord.xy).r;
	float c=0;
	float exposure=1;
	
#ifdef HDR
#ifdef BIG_KERNEL
	vec3 hdrSample[25];
    for (int i = 0; i < 25; i++){
		hdrSample[i] = texture2DRect(IMAGE, gl_FragCoord.xy + tc_offset[i]).rgb;
	}
	// 5x5 Gaussian filter kernel (HDR average local intensity)
    vec3 kernelcolor = (
           (1.0  * (hdrSample[0] + hdrSample[4] + hdrSample[20] + hdrSample[24])) +
           (4.0  * (hdrSample[1] + hdrSample[3] + hdrSample[5] + hdrSample[9] +
                    hdrSample[15] + hdrSample[19] + hdrSample[21] + hdrSample[23])) +
           (7.0  * (hdrSample[2] + hdrSample[10] + hdrSample[14] + hdrSample[22])) +
           (16.0 * (hdrSample[6] + hdrSample[8] + hdrSample[16] + hdrSample[18])) +
           (26.0 * (hdrSample[7] + hdrSample[11] + hdrSample[13] + hdrSample[17])) +
           (41.0 * hdrSample[12])
           ) / 273.0;
#endif
#endif    
	vec3 pixel[9];
	float nmask[9];
	float smask[9];
    float surface=texture2DRect(FLAGS, gl_FragCoord.xy).r;
    float z=texture2DRect(FLAGS, gl_FragCoord.xy).b;
    float surface_sum=0.0;
    vec4 flags;
    for (int k = 0; k < 3; k++){       
	    for (int j = 0; j < 3; j++){
	        int i=j*3+k;
	        int l=k+1+(j+1)*5;
#ifdef HDR
#ifdef BIG_KERNEL
	    	pixel[i] = hdrSample[l];
#else
       	 	pixel[i] = texture2DRect(IMAGE, gl_FragCoord.xy + tc_offset[l]).rgb;
#endif
#else
            pixel[i] = texture2DRect(IMAGE, gl_FragCoord.xy + tc_offset[l]).rgb;
#endif
	        flags=texture2DRect(FLAGS, gl_FragCoord.xy + tc_offset[l]);
	        nmask[i] = flags.b;        // dot(normal-eye)
	        smask[i] = flags.a;        // type flag
	        surface_sum += flags.g>0.0?1:0;  // ave illumination
	    }
    }
    float dh = nmask[2] + nmask[5] + nmask[8]-nmask[0]- nmask[3]-nmask[6];
    float dv = nmask[0] + nmask[1] + nmask[2]-nmask[6]- nmask[7]-nmask[8];
    float dn = sqrt(dh * dh + dv * dv);
    dh = smask[2] + smask[5] + smask[8]-smask[0]- smask[3]-smask[6];
    dv = smask[0] + smask[1] + smask[2]-smask[6]- smask[7]-smask[8];
    float dt = sqrt(dh * dh + dv * dv);
    dt=clamp(dt,0.0,1.0);

//   1 2 1
//   2 3 2   /15 = gaussian average
//   1 2 1
    vec3 ave =(pixel[0]+2.0*pixel[1]+pixel[2]+2.0*pixel[3]+3.0*pixel[4]+2.0*pixel[5]+pixel[6]+2.0*pixel[7]+pixel[8])/15.0;

//--- standard Laplace --------------
//   -1 -1 -1
//   -1  8 -1
//   -1 -1 -1
// 
    vec3 delc;
    vec3 ctr=pixel[4];
    for (int k = 0; k < 9; k++){ 
       delc+=pixel[k]-ctr;
    }
    c=2*length(delc);  // color diff in adjacent pixels
#ifdef HDR
#ifndef BIG_KERNEL
	float kernelLuminance = dot(ave, vec3(0.3, 0.59, 0.11));
#else
	float kernelLuminance = dot(kernelcolor.rgb, vec3(0.3, 0.59, 0.11));
#endif
   // if(ss>0)
	exposure = sqrt(hdr_max / (2.5*kernelLuminance + 0.1*hdr_min));
	
	ave *= exposure;
	color *= exposure;
	
#endif
#ifdef EDGES    
    if(surface>0.5 && surface <1.9) // blur water
    	c*=10;
    float blend=(color_ampl*c+normal_ampl*dn+dt);
#ifdef SHOW                          
    gl_FragData[0].rgb=mix(0.5*color,vec3(color_ampl*c,normal_ampl*dn,dt),clamp(blend,0.0,1.0));
#else
    //if(ss==0)
    //gl_FragData[0].rgb=vec3(ss/2.0,0,0);
    //else

    gl_FragData[0].rgb=mix(color,ave,clamp(blend,0.0,1.0));  
#endif
#else
	  gl_FragData[0].rgb=color;		
#endif
    gl_FragData[0].a = 1.0;
}

