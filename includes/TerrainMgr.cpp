
#include "TerrainData.h"
#include "NodeIF.h"
#include "TerrainMgr.h"
#include "ImageMgr.h"
#include "TextureClass.h"
#include "ImageClass.h"
#include "SceneClass.h"
#include <iostream>
#include <math.h>

//#define DEBUG_EXPR_VARS  // show var status on save
//#define DEBUG_TD_VARS  // show var status on save
//#define DEBUG_INIT  // show var status on save
//#define DEBUG_IMAGES
//#define DEBUG_VALIDATE
extern void sx_error(char *msg,...);
extern double Rand();

extern void inc_tabs();
extern void dec_tabs();
extern char tabs[];
extern double Pscale,Gscale,Drop,Margin;

extern int hits,visits,misses;

// externs used in TerrainClass.cpp

Point           MapPt;
double          Theta,Phi,Temp=0,Tave=0,Tsol=0,Tgas=0,Sfact=0,MapTemp=0,Height=0.0,PX,PY,Radius=0,Density=0,Range=0,MaxHt=0,MinHt=0,FHt=0,Randval=0,Srand=0,Level=0;
Scope          *CurrentScope;

void NodeIF::setRands(){
	if(!fixed_rands)
	    lastn=getRandValue()*1234;
	lastn++;
	//cout<<lastn<<endl;
	for(int i=0;i<RANDS;i++){
		r[i]=URAND(lastn++);
		s[i]=RAND(lastn++);
	}
}
bool NodeIF::fixed_rands=false;
int NodeIF::nsave=0;
int NodeIF::ncount=0;

double TNvalue(TNode *n)
{
	INIT;
	n->eval();
	delete n;
	return S0.s;
}

static 	        TerrainData Td;

//************************************************************
// TNvar class
//************************************************************
TNvar::TNvar(char *n)  : TNunary(0)			{
	_name=n;
	expr=0;
	flags=SHOW_FLAG|EVAL_FLAG;
}
TNvar::TNvar(char *n, TNode *e)   : TNunary(e)	{
	_name=n;
	expr=0;
	flags=SHOW_FLAG|EVAL_FLAG;
	if(e)
	    e->setParent(this);
}

//-------------------------------------------------------------
// TNvar::init() initialize the node
//-------------------------------------------------------------
TNvar::~TNvar()
{
	FREE(_name)
	DFREE(expr);
}

//-------------------------------------------------------------
// TNvar::addToken() add to token list
//-------------------------------------------------------------
void TNvar::addToken(LinkedList<TNode*>&l)
{
	TNunary::addToken(l);
}

//-------------------------------------------------------------
// TNode::setValueString() replace value subtree
//        if self-replaced, return new obj else this
//-------------------------------------------------------------
NodeIF *TNvar::setValueString(char *s)
{
    NodeIF *oldval=getValue();
    if(!oldval) 		// no replacable value component
    	return 0;
	NodeIF *repnode=TheScene->parse_node(s);
	if(!repnode)
		return 0;		// invalid expression string
	setValue(repnode->getValue());
	delete oldval;
	return this;
}

//-------------------------------------------------------------
// TNvar::init() initalize the node
//-------------------------------------------------------------
void TNvar::init()
{
	INIT;
	setFlag(NODE_HIDE);
	TNunary::init();
}

//-------------------------------------------------------------
// TNvar::eval() evaluate the node
//-------------------------------------------------------------
void TNvar::eval()
{
	INIT;
	S0.image=0;
	TerrainSym *ts=0;

	if(CurrentScope->shader()){
		S0.set_inactive();
		return;
	}
	if(CurrentScope->texture()){
		S0.clr_inactive();
		return;
	}

    if(right){
		TNode *n=right;
		if(CurrentScope->preview() && expr)
		    n=expr;
		S0.image=0;
		n->eval();
		ts=CurrentScope->set_var(_name,S0);
		ts->set_constant(S0.constant());
		ts->set_reval(0);
	}
	else{
		ts=CurrentScope->get_var(_name,S0);
		if(ts){
			if(ts->reval()){
				TNvar *exp=CurrentScope->get_expr(_name);
				if(exp)
		    		exp->eval();
			}
			S0.clr_constant();
		}
		else{
		    SINIT;
		}
	}
}

//-------------------------------------------------------------
// TNvar::saveNode() save the node
//-------------------------------------------------------------
void TNvar::saveNode(FILE *f)
{
    if(CurrentScope){
	    TNvar *var=CurrentScope->get_expr(_name);
        if(var){
        	TNode *n=var->right;
        	if(n)
            	n->saveNode(f);
        }
    }
}

//-------------------------------------------------------------
// TNvar::save() save the node
//-------------------------------------------------------------
void TNvar::save(FILE *f)
{
    if(CurrentScope && !TheScene->keep_variables()){
	    TNvar *var=CurrentScope->get_expr(_name);
        if(var){
        	TNode *n=var->right;
        	if(n)
            	n->save(f);
        }
        return;
    }
	if(right){
	    NodeIF *p=getParent();
	    if(p && (p->typeValue()&ID_MGR)){
			fprintf(f,"%s%s=",tabs,_name);
			TNunary::save(f);
			fprintf(f,";\n");
		}
		else{
			fprintf(f,"%s=",_name);
			TNunary::save(f);
		}
	}
	else
		fprintf(f,_name);
}

