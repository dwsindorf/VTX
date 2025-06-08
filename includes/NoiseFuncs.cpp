// Perlin and Simplex noise functions
// Author: Stefan Gustavson (stegu@itn.liu.se) 2004, 2005
#include <NoiseFuncs.h>


#include <Perlin.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <iostream>
using namespace std;
// BASE class

int NoiseFunc::initialized=false;

NoiseFunc::NoiseFunc(){}

#define FLOOR(x) (x)>0 ? (int)(x) : (int)((x)-1);

//int NoiseFunc::p[256];

/* Stefan Gustavson:
   These are Ken Perlin's proposed gradients for 3D noise. I kept them for
   better consistency with the reference implementation, but there is really
   no need to pad this to 16 gradients for this particular implementation.
   If only the "proper" first 12 gradients are used, they can be extracted
   from the grad4[][] array: grad3[i][j] == grad4[i*2][j], 0<=i<=11, j=0,1,2
*/

int NoiseFunc::grad3[16][3] = 
	{{0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1},
    {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
    {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},  // 12 cube edges (used in this file)
    {1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1}}; // 4 more to make 16 (used in tex gen)

/* Stefan Gustavson:
   These are my own proposed gradients for 4D noise. They are the coordinates
   of the midpoints of each of the 32 edges of a tesseract, just like the 3D
   noise gradients are the midpoints of the 12 edges of a cube.
*/
int NoiseFunc::grad4[32][4]= 
	{ {0,1,1,1}, {0,1,1,-1}, {0,1,-1,1}, {0,1,-1,-1},
	{	0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
	{	1,0,1,1}, {1,0,1,-1}, {1,0,-1,1}, {1,0,-1,-1},
	{	-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
	{	1,1,0,1}, {1,1,0,-1}, {1,-1,0,1}, {1,-1,0,-1},
	{	-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
	{	1,1,1,0}, {1,1,-1,0}, {1,-1,1,0}, {1,-1,-1,0},
	{	-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0}};

int NoiseFunc::perm[256] = {151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

//-------------------------------------------------------------
// NoiseFunc::makePermTexureImage 
//  return a memory map for building the "perm" texture
//-------------------------------------------------------------
void *NoiseFunc::makePermTexureImage() {
	int *pn=Perlin::permTable();
	double *gn=Perlin::gradTable(3);
	int n=Perlin::noiseDim();
	int i,j,k,offset,value;
	char *pixels = (char*)malloc( n*n*4 );
	for (j = 0; j<n; j++){
		for (i = 0; i<n; i++) {
			offset = (j*n+i)*4;
			value = pn[(pn[i]+j)&0xff]&0xff;
			k=(i+j)&0xff; // keep same gradient vectors vertically and horizontally
		    pixels[offset] = gn[k*3]* 128 + 128; // Gradient x
			pixels[offset+1] = gn[k*3+1]* 128 + 128; // Gradient y
			pixels[offset+2] = gn[k*3+2]* 128 + 128; // Gradient z
			pixels[offset+3] = value; // Permuted index
		}
	}
	return pixels;
}

//-------------------------------------------------------------
// NoiseFunc::makePerlinTexureImage
//  return a memory map for building the "perm" texture
//-------------------------------------------------------------
void *NoiseFunc::makePerlinTexureImage() {
	int *pn=Perlin::permTable();
	double *gn=Perlin::gradTable(3);
	int n=Perlin::noiseDim();
	int i,j=0,k,offset,value;
	char *pixels = (char*)malloc( n*4 );
	for (i = 0; i<n; i++) {
		pixels[i*4] = gn[i*3]* 128 + 128; // Gradient x
		pixels[i*4+1] = gn[i*3+1]* 128 + 128; // Gradient y
		pixels[i*4+2] = gn[i*3+2]* 128 + 128; // Gradient z
		pixels[i*4+3] = pn[i]; // Permuted index
	}
	return pixels;
}
//-------------------------------------------------------------
// NoiseFunc::makeGradTexureImage 
//  return a memory map for building the "grad" texture
//-------------------------------------------------------------
char *NoiseFunc::makeGradTexureImage() {
	//initialize();
	char *pixels;
	int i, j;
	pixels = (char*)malloc( 256*256*4);
	for (i = 0; i<256; i++) {
		for (j = 0; j<256; j++) {
			int offset = (i*256+j)*4;
			char value = perm[(j+perm[i]) & 0xFF];
			pixels[offset] = grad4[value & 0x1F][0] * 64 + 64; // Gradient x
			pixels[offset+1] = grad4[value & 0x1F][1] * 64 + 64; // Gradient y
			pixels[offset+2] = grad4[value & 0x1F][2] * 64 + 64; // Gradient z
			pixels[offset+3] = grad4[value & 0x1F][3] * 64 + 64; // Gradient w
		}
	}
	return pixels;
}

//-------------------------------------------------------------
// NoiseFunc::makeNoise3DTexureImage 
//  return a memory map for building the "noise3D" texture
//-------------------------------------------------------------
char *NoiseFunc::makeNoise3DTexureImage(int size) {
	//initialize();
	int i, j, k,offset=0;
	
	double vec[3];

	char *pixels=(char*)malloc(size*size*size);
	double mod=32;
	double delta=mod/size;
	for (i = 0; i<size; i++) {
		for (j = 0; j<size; j++) {
			for (k = 0; k<size; k++,offset++) {
//				vec[0]=i*delta+0.5; vec[1]=j*delta+0.5;  vec[2]=k*delta+0.5;
//				pixels[offset] = (char)(255*(::noise3(vec)+0.5));
				double x=fmod(i*delta+0.5,mod);
				double y=fmod(j*delta+0.5,mod);
				double z=fmod(k*delta+0.5,mod);
				pixels[offset] = (char)(127.5*(Classic::noise(x,y,z)+1.0));
			}
		}
	}
	return (char*)pixels;
}


//-------------------------------------------------------------
// NoiseFunc::makeNoise3DTexureImage
//  return a memory map for building the "noise3D" texture
//-------------------------------------------------------------
void *NoiseFunc::makeNoise3DVectorTexureImage(int size) {
	//initialize();
	int i, j, k,l,offset=0;

	double v[4];

	char *pixels=(char*)malloc(size*size*size*4);
	double mod=32;
	double delta=mod/size;
	for (i = 0; i<size; i++) {
		for (j = 0; j<size; j++) {
			for (k = 0; k<size; k++) {
				double x=fmod(i*delta+0.5,mod);
				double y=fmod(j*delta+0.5,mod);
				double z=fmod(k*delta+0.5,mod);
				Classic::noisedv(v,x,y,z);
				for (l = 0; l<4; l++,offset++)
					pixels[offset] = (char)(127.5*(v[l]+1.0));
			}
		}
	}
	return pixels;
}

//###########   ClassicNoise class ################################

double Classic::noise(double x){
	return 0;
}
double Classic::noise(double x, double y){
	return 0;
}
double Classic::noise(double a, double b, double c) {
	// Find unit grid cell containing point
	double x,y,z;
	int X,Y,Z;
    iGetIntegerAndFractional( a, X, x );
    iGetIntegerAndFractional( b, Y, y );
    iGetIntegerAndFractional( c, Z, z );

	// Calculate a set of eight hashed gradient indices
	int gi000 = perm[X+perm[Y+perm[Z]]] % 12;
	int gi001 = perm[X+perm[Y+perm[Z+1]]] % 12;
	int gi010 = perm[X+perm[Y+1+perm[Z]]] % 12;
	int gi011 = perm[X+perm[Y+1+perm[Z+1]]] % 12;
	int gi100 = perm[X+1+perm[Y+perm[Z]]] % 12;
	int gi101 = perm[X+1+perm[Y+perm[Z+1]]] % 12;
	int gi110 = perm[X+1+perm[Y+1+perm[Z]]] % 12;
	int gi111 = perm[X+1+perm[Y+1+perm[Z+1]]] % 12;
	// The gradients of each corner are now:
	// g000 = grad3[gi000];
	// g001 = grad3[gi001];
	// g010 = grad3[gi010];
	// g011 = grad3[gi011];
	// g100 = grad3[gi100];
	// g101 = grad3[gi101];
	// g110 = grad3[gi110];
	// g111 = grad3[gi111];
	// Calculate noise contributions from each of the eight corners
	double n000= dot(grad3[gi000], x, y, z);
	double n100= dot(grad3[gi100], x-1, y, z);
	double n010= dot(grad3[gi010], x, y-1, z);
	double n110= dot(grad3[gi110], x-1, y-1, z);
	double n001= dot(grad3[gi001], x, y, z-1);
	double n101= dot(grad3[gi101], x-1, y, z-1);
	double n011= dot(grad3[gi011], x, y-1, z-1);
	double n111= dot(grad3[gi111], x-1, y-1, z-1);
	// Compute the fade curve value for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);
	// Interpolate along x the contributions from each of the corners
	double nx00 = mix(n000, n100, u);
	double nx01 = mix(n001, n101, u);
	double nx10 = mix(n010, n110, u);
	double nx11 = mix(n011, n111, u);
	// Interpolate the four results along y
	double nxy0 = mix(nx00, nx10, v);
	double nxy1 = mix(nx01, nx11, v);
	// Interpolate the two last results along z
	double nxyz = mix(nxy0, nxy1, w);

	// nx00 = (1-u)*n000 + u*n100;
	// nx01 = (1-u)*n001 + u*n101;
	// nx10 = (1-u)*n010 + u*n110;
	// nx11 = (1-u)*n011 + u*n111;

	// nxy0 = (1-v)*nx00 + v*nx10;
	// nxy1 = (1-v)*nx01 + v*nx11;

	// nxyz = (1-w)*nxy0 + w*nxy1;

	return 0.7*nxyz;
}


void Classic::noisedv(double *vout, double a, double b, double c )
{
	// Find unit grid cell containing point
	double x,y,z;
	int X,Y,Z;
    iGetIntegerAndFractional( a, X, x );
    iGetIntegerAndFractional( b, Y, y );
    iGetIntegerAndFractional( c, Z, z );

	// Calculate a set of eight hashed gradient indices
	int gi000 = perm[X+perm[Y+perm[Z]]] % 12;
	int gi001 = perm[X+perm[Y+perm[Z+1]]] % 12;
	int gi010 = perm[X+perm[Y+1+perm[Z]]] % 12;
	int gi011 = perm[X+perm[Y+1+perm[Z+1]]] % 12;
	int gi100 = perm[X+1+perm[Y+perm[Z]]] % 12;
	int gi101 = perm[X+1+perm[Y+perm[Z+1]]] % 12;
	int gi110 = perm[X+1+perm[Y+1+perm[Z]]] % 12;
	int gi111 = perm[X+1+perm[Y+1+perm[Z+1]]] % 12;

	// Calculate noise contributions from each of the eight corners
	double n000= dot(grad3[gi000], x, y, z);
	double n100= dot(grad3[gi100], x-1, y, z);
	double n010= dot(grad3[gi010], x, y-1, z);
	double n110= dot(grad3[gi110], x-1, y-1, z);
	double n001= dot(grad3[gi001], x, y, z-1);
	double n101= dot(grad3[gi101], x-1, y, z-1);
	double n011= dot(grad3[gi011], x, y-1, z-1);
	double n111= dot(grad3[gi111], x-1, y-1, z-1);

	// Compute the fade curve value for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);
	double du = dfade(x);
	double dv = dfade(y);
	double dw = dfade(z);
	// Interpolate along x the contributions from each of the corners
	double nx00 = mix(n000, n100, u);
	double nx01 = mix(n001, n101, u);
	double nx10 = mix(n010, n110, u);
	double nx11 = mix(n011, n111, u);
	// Interpolate the four results along y
	double nxy0 = mix(nx00, nx10, v);
	double nxy1 = mix(nx01, nx11, v);
	// Interpolate the two last results along z
	double nxyz =  mix(nxy0, nxy1, w);
	vout[0] = 0.7*nxyz;

	// nx00 = n000-u*n000 + u*n100;
	// nx01 = n001-u*n001 + u*n101;
	// nx10 = n010-u*n010 + u*n110;
	// nx11 = n011-u*n011 + u*n111;

	// d(nx00)/du = n100-n000;
	// d(nx01)/du = n101-n001;
	// d(nx10)/du = n110-n010;
	// d(nx11)/du = n111-n011;

	//       nxy0 = nx00 - v*nx00 + v*nx10;
	// d(nxy0)/du = d(nx00)/du - v*d(nx00)/du + v*d(nx10)/du
	//            = n100-n000 -v*(n100-n000) + v*(n110-n010)
	//            = n100-n000 +v*(n110-n010-n100+n000)

	//       nxy1 = nx01 - v*nx01 + v*nx11
	// d(nxy1)/du = d(nx01)/du-v*d(nx01)/du + v*d(nx11)/du
	//            = n101-n001 -v*(n101-n001) +v*(n111-n011)
	//            = n101-n001 +v*(n111-n011-n101+n001)

	// nxyz = (1-w)*nxy0 + w*nxy1
	//
	// 1. derivative WRT u
	// d(nxyz)/du = (1-w)*d(nxy0)/du+w*d(nxy1)/du
	//            = (1-w)*(n100-n000 +v*(n110-n010-n100+n000))+w*(n101-n001 +v*(n111-n011-n101+n001))
	vout[1] = 0.7*du*((1-w)*(n100-n000 +v*(n110-n010-n100+n000))+w*(n101-n001 +v*(n111-n011-n101+n001)));
	// 2. derivative WRT v
	// d(nxyz)/dv = (1-w)*d(nxy0)/dv+w*d(nxy1)/dv
	//      = (1-w)*(nx10-nx00) + w*(nx11-nx01)
	vout[2] = 0.7*dv*((1-w)*(nx10-nx00) + w*(nx11-nx01));
	// 3. derivative WRT w
	// d(nxyz)/dw = nxy1-nxy0
	vout[3] = 0.7*dw*(nxy1-nxy0);

	//return 0.7*nxyz;

}

double Classic::noise(double x, double y, double z, double w){
	return 0.0; // WIP
}

//########### SimplexNoise class ################################

// A lookup table to traverse the simplex around a given point in 4D.
// Details can be found where this table is used, in the 4D noise method.
int Simplex::simplex[][4] = {
	{	0,1,2,3}, {0,1,3,2}, {0,0,0,0}, {0,2,3,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,2,3,0},
	{	0,2,1,3}, {0,0,0,0}, {0,3,1,2}, {0,3,2,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {1,3,2,0},
	{	0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0},
	{	1,2,0,3}, {0,0,0,0}, {1,3,0,2}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,3,0,1}, {2,3,1,0},
	{	1,0,2,3}, {1,0,3,2}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {2,0,3,1}, {0,0,0,0}, {2,1,3,0},
	{	0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0},
	{	2,0,1,3}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {3,0,1,2}, {3,0,2,1}, {0,0,0,0}, {3,1,2,0},
	{	2,1,0,3}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {3,1,0,2}, {0,0,0,0}, {3,2,0,1}, {3,2,1,0}};


// 1D simplex noise
double Simplex::minmax(double v){
	double result= noise(v,0.123*v,3.15*v);
	//#define TEST
	#ifdef TEST
		static double min=1000,max=-1000;
		static int cnt=0;
		if(result<min || result>max){
			min=result<min?result:min;
			max=result>max?result:max;
			cout<<"Simplex["<<cnt <<"] min:"<<min<<" max:"<<max <<" range:"<<max-min<<endl;
		}
		cnt++;
	#endif
		return result;
}

// 1D simplex noise
double Simplex::noise(double xin){
	return Simplex::noise(xin, xin) ;
}
// 2D simplex noise
double Simplex::noise(double xin, double yin) {
	double n0, n1, n2; // Noise contributions from the three corners
	// Skew the input space to determine which simplex cell we're in
	const double F2 = 0.5*(sqrt(3.0)-1.0);
	double s = (xin+yin)*F2; // Hairy factor for 2D
	int i = fastfloor(xin+s);
	int j = fastfloor(yin+s);
	const double G2 = (3.0-sqrt(3.0))/6.0;
	double t = (i+j)*G2;
	double X0 = i-t; // Unskew the cell origin back to (x,y) space
	double Y0 = j-t;
	double x0 = xin-X0; // The x,y distances from the cell origin
	double y0 = yin-Y0;
	// For the 2D case, the simplex shape is an equilateral triangle.
	// Determine which simplex we are in.
	int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
	if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
	else {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)
	// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
	// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
	// c = (3-sqrt(3))/6
	double x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
	double y1 = y0 - j1 + G2;
	double x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
	double y2 = y0 - 1.0 + 2.0 * G2;
	// Work out the hashed gradient indices of the three simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int gi0 = phash(ii+phash(jj)) % 12;
	int gi1 = phash(ii+i1+phash(jj+j1)) % 12;
	int gi2 = phash(ii+1+phash(jj+1)) % 12;
	// Calculate the contribution from the three corners
	double t0 = 0.5 - x0*x0-y0*y0;
	if(t0<0) n0 = 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
	}
	double t1 = 0.5 - x1*x1-y1*y1;
	if(t1<0) n1 = 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}
	double t2 = 0.5 - x2*x2-y2*y2;
	if(t2<0) n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}
	// Add contributions from each corner to get the noise value.
	// The result is scaled to return values in the interval [-1,1].
	return 70.0 * (n0 + n1 + n2);
}
// 3D simplex noise
double Simplex::noise(double x, double y, double z) {
	double n0, n1, n2, n3; // Noise contributions from the four corners
	// Skew the input space to determine which simplex cell we're in
	const double F3 = 1.0/3.0;
	double s = (x+y+z)*F3; // Very nice and simple skew factor for 3D
	int i = FLOOR(x+s);
	int j = FLOOR(y+s);
	int k = FLOOR(z+s);
	const double G3 = 1.0/6.0; // Very nice and simple unskew factor, too
	double t = (i+j+k)*G3;
	double X0 = i-t; // Unskew the cell origin back to (x,y,z) space
	double Y0 = j-t;
	double Z0 = k-t;
	double x0 = x-X0; // The x,y,z distances from the cell origin
	double y0 = y-Y0;
	double z0 = z-Z0;
	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
	// Determine which simplex we are in.
	int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
	int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
	   if (x0 >= y0) {
	        if (y0 >= z0) {
	            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; // X Y Z order
	        } else if (x0 >= z0) {
	            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; // X Z Y order
	        } else {
	            i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; // Z X Y order
	        }
	    } else { // x0<y0
	        if (y0 < z0) {
	            i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; // Z Y X order
	        } else if (x0 < z0) {
	            i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; // Y Z X order
	        } else {
	            i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; // Y X Z order
	        }
	    }
	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.
	double x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
	double y1 = y0 - j1 + G3;
	double z1 = z0 - k1 + G3;
	double x2 = x0 - i2 + 2.0*G3; // Offsets for third corner in (x,y,z) coords
	double y2 = y0 - j2 + 2.0*G3;
	double z2 = z0 - k2 + 2.0*G3;
	double x3 = x0 - 1.0 + 3.0*G3; // Offsets for last corner in (x,y,z) coords
	double y3 = y0 - 1.0 + 3.0*G3;
	double z3 = z0 - 1.0 + 3.0*G3;
	// Work out the hashed gradient indices of the four simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;

	int gi0 = phash(ii+phash(jj+phash(kk))) % 12;
	int gi1 = phash(ii+i1+phash(jj+j1+phash(kk+k1))) % 12;
	int gi2 = phash(ii+i2+phash(jj+j2+phash(kk+k2))) % 12;
	int gi3 = phash(ii+1+phash(jj+1+phash(kk+1))) % 12;
	// Calculate the contribution from the four corners
	double t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
	if(t0<0) n0 = 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
	}
	double t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
	if(t1<0) n1 = 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
	}
	double t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
	if(t2<0) n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
	}
	double t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
	if(t3<0) n3 = 0.0;
	else {
		t3 *= t3;
		n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
	}

	// Add contributions from each corner to get the noise value.
	// The result is scaled to stay just inside [-1,1]
	double result= 16.77*(n0 + n1 + n2 + n3);
	//#define TEST2
	#ifdef TEST2
		static double min=1000,max=-1000;
		static int cnt=0;
		if(result<min || result>max){
			min=result<min?result:min;
			max=result>max?result:max;
			cout<<"Simplex["<<cnt <<"] min:"<<min<<" max:"<<max <<" range:"<<max-min<<endl;
		}
		cnt++;
	#endif
		return result;

	//return 16.77*(n0 + n1 + n2 + n3);
}


