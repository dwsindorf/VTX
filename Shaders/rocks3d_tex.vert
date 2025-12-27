#version 120

//#include "attributes.h"
#include "textures.h"
#include "attributes.vert"
#include "common.h"

uniform float night_ligting;
uniform float textureScale;
varying vec3 Normal;
varying vec3 EyeDirection;
varying vec3 WorldPos;
varying vec3 TemplatePos;
varying vec4 WorldNormal;
attribute vec3 templatePosition;  // Attribute location 3
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

void main() {
    gl_Position = ftransform();
     
    // Normal in eye space
    Normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Eye direction (position in eye space)
    EyeDirection = -(gl_ModelViewMatrix * gl_Vertex).xyz;
    TemplatePos = templatePosition;  // Pass through
    
     WorldPos = templatePosition;
#ifdef COLOR
    Color = gl_Color;
#endif
   
//#include "set_tex.vert"
//#include "set_common.vert"
#ifdef TX0
    gl_TexCoord[0] = vec4(templatePosition*tex2d[0].scale, 1.0);
    WorldNormal.xyz = gl_TexCoord[0].xyz;
#endif
#ifdef TX1
    gl_TexCoord[1] = vec4(templatePosition, 1.0);
#endif
    // Normal = gl_Normal.xyz;
    //WorldNormal.xyz = gl_Normal;
    
}