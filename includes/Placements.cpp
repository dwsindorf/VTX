

#include "TerrainMgr.h"
#include "Placements.h"
#include "AdaptOptions.h"
#include "MapNode.h"
#include "Util.h"
#include "SceneClass.h"
#include "RenderOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int place_gid=0;
//int vtests=0,pts_fails=0,dns_fails=0;


//************************************************************
// classes Placable, PlacementMgr
//************************************************************

#define DEBUG_PLACEMENTS   // turn on to get hash table hits
#define PLACEMENTS_LOD     // turn on to enable lod rejection
#define DEBUG_LOD          // turn on to get lod info
#define DEBUG_HASH       //  turn on to get hash table stats
 
static TerrainData Td;
extern double ptable[];
extern double Hscale,Height;

//double sval=0;
//double cval=0;
//double htval=0;
//int    hits=0;
int    misses;
//int    scnt=0;
int    visits=0;

//#define SDATA_SIZE 1024
//SData   sdata[SDATA_SIZE];
//ValueList<SData*> slist(sdata,SDATA_SIZE);

double MaxSize;

#ifdef DEBUG_PLACEMENTS

// hash lookup overhead
//     PERM(pc.x+PERM(pc.y+PERM(pc.z+PERM(n2+id)))
// ~0.5 us on 330 MHz sparc


static PlacementMgr s_mgr=0;

void show_display_placements()
{
	if(!Render.display(CRTRINFO))
		return;
	PlacementStats::exec();
}

#endif

static LongSym popts[]={ 
	{"MAXHT",		MAXHT},
	{"MAXA",		MAXA},
	{"MAXB",		MAXB},
	{"NOLOD",		NOLOD},
	{"CNORM",		CNORM},
	{"FLIP",		FLIP},
	{"NNBRS",		NNBRS},
	{"MC3D",		MC3D}
};
NameList<LongSym*> POpts(popts,sizeof(popts)/sizeof(LongSym));

int PlacementStats::chits=0;
int PlacementStats::cvisits=0;
int PlacementStats::crejects=0;
int PlacementStats::nhits=0;
int PlacementStats::nmisses=0;
int PlacementStats::nvisits=0;
int PlacementStats::nrejects=0;
int PlacementStats::cmade=0;
int PlacementStats::cfreed=0;
int PlacementStats::vtests=0;
int PlacementStats::pts_fails=0;
int PlacementStats::dns_fails=0;
int PlacementStats::place_visits=0;
int PlacementStats::place_hits=0;
int PlacementStats::place_misses=0;

void PlacementStats::exec(){
	if(!Render.display(CRTRINFO))
		return;
	TheScene->draw_string(HDR1_COLOR,"------- placements ---------------------");
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f deleted:%5.1f active:%5.1f K",
	    "created",cmade/1000.0,cfreed/1000.0,(cmade-cfreed)/1000.0);
	if(vtests)
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f K fails pts %2.1f %% dns %2.1f %%",
		"tests",(double)vtests/1000,100.0*pts_fails/vtests,100.0*dns_fails/vtests);
#ifdef DEBUG_LOD
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f K passed %2.1f %% failed %2.1f %%",
		"lod",(double)place_visits/1000,100.0*place_hits/place_visits,100.0*place_misses/place_visits);
#endif
#ifdef DEBUG_HASH
	if(cvisits)
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f K hits %2.1f %% rejects %2.1f %%",
		"chash",(double)cvisits/1000,100.0*chits/cvisits,100.0*crejects/cvisits);
	if(nvisits)
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f K hits %2.1f %% misses %2.1f %%",
		"nhash",(double)nvisits/1000,100.0*nhits/nvisits,100.0*nmisses/nvisits);
#endif
	TheScene->draw_string(HDR1_COLOR,"------------------------------------");
}
//************************************************************
// PlacementMgr class
//************************************************************
//	arg[0]  levels   		scale levels
//	arg[1]  maxsize			size of largest placements
//	arg[2]  mult			size multiplier per level
//	arg[3]  density [dexpr]	scatter density or expr
//-------------------------------------------------------------

