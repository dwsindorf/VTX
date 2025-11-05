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

class Rock3D : public Rock
{
 public:
    MarchingCubesObject* mcObject;  // Only allocated for 3D rocks
	Rock3D(PlacementMgr&, Point4DL&,int);
	bool set_terrain(PlacementMgr  &mgr);
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
    //static TNode *default_noise;
	Rock3DMgr(int);
	Placement *make(Point4DL&,int);
	void eval();

	bool testColor();
	bool testDensity();

};

//************************************************************
// Class TNrocks
//************************************************************
class TNrocks : public TNplacements
{
public:
	int rock_id;
	TNrocks(int t, TNode *l, TNode *r, TNode *b);
	void eval();
	void eval3d();
	void init();
	bool hasChild(int );
	void applyExpr();
	int typeValue()			{ return ID_ROCKS;}
	const char *typeName ()	{ return "rocks";}
	NodeIF *addChild(NodeIF *x);
	NodeIF *addAfter(NodeIF *c,NodeIF *n);
	NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	NodeIF *replaceNode(NodeIF *c);
	bool isLeaf()			{ return false;}
	int linkable()      		    { return 1;}
	NodeIF *getInstance(NodeIF *prev, int m);
	TNrocks *newInstance(int m);
	bool randomize();
};

#endif


