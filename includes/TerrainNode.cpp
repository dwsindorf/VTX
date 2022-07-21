
#include "TerrainMgr.h"
#include "SceneClass.h"
#include "NodeIF.h"
#include <iostream>
#include <math.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char tabs[];
extern int hits,visits,misses;

extern Scope   *CurrentScope;
extern int addtabs;
extern void inc_tabs();
extern void dec_tabs();


int getargs(TNode *args, TerrainData *array, int max)
{
	int n=0;
	TNarg *arg=(TNarg*)args;
	while(arg && n<max){
		SPUSH;
		arg->eval();
		array[n++].copy(S0);
		SPOP;
		arg=arg->next();
	}
	return n;
}

int getargs(TNode *args, double *array, int max)
{
	int n=0;
	TNarg *arg=(TNarg*)args;
	while(arg && n<max){
		SPUSH;
		arg->eval();
		array[n++]=S0.s;
		SPOP;
		arg=arg->next();
	}
	return n;
}

int numargs(TNode *args)
{
	int n=0;
	TNarg *arg=(TNarg*)args;
	while(arg){
		n++;
		arg=arg->next();
	}
	return n;
}

static 	        TerrainData Td;
const double PIX2=PI*2;

//************************************************************
// TNode base class
//************************************************************
//-------------------------------------------------------------
// TNode:: virtual base class functions
//-------------------------------------------------------------
void TNode::setToken(char *)  	{}
char *TNode::getToken()			{return 0;}
void TNode::addToken(LinkedList<TNode*>&l)  {l.add(this);}
void TNode::eval()  			{}
void TNode::dump()  			{}
void TNode::init()  			{}
void TNode::save(FILE *f)		{}
void TNode::evalNode(TerrainData &t){
	t.reset();
    eval();
    t=S0;
}
void TNode::setStart(char *s){
	if(!CurrentScope->keep_limits())
		start=strlen(s);
}
void TNode::setMiddle(char *s){
	if(!CurrentScope->keep_limits())
		middle=strlen(s);
}
void TNode::setEnd(char *s){
	if(!CurrentScope->keep_limits())
		end=strlen(s);
}
int TNode::getStart()		{ return start;}
int TNode::getMiddle()  	{ return middle;}
int TNode::getEnd()			{ return end;}
void TNode::saveNode(FILE *f)		{ save(f);}

//-------------------------------------------------------------
// TNode::getMgr() get ExprMgr or TerrainMgr
//-------------------------------------------------------------
ExprMgr *TNode::getMgr()
{
	NodeIF *obj=getParent();

	while(obj && !(obj->typeValue() & ID_MGR)){
		obj=obj->getParent();
	}
	return (ExprMgr*)obj;
}
//-------------------------------------------------------------
// TNode::getExprMgr() get ExprMgr
//-------------------------------------------------------------
ExprMgr *TNode::getExprMgr()
{
	NodeIF *obj=getParent();

	while(obj && obj->typeValue() != ID_TNMGR){
		obj=obj->getParent();
	}
	return (ExprMgr*)obj;
}

//-------------------------------------------------------------
// TNode::getTerrainMgr() get TerrainMgr
//-------------------------------------------------------------
TerrainMgr *TNode::getTerrainMgr()
{
	NodeIF *obj=getParent();

	while(obj && !(obj->typeValue() & ID_TMGR)){
		obj=obj->getParent();
	}
	return (TerrainMgr*)obj;
}

//-------------------------------------------------------------
// TNode::visit() visit the objects children & apply function
//-------------------------------------------------------------
void TNode::visit(void (*func)(TNode*))
{
	(*func)(this);
}

//-------------------------------------------------------------
// TNode::addChild
//-------------------------------------------------------------
NodeIF *TNode::addChild(NodeIF *x){
    NodeIF *p=getParent();
	if(!p)
	    return x;

    TNadd *node=new TNadd(this,(TNode*)x);
	p->replaceChild(this,node);
	return x;
}

//-------------------------------------------------------------
// TNode::getObject() return container object
//-------------------------------------------------------------
Object3D *TNode::getObject()
{
    NodeIF *p=getParent();
    while(p && !(p->typeClass()&ID_OBJECT))
        p=p->getParent();
	return (Object3D*)p;
}

//-------------------------------------------------------------
// TNode::setPropertyString() replace property subtree
//-------------------------------------------------------------
NodeIF *TNode::setPropertyString(char *s)
{
    NodeIF *oldval=getProperties();
    if(!oldval) 		// no replacable component
    	return 0;
	NodeIF *repnode=TheScene->parse_node(s);
	if(!repnode)
		return 0;		// invalid expression string
	NodeIF *repval=repnode->getProperties();
    if(!repval)
    	return 0;		// invalid replacement node

	repnode->removeChild(repval);
    setProperties(repval);
	delete repnode;
	return oldval;
}

//-------------------------------------------------------------
// TNode::setValueString() replace value subtree
//        if self-replaced, return new obj else this
//-------------------------------------------------------------
NodeIF *TNode::setValueString(char *s)
{
    NodeIF *oldval=getValue();
    if(!oldval) 		// no replacable value component
    	return 0;
	NodeIF *repnode=TheScene->parse_node(s);
	if(!repnode)
		return 0;		// invalid expression string
	if(oldval==this){	// replace this node
		replaceNode(repnode);
		return repnode;
	}
	else{				// replace child node
		setValue(repnode);
		delete oldval;
		return this;
	}
    return 0;
}

//-------------------------------------------------------------
void TNode::invalidate(){
	Object3D *p=getObject();
    if(p)
        p->invalidate();
}
TNode::~TNode()     		{}
Orbital *TNode::getOrbital(NodeIF *obj)
{
	while(obj && !(obj->typeValue() & ID_ORBITAL))
		obj=obj->getParent();

	return (Orbital*)obj;
}

//************************************************************
// Class TNunary
//************************************************************

//-------------------------------------------------------------
// TNunary:: virtual base class functions
//-------------------------------------------------------------
TNunary::TNunary(TNode *r){
	right=r;
	if(right)
		right->setParent(this);
}
void TNunary::eval()  		{ if(right) right->eval();}
void TNunary::init()  		{ if(right) right->init();}
void TNunary::save(FILE *f)	{ if(right) right->save(f);}
void TNunary::visit(void (*func) (TNode *)){
    (*func)(this);
    if(right)
        right->visit(func);
}

