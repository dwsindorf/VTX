// Plants.h

#ifndef _PLANTS
#define _PLANTS

#include "Placements.h"

class PlantMgr;
class TNplant;
class Plant;

class PlantPoint : public Placement
{
public:
	double ht;
	double aveht;
	double wtsum;
	double dist;
	int hits;
	int visits;
	int plants_dim;
	double variability;
	double rand_flip_prob;
	double select_bias;

	int instance;

	PlantPoint(PlantMgr&, Point4DL&,int);
	bool set_terrain(PlacementMgr  &mgr);
	void dump();
	void reset();
};

class PlantData
{
public:
	int    type;
	double distance;
	double radius;
	double ht;
	double aveht;
	double pntsize;
	double variability;
	double rand_flip_prob;
	double select_bias;
	int instance;

	int plants_dim;
	int visits;
	Point4DL point;
	Point center;
	Point base;
	PlantData(PlantPoint*,Point, Point,double,double);
	double value() { return distance;}
	void print();
	int get_id()				{ return type&PID;}
	int get_class()				{ return type&PLACETYPE;}
	int flip()				    { return type & FLIP;}
};

class PlantMgr : public PlacementMgr
{
protected:
public:
	Placement *make(Point4DL&,int);

	Color c;
	int plants_dim;
	double variability;
	double slope_bias;
	double ht_bias;
	double lat_bias;
	double rand_flip_prob;
	double select_bias;
	
	~PlantMgr();
	PlantMgr(int);

	void init();
	void eval();
	static bool setProgram();
	bool valid();
	void reset();
};

//************************************************************
// Class TNplant
//************************************************************
class TNplant : public TNplacements
{
protected:
	char plants_file[256];

public:

	int instance;
	Plant *plant;
	double radius;
	double maxdensity;
	TNplant(char *s, TNode *l, TNode *r);
	~TNplant();
	void eval();
	void init();
	int typeValue()			{ return ID_PLANT;}
	const char *typeName ()	{ return "plant";}
	const char *symbol()	{ return "Plant";}

	bool isLeaf()			{ return true;}
	int linkable()          { return 0;}
	void setName(char*);
	void valueString(char *);
	void save(FILE*);
	void set_id(int i);
	char *nodeName()  { return plants_file;}
	void saveNode(FILE *f);
	void applyExpr();	   
};

class Plant
{
public:
	int    type;
	TNplant  *expr;
	unsigned int plant_id;

	bool   valid;
	static ValueList<PlantData*> plants;

	Plant(int l, TNode *e);
	
	PlantMgr *mgr() { return expr->mgr;}
	char *name() { return expr->nodeName();}
	void eval();
	void print();
	bool setProgram();
	bool initProgram();
	int get_id()				{ return plant_id;}
	void set_id(int i)			{ plant_id=i;}
	static void reset();
	static void collect();
};
#endif