SData  PlacementMgr::sdata[SDATA_SIZE];
ValueList<SData*> PlacementMgr::slist(sdata,SDATA_SIZE);
int PlacementMgr::scnt=0;
int PlacementMgr::hits=0;
double PlacementMgr::sval=0;
double PlacementMgr::cval=0;
double PlacementMgr::htval=0;

PlacementMgr::PlacementMgr(int i, int h)
{
	type=i&PID;
    options=i&(~PID);
	flags.l=0;
	instance=0;

	levels=5;
	maxsize=0.01;
	mult=0.8;
	level_mult=0.8;
	density=1.0;
  	dexpr=0;
  	base=0;

  	hash=0;
  	hashsize=h;
  	index=0;

  	//hits=0;
  	roff=0.5*PI;
  	roff2=1;
	render_ptsize=1;
	adapt_ptsize=1;

    set_first(0);
	set_finalizer(i&FINAL?1:0);
}

PlacementMgr::PlacementMgr(int i):PlacementMgr(i,PERMSIZE)
{	
}

PlacementMgr::~PlacementMgr()
{
	Placement *h;
	list.reset();

  	if(hash && finalizer()){    
#ifdef DEBUG_PMEM
  		printf("PlacementMgr::free()\n");
#endif
  		free_htable();
		FREE(hash);
	}
}

bool PlacementMgr::testColor()  		{ return options & COLOR_TEST?Render.color_test():false;}
bool PlacementMgr::testDensity()  		{ return options & DENSITY_TEST?true:false;}

//-------------------------------------------------------------
// PlacementMgr::free_htable() reset for eval pass
//-------------------------------------------------------------
void PlacementMgr::free_htable()
{
	Placement *h;
	Stats.cfreed=0;
	for(int i=0;i<hashsize;i++){
		h=hash[i];
		if(h){
#ifdef DEBUG_PLACEMENTS
			Stats.cfreed++;
#endif
			delete h;
			hash[i]=0;
		}
	}
	//cout<<"PlacementMgr::free_htable freed:"<<cfreed<<endl;
}

//-------------------------------------------------------------
// PlacementMgr::reset() reset for eval pass
//-------------------------------------------------------------
void PlacementMgr::reset()
{
	for(int i=0;i<hashsize;i++){
		Placement *h=hash[i];
		if(h){
			h->reset();
		}		
	}
	list.reset();
	cval=0;
	scnt=0;
}

//-------------------------------------------------------------
// PlacementMgr::init()	initialize hash tables
//-------------------------------------------------------------
void PlacementMgr::init()
{
	static bool finisher_added=false;
	cnt=0;
	if(hash==0){
#ifdef DEBUG_PMEM
  		printf("PlacementMgr::init()\n");
#endif
  		CALLOC(hashsize+1,Placement*,hash);
#ifdef DEBUG_PLACEMENTS
  		if(!finisher_added){
  			s_mgr=this;
			add_finisher(show_display_placements);
  		}
  		finisher_added=true;
 
#endif
	}
	reset();
}

void PlacementMgr::setTests() {
	if(!test() || hits==0)
		return;
	double x=1-cval;
	if(testColor()) {
		S0.set_cvalid();
		if(fabs(x)>0)
			S0.c=Color(1-x,0,1);
		else
			S0.c=Color(1,1,0);
		Td.diffuse=Td.diffuse.mix(S0.c,0.5);
	}
	if(testDensity()) {
		x=1/(cval+1e-6);
		x=x*x; //*x*x;
		Td.density+=lerp(cval,0,0.2,0,0.05*x);
	}
}

//-------------------------------------------------------------
// PlacementMgr::make()	virtual factory method to make Placement
//-------------------------------------------------------------
Placement *PlacementMgr::make(Point4DL &p, int n)
{
    return new Placement(*this,p,n);
}

