#include "SceneClass.h"
#include "RenderOptions.h"
#include "Plants.h"
#include "Util.h"
#include "MapNode.h"
#include "ModelClass.h"
#include "AdaptOptions.h"
#include "FileUtil.h"
#include "GLSLMgr.h"
#include "Effects.h"
#include "TerrainClass.h"

#define COLOR_TEST
#define DENSITY_TEST

#define SRAND(x) 	(rands[PERM((x++))])
#define URAND(x) 	(rands[PERM((x++))]+0.5)


#define FIRST  1

// TODO: 
// 1) Better branching model
//  Currently all branches spawn from the same point in screen space
//  Improve this by trying the following:
//  a)allow branches to spawn at random locations along parent stem vector
//  b)don't require branch starting width to be the same as parents width
//    - scale next segment width based on random factor or (multi-branch case) size of child branch
//    - in screen space offset origin of new fork on a line between parents tip xy left and right
//    - make sure new tip base is contained within parent's top tip
//  c) modify branch z position based on dot-product with new branch vector with eye vector 
//     - add z distance to back facing branches (-dp) to simulate parent branch width
// 2) In multi-branch cases have a minimum level to start producing new branches and max level to stop
//    - min level prevents branches from forming lower in parent trunk
//    - child branches should form only towards the tip of parent branches
//    - Label branch ends as "terminal" if next level would be below width of branch level thresholds
// 3) add support for textures
// 4) add support for bump shading from textures
// 5) use a spline function when connecting levels on same branch
//   - generate additional line segments between segment end points (no need for branching)
// 6) better lighting model for branches
//   - currently only uses x-direction in screen space
//   - works good for vertical trunks but makes horizontal branches look flat
//   - idea:incorporate delta-y to get better effect ?
// 7) go to a true 3-d model vs. lines with rendered billboards in screen space
	
// BUGS/problems
// 1) see a lot of jitter on branches when moving around
//   problem traced to starting width variation based on position
//   - if width is used for early exit this results in different number of RAND calls
//     for the same plant which changes "seed" value resulting in a different branching pattern
//   - solution (workaround)not use width as an exit criteria but reduce overhead by bypassing
//     drawing and calculation sections in emit

//************************************************************
// classes PlantPoint, PlantMgr
//************************************************************

extern double Hscale, Drop, MaxSize,Height,Phi,Density;
extern double ptable[];
extern Point MapPt;
extern double  zslope();
extern NameList<LongSym*> POpts;

extern void inc_tabs();
extern void dec_tabs();
extern char   tabs[];
extern int addtabs;

extern int test3;
static double sval=0;
static double cval=0;
static double mind=0;
static double htval=0;
static int ncalls=0;
static int nhits=0;
static double thresh=1.0;    // move to argument ?
static double ht_offset=1; // move to argument ?

//static double roff_value=0;
//static double roff2_value=0.05*PI;

static double roff_value=1e-6;//0.5*PI;
static double roff2_value=0.5;

static double min_render_pts=2; // for render
static double min_adapt_pts=5; //  for adapt - increase resolution only around nearby plants

static int cnt=0;
static int tests=0;
static int pts_fails=0;
static int dns_fails=0;

static TerrainData Td;
static PlantMgr *s_mgr=0;
static int hits=0;
static int branch_nodes;
static int trunk_nodes;
static int line_nodes;

//#define DEBUG_PMEM

#define USE_AVEHT
//#define SHOW
#define MIN_VISITS 2
#define TEST_NEIGHBORS 1
#define TEST_PTS 
//#define LINES_ONLY
#define DRAW_TRIANGLES
#define TRIANGLE_LINES
//#define SHOW_STATS
//#define DUMP
#ifdef TRIANGLE_LINES
#define MIN_TRIANGLE_WIDTH 2.0
#define MIN_LINE_WIDTH 1.0
#define MIN_DRAW_WIDTH MIN_LINE_WIDTH
#else
#define MIN_TRIANGLE_WIDTH 1.0
#define MIN_DRAW_WIDTH MIN_TRIANGLE_WIDTH

#endif
#ifdef DUMP
static void show_stats()
{
	if(s_mgr)
		s_mgr->dump();
	cout<<"calls="<<ncalls<< " hits="<<nhits<<endl;
}
#endif

static int randval=0;
class SData {
public:
    double v;
    double f;
    double value()   { return v;}
};
static SData   sdata[256];
static ValueList<SData*> slist(sdata,256);
static int          scnt;

