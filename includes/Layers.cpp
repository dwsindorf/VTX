#include "TerrainClass.h"
#include "Layers.h"
#include "MapNode.h"
#include "MapClass.h"
#include "AdaptOptions.h"
#include "ImageClass.h"

#include "FileUtil.h"
#include <iostream>

int mindex=0;

extern void inc_tabs();
extern void dec_tabs();
extern double zslope();

extern Map     *TheMap;

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
	TNarg *arg=left;
	TNarg *node=arg->left;
	if(node->typeValue() == ID_STRING){		
		setName(((TNstring*)node)->value);
		left=arg->next();
		left->setParent(this);
		arg->right=0;
		delete arg;	
	}

	margin=0.1;
	mdrop=DFLT_DROP;
	mmorph=0;
	mscale=1;
	mbase=-0.5;
	msmooth=1;
}

NodeIF *TNmap::replaceNode(NodeIF *c){
	TNmap *newmap=(TNmap *)c;
	NodeIF *newleft=newmap->left;
	delete left;
	left=newleft;
	left->setParent(this);
	return this;
}
void TNmap::saveNode(FILE *f){
	TNfunc::saveNode(f);
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
	TNlayer *first_layer=(TNlayer*)right;
	if(!first_layer){
		return this;
	}
	p->addChild(first_layer->base);
	first_layer->base=0;
	delete first_layer;
	invalidate();
	right=0;
	Object3D *obj=getObject();
	obj->invalidate();

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

void TNmap::setDefault(){
	Object3D *obj=getObject();
	if(obj)
		obj->setDefault();
}

void TNmap::setRandom(bool b){
	Object3D *obj=getObject();
	if(obj)
		obj->setRandom(b);
}

bool TNmap::randomize(){
	Object3D *obj=getObject();
	if(obj)
		return obj->randomize();
	return false;
}
//-------------------------------------------------------------
// TNmap::setdrop() set drop for all layers
//-------------------------------------------------------------
void TNmap::setdrop(double d)
{
	TNlayer *node=(TNlayer*)right;
	char buff[32];
	sprintf(buff,"%g\n",d);
	mdrop=d;
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

	    node->drop=mdrop;
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
	mmorph=d;
	while(node && node->typeValue()==ID_LAYER){
		TNlayer *layer=node;
		TNlayer *lexpr=(TNlayer*)layer->getExprNode();
		if(lexpr)
			layer=lexpr;
		if(mmorph){
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
	    node->morph=mmorph;
		node=(TNlayer*)node->right;
	}
}
//-------------------------------------------------------------
// TNmap::init() evaluate the node
//-------------------------------------------------------------
void TNmap::init()
{
	TheMap->set_multilayer();

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
		//if(Td.get_flag(FVALUE))
		//	Td.set_flag(MULTILAYER);
		while(layer && layer->typeValue()==ID_LAYER){
			layer->id=Td.tids-1;
			layer->base->eval();
			Td.tp->ntexs=Td.tp->textures.size;

			if(Td.tp->textures.size && !Td.tp->has_color())
				Td.tp->color=Td.tp->textures[0]->aveColor;

			Td.tp->set_color(true);

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
		double args[6];
		int n=0;
  		TNarg *arg=(TNarg*)left;
 
		if(arg)
			n=getargs(arg,args,6);
		mht=n>0?args[0]:0;
		mbase=n>1?args[1]:0;
		mscale=n>2?args[2]:1;
		msmooth=n>3?args[3]:1;
		mdrop=n>4?args[4]:DFLT_DROP;
		mmorph=n>5?args[5]:0;
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

	#define CHKLOW \
	        edge_layer->morph \
	     && Td.zlevel[1].p.z>-10 \
	     && !CurrentScope->zpass()

	int in_map=S0.get_flag(CLRTEXS);
	S0.set_flag(CLRTEXS);
	S0.datacnt=0;
	//if(Td.get_flag(!ROCKLAYER))
	//	Td.set_flag(MULTILAYER);

	while(layer && layer->typeValue()==ID_LAYER){
		if(!layer->isEnabled()){
			layer=(TNlayer*)layer->right;
			continue;
		}
		S0.clr_flag(LOWER);
		layer->map=this;
	    S0.height=0;
		layer->edge=edge;
		layer->drop=mdrop;
		layer->morph=mmorph;
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
 			//Margin=fabs((edge-mht)/margin);
 			if(df)
 				d+=df*clamp(f,0,1)*edge_layer->drop;
		}

    	INIT;
    	Drop=d;
	    S0.clr_flag(TEXFLAG);
		S0.clr_flag(ROCKLAYER);

		layer->base->eval();
 		if(!Td.get_flag(ROCKLAYER))
			Td.set_flag(MULTILAYER);
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

		id++;
	}
	mindex--;
	if(mscale){
		for(int i=0;i<MAX_TDATA;i++){
			if(Td.zlevel[i].p.z>TZBAD)
				Td.zlevel[i].p.z+=mht*mscale;
		}
	}
	bool inmargin=false;
	bool inedge=false;

	double dz=Td.zlevel[0].p.z-Td.zlevel[1].p.z;
	//f=0;
	double s1=5e-4;
	double s2=1e-6;
	s1=msmooth*rampstep(TheScene->height,1e-7,1e-4,1e-3,0.01);
	s2=0.1*s1;
	//  cout<<"ht:"<<TheScene->height<<" s1:"<<s1<<" s2:"<<s2<<endl;

    // 1) Remove visual artifact when different textures meet at layer intersection
    //    get tiled effect (due to texture colors not being blended)
    //    work-around
    //    - reduce texture opacity to zero in small dz region at layer intersections
    //    - replace texture with texture average color
    //    - blend colors from adjacent layers
    double dm=rampstep(top_layer->morph,0,1,s1,s2);
    Td.margin=smoothstep(fabs(dz),0,dm,0,1); // sets texture opacity 
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
    }
    if(Td.margin<1){
    	int id=1+Td.zlevel[0].id(); // get properties for top layer
		if(Td.zlevel[0].properties[id]->ntexs){
	    	int tid=Td.zlevel[0].properties[id]->ntexs-1; // top texture in layer
			FColor  avec=Td.zlevel[0].properties[id]->textures[tid]->aveColor;
			Color c=Color(avec.red(),avec.green(),avec.blue());
			Td.zlevel[0].c=Color(avec.red(),avec.green(),avec.blue());
		}
		Td.zlevel[0].set_cvalid();
		for(int i=1;i<MAX_TDATA;i++){
			if(Td.zlevel[i].p.z==TZBAD)
				break;
	    	Td.zlevel[i].set_cvalid();
	    	Td.zlevel[i].c=Td.zlevel[0].c; // not sure why this works
		}
		if(Td.margin<0.75)
			S0.set_flag(INEDGE);
    }
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
	Td.zlevel[0].set_cvalid();

	S0.copy(Td.zlevel[0]); // return top level

	double ave=0;
	double maxht=-10;
	double minht=10;
	for(i=0;i<mdcnt;i++){
		MapData *d=mapdata[i]->surface1();
		if(d && (d->type()!=Td.zlevel[0].type())){
			S0.set_flag(INMARGIN);
			inmargin=true;
		}
		double z=d->Z();
		ave+=z/mdcnt;
	}
	extern int test3;
	if(!in_map)    // in case we were in another map on entry
		S0.clr_flag(CLRTEXS);

	if(Adapt.mindcnt()){
		// minimize dual terrain nodes (edges only)
		// - Only keep terrain data for highest layer (reduces memory and processing costs)
		// check for layer intersection
		// - special case tilted terrain ?
/*
		for(i=1;i<MAX_TDATA;i++){
			if(Td.zlevel[i].p.z<=TZBAD){
				break;
			}
			double dx=fabs(Td.zlevel[i].p.x-S0.p.x);
			double dy=fabs(Td.zlevel[i].p.y-S0.p.y);
			double dmax=dx>dy?dx:dy;

			dz=fabs(S0.p.z-Td.zlevel[i].p.z);
			if(dz<=dmax){
				//S0.set_flag(INMARGIN);
				inmargin=true;
				break;
			}
		}
*/
		// 2) Remove ridge artifact at center of layer intersection
		//    for some reason it looks like when dz~=0 "fractal" doesn't process the node
		//    which leaves a "wall" in the center
		//    throwing out the calculated ht and using the average of the nodes neighbors seams to fix the problem
		if(inmargin){  // at least one neighbor is from another layer
			if(Td.get_flag(FVALUE)){
				for(int i=0;i<MAX_TDATA;i++){
					if(Td.zlevel[i].p.z==TZBAD)
						break;
					Td.zlevel[i].p.z=ave;
				}
			}
			Td.end();
		}
	}
	else{
		// retain data for buried layers
		// when fractal is present the result is different than previous case
		// - more intermixed terrain segments
		// - artifact at some layer intersections (looks like deep slot)
		Td.end();
	}
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
					//drop=1e-4*S0.s;
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

static bool enabled;
static void enableTexture(NodeIF *obj)
{
	if(obj->typeValue()==ID_TEXTURE){
		TNtexture  *tc=(TNtexture*)obj;
		tc->setEnabled(enabled);
	}
}
//-------------------------------------------------------------
// TNlayer::setEnabled disable or enable all textures in layer
//-------------------------------------------------------------
void TNlayer::setEnabled(bool b){
	TNbase::setEnabled(b);
	if(!base)
		return;
	enabled=b;
	base->visitNode(enableTexture);
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
	if(left)
		delete left;
	if(base)
		delete base;
	TNlayer *newlayer=(TNlayer *)c;
	left=newlayer->left;
	left->setParent(this);
	base=newlayer->base;
	setName(newlayer->getName());
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
}
//-------------------------------------------------------------
// TNlayer::setName() set name
//-------------------------------------------------------------
void TNlayer::setName(char *s)
{
	strcpy(name_str,s);
}
//-------------------------------------------------------------
// TNlayer::setName() set name
//-------------------------------------------------------------
char *TNlayer::getName()
{
	if(strlen(name_str)>0)
		return name_str;
	return symbol();
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
	saveNode(f);
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

//-------------------------------------------------------------
// TNlayer::getInstance() return a randomly generated instance
//-------------------------------------------------------------
NodeIF *TNlayer::getInstance(){
	int last=lastn;
	lastn=getRandValue()*1135;
	char buff[2048];
		
	Planetoid *orb=(Planetoid *)getOrbital(this);
	
	std::string str=Planetoid::newLayer(orb);
	strcpy(buff,str.c_str());
	TNlayer *layer=TheScene->parse_node(buff);
	layer->setParent(getParent());
	lastn=last;
	return layer;
}
