#ifndef GLSLVARS_H_
#define GLSLVARS_H_

#include <GLheaders.h>
#include <FrameBuffer.h>
#include <ListClass.h>

#include <stdio.h>
#include <stdlib.h>


enum vartypes {
	BOOLVAR  =0,
	INTVAR   =1,
	FLOATVAR =2,
	FLOATVECVAR =3,	
	INTVECVAR   =4,	
	FLOATARRAYVAR =5,	
	INTARRAYVAR =6,	
	USERVAR =7,	
};

class UniformVar
{
public:
	int id;
	int type;
	bool inprogram;
	bool infile;
	const char	*text;
	int				value;
	char *name()		{ 	return (char*)text;}
	int  cmp(char *s)	{	return strcmp(text,s);}

	GLhandleARB program;
	UniformVar(const char *s){
		text=s;
		id=-1;
		type=-1;
		inprogram=true;
		infile=true;
		program=0;
	}
	UniformVar(const char *s, int t){
		text=s;
		id=-1;
		type=t;
		inprogram=true;
		infile=true;
		program=0;
	}
	bool inFile(){
		return infile;
	}
	void setInFile(bool b){
		infile=b;
	}
	bool inProgram(){
		return inprogram;
	}
	void setInProgram(bool b){
		inprogram=b;
	}
	bool setProgram(GLhandleARB programObj){
		program=programObj;
		if(program==0)
			return false;
		if(!inprogram)
			return true;
		id = glGetUniformLocationARB(program, text);
		if (id == -1){
			return false;
		}
		return true;
	}	
	virtual void load(){}
	virtual void getDeclaration(char *s){}
	virtual void getValue(char *s){}
	virtual void setValue(char *s){}

};

class UniformBool : public UniformVar
{
public:
	GLboolean value;
	UniformBool(const char *s) : UniformVar(s,BOOLVAR){value=false;}
	UniformBool(const char *s,bool b) : UniformVar(s,BOOLVAR){value=b;}
	GLboolean get(){
		return value;
	}
	void set(GLboolean b){
		value=b;
	}
	void load(GLboolean b){
		value=b;
		load();
	}
	void load(){
	if (id != -1 && program !=0 && inprogram)
		glUniform1iARB(id, value);
	}
	void getDeclaration(char *s){
		if (inprogram)
			sprintf(s,"uniform bool %s;\n",text);
	}
	void getValue(char *s){
	    if(strcmp(s,"true")==0)
	    	value=true;
	    else
	    	value=false;
	}
	void setValue(char *s){
		sprintf(s,"%s",(value==true)?"true":"false");
	}
};

class UniformInt : public UniformVar
{
public:
	GLint value;
	UniformInt(const char *s) : UniformVar(s,INTVAR){value=0;}
	UniformInt(const char *s,int i) : UniformVar(s,INTVAR){value=i;}
	GLint get(){
		return value;
	}
	void set(int i){
		value=i;
	}
	void load(int i){
		value=i;
		load();
	}
	void load(){
	if (id != -1 && program !=0&& inprogram)
		glUniform1iARB(id, value);
	}
	void getDeclaration(char *s){
		if (inprogram)
			sprintf(s,"uniform int %s;\n",text);
	}
	void getValue(char *s){
	    sscanf(s,"%d",&value);	
	}
	void setValue(char *s){
		sprintf(s,"%d",value);
	}

};
class UniformFloat : public UniformVar
{
public:
	GLfloat value;
	UniformFloat(const char *s) : UniformVar(s,FLOATVAR){value=0;}
	UniformFloat(const char *s,float f) : UniformVar(s,FLOATVAR){value=f;}
	GLfloat get(){
		return value;
	}
	void set(float f){
		value=f;
	}
	void load(float f){
		value=f;
		load();
	}
	void load(){
	if (id != -1 && inprogram)
		glUniform1fARB(id, value);
	}
	void getDeclaration(char *s){
		if (inprogram)
			sprintf(s,"uniform float %s;\n",text);
	}
	void getValue(char *s){
	    sscanf(s,"%g",&value);		
	}
	void setValue(char *s){
		sprintf(s,"%g",value);
	}
};

class UniformFloatVec : public UniformVar
{
public:
	GLfloat value[4];
	int dim;
	void set(float x, float y){
		value[0]=x;value[1]=y;dim=2;
	}
	void set(float x, float y, float z){
		value[0]=x;value[1]=y;value[2]=z;dim=3;
	}
	void set(float x, float y, float z,float w){
		value[0]=x;value[1]=y;value[2]=z;value[3]=w;dim=4;
	}
	void set(float *src){
		for(int i=0;i<dim;i++)
			value[i]=src[i];
	}
	void get(float *dst){
		for(int i=0;i<dim;i++)
			dst[i]=value[i];
	}
	void set(double *src){
		for(int i=0;i<dim;i++)
			value[i]=(float)src[i];
	}
	void get(double *dst){
		for(int i=0;i<dim;i++)
			dst[i]=(double)value[i];
	}

