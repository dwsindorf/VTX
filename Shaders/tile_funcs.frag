#ifdef NOTILE

// without this call de-aliasing textures (tile_funcs) take big performance drop (4-5x)
vec4 warmup(){
   // notes:
   // 1) brittle code: everything seems to be needed here
   // 2) this somehow must fool the ogl compiler into doing something more efficiently
   // 3) seems to affect performance more in "adapt" pass than render pass

	v1= Vertex1.xyz; // required for mapping to work
	vec4 result=vec4(0.0);
	vec4 val;
	
	float weight=0.0;
 
	for(int i=0;i<2;i++) {
        vec4 P1=noise3D(v1*2);
		val=P1;//nvec*weight;
		result += val;
		if(weight<=0.0) // will always exit here on first loop pass
		    break;
		weight*=val.x; // but this is still needed to fix problem (!)
	} 
	return result;	
}

float sum( vec4 v ) { return v.x+v.y+v.z; }

// notes: 1) needs shader version 130 to compile (for textureGrad)
//        2) ref https://www.iquilezles.org/www/articles/texturerepetition/texturerepetition.htm
//           modified "Technique 3" to use perlin noise texture lookup for random index
//        3) impacts rendering speed by ~2-3x (slower)

vec4 textureNoTile( int id, in vec2 uv, float mm)
{
    sampler2D samp=samplers2d[id];
    float scale =0.7123*tex2d[id].scale;
    vec3 P=v1*scale;
    float r=noise3D(P).x; 
    
    // compute index    

    float index =r*8.0;
    float i = floor( index );
    float f = fract( index );

    // offsets for the different virtual patterns    
    vec2 offa = sin(vec2(3.0,7.0)*(i+0.0)); // can replace with any other hash    
    vec2 offb = sin(vec2(3.0,7.0)*(i+1.0)); // can replace with any other hash   
     
    // sample the two closest virtual patterns    

    vec4 cola = texture2D(samp, uv + offa,mm);
    vec4 colb = texture2D(samp, uv + offb,mm); 
    
    // interpolate between the two virtual patterns    

    vec4 result=mix(cola, colb, smoothstep(0.2,0.8,f-0.1*sum(cola-colb)));

    return result;
}
vec4 triplanarNoTile(int id, vec4 pos, float mm) {
    sampler2D samp=samplers2d[id];
    vec3 N = normalize(WorldNormal);
    vec3 blendWeights = abs(N);
     
    blendWeights = blendWeights / (blendWeights.x + blendWeights.y + blendWeights.z);
    
    vec3 V = pos.xyz;
    
    vec4 xProj = textureNoTile(id, V.yz, mm);
    vec4 yProj = textureNoTile(id, V.xz, mm);
    vec4 zProj = textureNoTile(id, V.xy, mm);
    
    return xProj * blendWeights.x +
           yProj * blendWeights.y +
           zProj * blendWeights.z;
}

#endif