NodeIF *TNunary::removeChild(NodeIF *c){
	if(right==c)
	    right=0;
	return c;
}
void TNunary::addToken(LinkedList<TNode*>&l)
{
	l.add(this);
	if(right)
		right->addToken(l);
}

TNunary::~TNunary()
{
	DFREE(right);
}

//-------------------------------------------------------------
// TNunary::getChildren() return true if children
//-------------------------------------------------------------
bool TNunary::hasChildren()
{
	return right?true:false;
}

//-------------------------------------------------------------
// TNunary::getChildren() add to child list
//-------------------------------------------------------------
int TNunary::getChildren(LinkedList<NodeIF*>&l)
{
	l.add(right);
	return 1;
}
//-------------------------------------------------------------
// TNunary::replaceChild
//-------------------------------------------------------------
NodeIF *TNunary::replaceChild(NodeIF *c, NodeIF *n)
{
	if(right==c){
	    right=(TNode*)n;
		right->setParent(this);
	}
	return c;
}

//-------------------------------------------------------------
// TNunary::applyExpr() apply expr value
//-------------------------------------------------------------
void TNunary::applyExpr()
{
	if(right)
		right->applyExpr();
}

//-------------------------------------------------------------
// TNunary::clearExpr() clear expr value
//-------------------------------------------------------------
void TNunary::clearExpr()
{
	if(right)
		right->clearExpr();
}

//************************************************************
// TNinvert class
//************************************************************
void TNinvert:: eval()
{
	right->eval();
	S0.s=-S0.s;
	if(S0.pvalid()){
		S0.p.x=-S0.p.x;
		S0.p.y=-S0.p.y;
		S0.p.z=-S0.p.z;
	}
}
void TNinvert::addToken(LinkedList<TNode*>&l)
{
	l.add(this);
	//if(right)
	//right->addToken(l);
}
//-------------------------------------------------------------
// TNinvert::valueString()  get value substring
//-------------------------------------------------------------
void TNinvert::valueString(char *s)
{
	setStart(s);
	strcat(s,"-");
	right->valueString(s);
	setEnd(s);
}

//************************************************************
// Class TNgroup
//************************************************************
void TNgroup::valueString(char *s)
{
	char rstr[MAXSTR]={0};
    if(right)
	    right->valueString(rstr);
	if(strlen(rstr)){
		setStart(s);
	    sprintf(s+strlen(s),"(%s)",rstr);
		setEnd(s);
    }
    else
    	S0.set_inactive();
}
//-------------------------------------------------------------
// TNgroup::addToken() add to token list
//-------------------------------------------------------------
void TNgroup::addToken(LinkedList<TNode*>&l)
{
	TNunary::addToken(l);
}

//************************************************************
// Class TNvector
//************************************************************
//-------------------------------------------------------------
// TNvector::~TNvector() destructor
//-------------------------------------------------------------
TNvector::~TNvector()
{
	DFREE(expr);
	DFREE(right);
}
//-------------------------------------------------------------
// TNvector::valueString() node value substring
//-------------------------------------------------------------
void TNvector::valueString(char *s)
{
	TNvector *vector=expr?expr:this;
	setStart(s);
	Td.set_flag(COMPLEX);
	if(CurrentScope->tokens() && token && strlen(token)>0)
		strcat(s,token);
	else{
		sprintf(s+strlen(s),"%s(",symbol());
		TNarg *arg=(TNarg*)vector->right;
		while(arg){
			INIT;
			arg->valueString(s);
			arg=arg->next();
			if(arg)
				strcat(s,",");
		}
		strcat(s,")");
	}
    setEnd(s);
}

//-------------------------------------------------------------
// TNvector::save() save the node
//-------------------------------------------------------------
void TNvector::save(FILE *f)
{
	fprintf(f,"%s(",symbol());
	TNunary::save(f);
	fprintf(f,")");
	if(addtabs)
	    fprintf(f,"\n%s",tabs);
}

//-------------------------------------------------------------
// TNvector::setExpr() set expr string
//-------------------------------------------------------------
void TNvector::setExpr(char *c)
{
	TNvector *n=(TNvector*)TheScene->parse_node(c);
	if(n){
	    DFREE(expr);
	    expr=n;
	    expr->setParent(parent);
	}
}

//-------------------------------------------------------------
// TNvector::applyExpr() apply expr value
//-------------------------------------------------------------
void TNvector::applyExpr()
{
    if(expr){
        DFREE(right);
        right=expr->right;
		expr->right=0;
		if(right)
			right->setParent(this);
		delete expr;
        expr=0;
    }
}

//-------------------------------------------------------------
// TNvector::clearExpr() clear expr value
//-------------------------------------------------------------
void TNvector::clearExpr()
{
    DFREE(expr);
}
//************************************************************
// Class TNbinary
//************************************************************
//-------------------------------------------------------------
// TNbinary::visit() apply visit function
//-------------------------------------------------------------
void TNbinary::visit(void (*func) (TNode *)){
    (*func)(this);
    if(left)
        left->visit(func);
    if(right)
        right->visit(func);
}
//-------------------------------------------------------------
// TNbinary::getChildren() return true if children
//-------------------------------------------------------------
bool TNbinary::hasChildren()
{
	return (right||left)?true:false;
}
//-------------------------------------------------------------
// TNbinary::getChildren() add to child list
//-------------------------------------------------------------
int TNbinary::getChildren(LinkedList<NodeIF*>&l)
{
	int i=0;
	if(left){
		l.add(left);
		i++;
	}
	if(right){
		l.add(right);
		i++;
	}
	return i;
}


//-------------------------------------------------------------
// TNbinary::addToken() add to token list
//-------------------------------------------------------------
void TNbinary::addToken(LinkedList<TNode*>&l)
{
	if(left)
		left->addToken(l);
	l.add(this);
	if(right)
		right->addToken(l);
}
//-------------------------------------------------------------
// TNbinary::init() initialize the node
//-------------------------------------------------------------
void TNbinary::init()
{
	if(left)
		left->init();
	if(right)
		right->init();
}

//-------------------------------------------------------------
// TNbinary::applyExpr() apply expr value
//-------------------------------------------------------------
void TNbinary::applyExpr()
{
	if(left)
		left->applyExpr();
	if(right)
		right->applyExpr();
}

//-------------------------------------------------------------
// TNbinary::clearExpr() clear expr value
//-------------------------------------------------------------
void TNbinary::clearExpr()
{
	if(left)
		left->clearExpr();
	if(right)
		right->clearExpr();
}

