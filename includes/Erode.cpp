#include "NodeData.h"
#include "Erode.h"

// Erosion algorithms - non uniform grid
// I. Simple height function
//  algorithm:
//  - move material (sediment) from high to low areas
//  implementation:
//  - generate terrain normally until some sub-division threshold is reached level=l1
//  - if height(z) < some max value(zmax) and > some min value(zmin) decrease z proportional to z - zmin
//  effects: 
//  - produces cliff like structures when z is between zmin and zmax
//  - otherwise terrain is unchanged
//  - doesn't require addition data other than unmodified height function (rock)
// II. Thermal erosion
// alorithm:
// - "slump" terrain to constrain max slope to some critical angle (e.g. 45 degrees)
// III. Hydrolic erosion
// alorithm:
// - carry "sediment" from high to low height areas using water model
// - each vertex needs water value, sediment value, current height (+hardness), slope (2d)
// - if slope > threshold && water value> threshold, remove material (sediment) from vertex (decrease ht)
// - otherwise, if sediment > water carry limit, deposit sediment at vertex (increase ht)
// - at each vertex move sediment and water between 4 neighboring vertexes (use delta ht, slope to determine outcome)
// - "evaporate" some of the water (decrease carry capacity)
// - continue until 
// challenges:
// - most published algorithms use iterative methods to move material around in a fixed grid
// - with dynamically generated vertexes this may be more problematic


//**************** extern API area ************************
extern NameList<LongSym*> NOpts;

//extern int hits,misses,visits;
static 			TerrainData Td;

//**************** static and private *********************

#define NBSED(n) mapdata[n]->sediment()
#define smallest(x,y) x<y?x:y
#define largest(x,y)  x>y?x:y

extern double   ptable[];
extern const double INV2PI;
const unsigned int  MAXLVLS=63;

//-------------------------------------------------------------
// cell_min()		return fractal min ht
//-------------------------------------------------------------
//static double sedmin()
//{
//	double s=smallest(NBSED(0),NBSED(1));
//	if(mdcnt==4){
//		double s2=smallest(NBSED(2),NBSED(3));
//		if(s2<s)
//			s=s2;
//	}
//	return s;
//}

//-------------------------------------------------------------
// cell_ave()		return ave sediment
//-------------------------------------------------------------
static double ave_sediment()
{
	double s1,s2,s3,s4,f1,f2;
 	static double lastz=0;
 	if(mdcnt>0){
        s1=mdctr->span(mapdata[0]);
        s2=mdctr->span(mapdata[1]);
        f1=s2/(s1+s2);
        if(mdcnt==4){
            s3=mdctr->span(mapdata[2]);
            s4=mdctr->span(mapdata[3]);
            f2=s4/(s3+s4);
            double ns0=mapdata[0]->sediment();
            double ns1=mapdata[1]->sediment();
            double ns2=mapdata[2]->sediment();
            double ns3=mapdata[3]->sediment();
            lastz=0.5*(f1*ns0+(1-f1)*ns1+f2*ns2+(1-f2)*ns3);
        }
        else
            lastz=f1*NBSED(0)+(1-f1)*NBSED(1);
 	}
	return lastz;
}

#define NBRHT(n) mapdata[n]->fractal()

double eslope()
{
    double smax=100;
    double s=0;
    CELLSLOPE(solid(),s);
    //s=smoothstep(0,smax,s);

    double slope=TheMap->hscale*s*INV2PI;

    return slope;
}

//************************************************************
// TNhardness class
//************************************************************
TNhardness::TNhardness(TNode *l, TNode *r) : TNfunc(l,r) {}
//-------------------------------------------------------------
// TNhardness::eval() evaluate the node
//-------------------------------------------------------------
void TNhardness::eval()
{
  	TNarg *arg=(TNarg*)left;
	double softness=0;
	if(arg){
	    arg->eval();
	    softness=clamp(S0.s,0,1);
		//cout<<"["<<S0.hardness<<"]";
	}
	INIT;
	if(right){
		right->eval();
	}
	Td.softness=softness;
	S0.set_hardness();
}

//-------------------------------------------------------------
// TNfunc::applyExpr() apply expr value
//-------------------------------------------------------------
void TNhardness::applyExpr()
{
    if(expr){
        DFREE(left);
        left=expr->left;
        left->setParent(this);
        expr=0;
    }
    //if(right)
    //    right->applyExpr();
}
//************************************************************
// TNerode class
//************************************************************
TNerode::TNerode(int t, TNode *l, TNode *r) : TNfunc(l,r) 
{
	options=t;
}

