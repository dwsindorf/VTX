// ModelClass.cpp
#define HIDE_UNIVERSE

#define INIT_GLOBAL  	0
#define INIT_SYSTEM1  	1
#define INIT_STAR1    	2
#define INIT_MERCURY  	3
#define INIT_VENUS  	4
#define INIT_EARTH   	5
#define INIT_JUPITER 	6
#define INIT_SATURN  	7
#define INIT_TITAN  	8
#define INIT_SYSTEM2  	9
#define INIT_STAR2  	10

#define INIT_MODE    INIT_SYSTEM1
//#define INIT_MODE    INIT_SYSTEM2
//#define INIT_MODE    INIT_STAR1
//#define INIT_MODE    INIT_STAR2
//#define INIT_MODE    INIT_EARTH
//#define INIT_MODE    INIT_MERCURY
//#define INIT_MODE    INIT_VENUS
//#define INIT_MODE    INIT_JUPITER
//#define INIT_MODE    INIT_SATURN
//#define INIT_MODE    INIT_TITAN

//#define  INIT_SURFACE


#include "UniverseModel.h"
#include "OrbitalClass.h"
#include "TerrainClass.h"
#include "TerrainData.h"
#include "ImageClass.h"
#include "TextureClass.h"
#include "FileUtil.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//************************************************************
// UniverseModel class
//************************************************************
void UniverseModel::getFullPath(ModelSym *m,char *path)
{
	path[0]=0;
	if(m->isFile()){
		//sprintf(path,"%s%s.spx",m->path,m->name);
		sprintf(path,"%s",m->path);
	}
	else{
		char dir[MAXSTR];
		getObjectDirectory(m->value,dir);
		sprintf(path,"%s%s%s.spx",dir,File.separator,m->name());
	}
}

void UniverseModel::getFileList(int type,LinkedList<ModelSym*>&list)
{
	char dir[MAXSTR];
	getObjectDirectory(type,dir);
	if(!dir[0])
		return;
	File.getFileNameList(dir,"*.spx",list);
}

//-------------------------------------------------------------
// UniverseModel::getObjectDirectory() return object's directory
//-------------------------------------------------------------
void UniverseModel::getObjectDirectory(int type,char *dir)
{
	dir[0]=0;

	char basedir[MAXSTR];
	char objects[MAXSTR];
	File.getBaseDirectory(basedir);
	if(!File.makeSubDirectory(basedir,File.objects,objects))
		return;
	ModelSym* sym=getObjectSymbol(type);
	File.makeSubDirectory(objects,sym->name(),dir);
	delete sym;
}

int UniverseModel::setPrototype(NodeIF *parent, NodeIF *child)
{
	if(!parent)
		return 0;
	int type=parent->typeClass();
//	cout << "c:" << child->typeName() << endl;
//	cout << "p:"<< parent->typeName() << endl;
	if((type & ID_OBJECT)==0)
		return 0;
	type=child->getFlag(TN_TYPES);
	double f;

	double psize=((Object3D*)parent)->size;
	switch(type){
	case TN_GALAXY:
		f=100000*LY;
		((Galaxy*)child)->origin=Point(f*SRand(),f*SRand(),f*SRand());
		break;
	case TN_SYSTEM:
		f=1000*LY;
		((System*)child)->origin=Point(f*SRand(),0.01*f*SRand(),0.1*f*SRand());
		break;
	case TN_PLANET:
		((Planetoid*)child)->orbit_radius=psize*(0.1+0.2*SRand());
		((Planetoid*)child)->orbit_phase=360*Rand();
		break;
	case TN_CORONA:
		((Corona*)child)->size=psize*6;
		((Corona*)child)->size*=1+0.25*SRand();
		break;
	case TN_CLOUDS:
		((CloudLayer*)child)->size=psize*(1+0.01*(1+Rand()));
		break;
	case TN_SKY:
		((Sky*)child)->size=psize*(1+0.02*(1+0.5*Rand()));
		break;
	case TN_MOON:
		((Moon*)child)->size=0.05*psize*(1+0.5*SRand());
		((Moon*)child)->orbit_radius=((Moon*)child)->size+psize*(1+0.25*Rand());
		((Moon*)child)->orbit_phase=360*Rand();
		break;
	case TN_RING:
		((Ring*)child)->width=0.5*psize*(1+0.25*(Rand()-0.5));
		((Ring*)child)->inner_radius=1.20*psize*(1+0.25*Rand());
		((Ring*)child)->set_geometry();
		break;
	}
	return 1;
}

int UniverseModel::getPrototype(NodeIF *parent,int type,char *tmp)
{
	tmp[0]=0;
	double psize=0.5;
	if(parent && (parent->typeClass()& ID_OBJECT)){
		psize=((Object3D*)parent)->size;
	}
	switch(type&TN_TYPES){
	case TN_GALAXY:
		sprintf(tmp,"Galaxy(10000) {}\n");
		break;
	case TN_SYSTEM:
		sprintf(tmp,"System(1000) {}\n");
		break;
	case TN_STAR:
		sprintf(tmp,"Star(0.5) {Corona(4){}}\n");
		break;
	case TN_CORONA:
		sprintf(tmp,"Corona(4) {}\n");
		break;
	case TN_PLANET:
		sprintf(tmp,"Planet(0.004,0.01){day=24;year=100;Surface{}}\n");
		break;
	case TN_MOON:
		sprintf(tmp,"Moon(0.01,0.05){day=30;year=30;Surface{}}\n");
		break;
	case TN_SKY:
		sprintf(tmp,"Sky(0.1){color=Color(0,0.9,0.9);density=0.2;}\n");
		break;
	case TN_CLOUDS:
		sprintf(tmp,"Clouds(0.01){Surface{terrain=clouds()Color(1,1,1,noise(1,5));\n}}\n");
		break;
	case TN_RING:
		sprintf(tmp,"Ring(1.5,0.2){Surface{terrain=Texture(\"rings\",S,PHI,1.0,1.7);\n}}\n");
		break;
	case TN_COMP:
		sprintf(tmp,"()\n");
		break;
	case TN_TEXTURE:
		sprintf(tmp,"Texture(\"jupiter\",PHI)\n");
		break;
	case TN_MAP:
		sprintf(tmp,"map(noise(1,5))\n");
		break;
	case TN_LAYER:
		sprintf(tmp,"layer(MORPH,0.1)[Z(0)]\n");
		break;
	case TN_CRATERS:
		sprintf(tmp,"craters(1,0.05,0.2)\n");
		break;
	case TN_ROCKS:
		sprintf(tmp,"rocks(1,1e-6)[Z(0)]\n");
		break;
	case TN_FOG:
		sprintf(tmp,"fog(0.0)\n");
		break;
	case TN_SNOW:
		sprintf(tmp,"snow(1,0.7,1,1,1)\n");
		break;
	case TN_CLOUD:
		sprintf(tmp,"clouds()\n");
		break;
	case TN_FCHNL:
		sprintf(tmp,"fractal(SS|SQR,8,10,0.1)\n");
		break;
	case TN_ERODE:
		sprintf(tmp,"erode(4,6,1)\n");
		break;
	case TN_GLOSS:
		sprintf(tmp,"gloss(1,1)\n");
		break;
	case TN_BASE:
		sprintf(tmp,"base(0)\n");
		break;
	case TN_WATER:
		sprintf(tmp,"water(0.0)\n");
		break;
	case TN_COLOR:
		sprintf(tmp,"Color(1,1,1)\n");
		break;
	case TN_POINT:
		sprintf(tmp,"Z(0)\n");
		break;
	case TN_DENSITY:
		sprintf(tmp,"Density(noise(3,6))\n");
		break;
	case TN_NOISE:
		sprintf(tmp,"noise(3,2)\n");
		break;
	}
	return tmp[0]?1:0;
}

