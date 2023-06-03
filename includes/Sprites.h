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
	double dist;
	int hits;
	int visits;

	SpritePoint(PlacementMgr&, Point4DL&,int);
	bool set_terrain(PlacementMgr  &mgr);
	void dump();
	void reset();

};

class SpriteData
{
public:
	double distance;
	double radius;
	double ht;
	double aveht;
	Point center;
	SpriteData(SpritePoint*);
	double value() { return distance;}
	void print();
};

class Sprite
{
	ValueList<SpriteData*> sprites;
public:
	Image *image;
	int    options;
	TNsprite  *expr;

	Sprite(Image *i, int l, TNode *e);
	void eval();
	void collect();
	void print();

};
class SpriteMgr : public PlacementMgr
{
protected:
public:
	Placement *make(Point4DL&,int);

	Color c;

	~SpriteMgr();
	SpriteMgr(int);

	void init();
	void eval();
};

//************************************************************
// Class TNsprite
//************************************************************
class TNsprite : public TNplacements
{
protected:
	GLuint sprites_image;
	char sprites_file[256];
	char sprites_type[256];
	GLuint sprites_dim;

public:
	Image *image;
	Sprite *sprite;
	TNsprite(char *, TNode *l, TNode *r);
	~TNsprite();
	void eval();
	void init();
	int typeValue()			{ return ID_SPRITE;}
	const char *typeName ()	{ return "sprite";}
	const char *symbol()	{ return "Sprite";}

	bool isLeaf()			{ return false;}
	int linkable()          { return 1;}
	void setName(char*);
	void setType(char*);
	void valueString(char *);
	void save(FILE*);
	void setSpritesTexture();
	void getSpritesFilePath(char *dir);
};

#endif


