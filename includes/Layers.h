// Layers.h

#ifndef _LAYERS
#define _LAYERS

#include "TerrainMgr.h"

#define DFLT_DROP 0.0
#define DFLT_WIDTH 0.1

enum  {
	MESH   = 0x0100,  // dual surface
	MORPH  = 0x0200,  // ht blending
	CBLEND = 0x1000,  // color blending
	ZONLY  = 0x2000,  // single surface
	FSQR   = 0x4000,  // square f
	LOPTS  = 0xf000,  // option mask
	LTYPES = 0x0f00   // type mask
};

//************************************************************
// Class TNmap
//************************************************************
class TNmap : public TNfunc
{
protected:
public:
	double margin;
	double mht;
	double mbase;
	double mscale;
	double mdrop;
	double mmorph;
	double msmooth;

    int layers;
	TNmap(TNode *l, TNode *r);
	void eval();
	void init();
	int typeValue()			{ return ID_MAP;}
	const char *typeName ()	{ return "map";}
	int valueValid()				{ return 1;}
	TNode *operator[](int i);
	void valueString(char*);
	void setdrop(double d);
	void setmorph(double d);

	NodeIF *replaceNode(NodeIF *c);
	void saveNode(FILE *f);

	NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	NodeIF *removeNode();
	NodeIF *addChild(NodeIF *x);
	bool randomize();
	void setRandom(bool b);
	void setDefault();

	bool canRandomize() { return true;}
	bool isLeaf()		{ return false;}

	friend class TNlayer;
};

//************************************************************
// Class TNlayer
//************************************************************
class TNlayer : public TNbase
{
protected:
public:
	TNmap *map;
    char name_str[64];

	double edge;
	double width;
	double drop;
	double morph;
	double ramp;
	int id;
	int optionString(char *);
	TNlayer(TNode *l, TNode *r,TNode *b);
	void eval();
	void removeImages();
	void init();
	void save(FILE*);
	void saveNode(FILE *f);
	NodeIF *replaceNode(NodeIF *c);
	bool hasChild(int );
	NodeIF *findChild(int );
	int typeValue()			{ return ID_LAYER;}
	const char *typeName()	{ return "layer";}
	void valueString(char*);
	void propertyString(char *s);
	void setExpr(char*);
	void applyExpr();
	NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	bool isLeaf()						{ return false;}
	void raise();
	void lower();
	void setName(char*);
	char *getName();

	char *nodeName()  { return name_str;}
	void setEnabled(bool b);
	NodeIF *addChild(NodeIF *x);
	friend class TNmap;
	NodeIF *getInstance();

};
#endif

