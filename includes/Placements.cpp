

#include "TerrainMgr.h"
#include "Placements.h"
#include "AdaptOptions.h"
#include "MapNode.h"
#include "Util.h"
#include "SceneClass.h"
#include "RenderOptions.h"
#include "TerrainClass.h"
#include <map>
#include <unordered_set>
#include <unordered_map>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//************************************************************
// classes Placable, PlacementMgr
//************************************************************
#define HASHSIZE PERMSIZE

#define DEBUG_PLACEMENTS   // turn on to get hash table hits
#define PLACEMENTS_LOD     // turn on to enable lod rejection
#define DEBUG_LOD        // turn on to get lod info
//#define DEBUG_HASH         //  turn on to get hash table stats
//#define NO_CHAIN          // TEST mode turn off chaining
//#define DEBUG_HASH_CHAINS  //  turn on to get hash table chain stats
#define PRINT_PLACEMENT_STATS
//#define DEBUG_DENSITY

static TerrainData Td;
extern double ptable[];
extern double Hscale,Height,Slope;
extern Point MapPt;

extern bool UseDepthBuffer;

double MaxSize;

int place_gid=0;
static int 	cnt;

//#ifdef DEBUG_PLACEMENTS

// hash lookup overhead
//     PERM(pc.x+PERM(pc.y+PERM(pc.z+PERM(n2+id)))
// ~0.5 us on 330 MHz sparc

void show_display_placements()
{
#ifdef PRINT_PLACEMENT_STATS
	PlacementStats::dump();
#endif
#ifdef DEBUG_HASH_CHAINS
		PlacementMgr::printChainStats();
#endif

	if(!Render.display(CRTRINFO))
		return;
	PlacementStats::exec();
}

//#endif

static LongSym popts[]={ 
	{"MAXHT",		MXHT},
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
int PlacementStats::cchained=0;
int PlacementStats::nhits=0;
int PlacementStats::nmisses=0;
int PlacementStats::nvisits=0;
int PlacementStats::nrejects=0;
int PlacementStats::nchained=0;
int PlacementStats::cmade=0;
int PlacementStats::cfreed=0;
int PlacementStats::vtests=0;
int PlacementStats::pts_fails=0;
int PlacementStats::dns_fails=0;
int PlacementStats::dns_pass=0;
int PlacementStats::lod_visits=0;
int PlacementStats::lod_hits=0;
int PlacementStats::lod_fails=0;

std::unordered_set<int> types;

void PlacementStats::exec(){
	if(!Render.display(CRTRINFO))
		return;
	TheScene->draw_string(HDR1_COLOR,"------- placements ---------------------");
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f deleted:%5.1f active:%5.1f K",
	    "created",cmade/1000.0,cfreed/1000.0,(cmade-cfreed)/1000.0);
	if(vtests)
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f K fails pts %2.1f %% dns %2.1f %%",
		"tests",(double)vtests/1000,100.0*pts_fails/vtests,100.0*dns_fails/cmade);
#ifdef PLACEMENTS_LOD
#ifdef DEBUG_LOD
	if(lod_visits>0)
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f K passed %2.1f %% failed %2.1f %%",
		"lod",(double)lod_visits/1000,100.0*lod_hits/lod_visits,100.0*lod_fails/lod_visits);
#endif
#endif
#ifdef DEBUG_HASH
	if(cvisits)
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f K chained %2.1f K hits %2.1f %% rejects %2.1f %%",
		"chash",(double)cvisits/1000,(double)cchained/1000,100.0*chits/cvisits,100.0*crejects/cvisits);
	if(nvisits)
	TheScene->draw_string(DATA_COLOR,"%10s %5.1f K chained %5.1f K hits %2.1f %% misses %2.1f %%",
		"nhash",(double)nvisits/1000,(double)nchained/1000,100.0*nhits/nvisits,100.0*nmisses/nvisits);
#endif
	TheScene->draw_string(HDR1_COLOR,"------------------------------------");
}

