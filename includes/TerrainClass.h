// TerrainClass.h

#ifndef _TERRAIN_CLASS
#define _TERRAIN_CLASS

#include "TerrainMgr.h"

class ExprMgr;
class TerrainSym;
class TNode;
class TerrainData;
class TNarg;
class Image;
class Texture;
class Object3D;
class OceanState;
class PlacementMgr;

// TNpoint codes

//~~~~~~~~~~~~~~~~~ terminal nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//************************************************************
// Class TNconst
//************************************************************
class TNconst : public TNode
{
public:
	double value;
	TNconst(double c) {value=c;}
	void dump()				{ printf("const: %g\n",value);}
	void eval();
	int typeValue()			{ return ID_CONST;}
	const char *typeName ()	{ return "constant";}
	void valueString(char *s);
	void save(FILE *f);
};

//************************************************************
// Class TNstring
//************************************************************
class TNstring : public TNode
{
public:
	char value[512];
	TNstring(char *c) 		{ strcpy(value,c);}
	~TNstring();
	void dump()				{ printf("string: %s\n",value);}
	void eval();
	int typeValue()			{ return ID_STRING;}
	const char *typeName ()	{ return "string";}
	void valueString(char *s);
	void save(FILE *f);
};

//************************************************************
// Class TNglobal
//************************************************************
class TNglobal : public TNode
{
	int gtype;
public:
	TNglobal(int c) {gtype=c;}
	void init();
	void eval();
	void save(FILE *f);
	int typeValue()			{ return ID_GLOBAL;}
	const char *typeName ()	{ return "global";}
	void valueString(char *s);
};

//~~~~~~~~~~~~~~~~~ unary nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//************************************************************
// Class TNpoint
//************************************************************
class TNpoint : public TNvector
{
public:
	TNnoise *snoise;
	TNpoint(TNode *r) : TNvector(r) {}
	void dump()				{ printf("%s()\n",symbol());}
	void eval();
	int typeValue()			{ return ID_POINT;}
	const char *typeName()	{ return "point";}
	const char *symbol();
	void valueString(char *);
	void addToken(LinkedList<TNode*>&);
	bool initProgram();
	bool setProgram();
	TNode *operator[](int i);
};

//************************************************************
// Class TNzbase
//************************************************************
class TNzbase : public TNvector
{
public:
	TNzbase(TNode *r) :  TNvector(r) {}
	const char *typeName()	{ return "base";}
	int typeValue()			{ return ID_ZBASE;}
	void eval();
};

//************************************************************
// Class TNgloss
//************************************************************
class TNgloss : public TNvector
{
public:
	TNgloss(TNode *r) : TNvector(r) {}
	void eval();
	int typeValue()			{ return ID_GLOSS;}
	const char *typeName ()	{ return "gloss";}
};

//************************************************************
// Class TNcolor
//************************************************************
class TNcolor : public TNvector
{
public:
	static double rand_val;
	TNcolor(TNode *r) : TNvector(r) {}
	void dump()				{ printf("%s()\n",symbol());}
	void eval();
	void init();
	int typeValue()			{ return ID_COLOR;}
	const char *typeName ()	{ return "color";}
	const char *symbol();
	bool initProgram();
	int comps();
	bool randomize();

	void addToken(LinkedList<TNode*>&);
	TNode *operator[](int i);
};

//************************************************************
// Class TNdensity
//************************************************************
class TNdensity : public TNvector
{
public:
	TNdensity(TNode *r) : TNvector(r) {}
	void eval();
	int typeValue()			{ return ID_DENSITY;}
	const char *typeName ()	{ return "Density";}
	const char *symbol()	{ return "Density";}
	void init();
	bool initProgram();
};

//************************************************************
// Class TNclist
//************************************************************
class TNclist : public TNvector
{
public:
    Array<Color *>colors;
	TNclist(TNode *r): TNvector(r) {}
	~TNclist();
	int typeValue()			{ return ID_CLIST;}
	const char *typeName ()	{ return "colors";}
	const char *symbol()	{ return "Colors";}
	int size()				{ return colors.size;}
	void init();
	void save(FILE *f);
	void set_color(int i,Color c);
	Color get_color(int i);
	Color color(double f);
};


//************************************************************
// Class TNnoise
//************************************************************
class TNnoise : public TNvector
{
protected:
    enum {
        TNN_NORM        = 1,
        TNN_NORMALIZED  = 2
    };
	void optionString(char *);
	void getOpts(char *);
	void getNtype(char *);
	int flags;
public:
	double scale;
	double rate;

	double mn,mx,ma,mb;
    int type;
    int id;
    TNnoise(int t, TNode *m);
	void save(FILE*);
	void eval();
	void init();
	int norm() 			        { return flags & TNN_NORM;}
	int normalized()            { return flags & TNN_NORMALIZED;}
	void set_normalized()       { BIT_ON(flags,TNN_NORMALIZED);}
	void clr_normalized()       { BIT_OFF(flags,TNN_NORMALIZED);}

	int typeValue()			    { return ID_NOISE;}
	const char *typeName ()		{ return "noise";}
	void valueString(char *);
	void propertyString(char *);
	bool setProgram();
	void addToken(LinkedList<TNode*>&);
	static std::string randomize(char *src,double f,double t);
};