//-------------------------------------------------------------
// UniverseModel::getObjectSymbol() return object's dir symbol
//-------------------------------------------------------------
ModelSym* UniverseModel::getObjectSymbol(int type){
    switch(type&TN_TYPES){
	case TN_GALAXY:
		return new ModelSym("Galaxy",type);
	case TN_SYSTEM:
		return new ModelSym("System",type);
	case TN_STAR:
		return new ModelSym("Star",type);
	case TN_CORONA:
		return new ModelSym("Corona",type);
	case TN_PLANET:
		return new ModelSym("Planet",type);
	case TN_MOON:
		return new ModelSym("Moon",type);
	case TN_LAYER:
		//return new ModelSym("Layer",type);
	case TN_SURFACE:
		return new ModelSym("Surface",type);
	case TN_RING:
		return new ModelSym("Ring",type);
	case TN_SKY:
		return new ModelSym("Sky",type);
	case TN_CLOUDS:
		return new ModelSym("Clouds",type);
	case TN_TEXTURE:
		return new ModelSym("Texture",type);
	case TN_POINT:
		return new ModelSym("Point",type);
	case TN_DENSITY:
		return new ModelSym("Density",type);
	case TN_COLOR:
		return new ModelSym("Color",type);
	case TN_COMP:
		return new ModelSym("Group",type);
	case TN_MAP:
		return new ModelSym("Map",type);
	case TN_CRATERS:
		return new ModelSym("Craters",type);
	case TN_ROCKS:
		return new ModelSym("Rocks",type);
	case TN_FOG:
		return new ModelSym("Fog",type);
	case TN_WATER:
		return new ModelSym("Water",type);
	case TN_SNOW:
		return new ModelSym("Snow",type);
	case TN_CLOUD:
		return new ModelSym("Cloud",type);
	case TN_ERODE:
		return new ModelSym("Erode",type);
	case TN_GLOSS:
		return new ModelSym("gloss",type);
	case TN_BASE:
		return new ModelSym("Base",type);
	case TN_FCHNL:
		return new ModelSym("Fractal",type);
	case TN_NOISE:
		return new ModelSym("Noise",type);
	case TN_WAVES:
		return new ModelSym("Waves",type);
	}
	return 0;
}

//-------------------------------------------------------------
// UniverseModel::getReplaceList() return list of replacable objects
//-------------------------------------------------------------
int UniverseModel::getReplaceList(NodeIF *obj,LinkedList<ModelSym*>&list)
{
	int type=obj->getFlag(TN_TYPES);
	switch(type){
	case TN_LAYER:
		list.add(getObjectSymbol(TN_LAYER));
	case TN_SURFACE:
		list.add(getObjectSymbol(TN_SURFACE));
		break;
	default:
		getAddList(obj,list);
		//list.add(getObjectSymbol(type));
		break;
	}
	return list.size;
}

//-------------------------------------------------------------
// UniverseModel::getSaveList() return list of save directories
//-------------------------------------------------------------
int UniverseModel::getSaveList(NodeIF *obj,LinkedList<ModelSym*>&list)
{
	int type=obj->getFlag(TN_TYPES);
	switch(type){
	case TN_LAYER:
		list.add(getObjectSymbol(TN_LAYER));
	case TN_SURFACE:
		list.add(getObjectSymbol(TN_SURFACE));
		break;
	default:
		list.add(getObjectSymbol(type));
		break;
	}
	return list.size;
}

//-------------------------------------------------------------
// UniverseModel::getAddList() return list of addable objects
//-------------------------------------------------------------
int UniverseModel::getAddList(NodeIF *obj,LinkedList<ModelSym*>&list)
{
	int type=obj->getFlag(TN_TYPES);
	switch(type&TN_TYPES){
	case TN_SCENE:
	case TN_UNIVERSE:
		list.add(getObjectSymbol(TN_GALAXY));
		break;
	case TN_GALAXY:
		if(obj->expanded() )
			list.add(getObjectSymbol(TN_SYSTEM));
		else if(obj->hasChildren())
			list.add(getObjectSymbol(TN_GALAXY));
		else{
			list.add(getObjectSymbol(TN_GALAXY));
			list.add(getObjectSymbol(TN_SYSTEM));
		}
		break;
	case TN_SYSTEM:
		if(obj->collapsed() && obj->hasChildren() && obj->getParent())
			return getAddList(obj->getParent(),list);
		list.add(getObjectSymbol(TN_STAR));
		list.add(getObjectSymbol(TN_PLANET));
		break;
	case TN_CORONA:
	//case TN_RING:
		break;
	case TN_STAR:
		if(obj->collapsed() && obj->getParent()){
			return getAddList(obj->getParent(),list);
		}
		if(!obj->hasChild(ID_CORONA))
			list.add(getObjectSymbol(TN_CORONA));
		break;
	case TN_PLANET:
		if(obj->collapsed() && obj->getParent()){
			list.add(getObjectSymbol(TN_PLANET));
			//return getAddList(obj->getParent(),list);
		}
		else{
			if(!obj->hasChild(ID_SKY))
				list.add(getObjectSymbol(TN_SKY));
			list.add(getObjectSymbol(TN_CLOUDS));
			list.add(getObjectSymbol(TN_MOON));
			list.add(getObjectSymbol(TN_RING));
		}
		break;
	case TN_MOON:
		if(obj->collapsed()&& obj->hasChildren() && obj->getParent())
			return getAddList(obj->getParent(),list);
		if(!obj->hasChild(ID_SKY))
			list.add(getObjectSymbol(TN_SKY));
		list.add(getObjectSymbol(TN_CLOUDS));
		break;
	case TN_SKY:
	case TN_CLOUDS:
	case TN_RING:
		if(obj->collapsed() && obj->getParent())
			return getAddList(obj->getParent(),list);
		break;
	case TN_MAP:
		//if(obj->collapsed()&& obj->hasChildren() && obj->getParent())
		//	return getAddList(obj->getParent(),list);
		//list.add(getObjectSymbol(TN_LAYER));
		list.add(new ModelSym("Layer",TN_LAYER));
		break;
	case TN_LAYER:
		if(obj->collapsed())
			list.add(new ModelSym("Layer",TN_LAYER));
			//list.add(getObjectSymbol(TN_LAYER));
		else{
			if(!obj->hasChild(ID_POINT))
				list.add(getObjectSymbol(TN_POINT));
			if(!obj->hasChild(ID_COLOR))
				list.add(getObjectSymbol(TN_COLOR));
			if(!obj->hasChild(ID_GLOSS))
				list.add(getObjectSymbol(TN_GLOSS));
			list.add(getObjectSymbol(TN_TEXTURE));
			//list.add(getObjectSymbol(TN_BASE));
			//list.add(getObjectSymbol(TN_MAP));
			list.add(getObjectSymbol(TN_ROCKS));
			//list.add(getObjectSymbol(TN_FCHNL));
		}
		break;

	case TN_ROCKS:
		if(obj->collapsed()&& obj->hasChildren() && obj->getParent())
			return getAddList(obj->getParent(),list);
		else{
			if(!obj->hasChild(ID_POINT))
				list.add(getObjectSymbol(TN_POINT));
			if(!obj->hasChild(ID_COLOR))
				list.add(getObjectSymbol(TN_COLOR));
			if(!obj->hasChild(ID_GLOSS))
				list.add(getObjectSymbol(TN_GLOSS));
			list.add(getObjectSymbol(TN_TEXTURE));
		}
		break;

	case TN_SURFACE:
		if(obj->collapsed() && obj->hasChildren() && obj->getParent())
			return getAddList(obj->getParent(),list);
		if(!obj->hasChild(ID_POINT))
			list.add(getObjectSymbol(TN_POINT));
		if(!obj->hasChild(ID_COLOR))
			list.add(getObjectSymbol(TN_COLOR));
		if(!obj->hasChild(ID_GLOSS))
			list.add(getObjectSymbol(TN_GLOSS));
		list.add(getObjectSymbol(TN_TEXTURE));
		if(obj->hasChild(ID_CLOUD)){
			break;
		}
		if(!obj->hasChild(ID_FOG))
			list.add(getObjectSymbol(TN_FOG));
		if(!obj->hasChild(ID_WATER))
			list.add(getObjectSymbol(TN_WATER));
		if(!obj->hasChild(ID_SNOW))
		    list.add(getObjectSymbol(TN_SNOW));
		if(!obj->hasChild(ID_MAP))
			list.add(getObjectSymbol(TN_MAP));
		list.add(getObjectSymbol(TN_ROCKS));
	//	list.add(getObjectSymbol(TN_BASE));
	//	list.add(getObjectSymbol(TN_COMP));
	//	list.add(getObjectSymbol(TN_CRATERS));
	//	list.add(getObjectSymbol(TN_FCHNL));
	//	list.add(getObjectSymbol(TN_ERODE));
		break;
	case TN_TEXTURE:
		list.add(getObjectSymbol(TN_TEXTURE));
		break;
	}
	return list.size;
}

