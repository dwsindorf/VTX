#ifndef PERLIN_H_
#define PERLIN_H_
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
	static double minmax(double v);

//	static void init_noise();

	static int noiseDim();
	static double *gradTable(int);
	static int *permTable();

};
#endif /*PERLIN_H_*/
