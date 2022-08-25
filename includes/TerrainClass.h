// TerrainClass.h

#ifndef _TERRAIN_CLASS
#define _TERRAIN_CLASS

#include "TerrainMgr.h"

class ExprMgr;
class TerrainSym;
class TNode;
class TerrainData;
class TNarg;
class Image;
class Texture;
class Object3D;

// TNpoint codes

//~~~~~~~~~~~~~~~~~ terminal nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//************************************************************
// Class TNconst
//************************************************************
class TNconst : public TNode
{
public:
	double value;
	TNconst(double c) {value=c;}
	void dump()				{ printf("const: %g\n",value);}
	void eval();
	int typeValue()			{ return ID_CONST;}
	const char *typeName ()	{ return "constant";}
	void valueString(char *s);
	void save(FILE *f);
};

//************************************************************
// Class TNstring
//************************************************************
class TNstring : public TNode
{
public:
	char *value;
	TNstring(char *c) 		{ value=c;}
	~TNstring();
	void dump()				{ printf("string: %s\n",value);}
	void eval();
	int typeValue()			{ return ID_STRING;}
	const char *typeName ()	{ return "string";}
	void valueString(char *s);
	void save(FILE *f);
};

//************************************************************
// Class TNglobal
//************************************************************
class TNglobal : public TNode
{
	int gtype;
public:
	TNglobal(int c) {gtype=c;}
	void init();
	void eval();
	void save(FILE *f);
	int typeValue()			{ return ID_GLOBAL;}
	const char *typeName ()	{ return "global";}
	void valueString(char *s);
};

//~~~~~~~~~~~~~~~~~ unary nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//************************************************************
// Class TNpoint
//************************************************************
class TNpoint : public TNvector
{
public:
	TNnoise *snoise;
	TNpoint(TNode *r) : TNvector(r) {}
	void dump()				{ printf("%s()\n",symbol());}
	void eval();
	int typeValue()			{ return ID_POINT;}
	const char *typeName()	{ return "point";}
	const char *symbol();
	void valueString(char *);
	void addToken(LinkedList<TNode*>&);
	bool initProgram();
	bool setProgram();
	TNode *operator[](int i);
};

//************************************************************
// Class TNzbase
//************************************************************
class TNzbase : public TNvector
{
public:
	TNzbase(TNode *r) :  TNvector(r) {}
	const char *typeName()	{ return "base";}
	int typeValue()			{ return ID_ZBASE;}
	void eval();
};

//************************************************************
// Class TNgloss
//************************************************************
class TNgloss : public TNvector
{
public:
	TNgloss(TNode *r) : TNvector(r) {}
	void eval();
	int typeValue()			{ return ID_GLOSS;}
	const char *typeName ()	{ return "gloss";}
};

//************************************************************
// Class TNcolor
//************************************************************
class TNcolor : public TNvector
{
public:
	TNcolor(TNode *r) : TNvector(r) {}
	void eval();
	void init();
	int typeValue()			{ return ID_COLOR;}
	const char *typeName ()	{ return "color";}
	const char *symbol();
	bool initProgram();

	void addToken(LinkedList<TNode*>&);
	TNode *operator[](int i);
};

//************************************************************
// Class TNdensity
//************************************************************
class TNdensity : public TNvector
{
public:
	TNdensity(TNode *r) : TNvector(r) {}
	void eval();
	int typeValue()			{ return ID_DENSITY;}
	const char *typeName ()	{ return "Density";}
	const char *symbol()	{ return "Density";}
	void init();
	bool initProgram();
};

//************************************************************
// Class TNclist
//************************************************************
class TNclist : public TNvector
{
public:
    Array<Color *>colors;
	TNclist(TNode *r): TNvector(r) {}
	~TNclist();
	int typeValue()			{ return ID_CLIST;}
	const char *typeName ()	{ return "colors";}
	const char *symbol()	{ return "Colors";}
	int size()				{ return colors.size;}
	void init();
	void save(FILE *f);
	void set_color(int i,Color c);
	Color get_color(int i);
	Color color(double f);
};


//************************************************************
// Class TNnoise
//************************************************************
class TNnoise : public TNvector
{
protected:
    enum {
        TNN_NORM        = 1,
        TNN_NORMALIZED  = 2
    };
	void optionString(char *);
	int flags;
public:
	double scale;
	double rate;

	double mn,mx,ma,mb;
    int type;
    int id;
    TNnoise(int t, TNode *m);
	void save(FILE*);
	void eval();
	void init();
	int norm() 			        { return flags & TNN_NORM;}
	int normalized()            { return flags & TNN_NORMALIZED;}
	void set_normalized()       { BIT_ON(flags,TNN_NORMALIZED);}
	void clr_normalized()       { BIT_OFF(flags,TNN_NORMALIZED);}

	int typeValue()			    { return ID_NOISE;}
	const char *typeName ()		{ return "noise";}
	void valueString(char *);
	void propertyString(char *);
	bool setProgram();
	void addToken(LinkedList<TNode*>&);
};

//~~~~~~~~~~~~~~~~~ binary nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//************************************************************
// Class TNwater
//************************************************************
class TNwater : public TNfunc
{
public:
	TNwater(TNode *l, TNode *r) : TNfunc(l,r) {}
	void eval();
	void saveNode(FILE *);
	NodeIF *replaceNode(NodeIF *c);
	int typeValue()			{ return ID_WATER;}
	const char *typeName ()	{ return "ocean";}
	NodeIF *getInstance();
	static NodeIF *newInstance();
};

//************************************************************
// Class TNfog
//************************************************************
class TNfog : public TNfunc
{
public:
	TNfog(TNode *l, TNode *r) : TNfunc(l,r) {}
	void eval();
	void saveNode(FILE *);
	NodeIF *replaceNode(NodeIF *c);
	int typeValue()			{ return ID_FOG;}
	const char *typeName ()	{ return "fog";}
};


//************************************************************
// Class TNsnow
//************************************************************
class TNsnow : public TNfunc
{
public:
	Texture *texture;
	Image *image;
	TNsnow(TNode *l, TNode *r);
	~TNsnow();
	void init();
	void eval();
	bool initProgram();
	int typeValue()			{ return ID_SNOW;}
	const char *typeName ()	{ return "snow";}
};


//************************************************************
// Class TNclouds
//************************************************************
class TNclouds :  public TNfunc
{
public:
	int opts;
	TNclouds(TNode *l, TNode *r) : TNfunc(l,r) {opts=0;}
	TNclouds(int i, TNode *l, TNode *r) : TNfunc(l,r) {opts=i;}
	void eval();
	void init();
	int typeValue()			{ return ID_CLOUD;}
	const char *typeName ()	{ return "clouds";}
	bool initProgram();
	bool setProgram();
	bool threeD();
};
#endif