//-------------------------------------------------------------
// UniverseModel::buildTree() build default model
//-------------------------------------------------------------
TreeNode *UniverseModel::buildTree(NodeIF *node){
	TreeNode *root = new TreeNode(node);
	setType(node);
	LinkedList<NodeIF*>children;
	if(node->getChildren(children)){
		NodeIF *child;
		children.ss();
		while((child=children++)>0)
			addToTree(root,child);
	}
	return root;
}

//-------------------------------------------------------------
// UniverseModel::setType() set TN_TYPE
//-------------------------------------------------------------
void UniverseModel::setType(NodeIF *node)
{
	TerrainData td;
	int type=node->typeValue();
  	int compflag=0;
    node->clrAllFlags();
	switch(node->typeClass()){
	case ID_SCENE:
		node->setFlag(TN_SCENE);
		break;
	case ID_SCOPE:
		switch(type){
		case ID_TNMGR:
			node->setFlag(TN_SURFACE);
			node->setFlag(TN_HIDEFLAG);
			node->setFlag(TN_BRANCH);
			break;
		}
		break;
	case ID_ORBITAL:
		node->clrFlag(TN_HIDEFLAG);
		switch(type){
		case ID_UNIVERSE:
			node->setFlag(TN_UNIVERSE);
#ifdef HIDE_UNIVERSE
			node->setFlag(TN_HIDEFLAG);
			node->setFlag(NODE_HIDE);
#endif
			break;
		case ID_GALAXY:
			node->setFlag(TN_GALAXY);
			break;
		case ID_SYSTEM:
			node->setFlag(TN_SYSTEM);
			break;
		case ID_STAR:
			node->setFlag(TN_STAR);
			break;
		case ID_CORONA:
			node->setFlag(TN_CORONA);
			break;
		case ID_PLANET:
			node->setFlag(TN_PLANET);
			break;
		case ID_MOON:
			node->setFlag(TN_MOON);
			break;
		case ID_RING:
			node->setFlag(TN_RING);
			break;
		case ID_SKY:
			node->setFlag(TN_SKY);
			break;
		case ID_CLOUDS:
			node->setFlag(TN_CLOUDS);
			break;
		}
		break;

	case ID_TERRAIN:
		switch(type){
		case ID_ADD:
		case ID_SUBTRACT:
		case ID_MULTIPLY:
		case ID_DIVIDE:
		case ID_EXPR:
		case ID_CONST:
		case ID_GLOBAL:
			node->setFlag(TN_COMP|TN_HIDEFLAG);
			break;
		case ID_GROUP:
			node->setFlag(TN_COMP);
			node->setFlag(TN_HIDEFLAG);
			node->setFlag(TN_BRANCH);
			break;
		case ID_LAYER:
			node->setFlag(TN_LAYER);
			node->setFlag(TN_HIDEFLAG);
			break;
		case ID_MAP:
			node->setFlag(TN_MAP);
			node->setFlag(TN_BRANCH);
			break;
		case ID_FOG:
			node->setFlag(TN_FOG);
			break;
		case ID_CRATERS:
			node->setFlag(TN_CRATERS);
			break;
		case ID_ROCKS:
			node->setFlag(TN_ROCKS);
			node->setFlag(TN_HIDEFLAG);
			node->setFlag(TN_BRANCH);
			break;
		case ID_WATER:
			node->setFlag(TN_WATER);
			break;
		case ID_SNOW:
			node->setFlag(TN_SNOW);
			break;
		case ID_CLOUD:
			node->setFlag(TN_CLOUD);
			node->setFlag(TN_HIDEFLAG);
			node->setFlag(NODE_HIDE);
			break;
		case ID_ERODE:
			node->setFlag(TN_ERODE);
			break;
		case ID_GLOSS:
			node->setFlag(TN_GLOSS);
			break;
		case ID_ZBASE:
			node->setFlag(TN_BASE);
			break;
		case ID_FCHNL:
			node->setFlag(TN_FCHNL);
			break;
		case ID_ROOT:
			node->setFlag(NODE_HIDE);
			break;
		case ID_POINT:
			node->setFlag(TN_POINT);
			break;
		case ID_COLOR:
			node->setFlag(TN_COLOR);
			break;
		case ID_TEXTURE:
			{
				TNtexture *tex=(TNtexture *)node;
				tex->init();
				node->setFlag(TN_TEXTURE);
			}
			break;
		case ID_DENSITY:
			node->setFlag(TN_DENSITY);
			break;
		default:
			node->setFlag(NODE_COMP,compflag);
		}
	}
}
//-------------------------------------------------------------
// UniverseModel::replaceInTree() replace a tree node
//-------------------------------------------------------------
TreeNode *UniverseModel::replaceInTree(TreeNode *parent, TreeNode *item, NodeIF *node)
{
	setType(node);

	switch(parent->getFlag(TN_TYPES)){
	case TN_PLANET:
	case TN_MOON:
	case TN_SKY:
	case TN_CLOUDS:
	case TN_STAR:
	case TN_RING:
		if(item->getFlag(TN_TYPES)==TN_SURFACE){  // replacing surface
			item->children.free();
			addToTree(item,node);
			return item;
		}
		break;
	case TN_MAP:
		if(item->getFlag(TN_TYPES)==TN_LAYER){  //replacing layer
			item->children.free();
			addToTree(item,node);
			return item;
		}
		break;
	}
	return addToTree(parent, node);
}

