
varying in vec4 Color_G[1];
varying in vec4 Normal_G[1];
varying in vec4 EyeDirection_G[1];
varying in vec4 Constants_G[1];
varying in vec4 Factors[1];

varying out vec4 Color;
varying out vec4 Normal;
varying out vec4 EyeDirection;
varying out vec4 Constants1;

uniform float INVROWS;
uniform float ROWS;

void main(void) {
	int i;
	float ta=Factors[0].x;  // angle
	float ts=Factors[0].y;  // offset

	Normal=Normal_G[0];
	Constants1=Constants_G[0];
	EyeDirection=EyeDirection_G[0];
	Color=Color_G[0];

	vec3 ps=gl_PositionIn[0].xyz;

	float pa=Factors[0].z; // random position angle
	float pr=Factors[0].w; // position offset from center

	float cosp=pr*cos(pa);
	float sinp=pr*sin(pa);

	ps.xy+=vec2(cosp,sinp);

	float d=Constants1.w;

	float cosa=ts*cos(ta);
	float sina=ts*sin(ta);

	float ty=floor(d*INVROWS);
	float tx=floor(d-ROWS*ty);

	// 0
	gl_Position = vec4(ps.x+sina,ps.y+cosa,ps.z,1);
	gl_TexCoord[0].xy=vec2(tx,ty);
	EmitVertex();

	// 3
	gl_Position = vec4(ps.x-cosa,ps.y+sina,ps.z,1);
	gl_TexCoord[0].xy=vec2(tx,1.0+ty);
	EmitVertex();

	// 1
	gl_Position = vec4(ps.x+cosa,ps.y-sina,ps.z,1);
	gl_TexCoord[0].xy=vec2(1.0+tx,ty);
	EmitVertex();

	// 2
	gl_Position = vec4(ps.x-sina,ps.y-cosa,ps.z,1);
	gl_TexCoord[0].xy=vec2(1.0+tx,1.0+ty);
	EmitVertex();


	EndPrimitive();

}