void PlacementStats::dump(){
	if(!cmade)
		return;
		
	printf("------- placements ---------------------\n");
	printf("%10s %5.1f deleted:%5.1f active:%5.1f K types:%d\n",
	    "created",cmade/1000.0,cfreed/1000.0,(cmade-cfreed)/1000.0,types.size());
	if(vtests)
	printf("%10s %5.1f K fails pts %2.1f %% dns %2.1f %%\n",
		"tests",(double)vtests/1000,100.0*pts_fails/vtests,100.0*dns_fails/cmade);
#ifdef PLACEMENTS_LOD
#ifdef DEBUG_LOD
	if(lod_visits)
	printf("%10s %5.1f K passed %2.1f %% failed %2.1f %%\n",
		"lod",(double)lod_visits/1000,100.0*lod_hits/lod_visits,100.0*lod_fails/lod_visits);
#endif
#endif
#ifdef DEBUG_HASH
	if(cvisits)
	printf("%10s %5.1f K chained %2.1f K (%2.1f %%) hits %2.1f %% rejects %2.1f %%\n",
		"chash",(double)cvisits/1000,(double)cchained/1000,100.0*cchained/cvisits,100.0*chits/cvisits,100.0*crejects/cvisits);
	if(nvisits)
	printf("%10s %5.1f K chained %5.1f K (%2.1f %%) hits %2.1f %% misses %2.1f %%\n",
		"nhash",(double)nvisits/1000,(double)nchained/1000,100.0*nchained/nvisits,100.0*nhits/nvisits,100.0*nmisses/nvisits);
#endif
	printf("------------------------------------");
	cout<<endl;
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
int PlacementMgr::slvl=0;

double PlacementMgr::sval=0;
double PlacementMgr::cval=0;
double PlacementMgr::htval=0;
Point4D	PlacementMgr::mpt;
Point4D	PlacementMgr::offset;
double PlacementMgr::size=0.0;
int PlacementMgr::lvl=0;
LinkedList<Placement*> PlacementMgr::list;
PlacementStats PlacementMgr::Stats;
int PlacementMgr::trys=0;
int PlacementMgr::visits=0;
int PlacementMgr::bad_visits=0;
int PlacementMgr::bad_valid=0;
int PlacementMgr::bad_active=0;
int PlacementMgr::new_placements=0;
int PlacementMgr::bad_pts=0;
double PlacementMgr::roff=1e-6;
double PlacementMgr::roff2=1.0;
int PlacementMgr::hashsize=HASHSIZE;
double PlacementMgr::render_ptsize=1;
double PlacementMgr::adapt_ptsize=2;
double PlacementMgr::collect_minpts=2;
int PlacementMgr::min_hits=2;

Placement* PlacementMgr::currentChain=nullptr;  // ⭐ Add this member variable
int PlacementMgr::index=0;
Placement  **PlacementMgr::hash=0;

PlacementMgr::PlacementMgr(int i)
{
	type=i&PID;
    options=i&(~PID);
	flags.l=0;
	instance=0;
	layer=0;

	levels=5;
	maxsize=0.01;
	mult=0.8;
	level_mult=0.8;
	density=1.0;
  	dexpr=0;
  	base=0;
  	comp=0;
  	drop=0;
	slope_bias=ht_bias=lat_bias=hardness_bias=0;
	maxdensity=1.0;
   	index=0; 	
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

bool PlacementMgr::testColor()  		{ return options & COLOR_TEST?Render.color_test():false;}
bool PlacementMgr::testDensity()  		{ 
	extern bool UseDepthBuffer;
	if(UseDepthBuffer)
		return false;
	return options & DENSITY_TEST?true:false;
}

void PlacementMgr::setHashcode(){
	double hashcode=(levels+
		            1/maxsize+
					1/mult
					);
	//id=(int)hashcode+type+instance+hashcode*TheNoise.rseed;
	id=(int)hashcode+type+hashcode*TheNoise.rseed;

}
//-------------------------------------------------------------
// PlacementMgr::free_htable() reset for eval pass
//-------------------------------------------------------------
void PlacementMgr::free_htable()
{
    Stats.cfreed=0;
    if(!hash) 
    	return;
    //cout<<"PlacementMgr::free_htable"<<endl;
    for(int i=0; i<hashsize; i++){
        Placement* h = hash[i];
        while(h){  // ⭐ Loop through chain
            Placement* next = h->next;
            delete h;
#ifdef DEBUG_HASH
            Stats.cfreed++;
#endif
            h = next;
        }
        hash[i] = 0;
    }
    resetAll();
}

//-------------------------------------------------------------
// PlacementMgr::reset() reset for eval pass
//-------------------------------------------------------------
void PlacementMgr::reset()
{
	index=0;
    if(!hash) 
    	return;
    for(int i=0; i<hashsize; i++){
        Placement* h = hash[i];
        while(h){  // ⭐ Loop through chain
            h->reset();
            h = h->next;
        }
    }
    list.reset();
    cval=0;
    scnt=0;
    slvl=0;
    types.clear();
}

//-------------------------------------------------------------
// PlacementMgr::next()	retrieve placement from the hash table
//-------------------------------------------------------------
Placement *PlacementMgr::next()
{
 	return next(0);
}
Placement *PlacementMgr::next(int type)
{
   if(!hash)
		return 0;
	
	// If we're in the middle of a chain, continue it
	if(currentChain && currentChain->next){
		currentChain = currentChain->next;
		return currentChain;
	}
	
	// Need to find next hash slot with a chain
	// Start from current index
	while(index < hashsize){
		Placement* h = hash[index];
		if(h && (h->type==type || type==0)){
			currentChain = h;
			index++;  // Advance for next call
			return h;
		}
		index++;
	}
	
	// Reached end
	currentChain = nullptr;
	return nullptr;
}

void PlacementMgr::resetAll()
{
	 trys=visits=bad_visits=bad_valid=bad_active=bad_pts=new_placements=0;
}
void PlacementMgr::printChainStats()
{
    if(!hash)
        return;
    
    int totalPlacements = 0;
    int occupiedBuckets = 0;
    int emptyBuckets = 0;
    int maxChainLength = 0;
    long long totalChainLength = 0;
      
    // Histogram: count buckets by chain length
    std::map<int, int> histogram;
    
    for(int i = 0; i < hashsize; i++){
        int chainLen = 0;
        Placement* h = hash[i];
           
        while(h){
            chainLen++;
            totalPlacements++;
            h = h->next;
        }
        
        if(chainLen == 0){
            emptyBuckets++;
        } else {
            occupiedBuckets++;
            totalChainLength += chainLen;
            if(chainLen > maxChainLength)
                maxChainLength = chainLen;
            histogram[chainLen]++;
        }
    }
    
    printf("\n=== Hash Table Chain Statistics ===\n");
    printf("Hash table size:    %d buckets\n", hashsize);
    printf("Total placements:   %d\n", totalPlacements);
    printf("Occupied buckets:   %d (%.1f%%)\n", 
           occupiedBuckets, 100.0*occupiedBuckets/hashsize);
    printf("Empty buckets:      %d (%.1f%%)\n", 
           emptyBuckets, 100.0*emptyBuckets/hashsize);
    printf("Max chain length:   %d\n", maxChainLength);
    printf("Avg chain length:   %.2f (in occupied buckets)\n",
           occupiedBuckets ? (double)totalChainLength/occupiedBuckets : 0);
    printf("Load factor:        %.4f\n", (double)totalPlacements/hashsize);
    
    printf("\nChain length distribution:\n");
    for(auto& pair : histogram){
        int length = pair.first;
        int count = pair.second;
        printf("  Length %3d: %6d buckets (%.2f%% of occupied)\n",
               length, count, 100.0*count/occupiedBuckets);
        if(length > 20) break;  // Don't spam if there are very long chains
    }
    
    // Show a few examples of long chains
    if(maxChainLength > 10){
        printf("\nExample long chains:\n");
        int shown = 0;
        for(int i = 0; i < hashsize && shown < 3; i++){
            int chainLen = 0;
            Placement* h = hash[i];
            while(h){
                chainLen++;
                h = h->next;
            }
            
            if(chainLen > 10){
                printf("  Bucket %d (length %d):\n", i, chainLen);
                h = hash[i];
                int count = 0;
                while(h && count < 5){
                    printf("    lvl=%d point=(%d,%d,%d,%d) type=0x%x\n",
                           h->lvl, h->point.x, h->point.y, h->point.z, h->point.w, h->type);
                    h = h->next;
                    count++;
                }
                if(chainLen > 5)
                    printf("    ... (%d more)\n", chainLen - 5);
                shown++;
            }
        }
    }
    printf("======================================");
    cout<<endl;
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
 			add_finisher(show_display_placements);
   		}
  		finisher_added=true;
 
#endif
	}
	reset();
}

