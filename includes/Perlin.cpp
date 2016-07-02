#include <math.h>
#include <stdio.h>
#include "defs.h"
#include "Perlin.h"


extern int IntRand();
extern int setRandSeed(int);

//************************* from Perlin *********************
//#define LONGLONG
#define OLD
#ifdef LONGLONG
#define NLONG long
#else
#define NLONG int
#endif

/* noise functions over 1, 2, and 3 dimensions */
//#define B2K super size it

#ifdef B2K
#define B 0x1000
#define BM 0xfff
#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff
#else
#define B 0x100
#define BM 0xff
#define N 0x100
#define NP 8   /* 2^N */
#define NM 0xff

#endif


const double norm_factor=1.0;//0.77;  // adjust for value range
static int P[B + B + 2];
static double G4[4*B + 4*B + 8];
static double G3[B + B + 2][3];
static double G2[B + B + 2][2];
static double G1[B + B + 2];


bool Perlin::initialized=false;

int Perlin::noiseDim(){
	return B;
}
double *Perlin::gradTable(int dim){
	initialize();
	switch (dim){
	case 1: return (double*)G1;
	case 2: return (double*)G2;
	case 3: return (double*)G3;
	case 4: return (double*)G4;
	}
	return 0;
}
int *Perlin::permTable(){
	initialize();
	return P;
}

#ifdef OLDPERLIN
#define fade(t) ( t * t * (3. - 2. * t) )
#else
#define fade(t) ( t * t * t* (t*(t*6-15)+10) )
#endif

#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	t = vec[i]; \
	b0 = ((NLONG)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (NLONG)t;\
	r1 = r0 - 1.0;

double Perlin::noise1(double arg)
{
	int bx0, bx1;
	double rx0, rx1, sx, t, u, v, vec[1];

	vec[0] = arg;

	setup(0, bx0,bx1, rx0,rx1);

	sx = fade(rx0);

	u = rx0 * G3[ P[ bx0 ] ][0];
	v = rx1 * G3[ P[ bx1 ] ][0];

	return norm_factor*lerp(sx, u, v);
}

double Perlin::noise2(double vec[2])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);

	i = P[ bx0 ];
	j = P[ bx1 ];

	b00 = P[ i + by0 ];
	b10 = P[ j + by0 ];
	b01 = P[ i + by1 ];
	b11 = P[ j + by1 ];

	sx = fade(rx0);
	sy = fade(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = &G3[ b00 ][0] ; u = at2(rx0,ry0);
	q = &G3[ b10 ][0] ; v = at2(rx1,ry0);
	
	a = lerp(sx, u, v); // returned by noise1

	q = &G3[ b01 ][0] ; u = at2(rx0,ry1);
	q = &G3[ b11 ][0] ; v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return norm_factor*lerp(sy, a, b);
}

double Perlin::noise3(double vec[3])
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, rz0, rz1, *q, sx, sy, sz,t, u, v;
	double n_x[4],n_xy[2],n_xyz;
	register int i, j;
	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);

	i = P[ bx0 ];
	j = P[ bx1 ];
	b00 = P[ i + by0 ];  // b00=perm00
	b10 = P[ j + by0 ];  // b10=perm10
	b01 = P[ i + by1 ];  // b01=perm01
	b11 = P[ j + by1 ];  // b11=perm11

	sx = fade(rx0);
	sy = fade(ry0);
	sz = fade(rz0);

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

	q = &G3[ b00 + bz0 ][0] ; u = at3(rx0,ry0,rz0); // q=grad000
	q = &G3[ b10 + bz0 ][0] ; v = at3(rx1,ry0,rz0); // q=grad100
	n_x[0] = lerp(sx, u, v);  // n_x.x

	q = &G3[ b01 + bz0 ][0] ; u = at3(rx0,ry1,rz0); // q=grad010
	q = &G3[ b11 + bz0 ][0] ; v = at3(rx1,ry1,rz0); // q=grad110
	n_x[2] = lerp(sx, u, v);  // n_x.z

	n_xy[0] = lerp(sy, n_x[0], n_x[2]);  // n_xy.x

	q = &G3[ b00 + bz1 ][0] ; u = at3(rx0,ry0,rz1); // q=grad001
	q = &G3[ b10 + bz1 ][0] ; v = at3(rx1,ry0,rz1); // q=grad101
	n_x[1] = lerp(sx, u, v);  // n_x.y

	q = &G3[ b01 + bz1 ][0] ; u = at3(rx0,ry1,rz1); // q=grad011
	q = &G3[ b11 + bz1 ][0] ; v = at3(rx1,ry1,rz1); // q=grad111
	n_x[3] = lerp(sx, u, v);  // n_x.w

	n_xy[1] = lerp(sy, n_x[1], n_x[3]);  // n_xy.y

	n_xyz=lerp(sz, n_xy[0], n_xy[1]);

	return norm_factor*n_xyz;
}
// 4D noise (extrapolated from noise3)

double Perlin::noise4(double vec[4])
{
	int bx0, bx1, by0, by1, bz0, bz1, bw0, bw1, b00, b10, b01, b11;
	double rx0, rx1, ry0, ry1, rz0, rz1, rw0, rw1, *q, sx, sy, sz,sw;
	double a, b, c, d, e, f, t, u, v;
	register int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);
	setup(3, bw0,bw1, rw0,rw1);

	i = P[ bx0 ];
	j = P[ bx1 ];

	b00 = P[ i + by0 ];
	b10 = P[ j + by0 ];
	b01 = P[ i + by1 ];
	b11 = P[ j + by1 ];

	sx = fade(rx0);
	sy = fade(ry0);
	sz = fade(rz0);
	sw = fade(rw0);

