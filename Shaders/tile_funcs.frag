#ifdef NOTILE
float sum( vec4 v ) { return v.x+v.y+v.z; }
// note: needs shader version 130 to compile (for textureGrad)
// ref https://www.iquilezles.org/www/articles/texturerepetition/texturerepetition.htm
// modified "Technique 3" to use perlin noise texture lookup for random index
vec4 textureNoTile( int id, sampler2D samp, in vec2 x , float mm)
{
    v1= Vertex1.xyz;
    v2= Vertex2.xyz;
 
    float scale =tex2d[id].scale;
    vec4 P1=noise3D(v1*scale);
    
    // compute index    
    float index = P1.x*8.0;
    float i = floor( index );
    float f = fract( index );

    // offsets for the different virtual patterns    
    vec2 offa = sin(vec2(3.0,7.0)*(i+0.0)); // can replace with any other hash    
    vec2 offb = sin(vec2(3.0,7.0)*(i+1.0)); // can replace with any other hash    

    // compute derivatives for mip-mapping    
    vec2 dx = dFdx(x), dy = dFdy(x);
    
    // sample the two closest virtual patterns    
    vec4 cola = textureGrad( samp, x + offa, dx, dy );
    vec4 colb = textureGrad( samp, x + offb, dx, dy );

    // interpolate between the two virtual patterns    

    vec4 result=mix( cola, colb, smoothstep(0.2,0.8,f-0.1*sum(cola-colb)) );

    return result;
}

#endif
