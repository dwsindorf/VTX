#include <GLSLVars.h>
#include <GLSLMgr.h>

GLSLVarMgr::GLSLVarMgr() { 
}

GLSLVarMgr::~GLSLVarMgr() { 
	vars.free();
}

bool GLSLVarMgr::containsKey(char *key){
	if(vars.inlist(key))
		return true;
	return false;
}

UniformVar *GLSLVarMgr::getVar(char *key){
	return vars.inlist(key);
}

bool GLSLVarMgr::loadVar(char *key){
	UniformVar *var=getVar(key);
	if(var){
		var->setProgram(GLSLMgr::programHandle());
		var->load();
		return true;
	}
	return false;
}

UniformVar *GLSLVarMgr::addVar(char *key,UniformVar *var){
	UniformVar *oldvar=vars.inlist(key);
	// Only want one copy in array but calling "remove" for
	// old copy leads to run-time errors (looks like an array 
	// index location is getting skipped after remove, leaving bad
	// data in the hole - probably bug in ListClass.h)
	// As a workaround, return old copy and delete new one
	// (assumes caller may call setValue on returned object)
	if(oldvar){
		delete var;
		return oldvar;
	}
	vars.add(var);
	vars.sort();
	return var;
}

UniformVar *GLSLVarMgr::newVar(char *name){
	UniformVar *var=new UniformVar(name);	
	var=(UniformVar *)addVar(name,var);
	return var;
}

UniformVar *GLSLVarMgr::newVar(char *name,UniformVar *v){
	UniformVar *var=(UniformVar *)addVar(name,v);
	return var;
}

UniformBool *GLSLVarMgr::newBoolVar(char *name){
	UniformBool *var=new UniformBool(name);	
	var=(UniformBool *)addVar(name,var);
	return var;
}

UniformBool *GLSLVarMgr::newBoolVar(char *name,bool val){
	UniformBool *var=newBoolVar(name);
	var->set(val);
	return var;
}

UniformInt *GLSLVarMgr::newIntVar(char *name){
	UniformInt *var=new UniformInt(name);
	var=(UniformInt*)addVar(name,var);
	return var;
}
UniformInt *GLSLVarMgr::newIntVar(char *name,int val){
	UniformInt *var=newIntVar(name);
	var->set(val);
	return var;
}

UniformFloat *GLSLVarMgr::newFloatVar(char *name){
	UniformFloat *var=new UniformFloat(name);
	var=(UniformFloat*)addVar(name,var);
	return var;
}
UniformFloat *GLSLVarMgr::newFloatVar(char *name,float val){
	UniformFloat *var=newFloatVar(name);
	var->set(val);
	return var;
}

// UniformIntVec

UniformIntVec *GLSLVarMgr::newIntVec(char *name){
	UniformIntVec *var=new UniformIntVec(name);
	var=(UniformIntVec*)addVar(name,var);
	return var;	
}

UniformIntVec *GLSLVarMgr::newIntVec(char *name, int d){
	UniformIntVec *var=new UniformIntVec(name,d);
	var=(UniformIntVec*)addVar(name,var);
	return var;	
}

UniformIntVec *GLSLVarMgr::newIntVec(char *name, int *f, int d){
	UniformIntVec *var=new UniformIntVec(name,f,d);
	var=(UniformIntVec*)addVar(name,var);
	return var;	
}

UniformIntVec *GLSLVarMgr::newIntVec(char *name, int x, int y, int z){
	UniformIntVec *var=new UniformIntVec(name,x,y,z);
	var=(UniformIntVec*)addVar(name,var);
	return var;	
}

UniformIntVec *GLSLVarMgr::newIntVec(char *name, int x, int y, int z, int w){
	UniformIntVec *var=new UniformIntVec(name,x,y,z,w);
	var=(UniformIntVec*)addVar(name,var);
	return var;	
}


// UniformFloatVec

UniformFloatVec *GLSLVarMgr::newFloatVec(char *name){
	UniformFloatVec *var=new UniformFloatVec(name);
	var=(UniformFloatVec*)addVar(name,var);
	return var;	
}

UniformFloatVec *GLSLVarMgr::newFloatVec(char *name, int d){
	UniformFloatVec *var=new UniformFloatVec(name,d);
	var=(UniformFloatVec*)addVar(name,var);
	return var;	
}