//-------------------------------------------------------------
// TNvar::valueString() make value string
//-------------------------------------------------------------
void TNvar::valueString(char *s)
{
	eval();
	if(CurrentScope->shader()){
	//if(S0.inactive()){
		return;
	}
	setStart(s);
    if(CurrentScope && !TheScene->keep_variables()){
	    TNvar *var=CurrentScope->get_expr(_name);
        if(var){
        	TNode *n=var->right;
        	if(n)
            	n->valueString(s);
        }
		else
			sprintf(s+strlen(s),_name);
    }
    else{
		if(right){
		    if(expr)
				expr->valueString(s);
		    else
			    right->valueString(s);
		}
		else
			sprintf(s+strlen(s),_name);
	}
	setEnd(s);
}

//-------------------------------------------------------------
// TNvar::removeChild() remove expression
//-------------------------------------------------------------
NodeIF *TNvar::removeChild(NodeIF *c)
{
	return removeNode();
}

//-------------------------------------------------------------
// TNvar::addChild
//-------------------------------------------------------------
NodeIF *TNvar::addChild(NodeIF *x)
{
	if(x->linkable()){
    	x->setParent(this);
    	x->addChild(right);
    	right=(TNode*)x;
    }
    else {
        TNadd *node=new TNadd((TNode*)x,right);
        node->setParent(this);
        right=node;
	}
	return x;
}

//-------------------------------------------------------------
// TNvar::setExpr() set expr string
//-------------------------------------------------------------
void TNvar::setExpr(char *c)
{
	TNode *n=(TNode*)TheScene->parse_node(c);
	if(n){
	    DFREE(expr);
	    expr=n;
	    expr->setParent(this);
		expr->eval();
	}
}

//-------------------------------------------------------------
// TNvar::applyExpr() apply expr value
//-------------------------------------------------------------
void TNvar::applyExpr()
{
    if(right && expr){
        DFREE(right);
        right=expr;
        expr=0;
        eval();
    }
}

//-------------------------------------------------------------
// TNvar::getExprNode() return alt. expr. node
//-------------------------------------------------------------
TNode *TNvar::getExprNode()
{
	return expr;
}

//-------------------------------------------------------------
// TNvar::clearExpr() clear expr value
//-------------------------------------------------------------
void TNvar::clearExpr()
{
    if(right){
        DFREE(expr);
        eval();
    }
}

//************************************************************
// TNroot class
//************************************************************
//-------------------------------------------------------------
// TNroot::save() archive
//-------------------------------------------------------------

TNroot::TNroot(char *c, TNode *r) : TNvar(c,r)
{
}

TNroot::TNroot(TNode *r) : TNvar(0,r)
{
	MALLOC(16,char,_name);
	strcpy(_name,"terrain");
}

void TNroot::save(FILE *f)
{
	fprintf(f,"%s%s=",tabs,typeName());
	inc_tabs();
	inc_tabs();
	if(right)
		right->save(f);
	fprintf(f,";\n");
	dec_tabs();
	dec_tabs();
}
//-------------------------------------------------------------
// TNroot::removeChild() remove expression
//-------------------------------------------------------------
NodeIF *TNroot::removeChild(NodeIF *c)
{
	return TNunary::removeChild(c);
}

//-------------------------------------------------------------
// TNroot::addChild
//-------------------------------------------------------------
NodeIF *TNroot::addChild(NodeIF *x)
{
	if(!right){
		right=x;
		x->setParent(this);
	}
	else if(x->linkable()){
    	x->setParent(this);
    	x->addChild(right);
    	right=(TNode*)x;
    }
	else if(right->linkable()){
    	x->setParent(right);
    	right->addChild(x);
    }
    else {
        TNadd *node=new TNadd((TNode*)x,right);
        node->setParent(this);
        right=node;
	}
	return x;
}

//-------------------------------------------------------------
// TNroot::addAfter
//-------------------------------------------------------------
NodeIF *TNroot::addAfter(NodeIF *a, NodeIF *n)	{
	if(a)
		a->addChild(n);
	else
		addChild(n);
	return this;
}

//************************************************************
// Scope class
//************************************************************

Scope::~Scope()
{
	free();
}
TNvar *Scope::get_expr(char *s){ return 0;}
void Scope::applyVarExprs(){}
void Scope::clearVarExprs(){}
void Scope::revaluate(){}
void Scope::setPreviewMode(){}
void Scope::setAdaptMode(){}
void Scope::setStandardMode(){}

//-------------------------------------------------------------
// Scope::free() remove vars
//-------------------------------------------------------------
void Scope::free()
{
	vars.free();
}

//-------------------------------------------------------------
// Scope::set_units() set variable units
//-------------------------------------------------------------
TerrainSym *Scope::set_units(const char *s,const char *u)
{
	TerrainSym *v=vars.inlist((char*)s);
	if(v){
		v->units=u;
		return v;
	}
	return 0;
}

