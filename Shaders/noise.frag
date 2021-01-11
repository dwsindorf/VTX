// ########## begin noise.frag #########################

uniform sampler2D permTexture;
uniform sampler2D gradTexture;


/*
 * To create offsets of one texel and one half texel in the
 * texture lookup, we need to know the texture image size.
 */
#define ONE 0.00390625
#define ONEHALF 0.001953125

#define fade(t) t*t*t*(t*(t*6.0-15.0)+10.0)
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}
// fast psuedo-noise lookup
float fnoise (in vec2 st){
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

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

//	Simplex 3D Noise 
//	by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

vec4 simplex3d(vec3 v){ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  vec4 vout;
  vout.x=36* dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  return vout;
}


// The numbers above are 1/256 and 0.5/256, change accordingly
// if you change the code to use another texture size.

const mat2 myt = mat2(.12121212, .13131313, -.13131313, .12121212);
const vec2 mys = vec2(1e4, 1e6);

vec2 rhash(vec2 uv) {
  uv *= myt;
  uv *= mys;
  return fract(fract(uv / mys) * uv);
}


float voronoi2d(const in vec2 point) {
  vec2 p = floor(point);
  vec2 f = fract(point);
  float res = 0.0;
  for (int j = -1; j <= 1; j++) {
    for (int i = -1; i <= 1; i++) {
      vec2 b = vec2(i, j);
      vec2 r = vec2(b) - f + rhash(p + b);
      res += 1. / pow(dot(r, r), 8.);
    }
  }
  return pow(1. / res, 0.0625);
}

vec3 hash(vec3 p) {
  return fract(
      sin(vec3(
       dot(p, vec3(1.0, 57.0, 113.0)),
       dot(p, vec3(57.0, 113.0, 1.0)),
       dot(p, vec3(113.0, 1.0, 57.0)))) *
       43758.5453);
}

vec4 voronoi3d(const in vec3 x) {
  vec3 p = floor(x);
  vec3 f = fract(x);

  vec2 res = vec2(100.0);
  for (int k = -1; k <= 1; k++) {
    for (int j = -1; j <= 1; j++) {
      for (int i = -1; i <= 1; i++) {
        vec3 b = vec3(float(i), float(j), float(k));
        vec3 r = vec3(b) - f + hash(p + b);
        float d = dot(r, r);

        float cond = max(sign(res.x - d), 0.0);
        float nCond = 1.0 - cond;

        float cond2 = nCond * max(sign(res.y - d), 0.0);
        float nCond2 = 1.0 - cond2;

        res = vec2(d, res.x) * cond + res * nCond;
        res.y = cond2 * d + nCond2 * res.y;
      }
    }
  }

  return vec4(2*(res-0.5),0,0);
}
// ########## end noise.frag #########################
