// Rocks.h

#ifndef _ROCKS
#define _ROCKS

#include "Placements.h"
#include "MCObjects.h"
#include <map>

class RockMgr;

#define MAX_ROCK_STATS 8

class Rock : public Placement
{
 public:
	Rock(PlacementMgr&, Point4DL&,int);
	virtual bool set_terrain(PlacementMgr  &mgr);
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
class Rock3DObjMgr : public PlaceObjMgr
{
public:
	static ValueList<PlaceData*> data;
    static MCObjectManager rocks;
    static std::map<int, MCObject*> lodTemplates;
    
    static bool vbo_valid;
    MCObject* getTemplateForLOD(int resolution, bool noisy, double noiseAmpl);
    static void freeLODTemplates();

    ~Rock3DObjMgr();
    
	void free();
	int placements(){ return data.size;}
	void collect();
	const char *name() { return "Rocks3D";}
    bool setProgram();
    void render();
 
};

class Rock3D : public PlaceObj
{
 public:
	Rock3D(int l, TNode *e);
    PlacementMgr *mgr();
};

class Rock3DMgr : public PlacementMgr
{
protected:
public:
	static int stats[MAX_ROCK_STATS][2];
	static void clearStats();
	static void printStats();
	static void setStats(int,int);
	static int getLODResolution(double pts);

 	Rock3DMgr(int);
	Placement *make(Point4DL&,int);
	void eval();
	void init();

	bool testColor();
	bool testDensity();
};


#endif