//************************************************************
// PlantMgr class
//************************************************************
//	arg[0]  levels   		scale levels
//	arg[1]  maxsize			size of largest craters
//	arg[2]  mult			size multiplier per level
//	arg[3]  density			density or dexpr
//
//-------------------------------------------------------------
PlantMgr::PlantMgr(int i,TNplant *p) : PlacementMgr(i)
{
#ifdef DUMP
	if(!s_mgr)
		add_finisher(show_stats);
#endif
	type|=SPRITES;
	plant=p;
	s_mgr=this;
	roff=roff_value;
	roff2=roff2_value;
	level_mult=0.2;
	slope_bias=0;
	ht_bias=0;
	lat_bias=0;
	rand_flip_prob=0.5;
	variability=1;
	dexpr=0;
	instance=0;
	select_bias=0;
	set_ntest(TEST_NEIGHBORS);
}
PlantMgr::~PlantMgr()
{
  	if(finalizer()){
  		s_mgr=0;
#ifdef DEBUG_PMEM
  		printf("PlantMgr::free()\n");
#endif
	}
}

//-------------------------------------------------------------
// PlantMgr::init()	initialize global objects
//-------------------------------------------------------------
void PlantMgr::init()
{
#ifdef DEBUG_PMEM
  	printf("PlantMgr::init()\n");
#endif
	PlacementMgr::init();
	ncalls=0;
	nhits=0;
	cnt=0;
	ss();
  	reset();
 }

void PlantMgr::eval(){	
	PlacementMgr::eval(); 
	if(!first() || !scnt)
	    return;
	for(int i=0;i<scnt;i++){
	    slist.base[i]=sdata+i;
	}
	slist.size=scnt;
	slist.sort();
	
	for(int i=0;i<scnt;i++){
	   double f=slist.base[i]->f;
	   cval=f;
	}
}

void PlantMgr::reset(){
	PlacementMgr::reset();
	tests=pts_fails=dns_fails=0;
	cval=0;
	scnt=0;
}
//-------------------------------------------------------------
// PlantPoint::set_terrain()	impact terrain
//-------------------------------------------------------------
bool PlantMgr::valid()
{ 
    tests++;

#ifdef TEST_PTS
	double mps=min_render_pts;
	if(TheScene->adapt_mode())
		mps=min_adapt_pts;
	Point pv=MapPt;
	double d=pv.length();
	
	double r=TheMap->radius*size;
	double f=TheScene->wscale*r/d;
    double pts=f;
    if(pts<mps){
    	pts_fails++;
    	return false;
    }
#endif  
    // TODO set density biases here ?

    if(density<=0){
    	dns_fails++;
    	return false;
    }
	return true;
}

bool PlantMgr::setProgram(){
	//glShadeModel(GL_FLAT);
	
	branch_nodes=0;
	trunk_nodes=0;
	line_nodes=0;
	TerrainProperties *tp=Td.tp;
	//cout<<"PlantMgr::setProgram()"<<endl;
	
	char defs[1024]="";
	sprintf(defs+strlen(defs),"#define NLIGHTS %d\n",Lights.size);
	if(Render.haze())
		sprintf(defs+strlen(defs),"#define HAZE\n");

    bool do_shadows=Raster.shadows() && (Raster.twilight() || Raster.night());
	if(do_shadows && !TheScene->light_view()&& !TheScene->test_view() &&(Raster.farview()))
		sprintf(defs+strlen(defs),"#define SHADOWS\n");
	
#ifdef DRAW_TRIANGLES
	sprintf(defs+strlen(defs),"#define DRAW_TRIANGLES\n");
#endif
	GLSLMgr::setDefString(defs);
	GLSLMgr::loadProgram("plants.bb.vert","plants.frag");
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;

	GLSLVarMgr vars;
	
	Planetoid *orb=(Planetoid*)TheScene->viewobj;
	Color diffuse=orb->diffuse;
	Color shadow=orb->shadow_color;
	Color haze=Raster.haze_color;
	
	vars.newFloatVec("Diffuse",diffuse.red(),diffuse.green(),diffuse.blue(),diffuse.alpha());
	vars.newFloatVec("Shadow",shadow.red(),shadow.green(),shadow.blue(),shadow.alpha());
	vars.newFloatVec("Haze",haze.red(),haze.green(),haze.blue(),haze.alpha());
	vars.newFloatVar("haze_zfar",Raster.haze_zfar);
	vars.newFloatVar("haze_grad",Raster.haze_grad);
	vars.newFloatVar("haze_ampl",Raster.haze_hf);
	vars.newBoolVar("lighting",Render.lighting());

	//vars.newFloatVec("vpoint",TheScene->vpoint.x,TheScene->vpoint.y,TheScene->vpoint.z,0);

	double zn=TheScene->znear;
	double zf=TheScene->zfar;
	double ws1=1/zn;
	double ws2=(zn-zf)/zf/zn;

	vars.newFloatVar("ws1",ws1);
	vars.newFloatVar("ws2",ws2);

	vars.setProgram(program);
	vars.loadVars();
	GLSLMgr::CommonID1=glGetAttribLocation(program,"CommonAttributes1"); // Constants1
	//GLSLMgr::CommonID2=glGetAttribLocation(program,"CommonAttributes2"); // Constants1

	GLSLMgr::setProgram();
	GLSLMgr::loadVars();
	
	for(int i=0;i<tp->plants.size;i++){
		TerrainProperties::sid=i;
		tp->plants[i]->setProgram();
	}
	
	int n=Plant::plants.size;
	int l=randval;
	
	glEnable(GL_BLEND);
	TNplant::clearStats();
	for(int i=n-1;i>=0;i--){
		PlantData *s=Plant::plants[i];
		
		int id=s->get_id();
		
		TNplant *plant=s->mgr->plant;
		plant->base_point=s->base;
		plant->size=s->radius; // placement size
		plant->pntsize=s->pntsize;
		
		Point pp=Point(s->point.x,s->point.y,s->point.x);
		
		
		double r=Random(pp);
		randval=256*r+id;
		int l=randval;
//		int oldforks=TNplant::stats[0][0];
			
		plant->setProgram();
//		int forks=TNplant::stats[0][0]-oldforks;
//		if(i<3){
//		    cout<<i<<" "<<l<<" "<<randval<<" "<<forks<<endl;
//		}
		
	}
	randval=l;
	TNplant::showStats();
	
	//cout<<"branches:"<<TNBranch::branches<<" lines:"<<TNBranch::lines<<endl;
	return true;
}
//-------------------------------------------------------------
// PlantMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *PlantMgr::make(Point4DL &p, int n)
{
    return new PlantPoint(*this,p,n);
}

