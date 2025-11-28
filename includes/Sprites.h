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
	double rand_flip_prob;
	double select_bias;
	
	~SpriteMgr();
	SpriteMgr(int);

	void init();
	void eval();
	static bool setProgram(Array<PlaceObj*> &objs);

};
class SpriteObjMgr : public PlaceObjMgr
{
public:
	static ValueList<PlaceData*> data;
	bool setProgram();
	void free() { data.free();}
	int placements(){ return data.size;}
	void collect();
	const char *name() { return "Sprites";}
};

class Sprite : public PlaceObj
{
public:
	Image *image;
	unsigned int texture_id;
	uint rows;
	uint cols;
	Sprite(Image *i, int l, TNode *e);
	
	SpriteMgr *mgr();// { return ((TNsprite*)expr)->mgr;}
	bool setProgram();
	void set_image(Image *, int r, int c);
};
#endif


