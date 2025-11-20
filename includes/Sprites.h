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
	//void setTests();
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
};

class Sprite : public PlaceObj
{
public:
	Image *image;
	unsigned int texture_id;
	uint rows;
	uint cols;

	//static ValueList<PlaceData*> data;

	Sprite(Image *i, int l, TNode *e);
	
	SpriteMgr *mgr();// { return ((TNsprite*)expr)->mgr;}
	void eval();
	bool setProgram();
	bool initProgram();
	void set_image(Image *, int r, int c);
	static void reset();
	static void collect(Array<PlaceObj*> &data);
	static void eval(Array<PlaceObj*> &data);
	
	void collect();
};
#endif


