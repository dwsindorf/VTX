// Sprites.h

#ifndef _SPRITES
#define _SPRITES

#include "Placements.h"

class SpriteMgr;

class Sprite : public Placement
{
public:
	double ht;
	double dist;
	int visits;
<<<<<<< HEAD
	int hits;
=======
>>>>>>> master

	Sprite(PlacementMgr&, Point4DL&,int);
	bool set_terrain(PlacementMgr  &mgr);
	void dump();
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
	Texture *texture;
	TNsprite(char *, TNode *l, TNode *r);
<<<<<<< HEAD
	~TNsprite();
=======
>>>>>>> master
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


