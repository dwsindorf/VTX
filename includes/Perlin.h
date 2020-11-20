#ifndef PERLIN_H_
#define PERLIN_H_
//extern double noise4(double vec[]);
//extern double noise3(double vec[]);
//extern double noise2(double vec[]);
//extern double noise1(double arg);
//
//extern double SRand();
//extern double Rand();
//extern double RandSum(int nargs, double *args);
//extern int IntRand();
//extern int *init_noise(int RandSeed);
class Perlin {
protected:
	static bool initialized;
	static void initialize();
public:
	static int seed;
	static double noise4(double vec[]);
	static double noise3(double vec[]);
	static double noise2(double vec[]);
	static double noise1(double arg);
//	static void init_noise();

	static int noiseDim();
	static double *gradTable(int);
	static int *permTable();

};
#endif /*PERLIN_H_*/