//-------------------------------------------------------------
// TNbinary::eval() evaluate the node
//-------------------------------------------------------------
void TNbinary::eval()
{
    int cflag=1;
    int aflag=0;
	if(left){
		left->eval();
		cflag=S0.constant()?cflag:0;
		if(S0.inactive())
			aflag++;
	}
	if(right){
		SPUSH;
		right->eval();
		cflag=S0.constant()?cflag:0;
		if(S0.inactive())
			aflag++;
		SPOP;
	}
	if(cflag)
		S0.set_constant();
	else
		S0.clr_constant();
	S0.clr_inactive();
	if(aflag==0)
		return;
	if(aflag==2 || (typeValue()==ID_MULTIPLY) || (typeValue()==ID_DIVIDE))
		S0.set_inactive();
}

//-------------------------------------------------------------
// TNbinary::valueString() set value string
//-------------------------------------------------------------
void TNbinary::valueString(char *s)
{
	setStart(s);
	if(left){
		INIT;
		left->eval();
		if(!S0.inactive())
			left->valueString(s);
	}
	if(right){
		INIT;
		right->eval();
		if(!S0.inactive())
			right->valueString(s);
	}
	setEnd(s);
}

//-------------------------------------------------------------
// TNbinary::save() evaluate the node
//-------------------------------------------------------------
void TNbinary::save(FILE *f)
{
	if(left)
		left->save(f);
	if(right)
		right->save(f);
}

//-------------------------------------------------------------
// TNbinary::removeChild
//-------------------------------------------------------------
NodeIF *TNbinary::removeChild(NodeIF *c){
	NodeIF *p=getParent();
	if(!p || p->typeClass()!=ID_TERRAIN)
		return 0;
	TNode *x;
	if(right==c){
	    x=left;
		left=0;
	}
	else if(left==c){
	    x=right;
		right=0;
	}
	else
		return 0;
	p->replaceChild(this,x);
	return this;
}
//-------------------------------------------------------------
// TNbinary::replaceChild
//-------------------------------------------------------------
NodeIF *TNbinary::replaceChild(NodeIF *c, NodeIF *n){
	if(right==c){
	    right=(TNode*)n;
		right->setParent(this);
	}
	else if(left==c){
	    left=(TNode*)n;
		left->setParent(this);
	}
	return c;
}

//-------------------------------------------------------------
// TNbinary::addChild
//-------------------------------------------------------------
NodeIF *TNbinary::addChild(NodeIF *x){
    TNode *node=(TNode*)x;
    //if(x->linkable())
    //    node=(TNode*)x;
   // else
    if(!right){
        right=node;
        node->setParent(this);
    }
	else if(typeValue()==ID_MULTIPLY||typeValue()==ID_DIVIDE){
        NodeIF *p=getParent();
        NodeIF *c=new TNadd(this,node);
        p->replaceChild(this,c);
	}
	else{
        node=new TNadd(right,node);
        node->setParent(this);
        right=node;
    }
    return x;
}

//-------------------------------------------------------------
// TNbinary::~TNbinary() destructor
//-------------------------------------------------------------
TNbinary::~TNbinary()
{
	DFREE(left);
	DFREE(right);
}
//************************************************************
// TNcond
//************************************************************
TNcond::TNcond(TNode *l, TNode *r, TNode *c) :  TNbinary(l,r)
{
    cond=c;
}
TNcond::~TNcond()
{
	if(cond)
		delete cond;
	cond=0;
}

//-------------------------------------------------------------
// TNcond::init() initialize the node
//-------------------------------------------------------------
void TNcond::init()
{
	cond->init();
	TNbinary::init();
}

//-------------------------------------------------------------
// TNcond::eval() evaluate the node
//-------------------------------------------------------------
void TNcond::eval()
{
	INIT;
	S0.s=0;
	cond->eval();
	if(S0.s)
	    left->eval();
	else
	    right->eval();
	S0.clr_constant();
}

//-------------------------------------------------------------
// TNcond::save() save the node
//-------------------------------------------------------------
void TNcond::save(FILE *f)
{
	cond->save(f);
	fprintf(f,"?");
	left->save(f);
	fprintf(f,":");
	right->save(f);
}

//************************************************************
// TNexpr
//************************************************************
TNexpr::TNexpr(TNode *l, TNode *r) :  TNbinary(l,r) {}
void TNexpr::valueString(char *s)
{
	char lstr[MAXSTR]={0};
	char rstr[MAXSTR]={0};
	if(left){
		SINIT;
		left->valueString(lstr);
	}
	if(right){
		SINIT;
		right->valueString(rstr);
	}
	SINIT;
	if(((typeValue()==ID_MULTIPLY) || (typeValue()==ID_DIVIDE))
	   && ((strlen(lstr)==0)||(strlen(rstr)==0))){
		return;
	}
	if(strlen(lstr)>0)
		strcat(s,lstr);
	if(strlen(lstr)>0&&strlen(rstr)>0)
		strcat(s,symbol());
	if(strlen(rstr)>0)
		strcat(s,rstr);
}
void TNexpr::save(FILE *f)
{
	if(left)
		left->save(f);
	fprintf(f,symbol());
	if(right)
		right->save(f);
}

//-------------------------------------------------------------
// TNexpr::addToken() add to token list
//-------------------------------------------------------------
void TNexpr::addToken(LinkedList<TNode*>&l)
{
	TNbinary::addToken(l);
}

//************************************************************
// TNexpr sub-classes
//************************************************************
#define BINARY_EVAL(OP) \
    int cflag=1; \
    int aflag=0; \
    SINIT; \
    if(typeValue()==ID_ADD)\
	   right->eval(); \
    else \
       left->eval(); \
	cflag=S0.constant()?cflag:0; \
	if(S0.inactive()) \
		aflag++; \
	SPUSH; \
    if(typeValue()==ID_ADD)\
	   left->eval(); \
    else \
       right->eval(); \
	if(S0.inactive()) \
		aflag++; \
	cflag=S0.constant()?cflag:0; \
	if(S1.svalid() && S0.svalid()) { \
		S1.s = S1.s OP S0.s; \
		S1.set_svalid(); \
	} \
	else if(S1.svalid()) {\
		if(S0.pvalid())  \
			S1.p = S0.p OP S1.s; \
		else if(S0.cvalid())\
			S1.c = S0.c OP S1.s; \
		else \
			S1.s = S1.s OP 0.0; \
	} \
	else if(S0.svalid()) { \
		if(S1.pvalid())  \
			S1.p = S1.p OP S0.s; \
		else if(S1.cvalid())\
			S1.c = S1.c OP S0.s; \
		else \
			S1.s = S1.s OP 0.0; \
	} \
	else { \
		if(S1.pvalid() && S0.pvalid() )  \
			S1.p = S1.p OP S0.p; \
		else if(S0.pvalid()) \
			S1.p = S0.p; \
		if(S0.cvalid() && S1.cvalid()) \
			S1.c = S1.c OP S0.c; \
		else if(S0.cvalid()) \
			S1.c = S0.c; \
	} \
	if(S0.pvalid()) { \
		S1.clr_svalid(); \
		S1.set_pvalid(); \
	} \
	if(S0.cvalid()) { \
		S1.clr_svalid(); \
		S1.set_cvalid(); \
	} \
	SPOP; \
	if(cflag) \
		S0.set_constant(); \
	else \
		S0.clr_constant(); \
	if(aflag==0) \
		S0.clr_inactive(); \
	if(aflag==2 && (typeValue()==ID_MULTIPLY) || (typeValue()==ID_DIVIDE)) \
		S0.set_inactive(); \
	else \
		S0.clr_inactive();


