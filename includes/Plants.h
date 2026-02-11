// Plants.h

#ifndef _PLANTS
#define _PLANTS

#include "Placements.h"
#include "defs.h"

class PlantMgr;
class TNplant;
class TNBranch;
class TNLeaf;
class Plant;
class BranchData;
class PlacementMgr;
#include <vector>
#include <map>

#define MAX_PLANT_STATS 7

#define USE_VBO

class Vec4{
public:
	float x,y,z,w;
	Vec4() {x=y=z=w=0.0f;}
	Vec4(Point p) { x=p.x;y=p.y;z=p.z;w=0.0f;}
	Vec4(Point p,double a) { x=p.x;y=p.y;z=p.z;w=a;}
	Vec4(float a,float b,float c){x=a;y=b;z=c;w=0.0f;}
	Vec4(double a,double b,double c,double d){x=a;y=b;z=c;w=d;}
	float length()	{ return sqrt(x*x+y*y+z*z+w*w);}
	Point point() { return Point(x,y,z);}
};

class LeafImageMgr : public ImageMgr
{
public:
	LeafImageMgr() : ImageMgr(){
		setImageBaseDir();
		getImageDirs();
	}
	void setImageBaseDir(){
		char base[512];
		File.getBaseDirectory(base);
	 	sprintf(base_dir,"%s/Textures/Plants/Leaf",base);
	}
};
extern LeafImageMgr leaf_mgr;

class BranchImageMgr : public ImageMgr
{
public:
	BranchImageMgr() : ImageMgr(){
		setImageBaseDir();
		getImageDirs();
	}
	void setImageBaseDir(char *c){
		ImageMgr::setImageBaseDir(c);
	}
	void setImageBaseDir(){
		File.getBranchesDir(base_dir);
		getImageDirs();
	}
};
extern BranchImageMgr branch_mgr;
class PlantMgr : public PlacementMgr
{
protected:
public:
	Placement *make(Point4DL&,int);
	PlaceData *make(Placement*s);
	
	TNplant *plant;
	static int stats[MAX_PLANT_STATS];

	static double render_time;
	static bool threed;
	static bool spline;
	static bool poly_lines;
	static bool shader_lines;
	static int textures;
	static bool show_one;
	static bool first_instance;
	static bool update_needed;
	static bool vbo_valid;

	~PlantMgr();
	PlantMgr(int,TNplant*);
	
	void init();

	static void clearStats();
};

struct BranchVertex {
    Vec4 pos;       // -> gl_Vertex (location 0)
    Vec4 common1;   // -> CommonAttributes1 (location 1)
    Vec4 color;     // -> gl_Color (location 3)
    Vec4 common2;   // -> CommonAttributes2 (location 4)
    Vec4 common3;   // -> CommonAttributes3 (location 5)
    Vec4 texcoord;  // -> TextureAttributes (location 6)
};


// In Plants.h

class BranchVBO {
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    int vertCount = 0;
    bool dirty = true;
    
    std::vector<BranchVertex> vertexesObjectSpace;  // Object space (permanent, cacheable)
    std::vector<BranchVertex> vertexesEyeSpace;     // Eye space (rebuilt each frame)

