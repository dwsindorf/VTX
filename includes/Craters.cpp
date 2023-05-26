

#include "Craters.h"
#include "SceneClass.h"
#include "RenderOptions.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define DEBUG_CRATERS

static 	TerrainData Td;
extern double Hscale,Height;
double Impact=0;

class CraterData {
public:
    double v;
    double f;
    double h;
    double b;
    double r;
    double d;
    double value()   { return v;}
};
static CraterData   cdata[256];
static ValueList<CraterData*> clist(cdata,256);
static int          ccnt;
static double  cmax;
static double  cmin;
static double  cval;
static double  dval;
static double  aht;
static double  bht;

static CraterMgr s_cm(FINAL);    // static finalizer
//static const char *def_rnoise_expr="noise(GRADIENT,0,12,0.5,0.4,1.9873215)\n";

static const char *def_rnoise_expr="noise(GRADIENT|NABS|RO1,0,14,0.5,0.6,2)\n";
static const char *def_vnoise_expr="noise(GRADIENT|NABS|SQR|NEG|RO1,1,10,0.4,0.5,1.9873215)\n";
//************************************************************
// classes Crater, CraterMgr
//************************************************************
//************************************************************
// CraterMgr class
//************************************************************
//  PlacementMgr args
//
//	arg[0]  levels   		scale levels
//	arg[1]  maxsize			size of largest craters
//	arg[2]  mult			size multiplier per level
//	arg[3]  density			density of dexpr
//
//  CraterMgr args
//
//	arg[4]  ampl		    amplitude factor
//	arg[5]  impact		    impact factor
//	arg[6]  rnoise		    radial noise component
//	arg[7]  vnoise	        vertical noise component
//	arg[8]  rise			rise factor
//	arg[9]  drop			drop factor
//	arg[10] rim			    rim radius
//	arg[11] flr			    floor radius
//	arg[12] ctr				center radius
//-------------------------------------------------------------
TNode *CraterMgr::default_rnoise=0;
TNode *CraterMgr::default_vnoise=0;

CraterMgr::CraterMgr(int i) : PlacementMgr(i)
{

	rim=0.8;
	flr=0.6;
	ctr=0.2;
	drop=1.0;
	rise=0.5;
	impact=0.8;
	offset=0;
	ampl=1;
 	noise_vertical=1;
  	noise_radial=1;
  	rnoise=vnoise=0;
}
CraterMgr::~CraterMgr()
{
  	if(finalizer()){
#ifdef DEBUG_PMEM
  		printf("CraterMgr::free()\n");
#endif
        DFREE(default_rnoise);
        DFREE(default_vnoise);
	}
}

//-------------------------------------------------------------
// CraterMgr::init()	initialize global objects
//-------------------------------------------------------------
void CraterMgr::init()
{
	if(default_rnoise==0){
#ifdef DEBUG_PMEM
  		printf("CraterMgr::init()\n");
#endif
		default_rnoise=(TNode*)TheScene->parse_node((char*)def_rnoise_expr);
		default_vnoise=(TNode*)TheScene->parse_node((char*)def_vnoise_expr);
	}
	PlacementMgr::init();
}

//-------------------------------------------------------------
// CraterMgr::reset() reset for eval pass
//-------------------------------------------------------------
void CraterMgr::reset()
{
	if(first()){
		ccnt=0;
    	cval=0;
   		cmax=0;
   		cmin=10;
   		dval=0;
   	}
}

