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

#define MAX_BRANCHES 6
#define MAX_PLANT_DATA 7
#define PLANT_STATS 7

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
	  	//cout<<"leaf base dir="<<base_dir<<endl;
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
	  	//cout<<"branch base dir="<<base_dir<<endl;
	}
};
extern BranchImageMgr branch_mgr;
class PlantMgr : public PlacementMgr
{
protected:
public:
	Placement *make(Point4DL&,int);
	TNplant *plant;
	static int stats[PLANT_STATS];
	static double render_time;

	static int adapt_tests;

	static bool threed;
	static bool spline;
	static bool poly_lines;
	static bool shader_lines;
	static bool no_cache;
	static int textures;

	static void setColorTest(bool b) { BIT_SET(adapt_tests,TEST_COLOR,b);}
	static void setDensityTest(bool b) { BIT_SET(adapt_tests,TEST_DENSITY,b);}
	static bool testColor()  { return BIT_TST(adapt_tests,TEST_COLOR);}
	static bool testDensity()  { return BIT_TST(adapt_tests,TEST_DENSITY);}

	Color c;
	double slope_bias;
	double ht_bias;
	double lat_bias;
	static bool shadow_mode;
	static int shadow_count;
	static double pmax;
	static double pmin;

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
	static void clearStats();
	static void collectStats();
	static void showStats();
};


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
// Class TNLeaf
//************************************************************
class BranchData
{
public:
	Point4D data[6];
	Color c;
	BranchData(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d, Point4D s, Color col){
		data[0]=p0;data[1]=p1;data[2]=p2;data[3]=f;data[4]=d;data[5]=s;
		c=col;
	}
	double distance();
	double value() { return distance();}
	void render();

};


//************************************************************
// Class TNBranch
//************************************************************
class TNBranch : public TNbase, public ImageInfo
{
protected:
	enum flags{
		TEX=0x1,
		COL=0x2,
		SHAPE=0x4,
		SHADOW=0x8,
		ENABLES=TEX|COL|SHAPE|SHADOW,
		LEAFS=0x01,
		BRANCHES=0x02,
		LINE_MODE=0x00,
		RECT_MODE=0x01,
		LEAF_MODE=0x02,
		SPLINE_MODE=0x03,
		THREED_MODE=0x04,
		SHADER_MODE=LINE_MODE|RECT_MODE|LEAF_MODE|SPLINE_MODE|THREED_MODE,
		POLY_FILL=0x08,
		POLY_LINE=0x10,
		POLY_MODE=POLY_FILL|POLY_LINE

	};
public:
	Array<TNode *>arglist;
	static ValueList<BranchData*> branches;
	static int collect_mode;

	int maxlvl;
	int level;
	int min_level;
	int max_level;
	int enables;
	int branch_id;
	int max_splits;
	int first_bias;
	double length; // trunk and main level
	double width; // trunk and main level	
	double randomness;
	double divergence;
	double flatness;
	double length_taper;
	double width_taper;
	double offset;
	double bias;
	double curvature;
	double density;
	unsigned int texture_id;
	int texid;
	int color_flags;
	bool alpha_texture;

	TNplant *root;
	TNcolor *color;
	
	int instance;

	char colorexpr[256];
		
	TNBranch(TNode *l, TNode *r, TNode *b);

    void getArgs();

	virtual int typeValue()	  { return ID_BRANCH;}
	virtual char *typeName () { return "branch";}
	virtual char *symbol()	  { return "Branch";}

	static void collectBranches(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d,Point4D s,Color c);
    
	static void setCollectLeafs(bool b){BIT_SET(collect_mode,flags::LEAFS,b);}
	static void setCollectBranches(bool b){BIT_SET(collect_mode,flags::BRANCHES,b);}
	static bool isCollectLeafsSet(){return BIT_TST(collect_mode,flags::LEAFS);}
	static bool isCollectBranchesSet(){return BIT_TST(collect_mode,flags::BRANCHES);}
	static void freeBranches() {branches.free();}
	static void renderBranches();
	
	static int polyMode(int m) { return m&POLY_LINE?GL_LINE:GL_FILL;}
	static int shaderMode(int m) { return m&SHADER_MODE;}
	void setTexEnabled(bool b){BIT_SET(enables,flags::TEX,b);setColorFlags();}
	void setColEnabled(bool b){BIT_SET(enables,flags::COL,b);setColorFlags();}
	void setShapeEnabled(bool b){BIT_SET(enables,flags::SHAPE,b);setColorFlags();}
	void setShadowEnabled(bool b){BIT_SET(enables,flags::SHADOW,b);}
	bool isColEnabled() { return BIT_TST(enables,flags::COL);}
	bool isTexEnabled() { return BIT_TST(enables,flags::TEX);}
	bool isShapeEnabled() { return BIT_TST(enables,flags::SHAPE);}
	bool isShadowEnabled() { return BIT_TST(enables,flags::SHADOW);}
	static bool isShadowEnabled(int t) { return BIT_TST(t,flags::SHADOW);}
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
	void setColor(bool b);
	void setColor(TNcolor*);
	TNcolor* getColor();
	void setColorFlags();
	void invalidateTexture();
	void setPlantImage(char *);
	double evalArg(int id, double d);
	
	virtual void emit(int, Point b, Point v,Point l, double w, double t, int lvl);
	virtual void fork(int, Point b, Point v,Point l, double w, double t, int lvl);
	virtual Point setVector(Point vec, Point start, int lvl);
	TNplant *getRoot();
	Point spline(double t, Point p0, Point p1, Point p2);
	int getChildren(LinkedList<NodeIF*>&l);
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
	static bool sorted;
	static void renderLeafs();	
	static void collectLeafs(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d,Point4D s,Color c);

	static void freeLeafs() {leafs.free();}
	static void sortLeafs() {leafs.sort();}
	static ValueList<BranchData*> leafs;
	
	Point setVector(Point vec, Point start, int lvl);

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


