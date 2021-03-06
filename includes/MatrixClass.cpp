
#include <math.h>
#include <stdio.h>
#include "defs.h"
#include "MatrixClass.h"

//************************************************************
// Vector class
//************************************************************

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
Vector::Vector(){
}

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
Vector::Vector(Vector *a){
    for(int i=0;i<dim;i++)
        D[i]=a->D[i];
}

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
Vector::Vector(double *a){
    for(int i=0;i<dim;i++)
        D[i]=a[i];
}

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
Vector::Vector(Point &p){
    D[0]=p.x;
    D[1]=p.y;
    D[2]=p.z;
    D[3]=1;
}

//-------------------------------------------------------------
// print vector
//-------------------------------------------------------------
void Vector::print(){
	vprint(D, dim);
}

//-------------------------------------------------------------
// index operator
//-------------------------------------------------------------
double & Vector::operator[](int i){
     return D[i];
}

//************************************************************
// Matrix class
//************************************************************

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
Matrix::Matrix(){
}

//-------------------------------------------------------------
// constructor (init all values)
//-------------------------------------------------------------
Matrix::Matrix(int v){
    for(int i=0;i<dim*dim;i++)
        D[i]=v;
}

//-------------------------------------------------------------
// constructor (init diagonal and off-diagonal all values)
//-------------------------------------------------------------
Matrix::Matrix(int c,int d){
	int i,j;
	for(i=0;i<dim;i++){
		for(j=0;j<dim;j++)
			D[i*dim+j]=(i==j?d:c);
	}
}

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
Matrix::Matrix(double *d){
    for(int i=0;i<dim*dim;i++)
        D[i]=d[i];
}

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
Matrix::Matrix(float *d){
    for(int i=0;i<dim*dim;i++)
        D[i]=d[i];
}

//-------------------------------------------------------------
// constructor
//-------------------------------------------------------------
Matrix::Matrix(Matrix *a){
    for(int i=0;i<dim*dim;i++)
        D[i]=a->D[i];
}

//-------------------------------------------------------------
// print matrix
//-------------------------------------------------------------
void Matrix::print(){
	int i,j;
	printf("\n");
	for(i=0;i<dim;i++){
		for(j=0;j<dim;j++){
		    double v=D[j*dim+i];
			printf("%-15g ",v);
		}
		printf("\n");
	}
	//mprint(D, dim);
}

//-------------------------------------------------------------
// sprint matrix
//-------------------------------------------------------------
void Matrix::sprint(char *s){
	int i,j;
	char value[64];
	s[0]=0;
	for(i=0;i<dim;i++){
		for(j=0;j<dim;j++){
		    double v=D[j*dim+i];
			sprintf(value,"%g ",v);
			strcat(s,value);
		}
	}
}

//-------------------------------------------------------------
// sscan matrix
//-------------------------------------------------------------
void Matrix::sscan(char *s){
	float F[16];
	sscanf(s,"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
	  F,F+1,F+2,F+3,F+4,F+5,F+6,F+7,F+8,F+9,F+10,F+11,F+12,F+13,F+14,F+15);
	for(int i=0;i<16;i++)
		D[i]=F[i];
}

//-------------------------------------------------------------
// print matrix
//-------------------------------------------------------------
double Matrix::difference(Matrix &b){
	return mdiff(D,b.D,dim);
}

//-------------------------------------------------------------
// print matrix
//-------------------------------------------------------------
double Matrix::magnitude(){
	return mmag(D,dim);
}

//-------------------------------------------------------------
// load identity matrix
//-------------------------------------------------------------
void Matrix::loadIdentity(){
	int i,j;
	for(i=0;i<dim;i++){
		for(j=0;j<dim;j++)
			D[i*dim+j]=(i==j?1.0:0.0);
	}
}

//-------------------------------------------------------------
// set values from an array
//-------------------------------------------------------------
void Matrix::setValues(double *d){
	for(int i=0;i<dim*dim;i++)
		D[i]=d[i];
}

//-------------------------------------------------------------
// multiply matrix
//-------------------------------------------------------------
Vector Matrix::operator*(Vector &v){
    Vector a;
    mvmul4(v.D,D,a.D);
    return Vector(a);
}
//-------------------------------------------------------------
// multiply matrix
//-------------------------------------------------------------
Point Matrix::operator*(Point &v){
	Point p;
	p.x=v.x*D[0]+v.y*D[4]+v.z*D[8]+D[12];
	p.y=v.x*D[1]+v.y*D[5]+v.z*D[9]+D[13];
	p.z=v.x*D[2]+v.y*D[6]+v.z*D[10]+D[14];
	return p;
}

//-------------------------------------------------------------
// multiply matrix
//-------------------------------------------------------------
Matrix Matrix::operator*(Matrix &b){
    Matrix a;
    CMmmul(b.D,D,a.D,dim);
    return Matrix(&a);
}

//-------------------------------------------------------------
// multiply matrix
//-------------------------------------------------------------
void Matrix::mm(Matrix &b){
    //Matrix a(D);
    Matrix a(this);
    CMmmul(b.D,a.D,D,dim);
}

//-------------------------------------------------------------
// index operator
//-------------------------------------------------------------
double & Matrix::operator[](int i){
     return D[i];
}

//-------------------------------------------------------------
// invert matrix
//-------------------------------------------------------------
Matrix Matrix::invert(){
    Matrix a;
    minv(D,a.D,dim);
    return Matrix(&a);
}


//************************************************************
// MatrixMgr class
//************************************************************

MatrixMgr::MatrixMgr(){
    size=8;
    MALLOC(size,Matrix,mats);
    current=0;
    mats[current].loadIdentity();
}

MatrixMgr::MatrixMgr(int n){
    size=n;
    current=0;
    MALLOC(size,Matrix,mats);
    mats[current].loadIdentity();
}

MatrixMgr::~MatrixMgr(){
	FREE(mats);
}

//-------------------------------------------------------------
// push matrix
//-------------------------------------------------------------
void MatrixMgr::pushMatrix(){
    current++;
    if(current>=size-1){
        printf("MatrixMgr stack overflow\n");
        current=size-1;
    }
    mats[current]=mats[current-1];
}

//-------------------------------------------------------------
// pop matrix
//-------------------------------------------------------------
void MatrixMgr::popMatrix(){
    current--;
    if(current<0){
        printf("MatrixMgr stack underflow\n");
        current=0;
    }
}

//-------------------------------------------------------------
// get matrix
//-------------------------------------------------------------
void MatrixMgr::getMatrix(Matrix &d){
    d=mats[current];
}

//-------------------------------------------------------------
// get matrix
//-------------------------------------------------------------
void MatrixMgr::getMatrix(double *d){
    for(int i=0;i<16;i++)
        d[i]=mats[current][i];
}

//-------------------------------------------------------------
// get matrix
//-------------------------------------------------------------
Matrix &MatrixMgr::getMatrix(){
    return mats[current];
}

//-------------------------------------------------------------
// set matrix
//-------------------------------------------------------------
void MatrixMgr::setMatrix(Matrix &d){
    mats[current]=d;
}
