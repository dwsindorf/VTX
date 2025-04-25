
//************************************************************
// classes Rock, RockMgr
//************************************************************
#include "SceneClass.h"
#include "RenderOptions.h"
#include "Rocks.h"
#include "Util.h"
#include "MapNode.h"
#include "ModelClass.h"
#include "AdaptOptions.h"
#include "TerrainClass.h"

extern double Hscale, Drop, MaxSize;
extern double ptable[];

static const char *def_rnoise_expr="noise(GRADIENT,0,2)\n";

static TerrainData Td;
static RockMgr *s_rm; // static finalizer
//************************************************************
// RockMgr class
//************************************************************
//	arg[0]  levels   		scale levels
//	arg[1]  maxsize			size of largest craters
//	arg[2]  mult			size multiplier per level
//	arg[3]  density			density or dexpr
//
//	arg[4]  zcomp			z compression factor
//	arg[5]  drop			z drop factor or function
//	arg[6]  noise		    noise amplitude
//	arg[7]  noise_expr		noise function
//-------------------------------------------------------------
TNode *RockMgr::default_noise=0;
RockMgr::RockMgr(int i) : PlacementMgr(i)
{
	noise_radial=0;
	noise_ampl=1;
	zcomp=0.1;
	drop=0.1;
	rnoise=0;
}
RockMgr::~RockMgr()
{
  	if(finalizer()){
#ifdef DEBUG_PMEM
  		printf("RockMgr::free()\n");
#endif
        DFREE(default_noise);
	}
}

//-------------------------------------------------------------
// RockMgr::init()	initialize global objects
//-------------------------------------------------------------
void RockMgr::init()
{
	if(default_noise==0){
#ifdef DEBUG_PMEM
  		printf("RockMgr::init()\n");
#endif
	   default_noise=(TNode*)TheScene->parse_node((char*)def_rnoise_expr);
	}
	PlacementMgr::init();
}

//-------------------------------------------------------------
// RockMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *RockMgr::make(Point4DL &p, int n)
{
    return new Rock(*this,p,n);
}

//************************************************************
// class Rock
//************************************************************
Rock::Rock(PlacementMgr&m, Point4DL&p,int n) : Placement(m,p,n)
{
}

//-------------------------------------------------------------
// Rock::set_terrain()	impact terrain
//-------------------------------------------------------------
bool Rock::set_terrain(PlacementMgr &pmgr)
{
	double d=pmgr.mpt.distance(center);
	double r,z,rm=0;

	RockMgr &mgr=(RockMgr&)pmgr;
	double thresh=mgr.noise_radial;
	double td=mgr.drop*mgr.maxsize;
	double t=lerp(td,0,1,radius*(1+thresh),0);
	r=radius;

	if(d>t)
		return false;

 	//if(thresh>0 && d<radius*(1+thresh)){
 	if(thresh>0){
 		SPUSH;
		Point4D np;
		if(mgr.offset_valid())
		    np=(mgr.mpt-mgr.offset)*(1/radius);
		else
		    np=(mgr.mpt)*(1/radius);
 		np=np+1/radius;
 		if(TheNoise.noise3D())
 		    np.w=0;
 		TheNoise.push(np);
 		CurrentScope->revaluate();
 		mgr.rnoise->eval();
 		TheNoise.pop();
 		rm=0.25*S0.s*mgr.noise_ampl*radius;
 		SPOP;
		d+=rm;
		r-=rm;
 	}
	
	z=mgr.base;
	
	S0.set_flag(ROCKBODY);

	setActive(true);

    z-=0.5*mgr.zcomp*r/Hscale;
	if(r>d)
		z+=(1-mgr.zcomp)*sqrt(r*r-d*d)/Hscale;
    if(z>mgr.ht)
        mgr.ht=z;
    return true;
}

//************************************************************
// TNrocks class
//************************************************************
TNrocks::TNrocks(int t, TNode *l, TNode *r, TNode *b) : TNplacements(t|ROCKS,l,r,b)
{
    mgr=new RockMgr(type);
	TNarg &args=*((TNarg *)left);
	TNode *arg=args[3];
	if(arg && (arg->typeValue() != ID_CONST))
		mgr->dexpr=arg;
	set_collapsed();
	id=0;
}

//-------------------------------------------------------------
// TNrocks::applyExpr() apply expr value
//-------------------------------------------------------------
void TNrocks::applyExpr()
{
    if(expr){
 		TNrocks* rocks=(TNrocks*)expr;
        DFREE(left);
        left=rocks->left;
        left->setParent(this);
        rocks->init();
		delete mgr;
		type=rocks->type;
		mgr=rocks->mgr;
		rocks->left=0;
		rocks->mgr=0;
		rocks->base=0;
		eval();
		delete rocks;
        expr=0;
    }
    if(base)
        base->applyExpr();
}

//-------------------------------------------------------------
// TNrocks::replaceChild replace content
//-------------------------------------------------------------
NodeIF *TNrocks::replaceChild(NodeIF *c,NodeIF *n)
{
	return TNbase::replaceChild(c,n);
}

