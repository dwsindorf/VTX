// Plants.h

#ifndef _PLANTS
#define _PLANTS

#include "Placements.h"

#define TNBRANCH TNBranch

class PlantMgr;
class TNplant;
class TNBranch;
class TNLeaf;
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
#define MAX_PLANT_DATA 7

//************************************************************
// Class TNplant
//************************************************************
class TNplant : public TNplacements
{
protected:
public:
	Point norm;
	int branches;
	int stats[MAX_BRANCHES][MAX_PLANT_DATA];

	int plant_id;
	Plant *plant;
	TNBranch *last_branch;
	TNLeaf *leaf;
	double radius;
	double size;
	double pntsize;
	double maxdensity;
	double base_drop;
	double width_scale;
	static double norm_max;
	static double norm_min;
	static int textures;

	Point base_point;
	int rendered;
	
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
	void getLeaf();
	void getLastBranch();
	void clearStats();
	void showStats();
	void addBranch(int id);
	void addLine(int id);
	void addTerminal(int id);
	void addLeaf(int id);
	void addSpline(int id);
	void addSkipped(int id);
};

//************************************************************
// Class TNBranch
//************************************************************
class TNBranch : public TNbase
{
public:
	int maxlvl;
	int level;
	int min_level;
	int max_level;
	int branch_id;
	double max_splits;
	double length; // trunk and main level
	double width; // trunk and main level	
	double first_bias;
	double randomness;
	double divergence;
	double flatness;
	double length_taper;
	double width_taper;
	unsigned int texture_id;
	int texid;
	int color_flags;
	bool alpha_texture;

	Image *image;
	TNplant *root;
	
	int instance;

	char texname[256];
	
	TNBranch(TNode *l, TNode *r, TNode *b);
	
	virtual int typeValue()	  { return ID_BRANCH;}
	virtual char *typeName () { return "branch";}
	virtual char *symbol()	  { return "Branch";}
	
	virtual void init();
	void eval();
	void valueString(char *);
	void save(FILE*);
	void saveNode(FILE *f);
	virtual bool setProgram();
	
	bool isPlantLeaf()   { return typeValue()==ID_LEAF;}
	bool isPlantBranch() { return typeValue()==ID_BRANCH;}
	void setTexture();
	void setColor();
	void setColorFlags();
	void invalidateTexture();
	void setImage(char *);
	
	virtual void emit(int, Point b, Point v,Point l, double w, double t, int lvl);
	virtual void fork(int, Point b, Point v,Point l, double w, double t, int lvl);
	virtual Point setVector(Point vec, Point start, int lvl);
	TNplant *getRoot();
	Point spline(double t, Point p0, Point p1, Point p2);

};

//************************************************************
// Class TNLeaf
//************************************************************
class LeafData
{
public:
	Point data[4];
	Color c;
	LeafData(Point b,Point t, Point f, Point d, Color col){
		data[0]=b;data[1]=t;data[2]=f;data[3]=d;
		c=col;
	}
	double distance();
	double value() { return distance();}
	void render();
};

class TNLeaf : public TNBranch
{
public:
	
	TNLeaf(TNode *l, TNode *r, TNode *b);

	int typeValue()		{ return ID_LEAF;}
	char *typeName ()	{ return "leaf";}
	char *symbol()		{ return "Leaf";}
	static void render();	
	static void collect(Point b,Point t, Point f, Point d,Color c){
		leafs.add(new LeafData(b,t,f,d,c));
	}
	static void free() { leafs.free();}
	static void sort() { leafs.sort();}	
	static ValueList<LeafData*> leafs;

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


