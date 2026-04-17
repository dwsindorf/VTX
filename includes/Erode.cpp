#include "NodeData.h"
#include "Erode.h"

//--------------------------------------------------------------------
// TNerode — single-pass elevation-weighted terrain incision
//--------------------------------------------------------------------
//
// ALGORITHM SUMMARY
//
// This is a single-pass, locally-computed elevation classifier rather
// than a true erosion simulation.
//
// For each terrain node as it is created by the LOD system, the child
// expression (noise, fractal, etc.) is evaluated to get the raw height
// 'base'. An erosion weight 'w' is then computed based solely on where
// 'base' falls within the elevation range defined by 'level' and 'edge':
//
//   base < level            →  w = 1.0  (full incision — valley floor)
//   level..level+edge       →  w tapers 1→0  (cliff face transition)
//   base > level+edge       →  w = 0.0  (untouched — mesa/plateau top)
//
// The final output height is:  z = base - depth * w
//
// The cliff profile is shaped by pow(w, shape) after the ramp.
// The SS option replaces the linear ramp with a smoothstep curve,
// eliminating derivative discontinuities at the ramp boundaries.
//
// WHAT IT PRODUCES
//
// The algorithm is more accurately described as differential subsidence
// than hydraulic erosion. It selectively lowers terrain below a threshold
// elevation, leaving higher ground as plateaus and creating cliff faces
// at the transition zone. This naturally produces mesa and butte landforms
// similar to arid environments such as the Colorado Plateau, where
// differential erosion of hard and soft rock layers creates flat-topped
// elevated terrain with steep sides.
//
// WHAT IT DOESN'T DO
//
// Real dendritic drainage forms because water from a large catchment area
// concentrates into progressively fewer, deeper channels following the
// steepest descent paths. That process requires either a global
// pre-computation pass over the whole terrain grid, or an iterative
// simulation with multiple passes. Neither is feasible in VTX's on-demand
// single-pass LOD architecture where each node is evaluated independently
// with only its immediate neighbours available.
//
// To approximate drainage patterns within the existing architecture, the
// most promising direction would be to modulate 'w' by a noise function
// with a dendritic spatial structure — such as Voronoi-based cell noise
// where the ridges between cells mimic watershed divides. Adding a fractal
// layer on top of erode (as an outer expression wrapper) already gets
// partway there by contributing directional streaking on the cliff faces.
//
//--------------------------------------------------------------------

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

    double base_n = base;

    // ── Erosion weight ─────────────────────────────────────────
    // SS: smoothstep ramp — zero derivative at both ends, removes
    //     the crease where the ramp meets the flat valley/mesa.
    // shape: applied to w after the ramp — controls cliff profile.
    //   >1 = more erosion concentrated near the bottom (concave cliff)
    //   <1 = more erosion spread toward the top (convex cliff)
    double w;
    if (base_n >= level + edge) {
        w = 0.0;
    } else if (base_n >= level) {
        double t = (base_n - level) / edge;  // 0 at level, 1 at level+edge
        if (options & SS)
            w = 1.0 - (t * t * (3.0 - 2.0 * t));  // smoothstep
        else
            w = 1.0 - t;                            // linear
    } else {
        w = 1.0;
    }

    if (options & SQR) w = w * w;
    if (shape != 1.0)  w = pow(w, shape);

    double incision = depth * w;
    double z = base - incision;

    if (S0.pvalid()) S0.p.z = z;
    else             S0.s   = z;

    Td.sediment = -incision;
}
