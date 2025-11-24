// Rocks.h

#ifndef _ROCKS
#define _ROCKS

#include "Placements.h"
#include "MarchingCubes.h"

class RockMgr;

class Rock : public Placement
{
 public:
	Rock(PlacementMgr&, Point4DL&,int);
	virtual bool set_terrain(PlacementMgr  &mgr);
};

class RockObjMgr : public PlaceObjMgr
{
	static ValueList<PlaceData*> data;
	void free() { data.free();}
	int placements(){ return data.size;}
	void collect();
	const char *name() { return "Rocks3D";}
};

class Rock3D : public PlaceObj
{
 public:
	Rock3D(int l, TNode *e);
    MarchingCubesObject* mcObject;  // Only allocated for 3D rocks
};

class RockMgr : public PlacementMgr
{
protected:
public:
    static TNode *default_noise;
	virtual Placement *make(Point4DL&,int);
	double  zcomp,drop,rdist,pdist;
	double  noise_ampl;
	double  rx,ry;
	TNode   *rnoise;
	~RockMgr();
	RockMgr(int);

	virtual void init();
};

class Rock3DMgr : public RockMgr
{
protected:
public:
	Rock3D *rock;
    //static TNode *default_noise;
	Rock3DMgr(int);
	Placement *make(Point4DL&,int);
	void eval();

	bool testColor();
	bool testDensity();

};


#endif


