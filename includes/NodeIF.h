// NodeIF.h

#ifndef _NODEIF
#define _NODEIF

#include "ListClass.h"

class NodeIF;

// tree node types
enum {
	TN_BRANCH       = 0x00000100,
	TN_HIDEFLAG     = 0x00000200,
	TN_INVALID      = 0x00000800,
	TN_PARENT       = 0x00001000,
	TN_CHILD        = 0x00002000,
	TN_EMPTY        = 0x00004000,
	TN_VOXEL        = 0x00008000,
	TN_FLAGS  	    = 0x0000ff00,
	TN_TYPES  	    = 0x000000ff,
	TN_ALL          = TN_FLAGS|TN_TYPES
};

// bits that control randomization behavior
enum {
	RND_RANDOMIZE   = 0x00100000, // need to randomize construction
	RND_PROTOTYPE   = 0x00200000, // need to randomize prototype
	RND_FLAGS=RND_RANDOMIZE|RND_PROTOTYPE
};

// bits that control runtime behavior
enum {
	RTM_DISABLED    = 0x00400000,
	RTM_ANIMATE     = 0x00800000,
	RTM_FLAGS       = RTM_DISABLED|RTM_ANIMATE
};


// bits that control treenode status
enum {
	NODE_HIDE       = 0x01000000,
	NODE_COMP       = 0x02000000,
	NODE_INVAL	    = 0x04000000,
	NODE_STOP	    = 0x08000000,
	NODE_BAD	    = 0x10000000,
	NODE_BRANCH     = 0x20000000,
	NODE_OPEN       = 0x40000000,
	NODE_CLOSED     = 0x80000000,
	NODE_FLAGS      = 0xff000000
};

// bits that define main object class types
// - these are hard-coded for each object in the object's constructor
// - see OrbitalClass.h, TerrainMgr.h and TerrainNode.h for sub-id codes
//
enum {
	ID_OBJECT       = 0x00010000,  // orbital node
	ID_ORBITAL      = 0x00030000,  // orbital node
	ID_TERRAIN      = 0x00040000,  // terrain node
	ID_SCOPE        = 0x000c0000,  // scope node
	ID_SCENE        = 0x00080000,  // scene node
	ID_SHADER       = 0x00100000,  // shader node
	ID_CLASS        = 0x001f0000,  //
	ID_LEVEL        = 0x0f000000,  // level mask
	ID_LEVEL0       = 0x00000000,  // level code
	ID_LEVEL1       = 0x01000000,  // level code
	ID_LEVEL2       = 0x02000000,  // level code
	ID_LEVEL3       = 0x03000000,  // level code
	ID_LEVEL4       = 0x04000000,  // level code
	ID_LEVEL5       = 0x05000000,  // level code
	ID_LEVEL6       = 0x06000000,  // level code
	ID_LEVEL7       = 0x07000000,  // level code
	ID_LEVEL8       = 0x08000000,  // level code
	ID_LEVEL9       = 0x09000000,  // level code
	ID_LEVEL10      = 0x0a000000,  // level code
};


class NodeIF
{
protected:
public:
    int nodeflags;
    NodeIF() { nodeflags=0;}
    virtual ~NodeIF() {}
	void clrNodeFlags()	 		{ BIT_OFF(nodeflags,NODE_FLAGS); }
	void clrAllFlags()	 		{ nodeflags=0; }
	int  valid()				{ return (nodeflags & NODE_INVAL)?0:1;}
	int  invalid()				{ return (nodeflags & NODE_INVAL)?1:0;}
	int  expanded()				{ return (nodeflags & NODE_OPEN)?1:0;}
	int  collapsed()			{ return (nodeflags & NODE_CLOSED)?1:0;}
	void set_expanded()			{ BIT_ON(nodeflags,NODE_OPEN);BIT_OFF(nodeflags,NODE_CLOSED);}
	void set_collapsed()		{ BIT_OFF(nodeflags,NODE_OPEN);BIT_ON(nodeflags,NODE_CLOSED);}
	void setFlag(int f,int i)	{ BIT_SET(nodeflags,f,i);}
	void setFlag(int i)			{ BIT_ON(nodeflags,i);}
	void clrFlag(int i)			{ BIT_OFF(nodeflags,i);}
	int getFlag(int i)			{ return nodeflags & i;}
	int getType()               { return nodeflags & TN_TYPES;}
	void setType(int t)         { BIT_OFF(nodeflags,TN_TYPES);BIT_ON(nodeflags,t); }

	void setShowing(bool b)     { if(b) clrFlag(NODE_HIDE); else setFlag(NODE_HIDE); }
	bool isShowing()			{ return getFlag(NODE_HIDE) ? false:true;}

