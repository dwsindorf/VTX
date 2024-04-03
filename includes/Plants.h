// Plants.h

#ifndef _PLANTS
#define _PLANTS

#include "Placements.h"

class PlantMgr;
class TNplant;
class TNbranch;
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
	PlantMgr *mgr;

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
	Point base;
	PlantMgr *mgr;
	
	PlantData(PlantPoint*,Point, double,double);
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
	TNplant *plant;

	Color c;
	int plants_dim;
	double variability;
	double slope_bias;
	double ht_bias;
	double lat_bias;
	double rand_flip_prob;
	double select_bias;
	
	~PlantMgr();
	PlantMgr(int,TNplant*);

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
public:
	Point norm;

	int instance;
	Plant *plant;
	TNbranch *branch;
	double radius;
	double size;
	double pntsize;
	double maxdensity;
	int branch_levels;
	Point base;
	
	TNplant(TNode *l, TNode *r);
	~TNplant();
	void eval();
	void init();
	int typeValue()			{ return ID_PLANT;}
	const char *typeName ()	{ return "plant";}
	const char *symbol()	{ return "Plant";}

	bool isLeaf()			{ return true;}
	int linkable()          { return 0;}
	//void setName(char*);
	void valueString(char *);
	void save(FILE*);
	void set_id(int i);
	//char *nodeName()  { return plants_file;}
	void saveNode(FILE *f);
	void applyExpr();
	bool setProgram();
};

//************************************************************
// Class TNstem
//************************************************************
class TNBranch : public TNbase
{
	int level;
public:
	int max_splits;
	double length; // trunk and main branches
	double first_bias;
	double randomness;
	double sameness;
	double flatness;
	double length_taper;
	double width_taper;
	
	TNBranch(TNode *l, TNode *r, TNode *b);
	
	int typeValue()			{ return ID_BRANCH;}
	const char *typeName ()	{ return "branch";}
	const char *symbol()	{ return "Branch";}
	
	void init();
	void valueString(char *);
	void save(FILE*);
	void saveNode(FILE *f);
	
	virtual void emit(Point b, Point v,Point l, double w, double t, int lvl);
	
	TNplant *getRoot();

};

//************************************************************
// Class TNLeaf
//************************************************************
class TNLeaf : public TNBranch
{
};
//************************************************************
// Class TNbranch
//************************************************************
class TNbranch : public TNbase
{
protected:
public:
	enum type {
		TRUNK,
		BRANCH,
		FIRST_BRANCH
	};
	void emit(type,Point b, Point v,Point l, double w, double t, double r, int lvl);
	int maxlevels;
	double split_probability;
	double branch_probability;
	double first_bias;
	double branch_flatness;
	
	double max_splits;
	double max_branch_splits;
	
	Color color;

	double length; // trunk and main branches
	double randomness;
	double width_taper;
	double length_taper; 
	
	double branch_length; // branches
	double branch_randomness;
	double branch_width_taper;
	double branch_length_taper; // small branches
	
	TNbranch(TNode *l, TNode *r, TNode *b);
	
	int typeValue()			{ return ID_BRANCH;}
	const char *typeName ()	{ return "branch";}
	const char *symbol()	{ return "Branch";}
	
	void eval();
	void init();
	void valueString(char *);
	void save(FILE*);
	void saveNode(FILE *f);
	void applyExpr();
	bool setProgram();
	TNplant *getRoot();
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