//-------------------------------------------------------------
// Scope::free_var() remove variable
//-------------------------------------------------------------
void Scope::free_var(char *s)
{
	TerrainSym *ts=vars.inlist(s);
	if(ts)
		vars.remove(ts);
}

//-------------------------------------------------------------
// Scope::set_show() show/hide variable
//-------------------------------------------------------------
void Scope::set_show(char *s, int b)
{
	TerrainSym *ts=vars.inlist(s);
	if(ts)
		ts->set_show(b);
}
//-------------------------------------------------------------
// Scope::set_eval() eval variable expr
//-------------------------------------------------------------
void Scope::set_eval(char *s, int b)
{
	TerrainSym *ts=vars.inlist(s);
	if(ts)
		ts->set_eval(b);
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(char *s, TerrainData &f)
{
	TerrainSym *ts=vars.inlist(s);
	if(ts){
		if(ts->do_show() && ts->do_eval())
		    ts->value=f;
	}
	else{
		ts=new TerrainSym(s,f);
		vars.add(ts);
		vars.sort();
	}
	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(const char *s, char *d, int b)
{
	TerrainSym *ts=set_var(s,d);
	if(ts)
	    ts->set_show(b);
	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(const char *s, char *d)
{
	TerrainSym *ts=vars.inlist((char*)s);
	if(!ts){
		ts=new TerrainSym(s,S0);
		vars.add(ts);
		vars.sort();
	}
	ts->value.InitS();
	ts->value.string=d;
	ts->value.set_strvalid();
	ts->value.set_constant();
	ts->set_eval(0);

	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::hide_var(const char *s)
{
	TerrainSym *ts=get_var(s);
	if(ts)
	    ts->set_show(false);
	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(const char *s, double d, int b)
{
	TerrainSym *ts=set_var(s,d);
	if(ts)
	    ts->set_show(b);
	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(const char *s, double d)
{
	TerrainSym *ts=vars.inlist((char*)s);
	if(!ts){
		ts=new TerrainSym(s,S0);
		vars.add(ts);
		vars.sort();
	}
	ts->value.InitS();
	ts->value.s=d;
	ts->value.set_svalid();
	ts->value.set_constant();
	ts->set_eval(0);

	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(const char *s, Point p, int b)
{
	TerrainSym *ts=set_var(s,p);
	if(ts)
	    ts->set_show(b);
	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(const char *s, Point p)
{
	TerrainSym *ts=vars.inlist((char*)s);
	if(!ts){
		ts=new TerrainSym(s,S0);
		vars.add(ts);
		vars.sort();
	}
	ts->value.InitS();
	ts->value.p=p;
	ts->value.set_pvalid();
	ts->value.set_constant();
	ts->set_eval(0);
	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(const char *s, Color c, int b)
{
	TerrainSym *ts=set_var(s,c);
	if(ts)
	    ts->set_show(b);
	return ts;
}

//-------------------------------------------------------------
// Scope::set_var() set variable value
//-------------------------------------------------------------
TerrainSym *Scope::set_var(const char *s, Color c)
{
	TerrainSym *ts=vars.inlist((char*)s);
	if(!ts){
		ts=new TerrainSym(s,S0);
		vars.add(ts);
		vars.sort();
	}
	ts->value.InitS();
	ts->value.c=c;
	ts->value.set_cvalid();
	ts->value.set_constant();
	ts->set_eval(0);

	return ts;
}

//-------------------------------------------------------------
// Scope::get_var() get variable
//-------------------------------------------------------------
TerrainSym *Scope::get_var(char *s)
{
	TerrainSym *ts=vars.inlist(s);
	if(ts)
		return ts;
	if(parent)
		return parent->get_var(s);
	return 0;
}

//-------------------------------------------------------------
// Scope::get_var() get variable value
//-------------------------------------------------------------
TerrainSym *Scope::get_var(char *s, TerrainData &f)
{
	TerrainSym *ts=vars.inlist(s);
	if(ts){
		f=ts->value;
		return ts;
	}
	if(parent)
		return parent->get_var(s,f);
	f.InitAll();
	return 0;
}

//-------------------------------------------------------------
// Scope::get_local() get variable value
//-------------------------------------------------------------
TerrainSym *Scope::get_local(const char *s)
{
	TerrainSym *ts=vars.inlist((char*)s);
	if(ts)
		return ts;
	return 0;
}

//-------------------------------------------------------------
// Scope::get_local() get variable value
//-------------------------------------------------------------
TerrainSym *Scope::get_local(const char *s, TerrainData &f)
{
	TerrainSym *ts=vars.inlist((char*)s);
	if(ts){
		f=ts->value;
		ts->set_eval(0);
		return ts;
	}
	f.InitAll();
	return 0;
}

//-------------------------------------------------------------
// Scope::save() archive function
//-------------------------------------------------------------
void Scope::save(FILE *f)
{
	CurrentScope=this;
	TerrainSym *v;
	vars.ss();
	for(int i=0;i<vars.size;i++){
		v=vars[i];
		v->save(f);
	}
}

//-------------------------------------------------------------
// Scope::eval_node() evaluate variable expr
//-------------------------------------------------------------
void Scope::eval_node(TNode *node)
{
 	CurrentScope=this;
	Td.reset();
	node->eval();
}

//************************************************************
// ExprMgr class
//************************************************************

ExprMgr::~ExprMgr()
{
	free();
}

//-------------------------------------------------------------
// ExprMgr::applyVarExprs() apply var exprs
//-------------------------------------------------------------
void ExprMgr::applyVarExprs()
{
	TNvar *exp=0;
	Node<TNode*> *ptr=exprs.ptr;
	exprs.ss();
	while((exp=(TNvar*)exprs++)){
	    exp->applyExpr();
	}
    exprs.ptr=ptr;
}

//-------------------------------------------------------------
// ExprMgr::clearVarExprs() clear var exprs
//-------------------------------------------------------------
void ExprMgr::clearVarExprs()
{
	TNvar *exp=0;
	Node<TNode*> *ptr=exprs.ptr;
	exprs.ss();
	while((exp=(TNvar*)exprs++)){
	    exp->clearExpr();
	}
    exprs.ptr=ptr;
}

//-------------------------------------------------------------
// ExprMgr::setPreviewMode() apply var exprs
//-------------------------------------------------------------
void ExprMgr::setPreviewMode()
{
	set_preview(1);
	eval();
}

//-------------------------------------------------------------
// ExprMgr::setStandardMode() clear var exprs
//-------------------------------------------------------------
void ExprMgr::setStandardMode()
{
	set_preview(0);
	eval();
}

//-------------------------------------------------------------
// ExprMgr::getVarExpr() get expr
//-------------------------------------------------------------
int ExprMgr::getVarExpr(char *n,char *s)
{
    TNvar *exp=get_expr(n);
    if(exp){
        exp->valueString(s);
        return 1;
    }
    else{
    	s[0]=0;
    	return 0;
	}
}

//-------------------------------------------------------------
// ExprMgr::setVarExpr() set expr
//-------------------------------------------------------------
int ExprMgr::setVarExpr(char *n,char *s)
{
    TNvar *exp=get_expr(n);
    if(exp){
        exp->setExpr(s);
        return 1;
    }
    return 0;
}

//-------------------------------------------------------------
// ExprMgr::applyVarExpr() set expr
//-------------------------------------------------------------
void ExprMgr::applyVarExpr(char *n)
{
    TNvar *exp=get_expr(n);
    if(exp)
       exp->applyExpr();
}

//-------------------------------------------------------------
// ExprMgr::clearVarExpr() set expr
//-------------------------------------------------------------
void ExprMgr::clearVarExpr(char *n)
{
    TNvar *exp=get_expr(n);
    if(exp)
        exp->clearExpr();
}

//-------------------------------------------------------------
// ExprMgr::get_expr() get expr
//-------------------------------------------------------------
TNvar *ExprMgr::get_expr(char *s)
{
	TNvar *exp=0;
	Node<TNode*> *ptr=exprs.ptr;
	exprs.ss();
	while((exp=(TNvar*)exprs++)){
	    if(strcmp(exp->name(),s)==0)
	    	break;
	}
    exprs.ptr=ptr;
    return exp;
}

//-------------------------------------------------------------
// Scope::get_local() get variable value
//-------------------------------------------------------------
TNvar *ExprMgr::getExprVal(char *s, TerrainData &f)
{
	TNvar *expr=get_expr(s);
	if(expr){
		expr->eval();
		return expr;
	}
	return 0;
}

//-------------------------------------------------------------
// Scope::get_local() get variable value
//-------------------------------------------------------------
TNvar *ExprMgr::setExprVal(char *s, TerrainData &f)
{
	TNvar *expr=get_expr(s);
	if(expr){
		expr->eval();
		return expr;
	}
	return 0;
}
//-------------------------------------------------------------
// ExprMgr::revaluate() invalidate vars
//-------------------------------------------------------------
void ExprMgr::revaluate()
{
	TerrainSym *ts;
	vars.ss();
	while((ts=(TerrainSym*)vars++)){
	    if(!ts->constant())
	    	ts->set_reval(1);
	}
}

//-------------------------------------------------------------
// ExprMgr::init() normalize all scaling parameters
//-------------------------------------------------------------
void ExprMgr::init()
{
	TNvar *var;

	CurrentScope=this;
	set_init_mode(1);

	//vars.free();

	exprs.ss();
	while((var=(TNvar*)exprs++)){
		if(var->do_show()){
			var->init();
			var->eval();
			var->set_constant(S0.constant());
	    }
	    else
	        var->set_constant(1);
//#ifdef DEBUG_INIT
// 		if(var->constant())
//	    	printf("const expr %s %g\n",var->name,S0.s);
//	    else
//	    	printf("init expr %s %g\n",var->name,S0.s);
//#endif
    }
	inodes.ss();
	TNinode *image;
	while((image=(TNinode*)inodes++)){
	   image->init();
#ifdef DEBUG_INIT
	   printf("image expr %s\n",image->name);
#endif
    }
	set_first(1);
	TheNoise.offset=0.5;
	TheNoise.scale=0.5;

	MapPt=Td.rectangular(0.0,0.0);
	TheNoise.set(MapPt);

	exprs.ss();
	while((var=(TNvar*)exprs++)){
		if(var->constant())
			 continue;
		var->init();
	}
	set_first(0);
	set_init_mode(0);
}

//-------------------------------------------------------------
// ExprMgr::free() remove exprs
//-------------------------------------------------------------
void ExprMgr::free()
{
	exprs.free();
	inodes.free();
	Scope::free();
}

//-------------------------------------------------------------
// TNode::save() evaluate the node
//-------------------------------------------------------------
void ExprMgr::save_vars(FILE *f)
{
	CurrentScope=this;
	Scope::save(f);
}

//-------------------------------------------------------------
// TNode::save() evaluate the node
//-------------------------------------------------------------
void ExprMgr::save(FILE *f)
{
	CurrentScope=this;
	save_selected(f);
	TNode *t;
	inodes.ss();
	while((t=inodes++))
		t->save(f);
}

//-------------------------------------------------------------
// TNode::save_selected() evaluate the node
//-------------------------------------------------------------
void ExprMgr::save_selected(FILE *f)
{
	CurrentScope=this;
	TerrainSym *ts;
	TNvar *var;

	Node<TNode*> *ptr=exprs.ptr;

	exprs.ss();
	while((var=(TNvar*)exprs++)){
	    ts=vars.inlist(var->name());
		if(ts)
	    var->set_show(ts->do_show() && ts->do_eval());
	}
	vars.ss();
	while((ts=vars++)){
	    if(ts->do_show() && !ts->do_eval())
			ts->save(f);
	}
	exprs.ss();
	while((var=(TNvar*)exprs++)){
	    if(var->do_show())
		    var->save(f);
	}

#ifdef DEBUG_TD_VARS
	vars.ss();
	while(ts=vars++){
	    if(!ts->do_show() || ts->do_eval())
	        printf("\tTD hide var %s\n",ts->name());
	    else
	        printf("\tTD show var %s\n",ts->name());
	}
#endif
#ifdef DEBUG_EXPR_VARS
	exprs.ss();
	while(var=(TNvar*)exprs++){
	    if(var->do_show())
	        printf("\texpr show expr %s\n",var->name());
		else
	        printf("\texpr hide expr %s\n",var->name());
	}

    exprs.ptr=ptr;
#endif

}

//-------------------------------------------------------------
// ExprMgr::eval() 		evaluate nodes
//-------------------------------------------------------------
void ExprMgr::eval()
{
	CurrentScope=this;
	TNvar *var;
	exprs.ss();
	Td.reset();

	while((var=(TNvar*)exprs++)){
		var->eval();
	}
}

//-------------------------------------------------------------
// ExprMgr::add_expr() set variable expr
//-------------------------------------------------------------
TNode *ExprMgr::add_expr(char *s,TNode *r)
{
	TNvar *var=new TNvar(s,r);
	var->setParent(this);
 	exprs.add(var);
 	return var;
}

//-------------------------------------------------------------
// ExprMgr::add_image() set variable expr
//-------------------------------------------------------------
TNode *ExprMgr::add_image(TNode *r)
{
	//cout<<"ExprMgr adding:"<<((TNinode*)r)->name<<endl;
 	inodes.add(r);
 	return r;
}

//-------------------------------------------------------------
// ExprMgr::get_image() get image expr
//-------------------------------------------------------------
TNinode *ExprMgr::get_image(char *s)
{
	TNinode *image=0;
	Node<TNode*> *ptr=inodes.ptr;
	inodes.ss();
	while((image=(TNinode*)inodes++)){
		if(strcmp(s,image->name)==0)
	    	break;
	}
    inodes.ptr=ptr;
    return image;
}

//-------------------------------------------------------------
// ExprMgr::get_image() get image expr
//-------------------------------------------------------------
TNinode *ExprMgr::replace_image(TNinode *img, TNinode *r)
{
	TNinode *image=0;
	TNinode *rimage=0;
	Node<TNode*> *ptr=inodes.ptr;
	inodes.ss();
	char *rn=r->name;
	
	while((image=(TNinode*)inodes++)){
		if(strcmp(r->name,image->name)==0){
			rimage=image;
	    	break;
		}
	}
	if(rimage){
		inodes.replace(rimage,r);
	}
	else
		
    inodes.ptr=ptr;
    return rimage;
}

//-------------------------------------------------------------
// ExprMgr::setVar() add or set a variable
//-------------------------------------------------------------
TNvar *ExprMgr::setVar(char *s, char *arg){
  	char *vn;
  	NodeIF *r=0;
  	if(strlen(arg))
  		r=TheScene->parse_node(arg);
    TNvar *var=0;
	TerrainSym *ts=vars.inlist(s);
  	if (!ts){
		MALLOC(strlen(s)+1 ,char,vn);
		strcpy(vn,s);
		var=(TNvar*)add_expr(vn,(TNode*)r);
    }
    else{
	    var=getVar(s);
	    if(var->right)
		    delete var->right;
	    var->right=(TNode*)r;
	    if(r)
	    	var->right->setParent(var);
	}
	return var;
}

//-------------------------------------------------------------
// ExprMgr::removeVar() remove a variable
//-------------------------------------------------------------
TNvar *ExprMgr::removeVar(char *s){
	TerrainSym *ts=vars.inlist(s);
    TNvar *var=0;
	if(!ts)
	    return 0;
	var=getVar(s);
	if(!var)
	    return 0;
	cout<<"free:"<<s<<endl;
	exprs.remove(var);
	vars.remove(ts);
	return var;
}

//-------------------------------------------------------------
// ExprMgr::removeVar() remove a variable
//-------------------------------------------------------------
TNvar *ExprMgr::removeExprVar(char *s){
    TNvar *var=0;
	var=getVar(s);
	if(!var)
	    return 0;
	//cout<<"free:"<<s<<endl;
	exprs.remove(var);
	return var;
}
//-------------------------------------------------------------
// ExprMgr::getVar() find a variable
//-------------------------------------------------------------
TNvar *ExprMgr::getVar(char *s){

	Node<TNode*> *ptr=exprs.ptr;

	TNvar *var;
	exprs.ss();
	while((var=(TNvar*)exprs++)){
		if(strcmp(var->name(),s)==0){
			exprs.ptr=ptr;
	        return var;
		}
	}
	exprs.ptr=ptr;
	return 0;
}

//-------------------------------------------------------------
// ExprMgr::NodeIF methods
//-------------------------------------------------------------
int ExprMgr::getChildren(LinkedList<NodeIF*>&l){return 0;}
NodeIF *ExprMgr::removeChild(NodeIF *c){return c;}
NodeIF *ExprMgr::addChild(NodeIF *c){return c;}
NodeIF *ExprMgr::addAfter(NodeIF *a,NodeIF *c){return c;}
NodeIF *ExprMgr::replaceChild(NodeIF *c,NodeIF *n){ return 0;}

//-------------------------------------------------------------
// ExprMgr::invalidate() invalidate
//-------------------------------------------------------------
void ExprMgr::invalidate(){
    NodeIF *p=getParent();
    while(p && !(p->typeClass()&ID_OBJECT))
        p=p->getParent();
    if(p)
        p->invalidate();
}


//************************************************************
// TerrainMgr class
//************************************************************
TerrainMgr::TerrainMgr() : ExprMgr()
{
	root=0;
	var_index=0;
	name_str[0]=0;
}

TerrainMgr::~TerrainMgr()
{
	if(root)
		delete root;
	root=0;
	//Td.init();
	Td.properties.free();
}

TNroot::~TNroot()
{
	//name=0;
}

//-------------------------------------------------------------
// TerrainMgr::set_root() set root node
//-------------------------------------------------------------
TNode *TerrainMgr::set_root(TNode *r)   {
	DFREE(root);
	if(r){
		root=new TNroot(r);
		root->setParent(this);
	}
	return root;

}

//-------------------------------------------------------------
// TerrainMgr::get_root() return root node
//-------------------------------------------------------------
	TNode *TerrainMgr::get_root()	{
	return root;
}

//-------------------------------------------------------------
// TerrainMgr::NodeIF methods
//-------------------------------------------------------------
bool TerrainMgr::hasChildren(){
	return root?true:false;
}


//-------------------------------------------------------------
// TerrainMgr::NodeIF methods
//-------------------------------------------------------------
static int find_type=0;
static bool find_test=false;
static bool find_enabled=false;
static TNode *find_child=0;

static void findType(NodeIF *obj)
{
	int type=obj->typeValue();
	if(type==ID_ROCKS||type==ID_MAP)
		obj->setFlag(NODE_STOP);
	else if(obj->typeValue()==find_type){
		if(obj->isEnabled() || !find_enabled)
			find_test=true;
		else
			find_test=false;
		obj->setFlag(NODE_STOP);
		find_child=obj;
	}
}
bool TerrainMgr::hasChild(int type){
	find_type=type;
	find_test=false;
	find_enabled=false;
	find_child=0;
	visitNode(findType);
	return find_test;
}
bool TerrainMgr::hasChild(int type,bool enabled){
	find_type=type;
	find_test=false;
	find_enabled=true;
	visitNode(findType);
	return find_test;
}
TNode *TerrainMgr::getChild(int type){
	if(hasChild(type))
		return find_child;
	return 0;
}

//-------------------------------------------------------------
// TerrainMgr::NodeIF methods
//-------------------------------------------------------------
int TerrainMgr::getChildren(LinkedList<NodeIF*>&l){
	if(root){
		l.add(root);
		return 1;
	}
	return 0;
}

//-------------------------------------------------------------
NodeIF *TerrainMgr::removeChild(NodeIF *c){
   if(c==root){
		TNroot  *tmp=root;
		root=0;
		return tmp;
   }
   else
   		return root->removeChild(c);
}

//-------------------------------------------------------------
NodeIF *TerrainMgr::addChild(NodeIF *c){
	if(!root)
		set_root((TNode*)c);
	else
   		root->addChild(c);
	return c;
}

//-------------------------------------------------------------
NodeIF *TerrainMgr::addAfter(NodeIF *a,NodeIF *c)
{
	if(!a)
		root->addChild(c);
	else if(!a->isLeaf())
		a->addAfter(a,c);
	else if(a)
		a->addChild(c);
	else
		root->addChild(c);
	return c;
}

//-------------------------------------------------------------
NodeIF *TerrainMgr::replaceNode(NodeIF *n)
{
	NodeIF *delobj=0;
	if(root){
		delobj=root->replaceChild(root->right,n);
		//validateVariables();
		validateTextures();
		delete delobj;
	}
	else
		set_root((TNode *)n);

	return this;
}

//-------------------------------------------------------------
NodeIF *TerrainMgr::replaceChild(NodeIF *c,NodeIF *n)
{
	NodeIF *delobj=0;
	if(c==this){
		if(root){
			delobj=root->replaceChild(root->right,n);
			//validateVariables();
			validateTextures();
		}
		else
			set_root((TNode *)n);
	}
	else{
		delobj=c->removeNode();
		addChild(n);
	}
	return delobj;
}

//-------------------------------------------------------------
int TerrainMgr::valueValid(){
    if(root && root->right && root->right->valueValid())
        return 1;
    return 0;
}


//-------------------------------------------------------------
void TerrainMgr::valueString(char *s){
	*s=0;
    if(root)
        root->valueString(s);
}

//-------------------------------------------------------------
void TerrainMgr::setValue(NodeIF *n){
    if(valueValid())
        root->setValue(n);
}

//-------------------------------------------------------------
NodeIF*  TerrainMgr::getValue(){
    if(root)
        return root->getValue();
    return 0;
}
//-------------------------------------------------------------
// TerrainMgr::validateVariables()
// 1. remove unused variables
//-------------------------------------------------------------
static NameList<TNvar*> vnodes;
static void getvar(TNode* n)
{
	if(n->typeValue()==ID_VAR){
		TNvar *v=(TNvar*)n;
		vnodes.add(v);
		vnodes.sort();
	}
}
void TerrainMgr::validateVariables()
{
	if(!root)
        return;
	CurrentScope=this;
	TNvar    *vnode;

	// check terrain stack

	vnodes.reset();
	root->visit(getvar);

	if(vnodes.size==0){
		exprs.free();  // no variable in terrain stack
	}
	else{

		//check for dependent variables
		exprs.ss();
		while((vnode=(TNvar*)exprs++))
			vnode->right->visit(getvar);

		exprs.ss();
		while((vnode=(TNvar*)exprs.at())){
			if(!vnodes.inlist(vnode->name())){  // unused
				exprs.pop();
				delete vnode;
			}
			else
				exprs++;
		}
	}
	vnodes.reset();
}

//-------------------------------------------------------------
// TerrainMgr::validateTextures()
// 1. insure all textures have a bands or image expression
// 2. remove image expressions not used in a texture
//-------------------------------------------------------------
static LinkedList<TNode*> textures;
static void get_textures(TNode* n) {
	if(n->typeValue()==ID_TEXTURE)
	    textures.add(n);
}
void TerrainMgr::validateTextures()
{
    if(!parent || !root)
        return;
	CurrentScope=this;

	textures.reset();
	root->visit(get_textures);
	textures.ss();
	TNtexture* tex;
	TNinode  *inode;
	TNode    *tnode;
	ExprMgr *imgr=(ExprMgr*)parent;

	imgr->inodes.ss();
	while((tnode=imgr->inodes++))
		tnode->NodeIF::invalidate();  // mark unused
	imgr->inodes.ss();

	// add new image variables if not present

	while((tex=(TNtexture*)textures++)){
		inode=imgr->get_image(tex->name);
		if(inode){
			inode->validate();
			if(inode->gradName()){
				TNinode *gnode=imgr->get_image(inode->gradName());
				if(gnode){
					gnode->validate();
				}
				else{
#ifdef DEBUG_VALIDATE
					cout << "TerrainMgr::validateTextures - adding missing grad image:"<<inode->gradName()<<endl;
#endif
					addTextureImage(inode->gradName());
				}
			}			
		}
		else{
#ifdef DEBUG_VALIDATE
    cout << "TerrainMgr::validateTextures - adding missing tex image: "<<tex->name<<endl;
#endif
			addTextureImage(tex->name);
		}
	}

	// remove unused texture variables

	imgr->inodes.ss();
	while((inode=imgr->inodes.at())){
		if(inode->NodeIF::invalid()){  // unused
#ifdef DEBUG_VALIDATE
    cout << "TerrainMgr::validateTextures - removing unused image: "<<inode->name<<endl;
#endif
			imgr->inodes.pop();
			delete tnode;
		}
		else
			imgr->inodes++;
	}
}

bool TerrainMgr::addTextureImage(char *name){
	ExprMgr *imgr=(ExprMgr*)parent;

    char *s=images.readSpxFile(name);
    if(!s)
		return false;
    TNode *tnode=(TNode*)TheScene->parse_node(s);
#ifdef DEBUG_IMAGES
    printf("%-20s ADDING TERRAIN IMAGE %s\n","TerrainMgr",name);
#endif
    //tnode->init();
    imgr->add_image(tnode);
    FREE(s);
    return true;

}
//-------------------------------------------------------------
// TerrainMgr::save() archive
//-------------------------------------------------------------
void TerrainMgr::save(FILE *f)
{
	CurrentScope=this;
	if(root){
		fprintf(f,"%s%s{\n",tabs,typeName());
		inc_tabs();
		if(exprs.size && TheScene->keep_variables())
			ExprMgr::save(f);
		root->save(f);
		dec_tabs();
		fprintf(f,"%s}\n",tabs);
	}
}

//-------------------------------------------------------------
// TerrainMgr::saveNode() save node file
//-------------------------------------------------------------
void TerrainMgr::saveNode(FILE *f)
{
	if(root && root->right)
		root->right->save(f); // save entire tree
}

//-------------------------------------------------------------
// TerrainMgr::eval() 		evaluate nodes
//-------------------------------------------------------------
void TerrainMgr::eval()
{
	TNvar *var;
	CurrentScope=this;
	set_zcpass();

	exprs.ss();

	Td.reset();
	while((var=(TNvar*)exprs++)){
		var->eval();
	}

	Td.reset();
	if(!root || !root->right)
	    return;
	root->right->eval();

	Height=S0.p.z;
	PX=S0.p.x;
	PY=S0.p.y;

}

//-------------------------------------------------------------
// TerrainMgr::set_surface() set surface of specified node
//-------------------------------------------------------------
void TerrainMgr::set_surface(TerrainData &data)
{
	double theta=data.p.x;
	double phi=data.p.y;
	Theta=theta;   // sign change at theta=0
	Phi=phi;
	Height=0;
	Drop=Margin=0;
	data.density=0.0;
	data.erosion=0.0;
	data.hardness=0.0;
	data.sediment=0.0;
	data.height=0.0;
	data.margin=0.0;
	data.depth=0.0;
	data.ocean=0.0;

	data.albedo=1.0;
	data.shine=1.0;

	CurrentScope=this;

	TheNoise.offset=0.5;
	TheNoise.scale=0.5;

	MapPt=Td.rectangular(theta,phi);
	TheNoise.set(MapPt);

	INIT;

	S0.c=data.c;
	S0.p=Point(0,0,0);
	set_eval_mode(1);

	eval();
	set_eval_mode(0);
	set_first(0);
	data=S0;
}

//-------------------------------------------------------------
// TerrainMgr::init() normalize all scaling parameters
//-------------------------------------------------------------
void TerrainMgr::init()
{
	TNvar *var;
	CurrentScope=this;

	set_init_mode(1);
	set_first(1);

	TheNoise.offset=0.5;
	TheNoise.scale=0.5;
	Td.fids=0;

	MapPt=Td.rectangular(0.0,0.0);
	TheNoise.set(MapPt);

	exprs.ss();
	while((var=(TNvar*)exprs++)){
		Td.init();
		var->init();
	}
	Td.init();

	if(root)
		root->init();

	set_init_mode(0);
	set_eval_mode(1);
}
//-------------------------------------------------------------
// TerrainMgr::init_render() 		evaluate nodes
//-------------------------------------------------------------
void TerrainMgr::init_render()
{
	CurrentScope=this;
	
	Td.plants.reset();
	Td.sprites.reset();

	Td.init();
	Td.properties.free();
	Td.add_id();
	Td.add_id();
	if(root && root->right){
		set_rpass();
		root->right->eval();
		set_zcpass();
	}
	Td.eval_properties();
}

//-------------------------------------------------------------
// TerrainMgr::applyVarExprs() apply var exprs
//-------------------------------------------------------------
void TerrainMgr::applyVarExprs()
{
    if(parent)
        parent->applyVarExprs();
	ExprMgr::applyVarExprs();
	if(root)
		root->init();
}

//-------------------------------------------------------------
// TerrainMgr::clearVarExprs() clear var exprs
//-------------------------------------------------------------
void TerrainMgr::clearVarExprs()
{
    if(parent)
        parent->clearVarExprs();
	ExprMgr::clearVarExprs();
	if(root)
		root->init();
}

//-------------------------------------------------------------
// TerrainMgr::clearExpr() clear root expr
//-------------------------------------------------------------
void TerrainMgr::clearExpr()
{
	if(root)
		root->clearExpr();
}

//-------------------------------------------------------------
// TerrainMgr::applyExpr() apply root expr
//-------------------------------------------------------------
void TerrainMgr::applyExpr()
{
	if(root)
		root->applyExpr();
}

//-------------------------------------------------------------
// TerrainMgr::setAdaptMode()
//-------------------------------------------------------------
void TerrainMgr::setAdaptMode()
{
	init_render();
	set_eval_mode(0);
}

//-------------------------------------------------------------
// TerrainMgr::setPreviewMode() apply var exprs
//-------------------------------------------------------------
void TerrainMgr::setPreviewMode()
{
	set_preview(1);
	if(parent)
        parent->setPreviewMode();
	init();
	init_render();
	set_eval_mode(0);
}

//-------------------------------------------------------------
// TerrainMgr::setStandardMode() clear var exprs
//-------------------------------------------------------------
void TerrainMgr::setStandardMode()
{
	set_preview(0);
    if(parent)
        parent->setStandardMode();
	init();
	init_render();
	set_eval_mode(0);
}
