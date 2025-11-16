
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
#include "UniverseModel.h"

extern double Hscale, Drop, MaxSize;
extern double ptable[];

static const char *def_rnoise_expr="noise(GRADIENT,0,2)\n";

static TerrainData Td;

// 3d rocks using marching cubes
// TODO:
// 1. generate color test if 3d flag is set (MC3D) - done
// 2. set density gradient on surface if 3d (as in plants)
// 3. if 3d don't generate a new layer (test color and density on surface layer)
//    - deform surface like craters
//    - need to collect textures and color separately
// 4. capture size and position information for placement
// 5. create array of 3d rocks with placement and size info
// 6. capture textures and color that would be set in MapData in 2d (like plants)
// 7. for each layer and rock in layer emit rock as a marchingCubesObj

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
	MSK_SET(type,PLACETYPE,ROCKS);
	noise_ampl=1;
	zcomp=0.1;
	drop=0.1;
	rnoise=0;
	rdist=0;
	pdist=1;
	rx=ry=0;
#ifdef TEST_ROCKS
    set_testColor(true);
#endif

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
// RockMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *RockMgr::make(Point4DL &p, int n)
{
    return new Rock(*this,p,n);
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
//************************************************************
// Rock class
//************************************************************
Rock::Rock(PlacementMgr&m, Point4DL&p,int n) : Placement(m,p,n)
{
}

//-------------------------------------------------------------
// Rock::set_terrain()	impact terrain 
//-------------------------------------------------------------
bool Rock::set_terrain(PlacementMgr &pmgr)
{
	double r,z,rm=0;
	RockMgr &mgr=(RockMgr&)pmgr;
	
	mgr.pdist=1;
	if(radius==0)
		return false;
	
	double d=pmgr.mpt.distance(center);
	
	double thresh=mgr.noise_ampl;
	double td=mgr.drop*mgr.maxsize;
	double t=1.75*radius;

	r=radius;

	if(d>t)
		return false;
	if(pmgr.testColor())
		Placement::set_terrain(pmgr);
	
	mgr.pdist=d/radius;
	mgr.pdist=clamp(mgr.pdist,0,1);

 	if(mgr.noise_ampl>0){
 		double nf=mgr.noise_ampl*radius/Hscale;
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
 		double z=0;
 		mgr.rnoise->eval();
 		if(mgr.rnoise->typeValue()==ID_POINT){
 			z=S0.p.z;
 			mgr.rx=nf*S0.p.x;
 			mgr.ry=nf*S0.p.y; 	
  		}
 		else
 			z=S0.s;
 		TheNoise.pop();
 		rm=0.25*z*mgr.noise_ampl*radius;
 		SPOP;
		d+=rm;
		r-=rm;
 	}
	mgr.rdist=d/r;
	
	d=clamp(d,0,1);
	
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
// Rock3DMgr class
//************************************************************
Rock3DMgr::Rock3DMgr(int i) : RockMgr(i)
{
	MSK_SET(type,PLACETYPE,MCROCKS);
#ifdef TEST_ROCKS
    set_testColor(true);
#endif
}

void Rock3DMgr::eval(){	
	PlacementMgr::eval(); 
}

//-------------------------------------------------------------
// RockMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *Rock3DMgr::make(Point4DL &p, int n)
{
    return new Placement(*this,p,n);
}

bool Rock3DMgr::testColor() { 
	return PlacementMgr::testColor()?true:false;
}
bool Rock3DMgr::testDensity(){ 
	return true;
}
//************************************************************
// Rock3D class
//************************************************************
//Rock3D::Rock3D(PlacementMgr&m, Point4DL&p,int n) : Rock(m,p,n)
//{
//	mcObject=0;
//}
//-------------------------------------------------------------
// Rock::set_terrain()	impact terrain
//-------------------------------------------------------------
//bool Rock3D::set_terrain(PlacementMgr &pmgr)
//{
//	return Placement::set_terrain(pmgr);
//}

//************************************************************
// TNrocks class
//************************************************************
TNrocks::TNrocks(int t, TNode *l, TNode *r, TNode *b) : TNplacements(t|ROCKS,l,r,b)
{
	if(is3D())
		mgr=new Rock3DMgr(type);
	else	
    	mgr=new RockMgr(type);
	TNarg &args=*((TNarg *)left);
	TNode *arg=args[3];
	if(arg && (arg->typeValue() != ID_CONST))
		mgr->dexpr=arg;
	set_collapsed();
	rock_id=0;
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
// TNrocks::replaceChild replace content
//-------------------------------------------------------------
NodeIF *TNrocks::replaceNode(NodeIF *c)
{
	((TNrocks*)c)->right=right;
	((TNrocks*)c)->parent=parent;
	parent->replaceChild(this,c);
	delete base;
	base=0;
	TheScene->rebuild_all();
	return c;
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
	rmgr->init();
	TNplacements::init();
	TNarg &args=*((TNarg *)left);

	if(args[7]){
		TNarg *tamp=args[6];
		tamp->eval();
		rmgr->noise_ampl=S0.s;
		rmgr->rnoise=args[7];			
	}
	mgr->set_first(1);
}


//-------------------------------------------------------------
// TNrocks::eval3d() evaluate the node
//-------------------------------------------------------------
void TNrocks::eval3d()
{
	if(CurrentScope->rpass()){
		INIT;		
		if(right) // ground
			right->eval();
		INIT;
		int nrocks=Td.tp->rocks.size;
		rock_id=nrocks;	
		mgr->instance=rock_id;
		Td.tp->rocks.add(this);
		mgr->setHashcode();
		return;
	}
	Rock3DMgr *rmgr=(RockMgr*)mgr;
	INIT;
	if(right) // ground
		right->eval();
	TerrainData ground;
	ground.copy(S0);
	INIT;

	TNplacements::eval(); // evaluate common arguments (0-3)
 	INIT;
		
 	rmgr->eval();
	S0.copy(ground);
	rmgr->setTests();
}
//-------------------------------------------------------------
// TNrocks::eval() evaluate the node
//-------------------------------------------------------------
void TNrocks::eval() {
	if (!isEnabled() || TheScene->viewtype != SURFACE) {
		if (right)
			right->eval();
		return;
	}
	if (is3D()) {
		eval3d();
		return;
	}

	bool test = is3D();
	TerrainData rock;
	TerrainData ground;
	int i;
	bool first = (right && right->typeValue() != ID_ROCKS);
	bool last = getParent()->typeValue() != ID_ROCKS;
	INIT;
	S0.set_flag(ROCKLAYER);
	int in_map = S0.get_flag(CLRTEXS);

	if (CurrentScope->rpass()) {
		INIT;
		if(right) // ground
			right->eval();
		INIT;
		Td.add_id();
		Td.tp->set_rock(true);
		Td.tp->ntexs=0;
		Td.tp->set_rock(true);
		if(!in_map)
		S0.set_flag(CLRTEXS);
		if(base)// rock texs
			base->eval();
		if(!in_map)// in case we were in another map on entry
			S0.clr_flag(CLRTEXS);
		mgr->setHashcode();
		return;
	}

	if (!in_map && first)
		Td.begin();
	ground.p.z = 0;

	INIT;
	bool other_rock = false;
	if (right)
		right->eval(); // ground
	other_rock = S0.get_flag(ROCKBODY);
	S0.p.z-=Drop;
	if (first) {
		S0.next_id();
		Td.insert_strata(S0);
	}
	ground.copy(S0);
	INIT;

	RockMgr *rmgr = (RockMgr*) mgr;
	TNplacements::eval(); // evaluate common arguments (0-3)
	TNarg &args = *((TNarg*) left);
	TNarg *a = args.index(4);
	if (a) {                // geometry exprs
		double arg[3];
		int n = getargs(a, arg, 3);
		if (n > 0)
			rmgr->zcomp = arg[0];      // compression factor
		if (n > 1)
			rmgr->drop = arg[1];       // drop factor
	}
	INIT;
	if (base)
		base->eval();
	if (!S0.pvalid())
	S0.p.z=ground.p.z;
	rmgr->base = S0.p.z-rmgr->drop*rmgr->maxsize/Hscale;
	S0.next_id();
	rock.copy(S0);
	INIT;
	rmgr->ht = mgr->base;
	rmgr->eval();  // calls set_terrain sets mgr->ht

	if (rmgr->noise_ampl)
		CurrentScope->revaluate();
	rock.p.z = rmgr->ht;
	double delta = (rock.p.z - ground.p.z) / fabs(ground.p.z);
	if (delta > 0) {
		rock.p.x = rmgr->rx * (1 - rmgr->rdist);
		rock.p.y = rmgr->ry * (1 - rmgr->rdist);
		S0.copy(rock);
		if (!test) {
			Td.tp->set_rock(true);
			S0.set_flag(ROCKBODY);
		}
	}
	else {
		Td.tp->set_rock(false);
		S0.copy(ground);
		if(!other_rock)
			S0.clr_flag(ROCKBODY);
	}
	Td.insert_strata(rock);
	if (!in_map && last)
		Td.end();
	//cout<<mgr->cval<<endl;
	rmgr->setTests();
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

// called by VtxSceneDialog ->scene->makeObject
// called by Scene->makeObject
// this = prototype obj=parent(layer) m=GN_TYPE
// this->setParent(obj) already set
// VtxSceneDialog ->addtoTree(this)
NodeIF *TNrocks::getInstance(NodeIF *obj, int m){	
	return newInstance(m);
}
bool TNrocks::randomize(){
	double f=0.2;
	TNarg *arg=(TNarg*)left;
	char buff[1024];
	buff[0]=0;
	arg=arg->index(1);
	int i=0;
	while(arg){
		if(arg->left->typeValue()==ID_CONST){
			TNconst *val=(TNconst*)arg->left;
			//cout<<s[i]<<" before:"<<val->value;
			val->value*=(1+f*s[i]);
			//cout<<" after:"<<val->value<<endl;
		}
		else if(arg->left->typeValue()==ID_NOISE){
			TNnoise *val=(TNnoise*)arg->left;
			buff[0]=0;
			val->valueString(buff);
			//cout<<"before:"<<buff<<endl;
			std::string str=TNnoise::randomize(buff,f,1);
			TNnoise *newval=TheScene->parse_node(str.c_str());
			arg->left=newval;
			//cout<<"after:"<<str<<endl;
			delete val;
		}
		i++;
     	arg=arg->next();
	}
	return true;
}

// this=prototype, this->parent=layer
#define PLANET_ROCKS
TNrocks *TNrocks::newInstance(int m){
	NodeIF::setRands();
	int gtype=m&GN_TYPES;
#ifdef PLANET_ROCKS	
	Planetoid *orb=(Planetoid *)getOrbital(this);
	Planetoid::makeLists();
	std::string str=Planetoid::newRocks(orb,gtype);
	TNrocks *rocks=TheScene->parse_node(str.c_str());
	rocks->setParent(parent);
#else	
	// randomize random selection from objects/rocks
	LinkedList<ModelSym*>flist;
	TheScene->model->getFileList(TN_ROCKS,flist);
	int ival=std::rand() % flist.size;
	ModelSym *sym=flist[ival];

	char sbuff[1024];
	TheScene->model->getFullPath(sym,sbuff);
	TNrocks *rocks=TheScene->open_node(this,sbuff);
	sbuff[0]=0;
	
	TNarg *arg=(TNarg*)rocks->left;
	
	double args[20];
	
	TNode* node=arg->index(1)->left;
	if(node->typeValue()==ID_CONST){
		TNconst *size=(TNconst*)node;
		switch(gtype){
		default:
			break;
		case GN_LARGE:
			size->value=1e-5;
			break;
		case GN_MED:
			size->value=1e-6;
			break;
		case GN_SMALL:
			size->value=1e-7;
			break;
		}
	}
#endif
	rocks->randomize();
	return rocks;
}
