#include "TerrainMgr.h"
#include "Layers.h"
#include "MapNode.h"
#include "AdaptOptions.h"
#include "FileUtil.h"
#include <iostream>

int mindex=0;

extern void inc_tabs();
extern void dec_tabs();
extern double zslope();


extern char   tabs[];
extern double Hscale,Rscale;
extern double ptable[];
double  Mheight,Drop,Margin;

extern int hits,visits,misses;

static TerrainData Td;
extern int addtabs;

// layer symbols


static LongSym ltypes[]={
	{"MESH",	MESH},
	{"MORPH",	MORPH},
};

NameList<LongSym*> LTypes(ltypes,sizeof(ltypes)/sizeof(LongSym));

static LongSym lopts[]={
	{"ZONLY",   ZONLY},
	{"CBLEND",	CBLEND},
	{"FSQR",	FSQR}
};
NameList<LongSym*> LOpts(lopts,sizeof(lopts)/sizeof(LongSym));

//************************************************************
// TNmap class
//************************************************************
TNmap::TNmap(TNode *l, TNode *r) :  TNfunc(l,r)
{
	margin=0.1;
	drop=DFLT_DROP;
	morph=0;
	mscale=1;
	mbase=-0.5;
}

//-------------------------------------------------------------
// TNmap::valueString() write value string
//-------------------------------------------------------------
void TNmap::valueString(char *s)
{
	int in_map=S0.get_flag(CLRTEXS);
	S0.set_flag(CLRTEXS);
    TNfunc::valueString(s);
    if(!in_map)
	    S0.clr_flag(CLRTEXS);
}

//-------------------------------------------------------------
// index function
//-------------------------------------------------------------
TNode *TNmap::operator[](int i)
{
	int n=0;
	TNlayer *layer=(TNlayer*)right;
	while(layer){
	    if(n==i)
	        break;
		layer=(TNlayer*)layer->right;
		n++;
	}
	if(layer && n==i)
	    return layer;
	return 0;
}

//-------------------------------------------------------------
// TNmap::addChild
//-------------------------------------------------------------
NodeIF *TNmap::addChild(NodeIF *x)
{
	TNode *n=(TNode *)x;

	if(x->typeValue()!=ID_LAYER){
		int t=MESH;
		if(right)
			t=((TNlayer*)right)->type;
		n=new TNlayer(0,t, 0, right,n);
    	n->setParent(this);
    	right=n;
	}
	else{
		n->setParent(this);
		if(right)
			n->addChild(right);
		right=n;
    }
	return x;
}

//-------------------------------------------------------------
// TNmap::removeNode replace child node
//-------------------------------------------------------------
NodeIF *TNmap::removeNode()
{
	NodeIF *p=getParent();
	if(p)
		p->removeChild(this);
	return this;
}

//-------------------------------------------------------------
// TNmap::replaceChild replace child node
//-------------------------------------------------------------
NodeIF *TNmap::replaceChild(NodeIF *c,NodeIF *n)
{
	if(c->typeValue()!=ID_LAYER)
		return c;
	if(c==right&&n->typeValue()==ID_LAYER){
		right=(TNode*)n;
		right->setParent(this);
		return c;
	}
	return ((TNlayer*)c)->replaceChild(c,n);
}

//-------------------------------------------------------------
// TNmap::setdrop() set drop for all layers
//-------------------------------------------------------------
void TNmap::setdrop(double d)
{
	TNlayer *node=(TNlayer*)right;
	char buff[32];
	sprintf(buff,"%g\n",d);
	drop=d;
	while(node && node->typeValue()==ID_LAYER){
		TNlayer *layer=node;
		TNlayer *lexpr=(TNlayer*)layer->getExprNode();
		if(lexpr)
			layer=lexpr;

		TNarg *arg=(TNarg *)layer->left;
		arg=arg->index(2);
		if(arg){
			TNode *newarg=(TNode*)TheScene->parse_node(buff);
			TNode *oldarg=arg->left;
			arg->replaceChild(oldarg,newarg);
			delete oldarg;
		}

	    node->drop=drop;
		node=(TNlayer*)node->right;
	}
}