#define LOGICAL_EVAL(OP) \
    int cflag=1; \
	left->eval(); \
	cflag=S0.constant()?cflag:0; \
	SPUSH; \
	right->eval(); \
	cflag=S0.constant()?cflag:0; \
	if(S1.svalid() && S0.svalid()) { \
		S1.s = (S1.s OP S0.s) ? 1.0:0.0; \
		S1.set_svalid(); \
	} \
	SPOP; \
	if(cflag) \
		S0.set_constant(); \
	else \
		S0.clr_constant()

//************************************************************
// TNexpr::eval()
//************************************************************
void TNadd::eval()  { BINARY_EVAL(+); }
void TNsub::eval()  { BINARY_EVAL(-); }
void TNmul::eval()  { BINARY_EVAL(*); }
void TNdiv::eval()  { BINARY_EVAL(/); }
void TNgt::eval()   { LOGICAL_EVAL(>);}
void TNlt::eval()  	{ LOGICAL_EVAL(<);}
void TNle::eval()   { LOGICAL_EVAL(<=);}
void TNge::eval()   { LOGICAL_EVAL(>=);}
void TNeq::eval()   { LOGICAL_EVAL(==);}
void TNne::eval()   { LOGICAL_EVAL(!=);}
void TNor::eval()   { LOGICAL_EVAL(||);}
void TNand::eval()  { LOGICAL_EVAL(&&);}

//-------------------------------------------------------------
// TNadd::valueString() set value string
//-------------------------------------------------------------
void TNadd::valueString(char *s)
{
	char lstr[MAXSTR];
	char rstr[MAXSTR];
	lstr[0]=rstr[0]=0;
	if(left){
		SINIT;
		left->eval();
		if(!CurrentScope->shader() || !S0.constant())
			left->valueString(lstr);
	}
	if(right){
		SINIT;
		right->eval();
		if(!CurrentScope->shader() || !S0.constant())
			right->valueString(rstr);
	}
	if(strlen(lstr)>0&&strlen(rstr)>0)
		sprintf(s+strlen(s),"%s+%s",lstr,rstr);
	else if(strlen(lstr)>0)
		strcat(s,lstr);
	else if(strlen(rstr)>0)
		strcat(s,rstr);
}

//-------------------------------------------------------------
// TNadd::valueString() set value string
//-------------------------------------------------------------
void TNsub::valueString(char *s)
{
	char lstr[MAXSTR];
	char rstr[MAXSTR];
	lstr[0]=rstr[0]=0;
	if(left){
		SINIT;
		left->eval();
		if(!CurrentScope->shader() || !S0.constant())
			left->valueString(lstr);
	}
	if(right){
		SINIT;
		right->eval();
		if(!CurrentScope->shader() || !S0.constant())
			right->valueString(rstr);
	}
	if(strlen(lstr)>0&&strlen(rstr)>0)
		sprintf(s+strlen(s),"%s-%s",lstr,rstr);
	else if(strlen(lstr)>0)
		sprintf(s+strlen(s),"%s",lstr);
	else if(strlen(rstr)>0)
		sprintf(s+strlen(s),"-%s",rstr);
}

//************************************************************
// Class TNarg
//************************************************************
TNarg::TNarg(TNode *l, TNode *p) : TNbinary(l,0)
{
	if(p)
		((TNarg*)p)->right=this;
}
//-------------------------------------------------------------
// TNarg::addToken() add to token list
//-------------------------------------------------------------
void TNarg::addToken(LinkedList<TNode*>&l)
{
	//TNunary::addToken(l);
	if(left)
		left->addToken(l);

	if(right)
		right->addToken(l);
}

//-------------------------------------------------------------
// index operator
//-------------------------------------------------------------
TNode *TNarg::operator[](int i)
{
	int n=0;
	TNarg *arg=this;
	while(arg){
	    if(n==i)
	        break;
		arg=arg->next();
		n++;
	}
	if(arg && n==i)
	    return arg->left;
	return 0;
}

//-------------------------------------------------------------
// index function
//-------------------------------------------------------------
TNarg *TNarg::index(int i)
{
	int n=0;
	TNarg *arg=this;
	while(arg){
	    if(n==i)
	        break;
		arg=arg->next();
		n++;
	}
	if(arg && n==i)
	    return arg;
	return 0;
}

//-------------------------------------------------------------
// TNarg::valueString() set value string
//-------------------------------------------------------------
void TNarg::valueString(char *s)
{
	setStart(s);
	left->valueString(s);
	setEnd(s);
}

//-------------------------------------------------------------
// TNarg::dump() print node info
//-------------------------------------------------------------
void TNarg::dump()
{
	printf("arg ");
	if(left)
		left->dump();
	if(right)
		right->dump();
}

//-------------------------------------------------------------
// TNarg::eval() evaluate the node
//-------------------------------------------------------------
void TNarg::eval()
{
	if(left){
		SINIT;
		left->eval();
	}
}

//-------------------------------------------------------------
// TNarg::save() save the node
//-------------------------------------------------------------
void TNarg::save(FILE *f)
{
	if(left)
		left->save(f);
	if(right){
		fprintf(f,",");
		right->save(f);
	}
}

//************************************************************
// Class TNfunc
//************************************************************

//-------------------------------------------------------------
// TNfunc::~TNfunc() destructor
//-------------------------------------------------------------
TNfunc::~TNfunc()
{
	DFREE(expr);
	DFREE(left);
	DFREE(right);
}

