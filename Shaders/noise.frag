// ########## begin noise.frag #########################

uniform sampler2D permTexture;

#ifndef CPX
#ifdef _BUMPS_
//#define VNOISE
#endif
#endif

/*
 * To create offsets of one texel and one half texel in the
 * texture lookup, we need to know the texture image size.
 */
#define ONE 0.00390625
#define ONEHALF 0.001953125

// The numbers above are 1/256 and 0.5/256, change accordingly
// if you change the code to use another texture size.

#define fade(t) t*t*t*(t*(t*6.0-15.0)+10.0)

// returns classic Perlin noise and derivatives using a 2D texture

vec4 noise3D(vec3 P){
    vec4 vout=vec4(0.0);
    vec3 Pi = ONE*(floor(P));
    vec3 Pf = fract(P);

    float b00 = texture2D(permTexture, Pi.xy).a;
    float b10 = texture2D(permTexture, Pi.xy + vec2(ONE, 0.0)).a;
    float b01 = texture2D(permTexture, Pi.xy + vec2(0.0, ONE)).a;
    float b11 = texture2D(permTexture, Pi.xy + vec2(ONE, ONE)).a;
     
    // contributions from (x=0, y=0), z=0 and z=1
    vec3 grad000 = texture2D(permTexture, vec2(b00, Pi.z)).rgb * 2.0-1.0;
    vec3 grad001 = texture2D(permTexture, vec2(b00, Pi.z + ONE)).rgb * 2.0-1.0;
 
    // contributions from (x=0, y=1), z=0 and z=1
    vec3 grad010 = texture2D(permTexture, vec2(b01, Pi.z)).rgb * 2.0-1.0;
    vec3 grad011 = texture2D(permTexture, vec2(b01, Pi.z + ONE)).rgb * 2.0-1.0;

    // contributions from (x=1, y=0), z=0 and z=1
    vec3 grad100 = texture2D(permTexture, vec2(b10, Pi.z)).rgb *2.0- 1.0;
    vec3 grad101 = texture2D(permTexture, vec2(b10, Pi.z + ONE)).rgb * 2.0-1.0;

    // contributions from (x=1, y=1), z=0 and z=1
    vec3 grad110 = texture2D(permTexture, vec2(b11, Pi.z)).rgb * 2.0- 1.0;
    vec3 grad111 = texture2D(permTexture, vec2(b11, Pi.z + ONE)).rgb * 2.0-1.0;

    float n000 = dot(grad000, Pf);
    float n100 = dot(grad100, Pf - vec3(1.0, 0.0, 0.0));    
    float n010 = dot(grad010, Pf - vec3(0.0, 1.0, 0.0));
    float n110 = dot(grad110, Pf - vec3(1.0, 1.0, 0.0));    
    float n001 = dot(grad001, Pf - vec3(0.0, 0.0, 1.0));
    float n101 = dot(grad101, Pf - vec3(1.0, 0.0, 1.0));    
    float n011 = dot(grad011, Pf - vec3(0.0, 1.0, 1.0));
    float n111 = dot(grad111, Pf - vec3(1.0, 1.0, 1.0));
    
    vec4 n1=vec4(n000, n001, n010, n011);
    vec4 n2=vec4(n100, n101, n110, n111);
 
    vec3 fpt=fade(Pf);
   
   // Blend contributions along x
   
    vec4 n_x = mix(n1, n2, fpt.x);
    // Blend contributions along y
    vec2 n_xy = mix(n_x.xy, n_x.zw, fpt.y);
    // Blend contributions along z
    float n_xyz = mix(n_xy.x, n_xy.y, fpt.z);
    vout.x=n_xyz;
    
#ifdef VNOISE

#define DELTA (1.15*ONEHALF)
    
    vec3 Pid = ONE*floor(P+DELTA);
    vec3 Pfd = fract(P+DELTA);

    if(Pid.x>Pi.x)
   	 	Pfd.x=1.0;
    if(Pid.y>Pi.y)
   	 	Pfd.y=1.0;
    if(Pid.z>Pi.z)
   	 	Pfd.z=1.0;
     
    // estimate x derivative
 
    vec3 Pd=P+vec3(DELTA);
    vec3 dpt=fade(Pfd);
    
    vec4 gx1=vec4(grad000.x, grad001.x, grad010.x,  grad011.x);
    vec4 gx2=vec4(grad100.x, grad101.x, grad110.x,  grad111.x);
   
    vec4 x1=n1+gx1*DELTA;
    vec4 x2=n2+gx2*DELTA;
              
    vec4 x_delta = mix(x1,x2,dpt.x);
    vec2 vx = mix(x_delta.xy, x_delta.zw, fpt.y); 
    vout.y=(mix(vx.x, vx.y, fpt.z)-n_xyz)/DELTA;

   // estimate y derivative

    vec4 gy1=vec4(grad000.y, grad001.y, grad010.y,  grad011.y);
    vec4 gy2=vec4(grad100.y, grad101.y, grad110.y,  grad111.y);
 
    vec4 y1=n1+gy1*DELTA;
    vec4 y2=n2+gy2*DELTA;
 
    vec4 y_delta = mix(y1,y2,fpt.x);
    vec2 vy = mix(y_delta.xy, y_delta.zw, dpt.y); 
    vout.z=(mix(vy.x, vy.y, fpt.z)-n_xyz)/DELTA;

   // estimate z derivative

    vec4 gz1=vec4(grad000.z, grad001.z, grad010.z,  grad011.z);
    vec4 gz2=vec4(grad100.z, grad101.z, grad110.z,  grad111.z);
 
    vec4 z1=n1+gz1*DELTA;
    vec4 z2=n2+gz2*DELTA;
    vec4 z_delta = mix(z1,z2,fpt.x);
    vec2 vz = mix(z_delta.xy, z_delta.zw, fpt.y); 
    vout.w=(mix(vz.x, vz.y, dpt.z)-n_xyz)/DELTA;
#endif
    return vout;
}

// ########## end noise.frag #########################