//************************************************************
// class PlantPoint
//************************************************************
PlantPoint::PlantPoint(PlantMgr&m, Point4DL&p,int n) : Placement(m,p,n)
{
	ht=0;
	aveht=0;
	wtsum=0;
	dist=1e16;
	visits=0;
	hits=0;
	mind=1e16;
	mgr=&m;
	
	plants_dim=m.plants_dim;
	variability=m.mult;
	rand_flip_prob=m.rand_flip_prob;
	select_bias=m.select_bias;
	instance=m.instance;
	//cout<<instance<<" ";

	flags.s.active=false;
}

//-------------------------------------------------------------
// PlantPoint::set_terrain()	impact terrain
//-------------------------------------------------------------
bool PlantPoint::set_terrain(PlacementMgr &pmgr)
{
	double d=pmgr.mpt.distance(center);
	d=d/radius;
	PlantMgr &mgr=(PlantMgr&)pmgr;
	sval=0;
	
	if(d>thresh)
		return false;
	visits++;
    flags.s.active=true;
	sval=lerp(d,0,thresh,0,1);

    double wt=1/(0.01+sval);
    aveht+=Height*wt;
	
    wtsum+=wt;

	if(d<dist){
		ht=Height;
		dist=d;
		mind=d;
		hits++;
	}
	::hits++;

 	sdata[scnt].v=hid;
   	sdata[scnt].f=sval;
  	if(scnt<255)
  	    scnt++;
	return true;
}

void PlantPoint::reset(){
	flags.s.active=0;
	visits=0;
	hits=0;
	dist=1e6;
	aveht=0;
	wtsum=0;
}
void PlantPoint::dump(){
	if(flags.s.valid && flags.s.active){
		Point4D p(point);
		p=center;
		char msg[256];
		char vh[32];
		sprintf(vh,"%d:%d",visits,hits);
		sprintf(msg,"%-3d %-2d %-8s dist:%-0.4f ht:%-1.6f x:%-1.5f y:%-1.5f z:%1.5f",cnt++,flags.l,vh,dist,ht,p.x,p.y,p.z);
		cout<<msg<<endl;
	}
}
//==================== PlantData ===============================
PlantData::PlantData(PlantPoint *pnt,Point bp,double d, double ps){
	type=pnt->type;
	ht=pnt->ht;
	
	point=pnt->point;
	
	aveht=pnt->aveht/pnt->wtsum;
	//center=tp;
	base=bp;
	
	radius=pnt->radius;
    pntsize=ps;
 	distance=d;//TheScene->vpoint.distance(t);
 	plants_dim=pnt->plants_dim;
	visits=pnt->visits;
	variability=pnt->variability;
	rand_flip_prob=pnt->rand_flip_prob;
	select_bias=pnt->select_bias;
	instance=pnt->instance;
	mgr=pnt->mgr;
}