//-------------------------------------------------------------
// CraterMgr::eval()	evaluate terrain
//-------------------------------------------------------------
// Crater impact model:
//   -  as each TNcraters object is evaluated in the terrain stack a call
//      to it's CraterMgr object is made.
//   -  each call to CraterMgr may result in several calls to
//      Crater::set_terrain as craters from multiple passes and neighbors
//      are evaluated.
//   -  each time a Crater::set_terrain call is made a CraterData
//      object (cdata[ccnt++]) is initialized with crater ht, impact
//      and base data and a unique identifier value (hid).
//   -  at the end of the terrain stack eval pass the list of ccnt
//      CraterData objects is sorted using the (arbitrary) crater
//      identifier (hid) as the sort index.
//   -  the sorted list represents the crater bombardment order
//      with the latest event stored as the first item in the list.
//   -  the list is evaluated for all ccnt objects. As each object
//      is processed the CraterMgr's height value (aht) is increased by the
//      Crater's height value, damped by the current value of the
//      CraterMgr's impact value (cval). After each Crater is processed
//      the Crater's impact value is added to cval.
//   -  In the crater base calculation, craters are evaluated in reverse
//      order (first=earliest). The base value is modified using a combined
//      lowest (zbuffer) and additive calculation. At the highest impact
//      value only the zbuffer term is used.
//
//  NOTES:
//    -  It is necessary to first capture the list of impact events and
//       then sort using an order-independent identifier because even
//       though the same craters may be encountered at a given surface
//       local the order in which the craters are evaluated via
//       set_terrain may vary arbitrarily from point to point.
//
//-------------------------------------------------------------
void CraterMgr::eval()
{
 	int i;
	PlacementMgr::eval();
	cmin=size<cmin?size:cmin;
	cmax=maxsize>cmax?maxsize:cmax;

	if(!first())
	    return;

	for(i=0;i<ccnt;i++){
	    clist.base[i]=cdata+i;
	}
	clist.size=ccnt;
	clist.sort();

	aht=bht=0;
	
	for(i=0;i<ccnt;i++){
	    double f=clist.base[i]->f;
	    double r=clist.base[i]->r;
	    double a=rampstep(0,(1-0.5*impact)*r,cval,1,1-impact);
	    double h=clist.base[i]->h*a;
	    double b=clist.base[ccnt-i-1]->b;
		cval+=f;
		//dval=clist.base[i]->d>dval?clist.base[i]->d:dval;
		dval=clist.base[i]->d;

		if(options & MAXA)
		    aht=h>aht?h:aht;
		else
			aht+=h;
		if(options & MAXB)
			bht=b<bht?b:bht;
		else{
			a=b*impact;
			bht=a<bht?a:bht;
			bht+=(1-impact)*b;
		}
	}
 }

//-------------------------------------------------------------
// CraterMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *CraterMgr::make(Point4DL &p, int n)
{
    return new Crater(*this,p,n);
}

//************************************************************
// class Crater
//************************************************************
Crater::Crater(PlacementMgr&m, Point4DL&p,int n) : Placement(m,p,n)
{
}

//-------------------------------------------------------------
// Crater::set_terrain()	impact terrain
//-------------------------------------------------------------
bool Crater::set_terrain(PlacementMgr &pmgr)
{
	double d,rr=0,vn=0,f,c=0,r;
	double drop,rise,scale,b,h,a=0;
	double max=1.2;

	d=pmgr.mpt.distance(center);
	d=d/radius;

	if(d>1.5)
		return false;

	CraterMgr &mgr=(CraterMgr&)pmgr;

	scale=100*mgr.ampl*radius;
	//scale=0.05*mgr.ampl*radius/Hscale;
	rise=scale*mgr.rise;
	drop=-scale*mgr.drop;

	r=d;
	double phid=0.5*PERM(hid);	  // 0-16k
	if(mgr.noise_radial>0){
		SPUSH;
		Point4D np=(mgr.mpt-center)*((d+0.25)/radius);
		np=np+phid;
		TheNoise.push(np);
		CurrentScope->revaluate();
		mgr.rnoise->eval();
		TheNoise.pop();
		rr=0.25*mgr.noise_radial*S0.s;
		SPOP;
		r=d-rr;
		if(!mgr.noise_vertical)
			CurrentScope->revaluate();
	}
	if(mgr.noise_vertical>0){
		SPUSH;
		Point4D np;
		np=np+phid;
		if(mgr.offset_valid())
		    np=(mgr.mpt-mgr.offset)*(1/radius);
		else
		    np=(mgr.mpt)*(1/radius);
		np=np+1/radius;
		if(TheNoise.noise3D())
		    np.w=0;

		TheNoise.push(np);
		if(!mgr.noise_radial)
			CurrentScope->revaluate();
		mgr.vnoise->eval();
		vn=0.25*mgr.noise_vertical*S0.s;
		TheNoise.pop();
		SPOP;
		CurrentScope->revaluate();
	}
	b=drop;
	h=0;
	f=radius;
	if(mgr.ctr>0){
		a=rampstep(0.01,0.001,radius,1,0);
    	c=0.125*mgr.noise_radial;
    }
	if(a>0 && r<=mgr.ctr-c)			// center peak
		h=rampstep(-c,mgr.ctr-c,r,-a*drop,0);
	if(r<=mgr.flr)					// crater floor
		h=0;
	else if(r<mgr.rim){				// rim (inside)
		f=smoothstep(max,mgr.flr,r,0,radius);
	    h=rampstep(mgr.rim,mgr.flr,r,rise,0);
	    b=rampstep(mgr.rim,mgr.flr,r,0,drop);
	}
	else {							// rim (outside)
	    h=rampstep(max,mgr.rim,r,0,rise);
	    b=0;
		f=smoothstep(max,mgr.flr,r,0,radius);
	}

	h=h*(1+vn)-0.25*b*vn;

 	cdata[ccnt].v=hid;
 	cdata[ccnt].h=h;
 	cdata[ccnt].f=f;
  	cdata[ccnt].b=b;
  	cdata[ccnt].d=d;
  	cdata[ccnt].r=radius;
  	if(ccnt<255)
  	    ccnt++;
  	return true;
}