#define at4(rx,ry,rz,rw) ( rx * q[0] + ry * q[1] + rz * q[2] + rw * q[3])

	q = &G4[ b00 + bz0 + bw0] ; u = at4(rx0,ry0,rz0,rw0);  // 0
	q = &G4[ b10 + bz0 + bw0] ; v = at4(rx1,ry0,rz0,rw0);  // 1
	
	a = lerp(sx, u, v); // returned by noise1
	
	// start 2D

	q = &G4[ b01 + bz0 + bw0] ; u = at4(rx0,ry1,rz0,rw0);  // 3
	q = &G4[ b11 + bz0 + bw0] ; v = at4(rx1,ry1,rz0,rw0);  // 3
	b = lerp(sx, u, v);

	c = lerp(sy, a, b);  // returned by noise2
	
	// start 3D

	q = &G4[ b00 + bz1 + bw0] ; u = at4(rx0,ry0,rz1,rw0);  // 4
	q = &G4[ b10 + bz1 + bw0] ; v = at4(rx1,ry0,rz1,rw0);  // 5
	a = lerp(sx, u, v);

	q = &G4[ b01 + bz1 + bw0] ; u = at4(rx0,ry1,rz1,rw0);  // 6
	q = &G4[ b11 + bz1 + bw0] ; v = at4(rx1,ry1,rz1,rw0);  // 7
	b = lerp(sx, u, v);

	d = lerp(sy, a, b);
	
	e = lerp(sz, c, d);  // returned by noise3

	// start 4D
	
	q = &G4[ b00 + bz0 + bw1] ; u = at4(rx0,ry0,rz0,rw1);  // 8
	q = &G4[ b10 + bz0 + bw1] ; v = at4(rx1,ry0,rz0,rw1);  // 9
	a = lerp(sx, u, v);

	q = &G4[ b01 + bz0 + bw1] ; u = at4(rx0,ry1,rz0,rw1);  // A
	q = &G4[ b11 + bz0 + bw1] ; v = at4(rx1,ry1,rz0,rw1);  // B
	b = lerp(sx, u, v);

	c = lerp(sy, a, b);

	q = &G4[ b00 + bz1 + bw1] ; u = at4(rx0,ry0,rz1,rw1);  // C
	q = &G4[ b10 + bz1 + bw1] ; v = at4(rx1,ry0,rz1,rw1);  // D
	a = lerp(sx, u, v);

	q = &G4[ b01 + bz1 + bw1] ; u = at4(rx0,ry1,rz1,rw1);  // E
	q = &G4[ b11 + bz1 + bw1] ; v = at4(rx1,ry1,rz1,rw1);  // F
	b = lerp(sx, u, v);

	d = lerp(sy, a, b);

	f = lerp(sz, c, d);

	return norm_factor*lerp(sw, e, f);
}

void Perlin::initialize()
{
	int i, j, k;
	double *q=&G4[0];
	
	if(initialized)
		return;
	
	setRandSeed(1);

	for (i = 0 ; i < B ; i++) {
        double dp=2;
        double r;
		P[i] = i;
		
		// build gradient noise vectors

		// 1-D noise

		G1[i] = (double)((IntRand() % (B + B)) - B) / B;

		// 2-D noise

		dp=2;
        while(dp>1){  // reject vectors outside unit sphere
            dp=0;
			for (j = 0 ; j < 2 ; j++){
				r=(double)((IntRand() % (B + B)) - B) / B;
				G2[i][j] = r;
				dp+=r*r;
			}
		}
		dp=sqrt(dp);
		for (j = 0 ; j < 2 ; j++)
			G2[i][j]/=dp;
		
		// 3-D noise
		
		dp=2;
        while(dp>1){
            dp=0;
			for (j = 0 ; j < 3 ; j++){
		    	r=(double)((IntRand() % (B + B)) - B) / B;
				G3[i][j] = r;
				dp+=r*r;
			}
		}
	    dp=sqrt(dp);
		for (j = 0 ; j < 3 ; j++)
			G3[i][j]/=dp;

		// 4-D noise
    
		dp=2;
        while(dp>1){
            dp=0;
			for (j = 0 ; j < 4 ; j++){
		    	r=(double)((IntRand() % (B + B)) - B) / B;
				q[4*i+j] = r;
				//g4[i][j] = r;
				dp+=r*r;
			}
		}
	    dp=sqrt(dp);
		for (j = 0 ; j < 4 ; j++)
		   q[4*i+j]/=dp;
		  // g4[i][j]/=dp;
	}
	
	while (--i) {
		k = P[i];
		P[i] = P[j = IntRand() % B];
		P[j] = k;
	}
	
	for (i = 0 ; i < B + 2 ; i++) {
		P[B + i] = P[i];
		G1[B + i] = G1[i];
		for (j = 0 ; j < 2 ; j++)
			G2[B + i][j] = G2[i][j];
		for (j = 0 ; j < 3 ; j++)
			G3[B + i][j] = G3[i][j];
		for (j = 0 ; j < 4 ; j++)
			q[4*B + 4*i+j] = q[4*i+j];
		    //g4[B + i][j] = g4[i][j];
	}
	initialized=true;
}

