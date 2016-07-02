
// matrix utilities

#ifndef _MATRIX
#define _MATRIX

#include <math.h>
#include <stdio.h>

void mcpy(double *a, double *b, int n);
int  minv(double *b, double *a, int n);
void RMmmul(double *a, double *b, double *c, int n);
void CMmmul(double *a, double *b, double *c, int n);
void RMmvmul(double *v, double *b, double *c, int n);
void CMmvmul(double *v, double *b, double *c, int n);
void mcpy(double *a, double *b, int n);
void mprint(char*, double *a, int n);
void mprint(double *a, int n);
void vprint(double *a, int n);
double mdiff(double *a, double *b, int n);
double mmag(double *a,int n);

#define mvmul4(v, m, c) \
	c[0]=m[0]*v[0]+m[4]*v[1]+m[8]*v[2] +m[12]; \
	c[1]=m[1]*v[0]+m[5]*v[1]+m[9]*v[2] +m[13]; \
	c[2]=m[2]*v[0]+m[6]*v[1]+m[10]*v[2]+m[14]; \
	c[3]=m[3]*v[0]+m[7]*v[1]+m[11]*v[2]+m[15]  

#define mvmul3(v, m, c) \
	c[0]=m[0]*v[0]+m[4]*v[1]+m[8]*v[2] +m[12]; \
	c[1]=m[1]*v[0]+m[5]*v[1]+m[9]*v[2] +m[13]; \
	c[2]=m[2]*v[0]+m[6]*v[1]+m[10]*v[2]+m[14] 

#endif