// 4D simplex noise
double Simplex::noise(double x, double y, double z, double w) {
	// The skewing and unskewing factors are hairy again for the 4D case
	const double F4 = (sqrt(5.0)-1.0)/4.0;
	const double G4 = (5.0-sqrt(5.0))/20.0;
	double n0, n1, n2, n3, n4; // Noise contributions from the five corners
	// Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
	double s = (x + y + z + w) * F4; // Factor for 4D skewing
	int i = FLOOR(x + s);
	int j = FLOOR(y + s);
	int k = FLOOR(z + s);
	int l = FLOOR(w + s);
	double t = (i + j + k + l) * G4; // Factor for 4D unskewing
	double X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
	double Y0 = j - t;
	double Z0 = k - t;
	double W0 = l - t;
	double x0 = x - X0; // The x,y,z,w distances from the cell origin
	double y0 = y - Y0;
	double z0 = z - Z0;
	double w0 = w - W0;
	// For the 4D case, the simplex is a 4D shape I won't even try to describe.
	// To find out which of the 24 possible simplices we're in, we need to
	// determine the magnitude ordering of x0, y0, z0 and w0.
	// The method below is a good way of finding the ordering of x,y,z,w and
	// then find the correct traversal order for the simplex we are in.
	// First, six pair-wise comparisons are performed between each possible pair
	// of the four coordinates, and the results are used to add up binary bits
	// for an integer index.
	int c1 = (x0 > y0) ? 32 : 0;
	int c2 = (x0 > z0) ? 16 : 0;
	int c3 = (y0 > z0) ? 8 : 0;
	int c4 = (x0 > w0) ? 4 : 0;
	int c5 = (y0 > w0) ? 2 : 0;
	int c6 = (z0 > w0) ? 1 : 0;
	int c = c1 + c2 + c3 + c4 + c5 + c6;
	int i1, j1, k1, l1; // The integer offsets for the second simplex corner
	int i2, j2, k2, l2; // The integer offsets for the third simplex corner
	int i3, j3, k3, l3; // The integer offsets for the fourth simplex corner
	// simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
	// Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
	// impossible. Only the 24 indices which have non-zero entries make any sense.
	// We use a thresholding to set the coordinates in turn from the largest magnitude.
	// The number 3 in the "simplex" array is at the position of the largest coordinate.
	i1 = simplex[c][0]>=3 ? 1 : 0;
	j1 = simplex[c][1]>=3 ? 1 : 0;
	k1 = simplex[c][2]>=3 ? 1 : 0;
	l1 = simplex[c][3]>=3 ? 1 : 0;
	// The number 2 in the "simplex" array is at the second largest coordinate.
	i2 = simplex[c][0]>=2 ? 1 : 0;
	j2 = simplex[c][1]>=2 ? 1 : 0;
	k2 = simplex[c][2]>=2 ? 1 : 0;
	l2 = simplex[c][3]>=2 ? 1 : 0;
	// The number 1 in the "simplex" array is at the second smallest coordinate.
	i3 = simplex[c][0]>=1 ? 1 : 0;
	j3 = simplex[c][1]>=1 ? 1 : 0;
	k3 = simplex[c][2]>=1 ? 1 : 0;
	l3 = simplex[c][3]>=1 ? 1 : 0;
	// The fifth corner has all coordinate offsets = 1, so no need to look that up.
	double x1 = x0 - i1 + G4; // Offsets for second corner in (x,y,z,w) coords
	double y1 = y0 - j1 + G4;
	double z1 = z0 - k1 + G4;
	double w1 = w0 - l1 + G4;
	double x2 = x0 - i2 + 2.0*G4; // Offsets for third corner in (x,y,z,w) coords
	double y2 = y0 - j2 + 2.0*G4;
	double z2 = z0 - k2 + 2.0*G4;
	double w2 = w0 - l2 + 2.0*G4;
	double x3 = x0 - i3 + 3.0*G4; // Offsets for fourth corner in (x,y,z,w) coords
	double y3 = y0 - j3 + 3.0*G4;
	double z3 = z0 - k3 + 3.0*G4;
	double w3 = w0 - l3 + 3.0*G4;
	double x4 = x0 - 1.0 + 4.0*G4; // Offsets for last corner in (x,y,z,w) coords
	double y4 = y0 - 1.0 + 4.0*G4;
	double z4 = z0 - 1.0 + 4.0*G4;
	double w4 = w0 - 1.0 + 4.0*G4;
	// Work out the hashed gradient indices of the five simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;
	int ll = l & 255;

	int gi0 = phash(ii+phash(jj+phash(kk+phash(ll)))) % 32;
	int gi1 = phash(ii+i1+phash(jj+j1+phash(kk+k1+phash(ll+l1)))) % 32;
	int gi2 = phash(ii+i2+phash(jj+j2+phash(kk+k2+phash(ll+l2)))) % 32;
	int gi3 = phash(ii+i3+phash(jj+j3+phash(kk+k3+phash(ll+l3)))) % 32;
	int gi4 = phash(ii+1+phash(jj+1+phash(kk+1+phash(ll+1)))) % 32;

#define DOT4(g,x,y,z,w) (g[0]*x + g[1]*y + g[2]*z + g[3]*w)

	// Calculate the contribution from the five corners
	double t0 = 0.6 - x0*x0 - y0*y0 - z0*z0 - w0*w0;
	if(t0<0)
		n0 = 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * DOT4(grad4[gi0], x0, y0, z0, w0);
	}
	double t1 = 0.6 - x1*x1 - y1*y1 - z1*z1 - w1*w1;
	if(t1<0)
		n1 = 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * DOT4(grad4[gi1], x1, y1, z1, w1);
	}
	double t2 = 0.6 - x2*x2 - y2*y2 - z2*z2 - w2*w2;
	if(t2<0)
		n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * DOT4(grad4[gi2], x2, y2, z2, w2);
	}
	double t3 = 0.6 - x3*x3 - y3*y3 - z3*z3 - w3*w3;
	if(t3<0)
		n3 = 0.0;
	else {
		t3 *= t3;
		n3 = t3 * t3 * DOT4(grad4[gi3], x3, y3, z3, w3);
	}
	double t4 = 0.6 - x4*x4 - y4*y4 - z4*z4 - w4*w4;
	double dp=0;
	if(t4<0)
		n4 = 0.0;
	else {
		t4 *= t4;
		dp=DOT4(grad4[gi4], x4, y4, z4, w4);
		n4 = t4 * t4 * dp;
	}

	// Sum up and scale the result to cover the range [-1,1]
	return 27.0* (n0 + n1 + n2 + n3 + n4);
}


