// ModelClass.cpp
#define HIDE_UNIVERSE

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
	if(child->protoValid())
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
		if(!dropping()){
			((Planetoid*)child)->orbit_radius=psize*(50+10*SRand());
			((Planetoid*)child)->orbit_phase=360*Rand();
		}
		break;
	case TN_CORONA:
		if(!dropping()){
			((Corona*)child)->size=psize*6;
			((Corona*)child)->size*=1+0.25*SRand();
		}			
		((Corona*)child)->ht=((Corona*)child)->size-psize;

		break;
	case TN_HALO:
		if(!dropping()){
			((Halo*)child)->size=psize*1.01;
		}
		((Halo*)child)->ht=((Halo*)child)->size-psize;

		break;
	case TN_CLOUDS:
		if(!dropping())
			((CloudLayer*)child)->size=psize*(1+0.01*(1+Rand()));
		((CloudLayer*)child)->ht=((CloudLayer*)child)->size-psize;
		break;
	case TN_SKY:
		if(!dropping())
			((Sky*)child)->size=psize*(1+0.02*(1+0.5*Rand()));
		((Sky*)child)->ht=((Sky*)child)->size-psize;
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
	if(child && !dropping()){
		if(((Orbital*)child)->isRandom())
			//((Orbital*)child)->setRseed(getRandValue());
		((Orbital*)child)->setRseed(Rand());
	}
	child->setProtoValid(true);
	return 1;
}

