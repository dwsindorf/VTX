
//  UniverseModel.h

#ifndef _UNIVERSEMODEL
#define _UNIVERSEMODEL

#include "ModelClass.h"

class Orbital;
extern void set_orbital(Orbital*);  // in sx.y

enum tree_codes{
	TN_SCENE  		= 0x000001,
	TN_UNIVERSE  	= 0x000002,
	TN_GALAXY  		= 0x000003,
	TN_NEBULA  		= 0x000004,
	TN_SYSTEM  		= 0x000005,
	TN_STAR     	= 0x000006,
	TN_CORONA  		= 0x000007,
	TN_HALO  		= 0x000008,
	TN_PLANET   	= 0x000009,
	TN_MOON     	= 0x00000a,
	TN_SKY      	= 0x00000b,
	TN_CLOUDS  		= 0x00000c,
	TN_RING	 		= 0x00000d,
	TN_VAR          = 0x00000e,
	TN_SURFACE      = 0x00000f,

	TN_WATER  		= 0x000010,
	TN_FOG    	    = 0x000011,
	TN_CRATERS  	= 0x000012,
	TN_ROCKS  	    = 0x000013,
	TN_MAP  		= 0x000014,
	TN_LAYER  		= 0x000015,
	TN_COMP  		= 0x000016,
	TN_VALUE  	    = 0x000017,
	TN_BANDS  	    = 0x000018,
	TN_COLOR  	    = 0x000019,
	TN_TEXTURE  	= 0x00001a,
	TN_IMAGE	  	= 0x00001b,
	TN_FCHNL  	    = 0x00001c,
	TN_NOISE  	    = 0x00001e,
	TN_GROUP  	    = 0x00001f,
	TN_ERODE  		= 0x00001f,
	TN_SNOW  		= 0x000020,
	TN_POINT  	    = 0x000021,
	TN_DENSITY  	= 0x000022,
	TN_VCOLOR  	    = 0x000023,
	TN_VTEXTURE  	= 0x000024,
	TN_WAVES  	    = 0x000025,
	TN_GLOSS  	    = 0x000026,
	TN_BASE  	    = 0x000027,
	TN_CLOUD  		= 0x000028,
	TN_SPRITE  	    = 0x000029,
	TN_PLANT  	    = 0x00002a,
	TN_PLANT_BRANCH = 0x00002b,
	TN_PLANT_LEAF  	= 0x00002c,
};

enum gen_codes{
	GN_RANDOM  		= 0x0000000,
	GN_TREE  		= 0x0010000,
	GN_BUSH  		= 0x0020000,
	GN_GRASS  		= 0x0030000,
	GN_PLANT  		= GN_TREE|GN_BUSH|GN_GRASS,
	GN_GASSY  		= 0x0040000,
	GN_ROCKY  		= 0x0050000,
	GN_ICY  		= 0x0060000,
	GN_OCEANIC      = 0x0070000,
	GN_PLANET  		= GN_GASSY|GN_ROCKY|GN_ICY|GN_OCEANIC,
	GN_MOON  		= GN_GASSY|GN_ROCKY|GN_ICY|GN_OCEANIC,
	GN_RED_STAR     = 0x00a0000,
	GN_YELLOW_STAR  = 0x00b0000,
	GN_WHITE_STAR   = 0x00c0000,
	GN_BLUE_STAR    = 0x00d0000,
	GN_STAR         = GN_YELLOW_STAR|GN_WHITE_STAR|GN_BLUE_STAR|GN_RED_STAR,

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
