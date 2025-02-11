
//  UniverseModel.h

#ifndef _UNIVERSEMODEL
#define _UNIVERSEMODEL

#include "ModelClass.h"

class Orbital;
extern void set_orbital(Orbital*);  // in sx.y

enum tree_codes{
	TN_SCENE  		= 0x00000001,
	TN_UNIVERSE  	= 0x00000002,
	TN_GALAXY  		= 0x00000003,
	TN_NEBULA  		= 0x00000004,
	TN_SYSTEM  		= 0x00000005,
	TN_STAR     	= 0x00000006,
	TN_CORONA  		= 0x00000007,
	TN_HALO  		= 0x00000008,
	TN_PLANET   	= 0x00000009,
	TN_MOON     	= 0x0000000a,
	TN_SKY      	= 0x0000000b,
	TN_CLOUDS  		= 0x0000000c,
	TN_RING	 		= 0x0000000d,
	TN_VAR          = 0x0000000e,
	TN_SURFACE      = 0x0000000f,

	TN_WATER  		= 0x00000010,
	TN_FOG    	    = 0x00000011,
	TN_CRATERS  	= 0x00000012,
	TN_ROCKS  	    = 0x00000013,
	TN_MAP  		= 0x00000014,
	TN_LAYER  		= 0x00000015,
	TN_COMP  		= 0x00000016,
	TN_VALUE  	    = 0x00000017,
	TN_BANDS  	    = 0x00000018,
	TN_COLOR  	    = 0x00000019,
	TN_TEXTURE  	= 0x0000001a,
	TN_IMAGE	  	= 0x0000001b,
	TN_FCHNL  	    = 0x0000001c,
	TN_NOISE  	    = 0x0000001e,
	TN_GROUP  	    = 0x0000001f,
	TN_ERODE  		= 0x0000001f,
	TN_SNOW  		= 0x00000020,
	TN_POINT  	    = 0x00000021,
	TN_DENSITY  	= 0x00000022,
	TN_VCOLOR  	    = 0x00000023,
	TN_VTEXTURE  	= 0x00000024,
	TN_WAVES  	    = 0x00000025,
	TN_GLOSS  	    = 0x00000026,
	TN_BASE  	    = 0x00000027,
	TN_CLOUD  		= 0x00000028,
	TN_SPRITE  	    = 0x00000029,
	TN_PLANT  	    = 0x0000002a,
	TN_PLANT_BRANCH = 0x0000002b,
	TN_PLANT_LEAF  	= 0x0000002c,
};

enum gen_codes{
	GN_RANDOM  		= 0x00000000,
	GN_TREE  		= 0x00010000,
	GN_BUSH  		= 0x00020000,
	GN_GRASS  		= 0x00030000,
	GN_PLANT  		= GN_TREE|GN_BUSH|GN_GRASS,
	GN_GASSY  		= 0x00040000,
	GN_CRATERED  	= 0x00050000,
	GN_VOLCANIC  	= 0x00060000,
	GN_ROCKY  		= 0x00070000,
	GN_ICY  		= 0x00080000,
	GN_OCEANIC      = 0x00090000,
	GN_PLANET  		= GN_GASSY|GN_ROCKY|GN_ICY|GN_CRATERED|GN_VOLCANIC|GN_OCEANIC,
	GN_MOON  		= GN_ROCKY|GN_ICY|GN_CRATERED|GN_VOLCANIC|GN_OCEANIC,
	GN_RED_STAR     = 0x000a0000,
	GN_YELLOW_STAR  = 0x000b0000,
	GN_WHITE_STAR   = 0x000c0000,
	GN_BLUE_STAR    = 0x000d0000,
	GN_STAR         = GN_YELLOW_STAR|GN_WHITE_STAR|GN_BLUE_STAR|GN_RED_STAR,
	GN_WATER        = 0x000e0000,
	GN_OCEAN        = GN_WATER,
	GN_THIN         = 0x00100000,
	GN_MED          = 0x00200000,
	GN_DENSE        = 0x00300000,
	GN_SKY          = GN_THIN|GN_DENSE|GN_MED,

};
class UniverseModel : public Model
{
protected:
public:
	static char default_model[];
	static char default_galaxy[];
	void make(FILE *);
	void make_libraries(int w);

	NodeIF *parse_node(NodeIF *parent,char *s);
	TreeNode *insertInTree(TreeNode *parent, NodeIF *node);
	TreeNode *addToTree(TreeNode *parent, NodeIF *node);
	TreeNode *addToTree(TreeNode *parent, TreeNode *child, NodeIF *node);
	TreeNode *replaceInTree(TreeNode *parent, TreeNode *child, NodeIF *node);
	TreeNode *lastBranch(TreeNode *n);
	int getPrototype(int,char *);
	int setPrototype(NodeIF*, NodeIF*);
	int getAddList(NodeIF*,LinkedList<ModelSym*>&list);
	int getSaveList(NodeIF*,LinkedList<ModelSym*>&list);
	int getReplaceList(NodeIF*,LinkedList<ModelSym*>&list);
	static ModelSym* getObjectSymbol(int);
	static ModelSym* getTypeSymbol(int);
	bool hasTypeList(int t);
	void getTypeList(int type,LinkedList<ModelSym*>&list);
	void getFileList(int type,LinkedList<ModelSym*>&list);
	void getObjectDirectory(int type,char *dir);
	void getFullPath(ModelSym*m,char *c);
	TreeNode *buildTree(NodeIF *);
	void setType(NodeIF *);
	NodeIF *open_node(NodeIF *parent,char *s);
	static std::string typeSymbol(int type);
};
#endif
