// Rocks.h

#ifndef _ROCKS
#define _ROCKS

#include "Placements.h"
#include "MCObjects.h"
#include <math.h>
#include <map>

class RockMgr;
class TNnode;
class TNtexture;
class Texture;
class Rock3DData;
class Rock3DMgr;


//#define OLD

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
	// 1. GLVertex first — used by everything else
	struct GLVertex {
		float pos[3];          // offset 0  (12 bytes)
		float normal[3];       // offset 12 (12 bytes)
		float templatePos[4];  // offset 24 (16 bytes)
		float faceNormal[4];   // offset 40 (16 bytes)
		float color[3];        // offset 56 (12 bytes)
	};                         // total: 68 bytes

	// 2. RockInstance — used by InstanceVBO and LODBatch
	struct RockInstance {
		float eyeCenter[3];
		float rockSize;
		float rval;
		float pad[3];
	};

	// 3. TemplateMeshVBO — uses GLVertex
	struct TemplateMeshVBO {
		GLuint vbo = 0;
		int vertexCount = 0;
		void upload(const std::vector<GLVertex>& vertices) {
			cleanup();
			if (vertices.empty()) return;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER,
						 vertices.size() * sizeof(GLVertex),
						 vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			vertexCount = (int)vertices.size();
		}
		void cleanup() {
			if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
			vertexCount = 0;
		}
		bool valid() const { return vbo != 0 && vertexCount > 0; }
	};

	// 4. InstanceVBO — uses RockInstance
	struct InstanceVBO {
		GLuint vbo = 0;
		int instanceCount = 0;
		void upload(const std::vector<RockInstance>& instances) {
			if (instances.empty()) { instanceCount = 0; return; }
			if (!vbo) glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER,
						 instances.size() * sizeof(RockInstance),
						 instances.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			instanceCount = (int)instances.size();
		}
		void cleanup() {
			if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
			instanceCount = 0;
		}
	};

	// 5. LODBatch — uses TemplateMeshVBO and InstanceVBO
	struct LODBatch {
		TemplateMeshVBO templateVBO;
		InstanceVBO instanceVBO;
		std::vector<RockInstance> instances;
		int lodLevel   = 0;
		int instanceId = 0;
		void cleanup() {
			templateVBO.cleanup();
			instanceVBO.cleanup();
			instances.clear();
		}
	};                     // total: 68 bytes
	struct VBOBatch {
	    int lodLevel;
	    int instanceId; 
	    std::vector<GLVertex> glVertices;  // replaces 5 separate vectors
	    	    
	    GLuint vboVertices = 0;  // single interleaved VBO
	    int uploadedVertexCount = 0;
	    
	    void clear() {
	        glVertices.clear();
	    }
	    
	    void deleteVBOs() {
	        if (vboVertices) glDeleteBuffers(1, &vboVertices);
	        vboVertices = 0;
	    }
	};
	struct BatchKey {
		int resolution;
		int instanceId;
		
		BatchKey(int r, int i) : resolution(r), instanceId(i) {}
		
		bool operator<(const BatchKey& other) const {
			if (resolution != other.resolution) return resolution < other.resolution;
			return instanceId < other.instanceId;
		}
	};
	struct AdaptiveBatch {
	    VBOBatch vbo;
	    Point eyeCenter;
	    float rockSize = 1.0f;
	    float rval = 0.0f;
	    int instanceId = 0;
	    
	    void clear() { vbo.clear(); }
	    void deleteVBOs() { vbo.deleteVBOs(); }
	};
    static MCObjTreeMgr rockTreeMgr;
    void fixupAdaptiveNormals(std::vector<MCTriangle>& mesh);
    void applyAdaptiveAttributes(std::vector<MCTriangle>& mesh,
                                 Rock3DMgr* pmgr, double isoNoiseAmpl);
    static std::map<int, AdaptiveBatch> adaptiveBatches;
    static std::map<BatchKey, LODBatch> lodBatches;

	void applyVertexAttributes(MCObject* rock, double amplitude, TNode *tv, TNode *tc);
	MCObject* getTemplateForLOD(Rock3DData *s);
    void rebuild();
    void calibrate();
    std::vector<Rock3DObjMgr::GLVertex> buildTemplateVertices(MCObject* tmpl);
    void renderLODBatch(LODBatch& lbatch, GLhandleARB program);
	
