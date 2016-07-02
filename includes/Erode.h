// Erode.h

#ifndef _ERODE
#define _ERODE

#include "TerrainMgr.h"


//************************************************************
// Class TNhardness
//************************************************************
class TNhardness : public TNfunc
{
public:
	TNhardness(TNode *l, TNode *r);
	void eval();
	int typeValue()			{ return ID_HARDNESS;}
	const char *typeName ()	{ return "hardness";}
};

//************************************************************
// Class TNerode
//************************************************************
class TNerode : public TNfunc
{
	int options;
	void optionString(char *);
public:
	TNerode(int t, TNode *l, TNode *r);
	void eval();
	int typeValue()			{ return ID_ERODE;}
	void save(FILE*);
	const char *typeName ()	{ return "erode";}
	void propertyString(char *s);
};

#endif