void PlacementMgr::setTests() {
	if(!test() || hits<min_hits)
		return;	
	extern Color getColor(int i);
	double x=fabs(1-cval);
	S0.clr_flag(DVALUE);
	if(x<0.25)
		return;
	x=lerp(x,0.25,1,0,1);
	double y=pow(x,2);
	
	if(testColor()) {
		S0.set_cvalid();
		int hash=(3*instance+type+layer)&0xf;
		double lmod=(1.0*slvl)/levels;
		Color c1=getColor(hash);
		c1=c1.darken(lmod);
		Color c3=getColor(hash+1);
		c3=c3.lighten(lmod);
		S0.c=c1.mix(c3,y);
	}
	if(testDensity()) {
		S0.set_flag(DVALUE);
		S0.s=x;
	}
}

//-------------------------------------------------------------
// PlacementMgr::make()	virtual factory method to make Placement
//-------------------------------------------------------------
Placement *PlacementMgr::make(Point4DL &p, int n)
{
    return new Placement(*this,p,n);
}

//-------------------------------------------------------------
// PlacementMgr::collect() collect list of valid placements 
//-------------------------------------------------------------
void PlacementMgr::collect(ValueList<PlaceData*> &data){
	resetIterator();
	Placement *s=next(); // get any type
	while (s) {  // from hash table
		if(s->type !=type){
			s = next(type);
			continue;
		}
		if (showstats()) {
			trys++;
			if (s->visits < minv)
				bad_visits++;
			if (!s->flags.s.valid)
				bad_valid++;
			if (!s->flags.s.active)
				bad_active++;
		}
		bool valid = s->visits >= 1 && s->flags.s.valid && s->flags.s.active;
		if (valid) {
			s->setVertex();
			double dist=s->dist;
			double pts=s->pts;
			if (testpts()) {  // reject small placements
				if (pts < collect_minpts) {
					bad_pts++;
					s = next(type);
					continue;
				}
			}
			if (s->visits >0) { // all good, generate a new final object (copy constructor ?)
				PlaceData *p1 = make(s); // may be overiden by extended classes
				data.add(p1);
			}
		}
		s = next(type);
	}	
	if(showstats() && trys>0){
		double usage=100.0*trys/hashsize;
		double badvis=100.0*bad_visits/trys;
		double badactive=100.0*bad_active/trys;
		double badpts=100.0*bad_pts/trys;
		cout<<"collected "<<new_placements<<" tests:"<<trys<<" %hash:"<<usage<<" %inactive:"<<badactive<<" %small:"<<badpts<<" %visited:"<<100-badvis<<endl;
	}
	//cout<<"placement collected="<<data.size<<endl;
}

