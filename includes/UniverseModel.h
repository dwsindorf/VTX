
//  UniverseModel.h

#ifndef _UNIVERSEMODEL
#define _UNIVERSEMODEL

#include "ModelClass.h"

class Orbital;
extern void set_orbital(Orbital*);  // in sx.y

enum tree_codes{
	TN_SCENE  		= 0x0001,
	TN_UNIVERSE  	= 0x0002,
	TN_GALAXY  		= 0x0003,
	TN_NEBULA  		= 0x0004,
	TN_SYSTEM  		= 0x0005,
	TN_STAR     	= 0x0006,
	TN_CORONA  		= 0x0007,
	TN_HALO  		= 0x0008,
	TN_PLANET   	= 0x0009,
	TN_MOON     	= 0x000a,
	TN_SKY      	= 0x000b,
	TN_CLOUDS  		= 0x000c,
	TN_RING	 		= 0x000d,
	TN_VAR          = 0x000e,
	TN_SURFACE      = 0x000f,

	TN_WATER  		= 0x0010,
	TN_FOG    	    = 0x0011,
	TN_CRATERS  	= 0x0012,
	TN_ROCKS  	    = 0x0013,
	TN_MAP  		= 0x0014,
	TN_LAYER  		= 0x0015,
	TN_COMP  		= 0x0016,
	TN_VALUE  	    = 0x0017,
	TN_BANDS  	    = 0x0018,
	TN_COLOR  	    = 0x0019,
	TN_TEXTURE  	= 0x001a,
	TN_IMAGE	  	= 0x001b,
	TN_FCHNL  	    = 0x001c,
	TN_NOISE  	    = 0x001e,
	TN_GROUP  	    = 0x001f,
	TN_ERODE  		= 0x001f,
	TN_SNOW  		= 0x0020,
	TN_POINT  	    = 0x0021,
	TN_DENSITY  	= 0x0022,
	TN_VCOLOR  	    = 0x0023,
	TN_VTEXTURE  	= 0x0024,
	TN_WAVES  	    = 0x0025,
	TN_GLOSS  	    = 0x0026,
	TN_BASE  	    = 0x0027,
	TN_CLOUD  		= 0x0028,
	TN_SPRITE  	    = 0x0029,
	TN_PLANT  	    = 0x002a,
	TN_PLANT_BRANCH = 0x002b,
	TN_PLANT_LEAF  	= 0x002c
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
	ModelSym* getObjectSymbol(int);
	void getFileList(int type,LinkedList<ModelSym*>&list);
	void getObjectDirectory(int type,char *dir);
	void getFullPath(ModelSym*m,char *c);
	TreeNode *buildTree(NodeIF *);
	void setType(NodeIF *);
	NodeIF *open_node(NodeIF *parent,char *s);
};
#endif
