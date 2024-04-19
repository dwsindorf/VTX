#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

varying in vec4 Color_G[];
varying in vec4 Normal_G[];
varying in vec4 Factors[];
varying in vec4 TexVars_G[];

varying out vec4 Normal;
varying out vec4 Color;
varying out vec4 pnorm;
varying out vec4 TexVars;


void main(void) {

	Color=Color_G[0];
	Normal.xyz=Normal_G[0].xyz;

	vec3 ps1=gl_PositionIn[0].xyz;	
	vec3 ps2=gl_PositionIn[1].xyz;
	
	float topx=Factors[0].r;
	float topy=Factors[0].g;
	float botx=Factors[0].b;
	float boty=Factors[0].a;
	
	float nscale=TexVars_G[0].w;

	TexVars=TexVars_G[0];
	
	
	float dx2=nscale*topx;
	float dx1=nscale*botx;
	
	vec3 norm;
   
	norm=vec3(dx2,0,1);
	norm=normalize(norm);
    pnorm.xyz = gl_NormalMatrix * norm;
    gl_Position = vec4(ps2.x-topx,ps2.y-topy,ps2.z,1); // top-left
    gl_TexCoord[0].xy=vec2(0,0);
    EmitVertex();
   
    norm=vec3(-dx2,0.0,1.0);
    norm=normalize(norm);
    pnorm.xyz = gl_NormalMatrix * norm;
    gl_Position = vec4(ps2.x+topx,ps2.y+topy,ps2.z,1); // top-right  
    gl_TexCoord[0].xy=vec2(1,0);
    EmitVertex();
    
     
    norm=vec3(dx1,0.0,1.0);
    norm=normalize(norm);
    pnorm.xyz = gl_NormalMatrix * norm;
    gl_TexCoord[0].xy=vec2(0,1);
    gl_Position = vec4(ps1.x-botx,ps1.y-boty,ps1.z,1);  // bot-left 
    EmitVertex();
    
    norm=vec3(-dx1,0.0,1.0);
    norm=normalize(norm);
    pnorm.xyz = gl_NormalMatrix * norm;
    gl_TexCoord[0].xy=vec2(1,1);
    gl_Position = vec4(ps1.x+botx,ps1.y+boty,ps1.z,1);  // bot-right 
    EmitVertex(); 
 
  	norm=vec3(dx2,0.0,1.0);
	norm=normalize(norm);
    pnorm.xyz = gl_NormalMatrix * norm;
    gl_TexCoord[0].xy=vec2(0,0);
    gl_Position = vec4(ps2.x-topx,ps2.y-topy,ps2.z,1); // top-left  
    EmitVertex();

    //EndPrimitive();
    
}