PlaceData *PlacementMgr::make(Placement*s){
	return new PlaceData(s);
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
	cout<<"hash placements="<<cnt<<endl;	
}

//-------------------------------------------------------------
// PlacementMgr::valid() early reject placement tests
//-------------------------------------------------------------
bool PlacementMgr::valid()
{ 
	if(testpts()){	// cull small placements	
		double mps=render_ptsize;
		if(TheScene->adapt_mode())
			mps=adapt_ptsize;
		Point pv=MapPt;
		double d=pv.length();		
		double r=TheMap->radius*size;
		double pts=TheScene->wscale*r/d;
		Stats.vtests++;
		if(pts<mps){
			Stats.pts_fails++;
			return false;
		} 
	}
    if(density<=0){  // cull density rejects
     	Stats.dns_fails++;
     	return false;
    }
	return true;
}

int PlacementMgr::hashPoint(Point4DL& pc, int lvl, int id) {
    // Use large primes to mix the bits
    uint64_t h = (uint64_t)pc.x * 73856093ULL;
    h ^= (uint64_t)pc.y * 19349663ULL;
    h ^= (uint64_t)pc.z * 83492791ULL;
    h ^= (uint64_t)pc.w * 42841739ULL;
    h ^= (uint64_t)lvl * 999983ULL;
    h ^= (uint64_t)id * 314159ULL;
    
    return (int)(h % hashsize);
}
//-------------------------------------------------------------
// PlacementMgr::eval()	modulate terrain
//-------------------------------------------------------------
#define ROFF(x) 	  rands[(x)&PMASK]
#define SRAND(x) 	  rands[PERM((x)+seed)]
#define URAND(x)      (SRAND(x)+0.5)
#define LODSIZE       10*size
void PlacementMgr::eval()
{
    Point4D p;
    Point4D pv=TheNoise.get_point();
    pv=pv-TheNoise.offset;

    if(TheNoise.noise3D())
         pv.w=0;
    double l=pv.length();
    pv=pv.normalize();
     
    sval=0;
    hits=0;
    cval=0;
    scnt=0;
    slvl=0;

    msize=maxsize;
     for(lvl=0, size=msize; lvl<levels; size*=0.5*(mult+1), lvl++){
        if(!valid())
            continue;
         Stats.cvisits++;

#ifdef PLACEMENTS_LOD
        if(!Td.level>0 &&CurrentScope->init_mode() && Adapt.lod()/*&&lod()*/){
            double x=ptable[(int)Td.level];
            double lf=size/x;
#ifdef DEBUG_LOD
            Stats.lod_visits++;
            if(lf>0.5){ // size is ok 
                Stats.lod_hits++;
            }
            else{  // size is too small exit
            	Stats.lod_fails++;
            	return;
            }
#else
            if(lf<0.5)
                return;
#endif
        }
#endif

        int seed=lvl*13+id;
            
        if(lvl>0 && roff>0){   
           // set_offset_valid(1);
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
            mpt=pv;     
            p=pv*(1.0/size);
            offset.clear();
        }
        
        Point4DL pc(FLOOR(p.x),
                    FLOOR(p.y),
                    FLOOR(p.z),
                    FLOOR(p.w));
        if(TheNoise.noise3D())
            p.w=0;

        int n=hashPoint(pc,lvl,id);     
 
        // ⭐ MODIFIED: Search chain for matching placement
        Placement* h = hash[n];
        Placement* found = nullptr;
                
        // Search chain for existing placement with same point
        while(h){
           	if(h->type)
            	types.insert(h->type).second;
           	if(h->isEqual(pc,type,lvl,instance,layer)){
                found = h;
                Stats.chits++;
                break;
            } 
#ifdef NO_CHAIN
            break;
#endif 
            h = h->next;
            if(h)
                Stats.cchained++;
        }
        
        // If not found, create new placement
        if(!found){
            Placement* c = make(pc, n);
            if(!c->flags.s.valid){
                delete c;
                Stats.crejects++;
                continue;
            }
            else{
				// ⭐ Add to front of chain (no deletion!)
				c->next = hash[n];
				hash[n] = c;
				found = c;
            }
        }
        if(found->radius > 0.0){        
            if(!found->set_terrain(*this)){
                Stats.crejects++;
            }
        }
        else
            Stats.crejects++;
        if(ntest()){
            find_neighbors(found);
            list.ss();
            while((h=list++)){
                if(!h->set_terrain(*this))
                    Stats.nmisses++;
                h->users--; 
                // ⭐ MODIFIED: Don't delete from hash here
                // Cleanup happens in free_htable()
            }
            list.reset();
        }
        
    }
    if(!first() || !scnt)
        return;
    for(int i=0; i<scnt; i++){
        slist.base[i]=sdata+i;
    }

    slist.size=scnt;
    slist.sort();
    cval=slist.base[scnt-1]->f;
    slvl=slist.base[scnt-1]->l;
    hits=slist.base[scnt-1]->h;
    //cout<<hits<<" "<<chits<<endl;
}
//-------------------------------------------------------------
// Placement::find_neighbors()	build neighbor list
//-------------------------------------------------------------
void PlacementMgr::find_neighbors(Placement *placement)
{
    int i,j,k,l=0,l1=0,l2=0,n;
    int n4d=TheNoise.noise4D();
    Placement *h;
    
    if(n4d){
        l1=-1;
        l2=1;
    }
    
    list.ss();
    placement->users++;
    
    for(i=-1; i<=1; i++){
        for(j=-1; j<=1; j++){
            for(k=-1; k<=1; k++){
                for(l=l1; l<=l2; l++){
                    if(!i && !j && !k && !l)
                        continue;

                    Point4DL pc(placement->point.x+i,
                                placement->point.y+j,
                                placement->point.z+k,
                                placement->point.w+l);

        			int n=hashPoint(pc,lvl,id);
                                 
#ifdef DEBUG_PLACEMENTS
                    Stats.nvisits++;
#endif
                    // ⭐ MODIFIED: Search chain for matching placement
                    h = hash[n];
                    Placement* found = nullptr;
                    
                    while(h){
                        //if(h->point == pc && h->type == type && h->lvl ==lvl && h->instance == instance){
                    	if(h->isEqual(pc,type,lvl,instance,layer)){
                            found = h;
                            if(found->users){
#ifdef DEBUG_PLACEMENTS
                                Stats.nmisses++;
#endif
                                found = nullptr;  // Already in use
                            }
                            break;
                        }
#ifdef NO_CHAIN
            			break;
#endif 
                        h = h->next;
                        if(h)
                            Stats.nchained++;

                    }
                    
                    if(found){
#ifdef DEBUG_PLACEMENTS
                        Stats.nhits++;
#endif
                    }
                    else{
                        // Not found in chain, create new
                        Placement* c = make(pc, n);
                        
                        // ⭐ Add to front of chain
                        //if(c->flags.s.valid){
                        c->next = hash[n];
                        hash[n] = c;
                        found = c;
                        //}
                        //else
                        //	delete c;
                    }

                    if(found && found->radius > 0.0){
                        found->users++;
                        list.add(found);
                    }
#ifdef DEBUG_PLACEMENTS
                    else if(found)
                        Stats.nrejects++;
#endif
                }
            }
        }
    }
    placement->users--;
}

