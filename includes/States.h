#ifndef _ID_STATES_CLASS
#define _ID_STATES_CLASS

#include "TerrainMgr.h"

#define NUM_OCEAN_TYPES 7
class MaterialState : public TNunary {
public:
	char name[256];
	char expr[512];

	MaterialState();
	MaterialState(TNode *a);
	Color color1;
	Color color2;
	TNcolor *color;
	char colorexpr[256];

	double clarity;
	double mix;
	double specular;
	double shine;
	double temp;
	double trans_temp;
	double volatility;
	
	void setColor(TNcolor*);
	TNcolor* getColor();


	void setName(char *e){ strcpy(name,e);}
	char *getName(){ return name;}

    void setExpr(char *e){ strcpy(expr,e);}
    char *getExpr(){ return expr;}
	void valueString(char *s);
	void saveNode(FILE *);

};
class LiquidState : public MaterialState {
public:
	LiquidState(){}
	LiquidState(TNode *a);
	const char *symbol()	{ return "liquid";}
	int typeValue()			{ return ID_LIQUID;}
	static void newInstance(char *);
	static NodeIF *newInstance();
};
class SolidState : public MaterialState {
public:
	SolidState(){}
	SolidState(TNode *a);
	const char *symbol()	{ return "solid";}
	int typeValue()			{ return ID_SOLID;}
	static void newInstance(char *);
	static NodeIF *newInstance();
};
class OceanState  : public TNfunc {
public:
	OceanState(TNode *l, LiquidState *m1, SolidState *m2);
	int typeValue()			{ return ID_OCEAN;}
	double rseed;

	LiquidState *liquid;
	SolidState *solid;
	TNode  *ocean_expr;
	char expr[512];
	
	enum Types {LAV,SLF,H2O,SO2,CO2,CH4,N2};

	double evalOceanFunction();

	char *getOceanName()            { return nodeName();}
	void setOceanName(char *s)      { setName(s);}

	char *getOceanLiquidExpr()      { return liquid->getExpr();}
	void setOceanLiquidExpr(char *s){ liquid->setExpr(s);}

	char *getOceanSolidExpr()      { return solid->getExpr();}
	void setOceanSolidExpr(char *s){ solid->setExpr(s);}
	void valueString(char *s);

	void getOceanFunction(char *buff);
	bool setOceanFunction(char *expr);
	char *getOceanExpr();
	void setOceanExpr();
	
	static char *getDfltOceanExpr();
	static char *getDfltOceanSolidExpr();
	static char *getDfltOceanLiquidExpr();
	static OceanState *makeDefaultState(char *,char*,char *,char *);
	
	static Array<OceanState *>oceanTypes;
	static char *oceanNames[NUM_OCEAN_TYPES];
	static void setDefaults();
	void saveNode(FILE *);
	
	bool randomize();
	void setRseed(double s) {rseed=s;}
	double getRseed() {return rseed;}
	
	static NodeIF *newInstance();
};

#endif
