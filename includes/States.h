#ifndef _ID_STATES
#define _ID_STATES

#include "TerrainMgr.h"

class MaterialState : public TNunary {
public:
	char name[256];
	char expr[512];

	MaterialState();
	MaterialState(TNode *a);
	Color color1;
	Color color2;
	double clarity;
	double mix;
	double specular;
	double shine;
	double temp;
	void setName(char *e){ strcpy(name,e);}
	char *getName(){ return name;}

    void setExpr(char *e){ strcpy(expr,e);}
    char *getExpr(){ return expr;}
	void print(char *suffix);
	void valueString(char *s);
	void saveNode(FILE *);

};
class LiquidState : public MaterialState {
public:
	LiquidState(){}
	LiquidState(TNode *a);
	const char *symbol()	{ return "liquid";}
	int typeValue()			{ return ID_LIQUID;}
};
class SolidState : public MaterialState {
public:
	SolidState(){}
	SolidState(TNode *a);
	const char *symbol()	{ return "solid";}
	int typeValue()			{ return ID_SOLID;}
};
class OceanState  : public TNfunc {
public:
	OceanState(TNode *l, LiquidState *m1, SolidState *m2);

	int typeValue()			{ return ID_OCEAN;}

	LiquidState *liquid;
	SolidState *solid;
	void setWaterColor1(Color c) 	{liquid->color1=c;}
	void setWaterColor2(Color c) 	{liquid->color2=c;}
	void setIceColor1(Color c)   	{solid->color1=c;}
	void setIceColor2(Color c)   	{solid->color2=c;}
	Color waterColor1()          	{return liquid->color1;}
	Color waterColor2()          	{return liquid->color2;}
	Color iceColor1()			 	{return solid->color1;}
	Color iceColor2()			 	{return solid->color2;}

	double waterClarity()		 	{return liquid->clarity;}
	double waterMix()			 	{return liquid->mix;}
	double waterSpecular()       	{return liquid->specular;}
	double waterShine()			 	{return liquid->shine;}
	void setWaterClarity(double f)  {liquid->clarity=f;}
	void setWaterMix(double f)      {liquid->mix=f;}
	void setWaterSpecular(double f) {liquid->specular=f;}
	void setWaterShine(double f) 	{liquid->shine=f;}

	double iceClarity()				{return solid->clarity;}
	double iceMix()					{return solid->mix;}
	double iceSpecular()			{return solid->specular;}
	double iceShine()				{return solid->shine;}
	void setIceClarity(double f)	{solid->clarity=f;}
	void setIceMix(double f)		{solid->mix=f;}
	void setIceSpecular(double f)	{solid->specular=f;}
	void setIceShine(double f)		{solid->shine=f;}
	
	double oceanSolidTemp()        { return solid->temp;}
	double oceanGasTemp()           { return liquid->temp;}
	void setOceanSolidTemp(double f) { solid->temp=C2K(f);}
	void setOceanGasTemp(double f)    { liquid->temp=C2K(f);}
	
	char *getOceanName()            { return getName();}
	void setOceanName(char *s)      { setName(s);}

	char *getOceanLiquidExpr()      { return liquid->getExpr();}
	void setOceanLiquidExpr(char *s){ liquid->setExpr(s);}

	char *getOceanSolidExpr()      { return solid->getExpr();}
	void setOceanSolidExpr(char *s){ solid->setExpr(s);}
	void valueString(char *s);

	static char *getDfltOceanExpr();
	static char *getDfltOceanSolidExpr();
	static char *getDfltOceanLiquidExpr();
	
	static Array<OceanState *>oceanTypes;
	static void setDefaults();
	void saveNode(FILE *);

	void print();
};

#endif
