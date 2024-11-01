// Plants.h

#ifndef _PLANTS
#define _PLANTS

#include "Placements.h"
#include "defs.h"

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


class LeafImageMgr : public ImageMgr
{
public:
	LeafImageMgr() : ImageMgr(){
		setImageBaseDir();
		getImageDirs();
	}
	void setImageBaseDir(){
		char base[512];
		File.getBaseDirectory(base);
	 	sprintf(base_dir,"%s/Textures/Plants/Leaf",base);
	  	cout<<"leaf base dir="<<base_dir<<endl;
	}
};
extern LeafImageMgr leaf_mgr;

class BranchImageMgr : public ImageMgr
{
public:
	BranchImageMgr() : ImageMgr(){
		setImageBaseDir();
		getImageDirs();
	}
	void setImageBaseDir(){
		char base[512];
		File.getBaseDirectory(base);
	 	sprintf(base_dir,"%s/Textures/Plants/Branch",base);
	  	cout<<"branch base dir="<<base_dir<<endl;
	}
};
extern BranchImageMgr branch_mgr;
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
	static bool shadow_mode;

	~PlantMgr();
	PlantMgr(int,TNplant*);

	void init();
	void eval();
	static bool setProgram();
	bool valid();
	void reset();
	static void render();
	static void render_zvals();
	static void render_shadows();
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
	int instance;
	Plant *plant;
	double radius;
	double size;
	double pntsize;
	double distance;
	double maxdensity;
	double base_drop;
	double width_scale;
	double size_scale;
	double seed;
	static double norm_max;
	static double norm_min;
	static int textures;
	static bool threed;
	static bool spline;

	Point base_point;
	int rendered;
	
	TNplant(TNode *l, TNode *r);
	~TNplant();
	void eval();
	void init();
	int typeValue()			{ return ID_PLANT;}
	const char *typeName ()	{ return "plant";}
	const char *symbol()	{ return "Plant";}

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
	int getChildren(LinkedList<NodeIF*>&l);
	NodeIF *removeNode();
	NodeIF *replaceNode(NodeIF *c);
	NodeIF *addChild(NodeIF *n);
	NodeIF *lastChild();
};

//************************************************************
// Class TNBranch
//************************************************************
class TNBranch : public TNbase, public ImageInfo
{
protected:
	enum flags{
		TEX=1,
		COL=2,
		SHAPE=4,
		SHADOW=8,
		ALL=TEX|COL|SHAPE|SHADOW
	};

public:

	int maxlvl;
	int level;
	int min_level;
	int max_level;
	int enables;
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
	double offset;
	double bias;
	unsigned int texture_id;
	int texid;
	int color_flags;
	bool alpha_texture;

	//Image *image;
	TNplant *root;
	TNcolor *color;
	
	int instance;

	//char texname[256];
	char colorexpr[256];
		
	TNBranch(TNode *l, TNode *r, TNode *b);
	
	virtual int typeValue()	  { return ID_BRANCH;}
	virtual char *typeName () { return "branch";}
	virtual char *symbol()	  { return "Branch";}
	
	void setTexEnabled(bool b){BIT_SET(enables,flags::TEX,b);setColorFlags();}
	void setColEnabled(bool b){BIT_SET(enables,flags::COL,b);setColorFlags();}
	void setShapeEnabled(bool b){BIT_SET(enables,flags::SHAPE,b);setColorFlags();}
	void setShadowEnabled(bool b){BIT_SET(enables,flags::SHADOW,b);}
	bool isColEnabled() { return BIT_TST(enables,flags::COL);}
	bool isTexEnabled() { return BIT_TST(enables,flags::TEX);}
	bool isShapeEnabled() { return BIT_TST(enables,flags::SHAPE);}
	bool isShadowEnabled() { return BIT_TST(enables,flags::SHADOW);}
	bool colValid();
	bool texValid();
	void init();
	virtual void initArgs();
	void eval();
	void valueString(char *);
	void save(FILE*);
	void saveNode(FILE *f);
	virtual NodeIF *removeNode();
	virtual NodeIF *replaceNode(NodeIF *c);
	virtual bool setProgram();
	
	bool isPlantLeaf()   { return typeValue()==ID_LEAF;}
	bool isPlantBranch() { return typeValue()==ID_BRANCH;}
	void getTextureName();
	void getColorString();
	void setColorExpr(char *s);
	char *getColorExpr(){return colorexpr;}
	void setColorFromExpr();
	void setColor();
	void setColor(TNcolor*);
	TNcolor* getColor();
	void setColorFlags();
	void invalidateTexture();
	void setPlantImage(char *);
	
	virtual void emit(int, Point b, Point v,Point l, double w, double t, int lvl);
	virtual void fork(int, Point b, Point v,Point l, double w, double t, int lvl);
	virtual Point setVector(Point vec, Point start, int lvl);
	TNplant *getRoot();
	Point spline(double t, Point p0, Point p1, Point p2);
	int getChildren(LinkedList<NodeIF*>&l);

};

//************************************************************
// Class TNLeaf
//************************************************************
class LeafData
{
public:
	Point4D data[6];
	Color c;
	LeafData(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d, Point4D s, Color col){
		data[0]=p0;data[1]=p1;data[2]=p2;data[3]=f;data[4]=d;data[5]=s;
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
	
	static int left_side;
	double phase;

	int typeValue()		{ return ID_LEAF;}
	char *typeName ()	{ return "leaf";}
	char *symbol()		{ return "Leaf";}
	static bool collect_mode;
	static void render();	
	static void collect(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d,Point4D s,Color c){
		leafs.add(new LeafData(p0,p1,p2,f,d,s,c));
	}
	static void free() { leafs.free();}
	static void sort() { leafs.sort();}	
	static ValueList<LeafData*> leafs;
	
	//void getPlantImageDir(int dim,char *);
	Point setVector(Point vec, Point start, int lvl);
	//char *getPlantImageName();

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