void PlacementMgr::ss(){ 
	index=0;
	//hits=0;
}
//-------------------------------------------------------------
// PlacementMgr::make()	virtual factory method to make Placement
//-------------------------------------------------------------
Placement *PlacementMgr::next()
{
	if(!hash || index>=hashsize)
		return 0;
	Placement *h=hash[index];
	while(!h && index<hashsize-1){
		index++;
		h=hash[index];
	}
	index++;
    return h;
}
void PlacementMgr::dump(){
	if(!hash)
		return;
	int cnt=0;
	for(int i=0;i<hashsize;i++){
		Placement *h=hash[i];
		if(h){
			h->dump();
			cnt++;
		}		
	}
	cout<<"num placements="<<cnt<<endl;
	
}

bool PlacementMgr::valid()
{ 
	extern Point Mpt;
    //if(!sizetest())
    //	return true;
	double mps=render_ptsize;
	if(TheScene->adapt_mode())
		mps=adapt_ptsize;
	Point pv=Mpt;
	double d=pv.length();
	
	double r=TheMap->radius*size;
	double f=TheScene->wscale*r/d;
    double pts=f;
    Stats.vtests++;
    if(pts<mps){
     	Stats.pts_fails++;
       	return false;
    } 
    if(density<=0){
     	Stats.dns_fails++;
     	return false;
    }
	return true;
}
//-------------------------------------------------------------
// PlacementMgr::eval()	modulate terrain
//-------------------------------------------------------------
#define ROFF(x) 	  rands[(x)&PMASK]
#define SRAND(x) 	  rands[PERM((x)+seed)]
#define URAND(x)      (SRAND(x)+0.5)
#define LODSIZE       2*size
void PlacementMgr::eval()
{
	Point4D pc,p;
	Point4D pv=TheNoise.get_point();
	pv=pv-TheNoise.offset;  // reset origin to ~0

	if(TheNoise.noise3D())
		 pv.w=0;
	double l=pv.length();
	pv=pv.normalize();  // project on unit sphere
	
	sval=0;
	hits=0;
	cval=0;
	scnt=0;

	msize=maxsize;//ntest()?maxsize:maxsize*4;
	for(lvl=0,size=msize;lvl<levels;size*=0.5*(level_mult+1),lvl++){
		if(!valid())
			continue;

#ifdef PLACEMENTS_LOD
		if(!CurrentScope->init_mode()&&lod()&& Adapt.lod()){
		    double x=ptable[(int)Td.level];
#ifdef DEBUG_LOD
		    Stats.place_visits++;
		    if(x>LODSIZE){
		        Stats.place_hits+=levels-lvl;
		        return;
		    }
		    Stats.place_misses++;
#else
		    if(x>LODSIZE)
		        return;
#endif
	    }
#endif
		//  offset domain to mis-register overlap of successive levels

		int seed=lvl*13+id;
		    
        if(lvl>0&&roff>0){   
		    set_offset_valid(1);
			offset.x=roff*SRAND(1);
			offset.y=roff*SRAND(2);
			offset.z=roff*SRAND(3);
			if(TheNoise.noise4D())
				offset.w=roff*SRAND(4);
			else
			    offset.w=0;
			mpt=pv+offset;
			p=pv*(1.0/size)+offset;
		}
		else{
		    set_offset_valid(0);
			mpt=pv;		
			p=pv*(1.0/size);
		}
		
		Point4DL pc(FLOOR(p.x),
		            FLOOR(p.y),
		            FLOOR(p.z),
		            FLOOR(p.w));
		if(TheNoise.noise3D())
		    p.w=0;
		
		int n=PERM(pc.x+PERM(pc.y+PERM(pc.z+PERM(lvl+id))));
		//cout<<lvl<<" "<<id<<" "<<lvl+id<<" "<<n<<endl;

		if(TheNoise.noise4D())
			n=PERM(pc.w+n);

		Placement *h=hash[n];
		Stats.cvisits++;
		if(!h|| h->point!=pc || h->type !=type){
			Placement *c=make(pc,n);
			if(!c->flags.s.valid){
				delete c;
				Stats.crejects++;
				continue;
			}
			if(h){
				Stats.cfreed++;
				delete h;
			}
			h=c;
			hash[n]=h;
		}
		else
			Stats.chits++;
		if(h->radius>0.0)
		  	if(!h->set_terrain(*this))
				Stats.crejects++;
		else
			Stats.crejects++;

		if(ntest()){
		  	find_neighbors(h);
			list.ss();
			while((h=list++)){
				//if(!h->flags.s.valid)
				//	continue;
		  		if(!h->set_terrain(*this))
		  			Stats.nmisses++;

		  		h->users--;
				if(hash[h->hid]!=h){
				    hash[h->hid]=0;
					Stats.cfreed++;
					delete h;
				}
			}
			list.reset();
		}
	}

	if(!first() || !scnt)
	    return;
	for(int i=0;i<scnt;i++){
	    slist.base[i]=sdata+i;
	}
	slist.size=scnt;
	slist.sort();
	
	cval=slist.base[scnt-1]->f;

}

