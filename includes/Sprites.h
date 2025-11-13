// Sprites.h

#ifndef _SPRITES
#define _SPRITES

#include "Placements.h"
#include "FileUtil.h"

class SpriteMgr;
class TNsprite;

class SpritePoint : public Placement
{
public:
	int sprites_rows;
	int sprites_cols;

	double variability;
	double rand_flip_prob;
	double select_bias;

	SpritePoint(SpriteMgr&, Point4DL&,int);
};

class SpriteData  : public PlaceData
{
public:
	int sprites_rows;
	int sprites_cols;
	
	double variability;
	double rand_flip_prob;
	double select_bias;

	SpriteData(SpritePoint*s);
	int flip(){ return type & FLIP;}

};

class SpriteImageMgr : public ImageMgr
{
public:
	SpriteImageMgr() : ImageMgr(){
		setImageBaseDir();
		getImageDirs();
	};
	void setImageBaseDir();
};
extern SpriteImageMgr sprites_mgr;

class SpriteMgr : public PlacementMgr
{
protected:
public:
	Placement *make(Point4DL&,int);
	PlaceData *make(Placement*s);

	Color c;

	int sprites_rows;
	int sprites_cols;
	double variability;
	double slope_bias;
	double ht_bias;
	double lat_bias;
	double rand_flip_prob;
	double select_bias;
	
	~SpriteMgr();
	SpriteMgr(int);

	void init();
	void eval();
	void setTests();
	static bool setProgram();
};

//************************************************************
// Class TNsprite
//************************************************************
class TNsprite : public TNplacements, public ImageInfo
{
public:
	int instance;

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

	//bool isLeaf()			{ return true;}
	//int linkable()          { return 0;}
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

class Sprite
{
public:
	Image *image;
	int    type;
	TNsprite  *expr;
	unsigned int texture_id;
	unsigned int sprite_id;
	uint rows;
	uint cols;

	bool   valid;
	static ValueList<PlaceData*> data;

	Sprite(Image *i, int l, TNode *e);
	
	SpriteMgr *mgr() { return expr->mgr;}
	char *name() { return expr->nodeName();}
	void eval();
	void print();
	bool setProgram();
	bool initProgram();
	int get_id()				{ return sprite_id;}
	void set_id(int i)			{ sprite_id=i;}
	void set_image(Image *, int r, int c);
	static void reset();
	static void collect();
};
#endif


