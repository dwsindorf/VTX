

#include "TerrainMgr.h"
#include "Placements.h"
#include "AdaptOptions.h"
#include "Util.h"
#include "SceneClass.h"
#include "RenderOptions.h"
#include "Util.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int place_visits=0;
int place_hits=0;
int place_misses=0;
int place_gid=0;
extern int hits,visits;

//************************************************************
// classes Placable, PlacementMgr
//************************************************************

#define DEBUG_PLACEMENTS   // turn on to get hash table hits
#define PLACEMENTS_LOD     // turn on to enable lod rejection
#define DEBUG_LOD          // turn on to get lod info
//#define USE_HASHTABLE
 
static TerrainData Td;
extern double ptable[];
extern double Hscale;

double MaxSize;

#ifdef DEBUG_PLACEMENTS

// hash lookup overhead
//     PERM(pc.x+PERM(pc.y+PERM(pc.z+PERM(n2+id)))
// ~0.5 us on 330 MHz sparc

static int	chits=0,cvisits=0,crejects=0;
static int	nhits=0,nmisses=0,nvisits=0,nrejects=0;
static int  cmade=0,cfreed=0;

static PlacementMgr s_mgr=0;

void init_display_placements()
{
}

void show_display_placements()
{
	if(!Render.display(CRTRINFO))
		return;
	TheScene->draw_string(HDR1_COLOR,"------- placements ---------------------");
	TheScene->draw_string(DATA_COLOR,"created:%5.1f deleted:%5.1f active:%5.1f K",
	    cmade/1000.0,cfreed/1000.0,(cmade-cfreed)/1000.0);
	if(cvisits)
	TheScene->draw_string(DATA_COLOR,"chash visits: %5.1f K hits %2.1f %% rejects %2.1f %%",
		(double)cvisits/1000,100.0*chits/cvisits,100.0*crejects/cvisits);
	if(nvisits)
	TheScene->draw_string(DATA_COLOR,"nhash visits: %5.1f K hits %2.1f %% rejects %2.1f %%",
		(double)nvisits/1000,100.0*nhits/nvisits,100.0*nrejects/nvisits);
	TheScene->draw_string(HDR1_COLOR,"------------------------------------");
}

#endif

static LongSym popts[]={ 
	{"MAXHT",		MAXHT},
	{"MAXA",		MAXA},
	{"MAXB",		MAXB},
	{"NOLOD",		NOLOD},
	{"CNORM",		CNORM},
	{"FLIP",		FLIP},
	{"NNBRS",		NNBRS}
};
NameList<LongSym*> POpts(popts,sizeof(popts)/sizeof(LongSym));

//************************************************************
// PlacementMgr class
//************************************************************
//	arg[0]  levels   		scale levels
//	arg[1]  maxsize			size of largest placements
//	arg[2]  mult			size multiplier per level
//	arg[3]  density [dexpr]	scatter density or expr
//-------------------------------------------------------------
Placement **PlacementMgr::hash=0;
int PlacementMgr::index=0;
int PlacementMgr::hits=0;
int PlacementMgr::hashsize=PERMSIZE;
LinkedList<Placement*> PlacementMgr::list;