//-------------------------------------------------------------
// TNmap::setmorph() set morph for all layers
//-------------------------------------------------------------
void TNmap::setmorph(double d)
{
	TNlayer *node=(TNlayer*)right;
	char buff[32];
	sprintf(buff,"%g\n",d);
	morph=d;
	while(node && node->typeValue()==ID_LAYER){
		TNlayer *layer=node;
		TNlayer *lexpr=(TNlayer*)layer->getExprNode();
		if(lexpr)
			layer=lexpr;
		if(morph){
			BIT_ON(layer->type,MORPH);
			BIT_OFF(layer->type,MESH);
		}
		else{
			BIT_OFF(layer->type,MORPH);
			BIT_ON(layer->type,MESH);
		}
		TNarg *arg=(TNarg *)layer->left;
		arg=arg->index(0);
		if(arg){
			TNode *newarg=(TNode*)TheScene->parse_node(buff);
			TNode *oldarg=arg->left;
			arg->replaceChild(oldarg,newarg);
			delete oldarg;
		}
	    node->morph=morph;
		node=(TNlayer*)node->right;
	}
}
//-------------------------------------------------------------
// TNmap::init() evaluate the node
//-------------------------------------------------------------
void TNmap::init()
{
	TNlayer *layer=(TNlayer*)right;
	layers=0;
	while(layer && layer->typeValue()==ID_LAYER){
	    layer->map=this;
		layer=(TNlayer*)layer->right;
	}
	TNfunc::init();
}