//-------------------------------------------------------------
// TNfunc::addToken() add to token list
//-------------------------------------------------------------
void TNfunc::addToken(LinkedList<TNode*>&l)
{
	l.add(this);
	if(right)
		right->addToken(l);
}

//-------------------------------------------------------------
// TNfunc::hasChildren()
//-------------------------------------------------------------
bool TNfunc::hasChildren()
{
	return right?true:false;
}

//-------------------------------------------------------------
// TNfunc::getChildren() add to child list
//-------------------------------------------------------------
int TNfunc::getChildren(LinkedList<NodeIF*>&l)
{
	if(right){
		l.add(right);
		return 1;
	}
	return 0;
}

//-------------------------------------------------------------
// TNfunc::save() archive the node
//-------------------------------------------------------------
void TNfunc::save(FILE *f)
{
	fprintf(f,"%s(",symbol());
	if(left)
		left->save(f);
	fprintf(f,")\n%s",tabs);
	if(right)
		right->save(f);
}

//-------------------------------------------------------------
// TNfunc::save() archive the node
//-------------------------------------------------------------
void TNfunc::saveNode(FILE *f)
{
	fprintf(f,"%s(",symbol());
	if(left)
		left->save(f);
	fprintf(f,")\n");
}

//-------------------------------------------------------------
// TNfunc::propertyString() write property nodes
//-------------------------------------------------------------
void TNfunc::propertyString(char *s)
{
	sprintf(s+strlen(s),"%s(",symbol());
	TNarg *arg=(TNarg*)left;
	while(arg){
		arg->valueString(s);
		arg=arg->next();
		if(arg)
			strcat(s,",");
	}
	strcat(s,")");
}

//-------------------------------------------------------------
// TNfunc::valueString() write value string
//-------------------------------------------------------------
void TNfunc::valueString(char *s)
{
    TNfunc *func=expr?expr:this;
	setStart(s);
    if(CurrentScope->tokens() && token[0])
        strcat(s,token);
    else{
		func->propertyString(s);
		//strcat(s,"\n");
	}
	setEnd(s);
	if(right)
		right->valueString(s);
}

//-------------------------------------------------------------
// TNfunc::removeChild
//-------------------------------------------------------------
NodeIF *TNfunc::removeChild(NodeIF *c){
	if(right==c)
	    right=0;
	return c;
}
//-------------------------------------------------------------
NodeIF *TNfunc::removeNode()
{
	NodeIF *p=getParent();
	if(p){
		if(right){
			p->replaceChild(this,right);
			right=0;
		}
		else
			p->removeChild(this);
	}
	return this;
}

//-------------------------------------------------------------
// TNfunc::addChild
//-------------------------------------------------------------
NodeIF *TNfunc::addChild(NodeIF *x){
    if(!right){
    	x->setParent(this);
    	right=(TNode*)x;
    }
    else if(x->linkable()){
    	x->setParent(this);
    	x->addChild(right);
    	right=(TNode*)x;
    }
    else
        right->addChild(x);
    return x;
}

//-------------------------------------------------------------
// TNfunc::setExpr() set expr string
//-------------------------------------------------------------
void TNfunc::setExpr(char *c)
{
	if(strlen(c)==0)
		return;
	TNfunc *n=(TNfunc*)TheScene->parse_node(c);
	if(n){
	    DFREE(expr);
	    expr=n;
	    expr->setParent(parent);
	}
}

//-------------------------------------------------------------
// TNfunc::applyExpr() apply expr value
//-------------------------------------------------------------
void TNfunc::applyExpr()
{
    if(expr){
        DFREE(left);
        left=expr->left;
        left->setParent(this);
        expr=0;
    }
    if(right)
        right->applyExpr();
}

//-------------------------------------------------------------
// TNfunc::clearExpr() clear expr value
//-------------------------------------------------------------
void TNfunc::clearExpr()
{
    DFREE(expr);
    if(right)
        right->clearExpr();
}


//************************************************************
// TNbase class
//************************************************************
TNbase::TNbase(int t, TNode *l, TNode *r, TNode *b) :  TNfunc(l,r)
{
 	type=t;
	base=b;
	if(base)
		base->setParent(this);
}

TNbase::~TNbase()
{
    DFREE(base);
}

//-------------------------------------------------------------
// TNbase::visit() apply visit function
//-------------------------------------------------------------
void TNbase::visit(void (*func) (TNode *)){
    (*func)(this);
    if(left)
        left->visit(func);
    if(base)
    	base->visit(func);
    if(right)
        right->visit(func);
}

//-------------------------------------------------------------
// TNbase::init() apply init function
//-------------------------------------------------------------
void TNbase::init()
{
    if(left)
        left->init();
    if(base)
    	base->init();
    if(right)
        right->init();
}

//-------------------------------------------------------------
// TNbase::optionString() write options
//-------------------------------------------------------------
int TNbase::optionString(char *s)
{
	return 0;
}

//-------------------------------------------------------------
// TNbase::propertyString() write property nodes
//-------------------------------------------------------------
void TNbase::propertyString(char *s)
{
	sprintf(s+strlen(s),"%s(",symbol());
	char opts[64];
	opts[0]=0;
	if(optionString(opts)){
		sprintf(s+strlen(s),"%s",opts);
		if(left)
			strcat(s,",");
	}
	TNarg *arg=(TNarg*)left;
	while(arg){
		arg->valueString(s);
		arg=arg->next();
		if(arg)
			strcat(s,",");
	}
	strcat(s,")");
}

//-------------------------------------------------------------
// TNbase::valueString() write value string
//-------------------------------------------------------------
void TNbase::valueString(char *s)
{
    TNbase *value=this;
 	setStart(s);
	if(CurrentScope->tokens() && token[0])
		strcat(s,token);
	else{
		if(expr)
			value=(TNbase*)expr;
		value->propertyString(s);
		if(base){
			strcat(s,"[");
			base->valueString(s);
			strcat(s,"]\n");
		}
	}
	setEnd(s);
	if(right)
		right->valueString(s);
}