PlacementMgr::PlacementMgr(int i)
{
	type=i&PID;
    options=i&(~PID);
	flags.l=0;
	instance=0;

	levels=5;
	maxsize=0.01;
	mult=0.8;
	level_mult=0.8;
	density=0.8;    			
  	dexpr=0;
  	base=0;
  	//hash=0;
  	hits=0;
  	roff=0.5*PI;
  	roff2=1;

    set_first(0);
	set_finalizer(i&FINAL?1:0);
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

//-------------------------------------------------------------
// PlacementMgr::free_htable() reset for eval pass
//-------------------------------------------------------------
void PlacementMgr::free_htable()
{
	Placement *h;
	cfreed=0;
	for(int i=0;i<hashsize;i++){
		h=hash[i];
		if(h){
#ifdef DEBUG_PLACEMENTS
			cfreed++;
#endif
			delete h;
			hash[i]=0;
		}
	}
	cout<<"PlacementMgr::free_htable freed:"<<cfreed<<endl;
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
	chits=cvisits=crejects=0;
	nhits=nmisses=nvisits=nrejects=0;
	cmade=cfreed=0;
}

//-------------------------------------------------------------
// PlacementMgr::init()	initialize hash tables
//-------------------------------------------------------------
void PlacementMgr::init()
{
	if(hash==0){
#ifdef DEBUG_PMEM
  		printf("PlacementMgr::init()\n");
#endif
  		CALLOC(hashsize+1,Placement*,hash);
#ifdef DEBUG_PLACEMENTS
		add_initializer(init_display_placements);
		add_finisher(show_display_placements);
#endif
	}
	reset();
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
	hits=0;
}
void PlacementMgr::end(){ 
	cout<<"placement tests:"<<index<<" hits:"<<hits;
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
	hits++;
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

	msize=maxsize;//ntest()?maxsize:maxsize*4;
	for(lvl=0,size=msize;lvl<levels;size*=0.5*(level_mult+1),lvl++){
		if(!valid())
			continue;

#ifdef PLACEMENTS_LOD
		if(!CurrentScope->init_mode()&&lod()&& Adapt.lod()){
		    double x=ptable[(int)Td.level];
#ifdef DEBUG_LOD
		    place_visits++;
		    if(x>LODSIZE){
		        place_hits+=levels-lvl;
		        return;
		    }
		    place_misses++;
#else
		    if(x>LODSIZE)
		        return;
#endif
	    }
#endif
		//  offset domain to mis-register overlap of successive levels

		int seed=lvl*131+id;
		    
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
		if(TheNoise.noise4D())
			n=PERM(pc.w+n);

		Placement *h=hash[n];
		cvisits++;
		if(!h|| h->point!=pc || h->type !=type){
			Placement *c=make(pc,n);
			if(h){
				cfreed++;
				delete h;
			}
			h=c;
			hash[n]=h;
		}
		else
			chits++;
		if(h->radius>0.0)
		  	h->set_terrain(*this);
		else
			crejects++;
		if(ntest()){
		  	find_neighbors(h);
			list.ss();
			while((h=list++)){
		  		h->set_terrain(*this);
		  		h->users--;
				if(hash[h->hid]!=h){
				    hash[h->hid]=0;
					cfreed++;
					delete h;
				}
			}
			list.reset();
		}
	}
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
					nvisits++;
#endif
					h=hash[n];
					if(h&&h->users){
#ifdef DEBUG_PLACEMENTS
						nmisses++;
#endif
						continue; // hash rehit (same id different nodes)
					}
					if(!h || h->point!=pc || h->type !=type){
						if(h){
#ifdef DEBUG_PLACEMENTS
							cfreed++;
#endif
							delete h;
						}
						h=make(pc,n);
						hash[n]=h;
					}
#ifdef DEBUG_PLACEMENTS
					else
						nhits++;
#endif
					if(h->radius>0.0){
						h->users++;
						list.add(h);
					}
#ifdef DEBUG_PLACEMENTS
					else
					    nrejects++;
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
Placement::Placement(PlacementMgr &mgr,Point4DL &pt, int n) : point(pt)
{
    type=mgr.type;
	hid=n;
	double d,r,pf=1;
	radius=0.0;
	users=0;
	flags.l=0;
#ifdef DEBUG_PLACEMENTS
	cmade++;
#endif
    if(!mgr.dexpr && mgr.density<1){
	    if(rands[hid]+0.5>mgr.density)
		    return;
	}

	Point4D	p(pt);

	int seed=PERM(hid);
	
    if(mgr.offset_valid())
		p=p-mgr.offset;

	p=(p+0.5)*mgr.size;

	if(mgr.dexpr){
		Point4D p1=p*TheNoise.scale+TheNoise.offset;
	    SPUSH;
		TheNoise.push(p1);
		CurrentScope->revaluate();
		mgr.dexpr->eval();
		TheNoise.pop();
		double f=S0.s;
		SPOP;
		CurrentScope->revaluate();
		if(rands[hid]+0.5>f)
		    return;
	}		

	d=fabs(p.length()-1);
	double rf=1-mgr.mult;
	if(mgr.ntest()){
		if(d>0.8*mgr.size)
			return;
		r=0.5*mgr.size*(1-URAND(1)*rf);
		pf=2*mgr.size-r;
	}
	else{
		//if(d>0.4*mgr.size)
		//	return;
		r=0.25*mgr.size*(1-URAND(1)*rf);
		pf=0.8*mgr.size-r;
	}
	pf*=mgr.roff2;
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
    if(mgr.offset_valid())
	   	p=p+mgr.offset;
	if(TheNoise.noise3D())
	    p.w=0;
	center=p;
	radius=r;
	flags.s.valid=true;
}

//-------------------------------------------------------------
// Placement::set_terrain()	impact terrain
//-------------------------------------------------------------
bool Placement::set_terrain(PlacementMgr &mgr)
{
    // extended classes must override this
	return false;
}

void Placement::dump(){
	// extended classes can override this
}
void Placement::reset(){
	flags.l=0;
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
// TNplacements::eval() evaluate commom arguments
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
	
	if(mgr->dexpr==0){
		dexpr=args[3];
		if(dexpr){
		    dexpr->eval();
		    mgr->density=S0.s;
		}
	}
	
	MaxSize=mgr->maxsize;

	double hashcode=(mgr->levels+
		            1/mgr->maxsize+
					1/mgr->mult
					);
	mgr->id=(int)hashcode+mgr->type+mgr->instance+hashcode*TheNoise.rseed;
}
