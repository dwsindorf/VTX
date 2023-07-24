// Sprites.h

#ifndef _SPRITES
#define _SPRITES

#include "Placements.h"

class SpriteMgr;
class TNsprite;

class SpritePoint : public Placement
{
public:
	double ht;
	double aveht;
	double wtsum;
	double dist;
	int hits;
	int visits;
	int sprites_dim;
	double variability;
	double rand_flip_prob;
	double select_bias;

	int instance;

	SpritePoint(SpriteMgr&, Point4DL&,int);
	bool set_terrain(PlacementMgr  &mgr);
	void dump();
	void reset();
};

class SpriteData
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

	int sprites_dim;
	int visits;
	Point4DL point;
	Point center;
	SpriteData(SpritePoint*,Point,double,double);
	double value() { return distance;}
	void print();
	int get_id()				{ return type&PID;}
	int get_class()				{ return type&PLACETYPE;}
	int flip()				    { return type & FLIP;}
};

class SpriteMgr : public PlacementMgr
{
protected:
public:
	Placement *make(Point4DL&,int);

	Color c;
	int sprites_dim;
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
	static bool setProgram();
	bool valid();
	void reset();
};

//************************************************************
// Class TNsprite
//************************************************************
class TNsprite : public TNplacements
{
protected:
	char sprites_file[256];

public:
	GLuint sprites_dim;

	int instance;
	Image *image;
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

	bool isLeaf()			{ return true;}
	int linkable()          { return 0;}
	void setName(char*);
	void valueString(char *);
	void save(FILE*);
	void setSpritesImage(char *name);
	void set_id(int i);
	int getFilePath(char*name,char *path);
	void getSpritesDir(int dim,char*dir);
	void getSpritesFilePath(char*name,int dim,char *dir);
	char *getSpritesFile(GLuint &dim);
	char *nodeName()  { return sprites_file;}
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
	GLuint sprites_dim;

	bool   valid;
	static ValueList<SpriteData*> sprites;

	Sprite(Image *i, int l, TNode *e);
	
	SpriteMgr *mgr() { return expr->mgr;}
	char *name() { return expr->nodeName();}
	void eval();
	void print();
	bool setProgram();
	bool initProgram();
	int get_id()				{ return sprite_id;}
	void set_id(int i)			{ sprite_id=i;}
	void set_image(Image *, int );
	static void reset();
	static void collect();
};
#endif


