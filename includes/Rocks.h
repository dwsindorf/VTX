// Rocks.h

#ifndef _ROCKS
#define _ROCKS

#include "Placements.h"
#include "MCObjects.h"
#include <map>

class RockMgr;
class TNnode;

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
	// Cache key based on world position
	struct RockCacheKey {
		long long x, y, z;  // Quantized world position
		
		RockCacheKey(const Point& worldPos, double snap = 1e-10) {  // Changed from 0.01 to 0.001
		    x = (long long)round(worldPos.x / snap);
		    y = (long long)round(worldPos.y / snap);
		    z = (long long)round(worldPos.z / snap);
		}		
		bool operator<(const RockCacheKey& other) const {
			if (x != other.x) return x < other.x;
			if (y != other.y) return y < other.y;
			return z < other.z;
		}
	};
	
	struct RockCacheEntry {
	    std::vector<MCTriangle> mesh;  // Just store the mesh, not the whole object
	    std::string estr;
	    Point worldVertex;
	    int resolution;
	    int seed;
	    int framesSinceUsed;
	};    
    static std::map<RockCacheKey, RockCacheEntry> rockCache;
    static int cacheHits;
    static int cacheMisses;
    static int cacheRegens;
  
	static bool noiseSettingsChanged();
	static ValueList<PlaceData*> data;
    static MCObjectManager rocks;
    static std::map<int, MCObject*> lodTemplates;
    
    static bool vbo_valid;
    MCObject* getTemplateForLOD(int resolution, bool noisy, double noiseAmpl, int seed, TNode *tc, double comp);
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
	TNode   *vnoise;
	TNode   *rnoise;
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


