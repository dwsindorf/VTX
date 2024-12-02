/* =========================================================================
 *  MODULE:     defs.h
 * =========================================================================
 * PURPOSE:     utility define macros (c or c++)
 * =========================================================================
 *         Copyright (c) 1995, Elscint MRI
 * ========================================================================*/

#ifndef DEFS_H
#define DEFS_H

/*====================== standard c includes ==============================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
using namespace std;
/* ========================================================================*/
#ifndef MAXSTR
#define MAXSTR 1024
#endif

#define CCHAR2CHAR(f)  const_cast<char*>((f))
//#define WXSTR2CHAR(f)  const_cast<char*>((f).ToAscii())

#define TRUE	1
#define FALSE 	0
#define PI		3.14159265359
#define RT2		1.414213562373

const double RPD=2.0*PI/360.0;
const double DPR=360.0/2.0/PI;
const double RTOMM = 1;   // rad 3.6 = pluto, 10=max
const double RTOM  = 1e6*RTOMM;
const double RTOF  = RTOM*5280;

const double MM    = 1.0/RTOMM;
const double FEET  = 1.0/RTOF;
const double MILES = 1.0/RTOM;
const double AU    = 92.96*MM;
const double BM    = 1000*MM;
const double LY    = 5865696*MM;

typedef unsigned int uint;

#define ROUND(x,n)		floor(0.5+(x)*(n))/(n)
#define DEBUGGER	   *(char*)0=0
#define	BIT_ON(a,b)		(a) |= (b)
#define	BIT_OFF(a,b)	(a )&= ~(b)
#define	BIT_TST(a,b)	a&b?1:0
#define	BIT_SET(a,b,f)	if(f) BIT_ON(a,b); else BIT_OFF(a,b)
#define	MSK_SET(a,b,f)	BIT_OFF(a,b); BIT_ON(a,f)

#define VERR(x)			printf("virtual call error : %s\n",x)
#define VERR0(s)		VERR(s); return 0;

#include <malloc.h>
#define MALLOC(a,b,c)   if((c=(b*)malloc((a)*sizeof(b)))==NULL) {printf("malloc error\n");exit(1);}

#define CALLOC(a,b,c)   if((c=(b*)calloc((a)*sizeof(b),1))==NULL)\
  							{printf("calloc error\n");exit(1);}

#define REALLOC(a,b,c)  if((c=(b*)realloc((char*)c,(a)*sizeof(b)))==NULL)\
  							{printf("realloc error\n");exit(1);}

#define FREE(a)   		if(a) {::free(a);a=0;}
#define DFREE(a)   	if(a) {delete a;a=0;}

#define K2C(t)      ((t)-273)
#define C2F(t)      ((t)*9.4/5.0+32)
#define K2F(t)      (K2C(t)*9.4/5.0+32)

#define MGR(type,name)	\
typedef struct name {			/* gp management structure			*/\
	type               *base;	/* storage base				 		*/\
	type               *ptr;	/* pointer into	storage	area		*/\
	int                 n;		/* number of objects				*/\
}                   name

#define FLOOR(a)   		((int)a-(a<0 && a != (int)a))

//#define P360(p) (p > 360.0 ? p-360.0 : p < 0 ? p+360.0 : p)
#define P360(p) (fmod(p,360))
#define P180(p) (fmod(p,180))

inline char *newstr(char *a){
	char *b;
	MALLOC(strlen(a)+1,char,b);
	strcpy(b,a);
	return b;
}

inline double unwrap(double a, double b){
	double d=b-a;
	d=d>=180?d-360:(d<=-180? d+360:d);
	return a+d;
}

inline double round(double f,int n){
	static double digits[]={1,10,1e2,1e3,1e4,1e5,1e6,1e7,1e8};
	if(n<0||n>8)
		return f;
	int i=(int)(f*digits[n]+0.5);
	return i/digits[n];
}

typedef void (*VFV)( void );
typedef void (*VFI)( int );
typedef void (*VFD)( double );

#endif