//-------------------------------------------------------------
// TNmap::eval() evaluate the node
//-------------------------------------------------------------
void TNmap::eval()
{
	TNlayer *layer=(TNlayer*)right;
	int i;

	if(layer && CurrentScope->rpass()){
		int in_map=S0.get_flag(CLRTEXS);
		S0.set_flag(CLRTEXS);
		while(layer && layer->typeValue()==ID_LAYER){
			layer->id=Td.tids-1;

			layer->base->eval();
			layer=(TNlayer*)layer->right;
			if(!layer || layer->typeValue()!=ID_LAYER)
				break;
			if(layer->isEnabled())
				Td.add_id();
		}
		if(!in_map)
			S0.clr_flag(CLRTEXS);
		return;
	}

    if(!CurrentScope->cpass()){
		double args[5];
		int n=0;
  		TNarg *arg=(TNarg*)left;

		if(arg)
			n=getargs(arg,args,5);
		mht=n>0?args[0]:0;
		mbase=n>1?args[1]:0;
		mscale=n>2?args[2]:1;
		drop=n>3?args[3]:DFLT_DROP;
		morph=n>4?args[4]:0;
	}
	if(!right)
		return;
	mindex++;
	if(mindex==1)
		Mheight=mht;
	INIT;

	Td.begin();
	Td.lower.p.z=TZBAD;


	layer=(TNlayer*)right;
	TNlayer *last_layer=layer;
	TNlayer *edge_layer=layer;
	TNlayer *top_layer=layer;

	double f=0,d;
	double edge=mbase;
	double lowdz=-10;
	Color  lowc;

	int id=0;
	//	 && edge_layer->type & CBLEND

	#define CHKLOW \
	        edge_layer->morph \
	     && Td.zlevel[1].p.z>-10 \
	     && !CurrentScope->zpass()

	int in_map=S0.get_flag(CLRTEXS);
	S0.set_flag(CLRTEXS);
	S0.datacnt=0;

	while(layer && layer->typeValue()==ID_LAYER){
		if(!layer->isEnabled()){
			layer=(TNlayer*)layer->right;
			continue;
		}
		S0.clr_flag(LOWER);
		layer->map=this;
	    S0.height=0;
		layer->edge=edge;
		layer->drop=drop;
		layer->morph=morph;
		layer->eval();

		edge_layer=layer;

	    if(layer->right){
			double w=layer->width;
			double width=w>0?w:0;
		    double h=-w/margin+1;
		    double df=layer->ramp;

			if(h<0)
		    	h=0;
			edge+=width;

			if(id==0 && mht<edge)
		    	f=0;
			else if(mht>=layer->edge && mht<edge)
		    	f=0;
			else if(mht>edge)
		    	f=(mht-edge)/margin;
			else if(mht<layer->edge){
		    	edge_layer=last_layer;
		    	f=(layer->edge-mht)/margin;
		    }
 			Margin=fabs((layer->edge-mht)/margin);
 			if(edge_layer->type & FSQR)
 			    f*=f;
 			if(layer->type & FSQR)
 			    h*=h;
 			d=f*edge_layer->drop;
 			if(h)
 			    d+=2*h*layer->drop;
 			if(df)
 				d+=df*clamp(f,0,1)*edge_layer->drop;
		}
		else{  // last layer
		    double df=layer->ramp;
			if(mht>=edge)
		    	f=0;
			else if(mht<edge)
		    	f=(edge-mht)/margin;
 			if(layer->type & FSQR)
 			    f*=f;
 			d=f*layer->drop;
 			Margin=fabs((edge-mht)/margin);
 			if(df)
 				d+=df*clamp(f,0,1)*edge_layer->drop;
		}
    	INIT;

    	Drop=d;
	    S0.clr_flag(TEXFLAG);
		layer->base->eval();
       	S0.p.z-=d;
       	if(S0.get_flag(LOWER))
       	    Td.lower.p.z-=d;
	    else
			S0.next_id();

		if(id==0){  // first layer
		    Td.zlevel[0].copy(S0);
      		if(S0.get_flag(LOWER))
       		    Td.zlevel[1].copy(Td.lower);
		}
		else {
			Td.insert_strata(S0);
			if(S0.get_flag(LOWER))
				Td.insert_strata(Td.lower);
		}
		last_layer=layer;
		layer=(TNlayer*)layer->right;

		if(S0.get_flag(LOWER))
 			S0.next_id();

		//if(layer && layer->typeValue()==ID_LAYER)
		//	S0.next_id();
		id++;
	}
	mindex--;
	if(mscale){
		for(int i=0;i<MAX_TDATA;i++){
			if(Td.zlevel[i].p.z>TZBAD)
				Td.zlevel[i].p.z+=mht*mscale;
		}
	}

	double dz=Td.zlevel[0].p.z-Td.zlevel[1].p.z;
	f=0;
    if(top_layer->morph){
        f=top_layer->morph*rampstep(0,margin,dz,0.5,0);
		if(f){
			Point p=Td.zlevel[0].p;
			Td.zlevel[0].p.z=(1-f)*Td.zlevel[0].p.z+f*Td.zlevel[1].p.z;
			Td.zlevel[0].p.x=(1-f)*Td.zlevel[0].p.x+f*Td.zlevel[1].p.x;
			Td.zlevel[0].p.y=(1-f)*Td.zlevel[0].p.y+f*Td.zlevel[1].p.y;
			Td.zlevel[1].p.z=(1-f)*Td.zlevel[1].p.z+f*p.z;
			Td.zlevel[1].p.x=(1-f)*Td.zlevel[1].p.x+f*p.x;
			Td.zlevel[1].p.y=(1-f)*Td.zlevel[1].p.y+f*p.y;
		}
        dz=Td.zlevel[0].p.z-Td.zlevel[1].p.z;
    }
    if(CurrentScope->zpass()){
        S0.copy(Td.zlevel[0]);
        return;
    }
    //if(f&&top_layer->type&CBLEND){
    if(f){
		Color c1=Td.zlevel[0].c;
		Color c2=Td.zlevel[1].c;
		if(lowdz>-10)
			c2=c2.blend(lowc,rampstep(0,margin,5*lowdz,0.5,0));
		Td.zlevel[1].c=c2.blend(c1,f);
		Td.zlevel[0].c=c1.blend(c2,f);
		Td.zlevel[0].set_cvalid();
		Td.zlevel[1].set_cvalid();
    }

	S0.copy(Td.zlevel[0]); // return top level

	if(!in_map)    // in case we were in another map on entry
		S0.clr_flag(CLRTEXS);

	if(Adapt.mindcnt()){  // minimize dual terrain nodes (edges only)
		for(i=0;i<rccnt;i++){
			MapData *md=mapdata[i];
			if(md>0){
				MapData *d=md->surface1();
				if(d && (d->type()!=Td.zlevel[0].type())){
					Td.end();
					return;
				}
			}
		}
		for(i=1;i<MAX_TDATA;i++){
			if(Td.zlevel[i].p.z<=TZBAD)
				break;
			double dx=fabs(Td.zlevel[i].p.x-S0.p.x);
			double dy=fabs(Td.zlevel[i].p.y-S0.p.y);
			double dmax=dx>dy?dx:dy;

			dz=fabs(S0.p.z-Td.zlevel[i].p.z);
			if(dz<=dmax){
			    Td.end();
			    return;
			}
		}
	}
	else
		Td.end();
}