UniformFloatVec *GLSLVarMgr::newFloatVec(char *name, float *f, int d){
	UniformFloatVec *var=new UniformFloatVec(name,f,d);
	var=(UniformFloatVec*)addVar(name,var);
	return var;	
}

UniformFloatVec *GLSLVarMgr::newFloatVec(char *name, float x, float y, float z){
	UniformFloatVec *var=new UniformFloatVec(name,x,y,z);
	var=(UniformFloatVec*)addVar(name,var);
	return var;	
}

UniformFloatVec *GLSLVarMgr::newFloatVec(char *name, float x, float y, float z, float w){
	UniformFloatVec *var=new UniformFloatVec(name,x,y,z,w);
	var=(UniformFloatVec*)addVar(name,var);
	return var;	
}


int GLSLVarMgr::getFloatVec(char *key, float *f){
	UniformVar *var=getVar(key);
	error=false;
	if(var != 0 && var->type==FLOATVECVAR){
		((UniformFloatVec*)var)->get(f);
		return ((UniformFloatVec*)var)->dim;		
	}
	return 0;
}

void  GLSLVarMgr::setFloatVec(char *key, float*v) { 
	setValue(key,v);
}

int GLSLVarMgr::getFloatVec(char *key, double *f){
	UniformVar *var=getVar(key);
	error=false;
	if(var != 0 && var->type==FLOATVECVAR){
		((UniformFloatVec*)var)->get(f);
		return ((UniformFloatVec*)var)->dim;		
	}
	return 0;
}

void  GLSLVarMgr::setFloatVec(char *key, double*v) { 
	UniformVar *var=getVar(key);
	error=true;
	if(var !=0 && var->type==FLOATVECVAR){
		((UniformFloatVec*)var)->set(v);
		error=false;
	}
}

void   GLSLVarMgr::setVecValue(char *key, float f, int index){
	UniformVar *var=getVar(key);
	if(var != 0 && var->type==FLOATVECVAR)
		((UniformFloatVec*)var)->value[index]=f;
		
}
float  GLSLVarMgr::getVecValue(char *key, int index){
	UniformVar *var=getVar(key);
	error=false;
	if(var != 0 && var->type==FLOATVECVAR && index>=0 && index<4)
		return ((UniformFloatVec*)var)->value[index];
	else{
		error=true;
		return 0;
	}
}
void  GLSLVarMgr::setVecValues(char *key, float *vals, int max){
	UniformVar *var=getVar(key);
	error=true;
	if(var != 0 && var->type==FLOATVECVAR && max<=4){
		error=false;		
		for(int i=0;i<max;i++)
			((UniformFloatVec*)var)->value[i]=vals[i];
	}
}
void GLSLVarMgr::getVecValues(char *key, float *vals, int max){
	UniformVar *var=getVar(key);
	error=true;
	if(var != 0 && var->type==FLOATVECVAR && max<=4){
		error=false;
		for(int i=0;i<max;i++)
			vals[i]=((UniformFloatVec*)var)->value[i];
	}
}

void  GLSLVarMgr::setValue(char *key, float *f){
	UniformVar *var=getVar(key);
	error=true;
	if(var !=0 && var->type==FLOATVECVAR){
		((UniformFloatVec*)var)->set(f);
		error=false;
	}
}

void  GLSLVarMgr::setValue(char *key, float x,float y){
	UniformVar *var=getVar(key);
	error=true;
	if(var !=0 && var->type==FLOATVECVAR){
		((UniformFloatVec*)var)->set(x,y);
		error=false;
	}
}
void  GLSLVarMgr::setValue(char *key, float x,float y,float z){
	UniformVar *var=getVar(key);
	error=true;
	if(var !=0 && var->type==FLOATVECVAR){
		((UniformFloatVec*)var)->set(x,y,z);
		error=false;
	}
}

void  GLSLVarMgr::setValue(char *key, float x,float y,float z,float w){
	UniformVar *var=getVar(key);
	error=true;
	if(var !=0 && var->type==FLOATVECVAR){
		((UniformFloatVec*)var)->set(x,y,z,w);
		error=false;
	}	
}
 

// UniformIntArray

UniformIntArray *GLSLVarMgr::newIntArray(char *name, GLint *v, int size){
	UniformIntArray *var=new UniformIntArray(name,v,size);
	var=(UniformIntArray*)addVar(name,var);
	return var;	
}