void PlantData::print(){
	char msg[256];
	Point pp=Point(point.x,point.y,point.z);
	//pp=pp.normalize();
	//Point ps=pp.spherical();
	double h=TheMap->radius*TheMap->hscale;
	
	//sprintf(msg,"visits:%-1d t:%-1.6g p:%-1.3g ht:%-1.4f aveht:%-1.4f dist:%g",visits,ps.x+180,ps.y,h*ht/FEET,h*aveht/FEET,distance/FEET);
	sprintf(msg,"visits:%-1d x:%d y:%d z:%d ht:%-1.4f aveht:%-1.4f dist:%g",visits,point.x,point.y,point.z,h*ht/FEET,h*aveht/FEET,distance/FEET);
	cout<<msg<<endl;
	
}
//===================== Plant ==============================
ValueList<PlantData*> Plant::plants;
//-------------------------------------------------------------
// Plant::Plant() Constructor
//-------------------------------------------------------------
Plant::Plant(int l, TNode *e)
{
	type=l;
	plant_id=get_id();
	expr=e;
	valid=false;
}

void Plant::reset()
{
	plants.free();
#ifdef GLOBAL_HASH
	PlacementMgr::free_htable();
#else
	TerrainProperties *tp=Td.tp;
	for(int i=0;i<tp->plants.size;i++){
		Plant *plant=tp->plants[i];
		plant->mgr()->free_htable();
	}
#endif
}

//-------------------------------------------------------------
// Plant::collect() collect valid plant points
//-------------------------------------------------------------
void Plant::collect()
{

#ifdef TEST_PTS
	if(tests>0)
		cout<<"tests:"<<tests<<" fails  pts:"<<100.0*pts_fails/tests<<" %"<<" dns:"<<100.0*dns_fails/tests<<endl;
#endif
	//cout<<"znear:"<< TheScene->znear<<" zfar:"<<TheScene->zfar<<" "<<TheScene->zfar/TheScene->znear<<endl;
	int new_plants=0;
	int bad_pts=0;
#ifdef SHOW_STATS	
	int trys=0;
	int visits=0;
	int bad_visits=0;
	int bad_valid=0;
	int bad_active=0;
#endif	
#ifdef GLOBAL_HASH
	PlacementMgr::ss();
	PlantPoint *s=(PlantPoint*)PlacementMgr::next();
#else
	TerrainProperties *tp=Td.tp;
	for(int i=0;i<tp->plants.size;i++){
#ifdef SHOW_STATS	
		trys=visits=bad_visits=bad_valid=bad_active=bad_pts=new_plants=0;
#endif
		Plant *plant=tp->plants[i];
		plant->mgr()->ss();
		PlantPoint *s=(PlantPoint*)plant->mgr()->next();
#endif
	while(s){
#ifdef SHOW_STATS
		trys++;
		
		if(s->visits<MIN_VISITS)
			bad_visits++;
		if(!s->flags.s.valid)
			bad_valid++;
		if(!s->flags.s.active)
			bad_active++;
#endif	
		if(s->visits>=1 && s->flags.s.valid && s->flags.s.active){
			Point4D	p(s->center);
			Point pp=Point(p.x,p.y,p.z);
			Point ps=pp.spherical();
#ifdef USE_AVEHT
			double ht=s->aveht/s->wtsum;
#else
			double ht=s->ht;
#endif			
			Point base=TheMap->point(ps.x, ps.y,ht); // spherical-to-rectangular
			Point bp=Point(-base.x,base.y,-base.z);  // Point.rectangular has 180 rotation around y
			double d=bp.distance(TheScene->vpoint);  // distance	
			double r=TheMap->radius*s->radius;
			double f=TheScene->wscale*r/d;
		    double pts=f;
		    double minv=lerp(pts,min_render_pts,10*min_render_pts,1,2*MIN_VISITS); 
		    bool pts_test=true;
#ifdef TEST_PTS
		    if(pts<min_render_pts){
		    	pts_test=false;
		    	bad_pts++;
		    }
#endif
		    if(pts_test && s->visits>=minv){
		    	new_plants++;
		    	plants.add(new PlantData((PlantPoint*)s,bp,d,pts));
		    }
		}
#ifdef GLOBAL_HASH
		s=PlacementMgr::next();
	  }
#else 
		s=plant->mgr()->next();
	  }	
#ifdef SHOW_STATS
	double usage=100.0*trys/PlacementMgr::hashsize;
	double badvis=100.0*bad_visits/trys;
	double badactive=100.0*bad_active/trys;
	double badpts=100.0*bad_pts/trys;
	cout<<plant->name()<<" plants "<<new_plants<<" tests:"<<trys<<" %hash:"<<usage<<" %inactive:"<<badactive<<" %small:"<<badpts<<endl;
#endif

	} // next plant
