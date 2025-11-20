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

#define MAX_BRANCHES 6
#define MAX_PLANT_DATA 7
#define MAX_PLANTS 7

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
	static int stats[MAX_PLANTS];

	static double render_time;
	static bool threed;
	static bool spline;
	static bool poly_lines;
	static bool shader_lines;
	static bool no_cache;
	static int textures;
	static bool shadow_mode;
	static int shadow_count;
	static bool show_one;
	static bool first_instance;
	
	Color c;
	double slope_bias;
	double ht_bias;
	double lat_bias;
	double hardness_bias;

	~PlantMgr();
	PlantMgr(int,TNplant*);
	PlantMgr(int);

	void init();

	static void clearStats();
};


class Plant : public PlaceObj
{
public:
	static ValueList<PlaceData*> data;
	ValueList<BranchData*> branches;
	ValueList<BranchData*> leafs;

	void freeBranches() {branches.free();}
	void renderBranches();
	void collectBranches(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d,Point4D s,Color c);

	bool sorted;
	void renderLeafs();	
	void collectLeafs(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d,Point4D s,Color c);
	void freeLeafs() {leafs.free();sorted=false;}
	void sortLeafs() {if(!sorted)leafs.sort();sorted=true;}

	Plant(int l, TNode *e);
	
	PlacementMgr *mgr();
	void eval();
	bool setProgram();
	void clearStats();
	void showStats();
	
	static void reset();
	static void collect(Array<PlaceObj*> &data);
	static void eval(Array<PlaceObj*> &data);
	
	static void freeLeafs(Array<PlaceObj*> &data);
	static void freeBranches(Array<PlaceObj*> &data);
	static void renderBranches(Array<PlaceObj*> &data);
	static void renderLeafs(Array<PlaceObj*> &data);	
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
	void collect();
	void free() { data.free();}
	int placements(){ return data.size;}

};
//************************************************************
// Class TNLeaf
//************************************************************
class BranchData
{
public:
	Point4D data[6];
	Color c;
	BranchData(Point4D p0,Point4D p1,Point4D p2, Point4D f, Point4D d, Point4D s, Color col){
		data[0]=p0;data[1]=p1;data[2]=p2;data[3]=f;data[4]=d;data[5]=s;
		c=col;
	}
	double distance();
	double value() { return distance();}
	void render();

};

#endif