//-------------------------------------------------------------
// UniverseModel::insertInTree() add tree node (add mode)
//-------------------------------------------------------------
TreeNode *UniverseModel::insertInTree(TreeNode *parent, NodeIF *node)
{
	switch(parent->getFlag(TN_TYPES)){
	case TN_SCENE:
	//	if(after && root->getFlag(TN_TYPES)!=TN_GALAXY)
	//		parent=after;
		break;
	case TN_GALAXY:
		//if(node->typeValue()==ID_SYSTEM)
		//	parent=after;
		break;
	case TN_SURFACE:
		if(node->typeValue()==ID_MAP){
			//parent->children.free();
			parent->setFlag(TN_INVALID);
		}
		break;
	case TN_ROCKS:
		node->setFlag(NODE_BRANCH);
		break;
	}
	return addToTree(parent, node);
}

//-------------------------------------------------------------
// UniverseModel::addToTree() add tree node (build mode)
//-------------------------------------------------------------
TreeNode *UniverseModel::addToTree(TreeNode *parent, NodeIF *node)
{
	return addToTree(parent,0,node);
}
//-------------------------------------------------------------
// UniverseModel::addToTree() add tree node (build mode)
//-------------------------------------------------------------
TreeNode *UniverseModel::addToTree(TreeNode *parent, TreeNode *child, NodeIF *node)
{
    int branch=node->getFlag(NODE_BRANCH);
	TreeNode *root=new TreeNode(node);
	setType(node);

	switch(parent->getFlag(TN_TYPES)){
	case TN_DENSITY:
	case TN_POINT:
	case TN_COLOR:
	case TN_CRATERS:
	case TN_TEXTURE:
	case TN_FOG:
	case TN_SNOW:
	case TN_ERODE:
	case TN_GLOSS:
	case TN_BASE:
	case TN_WATER:
	case TN_FCHNL:
		parent=parent->getParent();
		break;
	case TN_ROCKS:
	    if(!branch)
			parent=parent->getParent();
		break;
	case TN_CLOUD:
		parent=parent->getParent();
		break;
	}
	int ntype=node->getFlag(TN_TYPES);
	switch(ntype){
	case TN_SURFACE:
		switch(parent->getFlag(TN_TYPES)){
		case TN_CORONA:
		case TN_SKY:
			node->setFlag(NODE_HIDE);
			parent->setFlag(TN_HIDEFLAG);
			parent=parent->getParent();
			break;
		case TN_RING:
			node->setFlag(NODE_HIDE);
			parent->setFlag(TN_HIDEFLAG);
			break;
		}
		break;
	case TN_COMP:
	    if(!node->getFlag(TN_BRANCH))
			node->setFlag(NODE_HIDE,getFlag(parent,TN_HIDEFLAG));
		root->setName("group");
	    break;
	case TN_POINT:
		root->setName("point");
		break;
	case TN_GLOSS:
		root->setName("shine");
		break;
	case TN_BASE:
		root->setName("base");
		break;
	case TN_DENSITY:
		root->setName("density");
		break;
	case TN_COLOR:
		root->setName("color");
		break;
	case TN_TEXTURE:
		root->setName("texture");
//		if(parent->getFlag(TN_TYPES)==TN_RING)
//			node->setFlag(NODE_HIDE);
		break;
	case TN_LAYER:
    	if(parent->node->getFlag(TN_TYPES)!=TN_MAP)
  			parent=parent->getParent();
		break;
	}

	if(parent && node->getFlag(NODE_HIDE)){
        delete root;
        root=parent;
    }
    else{
 	    root->setParent(parent);
 	    if(child)
 	    	parent->addAfter(child, root); // insert after child
 	   // else if(parent->node->expanded())
        //	parent->addAfter(0, root);     // insert as first child
		else
        	parent->addChild(root);        // add as last child
    }

	if(!node->getFlag(NODE_COMP)){
		LinkedList<NodeIF*>children;
		if(node->getChildren(children)){
			NodeIF *child;
			children.ss();
			while((child=children++)>0)
				addToTree(root,child);
		}
	}
    return root;
}

//-------------------------------------------------------------
// UniverseModel::lastBranch() return first parent with TN_BRANCH
//-------------------------------------------------------------
TreeNode *UniverseModel::lastBranch(TreeNode *n)
{
    TreeNode *tn=n->getParent();
    while(tn && tn->node && !tn->node->getFlag(TN_BRANCH))
    	tn=tn->getParent();
    return tn;
}

//-------------------------------------------------------------
// UniverseModel::make() build default model
//-------------------------------------------------------------
void UniverseModel::make(FILE *fp)
{
	fprintf(fp,"%s\n",default_model);
}

//-------------------------------------------------------------
// UniverseModel::make() build default libraries
//-------------------------------------------------------------
void UniverseModel::make_libraries(int i)
{
	saveLibraryItem(TN_NOISE, "rounded", "noise(GRADIENT,11,4,0.5,0.5)", i);
	saveLibraryItem(TN_NOISE, "ridged", "noise(RIDGED,11,4,0.5,0.5)", i);
	saveLibraryItem(TN_NOISE, "eroded", "noise(ERODED,11,4,0.5,0.5)", i);
	saveLibraryItem(TN_WAVES, "ripples", "0.01*noise(SCALE,22,2,0.7)", i);
	saveLibraryItem(TN_WAVES, "choppy", "0.02*noise(RIDGED|SCALE,15,6,0.4,0.2)", i);
	saveLibraryItem(TN_WAVES, "seawaves", "0.01*noise(RIDGED|SCALE,16,6,0.4,0.2)+0.01*noise(SCALE,22,2,0.7)", i);
	saveLibraryItem(TN_CRATERS, "volcanos", "craters(ID1,craters(2,1.3,0.25,1,1,0.5,0.5,1,5,0.1,0.2,0,0)", i);
	saveLibraryItem(TN_CRATERS, "craters", "craters(ID1,4,0.2,0.05,1,1,1,0.8)", i);
}

