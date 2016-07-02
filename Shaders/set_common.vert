// ########## begin set_common.vert #########################
#ifdef COLOR
	Color=gl_Color;
#endif

#if NTEXS >0
	Tangent=TextureAttributes;
#endif

#if NVALS >0
	Vertex1=Position1;
	Vertex2=Position2;
#endif
	Constants=CommonAttributes;
// ########## end set_common.vert #########################