//************************************************************
// TNlayer class
//************************************************************
TNlayer::TNlayer(char *n, int t, TNode *l, TNode *r, TNode *b) : TNbase(t,l,r,b)
{
	map=0;
	width=0.1;
	drop=DFLT_DROP;
	morph=0;
	edge=0;
	ramp=0;
	id=0;
	name_str[0]=0;
	if(n){
		strcpy(name_str,n);
		::free(n);
	}
}

//-------------------------------------------------------------
// TNlayer::valueString() write value string
//-------------------------------------------------------------
void TNlayer::valueString(char *s)
{
	if(S0.get_flag(CLRTEXS))
	    TNbase::valueString(s);
	else if(base)
	    base->valueString(s);
}

//-------------------------------------------------------------
// TNlayer::setExpr() set expr string
//-------------------------------------------------------------
void TNlayer::setExpr(char *c)
{
	TNlayer *n=(TNlayer*)TheScene->parse_node(c);
	if(n){
	    DFREE(expr);
	    expr=n;
	    expr->setParent(this);
	}
}

//-------------------------------------------------------------
// TNlayer::applyExpr() apply expr value
//-------------------------------------------------------------
void TNlayer::applyExpr()
{
    if(expr){
		TNlayer *layer=(TNlayer*)expr;
        DFREE(left);
        left=layer->left;
        left->setParent(this);
		type=layer->type;
        expr=0;
    }
}
//-------------------------------------------------------------
// TNlayer::eval() evaluate the node
//-------------------------------------------------------------
void TNlayer::eval()
{
	if(!CurrentScope->cpass()){
		TNarg *arg=(TNarg*)left;
		if(arg){	// morph expr
			arg->eval();
			morph=clamp(S0.s,0,1);
			arg=arg->next();
			if(arg){  // width expr
				arg->eval();
				width=S0.s;
				arg=arg->next();
				if(arg){ // drop expr
					arg->eval();
					drop=S0.s;
					if(drop<0)
						drop=0;
					arg=arg->next();
					if(arg){ // ramp expr
						arg->eval();
						ramp=S0.s;
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------
// TNlayer::hasChild return true if child exists
//-------------------------------------------------------------
static int find_type=0;
static bool find_test=false;
static void findType(NodeIF *obj)
{
	int type=obj->typeValue();
	if(type==ID_ROCKS||type==ID_MAP)
		obj->setFlag(NODE_STOP);
	else if(obj->typeValue()==find_type){
		find_test=true;
		obj->setFlag(NODE_STOP);
	}
}
bool TNlayer::hasChild(int type){
	find_type=type;
	find_test=false;
	if(base)
		base->visitNode(findType);
	return find_test;
}

//-------------------------------------------------------------
// TNlayer::replaceChild replace content
//-------------------------------------------------------------
NodeIF *TNlayer::replaceChild(NodeIF *c,NodeIF *n)
{
	if(c!=this)
		return TNbase::replaceChild(c,n);
	if(!base){
		base=(TNode*)n;
		base->setParent(this);
		return 0;
	}
	NodeIF *delobj=base;
	base=(TNode*)n;
	base->setParent(this);
	return delobj;
}

//-------------------------------------------------------------
// TNlayer::addChild
//-------------------------------------------------------------
NodeIF *TNlayer::addChild(NodeIF *x)
{
	TNode *n=(TNode *)x;
    //cout << x->typeName()<<endl;
	if(x->typeValue()!=ID_LAYER){

		NodeIF *newbase=n;
		while(n->typeValue()&ID_FUNC){
			TNfunc *f=(TNfunc *)n;
			n=f->right;
			f->right=0;
		}
		if(newbase !=n)
		    delete newbase;

		int t=MESH;
		if(right)
			t=((TNlayer*)right)->type;
		n=new TNlayer(0,t, 0, right,n);
    	n->setParent(this);
    	right=n;
	}
	else{
		n->setParent(this);
		if(right)
			n->addChild(right);
		right=n;
    }
	return n;
}
//-------------------------------------------------------------
// TNlayer::replaceNode
//-------------------------------------------------------------
NodeIF *TNlayer::replaceNode(NodeIF *c){
	if(base)
		delete base;
	NodeIF *newbase=c;
	while(c->typeValue()&ID_FUNC){
		TNfunc *f=(TNfunc *)c;
		c=f->right;
		f->right=0;
	}
	delete newbase;
	base=(TNode*)c;
	base->setParent(this);

	return this;
}

//-------------------------------------------------------------
// TNlayer::raise raise position in  layer list
//-------------------------------------------------------------
void TNlayer::raise()
{
	TNlayer *sibling=(TNlayer*)getParent();
	if(!sibling || sibling->typeValue()!=ID_LAYER)
		return;
	TNode *n=sibling->base;
	sibling->base=base;
	base->setParent(sibling);
	base=n;
	base->setParent(this);
}

//-------------------------------------------------------------
// TNlayer::lower lower position in  layer list
//-------------------------------------------------------------
void TNlayer::lower()
{
	TNlayer *sibling=(TNlayer*)right;
	if(!sibling || sibling->typeValue()!=ID_LAYER)
		return;
	TNode *n=sibling->base;
	sibling->base=base;
	base->setParent(sibling);
	base=n;
	base->setParent(this);
}

//-------------------------------------------------------------
// TNlayer::saveNode save the node
//-------------------------------------------------------------
void TNlayer::saveNode(FILE *f)
{
	if(base){
		base->save(f);
	}
}
//-------------------------------------------------------------
// TNlayer::setName() set name
//-------------------------------------------------------------
void TNlayer::setName(char *s)
{
	strcpy(name_str,s);
}

//-------------------------------------------------------------
// TNlayer::propertyString() write property nodes
//-------------------------------------------------------------
void TNlayer::propertyString(char *s)
{
	if(strlen(name_str))
		sprintf(s+strlen(s),"%s(\"%s\",",symbol(),name_str);
	else
		sprintf(s+strlen(s),"%s(",symbol());
	char opts[64];
	opts[0]=0;
	if(optionString(opts)){
		sprintf(s+strlen(s),"%s",opts);
		if(left)
			strcat(s,",");
		else
			sprintf(s+strlen(s),",%g",morph);
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
// TNlayer::save() evaluate the node
//-------------------------------------------------------------
void TNlayer::save(FILE *f)
{
	char buff[256];
	buff[0]=0;
	propertyString(buff);
	fprintf(f,"%s",buff);
	inc_tabs();
	addtabs=1;
	fprintf(f,"\n%s[",tabs);
	if(base)
		base->save(f);
	dec_tabs();
	fprintf(f,"]\n%s",tabs);
	addtabs=0;
	if(right)
		right->save(f);
}

//-------------------------------------------------------------
// TNlayer::optionString() get option string
//-------------------------------------------------------------
int TNlayer::optionString(char *s)
{
	int lopt=type&LOPTS;
	int ltype=type&LTYPES;
	int i,n;

	n=sizeof(ltypes)/sizeof(LongSym);
	for(i=0;i<n;i++){
    	if(ltype==ltypes[i].value){
     		strcpy(s+strlen(s),ltypes[i].name());
     		break;
     	}
	}
	if(i==n)
     	strcpy(s+strlen(s),ltypes[0].name());
    n=sizeof(lopts)/sizeof(LongSym);
	for(i=0;i<n;i++){
    	if(lopt & lopts[i].value){
    		strcat(s,"|");
     		strcat(s,lopts[i].name());
   		}
	}
	return 1;
}
