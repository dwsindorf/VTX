#ifndef NOISEFUNCS_H_
#define NOISEFUNCS_H_

class NoiseFunc {
protected:
	static int initialized;
	static void initialize();
public:
	static int grad3[][3];
	static int grad4[32][4];
	static int p[256];
	static int perm[512];

	NoiseFunc();
	
	static void *makePermTexureImage();
	static void *makePerlinTexureImage();
	static char *makeGradTexureImage();
	static char *makeNoise3DTexureImage(int);
	static void *makeNoise3DVectorTexureImage(int size);
	static  int fastfloor(double x) {
		return x>0 ? (int)x : (int)x-1;
	}
	static double dot(int g[], double x, double y) {
		return g[0]*x + g[1]*y;
	}
	static double dot(int g[], double x, double y, double z) {
		return g[0]*x + g[1]*y + g[2]*z;
	}
	static double dot(int g[], double x, double y, double z, double w) {
		return g[0]*x + g[1]*y + g[2]*z + g[3]*w;
	}

	static double mix(double a, double b, double t) {
		return (1-t)*a + t*b;
	}
	static void iGetIntegerAndFractional(double x, int &i, double &u ){
		i = fastfloor(x);
		u = x - i;
		i = i & 255;
	}

};

class ClassicNoise: public NoiseFunc
{

public:
	static  double fade(double t) {
		return t*t*t*(t*(t*6-15)+10);
	}
	static  double dfade(double u) {
		return  30.0*u*u*(u*(u-2.0)+1.0);
	}
	ClassicNoise();
	static double noise(double x); // 1D
	static double noise(double x, double y); // 2D
	static double noise(double x, double y, double z); // 3D
	static double noise(double x, double y, double z, double w); // 4D
	static void   noisedv(double *vout, double x, double y, double z );

};

class SimplexNoise: public NoiseFunc
{
public:
	static int simplex[][4];
	
	SimplexNoise();
	static double noise(double x); // 1D
	static double noise(double x, double y); // 2D
	static double noise(double x, double y, double z); // 3D
	static double noise(double x, double y, double z, double w); // 4D

};

#endif /*NOISEFUNCS_H_*/
