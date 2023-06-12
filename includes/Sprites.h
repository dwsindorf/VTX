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
	int    type;
	double distance;
	double radius;
	double ht;
	double aveht;
	double pntsize;
	Point center;
	SpriteData(SpritePoint*,Point,double,double);
	double value() { return distance;}
	void print();
	int get_id()				{ return type&PID;}
	int get_class()				{ return type&PLACETYPE;}
};

class Sprite
{
public:
	Image *image;
	int    type;
	TNsprite  *expr;
	unsigned int texture_id;
	unsigned int sprite_id;
	bool   valid;
	static ValueList<SpriteData*> sprites;

	Sprite(Image *i, int l, TNode *e);
	
	void eval();
	void print();
	bool setProgram();
	bool initProgram();
	int get_id()				{ return sprite_id;}
	void set_id(int i)			{ sprite_id=i;}
	static void reset();
	static void collect();
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
	static bool setProgram();
};

//************************************************************
// Class TNsprite
//************************************************************
class TNsprite : public TNplacements
{
protected:
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
	void set_id(int i);
};

#endif