#endif
	//}
    cout<<"total plants collected:"<<plants.size<<endl;
	plants.sort();
#ifdef SHOW
	//int pnrt_num=plants.size-1;
	int pnrt_num=min(2,plants.size-1);

	for(int i=pnrt_num;i>=0;i--){
		cout<<i<<" ";
		plants[i]->print();	
	}
#endif
	
}
//-------------------------------------------------------------
// Plant::eval() evaluate TNtexture string
//-------------------------------------------------------------
void Plant::eval()
{
	int mode=CurrentScope->passmode();
	CurrentScope->set_spass();
	expr->eval(); // TNplant.eval()
	CurrentScope->set_passmode(mode);
}

bool Plant::setProgram(){
	return true;
}
bool Plant::initProgram(){
	return false;
}


//===================== TNplant ==============================
int TNplant::stats[MAX_BRANCHES][4];
int TNplant::max_branches=0;
//************************************************************
// TNplant class
//************************************************************
TNplant::TNplant(TNode *l, TNode *r) : TNplacements(0,l,r,0)
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
	plant=0;
	levels=0;
	
    mgr=new PlantMgr(PLANTS|NOLOD,this);
}

//-------------------------------------------------------------
// TNplant::~TNplant() destructor
//-------------------------------------------------------------
TNplant::~TNplant()
{
	DFREE(plant);
}

//-------------------------------------------------------------
// TNplant::applyExpr() apply expr value
//-------------------------------------------------------------
void TNplant::applyExpr()
{
   if(expr){
	    TNplant* sprt=(TNplant*)expr;
	    mgr->type=type;
		DFREE(left);
		left=expr->left;
		left->setParent(this);
		expr=0;
	}
	if(right)
		right->applyExpr();
 }
//-------------------------------------------------------------
// TNplant::init() initialize the node
//-------------------------------------------------------------
void TNplant::init()
{
	PlantMgr *smgr=(PlantMgr*)mgr;
    levels=0;
	if(plant==0)
		plant=new Plant(type,this);
	smgr->set_first(1);
	smgr->init();
	
	//TNplacements::init();
	double arg[11];
	INIT;
	TNarg &args=*((TNarg *)left);
	int n=getargs(&args,arg,11);
	
	if(n>0) max_levels=(int)arg[0]; 	// branch levels
	if(n>1) mgr->levels=(int)arg[1]; 	// scale levels
	if(n>2) mgr->maxsize=arg[2];     	// size of largest 
	if(n>3) mgr->mult=arg[3];			// random scale multiplier
	if(n>4) mgr->level_mult=arg[4];     // scale multiplier per level
	if(n>5) maxdensity=arg[5];
	if(n>6) smgr->slope_bias=arg[6];
	if(n>7) smgr->ht_bias=arg[7];
	if(n>8) smgr->lat_bias=arg[8];
	if(n>9) smgr->select_bias=arg[9];
	
	if(right)
	   right->init();
}

void TNplant::set_id(int i){
	BIT_OFF(type,PID);
	type|=i&PID;
}
//-------------------------------------------------------------
// TNplant::eval() evaluate the node
//-------------------------------------------------------------
void TNplant::eval()
{	
	SINIT;
	if(right)
		right->eval();
	if(!isEnabled()){
		return;
	}
	if(CurrentScope->rpass()){
		int size=Td.tp->plants.size;
		instance=size;	
		mgr->instance=instance;
		if(plant)
			plant->set_id(size);		
		Td.add_plant(plant);		
		return;
	}	
	if(!CurrentScope->spass()){
		return;
	}
	Color c =Color(1,1,1);
	PlantMgr *smgr=(PlantMgr*)mgr;

	htval=Height;
	ncalls++;
	
	double density=maxdensity;
	MaxSize=mgr->maxsize;
	radius=TheMap->radius;
	TerrainProperties *tp=TerrainData::tp;
		
	mgr->type=type;
	if(smgr->slope_bias){
		double slope=8*zslope();
		double f=2*lerp(fabs(smgr->slope_bias)*slope,0,1,-smgr->slope_bias,smgr->slope_bias);
#ifdef DEBUG_SLOPE_BIAS
		if(ncalls%100==0)
			cout<<"slope:"<<slope<<" f:"<<f<<endl;
#endif
		density+=f;
	}
	if(smgr->ht_bias){
		double f=2*lerp(8*fabs(smgr->ht_bias)*Height,-1,1,-smgr->ht_bias,smgr->ht_bias);
		density+=f;
	}
	if(smgr->lat_bias){
		double f=lerp(fabs(smgr->lat_bias)*fabs(2*Phi/180),0,1,-smgr->lat_bias,+smgr->lat_bias);
		density+=f;
	}
    density*=maxdensity;
	density=clamp(density,0,1);
	density=sqrt(density);

	mgr->density=density;
	double hashcode=(mgr->levels+
		            1/mgr->maxsize
					+11*tp->id
					+7*instance
					);
	mgr->id=(int)hashcode+mgr->type+PLANTS+hashcode*TheNoise.rseed;
	
	sval=0;
	hits=0;
	cval=0;
	scnt=0;

	smgr->eval();  // calls PlantPoint.set_terrain
   
	if(hits>0){ // inside target radius
		nhits++;
		double x=1-cval;
#ifdef COLOR_TEST
		if(instance==0)
			c=Color(x,1,0);
		else
			c=Color(x,1,0);
		Td.diffuse=Td.diffuse.mix(c,0.5);
#endif
#ifdef DENSITY_TEST
		x=1/(cval+1e-6);
		x=x*x;//*x*x;
		Td.density+=lerp(cval,0,0.2,0,0.05*x);
#endif
	}
 }