// UniformFloatArray

UniformFloatArray *GLSLVarMgr::newFloatArray(char *name,GLfloat*v, int size){
	UniformFloatArray *var=new UniformFloatArray(name,v,size);
	var=(UniformFloatArray*)addVar(name,var);
	return var;	
}

UniformFloatArray *GLSLVarMgr::newFloatArray(char *name,GLdouble*v, int size){
	UniformFloatArray *var=new UniformFloatArray(name,v,size);
	var=(UniformFloatArray*)addVar(name,var);
	return var;
}


void  GLSLVarMgr::setIntArrayValue(char *key, GLint val, int index){
	UniformVar *var=getVar(key);
	error=true;
	if(var !=0){
		((UniformIntArray*)var)->value[index]=val;
		error=false;
	}	
}

void  GLSLVarMgr::setFloatArrayValue(char *key, GLfloat val, int index){
	UniformVar *var=getVar(key);
	error=true;
	if(var !=0){
		((UniformFloatArray*)var)->value[index]=val;
		error=false;
	}	
}

GLfloat GLSLVarMgr::getFloatArrayValue(char *key, int index){
	UniformVar *var=getVar(key);
	error=true;
	if(var != 0 && var->type==FLOATARRAYVAR){
		UniformFloatVec *array=(UniformFloatVec*)var;
		if(index>=0 && index<array->dim){
			error=false;
			return array->value[index];
		}		
	}
	return 0.0;
}

GLint GLSLVarMgr::getIntArrayValue(char *key, int index){
	UniformVar *var=getVar(key);
	error=true;
	if(var != 0 && var->type==INTARRAYVAR){
		UniformIntVec *array=(UniformIntVec*)var;
		if(index>=0 && index<array->dim){
			error=false;
			return array->value[index];
		}		
	}
	return 0;	
}

void GLSLVarMgr::setInProgram(char *key, bool b){
	UniformVar *var=getVar(key);
	if(var !=0)
		var->setInProgram(b);
	
}

void GLSLVarMgr::setInFile(char *key, bool b){
	UniformVar *var=getVar(key);
	if(var !=0)
		var->setInFile(b);
	
}


void GLSLVarMgr::setValue(char *key, float val){
	UniformVar *var=getVar(key);
	if(var !=0 && var->type==FLOATVAR){
		((UniformFloat*)var)->set(val);
		error=false;
	}
	else
		error=true;		
}
void GLSLVarMgr::setValue(char *key, bool val){
	UniformVar *var=getVar(key);
	if(var !=0 && var->type==BOOLVAR){
		((UniformBool*)var)->set(val);
		error=false;
	}
	else
		error=true;		
}

void GLSLVarMgr::setValue(char *key, int val){
	UniformVar *var=getVar(key);
	if(var !=0 && var->type==INTVAR){
		((UniformInt*)var)->set(val);
		error=false;
	}
	else
		error=true;		
}

float GLSLVarMgr::getFloatValue(char *key){
	UniformVar *var=getVar(key);
	if(var !=0 && var->type==FLOATVAR){
		error=false;
		return ((UniformFloat*)var)->get();
	}
	error=true;	
	return 0.0;
}

bool GLSLVarMgr::getBoolValue(char *key){
	UniformVar *var=getVar(key);
	if(var !=0 && var->type==BOOLVAR){
		error=false;
		return ((UniformBool*)var)->get();
	}
	error=true;	
	return false;
}

int GLSLVarMgr::getIntValue(char *key){
	UniformVar *var=getVar(key);
	if(var !=0 && var->type==INTVAR){
		error=false;
		return ((UniformInt*)var)->get();
	}
	error=true;	
	return false;
}

void GLSLVarMgr::loadProgram(){
	GLhandleARB p=GLSLMgr::programHandle();
	vars.ss();
	UniformVar *var;
	while((var=vars++)){
		var->setProgram(p);
	}
}

void GLSLVarMgr::setProgram(GLuint p){
	UniformVar *var;
	vars.ss();
	while((var=vars++)){
		var->setProgram(p);
	}
}
void GLSLVarMgr::loadVars(){
	UniformVar *var;
	for(int i=0;i<vars.size;i++){
		var=vars[i];
		var->load();
	}

}
