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

// BUGS/problems

//************************************************************
// classes PlantPoint, PlantMgr
//************************************************************

extern double Hscale, Drop, MaxSize,Height,Phi;
extern double ptable[];
extern Point MapPt;
extern double  zslope();
extern NameList<LongSym*> POpts;

extern void inc_tabs();
extern void dec_tabs();
extern char   tabs[];
extern int addtabs;

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
static PlantMgr *s_mgr=0; // static finalizer
static int hits=0;
//#define DEBUG_PMEM

#define USE_AVEHT
//#define SHOW
#define MIN_VISITS 2
#define TEST_NEIGHBORS 1
#define TEST_PTS 
//#define SHOW_STATS
//#define DUMP
#ifdef DUMP
static void show_stats()
{
	if(s_mgr)
		s_mgr->dump();
	cout<<"calls="<<ncalls<< " hits="<<nhits<<endl;
}
#endif

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
PlantMgr::PlantMgr(int i) : PlacementMgr(i)
{
#ifdef DUMP
	if(!s_mgr)
		add_finisher(show_stats);
#endif
	type|=SPRITES;
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
	TerrainProperties *tp=Td.tp;
	cout<<"PlantMgr::setProgram()"<<endl;
	
	char defs[1024]="";
	sprintf(defs+strlen(defs),"#define NLIGHTS %d\n",Lights.size);
	if(Render.haze())
		sprintf(defs+strlen(defs),"#define HAZE\n");

    bool do_shadows=Raster.shadows() && (Raster.twilight() || Raster.night());
	if(do_shadows && !TheScene->light_view()&& !TheScene->test_view() &&(Raster.farview()))
		sprintf(defs+strlen(defs),"#define SHADOWS\n");

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

	//glVertexAttrib4d(GLSLMgr::CommonID2, TheScene->vpoint.x,TheScene->vpoint.y,TheScene->vpoint.z,0); // Constants1
	GLSLMgr::setProgram();
	GLSLMgr::loadVars();
	
	for(int i=0;i<tp->plants.size;i++){
		TerrainProperties::sid=i;
		tp->plants[i]->setProgram();
	}
	
	int n=Plant::plants.size;
	int l=lastn;
	lastn=0;
	// TODO: 
	// 1. create a STABLE random offset for top of line (DONE)
	//    get unique noise value for each plant
	//     - use point projected on unit sphere vs point that includes ht
	// 2) tesselerize top bottom points into triangle fan vs lines (DONE)
	//    - convert 3d point to screen space in shader
	//    - offset x by +- point size
	// 3) use geometry shader vs CPU to generate triangles
	// 4) create a random 3-point spline vs simple line segments in CPU
	// 5) use geometry shader for above
	for(int i=n-1;i>=0;i--){
		PlantData *s=Plant::plants[i];
		int id=s->get_id();
		Point bot=s->base; // surface point
				
		double l=bot.length();
		double ht=l-TheMap->radius;
		
		Point top=bot*(1+s->radius);

		Point4D npt;
		lastn=0;
		Point pp=Point(s->point.x,s->point.y,s->point.x);
		
		double r=Random(pp);
		lastn=10000*r+id;
		
		glVertexAttrib4d(GLSLMgr::CommonID1, s->pntsize, 0,0, 0); // Constants1

		top.x+=2e-9*RAND(lastn++);
		top.y+=2e-9*RAND(lastn++);
		top.z+=2e-9*RAND(lastn++);
		
		top=top-TheScene->vpoint;
		bot=bot-TheScene->vpoint;
		
		glColor4d(URAND(lastn++),URAND(lastn++),URAND(lastn++),1);
		glBegin(GL_TRIANGLE_FAN);
		glVertex4d(top.x,top.y,top.z,0);
		glVertex4d(top.x,top.y,top.z,1);
		glVertex4d(bot.x,bot.y,bot.z,2);
		glVertex4d(bot.x,bot.y,bot.z,3);
	
		glEnd();
		
	}
	lastn=l;
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
PlantPoint::PlantPoint(PlantMgr&mgr, Point4DL&p,int n) : Placement(mgr,p,n)
{
	ht=0;
	aveht=0;
	wtsum=0;
	dist=1e16;
	visits=0;
	hits=0;
	mind=1e16;
	plants_dim=mgr.plants_dim;
	variability=mgr.mult;
	rand_flip_prob=mgr.rand_flip_prob;
	select_bias=mgr.select_bias;
	instance=mgr.instance;
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
	cout<<"TODO Plant::setProgram()"<<endl;
//	char str[MAXSTR];
//	
//	GLSLVarMgr vars;
//	GLhandleARB program=GLSLMgr::programHandle();
//	vars.setProgram(program);
//	vars.loadVars();

	return true;
}
bool Plant::initProgram(){
	return false;
}

//===================== TNplant ==============================
//************************************************************
// TNplant class
//************************************************************
TNplant::TNplant(char *s, TNode *l, TNode *r) : TNplacements(0,l,r,0)
{
	set_collapsed();
 	//cout<<"TNplant ID="<<get_id()<<endl;
	setName(s);
	FREE(s);
	plant=0;
    mgr=new PlantMgr(PLANTS|NOLOD);
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
	if(right)
    	right->init();

	PlantMgr *smgr=(PlantMgr*)mgr;

	if(plant==0)
		plant=new Plant(type,this);
	smgr->set_first(1);
	smgr->init();
	TNplacements::init();
}

void TNplant::set_id(int i){
	//instance=i;
	BIT_OFF(type,PID);
	type|=i&PID;
}
//-------------------------------------------------------------
// TNplant::eval() evaluate the node
//-------------------------------------------------------------
#define COLOR_TEST
#define DENSITY_TEST
void TNplant::eval()
{	
	SINIT;
	if(!isEnabled()){
		if(right)
			right->eval();
		return;
	}
	if(CurrentScope->rpass()){
		int size=Td.tp->plants.size;
		instance=size;
	
		mgr->instance=instance;

		if(plant)
			plant->set_id(size);
		
		Td.add_plant(plant);
		if(right)
			right->eval();
		return;
	}	
	if(!CurrentScope->spass()){
		if(right)
			right->eval();
		return;
	}
	
	Color c =Color(1,1,1);
	PlantMgr *smgr=(PlantMgr*)mgr;

	htval=Height;
	ncalls++;
	
	double arg[10];
	INIT;
	TNarg &args=*((TNarg *)left);
	TNode *dexpr;
	
	int n=getargs(&args,arg,9);
	
	double density=1;

	if(n>0) mgr->levels=(int)arg[0]; 	// scale levels
	if(n>1) mgr->maxsize=arg[1];     	// size of largest 
	if(n>2) mgr->mult=arg[2];			// random scale multiplier
	if(n>3) mgr->level_mult=arg[3];     // scale multiplier per level
	if(n>4) maxdensity=arg[4];
	if(n>5) smgr->slope_bias=arg[5];
	if(n>6) smgr->ht_bias=arg[6];
	if(n>7) smgr->lat_bias=arg[7];
	if(n>8) smgr->select_bias=arg[8];
	
	density=maxdensity;
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
		Td.density+=lerp(cval,0,0.2,0,.05*x);
#endif
	}
 }
//-------------------------------------------------------------
// TNinode::setName() set name
//-------------------------------------------------------------
void TNplant::setName(char *s)
{
	if(s)
		strcpy(plants_file,s);
	else
		plants_file[0]=0;
}

//-------------------------------------------------------------
// TNtexture::valueString() node value substring
//-------------------------------------------------------------
void TNplant::valueString(char *s)
{
	if(strlen(plants_file)>0)
		sprintf(s+strlen(s),"%s(\"%s\",",symbol(),plants_file);
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
// TNfunc::save() archive the node
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