	UniformFloatVec(const char *s) : UniformVar(s,FLOATVECVAR){
		value[0]=value[1]=value[2]=0;
		value[3]=1;
		dim=3;
	}
	UniformFloatVec(const char *s,int n) : UniformVar(s,FLOATVECVAR){
		dim=n;
		value[3]=1;
		for(int i=0;i<dim;i++)
			value[i]=0;
	}
	UniformFloatVec(const char *s,float *d, int n) : UniformVar(s,FLOATVECVAR){
		dim=n;
		value[3]=1;
		for(int i=0;i<dim;i++)
			value[i]=d[i];
	}
	UniformFloatVec(const char *s,float x,float y) : UniformVar(s,FLOATVECVAR){
		value[0]=x;
		value[1]=y;
		value[2]=0;
		value[3]=1;
		dim=2;
	}
	UniformFloatVec(const char *s,float x,float y, float z) : UniformVar(s,FLOATVECVAR){
		set(x,y,z);	
	}
	UniformFloatVec(const char *s,float x,float y, float z,float w) : UniformVar(s,FLOATVECVAR){
		set(x,y,z,w);
	}
	void load(){
	if (id != -1 && inprogram)
		switch(dim){
		case 2:
			glUniform2fARB(id, value[0],value[1]);
			break;
		case 3:
			glUniform3fARB(id, value[0],value[1],value[2]);
			break;
		case 4:
			glUniform4fARB(id, value[0],value[1],value[2],value[3]);
			break;
		}
	}
	virtual void getDeclaration(char *s){
		if (inprogram)
			sprintf(s,"uniform vec%d %s;\n",dim,text);
	}
	void getValue(char *s){
		switch(dim){
		case 2:
			sscanf(s,"%g %g",value,value+1);
			break;
		case 3:
			sscanf(s,"%g %g %g",value,value+1,value+2);
			break;
		case 4:
			sscanf(s,"%g %g %g %g",value,value+1,value+2,value+3);
			break;
		}
	}
	void setValue(char *s){
		switch(dim){
		case 2:
			sprintf(s,"%g %g",value[0],value[1]);
			break;
		case 3:
			sprintf(s,"%g %g %g",value[0],value[1],value[2]);
			break;
		case 4:
			sprintf(s,"%g %g %g %g",value[0],value[1],value[2],value[3]);
			break;
		}
	}
};


class UniformIntVec : public UniformVar
{
public:
	GLint value[4];
	int dim;
	void set(int x, int y){
		value[0]=x;value[1]=y;dim=2;
	}
	void set(int x, int y, int z){
		value[0]=x;value[1]=y;value[2]=z;dim=3;
	}
	void set(int x, int y, int z,int w){
		value[0]=x;value[1]=y;value[2]=z;value[3]=w;dim=4;
	}
	void set(int *src){
		for(int i=0;i<dim;i++)
			value[i]=src[i];
	}
	void get(int *dst){
		for(int i=0;i<dim;i++)
			dst[i]=value[i];
	}

	UniformIntVec(const char *s) : UniformVar(s,INTVECVAR){
		value[0]=value[1]=value[2]=0;
		value[3]=1;
		dim=3;
	}
	UniformIntVec(const char *s,int n) : UniformVar(s,INTVECVAR){
		dim=n;
		value[3]=1;
		for(int i=0;i<dim;i++)
			value[i]=0;
	}
	UniformIntVec(const char *s,int *d, int n) : UniformVar(s,INTVECVAR){
		dim=n;
		value[3]=1;
		for(int i=0;i<dim;i++)
			value[i]=d[i];
	}
	UniformIntVec(const char *s,int x,int y) : UniformVar(s,INTVECVAR){
		value[0]=x;
		value[1]=y;
		value[2]=0;
		value[3]=1;
		dim=2;
	}
	UniformIntVec(const char *s,int x,int y, int z) : UniformVar(s,INTVECVAR){
		set(x,y,z);	
	}
	UniformIntVec(const char *s,int x,int y, int z,int w) : UniformVar(s,INTVECVAR){
		set(x,y,z,w);
	}
	void load() {
		if (id != -1 && inprogram)
			glUniform1iv(id,dim,value);
	}
	void getValue(char *s){
		switch(dim){
		case 2:
			sscanf(s,"%d %d",value,value+1);
			break;
		case 3:
			sscanf(s,"%d %d %d",value,value+1,value+2);
			break;
		case 4:
			sscanf(s,"%d %d %d %d",value,value+1,value+2,value+3);
			break;
		}
	}
	void setValue(char *s){
		switch(dim){
		case 2:
			sprintf(s,"%d %d",value[0],value[1]);
			break;
		case 3:
			sprintf(s,"%d %d %d",value[0],value[1],value[2]);
			break;
		case 4:
			sprintf(s,"%d %d %d %d",value[0],value[1],value[2],value[3]);
			break;
		}
	}
};