void TNplant::clearStats(){
	max_branches=0;
	for(int i=0;i<MAX_BRANCHES;i++){
		stats[i][0]=stats[i][1]=stats[i][2]=stats[i][3]=0;
	}
}
void TNplant::showStats(){
	for(int i=0;i<max_branches;i++){
		cout<<i<<" branches:"<<stats[i][0]<<" lines:"<<stats[i][1]<<" terminals:"<<stats[i][2]<<" skipped:"<<stats[i][3]<<endl;
	}
}
void TNplant::addLine(int id){
	if(id>=max_branches)
		max_branches++;
	stats[id][1]++;	
}
void TNplant::addBranch(int id){
	if(id>=max_branches)
		max_branches++;
	stats[id][0]++;	
}
void TNplant::addTerminal(int id){
	if(id>=max_branches)
		max_branches++;
	stats[id][2]++;	
}
void TNplant::addSkipped(int id){
	if(id>=max_branches)
		max_branches++;
	stats[id][3]++;	
}
//-------------------------------------------------------------
// TNtexture::valueString() node value substring
//-------------------------------------------------------------
void TNplant::valueString(char *s)
{
	if(strlen(name_str)>0)
		sprintf(s+strlen(s),"%s(\"%s\",",symbol(),name_str);
	else
		sprintf(s+strlen(s),"%s(",symbol());

	TNarg *arg=(TNarg*)left;
	while(arg){
		arg->valueString(s+strlen(s));
		arg=arg->next();
		if(arg)
			strcat(s,",");
	}
	strcat(s,")");
}

//-------------------------------------------------------------
// TNplant::save() archive the node
//-------------------------------------------------------------
void TNplant::save(FILE *f)
{
	char buff[1024];
	buff[0]=0;
	valueString(buff);
	//if(addtabs)
	    fprintf(f,"\n%s",tabs);
	fprintf(f,"%s",buff);
	if(right)
		right->save(f);
}

//-------------------------------------------------------------
// TNplant::save() archive the node
//-------------------------------------------------------------
void TNplant::saveNode(FILE *f)
{
	char buff[1024];
	buff[0]=0;
	valueString(buff);
	//if(addtabs)
	    fprintf(f,"\n%s",tabs);
	fprintf(f,"%s",buff);
}

bool TNplant::setProgram(){

	double length=base_point.length();
		
	Point bot=base_point;
	
	norm=bot.normalize();

	TNBRANCH *branch=0;
	if(right && right->typeValue() == ID_BRANCH) 
	    branch=(TNBRANCH*)right;
	else
		return false;

	double branch_size=length*size*branch->length;
	Point top=bot*(1+branch_size); // starting trunk size
	Point p1=bot;
	Point p2=top;
		
	double width=pntsize;
	
	Point tip;
	tip.x=width/TheScene->wscale;
	tip.y=0;
	//double f=width*pow(branch->width_taper,n)
 	branch->fork(0,p1,p2-p1,tip,length*size,width,0);

	return true;

}

//===================== TNBranch ==============================
//************************************************************
// TNBranch class
//************************************************************

