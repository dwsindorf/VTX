
//  ModelClass.h

#ifndef _MODELCLASS
#define _MODELCLASS

#include "ListClass.h"
#include "NodeIF.h"
#include <stdio.h>

//#ifndef _WIN32
#include <strings.h>
//#endif



extern void set_node(NodeIF *node); // in sx.y
extern NodeIF *get_node();			// in sx.y
extern int parse_file(char *);     // in sx.l

class ObjectMgr;

class ModelSym {
	char   _name[256];
public:
	int   value;
	char   path[1024];
	ModelSym(const char *n, int v) {
		value=v;
		strcpy(_name,n);
		path[0]=0;
	}
	ModelSym(const char *n, const char *p) {
		value=0;
		strcpy(_name,n);
		strcpy(path,p);
	}
	~ModelSym(){
		//cout<<"~ModelSym() "<<_name<<endl;
	}
	int isFile() { return path[0];}
	char *name() { return _name;}
};

class TreeNode
{
enum {
   maxstr=20
};
protected:
    char lablestr[maxstr];
    char namestr[maxstr];
    TreeNode *parent;
public:
    static int level;
 	NodeIF *node;
	LinkedList<TreeNode *> children;

	TreeNode(NodeIF *n);
	~TreeNode();

	int numChildren()					{ return children.size;}

	virtual bool isEnabled()           	{ return node->isEnabled();}
	virtual void setEnabled(bool b)    	{ node->setEnabled(b);}
	virtual bool isShowing()			{ return node->isShowing();}
	virtual void setShowing(bool b)     { node->setShowing(b); }
	virtual bool isObject()             { return node->isObject();}

	virtual TreeNode *getParent()     		{ return parent;}
	virtual void setParent(TreeNode *p)     { parent=p;}
	virtual void addChild(TreeNode *t);
	virtual void addAfter(TreeNode *a,TreeNode *b);
	virtual void removeChild(TreeNode *t) 	{ children.remove(t);}
	virtual void replaceChild(TreeNode *c,TreeNode *n)  {
		children.replace(c,n);
		n->setParent(this);
	}
	virtual void removeChildren();

	virtual void print(FILE *fp);
	void setName(const char *s) 	{ strncpy(namestr,s,maxstr);}
	void setName(char *s) 	{ strncpy(namestr,s,maxstr);}
	char *name()          	{ return namestr;}
	void setLabel(char *s) 	{ strncpy(lablestr,s,maxstr);}
	char *label()          	{ return lablestr;}
	int hasLabel()          { return lablestr[0];}
    int typeValue()       	{ return node->typeValue();}
    void visit(void (*func)	(TreeNode *));
	void setFlag(int f)		{ node->setFlag(f);}
	void clrFlag(int f)		{ node->clrFlag(f);}
	int getFlag(int f)		{ return node->getFlag(f);}
	void setType(int f)		{ node->setType(f);}
	int getType()			{ return node->getType();}

};

class Model
{
protected:
public:
	enum {
		ADDING,DROPPING,REPLACING
	};
	int actionmode;
	void setActionMode(int m) { actionmode=m;}
	int getActionMode()       {return actionmode;}
	int dropping()            { return actionmode==DROPPING;}
	int adding()               { return actionmode==ADDING;}
	int replacing()               { return actionmode==REPLACING;}
    int countNodes(TreeNode *);
	TreeNode *getNode(TreeNode *,NodeIF *);
    void visit(TreeNode *root, void (*func)(TreeNode *));

	virtual void make(FILE *);
	virtual void make_libraries(int i);
	virtual void saveLibraryItem(int type, const char *name, const char *expr, int w);

	virtual int parse(char *);
	virtual TreeNode *buildTree(NodeIF *);
	virtual void setType(NodeIF *);
	virtual NodeIF *parse_node(NodeIF *parent,char *s);
	virtual NodeIF *open_node(NodeIF *parent,char *s);
	virtual void save_node(NodeIF *obj,char *path);
	virtual void freeTree(TreeNode *);
	virtual TreeNode *insertInTree(TreeNode *parent, NodeIF *node);
	virtual TreeNode *addToTree(TreeNode *parent, NodeIF *node);
	virtual TreeNode *addToTree(TreeNode *parent, TreeNode *child, NodeIF *node);
	virtual TreeNode *replaceInTree(TreeNode *parent, TreeNode *child, NodeIF *node);

	virtual void printTree(TreeNode *root,char *);
	virtual int setPrototype(NodeIF*, NodeIF*);
	virtual int getPrototype(int,char *);
	virtual NodeIF* makeObject(NodeIF*, int)  { return 0;}

	virtual int getAddList(NodeIF*,LinkedList<ModelSym*>&list);
	virtual int getSaveList(NodeIF*,LinkedList<ModelSym*>&list);
	virtual int getReplaceList(NodeIF*,LinkedList<ModelSym*>&list);

	virtual ModelSym* getObjectSymbol(int);
	virtual ModelSym* getTypeSymbol(int);

	virtual void getTypeList(int,LinkedList<ModelSym*>&list);
	virtual void getFileList(int type,LinkedList<ModelSym*>&list);
	virtual void getObjectDirectory(int type,char *dir);
	virtual void getFullPath(ModelSym*,char*);

	void setFlag(TreeNode *n,int f) { if(n)n->node->setFlag(f);}
	void clrFlag(TreeNode *n,int f) { if(n)n->node->clrFlag(f);}
	int getFlag(TreeNode *n,int f)	{ return n?n->node->getFlag(f):0;}
};

#endif
