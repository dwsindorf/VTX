// notes:
// 1. second shader in pipeline
// 2. input from vertex shader
// 3. output to fragment shader
// 
// development plan
// 1. simple pass-through
// 2. tesselate only (1 triangle->4)
//  - no displacement
//  - since normals etc aren't change should not affect polygon appearances
//  - In lines mode, should show extra triangles
// 3. tesselate only (multilevel)
//  - use uniforms to pass in number of levels etc.
// 4. displacement 1 (using noise function)
//  - turn on if point noise has "fragment" property set
//  - move vertex in in z direction (normal direction?)
//  - use face direction for normals
// 5. displacement 2 (using noise function)
//  - add better support for lighting (ave norms etc)
//  - addaptive subdivision level ?

//#version 120
//#extension GL_EXT_geometry_shader4 : enable

// received from vertex shader

varying in vec4 Normal_G[];
varying in vec4 EyeDirection_G[];
varying in vec4 Constants_G[];
varying in vec4 Color_G[];
#if NVALS >0
varying in vec4 Vertex1_G[];
varying in vec4 Vertex2_G[];
#endif
#if NTEXS >0
varying in vec4 Tangent_G[];
#endif

varying in vec4 Attributes1_G[];
varying in vec4 Attributes2_G[];

// forward to fragment shader 
varying out vec4 Color;
varying out vec4 Normal;
varying out vec4 EyeDirection;
varying out vec4 Constants;
#if NVALS >0
varying out vec4 Vertex1;
varying out vec4 Vertex2;
#endif
#if NTEXS >0
varying out vec4 Tangent;
#endif

varying out vec4 attributes[2];

void main(void) {
	int i;
	for(i=0; i< gl_VerticesIn; i++){
		gl_Position = gl_PositionIn[i];

		Normal=Normal_G[i];
		Constants=Constants_G[i];
		EyeDirection=EyeDirection_G[i];
#ifdef COLOR
		Color=Color_G[0];
#endif

#if NTEXS >0
		Tangent=Tangent_G[i];
#endif

#if NVALS >0
		Vertex1=Vertex1_G[i];
		Vertex2=Vertex1_G[i];
#endif
		Constants=Constants_G[i];

    	gl_TexCoord[0]=gl_TexCoordIn[i][0];
    	gl_TexCoord[1]=gl_TexCoordIn[i][1];
    	gl_TexCoord[2]=gl_TexCoordIn[i][2];
    	
		attributes[0]=Attributes1_G[i];
		attributes[1]=Attributes2_G[i];
		EmitVertex();
	}
	EndPrimitive();
}