void PlacementMgr::getArgs(TNarg *left){
	extern double Theta,Phi,Slope,MinHt,MaxHt,Height,Hardness;
	TNarg &args=*((TNarg *)left);
	double arg[13];
	
	double f=0;
	double fs=1,fl=1,fh=1,fd=1;

	int n=getargs(&args,arg,13);
	// common 
	if(n>0) levels=(int)arg[0]; 	// scale levels
	if(n>1) maxsize=arg[1];     	// size of largest placement
	if(n>2) mult=arg[2];			// scale multiplier
	if(n>3) maxdensity=arg[3];      // density
	if(n>4) comp=arg[4];
	if(n>5) drop=arg[5];
	if(n>6) noise_amp=arg[6];
	if(n>7) noise_expr=arg[7];
	
	// standard (TN classes may overide)
	if(n>8) slope_bias=arg[8];
	if(n>9) ht_bias=arg[9];
	if(n>10) lat_bias=arg[10];
	if(n>11) hardness_bias=arg[11];
	if(n>12) selection_bias=arg[12];
	
	double dht=(Height-MinHt)/(MaxHt-MinHt);
	
	density=maxdensity;
//	if(slope_bias)
//		fs=calcDensity(Slope,0.3,slope_bias,0.1);
//	if(ht_bias){
//		fh=calcDensity(dht,0.5,ht_bias,0.5);	
//	}
//	if(lat_bias)
//		fl=calcDensity(fabs(2*Phi/180),0.5,lat_bias,0.5);
//	if(hardness_bias)
//		fd=calcDensity(Hardness,0.5,hardness_bias,0.5);
//	density*=fs*fh*fl*fd;
	//f=fs+fl+fh+fd;
   // density=maxdensity*(1+f);
	density=clamp(density,0,1);
#ifdef DEBUG_DENSITY
	if(cnt%10000==0){
		char buff[256];
		sprintf(buff,"density max:%-1.2f slope:%-1.2f(%-1.2f) ht:%-1.2f(%-1.2f) lat:%-1.2f final:%-1.2f ",
				maxdensity,Slope,fs,dht,fh,fl,density);
		cout<<buff<<endl;
	}
#endif
	cnt++;
	
}
//************************************************************
// class Placement
//************************************************************
Placement::Placement(PlacementMgr &pmgr,Point4DL &pt, int n) : point(pt)
{
	static int cnt=0;

	mgr=&pmgr;
    type=pmgr.type;
	hid=n;
	double d,r,pf=1;
	radius=0;
	users=0;
	flags.l=0;
	ht=0;
	dist=1e16;
	visits=0;
	//place_hits=0;
	instance=pmgr.instance;
	layer=pmgr.layer;
	lvl=pmgr.lvl;
	pts=0;
	rval=0;	
	next=0;
	wtsum=0;
	aveht=0;
	hits=0;
	vertex=MapPt+TheScene->vpoint;
	
#ifdef DEBUG_PLACEMENTS
	mgr->Stats.cmade++;
#endif
	double dns=mgr->density;
	Point4D	p(pt);

	int seed=PERM(hid);
	
	Point pp=Point(point.x,point.y,point.z);

	mgr->Stats.vtests++;

	r=Random(pp);
	rval=1024*fabs(r);//+instance;
	
	p=p-mgr->offset;
 	p=(p+0.5)*mgr->size;
	double rtest=rands[hid]+0.5;

//	if(mgr->dexpr){  // density expr
//		Point4D p1=p*TheNoise.scale+TheNoise.offset;
//	    SPUSH;
//		TheNoise.push(p1);
//		CurrentScope->revaluate();
//		mgr->dexpr->eval();
//		TheNoise.pop();
//		dns+=S0.s;
//		SPOP;
//		CurrentScope->revaluate();
//		dns=clamp(dns,0,1);
//	}

	if(rtest>dns){
		PlacementMgr::Stats.dns_fails++;
#ifdef DEBUG_DENSITY2
		if(cnt%1000==0)
			cout<<"fail dns:"<<dns<<" rtest:"<<rtest<<endl;
#endif
		return;
	}
	PlacementMgr::Stats.dns_pass++;
#ifdef DEBUG_DENSITY2
	if(rtest<=dns && cnt%1000==0)
	  cout<<"pass dns:"<<dns<<" rtest:"<<rtest<<endl;
	cnt++;
#endif
	d=fabs(p.length()-1);
	double rf=1-mgr->mult;
	
	// randomly mis-align placement center 
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
	p=p+mgr->offset;
	if(TheNoise.noise3D())
	    p.w=0;
	center=p;
	radius=r;
	

	flags.s.valid=true;
}

