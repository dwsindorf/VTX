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
	struct BatchKey {
		    int resolution;
		    int instanceId;
		    
		    BatchKey(int r, int i) : resolution(r), instanceId(i) {}
		    
		    bool operator<(const BatchKey& other) const {
		        if (resolution != other.resolution) return resolution < other.resolution;
		        return instanceId < other.instanceId;
		    }
		};
	struct VBOBatch {
	    int lodLevel;
	    int instanceId; 
	    std::vector<float> vertices;
	    std::vector<float> normals;
	    std::vector<float> faceNormals;
	    std::vector<float> colors;
	    std::vector<float> templatePos;
	    
	    // Per-rock info for rendering
	    std::vector<int> rockDataIndices;    // Index into data array
	    std::vector<int> rockInstanceIds;    // Rock instance/type
	    std::vector<int> rockOffsets;        // Vertex offset for each rock
	    std::vector<int> rockTriCounts;      // Triangle count for each rock
	    
	    GLuint vboVertices = 0;
	    GLuint vboNormals = 0;
	    GLuint vboFaceNormals = 0;
	    GLuint vboColors = 0;
	    GLuint vboTemplatePos = 0;
	    
	    void clear() {
	        vertices.clear();
	        normals.clear();
	        faceNormals.clear();
	        colors.clear();
	        templatePos.clear();
	        rockDataIndices.clear();
	        rockInstanceIds.clear();
	        rockOffsets.clear();
	        rockTriCounts.clear();
	    }
	    
	    void deleteVBOs() {
	        if (vboVertices) glDeleteBuffers(1, &vboVertices);
	        if (vboNormals) glDeleteBuffers(1, &vboNormals);
	        if (vboFaceNormals) glDeleteBuffers(1, &vboFaceNormals);
	        if (vboColors) glDeleteBuffers(1, &vboColors);
	        if (vboTemplatePos) glDeleteBuffers(1, &vboTemplatePos);
	        vboVertices = vboNormals = vboFaceNormals = vboColors = vboTemplatePos = 0;
	    }
	};


	static std::map<BatchKey, VBOBatch> rockBatches;  // Changed key type
	//static std::map<int, VBOBatch> rockBatches;
	void applyVertexAttributes(MCObject* rock, double amplitude, TNode *tv, TNode *tc);
	MCObject* getTemplateForLOD(Rock3DData *s);
public:
	// Cache key based on world position
	struct RockCacheKey {
		long long x, y, z;  // Quantized world position
		int instance;
		
		RockCacheKey(const Point& worldPos, int inst, double snap = 1e-9) {
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
    static int lodCacheHits;
    static int lodCacheMisses;
    static int templates_per_lod;
    static std::map<int, MCObject*> lodTemplates;
 
	static ValueList<PlaceData*> data;
    static MCObjectManager rocks;  
    static void freeLODTemplates();

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
	static int stats[MAX_ROCK_STATS][3];
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
};


#endif