//-------------------------------------------------------------
// UniverseModel::parse_node() parse a node string
//-------------------------------------------------------------
NodeIF *UniverseModel::parse_node(NodeIF *parent, char *s)
{
#ifdef HIDE_UNIVERSE
	if(parent && parent->typeClass() == ID_SCENE){
		LinkedList<NodeIF*> list;
		parent->getChildren(list);
		parent=list[0];
	}
#endif
	set_orbital(0);

	NodeIF *node=Model::parse_node(parent,s);

	if(node && node->typeValue() & ID_OBJECT){
		((Object3D*)node)->visitAll(&Object3D::init);
	}
	return node;
}

//-------------------------------------------------------------
// UniverseModel::parse_node() parse a node string
//-------------------------------------------------------------
NodeIF *UniverseModel::open_node(NodeIF *parent,char *fn)
{
#ifdef HIDE_UNIVERSE
	if(parent && parent->typeClass() == ID_SCENE){
		LinkedList<NodeIF*> list;
		parent->getChildren(list);
		parent=list[0];
	}
#endif
	set_orbital(0);

	NodeIF *node=Model::open_node(parent,fn);
	if(node && node->typeValue() & ID_OBJECT){
		((Object3D*)node)->visitAll(&Object3D::init);
	}
	return node;
}

//************************************************************
// default systems
//************************************************************
char UniverseModel::default_galaxy[]=""
" Scene() {\n"
" 	View{\n"
"		view=GLOBAL;\n"
"       theta=0;\n"
"       radius=0;\n"
"       height=0;\n"
"       pitch=-90;\n"
"       heading=90;\n"
"       phi=0;\n"
"		gndlvl=0 ft;\n"
"		speed=0.1 ly;\n"
" 		time=1;\n"
"		origin=Point(0,0,0);\n"
"		viewobj=Universe.1.Galaxy.1;\n"
" 	}\n"
" 	Universe {\n"
" 		Galaxy(20000) {\n"
"			name=\"Uncharted\";\n"
"			origin=Point(0,0,0);\n"
"		}\n"
"	}\n"
"}\n";