//-------------------------------------------------------------
// Placement::set_terrain()	impact terrain (maybe overridden)
//-------------------------------------------------------------
bool Placement::set_terrain(PlacementMgr &pmgr)
{
	double d=pmgr.mpt.distance(center);
	d=d/radius;

	pmgr.sval=0;
	visits++;
	
	if(d>1.0)
		return false;
	if(!flags.s.valid)
		return false;


    flags.s.active=true;
    pmgr.sval=lerp(d,0,1.0,0,1);
    
 	if(d<dist){
		ht=Height; // closest to center
		dist=d;
	}
    if(pmgr.useaveht()){
    	double wt=1/(0.01+pmgr.sval);
     	aveht+=Height*wt;	
    	wtsum+=wt;
		ht=aveht/wtsum;
    }

	hits++;
	pmgr.sdata[pmgr.scnt].h=hits;
	pmgr.sdata[pmgr.scnt].v=hid;
	pmgr.sdata[pmgr.scnt].f=pmgr.sval;
	pmgr.sdata[pmgr.scnt].l=pmgr.lvl;
  	if(pmgr.scnt<SDATA_SIZE)
  		pmgr.scnt++;

	return true;
}

void Placement::reset(){
	flags.s.active=0;
	visits=0;
	dist=1e6;
}
void Placement::dump(){
	if(flags.s.valid && flags.s.active){
		Point4D p(point);
		p=center;
		char msg[256];
		char vh[32];
		//sprintf(vh,"%d:%d",visits,place_hits);
		sprintf(msg,"%-3d %-2d %-8s dist:%-0.4f ht:%-1.6f x:%-1.5f y:%-1.5f z:%1.5f",cnt++,flags.l,vh,dist,ht,p.x,p.y,p.z);
		cout<<msg<<endl;
	}
}