//-------------------------------------------------------------
// TNrocks::addAfter append x after base if c==this
// - used when adding a TNnode object to a TerrainMgr stack
// - always want to append other objects after Rocks in tree
//-------------------------------------------------------------
NodeIF *TNrocks::addAfter(NodeIF *c,NodeIF *x){
	if(c==this){
		TNode *node=(TNode*)x;
		if(right){
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
		}
		else{
			right=node;
			right->setParent(this);
		}
	}
	else if(c)
		c->addChild(x);
	else
		addChild(x);
	return x;
}
//-------------------------------------------------------------
// TNrocks::addChild
//-------------------------------------------------------------
NodeIF *TNrocks::addChild(NodeIF *x){
	TNode *node=(TNode*)x;
	//cout<<"rocks::addChild "<<x->typeName()<<end;
	if(collapsed()){
		if(right){
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
		}
		else{
			right=node;
			right->setParent(this);
		}
	}
	else{
		if(base){
			node=new TNadd(node,base);
			node->setParent(this);
			base=node;
		}
		else{
			base=node;
			base->setParent(this);
		}
	}
	return x;
	//TNplacements::addChild(x);
}

//-------------------------------------------------------------
// TNrocks::init() initialize the node
//-------------------------------------------------------------
void TNrocks::init()
{
	RockMgr *rmgr=(RockMgr*)mgr;
	//cout<<base->typeName()<<" "<<right->typeName()<<endl;
	rmgr->init();
	TNplacements::init();
	TNarg &args=*((TNarg *)left);

	if(args[7]){
		TNarg *tamp=args[6];
		tamp->eval();
		rmgr->noise_ampl=S0.s;
		rmgr->rnoise=args[7];
		rmgr->rnoise->eval();
		if(rmgr->rnoise->typeValue()==ID_NOISE){
			TNnoise *noise=(TNnoise *)rmgr->rnoise;
			//cout<<noise->mx<<" "<<noise->mn<<endl;
			rmgr->noise_radial=noise->mx;
			rmgr->noise_radial=clamp(rmgr->noise_radial,0,1.5);
		}
	}
}

//-------------------------------------------------------------
// TNrocks::eval() evaluate the node
//-------------------------------------------------------------
void TNrocks::eval()
{
	//static int cnt=0;
	TerrainData rock;
	TerrainData ground;
	int i;

	if(!isEnabled()){
		if(right)
			right->eval();
		return;
	}

	S0.set_flag(ROCKLAYER);
	int in_map=S0.get_flag(CLRTEXS);

    if(CurrentScope->rpass()){
		INIT;
		
		if(right)
			right->eval();
		INIT;

		Td.add_id();
		id=Td.rids;
		Td.rids++;
		Td.tp->ntexs=0;
		if(!in_map) 
			S0.set_flag(CLRTEXS);

 		if(base)
			base->eval();

		if(!in_map)    // in case we were in another map on entry
			S0.clr_flag(CLRTEXS);
       return;
    }
    
    if(!in_map && (Td.rid==0))
    	Td.begin();
	ground.p.z=0;
	
	
	INIT;
	right->eval();
    if(Td.rid==0){
    	//if(!in_map)
		S0.next_id();
		Td.insert_strata(S0);
    }
    //if(cnt%100==0)
	//	cout<<Td.rid<<" "<<Td.tids<<endl;
    Td.rid++;
    //cnt++;
    
    //S0.set_id(Td.tids);

 	ground.copy(S0);
    //INIT;

	RockMgr *rmgr=(RockMgr*)mgr;

	TNplacements::eval();  // evaluate common arguments

	TNarg &args=*((TNarg *)left);
	TNarg *a=args.index(4);
	if(a){                // geometry exprs
		double arg[3];
		int n=getargs(a,arg,3);
		if(n>0) rmgr->zcomp=arg[0];      // compression factor
		if(n>1) rmgr->drop=arg[1];       // drop factor
	}
	
	S0.p.z=0;
	base->eval();
	if(!S0.pvalid())
		S0.p.z=ground.p.z;
	rmgr->base=S0.p.z-rmgr->drop*rmgr->maxsize/Hscale;
	S0.next_id();
	rock.copy(S0);

	//INIT;
    rmgr->ht=mgr->base;
	rmgr->eval();

	if(rmgr->noise_radial)
	 	CurrentScope->revaluate();

	//INIT;
    rock.p.z=rmgr->ht;

	if(rock.p.z>ground.p.z){		
		S0.copy(rock);
		S0.set_flag(ROCKBODY);
	}
	else{
		S0.copy(ground);
		S0.clr_flag(ROCKBODY);
	}
	//cout<<S0.c.blue()<<endl;
	S0.set_flag(LOWER);
    Td.insert_strata(rock);
	S0.set_flag(ROCKLAYER);
    if(!in_map && (Td.rid==Td.rids))
		Td.end();
}
//-------------------------------------------------------------
// TNrocks::hasChild return true if child exists
//-------------------------------------------------------------
static int find_type=0;
static bool find_test=false;
static void findType(NodeIF *obj)
{
	if(obj->typeValue()==find_type){
		find_test=true;
		obj->setFlag(NODE_STOP);
	}
}
bool TNrocks::hasChild(int type){
	find_type=type;
	find_test=false;
	if(base)
		base->visitNode(findType);
	return find_test;
}

