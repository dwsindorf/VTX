// ########## begin set_common.vert #########################
#ifdef COLOR
	Color=gl_Color;
#endif

#if NTEXS >0
	Tangent=TextureAttributes;
#endif

//#if NVALS >0
	Vertex1=Position1;
//#endif
	Constants1=CommonAttributes1;
// ########## end set_common.vert #########################