//-------------------------------------------------------------
// TNbase::save() save the node
//-------------------------------------------------------------
void TNbase::save(FILE *f)
{
	fprintf(f,"%s(",symbol());
	char opts[64];
	opts[0]=0;
	if(optionString(opts))
		fprintf(f,"%s,",opts);
	if(left)
		left->save(f);
	fprintf(f,")");
	if(base){
		fprintf(f,"[");
		base->save(f);
		fprintf(f,"]");
	}
	if(right){
		fprintf(f,"\n%s",tabs);
		right->save(f);
	}
}
//-------------------------------------------------------------
// TNbase::saveNode save the node
//-------------------------------------------------------------
void TNbase::saveNode(FILE *f)
{
	fprintf(f,"%s(",symbol());
	char opts[64];
	opts[0]=0;
	if(optionString(opts))
		fprintf(f,"%s,",opts);
	if(left)
		left->save(f);
	fprintf(f,")");
	if(base){
		inc_tabs();
		addtabs=1;
		fprintf(f,"[");
		base->save(f);
		dec_tabs();
		fprintf(f,"]");
		addtabs=0;
	}
}

//-------------------------------------------------------------
// TNbase::removeChild
//-------------------------------------------------------------
NodeIF *TNbase::removeChild(NodeIF *c){
	if(right==c)
	    right=0;
	else if(base==c)
	    base=0;
    else
        return 0;
	return c;
}

//-------------------------------------------------------------
// TNbase::replaceChild
//-------------------------------------------------------------
NodeIF *TNbase::replaceChild(NodeIF *c, NodeIF *n)
{
	TNbase *r1,*r2;
 	if(right==c){
		right=(TNode*)n;
		right->setParent(this);
	}
	// ------------------------------------------------------
 	else if(base==c){
		if((n->typeValue()&ID_BASE)&&(n->typeValue()==c->typeValue())){
			r1=(TNbase*)c;
			r2=(TNbase*)n;
			r2->right=r1->right;
			if(r2->right)
				r2->right->setParent(r2);
			r1->right=0;
			if(r1->base && !r2->base){
				r2->base=r1->base;
				r1->base->setParent(r2);
				r1->base=0;
			}
		}
	// ------------------------------------------------------
	    base=(TNode*)n;
		base->setParent(this);
	}
	return c;
}
//-------------------------------------------------------------
// TNbase::hasChildren()
//-------------------------------------------------------------
bool TNbase::hasChildren()
{
	return (base||right)?true:false;
}

//-------------------------------------------------------------
// TNbase::getChildren
//-------------------------------------------------------------
static void set_branch(TNode *n)
{
    n->setFlag(NODE_BRANCH);
}
static void clr_branch(TNode *n)
{
    n->clrFlag(NODE_BRANCH);
}
int TNbase::getChildren(LinkedList<NodeIF*>&l)
{
	int i=0;
	if(base){
	    base->visit(set_branch);
		l.add(base);
		i++;
	}
	if(right){
	    right->visit(clr_branch);
		l.add(right);
		i++;
	}
	return i;
}

//-------------------------------------------------------------
// TNbase::addChild
//-------------------------------------------------------------
NodeIF *TNbase::addChild(NodeIF *x){
	TNode *node=(TNode*)x;
	if(base && expanded()){
		if(node->typeValue() & ID_FUNC){
			TNfunc *n=(TNfunc*)node;
			n->TNfunc::addChild(base);
		}
		else{
			node=new TNadd(node,base);
		}
		node->setParent(this);
		base=node;
		set_branch(node);
    }
	else
		TNfunc::addChild(x);
	return x;
}

//-------------------------------------------------------------
// TNbase::applyExpr() apply expr value
//-------------------------------------------------------------
void TNbase::applyExpr()
{
    if(expr){
        DFREE(left);
        left=expr->left;
        left->setParent(this);
        expr=0;
    }
    if(base)
        base->applyExpr();
    if(right)
        right->applyExpr();
}

//-------------------------------------------------------------
// TNbase::clearExpr() clear expr value
//-------------------------------------------------------------
void TNbase::clearExpr()
{
    DFREE(expr);
    if(base)
        base->clearExpr();
    if(right)
        right->clearExpr();
}
//************************************************************
// Class TNsubr
//************************************************************
TNsubr::~TNsubr()
{
	if(name)
		delete name;
	name=0;
}

//-------------------------------------------------------------
// TNsubr::save() save the node
//-------------------------------------------------------------
void TNsubr::save(FILE *f)
{
	fprintf(f,"%s(",name);
	if(right)
		TNunary::save(f);
	fprintf(f,")");
}
//-------------------------------------------------------------
// TNsubr::addToken() forward to args
//-------------------------------------------------------------
void TNsubr::addToken(LinkedList<TNode*>&l)
{
	if(right)
		right->addToken(l);
}

//-------------------------------------------------------------
// TNsubr::eval() base class evaluate
//-------------------------------------------------------------
void TNsubr::eval()
{
}
//############ begin local subr classes #####################

enum  {
	SUB_PZ,
	SUB_ABS,
	SUB_FLOOR,
	SUB_FRACT,
	SUB_CLAMP,
	SUB_MIN,
	SUB_MAX,
	SUB_POW,
	SUB_SCURVE,
	SUB_MOD,
	SUB_SIN,
	SUB_COS,
	SUB_SQRT,
	SUB_STEP,
	SUB_RAMP,
	SUB_LERP,
	SUB_SPLINE,
	SUB_RAND,
	SUB_SRAND,
	SUB_TWIST,
	SUB_GAUSS,
};

static LongSym subtypes[]={
	{"pz",	   		SUB_PZ},
	{"abs",			SUB_ABS},
	{"floor",	    SUB_FLOOR},
	{"fract",	    SUB_FRACT},
	{"clamp",	    SUB_CLAMP},
	{"min",			SUB_MIN},
	{"max",			SUB_MAX},
	{"mod",			SUB_MOD},
	{"pow",			SUB_POW},
	{"scurve",		SUB_SCURVE},
	{"sin",			SUB_SIN},
	{"cos",			SUB_COS},
	{"sqrt",		SUB_SQRT},
	{"step",		SUB_STEP},
	{"ramp",		SUB_RAMP},
	{"lerp",		SUB_LERP},
	{"spline",		SUB_SPLINE},
	{"rand",		SUB_RAND},
	{"srand",		SUB_SRAND},
	{"twist",		SUB_TWIST},
	{"gauss",		SUB_GAUSS},

};
NameList<LongSym*> Subs(subtypes,sizeof(subtypes)/sizeof(LongSym));


