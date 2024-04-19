// Plants.h

#ifndef _PLANTS
#define _PLANTS

#include "Placements.h"

#define TNBRANCH TNBranch

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
	int place_hits;
	int visits;
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
	int instance;

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
	double slope_bias;
	double ht_bias;
	double lat_bias;

	~PlantMgr();
	PlantMgr(int,TNplant*);

	void init();
	void eval();
	static bool setProgram();
	bool valid();
	void reset();
};

#define MAX_BRANCHES 6

//************************************************************
// Class TNplant
//************************************************************
class TNplant : public TNplacements
{
protected:
public:
	Point norm;
	int branches;
	int stats[MAX_BRANCHES][4];

	int plant_id;
	Plant *plant;
	TNbranch *branch;
	double radius;
	double size;
	double pntsize;
	double maxdensity;
	double size_scale;
	double norm_scale;
	double width_scale;
	static int textures;

	int max_levels;
	Point base_point;
	
	TNplant(TNode *l, TNode *r);
	~TNplant();
	void eval();
	void init();
	int typeValue()			{ return ID_PLANT;}
	const char *typeName ()	{ return "plant";}
	const char *symbol()	{ return "Plant";}

	bool isLeaf()			{ return true;}
	int linkable()          { return 1;}
	void valueString(char *);
	void save(FILE*);
	void set_id(int i);
	void saveNode(FILE *f);
	void applyExpr();
	bool setProgram();
	void emit();
	
	void clearStats();
	void showStats();
	void addBranch(int id);
	void addLine(int id);
	void addTerminal(int id);
	void addSkipped(int id);
};

//************************************************************
// Class TNBranch
//************************************************************
class TNBranch : public TNbase
{
public:
	int maxlvl;
	int max_plant_levels;
	int min_level;
	int max_level;
	int branch_id;
	double max_splits;
	double length; // trunk and main level
	double first_bias;
	double randomness;
	double divergence;
	double flatness;
	double length_taper;
	double width_taper;
	unsigned int texture_id;
	int texid;

	Image *image;
	TNplant *root;
	
	int instance;

	char texname[256];
	
	TNBranch(TNode *l, TNode *r, TNode *b);
	~TNBranch();
	
	int typeValue()			{ return ID_BRANCH;}
	const char *typeName ()	{ return "branch";}
	const char *symbol()	{ return "Branch";}
	
	void init();
	void eval();
	void valueString(char *);
	void save(FILE*);
	void saveNode(FILE *f);
	void getTexDir(char*dir);
	void getTexFilePath(char*name,char *dir);
	
	bool setProgram();
	
	void setTexture();
	void setColor();
	void invalidateTexture();
	void setImage(char *);
	
	virtual void emit(int, Point b, Point v,Point l, double w, double t, int lvl);
	virtual void fork(int, Point b, Point v,Point l, double w, double t, int lvl);
	
	TNplant *getRoot();

};

//************************************************************
// Class TNLeaf
//************************************************************
class TNLeaf : public TNBranch
{
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
	void clearStats();
	void showStats();
	int get_id()				{ return plant_id;}
	void set_id(int i)			{ plant_id=i;}
	static void reset();
	static void collect();
};
#endif