//-------------------------------------------------------------
// Placement::find_neighbors()	build neighbor list
//-------------------------------------------------------------
void PlacementMgr::find_neighbors(Placement *placement)
{
	int i,j,k,l=0,l1=0,l2=0,n;
	int n4d=TheNoise.noise4D();
	Placement *h;
	l=0;
	
	if(n4d){
	    l1=-1;
	    l2=1;
	}
	list.ss();
	placement->users++;
	for(i=-1;i<=1;i++){
		for(j=-1;j<=1;j++){
			for(k=-1;k<=1;k++){
				for(l=l1;l<=l2;l++){
					if(!i&&!j&&!k&&!l)
						continue;
	
					Point4DL pc(placement->point.x+i,
							    placement->point.y+j,
							    placement->point.z+k,
							    placement->point.w+l
							);

					n=PERM(pc.x+PERM(pc.y+PERM(pc.z+PERM(lvl+id))));
					if(n4d)
						n=PERM(pc.w+n);
							
#ifdef DEBUG_PLACEMENTS
					Stats.nvisits++;
#endif
					h=hash[n];
					if(h&&h->users){
#ifdef DEBUG_PLACEMENTS
						Stats.nmisses++;
#endif
						continue; // hash rehit (same id different nodes)
					}
					if(!h || h->point!=pc || h->type !=type){
						if(h){
#ifdef DEBUG_PLACEMENTS
							Stats.cfreed++;
#endif
							delete h;
						}
						h=make(pc,n);
						hash[n]=h;
					}
#ifdef DEBUG_PLACEMENTS
					else
						Stats.nhits++;
#endif
					if(h->radius>0.0){
						h->users++;
						list.add(h);
					}
#ifdef DEBUG_PLACEMENTS
					else
					    Stats.nrejects++;
#endif
				}
			}
		}
	}
	placement->users--;
}

