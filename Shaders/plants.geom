#extension GL_EXT_geometry_shader : enable
#extension GL_EXT_geometry_shader4 : enable

varying in vec4 Color_G[1];
varying in vec4 Norm_G[1];
varying in vec4 EyeDirection_G[1];
varying in vec4 Factors[1];
varying in vec4 TexVars[1];

varying out vec4 EyeDirection;
varying out vec4 Norm;
varying out vec4 Color;
varying out vec4 pnorm;

void main(void) {

	Norm=Norm_G[0];
 
	EyeDirection=EyeDirection_G[0];
	Color=Color_G[0];

	vec3 ps1=gl_PositionIn[0].xyz;	
	vec3 ps2=gl_PositionIn[1].xyz;
	
	float topx=Factors[0].r;
	float topy=Factors[0].g;
	float botx=Factors[0].b;
	float boty=Factors[0].a;
	
	float nscale=TexVars[0].w;
	
	float dx2=nscale*topx;
	float dx1=nscale*botx;
	
	vec3 norm;

	norm=vec3(dx2,0,1);
	norm=normalize(norm);
    pnorm.xyz = gl_NormalMatrix * norm;
    gl_Position = vec4(ps2.x-topx,ps2.y-topy,ps2.z,1); // top-left  
    EmitVertex();
   
    norm=vec3(-dx2,0.0,1.0);
    norm=normalize(norm);
    pnorm.xyz = gl_NormalMatrix * norm;
    gl_Position = vec4(ps2.x+topx,ps2.y+topy,ps2.z,1); // top-right  
    EmitVertex();
     
    norm=vec3(-dx1,0.0,1.0);
    norm=normalize(norm);
    pnorm = gl_NormalMatrix * norm;
    gl_Position = vec4(ps1.x+botx,ps1.y+boty,ps1.z,1);  // bot-right 
    EmitVertex(); 
   
    norm=vec3(dx1,0.0,1.0);
    norm=normalize(norm);
    pnorm = gl_NormalMatrix * norm;
    gl_Position = vec4(ps1.x-botx,ps1.y-boty,ps1.z,1);  // bot-left 
    EmitVertex();
  
  	norm=vec3(dx2,0.0,1.0);
	norm=normalize(norm);
    pnorm = gl_NormalMatrix * norm;
    gl_Position = vec4(ps2.x-topx,ps2.y-topy,ps2.z,1); // top-left  
    EmitVertex();
    
    EndPrimitive();
    
}

