// Plants.h

#ifndef _PLANTS
#define _PLANTS

#include "Placements.h"
#include "defs.h"

class PlantMgr;
class TNplant;
class TNBranch;
class TNLeaf;
class Plant;
class BranchData;

#define MAX_BRANCHES 6
#define MAX_PLANT_DATA 7
#define MAX_PLANTS 7

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
	void setImageBaseDir(char *c){
		ImageMgr::setImageBaseDir(c);
	}
	void setImageBaseDir(){
		File.getBranchesDir(base_dir);
		getImageDirs();
	}
};
extern BranchImageMgr branch_mgr;
class PlantMgr : public PlacementMgr
{
protected:
public:
	Placement *make(Point4DL&,int);
	
	TNplant *plant;
	static int stats[MAX_PLANTS];

	static double render_time;
	static bool threed;
	static bool spline;
	static bool poly_lines;
	static bool shader_lines;
	static bool no_cache;
	static int textures;
	static bool shadow_mode;
	static int shadow_count;
	static bool show_one;
	static bool first_instance;
	
	Color c;
	double slope_bias;
	double ht_bias;
	double lat_bias;
	double hardness_bias;

	~PlantMgr();
	PlantMgr(int,TNplant*);
	PlantMgr(int);

	void init();
	void eval();

	static bool setProgram(Array<PlaceObj*> &objs);
	static void render(Array<PlaceObj*> &objs);
	static void render_zvals(Array<PlaceObj*> &objs);
	static void render_shadows(Array<PlaceObj*> &objs);
	static void clearStats();
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

	Plant *plant;
	double radius;
	double size;
	double pntsize;
	double distance;
	double maxdensity;
	double base_drop;
	double width_scale;
	double size_scale;
	double draw_scale;
	double seed;
	static double norm_max;
	static double norm_min;

	Point base_point;
	
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
	void setNormal();
	void emit();
	void getLeaf();
	void getLastBranch();
	void clearStats();
	void setScale();
	void addBranch();
	void addLine();
	void addRendered();
	void addLeaf();
	void addSpline();
	void addSkipped();
	int getChildren(LinkedList<NodeIF*>&l);
	bool randomize();
	NodeIF *removeNode();
	NodeIF *replaceNode(NodeIF *c);
	NodeIF *addChild(NodeIF *n);
	NodeIF *lastChild();
	NodeIF *getInstance();
	NodeIF *getInstance(NodeIF *prev);
	NodeIF *getInstance(NodeIF *prev, int m);
	
};
class Plant : public PlaceObj
{
public:
	static ValueList<PlaceData*> data;
	ValueList<BranchData*> branches;
	ValueList<BranchData*> leafs;

	void freeBranches() {branches.free();}
	void renderBranches();
	void collectBranches(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d,Point4D s,Color c);

	bool sorted;
	void renderLeafs();	
	void collectLeafs(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d,Point4D s,Color c);
	void freeLeafs() {leafs.free();sorted=false;}
	void sortLeafs() {if(!sorted)leafs.sort();sorted=true;}

	Plant(int l, TNode *e);
	
	PlacementMgr *mgr() { return ((TNplant*)expr)->mgr;}
	void eval();
	bool setProgram();
	bool initProgram();
	void clearStats();
	void showStats();
	
	static void reset();
	static void collect(Array<PlaceObj*> &data);
	static void eval(Array<PlaceObj*> &data);
	
	static void freeLeafs(Array<PlaceObj*> &data);
	static void freeBranches(Array<PlaceObj*> &data);
	static void renderBranches(Array<PlaceObj*> &data);
	static void renderLeafs(Array<PlaceObj*> &data);	
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
		TEX=0x01,
		COL=0x02,
		SHAPE=0x04,
		SHADOW=0x08,
		RANDOMIZE=0x10,
		ENABLES=TEX|COL|SHAPE|SHADOW|RANDOMIZE,
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
    
	static void setCollectLeafs(bool b){BIT_SET(collect_mode,flags::LEAFS,b);}
	static void setCollectBranches(bool b){BIT_SET(collect_mode,flags::BRANCHES,b);}
	static bool isCollectLeafsSet(){return BIT_TST(collect_mode,flags::LEAFS);}
	static bool isCollectBranchesSet(){return BIT_TST(collect_mode,flags::BRANCHES);}
	
	static int polyMode(int m) { return m&POLY_LINE?GL_LINE:GL_FILL;}
	static int shaderMode(int m) { return m&SHADER_MODE;}
	void setTexEnabled(bool b){BIT_SET(enables,flags::TEX,b);setColorFlags();}
	void setColEnabled(bool b){BIT_SET(enables,flags::COL,b);setColorFlags();}
	void setShapeEnabled(bool b){BIT_SET(enables,flags::SHAPE,b);setColorFlags();}
	void setShadowEnabled(bool b){BIT_SET(enables,flags::SHADOW,b);}
	void setRandEnabled(bool b){BIT_SET(enables,flags::RANDOMIZE,b);}
	bool isColEnabled() { return BIT_TST(enables,flags::COL);}
	bool isTexEnabled() { return BIT_TST(enables,flags::TEX);}
	bool isShapeEnabled() { return BIT_TST(enables,flags::SHAPE);}
	bool isShadowEnabled() { return BIT_TST(enables,flags::SHADOW);}
	bool isRandEnabled() { return BIT_TST(enables,flags::RANDOMIZE);}
	static bool isShadowEnabled(int t) { return BIT_TST(t,flags::SHADOW);}
	static bool isColEnabled(int t) { return BIT_TST(t,flags::COL);}
	static bool isTexEnabled(int t) { return BIT_TST(t,flags::TEX);}	
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
	void getTextureImage();
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
	void getRoot();
	Point spline(double t, Point p0, Point p1, Point p2);
	int getChildren(LinkedList<NodeIF*>&l);
	virtual bool randomize();
	virtual void propertyString(char *s);
	virtual bool canGenerate()  { return false;}
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
	
	Point setVector(Point vec, Point start, int lvl);

};

#endif