//########### Voronoi Noise class ################################

extern void make_lut();
#define MAX(a,b) (a)>(b)?(a):(b)
#define SIGN(x) (x)<0?-1:1
double Voronoi::rval=100.0;
double Voronoi::hval=43758.5453;
Point4D Voronoi::hpnt(1.0*hval, 57.0*hval, 113.0*hval, 33.0*hval);

//-------------------------------------------------------------
// Noise::Voronoi2D() 1d Voronoi noise
//-------------------------------------------------------------
double Voronoi::minmax(double v) {
	double result= noise(v,0.123*v,3.15*v);
	//#define TEST
	#ifdef TEST
		static double min=1000,max=-1000;
		static int cnt=0;
		if(result<min || result>max){
			min=result<min?result:min;
			max=result>max?result:max;
			cout<<"Voronoi["<<cnt <<"] min:"<<min<<" max:"<<max <<" range:"<<max-min<<endl;
		}
		cnt++;
	#endif
	return result;
}

//-------------------------------------------------------------
// Noise::Voronoi2D() 1d Voronoi noise
//-------------------------------------------------------------
double Voronoi::noise(double x) {
	return noise(x,x);
}

Point2D rhashv(Point2D uv) {
	static double myt[4] = { 0.12121212, 0.13131313, -0.13131313, 0.12121212 };
	static Point2D mys(1e4, 1e6);
	Point2D p2 = uv.mm4(myt);
	p2 = p2 * mys;
	Point2D p3 = p2.fract();
	p3 = p3 * p2;
	return p3.fract();
}

