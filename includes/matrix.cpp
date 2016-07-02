
#include "defs.h"
#include <math.h>
#include <stdio.h>

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

void mcpy(double *a, double *b, int n);

int minv(double *b, double *a, int n)
{
	int i,icol=0,irow=0,j,k,l,ll,retstat=0;
	double big,dum,pivinv,temp;
	int *indxc, *indxr, *ipiv;

	MALLOC(n,int,indxc);
	MALLOC(n,int,indxr);
	MALLOC(n,int,ipiv);

	if(a!=b)
		mcpy(b,a,n);
			
	for (j=0;j<n;j++)
		ipiv[j]=0;

	for (i=0;i<n;i++) {
		big=0.0;
		for (j=0;j<n;j++){
			if (ipiv[j] != 1) {
				for (k=0;k<n;k++) {
					if (ipiv[k] == 0) {
						if (fabs(a[j*n+k]) >= big) {
							big=fabs(a[j*n+k]);
							irow=j;
							icol=k;
						}
					} else if (ipiv[k] > 1)
						goto gjerr; /* singular matrix */
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol) {
			for (l=0;l<n;l++) 
				SWAP(a[irow*n+l],a[icol*n+l])
		}
		indxr[i]=irow;
		indxc[i]=icol;
		if (a[icol*n+icol] == 0.0)
			goto gjerr; /* singular matrix */

		pivinv=1.0/a[icol*n+icol];
		a[icol*n+icol]=1.0;
		for (l=0;l<n;l++) 
			a[icol*n+l] *= pivinv;
		for (ll=0;ll<n;ll++) {
			if (ll != icol) {
				dum=a[ll*n+icol];
				a[ll*n+icol]=0.0;
				for (l=0;l<n;l++) 
					a[ll*n+l] -= a[icol*n+l]*dum;
			}
		}
	}
	for (l=n-1;l>=0;l--) {
		if (indxr[l] != indxc[l]){
			for (k=0;k<n;k++)
				SWAP(a[k*n+indxr[l]],a[k*n+indxc[l]]);
		}
	}
	retstat=1; /* success */
gjerr:

	free(indxc);
	free(indxr);
	free(ipiv);
	
	return retstat;
			
}

// row major matrix x matrix

void RMmmul(double *a, double *b, double *c, int n)
{
	int i,j, k;
	for(i=0;i<n;i++){
		for(j=0;j<n;j++){
			c[i*n+j]=0.0;
			for(k=0;k<n;k++)
				c[i*n+j]+=a[i*n+k]*b[k*n+j];
		}
	}
}

// column major matrix x matrix

void CMmmul(double *a, double *b, double *c, int n)
{
	int i,j, k;
	for(i=0;i<n;i++){
		for(j=0;j<n;j++){
			c[j*n+i]=0.0;
			for(k=0;k<n;k++)
				c[j*n+i]+=a[k*n+i]*b[j*n+k];
		}
	}
}

// column major vector x matrix

void CMmvmul(double *v, double *m, double *c, int n)
{
	int i,j;
	for(i=0;i<n;i++){
		c[i]=0.0;
		for(j=0;j<n;j++)
			c[i]+=v[j]*m[j*n+i];
	}
}

// row major vector x matrix

void RMmvmul(double *v, double *b, double *c, int n)
{
	int i,j;
	for(i=0;i<n;i++){
		c[i]=0.0;
		for(j=0;j<n;j++)
			c[i]+=v[j]*b[i*n+j];
	}
}

void mcpy(double *a, double *b, int n)
{
	int i,j;
	for(i=0;i<n;i++){
		for(j=0;j<n;j++){
			b[i*n+j]=a[i*n+j];
		}
	}
}

double mdiff(double *a, double *b, int n)
{
	int i,j;
	double s=0;
	for(i=0;i<n;i++){
		for(j=0;j<n;j++){
		    double d=b[i*n+j]-a[i*n+j];
			s+=d*d;
		}
	}
	return sqrt(s/n);
}

double mmag(double *a, int n)
{
	int i,j;
	double s=0;
	for(i=0;i<n;i++){
		for(j=0;j<n;j++){
		    double d=a[i*n+j];
			s+=d*d;
		}
	}
	return sqrt(s/n);
}

void mprint(double *a, int n)
{
	int i,j;
	printf("\n");
	for(i=0;i<n;i++){
		for(j=0;j<n;j++){
			printf("%-15g ",a[i*n+j]);
		}
		printf("\n");
	}
}

void mprint(char *s, double *a, int n)
{
	printf("%s",s);
	mprint(a,n);
}

void vprint(double *a, int n)
{
	int i;

	printf("\n");
	for(i=0;i<n;i++){
		printf("%f ",a[i]);
	}
	printf("\n");
}