TNBranch::TNBranch(TNode *l, TNode *r, TNode *b) : TNbase(0,l,b,r)
{
	set_collapsed();
	TNarg *arg=left;
	TNarg *node=arg->left;
	if(node->typeValue() == ID_STRING){		
		setName(((TNstring*)node)->value);
		left=arg->next();
		if(left)
			left->setParent(this);
		arg->right=0;
		delete arg;	
	}
	length=2;
	width_taper=0.75;
	length_taper=0.95;
	randomness=0.25;
	max_splits=4;
	first_bias=0;
	flatness=0.9;
	divergence=0.75;
	min_level=0;
	max_level=0;
}
void TNBranch::init(){
	double arg[12];
	if(!left)
		return;
	INIT;
	TNarg &args=*((TNarg *)left);
	int n=getargs(&args,arg,11);
	if(n>0)max_splits=arg[0];
	if(n>1)length=arg[1];
	if(n>2)randomness=arg[2];
	if(n>3)divergence=arg[3];
	if(n>4)flatness=arg[4];
	if(n>5)width_taper=arg[5];
	if(n>6)length_taper=arg[6];	
	if(n>7)first_bias=arg[7];
	if(n>8)min_level=arg[8];
	if(n>9)max_level=arg[9];
	
	TNplant *root=getRoot();
	max_plant_levels=root->max_levels;
	branch_id=root->levels;
	root->levels+=1;
	//cout<<"min_level="<<min_level<<" max_level="<<max_level<<endl;
	
	if(right)
		right->init();
}
void TNBranch::fork(int opt, Point start, Point vec,Point tip,double size, double width, int lvl){
	if(lvl<min_level)
		return;
	maxlvl=max_plant_levels;
	maxlvl=(max_level>0&&max_level<maxlvl)?max_level:maxlvl;
   
    if(lvl>maxlvl)
    	return;
    width*=width_taper;
    size*=length_taper;
    
	int splits=max_splits*(1+0.5*randomness*SRAND(randval));
	if(first_bias) // add more branches at start of branch fork
		splits*=first_bias;
	splits=splits<1?1:splits;
	for(int i=0;i<splits;i++){
		emit(opt,start,vec,tip,size,width,lvl);
	}
}
void TNBranch::emit(int opt, Point start, Point vec, Point tip, double size,
		double width, int lvl) {
	if (lvl < min_level)
		return;
	if (lvl > maxlvl) {
		return;
	}
	double topx = 0;
	double topy = 0;
	double botx = 1;
	double boty = 1;
	Point v, p1, p2, bot;
	Color c;
	int lev = lvl;
	lev++;
	bool terminal=branch_id==getRoot()->levels-1;
	bool first = (opt & FIRST);
	int splits = max_splits * (1 + 0.5 * randomness * SRAND(randval));
	if (first && first_bias)
		splits *= first_bias;
	splits = splits >= 1 ? splits : 1;

	if (width < MIN_DRAW_WIDTH) {
		getRoot()->addSkipped(branch_id);
		randval += 5;
	} else {
		double scale=1.0;
		if(first)
			scale=length*getRoot()->size/size/TheScene->wscale;
	
		double offset = divergence; // how much to deviate from last segment direction
		offset *= first ? first_bias : 1.0;

		size *= 1 + 0.25 * randomness * SRAND(randval);

		v = vec.normalize();
        // add a random offset to first branch split
		if(first){
			double b = 0.5*randomness*URAND(randval);
			b=b<=1?b:1;				
		 	start=start-v*b*size * length;
		}

		v.x += offset * SRAND(randval);
		v.y += offset * SRAND(randval);
		v.z += offset * SRAND(randval);

		v = v.normalize();
				
		if (flatness > 0) {
			Point n = getRoot()->norm + start;
			n = n.normalize();
			Point tp1 = n.cross(v);
			Point vp = tp1.cross(n);
			double f = flatness;
			if (first && first_bias)
				f /= first_bias;
			vp = vp.normalize(); // projection of v along surface
			Point v1 = v * (1 - f);
			Point v2 = vp * f;
			v = v1 + v2;
		}
		v = v * size * length; // v = direction along last branch
		
		p2 = start + v; //new top
		bot = p2; // new base	

		p2 = p2 - TheScene->vpoint;
		p1 = start - TheScene->vpoint;

		v = bot - start; // new vector

		c = Color(0.1, 0.5, 0.0);

		Density = ((double) lev) / max_plant_levels;
		INIT;
		if (base) { // optional color, texture etc []
			base->eval();
			if (S0.cvalid())
			c=S0.c;
		}
		double off = width / TheScene->wscale;
#ifdef LINES_ONLY
		glColor4d(c.red(), c.green(), c.blue(), 1);
		glLineWidth(width);
		glBegin(GL_LINES);
		glVertex4d(p1.x,p1.y,p1.z,0);
		glVertex4d(p2.x,p2.y,p2.z,0);
		glEnd();
#else
#ifdef DRAW_TRIANGLES
#ifdef TRIANGLE_LINES	
	if (width > MIN_TRIANGLE_WIDTH) {
#endif
		if(/**/terminal &&(width*width_taper<MIN_TRIANGLE_WIDTH || lev>maxlvl) ){
			//c=Color(1,0,0);
			getRoot()->addTerminal(branch_id);
		}
		//if(first)
		//	c=Color(0,0,1);
		getRoot()->addBranch(branch_id);

		glColor4d(c.red(), c.green(), c.blue(), 1);
		Point q = TheScene->project(v); // convert model to screen space
		double a = atan2(q.y / q.z, q.x / q.z);
		double x = -sin(a);
		double y = cos(a);

		botx = x * off;
		boty = y * off;

		topx = x * off * width_taper*scale;
		topy = y * off * width_taper*scale;
        
		botx = tip.x*scale;
		boty = tip.y*scale;
		
		// fix billboard gap in sequential levels
		//  - set bottom offsets for next level to = top offsets for previous level
		tip.x = topx;
		tip.y = topy;

		glVertexAttrib4d(GLSLMgr::CommonID1, topx, topy, botx, boty); // Constants1
		glDisable(GL_CULL_FACE);

		if (test3) { // @ key - draw lines
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(2);
		}

		glBegin(GL_TRIANGLES);
		// 1) cover rectangle by drawing 2 triangles starting at top-left			
		glVertex4d(p2.x, p2.y, p2.z, 1);
		glVertex4d(p2.x, p2.y, p2.z, 2);
		glVertex4d(p1.x, p1.y, p1.z, 3);

		glVertex4d(p2.x, p2.y, p2.z, 1);
		glVertex4d(p1.x, p1.y, p1.z, 3);
		glVertex4d(p1.x, p1.y, p1.z, 4);

		// Note: shouldn't need to do this but otherwise half of the quad sometimes isn't drawn (??)

		// 2) cover same rectangle again by drawing 2 different triangles starting at bot-left	
		glVertex4d(p1.x, p1.y, p1.z, 4);
		glVertex4d(p2.x, p2.y, p2.z, 1);
		glVertex4d(p2.x, p2.y, p2.z, 2);

		glVertex4d(p1.x, p1.y, p1.z, 4);
		glVertex4d(p2.x, p2.y, p2.z, 2);
		glVertex4d(p1.x, p1.y, p1.z, 3);

		glEnd();
		if (test3) {
			glPolygonMode(GL_FRONT, GL_FILL);
			glPolygonMode(GL_BACK, GL_FILL);
		}
#ifdef TRIANGLE_LINES
	} else if (width >= MIN_LINE_WIDTH) {
		if(/**/terminal &&(width*width_taper<MIN_LINE_WIDTH  || lev>maxlvl)){
			getRoot()->addTerminal(branch_id);
			//c=Color(1,0,0);
		}

		glColor4d(c.red(), c.green(), c.blue(), 1);

		//glColor4d(0.1,0.5,0,1);
		getRoot()->addLine(branch_id);
		glLineWidth(width);
		glBegin(GL_LINES);
		glVertex4d(p1.x, p1.y, p1.z, 0);
		glVertex4d(p2.x, p2.y, p2.z, 0);
		glEnd();
	}
#endif
#endif
#endif
	}
	width *= width_taper;
	size *= length_taper;
	for (int i = 0; i < splits; i++) {
		emit(0, bot, v, tip, size, width, lev);
	}
	if (right && right->typeValue() == ID_BRANCH) {
		TNBranch *child = (TNBranch*) right;
		child->fork(FIRST, bot, v, tip, size, width, lev);
	}
}

TNplant* TNBranch::getRoot() {
	NodeIF *p = getParent();
	while (p && p->typeValue() != ID_PLANT) {
		p=p->getParent();
	}
	if(p && p->typeValue() == ID_PLANT)
		return p;
	return 0;
}
void TNBranch::valueString(char *s){
	if(strlen(name_str)>0)
		sprintf(s+strlen(s),"%s(\"%s\",",symbol(),name_str);
	else
		sprintf(s+strlen(s),"%s(",symbol());
	TNbase::valueString(s);
}
void TNBranch::save(FILE *f){
	fprintf(f,"\n%s",tabs);
	if(strlen(name_str)>0)
		fprintf(f,"%s(\"%s\"",symbol(),name_str);
	else
		fprintf(f,"%s(",symbol());
	if(left){
		fprintf(f,",");
		left->save(f);
	}
	fprintf(f,")");
	if(base){
		fprintf(f,"[");
		base->save(f);
		fprintf(f,"]");
	}
	if(right)
		right->save(f);
}
void TNBranch::saveNode(FILE *f){
	TNbase::saveNode(f);
}
void TNBranch::eval(){
	if(right)
		right->eval();
}