class UniformIntArray : public UniformVar
{
public:
	GLint *value;
	int dim;
	UniformIntArray(const char *s,GLint *v, int n) : UniformVar(s,INTARRAYVAR){
		dim=n;
		value=v;
	}
	void load() {
		if (id != -1 && inprogram)
			glUniform1iv(id,dim,value);
	}
};

class UniformFloatArray : public UniformVar
{
public:
	GLfloat *value;
	int dim;
	UniformFloatArray(const char *s,GLfloat *v, int n) : UniformVar(s,FLOATARRAYVAR){
		dim=n;
		MALLOC(n,GLfloat,value);
		for(int i=0;i<n;i++)
			value[i]=v[i];
	}
	UniformFloatArray(const char *s,GLdouble *v, int n) : UniformVar(s,FLOATARRAYVAR){
		dim=n;
		MALLOC(n,GLfloat,value);
		for(int i=0;i<n;i++)
			value[i]=(GLfloat)v[i];
	}
	void load() {
		if (id != -1 && inprogram){
			if(dim==16)
				glUniformMatrix4fv(id,1,false,value);
			else
			    glUniform1fv(id,dim,value);
		}
	}
	~UniformFloatArray(){
		free(value);
	}
};


class GLSLVarMgr
{
public:
	bool error;

	GLSLVarMgr();
	~GLSLVarMgr();

	NameList<UniformVar*> vars;
	
	virtual void loadProgram();
	virtual void loadVars();
	virtual void setProgram(GLhandleARB p);
	
	virtual UniformVar *addVar(char *key, UniformVar *var);
	virtual UniformVar *getVar(char *key);
	virtual bool containsKey(char *);
	
	virtual bool loadVar(char *key);
	
	virtual UniformVar *newVar(char *key);
	virtual UniformVar *newVar(char *key,UniformVar *v);
	
	virtual UniformFloat *newFloatVar(char *key);
	virtual UniformFloat *newFloatVar(char *key,float value);
	
	virtual UniformBool  *newBoolVar(char *key);
	virtual UniformBool  *newBoolVar(char *key,bool value);
	virtual UniformInt   *newIntVar(char *key);
	virtual UniformInt   *newIntVar(char *key,int value);
	
	virtual UniformFloatVec *newFloatVec(char *key);
	virtual UniformFloatVec *newFloatVec(char *key, int);
	virtual UniformFloatVec *newFloatVec(char *key, float*,int);
	virtual UniformFloatVec *newFloatVec(char *key, float x, float y, float z);
	virtual UniformFloatVec *newFloatVec(char *key, float x, float y, float z, float w);

	virtual UniformIntVec *newIntVec(char *key);
	virtual UniformIntVec *newIntVec(char *key, int);
	virtual UniformIntVec *newIntVec(char *key, int*,int);
	virtual UniformIntVec *newIntVec(char *key, int x, int y, int z);
	virtual UniformIntVec *newIntVec(char *key, int x, int y, int z, int w);
	
	virtual UniformIntArray *newIntArray(char *name,GLint *v, int size);
	virtual UniformFloatArray *newFloatArray(char *name, GLfloat *v, int size);
	virtual UniformFloatArray *newFloatArray(char *name, GLdouble *v, int size);
	
	virtual void  setIntArrayValue(char *key, GLint val, int index);
	virtual void  setFloatArrayValue(char *key, GLfloat val, int index);

	virtual GLint    getIntArrayValue(char *key, int index);
	virtual GLfloat  getFloatArrayValue(char *key, int index);

	virtual void  setVecValue(char *key, float val, int index);
	virtual float getVecValue(char *key, int index);
	virtual void  setVecValues(char *key, float *vals, int max);
	virtual void  getVecValues(char *key,  float *vals, int max);
	virtual void  setValue(char *, bool);
	virtual bool  getBoolValue(char*);

	virtual void  setValue(char *, int);
	virtual int  getIntValue(char*);

	virtual void setInProgram(char *, bool);
	virtual void setInFile(char *, bool);
	
	virtual void  setValue(char *, float);
	virtual float getFloatValue(char*);
	
	virtual int getFloatVec(char*,float*);
	virtual void  setFloatVec(char *key, float*v);

	virtual int getFloatVec(char*,double*);
	virtual void  setFloatVec(char *key, double*v);

	virtual void  setValue(char *, float,float);
	virtual void  setValue(char *, float,float,float);
	virtual void  setValue(char *, float,float,float,float);
	virtual void  setValue(char *, float*);

};

#endif /*GLSLVARS_H_*/
