#version 120

#include "textures.h"
#include "attributes.vert"
#include "common.h"

uniform float night_ligting;
uniform float textureScale;
uniform float wscale;
uniform vec4 xpoint;
varying vec3 Normal;
varying vec3 EyeDirection;
varying vec3 WorldPos;
varying vec3 TemplatePos;
varying vec3 WorldNormal;
attribute vec4 templatePosition;  // Attribute location 3
attribute vec4 faceNormal;        // Attribute location 4
#if NTEXS >0
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
	float near_bias; 	 // low frequency bias
	float far_bias; 	 // high frequency bias
	float tilt_bias;     // tilt bias	
	bool  randomize;     // randomized texture	
	bool  seasonal;      // seasonal	
	bool  t1d;           // 1d texture
	bool  triplanar;     // triplanar mapping	
	bool  t3d;           // 3d mapping	
	int   instance;      // placement instance
	int   active;        // placement instance	
};
uniform tex2d_info tex2d[NTEXS];
#endif

void main() {

    float depth=templatePosition.w;
    float dfactor = 10* wscale / depth;
    float max_orders = log2(dfactor);
    
    gl_Position = ftransform();
     
    // Normal in eye space
    Normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Eye direction (position in eye space)
    EyeDirection = -(gl_ModelViewMatrix * gl_Vertex).xyz;
    TemplatePos = templatePosition.xyz;  // Pass through
    
    Tangent.w = max_orders;
    
     WorldPos = normalize(xpoint.xyz);
     WorldNormal=normalize(templatePosition.xyz);
#ifdef COLOR
     Color = gl_Color;
#endif

#if NTEXS >0
	for(int i=0;i<NTEXS;i++){
	    float scale=tex2d[i].scale* depth*40;
	    vec3 coords=templatePosition.xyz*scale;
	    if(tex2d[i].t1d)
		   coords+=tex2d[i].bias; 	    
		gl_TexCoord[i] = vec4(coords, depth);
	}
	WorldNormal.xyz = faceNormal.xyz;//gl_TexCoord[0].xyz;
	Tangent.z=abs(faceNormal.w);
#endif    
}