//************************************************************
// class Placement
//************************************************************
Placement::Placement(PlacementMgr &pmgr,Point4DL &pt, int n) : point(pt)
{
	mgr=&pmgr;
    type=mgr->type;
	hid=n;
	double d,r,pf=1;
	radius=0;
	users=0;
	flags.l=0;
	ht=0;
	aveht=0;
	wtsum=0;
	dist=1e16;
	visits=0;
	//place_hits=0;
	instance=0;
	
#ifdef DEBUG_PLACEMENTS
	mgr->Stats.cmade++;
#endif
	double dns=mgr->density;
	Point4D	p(pt);

	int seed=PERM(hid);
	
    if(mgr->offset_valid())
		p=p-mgr->offset;

 	p=(p+0.5)*mgr->size;

	if(mgr->dexpr){
		Point4D p1=p*TheNoise.scale+TheNoise.offset;
	    SPUSH;
		TheNoise.push(p1);
		CurrentScope->revaluate();
		mgr->dexpr->eval();
		TheNoise.pop();
		dns+=S0.s;
		SPOP;
		CurrentScope->revaluate();
		dns=clamp(dns,0,1);
	}
	if(rands[hid]+0.5>dns)
		return;

	d=fabs(p.length()-1);
	double rf=1-mgr->mult;
	if(mgr->ntest()){
		if(d>0.8*mgr->size)
			return;
		r=0.5*mgr->size*(1-URAND(1)*rf);
		pf=2*mgr->size-r;
	}
	else{
		if(d>0.4*mgr->size)
			return;
		r=0.25*mgr->size*(1-URAND(1)*rf);
		pf=0.8*mgr->size-r;
	}
	pf*=mgr->roff2;
	if(pf>0){
		p.x+=pf*SRAND(2);
		p.y+=pf*SRAND(3);
		p.z+=pf*SRAND(4);
		if(TheNoise.noise4D())
			p.w+=pf*SRAND(5);
		else
			p.w=0;
	}
	p=p.normalize();
    if(mgr->offset_valid())
	   	p=p+mgr->offset;
	if(TheNoise.noise3D())
	    p.w=0;
	center=p;
	radius=r;

	flags.s.valid=true;
}

//-------------------------------------------------------------
// Placement::set_terrain()	impact terrain
//-------------------------------------------------------------
bool Placement::set_terrain(PlacementMgr &pmgr)
{
	double d=pmgr.mpt.distance(center);
	d=d/radius;

	PlacementMgr::sval=0;
	visits++;
	
	if(d>1.0)
		return false;
	if(!flags.s.valid)
		return false;

    flags.s.active=true;
    PlacementMgr::sval=lerp(d,0,1.0,0,1);

    double wt=1/(0.01+PlacementMgr::sval);
    aveht+=Height*wt;	
    wtsum+=wt;

	if(d<dist){
		ht=Height; // closest to center
		dist=d;
		place_hits++;
	}
	PlacementMgr::hits++;

	PlacementMgr::sdata[PlacementMgr::scnt].v=hid;
	PlacementMgr::sdata[PlacementMgr::scnt].f=PlacementMgr::sval;
  	if(PlacementMgr::scnt<SDATA_SIZE)
  		PlacementMgr::scnt++;
	return true;
}

void Placement::reset(){
	flags.s.active=0;
	visits=0;
	place_hits=0;
	dist=1e6;
	aveht=0;
	wtsum=0;
}
void Placement::dump(){
	if(flags.s.valid && flags.s.active){
		Point4D p(point);
		p=center;
		char msg[256];
		char vh[32];
		sprintf(vh,"%d:%d",visits,place_hits);
		sprintf(msg,"%-3d %-2d %-8s dist:%-0.4f ht:%-1.6f x:%-1.5f y:%-1.5f z:%1.5f",mgr->cnt++,flags.l,vh,dist,ht,p.x,p.y,p.z);
		cout<<msg<<endl;
	}
}

//void Placement::dump(){
//	// extended classes can override this
//}
//void Placement::reset(){
//	flags.l=0;
//}
//==================== PlantData ===============================
PlaceData::PlaceData(Placement *pnt,Point bp,double d, double ps){
	type=pnt->type;
	ht=pnt->ht;
	
	point=pnt->point;
	center=bp;
	
	aveht=pnt->aveht/pnt->wtsum;
	base=bp;
	
	radius=pnt->radius;
    pntsize=ps;
 	distance=d;//TheScene->vpoint.distance(t);
	visits=pnt->visits;
	instance=pnt->instance;
	mgr=pnt->mgr;
}

void PlaceData::print(){
	char msg[256];
	Point pp=Point(point.x,point.y,point.z);
	double h=TheMap->radius*TheMap->hscale;
	sprintf(msg,"visits:%-1d ht:%-1.4f aveht:%-1.4f dist:%g",visits,h*ht/FEET,h*aveht/FEET,distance/FEET);
	cout<<msg<<endl;
	
}