inline bool Placement::isEqual(Point4DL &p, int t, int l, int i, int s){
	return (p == point && t == type && l ==lvl && i == instance && s==layer);
}
//-------------------------------------------------------------
// Placement::setVertex() set Placement surface vertex
//-------------------------------------------------------------
void Placement::setVertex() {
	Point4D p(center);
	Point pp = Point(p.x, p.y, p.z);
	normal = pp.normalize();
	Point ps = pp.spherical();
	Point base = TheMap->point(ps.y, ps.x, ht); // spherical-to-rectangular
	vertex = Point(-base.x, base.y, -base.z);   // Point.rectangular has 180 rotation around y (??)
	double d = vertex.distance(TheScene->vpoint);  // distance	
	double r = TheMap->radius * radius;
	pts = TheScene->wscale * r / d;
	dist=d;
}

//************************************************************
// PlaceData class
//************************************************************
// copy constructor
PlaceData::PlaceData(Placement *pnt){
	type=pnt->type;
	ht=pnt->ht;
	vertex=pnt->vertex;
	normal=pnt->normal;	
	radius=pnt->radius;
    pts=pnt->pts;
 	dist=pnt->dist;//TheScene->vpoint.distance(t);
	instance=pnt->instance;
	mgr=pnt->mgr;
	rval=pnt->rval;
}