//-------------------------------------------------------------
// Noise::Voronoi2D() 1d Voronoi noise
//-------------------------------------------------------------
double Voronoi::noise(double x, double y) {
  Point2D point(x,y);
  Point2D p = point.floor();
  Point2D f = point.fract();
  double res = 0.0;
  for (int j = -1; j <= 1; j++) {
    for (int i = -1; i <= 1; i++) {
      Point2D b = Point2D(i, j);
      Point2D r = b- f + rhashv(p + b);
      double d=pow(r.magnitude(),8.0);
      res += 1.0 / d;
    }
  }
  return pow(1.0 / res, 0.0625);
}

#define pvmul3(p, m, p1) \
	p1.x=m.x*p.x+m.y*p.y+m.z*p.z; \
	p1.y=m.y*p.x+m.z*p.y+m.x*p.z; \
	p1.z=m.z*p.x+m.x*p.y+m.y*p.z;

inline Point Voronoi::hashv3(Point p) {
	Point p1;
	
	pvmul3(p, hpnt, p1);
	double d1=SIN(p1.x);
	double d2=SIN(p1.y);
	double d3=SIN(p1.z);
	
	Point p4= Point(d1,d2,d3);
	return p4.fract();
}

//-------------------------------------------------------------
// Noise::Voronoi3D() 3d Voronoi noise (for terrain ht etc.)
//-------------------------------------------------------------
double  Voronoi::noise(double x, double y, double z) {
//#ifdef USE_LUT
//	make_lut();
//#endif
	Point pnt=Point(x,y,z);
	Point p = pnt.floor();
	Point f = pnt.fract();
	Point2D res(rval);
	for (int k = -1; k <= 1; k++) {
		for (int j = -1; j <= 1; j++) {
			for (int i = -1; i <= 1; i++) {
				Point b = Point(double(i), double(j), double(k));
				Point r = Point(b) - f + hashv3(p + b);
				double d =r.magnitude();

				double cond = MAX(SIGN(res.x - d), 0.0);
				double nCond = 1.0 - cond;

				double cond2 = nCond * MAX(SIGN(res.y - d), 0.0);
				double nCond2 = 1.0 - cond2;
				res = Point2D(d, res.x) * cond + res * nCond;
				res.y = cond2 * d + nCond2 * res.y;
			}
		}
	}
	return res.x-0.5;
}