//************************************************************
// TNplacements class
//************************************************************
TNplacements::TNplacements(int t, TNode *l, TNode *r, TNode *b) : TNbase(t,l,r,b) 
{
	int id=type&PID;
 	if(id==0){
		id=place_gid++;
		id=id&PID;
		BIT_OFF(type,PID);
		BIT_ON(type,id);
	}
	token[0]=0;
    mgr=0;
}

TNplacements::~TNplacements()
{
    DFREE(mgr);
}

void TNplacements::set_id(int i)
{
	BIT_OFF(type,PID);
	type|=i&PID;
	if(mgr)
		mgr->set_id(type);
}

int TNplacements::get_id()
{
	return type&PID;
}

//-------------------------------------------------------------
// TNplacements::reset()	reset hash table
//-------------------------------------------------------------
void TNplacements::reset()
{
	mgr->free_htable();
}

//-------------------------------------------------------------
// TNplacements::valueString()	set value string
//-------------------------------------------------------------
void TNplacements::valueString(char *s)
{
	if(CurrentScope->tokens() && token[0]){
		setStart(s);
        strcat(s,token);
		setEnd(s);
		if(right)
			right->valueString(s);
	}
	else
		TNbase::valueString(s);
}
//-------------------------------------------------------------
// TNplacements::addToken() add to token list
//-------------------------------------------------------------
void TNplacements::addToken(LinkedList<TNode*>&l) 
{
	l.add(this);
	if(right)
		right->addToken(l);
}

//-------------------------------------------------------------
// TNplacements::save() save the node
//-------------------------------------------------------------
void TNplacements::save(FILE *f)
{
	TNbase::save(f);
}

//-------------------------------------------------------------
// TNplacements::optionString() get option string
//-------------------------------------------------------------
int TNplacements::optionString(char *c)
{
    c[0]=0;
	if(type&PID)
		sprintf(c,"ID%d",type&PID);
	for(int i=0;i<POpts.size;i++){
    	if(type & POpts[i]->value){
			if(c[0]>0)
				strcat(c,"|");
   			strcat(c,POpts[i]->name());
   		}
	}
	return c[0];  
}

//-------------------------------------------------------------
// TNplacements::setProperties() set argument
//-------------------------------------------------------------
void TNplacements::setProperties(NodeIF *n)	
{
	TNfunc::setProperties(n);
}

//-------------------------------------------------------------
// TNplacements::init() initialize the node
//-------------------------------------------------------------
void TNplacements::init()
{
	TNbinary::init();
	if(base)
	    base->init();
}

//-------------------------------------------------------------
// TNplacements::eval() evaluate common arguments
//-------------------------------------------------------------
void TNplacements::eval()
{
	double arg[4];
	INIT;
	TNarg &args=*((TNarg *)left);
	TNode *dexpr;
	
	int n=getargs(&args,arg,3);

	if(n>0) mgr->levels=(int)arg[0]; 	// scale levels
	if(n>1) mgr->maxsize=arg[1];     	// size of largest craters
	if(n>2){
		double randscale=arg[2];		// random scale multiplier
		mgr->mult=randscale;            // in same level
		mgr->level_mult=randscale;      // reduce size per level
	}
	//if(mgr->dexpr==0){
	// probability
		dexpr=args[3];
		if(dexpr){
		    dexpr->eval();
		    S0.s=clamp(S0.s,0,1);
		    mgr->density=S0.s;
		    //cout<<mgr->density<<endl;
		}
	//}
	
	MaxSize=mgr->maxsize;

	double hashcode=(mgr->levels+
		            1/mgr->maxsize+
					1/mgr->mult
					);
	mgr->id=(int)hashcode+mgr->type+mgr->instance+hashcode*TheNoise.rseed;
}
