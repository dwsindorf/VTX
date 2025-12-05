// Sprites.h

#ifndef _SPRITES
#define _SPRITES

#include "Placements.h"
#include "FileUtil.h"
#include <vector>

class SpriteMgr;
class TNsprite;

#define USE_SPRITES_VBO

struct SpriteVertex {
    float pos[3];       // vertex position -> gl_Vertex
    float texcoord[4];  // id, rows, pts, sel -> TexCoordsID
    float attrib1[4];   // flip, cols, sx, sy -> CommonID1
};
class SpriteVBO {
    GLuint vao = 0;
    GLuint vbo = 0;
    int vertCount = 0;
    bool dirty = true;
    std::vector<SpriteVertex> vertices;

public:
    void clear() {
        vertices.clear();
        dirty = true;
    }

    void addSprite(Point &p, int id, int rows, int cols, double pts, double sel, int sx, int sy, float flip) {
        SpriteVertex v;
        v.pos[0] = p.x;
        v.pos[1] = p.y;
        v.pos[2] = p.z;
        v.texcoord[0] = id + 0.1f;
        v.texcoord[1] = rows;
        v.texcoord[2] = pts;
        v.texcoord[3] = sel;
        v.attrib1[0] = flip;
        v.attrib1[1] = cols;
        v.attrib1[2] = sx;
        v.attrib1[3] = sy;
        vertices.push_back(v);
        dirty = true;
    }

    int size() { return vertices.size(); }
    bool empty() { return vertices.empty(); }

    void build();
    void render();
    void free();
};
class SpritePoint : public Placement
{
public:
	int sprites_rows;
	int sprites_cols;

	double variability;
	double rand_flip_prob;
	double select_bias;

	SpritePoint(SpriteMgr&, Point4DL&,int);
};

class SpriteData  : public PlaceData
{
public:
	int sprites_rows;
	int sprites_cols;
	
	double variability;
	double rand_flip_prob;
	double select_bias;

	SpriteData(SpritePoint*s);
	int flip(){ return type & FLIP;}

};

class SpriteImageMgr : public ImageMgr
{
public:
	SpriteImageMgr() : ImageMgr(){
		setImageBaseDir();
		getImageDirs();
	};
	void setImageBaseDir();
};
extern SpriteImageMgr sprites_mgr;

class SpriteMgr : public PlacementMgr
{
protected:
public:
	Placement *make(Point4DL&,int);
	PlaceData *make(Placement*s);

	Color c;

	int sprites_rows;
	int sprites_cols;
	double variability;
	double rand_flip_prob;
	double select_bias;
	
	~SpriteMgr();
	SpriteMgr(int);

	void init();
	void eval();
	static bool setProgram(Array<PlaceObj*> &objs);

};
class SpriteObjMgr : public PlaceObjMgr
{
public:
	static ValueList<PlaceData*> data;
    static SpriteVBO spriteVBO;  // Add this
	static bool vbo_valid;  // Add this

	bool setProgram();
	void render();
	void free() { data.free();}
	int placements(){ return data.size;}
	void collect();
	const char *name() { return "Sprites";}
};

class Sprite : public PlaceObj
{
public:
	Image *image;
	unsigned int texture_id;
	uint rows;
	uint cols;
	Sprite(Image *i, int l, TNode *e);
	
	SpriteMgr *mgr();// { return ((TNsprite*)expr)->mgr;}
	bool setProgram();
	void set_image(Image *, int r, int c);
};
#endif


