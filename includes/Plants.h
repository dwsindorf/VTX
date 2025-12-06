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

#define MAX_PLANT_STATS 7

#define USE_VBO

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
	
	TNplant *plant;
	static int stats[MAX_PLANT_STATS];

	static double render_time;
	static bool threed;
	static bool spline;
	static bool poly_lines;
	static bool shader_lines;
	static int textures;
	static bool shadow_mode;
	static int shadow_count;
	static bool show_one;
	static bool first_instance;
	static bool update_needed;
	static bool vbo_valid;

	~PlantMgr();
	PlantMgr(int,TNplant*);
	
	static void beginFrame();

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

class BranchVBO {
    GLuint vao = 0;
    GLuint vbo = 0;
    int vertCount = 0;
    bool dirty = true;
    std::vector<BranchVertex> vertices;

public:
    void clear() {
        vertices.clear();
        dirty = true;
        
        // Force GPU buffer to be recreated
        if (vbo) {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        if (vao) {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
    }

    void addBranch(Vec4 p0, Vec4 p1, Vec4 p2, Vec4 f, Vec4 d, Vec4 s, Color c, float shaderMode) {
        BranchVertex v;
        v.common1 = f;
        v.common2 = p0;
        v.common3 = s;
        v.texcoord = Vec4(d.x, d.y, d.z, shaderMode);
        v.color = Vec4(c.red(), c.green(), c.blue(), c.alpha());

        v.pos = p1;
        vertices.push_back(v);
        v.pos = p2;
        vertices.push_back(v);

        dirty = true;
    }

    int size() { return vertices.size() / 2; }
    bool empty() { return vertices.empty(); }

    void build();
    void render();
    void free();
};
class Plant : public PlaceObj
{
public:
#ifdef USE_VBO
	BranchVBO branchVBO;
	BranchVBO lineVBO;
	BranchVBO leafVBO;
    ~Plant() {
        branchVBO.free();
        lineVBO.free();
        leafVBO.free();
        leafs.free();
    }
#else
    ~Plant() {
    	leafs.free();
    }
	ValueList<BranchData*> branches;
	ValueList<BranchData*> lines;
#endif
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

class PlantObjMgr : public PlaceObjMgr
{
protected:
	static ValueList<PlaceData*> data;
	void freeLeafs();
	void freeBranches();
	void renderBranches();
	void renderLeafs();
public:
	bool setProgram();
	void render();
	void render_zvals();
	void render_shadows();
	bool supports_shadows() { return true;}
	void collect();
	void free();
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