// this model can be build without any resource (object) files
// - created if vtx is called without a file argument
// - #define INIT_XXX above to set initial view to desired object (defaults to Sol System)
char UniverseModel::default_model[]=""
" Scene() {	\n"
"	View { \n"
"		time=20000; \n"
#if INIT_MODE == INIT_GLOBAL
"		view=GLOBAL; \n"
"       origin=Point(18000 ly,0.01,0.01);\n"
#else
"		view=ORBITAL;\n"
"		heading=90;\n"
"		pitch=-90;\n"
"		gndlvl=0 ft;\n"
"		phi=0;\n"
"	    theta=300.00;\n"
#if INIT_MODE == INIT_SYSTEM1
"		height=30;\n"
"		viewobj=Universe.1.Galaxy.1.System.2;\n"
#elif INIT_MODE == INIT_SYSTEM2
"		height=30 miles;\n"
"		viewobj=Universe.1.Galaxy.1.System.1;\n"
#elif INIT_MODE == INIT_STAR1
"		radius=1.11;\n"
"		height=4;\n"
"		viewobj=Universe.1.Galaxy.1.System.2.Star.1;\n"
#elif INIT_MODE == INIT_STAR2
"		radius=2;\n"
"		height=4;\n"
"		viewobj=Universe.1.Galaxy.1.System.1.Star.2;\n"
#elif INIT_MODE == INIT_MERCURY
"		radius=0.0015;\n"
"		height=5000 miles;\n"
"	    theta=302.554;\n"
"    	phi=5;\n"
"		viewobj=Universe.1.Galaxy.1.System.2.Planet.1;\n"
#elif INIT_MODE == INIT_VENUS
"		radius=0.003;\n"
"		height=5000 miles;\n"
"		gndlvl=5872 ft;\n"
"	    theta=145;\n"
"    	phi=0;\n"
"		viewobj=Universe.1.Galaxy.1.System.2.Planet.2;\n"
#elif INIT_MODE == INIT_EARTH
#ifdef INIT_SURFACE
"		view=SURFACE;\n"
"		tilt=15;\n"
"		height=3339.85 ft;\n"
#else
"		height=5000 miles;\n"
#endif
"		time=230125;\n"
"		phi=18.031;\n"
"		theta=49.15;\n"
"		radius=0.0036;\n"
"		gndlvl=800 ft;\n"
"		viewobj=Universe.1.Galaxy.1.System.2.Planet.3;\n"
#elif INIT_MODE == INIT_JUPITER
"		radius=0.04;\n"
"		height=70000 miles;\n"
"		viewobj=Universe.1.Galaxy.1.System.2.Planet.5;\n"
#elif INIT_MODE == INIT_SATURN
"		theta=180;\n"
"		phi=15;\n"
"		radius=0.034;\n"
"		height=80000 miles;\n"
"		viewobj=Universe.1.Galaxy.1.System.2.Planet.6;\n"
#elif INIT_MODE == INIT_TITAN
"		view=SURFACE;\n"
"		phi=21.84;\n"
"		theta=68.6;\n"
"		heading=0;\n"
"		time=29830;\n"
"		tilt=15;\n"
"		radius=0.002;\n"
"		height=7363.53 ft;\n"
"		gndlvl=1145.82 ft;\n"
"		viewobj=Universe.1.Galaxy.1.System.2.Planet.6.Moon.1;\n"
#endif
#endif
"	} \n"
"   Universe { \n"
"   Galaxy(20000){ \n"
"     name=\"Milky Way\";\n"
"     origin=Point(20000,0,0);\n"
"	  noise.expr=noise(NLOD,2,7,1,0.2)+0.1*noise(NLOD,14,5,0.5);\n"
"	  System(1000) {\n"
"       name=\"Centari\";\n"
"       origin=Point(-2004,0, 2);\n"
"		Star(0.5,10) {\n"
"           name=\"Alpha Centari\";\n"
"           color=Color(1,1,0.7);\n"
"           symmetry=0.95;\n"
"           day=100;\n"
"           year=100;\n"
"           Surface {\n"
"				mc=0.2*noise(UNS,3,6,0.2)*noise(UNS,8,6,0.3);\n"
"				surface=Color(1,1-mc,0.7-mc);\n"
"			}\n"
"			Corona(3) {\n"
"				gradient=0.7;\n"
"				color1=Color(1,0,0,0.2);\n"
"				color2=Color(1,1,0.9,0.9);\n"
"			}\n"
"		}\n"
"		Star(2,10) {\n"
"           name=\"Beta Centari\";\n"
"           color=Color(0.7,0.8,1);\n"
"           symmetry=0.95;\n"
"           day=33;\n"
"           year=100;\n"
"           phase=180;\n"
"           Surface {\n"
"				mc=0.5*noise(UNS,3,6,0.2)*noise(UNS,8,6,0.3);\n"
"				surface=Color(0.7-mc,0.7-mc,1);\n"
"			}\n"
"			Corona(5) {\n"
"				color1=Color(0.7,1,1,0.9);\n"
"				color2=Color(0.9,1,1,0.7);\n"
"			}\n"
"		}\n"
"	  }\n"
"	  System(1000) {\n"
"       name=\"Solar\";\n"
"       origin=Point(-2000,0, 0);\n"
"		Star(1.5,0.01) {\n"
"           name=\"Sol\";\n"
"           color=Color(1,1,0.8);\n"
"           day=100;\n"
"			bands(\"star1\",ACHNL|CLAMP,16,Color(0.976,0.945,0.565),\n"
"				Color(1,0.769,0.122),\n"
"				Color(1,0.353,0.275),\n"
"				Color(0.702,0.361,0.016),\n"
"				Color(0.467,0,0),\n"
"				Color(0.294,0.016,0.071),\n"
"				Color(0.137,0.024,0.188),\n"
"				Color(0.027,0.02,0.141),\n"
"				Color(0.008,0,0));\n"
"           Surface {\n"
"				terrain=\n"
"					Texture(\"star1\",BORDER|S,noise(RIDGED|FS,1,10,0,0.4),1.6922,1.3212,1.05456,0.0616,2,2);\n"
"			}\n"
"			Corona(8.11) {\n"
"				color1=Color(1,1,0.8,0.8);\n"
"				color2=Color(1,1,0.7,1);\n"
"			}\n"
"		}\n"
"		Planet(0.0015,36) {\n"
"			ambient=Color(0.94,0.95,0.95,0);\n"
"			day=58.65;\n"
"			diffuse=Color(0.87,0.89,0.99);\n"
"			name=\"Mercury\";\n"
"			shadow=Color(0,0,0,0.95);\n"
"			shine=8.77663;\n"
"			specular=Color(1,0.96,0.86);\n"
"			sunset=0.01;\n"
"			year=87.97;\n"
"			albedo=0.73;\n"
"			image(\"ridges\",INVT|NORM,256,256,noise(ERODED|UNS,0,5,-0.3,0.1,2.22,1.5,1,0,-0.4));\n"
"			image(\"craters3\",NORM,256,256,craters(ID1,2,1.3,0.9,0.9,0.9,1,1,1,0.9,0.5,0.75,0.5,0.2));\n"
"			bands(\"mercury\",ACHNL|CLAMP,64,0.35186,0.31482,Color(1,0.502,0.251,0.314),Color(0.875,0.875,0.875),\n"
"				Color(0.502,0.502,0.502),\n"
"				Color(0.063,0.063,0.063),\n"
"				Color(0.502,0.502,0.502));\n"
"			Surface{\n"
"				terrain=Z(craters(ID1,4,0.2,0.05,1,0.75,1,0.8,1,0.5,1,0.8,0.6,0.2)\n"
"						noise(UNS|RO1,1,5,0.5,0.2))+\n"
"						Texture(\"ridges\",BUMP|NTEX,34.2968,-1.32072,1,0.67696,10,2.20752,0.92452,0)+\n"
"						Texture(\"craters3\",BUMP|NTEX|S,noise(2.8,4.6,0.06,0.3,2,0.05,1,0.41,0.2),1.92319,0.94336,2,0.30768,4,2.7,1,0)+\n"
"						Texture(\"Shist1\",BORDER|BUMP,6822.1,0.66664,2,0,7.33984,2.13208,1,0)+\n"
"						Texture(\"mercury\",BORDER|LINEAR|S,BMPHT,1,0.90568,0.32728,0.6792,1,2,1,0);\n"
"			}\n"
"		}\n"
"		Planet(0.003,50) {\n"
"			day=30;\n"
"			diffuse=Color(1,1,1,0.5);\n"
"			name=\"Venus\";\n"
"			resolution=4;\n"
"			shine=1.37856;\n"
"			specular=Color(1,1,1,0.42);\n"
"			tilt=10;\n"
"			year=100;\n"
"			albedo=0.1;\n"
"			Surface{\n"
"				terrain=craters(ID1,2,0.1,0.1,1,1,0.8)\n"
"						Z(0.1*noise(ERODED,11,4,0.5,0.5))+Color(noise(UNS,2.4,2.8,0,1,2,1,1,0,0),0.8*RED,0.3*RED,1);\n"
"			}\n"
"			Clouds(0.0031) {\n"
"				crot=1;\n"
"				day=30;\n"
"				diffuse=Color(1,1,1,0.59);\n"
"				resolution=4;\n"
"				shine=20;\n"
"				specular=Color(1,1,1);\n"
"				albedo=0.1;\n"
"				bands(\"yellow-white\",ACHNL|NORM,64,Color(1,1,0.502),Color(1,1,0.235),Color(1,0.58,0.157),Color(1,0.502,0),\n"
"					Color(0.502,0.251,0),\n"
"					Color(0.141,0,0));\n"
"				Surface{\n"
"					terrain=clouds()\n"
"							Texture(\"yellow-white\",BORDER|BUMP|S,twist(-0.3,noise(ERODED|FS|UNS,0,5,0,0.47,1.87,1,0,0,0)+noise(FS,2.8,5,1,0.6,2,0.17,0.13,0,0)),0.5,0.005,1,2.96768,1,1.58064,1,0);\n"
"				}\n"
"			}\n"
"		}\n"
"		Planet(0.0036,85) {  \n"
"		    day=23.934;\n"
"		    name=\"Earth\";\n"
"		    phase=50;\n"
"		    shadow=Color(0,0,0,0.6);\n"
"		    tilt=23.45;\n"
"		    water.clarity=200 ft;\n"
"		    water.color1=Color(0,0.7,1,0.2);\n"
"		    water.color2=Color(0,0,0.6);\n"
"		    water.level=1000 ft;\n"
"		    water.reflectivity=0.7;\n"
"		    fog.vmin=water.level;\n"
"		    fog.vmax=fog.vmin+200 ft;\n"
"		    fog.max=1200 ft;\n"
"		    fog.min=100 ft;\n"
"		    fog.value=0.94902;\n"
"		    year=365.26;\n"
"		    bands(\"strata1\",8,0.1,Color(1,0.9,0.5),0.2,RAND);\n"
"		    bands(\"strata2\",64,0.3,Color(0.2,0.4,0.2),0.1,RAND);\n"
"		    bands(\"strata3\",128,0.2,Color(0.8,0.7,0.2),0.3,RAND);\n"
"		    bands(\"strata4\",64,0.5,Color(0.5,0.5,0.5),0.2,RAND);\n"
"		    bands(\"lava\",64,0.1,0.2,Color(0.1,0.1,0.2,0.4),Color(0.1,0.1,0.1,0.5));\n"
"			image(\"eroded\",TILE|BUMP|NORM,256,256,noise(ERODED|UNS,1,5,0.6,0.1));\n"
"		    bands(\"grass\",CLAMP,32,0.1,0.1,RAND,Color(0,0.3,0.1,1),\n"
"		        Color(0.2,0.4,0.1,1),\n"
"		        Color(0.3,0.5,0,1),\n"
"		        Color(0.4,0.5,0,1),\n"
"		        Color(0.4,0.5,0,0));\n"
"		    bands(\"alpine\",CLAMP,16,0.1,0.1,RAND,Color(0,0.3,0.1,1),\n"
"		        Color(0.2,0.4,0.1,0.7),\n"
"		        Color(0.3,0.5,0,0.7),\n"
"		        Color(0.4,0.5,0,0.7),\n"
"		        Color(0.4,0.4,0.1,0.7),\n"
"		        Color(0.2,0.3,0.3,0.7),\n"
"		        Color(0.3,0.4,0.2,0.7),\n"
"		        Color(0.4,0.5,0,0.7),\n"
"		        Color(0.4,0.5,0,0));\n"
"		    Surface{\n"
"		        env=0.3*(1+0.5*noise(SQR|UNS,15,3,0.3,0.1))*(1+0.5*LAT)*(1+0.2*SLOPE);\n"
"		        terrain=fog(noise(SQR|UNS,8,5,0.5)*(1-DEPTH))\n"
"		        		water(0.01*noise(RIDGED|SCALE,16,6,0.4,0.2)+0.01*noise(SCALE,22,2,0.7))\n"
"		        		snow(0.8,0.25,0.5,0.25)\n"
"		        		map(noise(1,15,0.3,0.1)*(1+2*LAT),-0.6,2)\n"
"		        		layer(MORPH,0,0.5-0.3*noise(RO1,3,7,0.4,0.2))[\n"
"		        			rocks(2,0.0001,0.73,1,0.13,0.25,0.2,noise(ERODED,0,6,0.5,0.5,2))[\n"
"								Texture(\"eroded\",BUMP,100000,1)\n"
"								+gloss(3.4,2.5,1)\n"
"								+Color(0,1,0.72,1)\n"
"							]\n"
"							Z(0.2*noise(RIDGED|SQR|UNS,9,15,0.5,0.5))\n"
"		        			+Texture(\"strata1\",S,noise(9,15,0.4,0.5))\n"
"		        			+Color(0.9,0.9,0.7)\n"
"		        			]\n"
"		        		layer(MORPH,0,0.2-0.4*noise(RO2,2,8,0.2,0.1),0.3)\n"
"		        			[Z(0.2*noise(NABS|SQR|UNS,6,18,0.5,0.5))\n"
"		        			+Texture(\"grass\",CLAMP,(0.1+HT)*env)\n"
"		        			+gloss(0.5)\n"
"		        			]\n"
"						layer(MORPH,0,0.1+0.2*noise(RO3,3,7,0.3,0.1))\n"
"		        			[Z(0.6*craters(MAXHT,2,0.004,0.2,1,1,1,0.5,0.5,5,0.1,0.2,0,0))\n"
"		        			+Texture(\"lava\",S,HT*(1-0.1*SLOPE))\n"
"		        			+Color(noise(1,15,0.3,0.1)+0.2*noise(RO2,2,8,0.2,0.1)+0.2,RED,RED)\n"
"		        			+gloss(1.5)\n"
"		        			]\n"
"		        		layer(MORPH,0,0.2-0.5*noise(RO1,3,7,0.4,0.2))\n"
"		        			[Point(0.6*noise(SQR|RO2,8,12,0.5,0.4),0,0.4*noise(RIDGED,8,14,0.2,0.5)+0.7*PX)\n"
"		        			+Texture(\"strata3\",S,HT+0.3*noise(9,15,0.4,0.5))\n"
"		        			+Color(1,1-0.5*abs(2*PX),GREEN-0.3)\n"
"		        			+Texture(\"alpine\",CLAMP,(0.1+HT)*env)\n"
"		        			]\n"
"		        		layer(MORPH,0,0.3-0.4*noise(RO3,3,7,0.3,0.1))\n"
"		        			[Z(0.14*noise(SCALE|SS,18,5,1,0.5,2)+0.3*fractal(SS|SQR,14,18,0.15,2,3,0.74,0.32)noise(UNS,10,4,0.3,0.5,2))\n"
"		        			+Texture(\"strata4\",S,5*HT+0.5*noise(RIDGED|SQR|UNS,9,15,0.5,0.5))\n"
"		        			+Texture(\"alpine\",CLAMP,(0.1+HT)*env)\n"
"		        			+Color(1-noise(1,15,0.3,0.1)+0.2*noise(RO2,2,8,0.2,0.1)+0.2,RED,RED)\n"
"		        			]\n"
"		        		layer(MORPH,0)\n"
"		        			[Z(0.1*noise(RIDGED|SQR|UNS,9,15,0.5,0.5)+0.1)\n"
"		        			+Texture(\"strata1\",S,HT)\n"
"		        			+Texture(\"alpine\",CLAMP,(0.1+HT)*env)\n"
"		        			+Color(1,1,0.8)\n"
"		        			]\n"
"		          ;\n"
"			}\n"
"			Clouds(0.0036+10 miles) {\n"
"				albedo=0.8;\n"
"				shine=1;\n"
"				tilt=10;\n"
"				day=20;\n"
"				Surface {\n"
"					density=noise(NXY,2,10,0.9,0.2,2,1-LAT-0.1*noise(2,9,1,1,2));\n"
"					surface=Color(1-0.1*density,RED,RED,density+0.5);\n"
"				}\n"
"			}\n"
"			Sky(0.0036+100 miles) {\n"
"				haze.color=Color(1,0.9,0.8);\n"
"				haze.max=100 miles;\n"
"				haze.min=50 miles;\n"
"				haze.value=1;\n"
"				twilight=Color(0.9,0.3,0.7,0.9);\n"
"				albedo=0.6;\n"
"				density=0.05;\n"
"				color=Color(0.67,1,1,0.95);\n"
"			}\n"
"			Moon(0.001,0.1) {\n"
"               name=\"Luna\";\n"
"				color=Color(1,1,1);\n"
"               phase=100;\n"
"               year=29.5;\n"
"               day=29.5;\n"
"				shadow=Color(0,0,0,0.9);\n"
"               Surface {\n"
"                   surface=craters(4,0.3,0.2,0.9)\n"
"					        Z(0.5*noise(UNS,1,6,0.7)*noise(10,4))\n"
"					        +Color(1-PZ,RED,RED);\n"
"               }\n"
"           }\n"
"		}\n"
"		Planet(0.002,120) {\n"
"			day=30;\n"
"			fog.color=Color(1,0.7,0.2);\n"
"			fog.value=1;\n"
"			fog.vmax=5000 ft;\n"
"			fog.vmin=1000 ft;\n"
"			name=\"Mars\";\n"
"			phase=104.075;\n"
"			year=400;\n"
"			image(\"mars1\",TILE,256,256,Color(0.2+noise(NABS|NEG|SQR|UNS,0,9,0.96,0.5,2.1),0.5*RED,0.2*RED));\n"
"			image(\"mars2\",TILE,256,256,Color(0.2+noise(NABS|NEG|SQR|UNS,0,9,0.96,0.5,2.1),0.5*RED,0.2*RED,0.7));\n"
"			image(\"cracks\",TILE|BUMP|INVT,256,256,noise(RIDGED,1,5,0.3));\n"
"			image(\"eroded\",TILE|BUMP|NORM,256,256,noise(ERODED|UNS,1,5,0.6,0.1));\n"
"			Surface{\n"
"				b1=0.1*noise(UNS,3,9,1,0.5,2);\n"
"				terrain=snow(0.75,0.25,0.5,0.25)\n"
"					fog(1-1.2*noise(UNS,7,8,0.61,0.5,2))\n"
"					map(noise(UNS,2,6,1,0.5,2),0,1,0.5)\n"
"					layer(MORPH,0,0.51+noise(RO1,1,3,0.2,0.2,2),0.5,0,0)\n"
"						[craters(7,0.3,0.2,0.5,0.5,1,1,1,0.3,1,0.8,0.6,0.2)\n"
"						Z(noise(UNS|RO1,1,5,0.5,0.2)+0.2*noise(RIDGED,13,4,0.5,0.2))\n"
"						+Color(0.7,RED,RED)\n"
"						+Texture(\"mars1\",S,2)\n"
"						+Texture(\"cracks\",BUMP,40,0.2)\n"
"						]\n"
"					layer(MORPH,0,0.36,0.5,0,0)\n"
"						[Z(0.1*noise(RIDGED,10,11,0.5,0.4,2)+b1)\n"
"						+Color(1,0.75,0.0039,1)\n"
"						+Texture(\"mars2\",S,2)\n"
"						]\n"
"					layer(MORPH,0,0.1)\n"
"						[rocks(1,1e-005,0.8,0.31,0.36,0.1,0.33,noise(0,2))[Color(1,0.5,0.25,1)\n"
"						+Texture(\"mars2\",200000)\n"
"						+Texture(\"cracks\",BUMP,600000,0.26)\n"
"						+Z(b1)\n"
"						]\n"
"						Color(1,0.5,0,1)\n"
"						+Z(b1+0.49*noise(ERODED|SCALE,16,8,0.3,0.5,2))\n"
"						+Texture(\"mars2\",2)\n"
"						+Texture(\"eroded\",BUMP,200000,0.66)\n"
"						]\n"
"					;\n"
"			}\n"
"			Sky(0.00212088) {\n"
"				color=Color(0.01,0.69,0.99);\n"
"				density=0.12;\n"
"				emission=Color(1,1,1,0);\n"
"				twilight=Color(0.5,0,0.25);\n"
"				twilight.max=0.2958;\n"
"				twilight.min=-0.2958;\n"
"				haze.color=Color(1,1,0.5);\n"
"				haze.max=50 miles;\n"
"				haze.min=2 miles;\n"
"				haze.value=1;\n"
"			}\n"
"		}\n"
"		Planet(0.04,442) {\n"
"           name=\"Jupiter\";\n"
"			year=1000;\n"
"			day=12;\n"
"			phase=140;\n"
"			tilt=3;\n"
"			bands(\"jupiter\",64,0.2,Color(1.0,0.9,0.5),0.6,Color(1.0,0.3,0.1));\n"
"			Surface{\n"
"				surface=Texture(\"jupiter\",REPEAT,PHI+0.005*noise(3,17))\n"
"				        +Color(1,1,0.6)\n"
"				        +Z(noise(6,4));\n"
"			}\n"
"			Sky(0.0401) {\n"
"				density=0.1;\n"
"				color=Color(0.0,0.9,0.9,0.3);\n"
"			}\n"
"       }\n"
"		Planet(0.034,803) {\n"
"           name=\"Saturn\";\n"
"			year=600;\n"
"			day=20;\n"
"			tilt=25;\n"
"			phase=60;\n"
"			shadow=Color(0.2,0.2,0,0.5);\n"
"			c1=Color(1.0,0.9,0.3);\n"
"			c2=Color(1.0,0.6,0.0);\n"
"			c3=Color(0.7,0.4,0.0);\n"
"			bands(\"saturn\",64,0.5,0.3,RAND,c1,c2,c3,c1);\n"
"			Surface{\n"
"				surface=Texture(\"saturn\",REPEAT,2*PHI+0.01*noise(3,17));\n"
"			}\n"
"			Sky(0.0342) {\n"
"				density=0.1;\n"
"				color=Color(0.0,0.9,0.9,0.3);\n"
"			}\n"
"			Ring(0.045,0.013) {\n"
"				bands(\"rings\",64,0.5,0.6,RAND,Color(0.9,0.9,0.5,0.4));\n"
"				emission=Color(1,1,1,0.8);\n"
"				terrain=Texture(\"rings\",REPEAT,PHI);\n"
"			}\n"
"			Ring(0.060,0.010) {\n"
"				bands(\"rings\",64,0.5,0.5,RAND,Color(0.9,0.6,0.5,0.4),Color(1,1,0.3,0.5));\n"
"				emission=Color(1,1,1,0.8);\n"
"				terrain=Texture(\"rings\",REPEAT,PHI);\n"
"			}\n"
"			Moon(0.002,0.10) {\n"
"               name=\"Titan\";\n"
"				phase=155;\n"
"				tilt=-10;\n"
"				year=17;\n"
"				day=11;\n"
"				fog.color=Color(0.8,0.0,0.0,0.6);\n"
"				fog.max=5000 ft;\n"
"				fog.min=1000 ft;\n"
"				fog.vmin=600 ft;\n"
"				fog.vmax=800 ft;\n"
"				shadow=Color(0,0,0,0.7);\n"
"				bands(\"tmap1\",32,0.1,Color(0.5,0.5,0.5),0.5,RAND);\n"
"				image(\"bmap1\",TILE|BUMP,1,256,noise(RIDGED,3,3,0.5,0.5));\n"
"               Surface {\n"
"						zf=fractal(SS|SQR,14,9,0.2,2,2,0.6,1)craters(1,0.1,0.1,1,0.9);\n"
"						surface=fog(1-1.5*noise(UNS,7,4,1))\n"
"                               map(noise(UNS,0,9,1))\n"
"							    layer(MORPH,0,0.2)\n"
"							        [ Z(zf+0.3*noise(8,4,0.4))\n"
"							         +Texture(\"tmap1\",REPEAT,HT+0.5*noise(7,7,0.6))\n"
"							         +Texture(\"bmap1\",BUMP,HT+0.1*noise(8,7,0.6),2,0.1)\n"
"                                   ]\n"
"							    layer(MORPH)\n"
"							        [Z(0.1*noise(RIDGED,10,5,0.5,0.5))\n"
"							         +Color(1,1-10*PZ,0.2)\n"
"							        ];\n"
"               }\n"
"				Clouds(0.00201) {\n"
"					tilt=-8;\n"
"					day=10;\n"
"					gmax=2;\n"
"					Surface {\n"
"						surface=Color(1-0.9*noise(UNS,3,8),0.6*RED,0.2,RED);\n"
"					}\n"
"				}\n"
"				Sky(0.00202) {\n"
"					twilight=Color(0.5,0.3,0.1,0.4);\n"
"					density=0.3;\n"
"					color=Color(0.5,0.1,0,0.9);\n"
"					haze.color=Color(0.4,0,0.3);\n"
"					haze.max=300 miles;\n"
"					haze.min=50 miles;\n"
"				}\n"
"			}\n"
"		}\n"
"    }\n"
"  }\n"
" }\n"
"}\n";
