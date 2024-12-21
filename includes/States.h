#ifndef _ID_STATES_CLASS
#define _ID_STATES_CLASS

#include "TerrainMgr.h"

#define NUM_OCEAN_TYPES 5
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
	double trans_temp;
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

	LiquidState *liquid;
	SolidState *solid;
	TNode  *ocean_expr;
	char expr[512];
	
	enum Types {H2O,SO2,CO2,CH4,N2};

	double evalOceanFunction();

	void setWaterColor1(Color c) 	{liquid->color1=c;}
	void setWaterColor2(Color c) 	{liquid->color2=c;}
	void setIceColor1(Color c)   	{solid->color1=c;}
	void setIceColor2(Color c)   	{solid->color2=c;}
	Color waterColor1()          	{return liquid->color1;}
	Color waterColor2()          	{return liquid->color2;}
	Color iceColor1()			 	{return solid->color1;}
	Color iceColor2()			 	{return solid->color2;}

	double waterClarity()		 	{return liquid->clarity*FEET;}
	double waterMix()			 	{return liquid->mix;}
	double waterSpecular()       	{return liquid->specular;}
	double waterShine()			 	{return liquid->shine;}
	void setWaterClarity(double f)  {liquid->clarity=f;}
	void setWaterMix(double f)      {liquid->mix=f;}
	void setWaterSpecular(double f) {liquid->specular=f;}
	void setWaterShine(double f) 	{liquid->shine=f;}

	double iceClarity()				{return solid->clarity*FEET;}
	double iceMix()					{return solid->mix;}
	double iceSpecular()			{return solid->specular;}
	double iceShine()				{return solid->shine;}
	void setIceClarity(double f)	{solid->clarity=f;}
	void setIceMix(double f)		{solid->mix=f;}
	void setIceSpecular(double f)	{solid->specular=f;}
	void setIceShine(double f)		{solid->shine=f;}
	
	double oceanSolidTemp()        { return solid->temp;}
	double oceanGasTemp()           { return liquid->temp;}
	void setOceanSolidTemp(double f) { solid->temp=f;}
	void setOceanGasTemp(double f)   { liquid->temp=f;}
	
	double oceanSolidTransTemp()    { return solid->trans_temp;}
	double oceanGasTransTemp()      { return liquid->trans_temp;}
	void setOceanSolidTransTemp(double f) { solid->trans_temp=f;}
	void setOceanGasTransTemp(double f)   { liquid->trans_temp=f;}
	
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
	
	static NodeIF *newInstance();
};

#endif