//-------------------------------------------------------------
// Noise::Voronoi4D() 4d Voronoi noise (for seamless image generation)
//-------------------------------------------------------------
#define pvmul4(v, m, p) \
	p.x=m.x*v.x+m.w*v.y+m.z*v.z+m.y*v.w; \
	p.y=m.y*v.x+m.z*v.y+m.w*v.z+m.z*v.w; \
	p.z=m.z*v.x+m.y*v.y+m.x*v.z+m.w*v.w; \
	p.w=m.w*v.x+m.z*v.y+m.y*v.z+m.x*v.w;

inline Point4D Voronoi::hashv4(Point4D p) {
	Point4D p1;
	pvmul4(p, hpnt, p1);

	double d1=SIN(p1.x);
	double d2=SIN(p1.y);
	double d3=SIN(p1.z);
	double d4=SIN(p1.w);
	Point4D p4= Point4D(d4,d3,d2,d1);
	return p4.fract();
}

double Voronoi::noise(double x, double y, double z, double w) {
//#ifdef USE_LUT
//	make_lut();
//#endif
	double result = 0;
	Point4D pnt = Point4D(x,y,z,w);
	Point4D p = pnt.floor();
	Point4D f = pnt.fract();
	Point res(rval);
	for (int k = -1; k <= 1; k++) {
		for (int j = -1; j <= 1; j++) {
			for (int i = -1; i <= 1; i++) {
				for (int l = -1; l<= 1; l++) {
					Point4D b = Point4D(double(i), double(j), double(k), double(l));
					Point4D r = Point4D(b) - f  + hashv4(p + b);
					double d = r.magnitude();

					double cond = MAX(SIGN(res.x - d), 0.0);
					double nCond = 1.0 - cond;

					double cond2 = nCond * MAX(SIGN(res.y - d), 0.0);
					double nCond2 = 1.0 - cond2;

					double cond3 = nCond2 * MAX(SIGN(res.z - d), 0.0);
					double nCond3 = 1.0 - cond3;
					res = Point(d, res.x, res.y) * cond + res * nCond;
					res.y = cond2 * d + nCond2 * res.y;
					res.z = cond3 * d + nCond3 * res.z;
				}
			}
		}
	}
	return res.x-0.5;
}