    void clear() {
        vertexesObjectSpace.clear();
        vertexesEyeSpace.clear();
        dirty = true;
        
        if (vbo) {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        if (vao) {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
    }

    // Add branch in object space (no worldpos)
    void addBranch(Vec4 p0, Vec4 p1, Vec4 p2, Vec4 f, Vec4 d, Vec4 s, 
                   Color c, float shaderMode);

    // Upload vertexesEyeSpace to GPU
    void build();

    // Render from GPU
    void render();

    void free();

    int size()  { return vertexesObjectSpace.size() / 2; }
    bool empty(){ return vertexesObjectSpace.empty(); }
	void appendEyeSpace(const std::vector<BranchVertex>& objVerts, Point plantCenter);

};


class Plant : public PlaceObj
{
public:

	BranchVBO branchVBO;
	BranchVBO lineVBO;
	BranchVBO leafVBO;
	
	BranchVBO accumBranchVBO;  // Persistent: accumulates ALL instances
	BranchVBO accumLineVBO;
	BranchVBO accumLeafVBO;

    ~Plant() {
        branchVBO.free();
        lineVBO.free();
        leafVBO.free();
        accumBranchVBO.free();
        accumLineVBO.free();
        accumLeafVBO.free();
        leafs.free();
    }
	void collectBranches(Vec4 p0, Vec4 p1, Vec4 p2, Vec4 f, Vec4 d, Vec4 s, Color c);
	void collectLines(Vec4 p0, Vec4 p1, Vec4 p2, Vec4 f, Vec4 d, Vec4 s, Color c);
    void freeBranches();

	ValueList<BranchData*> leafs;
	void renderBranches();

	bool sorted;
	void renderLeafs();	
	void collectLeafs(Vec4 p0,Vec4 p1,Vec4 p2, Vec4 f, Vec4 d,Vec4 s,Color c);
	void freeLeafs();
	void sortLeafs();

	Plant(int l, TNode *e);
	
	PlacementMgr *mgr();
	bool setProgram();
	void clearStats();
	void showStats();
};

class PlantData : public PlaceData
{
public:
	PlantData(Placement *s) : PlaceData(s){}
    double value();
};
class PlantObjMgr : public PlaceObjMgr
{
	// Cache key - world position + plant type (like RockCacheKey)
	struct PlantCacheKey {
		long long x, y, z;
		int instance;
		
		PlantCacheKey(const Point& worldPos, int inst, double snap=1e-9) {
			//cout<<worldPos/snap<<endl;
			x = (int64_t)round(worldPos.x / snap);
			y = (int64_t)round(worldPos.y / snap);
			z = (int64_t)round(worldPos.z / snap);
			//cout<<x<<" "<<y<<" "<<z<<endl;
			instance = inst;
		}
		
		bool operator<(const PlantCacheKey& other) const {
			if (instance != other.instance) return instance < other.instance;
			if (x != other.x) return x < other.x;
			if (y != other.y) return y < other.y;
			return z < other.z;
		}
	};
	
	struct PlantCacheEntry {
		std::vector<BranchVertex> branches;  // Object space geometry
		std::vector<BranchVertex> lines;
		std::vector<BranchVertex> leafs;
		
		// Generation parameters
		Point  worldVertex;
		float  pts;
		float  drop;
		float  size;
		int    instance;
		int    seed;
		float  rval;
		int    framesSinceUsed = 0;
		
		bool needsRegen(float newPts) const {
			double delta=fabs(pts - newPts)/(pts+1);
			//cout<<pts<<" "<<newPts<<" "<<delta<<endl;
			return delta > 0.25f;
		}
	};
	
protected:
	static ValueList<PlaceData*> data;
	void freeLeafs();
	void freeBranches();
	void renderBranches();
	void renderLeafs();
public:
	static int cacheHits;
	static int cacheMisses;
	static int cacheDeletes;
	static int maxCacheSize;
	static std::map<PlantCacheKey, PlantCacheEntry> plantCache;

	bool setProgram();
	void render();
	void render_zvals();
	void render_shadows();
	bool supports_shadows() { return true;}
	void collect();
	void free();
	void updateCache();
	void clearCache();
	int placements(){ return data.size;}
	const char *name() { return "Plants";}

};
//************************************************************
// Class TNLeaf
//************************************************************

class BranchData
{
public:
	Vec4 data[6];
	Color c;
	Point position;
	BranchData(Vec4 p0,Vec4 p1,Vec4 p2, Vec4 f, Vec4 d, Vec4 s, Color col){
		data[0]=p0;data[1]=p1;data[2]=p2;data[3]=f;data[4]=d;data[5]=s;
		c=col;
	}
	double distance();
	double value() { return distance();}
	void render();
	void renderData();

};


#endif