//************************************************************
// TNcraters class
//************************************************************
TNcraters::TNcraters(int t, TNode *l, TNode *r, TNode *b) : TNplacements(t|CRATERS,l,r,b)
{
    mgr=new CraterMgr(type);
	TNarg &args=*((TNarg *)left);
	TNode *arg=args[3];
	if(arg && (arg->typeValue() != ID_CONST))
		mgr->dexpr=arg;
}

TNcraters::~TNcraters()
{
	if(mgr)
		delete(CraterMgr*)mgr;
	mgr=0;
}

//-------------------------------------------------------------
// TNcraters::valueString()	set value string
//-------------------------------------------------------------
void TNcraters::valueString(char *s)
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
// TNcraters::addToken() add to token list
//-------------------------------------------------------------
void TNcraters::addToken(LinkedList<TNode*>&l)
{
	l.add(this);
	if(right){
		right->addToken(l);
	}
}

//-------------------------------------------------------------
// TNcraters::init() initialize the node
//-------------------------------------------------------------
void TNcraters::init()
{
	CraterMgr *cmgr=(CraterMgr*)mgr;
	cmgr->init();
	if(!Td.get_flag(CFIRST)){
#ifdef DEBUG_CRATERS
		printf("TNcraters::init() first\n");
#endif
		cmgr->set_first(1);
		if(join(right))
		    Td.set_flag(CFIRST);
	}
	else {
	    if(joined(this)){
#ifdef DEBUG_CRATERS
			printf("TNcraters::init() joined \n");
#endif
			if(!join(right))
	    	    Td.clr_flag(CFIRST);
		}
		else
	    	Td.clr_flag(CFIRST);
	}
	TNplacements::init();
}

//-------------------------------------------------------------
// TNcraters::join() join child craters
//-------------------------------------------------------------
int TNcraters::join(TNode *n)
{
    if(!n || n->typeValue() !=ID_CRATERS || type & MAXHT)
         return 0;
	TNcraters* child=(TNcraters*)n;
	CraterMgr *cmgr=(CraterMgr*)(child->mgr);
    cmgr->set_joined(1);
    return 1;
}

//-------------------------------------------------------------
// TNcraters::join() join child craters
//-------------------------------------------------------------
int TNcraters::joined(TNode *n)
{
    if(!n || n->typeValue() != ID_CRATERS)
        return 0;
	TNcraters* child=(TNcraters*)n;
	CraterMgr *cmgr=(CraterMgr*)(child->mgr);
    return cmgr->joined();
}

//-------------------------------------------------------------
// TNcraters::next() join child craters
//-------------------------------------------------------------
TNode *TNcraters::next()
{
    if(!right || right->typeValue() != ID_CRATERS || !joined(right))
        return right;
	TNcraters* child=(TNcraters*)right;
    return child->next();
}

//-------------------------------------------------------------
// TNcraters::applyExpr() apply expr value
//-------------------------------------------------------------
void TNcraters::applyExpr()
{
    if(expr){
		TNcraters* ctrs=(TNcraters*)expr;
        DFREE(left);
        left=ctrs->left;
        left->setParent(this);
		delete mgr;
		type=ctrs->type;
		mgr=ctrs->mgr;
		ctrs->left=0;
		ctrs->mgr=0;
		eval();
		delete ctrs;
        expr=0;
    }
    if(base)
        base->applyExpr();
}