//************************************************************
// Class TNrand
//************************************************************
class TNrand : public TNsubr
{
public:
	TNrand(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval(){
		S0.s=Rand();
		S0.set_svalid();
	}
};
//************************************************************
// Class TNsrand
//************************************************************
class TNsrand : public TNsubr
{
public:
	TNsrand(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval(){
		if(CurrentScope->shader())
			S0.set_inactive();
		else if(CurrentScope->texture())
			S0.clr_inactive();
		else{
			S0.s=SRand();
			S0.set_svalid();
		}
	}
	void valueString(char *s){
		INIT;
		eval();
		if(!S0.inactive())
			TNsubr::valueString(s);
	}
};
//************************************************************
// Class TNpz
//************************************************************
class TNpz : public TNsubr
{
public:
	TNpz(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};

//-------------------------------------------------------------
// TNpz::eval() evaluate the node
//-------------------------------------------------------------
void TNpz::eval()
{
	TNsubr::eval();
	SINIT;
	right->eval();
	S0.s=S0.p.z;
	S0.set_svalid();
}

//************************************************************
// Class TNabs
//************************************************************
class TNabs : public TNsubr
{
public:
	TNabs(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};

//-------------------------------------------------------------
// TNabs::eval() evaluate the node
//-------------------------------------------------------------
void TNabs::eval()
{
	TNsubr::eval();
	SINIT;
	right->eval();
	if(S0.s<0)
		S0.s=-S0.s;
	S0.set_svalid();
}

//************************************************************
// Class TNfloor
//************************************************************
class TNfloor : public TNsubr
{
public:
	TNfloor(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};

//-------------------------------------------------------------
// TNfloor::eval() evaluate the node
//-------------------------------------------------------------
void TNfloor::eval()
{
	TNsubr::eval();
	if(right){
		SINIT;
		right->eval();
		S0.s=floor(S0.s);
		S0.set_svalid();
	}
}

//************************************************************
// Class TNfract
//************************************************************
class TNfract : public TNsubr
{
public:
	TNfract(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};

//-------------------------------------------------------------
// TNfract::eval() evaluate the node
//-------------------------------------------------------------
void TNfract::eval()
{
	TNsubr::eval();
	if(right){
		SINIT;
		right->eval();
		S0.s-=floor(S0.s);
		S0.set_svalid();
	}
}

//************************************************************
// Class TNstep
//************************************************************
class TNstep : public TNsubr
{
public:
	TNstep(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};

//-------------------------------------------------------------
// TNstep::eval() evaluate the node
//-------------------------------------------------------------
void TNstep::eval()
{
	double arg[6];
	SINIT;
	int n=getargs(right,arg,6);
	if(n==3)
		S0.s=smoothstep(arg[0],arg[1],arg[2]);
	else if(n==5)
		S0.s=smoothstep(arg[0],arg[1],arg[2],arg[3],arg[4]);
	S0.set_svalid();
	TNsubr::eval();
}

//************************************************************
// Class TNramp
//************************************************************
class TNramp : public TNsubr
{
public:
	TNramp(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNramp::eval() evaluate the node
//-------------------------------------------------------------
void TNramp::eval()
{
	double arg[5];
	INIT;

	int n=getargs(right,arg,5);
	if(n==5)
		S0.s=rampstep(arg[0],arg[1],arg[2],arg[3],arg[4]);
	else
		S0.s=rampstep(arg[0],arg[1],arg[2]);

	S0.set_svalid();
	TNsubr::eval();
}

//************************************************************
// Class TNlerp
//************************************************************
class TNlerp : public TNsubr
{
public:
	TNlerp(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
	void valueString(char *s){
		INIT;
		eval();
		if(!S0.inactive())
			TNsubr::valueString(s);
	}
};
//-------------------------------------------------------------
// TNramp::eval() evaluate the node
//-------------------------------------------------------------
void TNlerp::eval()
{
	double arg[5];
	//INIT;
	//int old_flag=Td.get_flag(SNOISEFLAG);
	//Td.clr_flag(SNOISEFLAG);
	getargs(right,arg,5);
	if(CurrentScope->shader() && !Td.get_flag(SNOISEFLAG))
		S0.set_inactive();
	else if (CurrentScope->texture() && Td.get_flag(SNOISEFLAG))
		S0.set_inactive();
	else{
		S0.s=rampstep(arg[1],arg[2],arg[0],arg[3],arg[4]);
		S0.set_svalid();
	}
	//Td.set_flag(old_flag);
}

//************************************************************
// Class TNsin
//************************************************************
class TNsin : public TNsubr
{
public:
	TNsin(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};

//-------------------------------------------------------------
// TNsin::eval() evaluate the node
//-------------------------------------------------------------
void TNsin::eval()
{
	extern double lsin(double);
	TNsubr::eval();
	SINIT;
	right->eval();
	//S0.s=umod(S0.s,PIX2);
	//S0.s=lsin(S0.s);
	S0.s=sin(S0.s);
	S0.set_svalid();
}

//************************************************************
// Class TNcos
//************************************************************
class TNcos : public TNsubr
{
public:
	TNcos(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNcos::eval() evaluate the node
//-------------------------------------------------------------
void TNcos::eval()
{
	extern double lcos(double);
	TNsubr::eval();
	SINIT;
	right->eval();
	//S0.s=umod(S0.s,PIX2);
	//S0.s=lcos(S0.s);
	S0.s=cos(S0.s);
	S0.set_svalid();
}

//************************************************************
// Class TNsqrt
//************************************************************
class TNsqrt : public TNsubr
{
public:
	TNsqrt(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNsqrt::eval() evaluate the node
//-------------------------------------------------------------
void TNsqrt::eval()
{
	TNsubr::eval();
	SINIT;
	right->eval();
	if(S0.s>=0)
		S0.s=sqrt(S0.s);
	else
		S0.s=-sqrt(-S0.s);
	S0.set_svalid();
}
//************************************************************
// Class TNmin
//************************************************************
class TNmin : public TNsubr
{
public:
	TNmin(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNmin::eval() evaluate the node
//-------------------------------------------------------------
void TNmin::eval()
{
	double arg[3];
	SINIT;
	getargs(right,arg,3);
	S0.s=arg[0]<arg[1]?arg[0]:arg[1];
	S0.set_svalid();
	TNsubr::eval();
}

//************************************************************
// Class TNmax
//************************************************************
class TNmax : public TNsubr
{
public:
	TNmax(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNmax::eval() evaluate the node
//-------------------------------------------------------------
void TNmax::eval()
{
	double arg[3];
	//cout<<S0.s<<endl;
	SINIT;

	int n=getargs(right,arg,3);
	if(n==3){
		S0.s=arg[2]<arg[1]?arg[2]:arg[0];
	}
	else
		S0.s=arg[0]>arg[1]?arg[0]:arg[1];
	S0.set_svalid();
	TNsubr::eval();
}

//************************************************************
// Class TNtwist
//************************************************************
class TNtwist : public TNsubr
{
public:
	TNtwist(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval(){
		TNarg &args=*((TNarg *)right);
		if(args[0]){
			if(CurrentScope->rpass())
				Td.set_flag(COMPLEX);
			args[0]->eval();
			Noise::distort(NTWIST,S0.s);
			S0.s=0;
			if(args[1])
				args[1]->eval();
			S0.set_svalid();
			Noise::pop();
		}
	}
	void valueString(char *s);
};
void TNtwist::valueString(char *s){
	char arg1[MAXSTR]={0};
	char arg2[MAXSTR]={0};

	TNarg &args=*((TNarg *)right);
	args[0]->valueString(arg1);
	args[1]->valueString(arg2);
	if(strlen(arg1) && strlen(arg2))
		TNsubr::valueString(s);
	else if(strlen(arg1)==0 && strlen(arg2)==0)
		return;
	else if(strlen(arg1)==0 )
		strcpy(s,arg2);
	else{
		if(strlen(arg2)==0)
			strcpy(arg2,"1");
		sprintf(s+strlen(s),"%s(%s,%s)",symbol(),arg1,arg2);
	}
}

//************************************************************
// Class TNclamp
//************************************************************
class TNclamp : public TNsubr
{
public:
	TNclamp(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNclamp::eval() evaluate the node
//-------------------------------------------------------------
void TNclamp::eval()
{
	double arg[3];
	SINIT;
	getargs(right,arg,3);
	S0.s=clamp(arg[0],arg[1],arg[2]);
	S0.set_svalid();
	TNsubr::eval();
}

//************************************************************
// Class TNmod
//************************************************************
class TNmod : public TNsubr
{
public:
	TNmod(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNmod::eval() evaluate the node
//-------------------------------------------------------------
void TNmod::eval()
{
	double arg[2];
	SINIT;
	getargs(right,arg,2);
	S0.s=umod(arg[0],arg[1]);
	S0.set_svalid();
	TNsubr::eval();
}

//************************************************************
// Class TNscurve
//************************************************************
class TNscurve : public TNsubr
{
public:
	TNscurve(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNscurve::eval() evaluate the node
// arg[0]: expected input -1..1 or 0..1
// arg[1]: sharpness factor (k)
// output: range same as input but sigmoidal or exponential
//         k=0:linear, k= -0.9:very sharp k=0.9:very wide
//-------------------------------------------------------------
void TNscurve::eval()
{
	double k=0.0,b=0;
	double arg[3];
	SINIT;
	int n=getargs(right,arg,3);
	double x=arg[0];
    if(n>1){
    	k=arg[1];
    	if(n>2)
    		b=arg[2];
    }
    x=clamp(x,-1,1);

	S0.s=b+(x-k*x)/(k-2*k*fabs(x)+1);
	//S0.s=1.0/(1.0+exp(-x*k)); // logistic function

	S0.set_svalid();
	TNsubr::eval();
}

//************************************************************
// Class TNpow
//************************************************************
class TNpow : public TNsubr
{
public:
	TNpow(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};
//-------------------------------------------------------------
// TNpow::eval() evaluate the node
//-------------------------------------------------------------
void TNpow::eval()
{
	double arg[2];
	SINIT;
	getargs(right,arg,2);
	int sgn=arg[0]<0?-1:1;
	S0.s=sgn*pow(fabs(arg[0]),arg[1]);
	S0.set_svalid();
	TNsubr::eval();
}

//************************************************************
// Class TNspline
//************************************************************
class TNspline : public TNsubr
{
public:
	TNspline(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};

//-------------------------------------------------------------
// TNspline::eval() evaluate the node
//-------------------------------------------------------------
void TNspline::eval()
{
	double arg[64];
	SINIT;
	int n=getargs(right,arg,64)-1;
	S0.s=spline(arg[0],n,arg+1);
	S0.set_svalid();
}

//************************************************************
// Class TNgauss
//************************************************************
class TNgauss : public TNsubr
{
public:
	TNgauss(char* s, TNode *r) :  TNsubr(s, r) {}
	void eval();
};

//-------------------------------------------------------------
// TNgauss::eval() evaluate the node
//-------------------------------------------------------------
void TNgauss::eval()
{
	double arg[3];
	SINIT;
	int n=getargs(right,arg,3);
	double x=arg[0];
	double m=arg[1];
	double s=arg[2];
	double a=1;///sqrt(2*PI)/s;
	S0.s=a*exp(-0.5*(x-m)*(x-m)/s/s);
	//cout<<"x:"<<x<<" g:"<<S0.s<<endl;
	S0.set_svalid();
}
//-------------------------------------------------------------
// subr_expr() return subroutine node
//-------------------------------------------------------------
TNsubr *subr_node(char *s, TNode *args)
{
	LongSym *ls;
    if((ls=Subs.inlist(s))>0){
		switch(ls->value){
		case SUB_PZ: 		return new TNpz(s, args);
		case SUB_ABS: 		return new TNabs(s, args);
		case SUB_MIN: 		return new TNmin(s, args);
		case SUB_MAX: 		return new TNmax(s, args);
		case SUB_FLOOR: 	return new TNfloor(s,args);
		case SUB_FRACT: 	return new TNfract(s,args);
		case SUB_POW:   	return new TNpow(s,args);
		case SUB_SCURVE:   	return new TNscurve(s,args);
		case SUB_MOD:   	return new TNmod(s,args);
		case SUB_SIN:   	return new TNsin(s,args);
		case SUB_COS:   	return new TNcos(s,args);
		case SUB_SQRT:  	return new TNsqrt(s,args);
		case SUB_CLAMP: 	return new TNclamp(s,args);
		case SUB_RAMP:  	return new TNramp(s,args);
		case SUB_LERP:  	return new TNlerp(s,args);
		case SUB_STEP:  	return new TNstep(s,args);
		case SUB_SPLINE:	return new TNspline(s,args);
		case SUB_RAND:		return new TNrand(s,args);
		case SUB_SRAND:		return new TNsrand(s,args);
		case SUB_TWIST:		return new TNtwist(s,args);
		case SUB_GAUSS:   	return new TNgauss(s,args);

		}
	}
	printf("error: unknown function %s\n",s);
	return 0;
}

//############ end local classes #############################
