
//  MatrixClass.h

#ifndef _MATRIXCLASS
#define _MATRIXCLASS

#include "defs.h"
#include "matrix.h"
#include "PointClass.h"
#include <math.h>
#include <stdio.h>

class Vector 
{
protected:
 	enum {dim=4};   
    double D[4];
public:   
    
    Vector(); 
    Vector(Point  &p); 
    Vector(double *a); 
    Vector(Vector *a); 

    int size()       { return dim;}
    double *values() { return D;}
	Vector operator*(double s);
	Vector operator*(Vector &p);
	Vector operator+(Vector &p);
	Vector operator-(Vector &p);
	double & operator[](int i);
	void print();
	friend class Matrix;
};

class Matrix 
{
protected:
 	enum {dim=4};   
    double D[16];
public:      
    Matrix();
    Matrix(int); 
    Matrix(int,int); 
    Matrix(double *a); 
    Matrix(float *a);
    
    Matrix(Matrix *a); 
    int size()       { return dim;}
    double *values() { return D;}
	void setValues(double *);
	Point  operator*(Point &p);
	Vector operator*(Vector &p);
	Matrix operator*(double s);
	Matrix operator*(Matrix &p);
	Matrix operator+(Matrix &p);
	Matrix operator-(Matrix &p);
	double & operator[](int i);
	Matrix invert();
	void mm(Matrix &p);
	void print();
	void sprint(char *);
	void sscan(char *);
	double difference(Matrix &p);
	double magnitude();
	void loadIdentity();
	friend class Vector;
};

class MatrixMgr 
{
 	int size;
 	Matrix *mats;
    int current;
public:
	MatrixMgr();
	MatrixMgr(int);
	~MatrixMgr();
	
    void pushMatrix();
    void popMatrix();
    
    void setMatrix(Matrix &d);
    void setMatrix(double *d);
    void getMatrix(Matrix &d);  
    void getMatrix(double *d);  
    Matrix &getMatrix();  
};

#endif