//-------------------------------------------------------------
// TNcraters::eval() evaluate the node
//-------------------------------------------------------------
void TNcraters::eval()
{
	TNarg &args=*((TNarg *)left);
	double arg[16];
	CraterMgr *cmgr=(CraterMgr*)mgr;
	
	//Point4D pv=TheNoise.get_point();

	cmgr->init();

    TNode *argexpr=args[4];   // ampl expr
    if(argexpr){
    	argexpr->eval();
    	cmgr->ampl=S0.s;
    }
    argexpr=args[5];   // impact expr
    if(argexpr){
    	argexpr->eval();
    	cmgr->impact=S0.s;
    }
	if(cmgr->rnoise==0){
		cmgr->rnoise=cmgr->default_rnoise;
    	argexpr=args[6];          // rnoise expr
	    if(argexpr){
	    	if(argexpr->typeValue() == ID_CONST){
		    	argexpr->eval();
		     	cmgr->noise_radial=S0.s;
		 	}
			else
		     	cmgr->rnoise=argexpr;
		}
	}
	if(cmgr->vnoise==0){
    	argexpr=args[7];          // vnoise expr
		cmgr->vnoise=cmgr->default_vnoise;
		if(argexpr){
	     	if(argexpr->typeValue() == ID_CONST){
		     	argexpr->eval();
		     	cmgr->noise_vertical=S0.s;
		 	}
		 	else
		     	cmgr->vnoise=argexpr;
		}
	}
	TNplacements::eval();  // evaluate common arguments
	TNarg *a=args.index(8);

	if(a){                // geometry exprs
		int n=getargs(a,arg,6);
		if(n>0) cmgr->rise=arg[0];        // rise factor
		if(n>1) cmgr->drop=arg[1];        // drop factor
		if(n>2){
			cmgr->rim=arg[2]; 			  // rim radius
			cmgr->flr=0.6*cmgr->rim;
			cmgr->ctr=cmgr->flr-0.2;
		}
		if(n>3){
			cmgr->flr=arg[3]; 	   	 	  // floor radius
			cmgr->ctr=cmgr->flr-0.2;
		}
		if(n>4)
			cmgr->ctr=arg[4];             // center peak radius
		if(n>5)
			cmgr->offset=arg[5];          // ht bias
	}
	cmgr->reset();

    int rbase=0;
	if(joined(right)){
	    rbase=1;
	    right->eval();
	}

	cmgr->eval();
	if(joined(this))
	    return;

 	double impact=smoothstep(0,cmin,cval,1,1-cmgr->impact);

 	Impact=cval/cmax;
#ifdef DEBUG_CRATERS
	static int maxcnt=0;
	if(ccnt>maxcnt){
	    maxcnt=ccnt;
	    printf("maxcnt %d  cmax %g cmin %g\n",maxcnt,cmax,cmin);
	}
#endif
	double hb=0;
	if(base){
		base->eval();
		if(S0.pvalid())
			hb=S0.p.z;
		else if(S0.svalid())
			hb=S0.s;
	}
    hb+=cmgr->offset;

    INIT;
    double ht=0;
    TNode *n=next();

    double h1=aht;
    double h2=bht;

	if(n){
		n->eval();
		if(S0.pvalid())
			ht=S0.p.z;
		else
			ht=S0.s;
	}
    ht-=hb;
	if(type & MAXHT)
    	ht=h1>ht?h1:ht;
    else
 	    ht=h1+ht*impact;
    ht+=h2+hb;
    if(type & CNORM || images.building()){
        //double ampl=0.025*cmax/Hscale;
       double ampl=0.025*cmax;
       ht=(ht+ampl*cmgr->drop)/(ampl*cmgr->rise+ampl*cmgr->drop);
    }
	//else
	//	ht*=cmgr->ampl;
	if(S0.pvalid()){
		S0.p.z=ht;
		S0.set_pvalid();
	}
	else{
		S0.s=ht;
		S0.set_svalid();
	}
	S0.clr_constant();
//#define TEST
#ifdef TEST
	S0.set_cvalid();
	if(fabs(dval)>0)
		S0.c=Color(1-dval,0,0);
	else
		S0.c=Color(1,1,1);
#endif
}