//~~~~~~~~~~~~~~~~~ binary nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//************************************************************
// Class TNwater
//************************************************************
class TNwater : public TNfunc
{
public:
	TNwater(TNode *l, TNode *r) : TNfunc(l,r) {}
	
	void eval();
	void save(FILE *);
	void saveNode(FILE *);
	NodeIF *replaceNode(NodeIF *c);
	int typeValue()			{ return ID_WATER;}
	const char *typeName ()	{ return "ocean";}
	NodeIF *getInstance();
	void getNoiseExprs(OceanState *s);
	void setNoiseExprs(OceanState *s);
	bool randomize();
	
};

//************************************************************
// Class TNfog
//************************************************************
class TNfog : public TNfunc
{
public:
	TNfog(TNode *l, TNode *r) : TNfunc(l,r) {}
	void eval();
	void saveNode(FILE *);
	NodeIF *replaceNode(NodeIF *c);
	int typeValue()			{ return ID_FOG;}
	const char *typeName ()	{ return "fog";}
};


//************************************************************
// Class TNsnow
//************************************************************
class TNsnow : public TNfunc
{
public:
	Texture *texture;
	Image *image;
	TNsnow(TNode *l, TNode *r);
	~TNsnow();
	void init();
	void eval();
	bool initProgram();
	int typeValue()			{ return ID_SNOW;}
	const char *typeName ()	{ return "snow";}
};


//************************************************************
// Class TNclouds
//************************************************************
class TNclouds :  public TNfunc
{
public:
	int opts;
	TNclouds(TNode *l, TNode *r) : TNfunc(l,r) {opts=0;}
	TNclouds(int i, TNode *l, TNode *r) : TNfunc(l,r) {opts=i;}
	void eval();
	void init();
	int typeValue()			{ return ID_CLOUD;}
	const char *typeName ()	{ return "clouds";}
	bool initProgram();
	bool setProgram();
	bool threeD();
};


//************************************************************
// Class TNplacements
//************************************************************
class TNplacements : public TNbase
{
protected:
public:
    PlacementMgr *mgr;
	TNplacements(int t, TNode *l, TNode *r, TNode *b);
	virtual ~TNplacements();
	
	virtual void reset();
	virtual void eval();
	virtual void init();
	virtual void save(FILE *);
	virtual int typeValue()			{ return ID_PLACED;}
	virtual void setProperties(NodeIF *n);
	virtual void valueString(char *);
	virtual int optionString(char *);
	virtual void addToken(LinkedList<TNode*>&l);
	virtual void set_id(int i);
	virtual int get_id();
	void set_flip(int i);
	bool is3D();
	void set3D(bool b);
	bool inLayer();

};

//************************************************************
// Class TNsprite
//************************************************************
class TNsprite : public TNplacements, public ImageInfo
{
public:
	Sprite *sprite;
	double radius;
	double maxdensity;
	TNsprite(char *, int opts,  TNode *l, TNode *r);
	~TNsprite();
	void eval();
	void init();
	int typeValue()			{ return ID_SPRITE;}
	const char *typeName ()	{ return "sprite";}
	const char *symbol()	{ return "Sprite";}

	void setName(char*);
	void valueString(char *);
	void save(FILE*);
	void setSpritesImage(char *name);
	void set_id(int i);
	char *nodeName();
	int optionString(char *);
	void saveNode(FILE *f);
	void applyExpr();
};

//************************************************************
// Class TNrocks
//************************************************************
class TNrocks : public TNplacements
{
public:
	TNrocks(int t, TNode *l, TNode *r, TNode *b);
	virtual void eval();
	virtual void init();
	virtual int typeValue()			{ return ID_ROCKS;}
	virtual const char *typeName ()	{ return "rocks";}
	virtual NodeIF *getInstance(NodeIF *prev, int m);
	virtual TNrocks *newInstance(int m);
	virtual bool randomize();
	bool hasChild(int );
	void applyExpr();
	NodeIF *addChild(NodeIF *x);
	NodeIF *addAfter(NodeIF *c,NodeIF *n);
	NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	NodeIF *replaceNode(NodeIF *c);
	bool isLeaf()			{ return false;}
	int linkable()          { return 1;}
};

class TNrocks3D : public TNplacements
{
public:
	Rock3D *rock;
	TNrocks3D(TNode *l, TNode *r, TNode *b);
	void eval();
	void init();
	int typeValue()			{ return ID_ROCK3D;}
	const char *typeName ()	{ return "rocks3d";}
	//const char *symbol()	{ return "Rock3D";}
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
	double radius;

	Plant *plant;
	double size;
	double pntsize;
	double distance;
	double width_scale;
	double size_scale;
	double seed;
	static double norm_max;
	static double norm_min;
	static double draw_scale;

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
	void setColor();
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

//************************************************************
// Class TNcraters
//************************************************************
class TNcraters : public TNplacements
{
protected:
	int join(TNode *);
	int joined(TNode *);
	TNode *next();
public:
	TNcraters(int t, TNode *l, TNode *r, TNode *b);
	~TNcraters();
	void eval();
	void init();
	void applyExpr();	   
	int typeValue()			{ return ID_CRATERS;}
	const char *typeName()	{ return "craters";}
	void addToken(LinkedList<TNode*>&l);
	void valueString(char *);
};

#endif