public:
	// Cache key based on world position
	struct RockCacheKey {
		long long x, y, z;  // Quantized world position
		int instance;
	    int viewBucketX, viewBucketY, viewBucketZ;  // quantized view direction
	    // Original - for template path (no view direction)
	    RockCacheKey(const Point& worldPos, int inst, double snap = 1e-10) {
	        x = (int64_t)round(worldPos.x / snap);
	        y = (int64_t)round(worldPos.y / snap);
	        z = (int64_t)round(worldPos.z / snap);
	        instance = inst;
	        viewBucketX = viewBucketY = viewBucketZ = 0;
	    }
	    RockCacheKey(const Point& worldPos, int inst, 
	                 const Point& viewDir,
	                 double angleBucketDeg = 30.0,
	                 double snap = 1e-10)  
	     {
		        x = (int64_t)round(worldPos.x / snap);
		        y = (int64_t)round(worldPos.y / snap);
		        z = (int64_t)round(worldPos.z / snap);
		        instance = inst;  // STORE INSTANCE
		        // Quantize view direction to buckets of angleBucketDeg
				double bucketSize = angleBucketDeg / 180.0;  
				viewBucketX = (int)round(viewDir.x / bucketSize);
				viewBucketY = (int)round(viewDir.y / bucketSize);
				viewBucketZ = (int)round(viewDir.z / bucketSize);
		}
	    bool operator<(const RockCacheKey& other) const {
	        if (instance != other.instance) return instance < other.instance;
	        if (x != other.x) return x < other.x;
	        if (y != other.y) return y < other.y;
	        if (z != other.z) return z < other.z;
	        if (viewBucketX != other.viewBucketX) return viewBucketX < other.viewBucketX;
	        if (viewBucketY != other.viewBucketY) return viewBucketY < other.viewBucketY;
	        return viewBucketZ < other.viewBucketZ;
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
    static int maxTexs;
    static int lodCacheHits;
    static int lodCacheMisses;
    static int templates_per_lod;
    static std::map<int, MCObject*> lodTemplates;
 
	static ValueList<PlaceData*> data;
    static MCObjectManager rocks;  
    static void freeLODTemplates();

    ~Rock3DObjMgr();
    
    void renderBatch(VBOBatch& batch, GLhandleARB program, const AdaptiveBatch& ab);
    void uploadBatchVBOs(VBOBatch& batch);
    void addTriangleToBatch(VBOBatch& batch, const MCTriangle& tri, Rock3DData* s);
    SurfaceFunction makeRockField(Rock3DMgr* pmgr,bool mode);
    
	void free();
	int placements(){ return data.size;}
	void collect();
	const char *name() { return "Rocks3D";}
    bool setProgram();
    void render();
	void render_zvals();
	void render_shadows();
    void render_objects();
    void clear();	
    bool supports_shadows() { return true;}
 
};

class Rock3DData : public PlaceData
{
public:
	Rock3DData(Placement *s) : PlaceData(s){}
    double value();
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
	struct NoiseCalib {
		double scale = 1.0;
		double offset = 0.0;
		bool calibrated = false;
	};
	struct RockLodEntry {
	    int    res;     // voxel resolution
	    double maxPts;  // upper bound for pts (pts < maxPts)
	};

	static const RockLodEntry kRockLodTable[MAX_ROCK_STATS];
    static int getLodIndex(int scaledRes, double resScale);

	NoiseCalib noiseCalib;
	ValueList<TNtexture *> texs;
	TNode   *vnoise;
	TNode   *rnoise;
	TNode   *color;
	static int stats[MAX_ROCK_STATS][5];
	static void clearStats();
	static void printStats();
	static void setStats(int,int,bool);
	static int getLODResolution(double pts);
	static double resScale,adaptThreshold;
	static double noiseFactor,maxDepth,minPointsize;


 	Rock3DMgr(int);
	Placement *make(Point4DL&,int);
	PlaceData *make(Placement*s);
	void eval();
	void init();
};


#endif


