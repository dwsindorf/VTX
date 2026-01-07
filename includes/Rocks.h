// Rocks.h

#ifndef _ROCKS
#define _ROCKS

#include "Placements.h"
#include "MCObjects.h"
#include <map>

class RockMgr;
class TNnode;
class TNtexture;
class Texture;
class Rock3DData;
class Rock3DMgr;

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
	double  rdist,pdist;
	double  rx,ry;
	TNode   *rnoise;
	TNode   *vnoise;

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
		int instance;
		
		RockCacheKey(const Point& worldPos, int inst, double snap = 1e-10) {
		        x = (int64_t)round(worldPos.x / snap);
		        y = (int64_t)round(worldPos.y / snap);
		        z = (int64_t)round(worldPos.z / snap);
		        instance = inst;  // STORE INSTANCE
		    }
		bool operator<(const RockCacheKey& other) const {
		        if (instance != other.instance) return instance < other.instance;  // CHECK INSTANCE FIRST
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
	    int instance;
	    int framesSinceUsed;
	};    
    static std::map<RockCacheKey, RockCacheEntry> rockCache;
    static int cacheHits;
    static int cacheMisses;
    static int cacheRegens;
    static int maxTexs;
  
	static ValueList<PlaceData*> data;
    static MCObjectManager rocks;  

    ~Rock3DObjMgr();
    
	void free();
	int placements(){ return data.size;}
	void collect();
	const char *name() { return "Rocks3D";}
    bool setProgram();
    void render();
	void render_zvals();
	void render_shadows();
    void render_objects();
	
    bool supports_shadows() { return true;}
 
};

class Rock3DData : public PlaceData
{
public:
	Rock3DData(Placement *s) : PlaceData(s){}
    double value() { return instance;}
};

class Rock3D : public PlaceObj
{
 public:
	GLuint   texid;
	Rock3D(int l, TNode *e);
    PlacementMgr *mgr();
    bool setProgram();
    bool initProgram();
    void invalidateTexture();
};

class Rock3DMgr : public PlacementMgr
{
protected:
public:
	ValueList<TNtexture *> texs;
	TNode   *vnoise;
	TNode   *rnoise;
	TNode   *color;
	static int stats[MAX_ROCK_STATS][2];
	static void clearStats();
	static void printStats();
	static void setStats(int,int,bool);
	static int getLODResolution(double pts);
	static double resScale;

 	Rock3DMgr(int);
	Placement *make(Point4DL&,int);
	PlaceData *make(Placement*s);
	void eval();
	void init();

	bool testColor();
	bool testDensity();
};


#endif


