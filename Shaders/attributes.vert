// ########## begin attributes.vert #########################
attribute vec4 CommonAttributes1;
attribute vec4 TextureAttributes;  // TextureAttributes.xy contains 2d components of vertex tangent

//#if NVALS >0
attribute vec4 Position1;
attribute vec4 Position2;
#ifdef NOISE_DP
attribute vec4 Position3;  // fan pivot (Position2 is the offset from it)
#endif
//#endif
attribute vec4 Attributes3;
attribute vec4 Attributes4;

// ########## end attributes.vert #########################
