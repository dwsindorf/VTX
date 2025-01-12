// ModelClass.cpp

#include "ModelClass.h"
#include "ObjectClass.h"
#include "FileUtil.h"

//************************************************************
// TreeNode class
//***********************************************************
int TreeNode::level=0;
//-------------------------------------------------------------
// TreeNode::TreeNode constructor
//-------------------------------------------------------------
TreeNode::TreeNode(NodeIF *n)
{
	node=n;
	parent=0;
	if(n){
		setName((char*)n->typeName());
		setLabel(n->nodeName());
	}
	else{
		setName((char*)"root");
		lablestr[0]=0;
	}
}

//-------------------------------------------------------------
// TreeNode::~TreeNode destructor
//-------------------------------------------------------------
TreeNode::~TreeNode()
{
    //node->clrAllFlags();
    parent=0;
	children.free();
}

//-------------------------------------------------------------
// TreeNode::addChild() add a tree node
//-------------------------------------------------------------
void  TreeNode::addChild(TreeNode *t)
{
	children.add(t);
	t->setParent(this);
}
//-------------------------------------------------------------
// TreeNode::addAfter() add a tree node
//-------------------------------------------------------------
void  TreeNode::addAfter(TreeNode *a,TreeNode *t)
{
	if(a)
		children.add(a,t);
	else{
		children.ss();
		children.push(t);
	}
	t->setParent(this);
}

//-------------------------------------------------------------
// TreeNode::removeChildren remove children without deletion
//-------------------------------------------------------------
void TreeNode::removeChildren()
{
	children.reset();
}

//-------------------------------------------------------------
// TreeNode::print() print a tree node
//-------------------------------------------------------------
void TreeNode::print(FILE *fp)
{
	char buff1[10000];
	char buff2[10000];
	buff1[0]=buff2[0]=0;
	if(node){
		for(int i=0;i<level;i++)
			strcat(buff1," ");
		if(lablestr[0]){
			sprintf(buff2,"%s<%s>",name(),label());
			strcat(buff1,buff2);
		}
		else
			strcat(buff1,name());
		node->printNode(buff2);
		//fprintf(fp,"%-30s %s tn:0x%-8X obj:0x%-8X\n",buff1,buff2,this,node);
		fprintf(fp,"%-30s %s\n",buff1,buff2);
	}
	level++;
	children.ss();
	TreeNode *child;
	while(child=children++)
		child->print(fp);
	level--;
}

//-------------------------------------------------------------
// TreeNode::visit() visit all nodes and apply function
//-------------------------------------------------------------
void TreeNode::visit(void (*func)(TreeNode *))
{
	(*func)(this);
	Node<TreeNode*> *ptr=children.ptr;
	children.ss();
	TreeNode *child;
	while(child=children++)
		child->visit(func);
	children.ptr=ptr;
}

//************************************************************
// Model class
//************************************************************
void Model::getFullPath(ModelSym*m,char *c){
	c[0]=0;
	if(m->isFile())
		sprintf(c,"%s%s",m->path,m->name());
}

void Model::setType(NodeIF *){}

