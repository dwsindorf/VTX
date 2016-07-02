// Fractal.h

#ifndef _FRACTAL
#define _FRACTAL

#include "TerrainMgr.h"

//************************************************************
// Class TNfchnl
//************************************************************
class TNfractal : public TNfunc
{
protected:
	void optionString(char *);
public:
	int options;
	TNfractal(int l, TNode *a, TNode *r);
	void eval();
	void save(FILE*);
	void valueString(char *s);
	int typeValue()			{ return ID_FCHNL;}
	const char *typeName ()	{ return "fractal";}
	void propertyString(char *s);
};


#endif