	bool isRandom()             { return getFlag(RND_RANDOMIZE) ? false:true;}
	void setRandom(bool b)      { if(b) clrFlag(RND_RANDOMIZE); else setFlag(RND_RANDOMIZE); }
	void setProtoValid(bool b)      { if(b) setFlag(RND_PROTOTYPE); else clrFlag(RND_PROTOTYPE); }
	bool protoValid()             { return getFlag(RND_PROTOTYPE) ? true:false;}
	virtual bool canRandomize() { return false;}
	virtual bool randomize()    { return false;}
	virtual void setRseed(double s) {}
	virtual double getRseed()   { return 0;}
	virtual void setDefault()   { }
	virtual NodeIF *getInstance() {return this;}

	virtual bool isAnimating()         { return getFlag(RTM_ANIMATE) ? true:false;}
	virtual void setAnimating(bool b)  { if(b) setFlag(RTM_ANIMATE); else clrFlag(RTM_ANIMATE);}
	virtual bool isEnabled()           { return getFlag(RTM_DISABLED) ? false:true;}
	virtual void setEnabled(bool b)    { if(b) clrFlag(RTM_DISABLED); else setFlag(RTM_DISABLED);}

	// virtual functions

	virtual bool setProgram()		{return true;}
	virtual bool initProgram()		{return true;}

	virtual bool isObject()         { return false;}

	virtual void invalidate()	{ BIT_ON(nodeflags,NODE_INVAL);}
	virtual void validate()		{ BIT_OFF(nodeflags,NODE_INVAL);}

	virtual NodeIF *getParent()					{ return 0;}
	virtual void setParent(NodeIF *p)			{ }
	virtual NodeIF *removeChild(NodeIF *n)		{ return 0;}
	virtual NodeIF *removeNode(){
		NodeIF *p=getParent();
		if(p)
			return p->removeChild(this);
		return this;
	}
	virtual NodeIF *replaceNode(NodeIF *n){
		NodeIF *p=getParent();
		if(p)
			p->replaceChild(this,n);
		return this;
	}
	virtual bool hasChild(NodeIF *n)			{ return false;}

	virtual bool hasChild(int vtype)			{ return false;}
	virtual bool hasChildren()					{ return false;}
	virtual int getChildren(LinkedList<NodeIF*>&l) { return 0;}
	virtual NodeIF *addChild(NodeIF *n)			{ return n;}
	virtual NodeIF *addAfter(NodeIF *a, NodeIF *n)	{
		if(a)
			return a->addChild(n);
		else
			return addChild(n);
	}
	virtual void addSibling(NodeIF *c){
		NodeIF *p=getParent();
		if(p)
			p->addAfter(this,c);
	}
	virtual NodeIF *replaceChild(NodeIF *c,NodeIF *n)	{ return 0;}
	virtual void removeChildren()				{ }
	virtual bool isLeaf()                       { return true;}
	virtual bool isEmpty()                       { return isLeaf()?false:!hasChildren();}

	virtual int linkable()      				{ return 0;}
	virtual int typeValue()						{ return 0;}
	virtual int typeClass()						{ return typeValue()&ID_CLASS;}
	virtual int typeLevel()						{ return typeValue()&ID_LEVEL;}

	virtual const char *typeName()				{ return "";}
	virtual char *nodeName()					{ return (char*)"";}
	virtual char *getName()                     { return 0;}
	virtual void  setName(char *s)              { }
	virtual void valueString(char *s)			{ strcpy(s+strlen(s),typeName());}
	virtual int  valueValid()					{ return 1;}
	virtual int  propertyValid()				{ return 0;}
	virtual void propertyString(char *s)		{ }
	virtual void saveNode(FILE *f)				{ }
	virtual void save(FILE *f)					{ }

	virtual void setValue(NodeIF *)				{ }
	virtual void setProperties(NodeIF *)		{ }
	virtual NodeIF *setValueString(char *)		{return 0;}
	virtual NodeIF *setPropertyString(char *)	{return 0;}
	virtual NodeIF *getValue()					{return 0;}
	virtual NodeIF *getProperties()				{return 0;}
	virtual void printNode(char *s){
		s[0]=0;
		if(propertyValid())
		    propertyString(s);
		else if(valueValid())
			valueString(s);
		else
			strcpy(s,typeName());
	}
	virtual int isExprNode()                    { return 0;}
	virtual void applyExpr()					{ }
	virtual void clearExpr()					{ }
	virtual void setExpr(char *c)				{ }
	virtual void visitNode(void  (*func)(NodeIF*))	{
		(*func)(this);
		if(getFlag(NODE_STOP)){
		    clrFlag(NODE_STOP);
			return;
		}
		LinkedList<NodeIF*> nlist;
		getChildren(nlist);
		NodeIF *obj;
		nlist.ss();
		while((obj=nlist++)>0){
			obj->visitNode(func);
			if(obj->getFlag(NODE_STOP)){
				obj->clrFlag(NODE_STOP);
				break;
			}
		}
 	}
};

#endif

