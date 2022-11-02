// Rocks.h

#ifndef _ROCKS
#define _ROCKS

#include "Placements.h"

class RockMgr;

class Rock : public Placement
{
protected:
public:
	Rock(PlacementMgr&, Point4DL&,int);
	void set_terrain(PlacementMgr  &mgr);
};

class RockMgr : public PlacementMgr
{
protected:
public:
    static TNode *default_noise;
	Placement *make(Point4DL&,int);

	double  zcomp,drop;

	double  noise_radial;
	TNode   *rnoise;
	~RockMgr();
	RockMgr(int);

	void init();
};

//************************************************************
// Class TNrocks
//************************************************************
class TNrocks : public TNplacements
{
public:
	TNrocks(int t, TNode *l, TNode *r, TNode *b);
	void eval();
	void init();
	bool hasChild(int );
	void applyExpr();
	int typeValue()			{ return ID_ROCKS;}
	const char *typeName ()	{ return "rocks";}
	NodeIF *addChild(NodeIF *x);
	NodeIF *addAfter(NodeIF *c,NodeIF *n);
	NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	bool isLeaf()			{ return false;}
	int linkable()      		    { return 1;}
};

#endif


