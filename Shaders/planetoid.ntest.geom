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

#ifndef TEST
#define TEST 0
#endif
// received from vertex shader

varying in vec4 Normal_G[];
varying in vec4 Constants_G[];
varying in vec4 Color_G[];
#if NVALS >0
varying in vec4 Vertex1_G[];
#endif
#if NTEXS >0
varying in vec4 Tangent_G[];
#endif
varying in vec4 Attributes_G[][2];

// forward to fragment shader 
varying out vec4 Color;
varying out vec4 Normal;
varying out vec4 EyeDirection;
varying out vec4 Constants1;
#if NVALS >0
varying out vec4 Vertex1;
#endi
#if NTEXS >0
varying out vec4 Tangent;
#endif

varying out vec4 attributes[2];

vec3 NormalFromTriangleVertices(vec3 triangleVertices[3])
{
    vec3 u = triangleVertices[1] - triangleVertices[0];
    vec3 v = triangleVertices[2] - triangleVertices[0];
    return gl_NormalMatrix * cross(v, u);
}

void main(void) {
    vec3 Vertex[3];
    vec3 computedNormal;

	int i;
	for(i=0; i< gl_VerticesIn; i++){
	    vec4 p=gl_PositionIn[i];
	    gl_Position=gl_ModelViewProjectionMatrix * p;
#if TEST >0 
        // calculate normals using vertex cross products
        // problem 1 can only compute one normal per face which results in "faceted" look 
        //           (as if vertex blending was disabled)
        //          - could use adjacency info but would need to change glBegin calls from TRIANGLE_FAN
        //            to TRIANGLES_ADJACENCY
		// problem 2 (fatal flaw?) when triangles get too small get s.p.f.p failures (black triangles)
	
        Vertex[0]=gl_PositionIn[0].xyz;
        Vertex[1]=gl_PositionIn[1].xyz;
        Vertex[2]=gl_PositionIn[2].xyz;
	   
        computedNormal = NormalFromTriangleVertices(Vertex);
		Normal=vec4(computedNormal,Normal_G[i].w);
#else
		Normal=Normal_G[i];
#endif
		Constants1=Constants_G[i];
		EyeDirection=-(gl_ModelViewMatrix * p);

#if NVALS >0
		Vertex1=Vertex1_G[i];
#endif
#ifdef COLOR
		Color=Color_G[i];
#endif
#if NTEXS >0
		Tangent=Tangent_G[i];
		for(int j=0;j<NTEXS;j++)	
			gl_TexCoord[j]=gl_TexCoordIn[i][j];
#endif
		Constants1=Constants_G[i];	
		for(int j=0;j<2;j++)
			attributes[j]=Attributes_G[j][i];
		EmitVertex();
	}
	EndPrimitive();
}