//-------------------------------------------------------------
// TNerode::save() evaluate the node
//-------------------------------------------------------------
void TNerode::save(FILE *f)
{
	char buff[256];
	buff[0]=0;
	propertyString(buff);
	fprintf(f,"%s",buff);
	if(right){
		right->save(f);
	}
}

//-------------------------------------------------------------
// TNerode::optionString() get option string
//-------------------------------------------------------------
void TNerode::optionString(char *c)
{
    if(!options)
        return;
	int nopt=options&NOPTS;
	int i;
	for(i=0;i<NOpts.size;i++){
    	if(nopt & NOpts[i]->value){
    	    if(c[0])
   			    strcat(c,"|");
   			strcat(c,NOpts[i]->name());
   		}
	}
}

//-------------------------------------------------------------
// TNerode::propertyString() get property string
//-------------------------------------------------------------
void TNerode::propertyString(char *s)
{
	sprintf(s+strlen(s),"%s(",typeName());
	char opts[64];
	opts[0]=0;
	optionString(opts);
	if(opts[0])
	    sprintf(s+strlen(s),"%s,",opts);
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
// TNfunc::applyExpr() apply expr value
//-------------------------------------------------------------
void TNerode::applyExpr()
{
    if(expr){
    	options=((TNerode*)expr)->options;
        DFREE(left);
        left=expr->left;
        left->setParent(this);
        expr=0;
    }
    if(right)
        right->applyExpr();
}

//-------------------------------------------------------------
// TNerode::eval()
//
// Elevation-weighted erosion. Single-pass, locally computed, no artefacts.
//
// Carves terrain below a threshold elevation, leaving higher ground as mesas.
//
//   base < level              → full incision = depth  (valley floors)
//   level..level+edge         → incision tapers depth→0  (cliff/hillside)
//   base > level+edge         → no incision  (mesa tops, untouched)
//
// Visual controls:
//   depth  — how much lower the valleys are vs the mesa tops (Z-units)
//   level  — the Z threshold separating valley from mesa.
//             Lower level = smaller mesa tops (more terrain below it = more eroded).
//             Higher level = larger mesa tops.
//   edge   — width of the cliff zone above 'level'.
//             Small edge = sharp vertical cliff.
//             Large edge = gentle hillside slope.
//   shape  — profile of the cliff face.
//             1 = linear, 2 = concave (overhangs near top), 0.5 = convex (rounded)
//
// args:
//   0  depth  0.3   incision in Z-units (height difference mesa top to valley floor)
//   1  level  0.0   Z threshold: below = full erosion, above = mesa top
//   2  edge   0.4   cliff zone width in Z-units (delta above level)
//   3  shape  1.0   cliff profile exponent
//
// Options: SQR = square the weight (sharper cliff-top edge)
//-------------------------------------------------------------
void TNerode::eval()
{
    INIT;
    Td.set_flag(EVALUE);

    TNarg *arg = (TNarg*) left;
    double args[4];
    int n = getargs(arg, args, 4);

    double depth = n > 0 ? args[0] : 0.3;
    double level = n > 1 ? args[1] : 0.0;
    double edge  = n > 2 ? args[2] : 0.4;
    double shape = n > 3 ? args[3] : 1.0;

    if (edge <= 0.0) edge = 0.001;
    double level_top = level + edge;

    if (!isEnabled()) {
        if (right) right->eval();
        return;
    }
    if (CurrentScope->rpass()) {
        Td.set_flag(EVALUE);
        if (right) right->eval();
        return;
    }

    double base = 0.0;
    if (right) {
        right->eval();
        base = S0.pvalid() ? S0.p.z : S0.s;
    }
    Td.rock = base;

    // ── Erosion weight ─────────────────────────────────────────
    double w;
    if (base >= level_top)
        w = 0.0;                                      // mesa top: untouched
    else if (base >= level)
        w = 1.0 - (base - level) / edge;             // cliff zone: taper 1→0
    else
        w = 1.0;                                      // valley: full erosion

    if (options & SQR) w = w * w;
    if (shape != 1.0) w = pow(w, shape);

    double incision = depth * w;
    double z = base - incision;

    if (S0.pvalid()) S0.p.z = z;
    else             S0.s   = z;

    Td.sediment = -incision;
}