int UniverseModel::getPrototype(int type,char *tmp)
{
	tmp[0]=0;
	char buff[MAXSTR];
	static uint stype=0;
	int ttype=type&TN_TYPES;
	int gtype=type&GN_TYPES;
	//cout<<"getPrototype t:"<<ttype<<" g:"<<gtype<<endl;

	switch(ttype){
	case TN_GALAXY:
		sprintf(tmp,"Galaxy(10000) {}\n");
		break;
	case TN_SYSTEM:
		sprintf(tmp,"System(1000) {Star(0.5) {Corona(4){}}}\n");
		break;
	case TN_STAR:
		sprintf(tmp,"Star(1) {Surface{terrain=0;}Halo(1.01){}Corona(1.1){}Corona(4){}}\n");
		break;
	case TN_CORONA:
		sprintf(tmp,"Corona(4) {}\n");
		break;
	case TN_HALO:
		sprintf(tmp,"Halo(1) {}\n");
		break;
	case TN_PLANET:
		sprintf(tmp,"Planet(0.005,40){day=24;year=100;Surface{}}\n");
		break;
	case TN_MOON:
		sprintf(tmp,"Moon(0.001,0.05){day=30;year=30;Surface{}}\n");
		break;
	case TN_SKY:
		sprintf(tmp,"Sky(0.1){color=Color(0.5,0.9,1.0);density=0.2;}\n");
		break;
	case TN_CLOUDS:
		sprintf(tmp,"Clouds(0.01){emission=Color(1,1,1,0.05);Surface{terrain=Color(1,1,1,noise(1,5));\n}}\n");
		break;
	case TN_RING:
		sprintf(tmp,"Ring(1.5,0.2){emission=Color(1,1,1,0.64);}\n");
		break;
	case TN_COMP:
		sprintf(tmp,"()\n");
		break;
	case TN_TEXTURE:
		sprintf(tmp,"Texture(\"jupiter\",TEX,1,2,1,0,1,2,0.5,0)\n");
		break;
	case TN_SPRITE:
		sprintf(tmp,"Sprite(\"firs2x2\",FLIP|NOLOD,1,1e-6,1,1,1,0,0,0)\n");
		break;
	case TN_PLANT:
		{
		char bstr1[256];
		char bstr2[256];
		char lstr[256];
		stype=0;
		getPrototype(TN_PLANT_BRANCH,bstr1);
		stype=1;
		getPrototype(TN_PLANT_BRANCH,bstr2);
		getPrototype(TN_PLANT_LEAF,lstr);
		sprintf(tmp,"Plant(1,1e-06)%s%s%s",bstr1,bstr2,lstr);
		}
		break;
	case TN_PLANT_BRANCH:
		if(stype==0)
			sprintf(tmp,"Branch(12,1,1,1,1)[\"Bark1\",Color(0.5,0.4,0.1,LVL)]\n");
		else
			sprintf(tmp,"Branch(6,2,2,0.5,1,1,0,0.6,0.8,2)[\"Bark1\",Color(0.1,0.4,0.1,LVL)]\n");			
		break;
	case TN_PLANT_LEAF:
		sprintf(tmp,"Leaf(4,1,10,2,1)[\"Leaf\",Color(0.5*URAND+0.25,0.5*RED,0,0.2*URAND)]\n");
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
		sprintf(tmp,"rocks(1,1e-6)[Color(0.5,0.5,0.5)]\n");
		break;
	case TN_FOG:
		sprintf(tmp,"fog(0.0)\n");
		break;
	case TN_SNOW:
		sprintf(tmp,"snow(0.2,0.7,1,1,1)\n");
		break;
	case TN_CLOUD:
		sprintf(tmp,"clouds(0.1,-0.1,0)\n");
		break;
	case TN_FCHNL:
		sprintf(tmp,"fractal(SS|SQR,14,3,0.1,0.1)\n");
		break;
	case TN_ERODE:
		sprintf(tmp,"erode(1,0,1,0.1,1,1,0,5)\n");
		break;
	case TN_GLOSS:
		sprintf(tmp,"gloss(1,1)\n");
		break;
	case TN_BASE:
		sprintf(tmp,"base(0)\n");
		break;
	case TN_WATER:
		sprintf(tmp,"ocean(%s,%s)\n",OceanState::getDfltOceanLiquidExpr(),OceanState::getDfltOceanSolidExpr());
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
		sprintf(tmp,"noise(SCALE,5,2)\n");
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
	case TN_HALO:
		return new ModelSym("Halo",type);
	case TN_PLANET:
		return new ModelSym("Planet",type);
	case TN_MOON:
		return new ModelSym("Moon",type);
	case TN_LAYER:
		return new ModelSym("Layer",type);
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
	case TN_SPRITE:
		return new ModelSym("Sprite",type);
	case TN_PLANT:
		return new ModelSym("Plant",type);
	case TN_PLANT_BRANCH:
		return new ModelSym("Branch",type);
	case TN_PLANT_LEAF:
		return new ModelSym("Leaf",type);
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
		return new ModelSym("Ocean",type);
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
// UniverseModel::getObjectSymbol() return object's dir symbol
//-------------------------------------------------------------
ModelSym* UniverseModel::getTypeSymbol(int type){
    switch(type){
	case GN_RANDOM:
		return new ModelSym("[Random]",type);
	case GN_TREE:
		return new ModelSym("Tree",type);
	case GN_BUSH:
		return new ModelSym("Bush",type);
	case GN_GRASS:
		return new ModelSym("Grass",type);
	case GN_GASSY:
		return new ModelSym("GasGiant",type);
	case GN_ROCKY:
		return new ModelSym("Rocky",type);
	case GN_OCEANIC:
		return new ModelSym("Oceanic",type);
	case GN_ICY:
		return new ModelSym("Icy",type);
	case GN_RED_STAR:
		return new ModelSym("Red",type);
	case GN_YELLOW_STAR:
		return new ModelSym("Yellow",type);
	case GN_WHITE_STAR:
		return new ModelSym("White",type);
	case GN_BLUE_STAR:
		return new ModelSym("Blue",type);
   }
}
std::string UniverseModel::typeSymbol(int type){
	ModelSym *tmp=getTypeSymbol(type);
	std::string str(tmp->name());
	delete tmp;
	return str;
}
//-------------------------------------------------------------
// UniverseModel::TypeTypeList() return list of generated objects
//-------------------------------------------------------------
void UniverseModel::getTypeList(int type,LinkedList<ModelSym*>&list)
{
	switch(type&TN_TYPES){
	default:
		break;
	case TN_MOON:
		list.add(getTypeSymbol(GN_RANDOM));
		list.add(getTypeSymbol(GN_ROCKY));
		list.add(getTypeSymbol(GN_OCEANIC));
		list.add(getTypeSymbol(GN_ICY));
		break;
	case TN_PLANET:
		list.add(getTypeSymbol(GN_RANDOM));
		list.add(getTypeSymbol(GN_GASSY));
		list.add(getTypeSymbol(GN_ROCKY));
		list.add(getTypeSymbol(GN_OCEANIC));
		list.add(getTypeSymbol(GN_ICY));
		break;
	case TN_PLANT:
		list.add(getTypeSymbol(GN_RANDOM));
		list.add(getTypeSymbol(GN_TREE));
		list.add(getTypeSymbol(GN_BUSH));
		list.add(getTypeSymbol(GN_GRASS));
		break;
	case TN_STAR:
		list.add(getTypeSymbol(GN_RANDOM));
		list.add(getTypeSymbol(GN_RED_STAR));
		list.add(getTypeSymbol(GN_YELLOW_STAR));
		list.add(getTypeSymbol(GN_WHITE_STAR));
		list.add(getTypeSymbol(GN_BLUE_STAR));
		break;
	}
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
		if(obj->collapsed() && obj->getParent()){
			obj=obj->getParent();
			obj->set_expanded();
			return getAddList(obj,list);
		}
		list.add(getObjectSymbol(TN_STAR));
		list.add(getObjectSymbol(TN_PLANET));
		break;
	case TN_CORONA:
		list.add(getObjectSymbol(TN_CORONA));
		break;
	case TN_STAR:
		if(obj->collapsed() && obj->getParent()){
			return getAddList(obj->getParent(),list);
		}
		list.add(getObjectSymbol(TN_CORONA));
		list.add(getObjectSymbol(TN_HALO));

		break;
	case TN_PLANET:
		if(obj->collapsed() && obj->getParent()){
			list.add(getObjectSymbol(TN_PLANET));
			//return getAddList(obj->getParent(),list);
		}
		else{
			if(!obj->hasChild(ID_SKY) || actionmode==DROPPING)
				list.add(getObjectSymbol(TN_SKY));
			if(!obj->hasChild(TN_CLOUDS) || actionmode==DROPPING)
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
		break;
	case TN_CLOUDS:
	case TN_RING:
		if(obj->collapsed() && obj->getParent())
			return getAddList(obj->getParent(),list);
		break;
	case TN_PLANT_BRANCH:
		list.add(new ModelSym("Branch",TN_PLANT_BRANCH));
		list.add(new ModelSym("Leaf",TN_PLANT_LEAF));
		break;
	case TN_PLANT_LEAF:
		list.add(new ModelSym("Leaf",TN_PLANT_LEAF));
		break;
	case TN_PLANT:
		if(obj->collapsed())
			list.add(new ModelSym("Plant",TN_PLANT));
		else
			list.add(new ModelSym("Branch",TN_PLANT_BRANCH));
		//list.add(new ModelSym("Leaf",TN_PLANT_LEAF));
		break;
	case TN_MAP:
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
			list.add(getObjectSymbol(TN_SPRITE));
			list.add(getObjectSymbol(TN_PLANT));
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
		list.add(getObjectSymbol(TN_TEXTURE));
		list.add(getObjectSymbol(TN_SPRITE));
		list.add(getObjectSymbol(TN_PLANT));
		list.add(getObjectSymbol(TN_POINT));

		if(!obj->hasChild(ID_COLOR))
			list.add(getObjectSymbol(TN_COLOR));
		if(!obj->hasChild(ID_GLOSS))
			list.add(getObjectSymbol(TN_GLOSS));
		if(obj->getParent()->getFlag(TN_TYPES)==TN_CLOUDS)
			break;
		//if(actionmode==DROPPING)
		//	break;

		if(!obj->hasChild(ID_ERODE))
			list.add(getObjectSymbol(TN_ERODE));
		if(!obj->hasChild(ID_FCHNL))
			list.add(getObjectSymbol(TN_FCHNL));
		if(!obj->hasChild(ID_FOG))
			list.add(getObjectSymbol(TN_FOG));
		if(!obj->hasChild(ID_WATER))
			list.add(getObjectSymbol(TN_WATER));
		if(!obj->hasChild(ID_SNOW))
		    list.add(getObjectSymbol(TN_SNOW));
		if(!obj->hasChild(ID_MAP))
			list.add(getObjectSymbol(TN_MAP));
		list.add(getObjectSymbol(TN_ROCKS));
		break;
	case TN_SNOW:
	case TN_TEXTURE:
	case TN_SPRITE:
	case TN_POINT:
	case TN_COLOR:
	case TN_GLOSS:
	case TN_FCHNL:
		if(actionmode!=DROPPING)
			break;
		list.add(getObjectSymbol(TN_TEXTURE));
		list.add(getObjectSymbol(TN_SPRITE));
		list.add(getObjectSymbol(TN_POINT));
		list.add(getObjectSymbol(TN_COLOR));
		list.add(getObjectSymbol(TN_GLOSS));
		list.add(getObjectSymbol(TN_SNOW));
		list.add(getObjectSymbol(TN_FCHNL));

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
		while(child=children++)
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
  	int rflags=node->getFlag(RND_FLAGS);
    node->clrAllFlags();
    node->setFlag(rflags);
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
		case ID_HALO:
			node->setFlag(TN_HALO);
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
		case ID_PLANT:
			node->setFlag(TN_PLANT);
			break;
		case ID_BRANCH:
			node->setFlag(TN_PLANT_BRANCH);
			break;
		case ID_LEAF:
			node->setFlag(TN_PLANT_LEAF);
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
		case ID_SPRITE:
			node->setFlag(TN_SPRITE);
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
	case TN_PLANT:
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
	TreeNode *init_parent=parent;
//#define DEBUG_ADD_TO_TREE
#ifdef DEBUG_ADD_TO_TREE
	if(child)
		cout<<" parent="<<parent->node->typeName()<<" newobj="<<node->typeName()<<" child="<<child->node->typeName();
	else
		cout<<" parent="<<parent->node->typeName()<<"<"<<parent->label()<<"> newobj="<<node->typeName()<<"<"<<node->nodeName()<<">";

#endif
    int branch=node->getFlag(NODE_BRANCH);
	TreeNode *root=new TreeNode(node);
	setType(node);

	int ptype=parent->getFlag(TN_TYPES);
	int ntype=node->getFlag(TN_TYPES);

	switch(ptype){
	case TN_DENSITY:
	case TN_POINT:
	case TN_COLOR:
	case TN_CRATERS:
	case TN_TEXTURE:
	case TN_SPRITE:
	case TN_FOG:
	case TN_SNOW:
	case TN_ERODE:
	case TN_GLOSS:
	case TN_BASE:
	case TN_WATER:
	case TN_FCHNL:
		parent=parent->getParent();
		break;
	case TN_PLANT_LEAF:
	case TN_PLANT_BRANCH:
		parent=parent->getParent();
		ptype=parent->getFlag(TN_TYPES);
		if(ntype==TN_PLANT_LEAF || ntype==TN_PLANT_BRANCH){
			while(parent && ptype==TN_PLANT_BRANCH ){
				parent=parent->getParent();
				if(parent)
					ptype=parent->getFlag(TN_TYPES);
			}
			break;
		}
		while(parent && (ptype==TN_PLANT_BRANCH ||ptype==TN_PLANT_LEAF ||ptype==TN_PLANT)){
			parent=parent->getParent();
			if(parent)
				ptype=parent->getFlag(TN_TYPES);
		}
		break;
	case TN_PLANT:
		if(ntype!=TN_PLANT_BRANCH)
			parent=parent->getParent();
		break;
	case TN_ROCKS:
	    if(!branch)
			parent=parent->getParent();
		break;
	case TN_SKY:
		break;
	case TN_CLOUD:
		parent=parent->getParent();
		break;
	case TN_CORONA:
	case TN_HALO:
		break;
    

	}
	ptype=parent->getFlag(TN_TYPES);
	switch(ntype){
	case TN_SURFACE:
		switch(ptype){
		case TN_CORONA:
		case TN_SKY:
		case TN_HALO:
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
		break;
	case TN_LAYER:
    	if(ptype!=TN_MAP)
  			parent=parent->getParent();
		break;
	case TN_PLANT_LEAF:
		break;

	case TN_PLANT_BRANCH:
		break;
	case TN_PLANT:
    	while(parent && parent->getParent() && ptype!=TN_SURFACE){
  			parent=parent->getParent();
  			ptype=parent->getFlag(TN_TYPES);
     	}
		break;
	}
	ptype=parent->getFlag(TN_TYPES);
#ifdef DEBUG_ADD_TO_TREE
	if(init_parent !=parent)
		cout<<" new parent="<<parent->node->typeName()<<"<"<<parent->label()<<">"<<endl;
	else
		cout<<endl;
#endif
	if(parent && node->getFlag(NODE_HIDE)){
        delete root;
        root=parent;
    }
    else{
 	    root->setParent(parent);
 	    if(child)
 	    	parent->addAfter(child, root); // insert after child
		else
        	parent->addChild(root);        // add as last child
    }

	if(!node->getFlag(NODE_COMP)){
		LinkedList<NodeIF*>children;
		if(node->getChildren(children)){
			NodeIF *child;
			children.ss();
			int i=0;
			while(child=children++){
				addToTree(root,child);
			}
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
	fprintf(fp,"%s\n",default_galaxy);
}

//-------------------------------------------------------------
// UniverseModel::make() build default libraries
//-------------------------------------------------------------
void UniverseModel::make_libraries(int i)
{
	saveLibraryItem(TN_NOISE, "rounded", "noise(GRADIENT,11,4,0.5,0.5)", i);
	saveLibraryItem(TN_NOISE, "ridged", "noise(NABS|SQR,11,4,0.5,0.5)", i);
	saveLibraryItem(TN_NOISE, "eroded", "noise(NABS|SQR|NEG,11,4,0.5,0.5)", i);
	saveLibraryItem(TN_WAVES, "ripples", "0.01*noise(SCALE,22,2,0.7)", i);
	saveLibraryItem(TN_WAVES, "choppy", "0.02*noise(NABS|SQR|SCALE,15,6,0.4,0.2)", i);
	saveLibraryItem(TN_WAVES, "seawaves", "0.01*noise(NABS|SQR|SCALE,16,6,0.4,0.2)+0.01*noise(SCALE,22,2,0.7)", i);
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
"		speed=0.1;\n"
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