//************************************************************
// PlaceObjMgr class
//************************************************************
bool PlaceObjMgr::shadow_mode=false;

PlaceObjMgr::PlaceObjMgr(){
	objs.reset();
}
void PlaceObjMgr::eval(){
	for(int i=0;i<objs.size;i++){
		objs[i]->expr->eval(); // TNode -> eval()
	}
}

//************************************************************
// PlaceObj class
//************************************************************
PlaceObj::PlaceObj(int t, TNode *e){
	type=t;
	get_id();
	expr=e;
	valid=false;
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
		mgr->set_pid(type);
}

int TNplacements::get_id()
{
	return type&PID;
}

void TNplacements::set_flip(int i)   	    { if(i)BIT_ON(type,FLIP); else BIT_OFF(type,FLIP); }
bool TNplacements::is3D()   				{ return type & MC3D;}
void TNplacements::set3D(bool b)          { BIT_SET(type,MC3D,b);}

//-------------------------------------------------------------
// TNplacements::reset()	reset hash table
//-------------------------------------------------------------
void TNplacements::reset()
{
	PlacementMgr::free_htable();
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
//#define DEBUG_DENSITY_CALC
double PlacementMgr::calcDensity(double s, double m, double b, double p){
	double x = (s - m) / p;
	    
	// Sigmoid curve: 0 to 1, centered at m
	double sigmoid = 1.0 / (1.0 + exp(-x * 6.0));
	
	// Apply bias:
	// b = -1: return (1 - sigmoid) - fully inverted
	// b = 0: return 0.5 - neutral/no preference
	// b = +1: return sigmoid - normal preference
	
	double y;
	if (b >= 0) {
		// Positive bias: blend from 0.5 to sigmoid
		y = 0.5 + b * (sigmoid - 0.5);
	} else {
		// Negative bias: blend from 0.5 to (1 - sigmoid)
		y = 0.5 + b * (sigmoid - 0.5);  // This inverts when b is negative
	}
	
	#ifdef DEBUG_DENSITY_CALC
	if(cnt%10000==0 && CurrentScope->spass())
		cout << "m:" << m << " b:" << b << " s:" << s << " p:" << p 
			 << " sigmoid:" << sigmoid << " y:" << y << endl;
	cnt++;
	#endif
	
	return clamp(y, 0.0, 1.0);//	double delta = s - m;

}

//-------------------------------------------------------------
// TNplacements::eval() evaluate common arguments
//-------------------------------------------------------------
void TNplacements::eval()
{
	double arg[5];
	INIT;
	TNarg &args=*((TNarg *)left);
	TNode *dexpr;
	
	int n=getargs(&args,arg,4);

	if(n>0) mgr->levels=(int)arg[0]; 	// scale levels
	if(n>1) mgr->maxsize=arg[1];     	// size of largest placements
	if(n>2) mgr->mult=arg[2];           // variability in size or level scale
	if(n>3) mgr->maxdensity=arg[3];      
	
	MaxSize=mgr->maxsize;
}

bool TNplacements::inLayer() {
	NodeIF *p = getParent();
	while (p && p->typeValue() != ID_LAYER) {
		p=p->getParent();
	}
	if(p && p->typeValue() == ID_LAYER)
		return true;
	return false;
}