//-------------------------------------------------------------
// Model::buildTree() build a tree
//-------------------------------------------------------------
TreeNode *Model::buildTree(NodeIF *node)
{
	TreeNode *root=new TreeNode(node);
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
// Model::replaceInTree() replace a tree node
//-------------------------------------------------------------
TreeNode *Model::replaceInTree(TreeNode *parent, TreeNode *child, NodeIF *node)
{
    TreeNode *root=new TreeNode(node);
	return root;
}
//-------------------------------------------------------------
// Model::insertInTree() add a tree node
//-------------------------------------------------------------
TreeNode *Model::insertInTree(TreeNode *parent, NodeIF *node)
{
	return addToTree(parent, node);
}

//-------------------------------------------------------------
// Model::addToTree() add a tree node
//-------------------------------------------------------------
TreeNode *Model::addToTree(TreeNode *parent, TreeNode *child, NodeIF *node)
{
    TreeNode *root=new TreeNode(node);

	if(parent)
        parent->addChild(root);

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
// Model::addToTree() add a tree node
//-------------------------------------------------------------
TreeNode *Model::addToTree(TreeNode *parent, NodeIF *node)
{
	return addToTree(parent,0,node);
}

//-------------------------------------------------------------
// Model::freeTree() build a tree
//-------------------------------------------------------------
void Model::freeTree(TreeNode *root)
{
	delete root;
}
//-------------------------------------------------------------
// Model::printTree() print tree to stdout
//-------------------------------------------------------------
void Model::printTree(TreeNode*root, char *fn)
{
 	FILE *fp;
 	if(root && ((fp = fopen(fn, "wb")) != NULL)){
		root->print(fp);
		int n=countNodes(root);
		fprintf(fp,"\ntotal tree nodes %d\n",n);
		fclose(fp);
	}
}

//-------------------------------------------------------------
// Model::visit() visit all nodes and apply function
//-------------------------------------------------------------
void Model::visit(TreeNode *root, void (*func)(TreeNode *))
{
	root->visit(func);
}

//-------------------------------------------------------------
// Model::countNodes() return node count
//-------------------------------------------------------------
static int ncount;
static void countnodes(TreeNode *node){ncount++;}
int Model::countNodes(TreeNode *root)
{
	ncount=0;
	root->visit(&countnodes);
	return ncount;
}

//-------------------------------------------------------------
// Model::getNode() return node count
//-------------------------------------------------------------
static NodeIF   *gnode=0;
static TreeNode *tnode=0;
static void getnode(TreeNode *tn){
	if(tn->node==gnode)
		tnode=tn;
}
TreeNode *Model::getNode(TreeNode *root,NodeIF *n)
{
    if(root==0)
        return 0;
	gnode=n;
	root->visit(&getnode);
	return tnode;
}

int Model::setPrototype(NodeIF *parent, NodeIF *child)
{
	return 0;
}

int Model::getPrototype(int i,char *)
{
	return 0;
}
int Model::getAddList(NodeIF *obj,LinkedList<ModelSym*>&list){return 0;}
int Model::getSaveList(NodeIF *obj,LinkedList<ModelSym*>&list){return 0;}
int Model::getReplaceList(NodeIF *obj,LinkedList<ModelSym*>&list){return 0;}

void Model::getTypeList(int type,LinkedList<ModelSym*>&list){}
void Model::getFileList(int type,LinkedList<ModelSym*>&list){}

void Model::getObjectDirectory(int type,char *dir){}
//ModelSym* Model::getObjectSymbol(int type){return 0;}
//ModelSym* Model::getTypeSymbol(int type){return 0;}
void Model::make(FILE *file){}
void Model::make_libraries(int i) {}

//-------------------------------------------------------------
// Model::saveLibraryItem() save a node file
//-------------------------------------------------------------
void Model::saveLibraryItem(int type, const char *name, const char *expr, int w)
{
    FILE *fp=0;
	char path[256];
	char file_name[256];
	sprintf(file_name,"%s%s",name,File.ext);
	getObjectDirectory(type,path);
	File.addToPath(path,file_name);
	if(!w){
		fp=fopen(path, "rb"); // already have
		if(fp){
			fclose(fp);
			return;
		}
	}
	fp=fopen(path, "wb");
	fprintf(fp,"%s\n",expr);
	fclose(fp);
}
//-------------------------------------------------------------
// Model::parse_node() parse a node string
//-------------------------------------------------------------
NodeIF *Model::parse_node(NodeIF *parent,char *s){
    FILE *fp = File.writeFile(File.system, (char*)"node.spx");
	if(!fp)
	    return 0;
	fprintf(fp,s);
	fclose(fp);
    char path[512];

    File.getBaseDirectory(path);
    File.addToPath(path,File.system);
    File.addToPath(path,"node.spx");

	fp=fopen(path,"rb");
	if(!fp)
	    return 0;
	set_node(0);
	int ok=parse(path);
	fclose(fp);
	if(ok)
		return get_node();
	return 0;
}

int Model::parse(char *fn){
	return parse_file(fn);
}

//-------------------------------------------------------------
// Model::parse_node() parse a node string
//-------------------------------------------------------------
void Model::save_node(NodeIF *obj,char *path){
    FILE *fp = fopen(path, "wb");
	if(fp){
		obj->saveNode(fp);
		fprintf(fp,"\n");
		fclose(fp);
	}
}

//-------------------------------------------------------------
// Model::open_node() parse a node string
//-------------------------------------------------------------
NodeIF *Model::open_node(NodeIF *parent,char *fn)
{
	set_node(0);

 	FILE *fp = fopen(fn, "rb");
 	if(fp != NULL){
		int ok=parse(fn);
		fclose(fp);
		if(!ok)
			return 0;
		return get_node();
	}
	return 0;
}
