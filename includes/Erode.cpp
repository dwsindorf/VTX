#include "NodeData.h"
#include "Erode.h"
#include "NoiseFuncs.h"  // for Voronoi::edge()
#include "NoiseClass.h"   // for TheNoise.get_point(), Noise4D, Voronoi4D

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
extern double   Theta, Phi;  // per-cell geographic coords (degrees), set before eval()
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
//-------------------------------------------------------------
// Elevation-weighted erosion with optional gradient-based drainage channels.
//
// ELEVATION WEIGHT (always active):
//   base < level              → w_elev = 1  (valley floors, full erosion)
//   level..level+edge         → w_elev tapers 1→0  (cliff face)
//   base > level+edge         → w_elev = 0  (mesa tops, untouched)
//
// DRAINAGE WEIGHT (active when nchannels > 0):
//   Uses the height gradient across neighbours to determine local flow
//   direction. The flow angle is quantized into nchannels sectors. Cells
//   near the centre of a sector lie on a channel floor (w_drain = 1).
//   Cells near a sector boundary lie on a watershed ridge (w_drain = 0).
//   This produces a radial drainage pattern whose channel density and
//   width are controlled by nchannels and drain_mix.
//
//   The gradient is computed from mapdata[] solid() values — the same
//   neighbour heights used by CELLSLOPE. With mdcnt < 4 the drainage
//   weight falls back to the elevation weight alone.
//
// Combined: incision = depth * w_elev * lerp(1, w_drain, drain_mix)
//
// args:
//   0  depth       0.3   incision depth in Z-units
//   1  level       0.0   Z threshold separating valley from mesa
//   2  edge        0.4   cliff zone width above level
//   3  shape       1.0   cliff profile exponent
//   4  nchannels   0     number of drainage sectors (0 = elevation only)
//   5  drain_mix   1.0   blend: 0=elevation only, 1=drainage modulates fully
//
// Options: SQR = square elev weight (sharper cliff top)
//          SS  = smoothstep ramp (smoother cliff transitions)
//-------------------------------------------------------------
void TNerode::eval()
{
    INIT;
    Td.set_flag(EVALUE);

    TNarg *arg = (TNarg*) left;
    double args[10];
    int n = getargs(arg, args, 10);

    double depth      = n > 0 ? args[0] : 0.3;
    double level      = n > 1 ? args[1] : 0.0;
    double edge       = n > 2 ? args[2] : 0.4;
    double shape      = n > 3 ? args[3] : 1.0;
    int    nchannels  = n > 4 ? (int)(args[4] + 0.5) : 0;
    double drain_mix  = n > 5 ? args[5] : 0.5;   // drainage amplitude (UI: "Drainage")
    double ampl       = n > 6 ? args[6] : 1.0;   // erosion amplitude  (UI: "Erosion")
    int    orders     = n > 7 ? (int)(args[7] + 0.5) : 3;
    double drain_delf = n > 8 ? args[8] : 2.0;
    double falloff    = n > 9 ? args[9] : 0.5;

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

    // ── Image mode ────────────────────────────────────────────
     if (images.building()) {
    	buildImage();
        return;
    }

    // ── Elevation weight ───────────────────────────────────────
    double w_elev;
    if (base >= level + edge) {
        w_elev = 0.0;
    } else if (base >= level) {
        double t = (base - level) / edge;
        if (options & SS)
            w_elev = 1.0 - (t * t * (3.0 - 2.0 * t));
        else
            w_elev = 1.0 - t;
    } else {
        w_elev = 1.0;
    }
    if (options & SQR) w_elev = w_elev * w_elev;
    if (shape != 1.0)  w_elev = pow(w_elev, shape);

    // ── Drainage weight ────────────────────────────────────────
    // Voronoi cells on the unit sphere.
    // tn in [0,1], pn in [0,0.5] — phi halved to preserve circular aspect
    // ratio (theta spans 360 degrees, phi spans 180 degrees).
    // base_freq = 2^start → cell size = 2^-start of the unit sphere,
    // matching fractal's subdivision level convention exactly.
    // start=12 → cells at LOD-12 scale.
    double w_drain = 0.0;
    if (nchannels > 0 && drain_mix > 0.0) {
        const double VMAX = 0.87;

        // t_cliff: position in cliff zone [0=base, 1=top]
        // Extended slightly beyond [0,1] for smooth envelope falloff
        double t_cliff;
        if (base >= level + edge)
            t_cliff = 1.0 + (base - (level + edge)) / edge;
        else if (base >= level)
            t_cliff = (base - level) / edge;
        else
            t_cliff = -(level - base) / edge;

        // Envelope: bell curve peaking near the cliff base (natural drainage
        // carves deepest near the bottom where water has most energy).
        // Fades smoothly 50% of edge-width above and below the cliff zone.
        double env = 0.0;
        if (t_cliff > -0.5 && t_cliff < 1.5) {
            double tc = (t_cliff + 0.5) / 2.0;  // remap to [0,1]
            const double lo = 0.175;             // peak at ~25% up cliff (near base)
            double u = (tc <= lo)
                ? tc / lo
                : 1.0 - (tc - lo) / (1.0 - lo);
            u = u < 0.0 ? 0.0 : u > 1.0 ? 1.0 : u;
            env = u * u * (3.0 - 2.0 * u);
        }

        // Unit sphere coords — phi halved for circular cells
        double tn = Theta / 360.0;
        double pn = (Phi + 90.0) / 360.0;

        // Clamp t_cliff to [0,1] for octave height weighting
        double tc_w = t_cliff < 0.0 ? 0.0 : t_cliff > 1.0 ? 1.0 : t_cliff;

        // Multi-octave Voronoi:
        // Multi-octave Voronoi — amplitude model matches texture overlays:
        // oct=0: amplitude = ampl,            freq = 2^start
        // oct=1: amplitude = ampl * falloff,  freq = 2^start * drain_delf
        // oct=2: amplitude = ampl * falloff^2, freq = 2^start * drain_delf^2
        // falloff=1: all octaves equal; falloff=0.5: each half previous; falloff=0: first only
        double vsum = 0.0, vtotal = 0.0;
        double f = pow(2.0, (double)nchannels);
        double oct_amp = 1.0;
        for (int oct = 0; oct < orders; oct++) {
            double peak = (orders > 1) ? double(oct) / double(orders - 1) : 0.5;
            double dist = fabs(tc_w - peak);
            double hw = 1.0 - dist * 1.5;
            hw = hw < 0.0 ? 0.0 : hw;
            hw = hw * hw * (3.0 - 2.0 * hw);

            double pnt[3] = { tn * f, pn * f, double(oct) * 3.7 };
            double v;
            if (options & NEG) {
                // Gully mode: smoothed absolute value of gradient noise.
                // |Perlin| gives narrow ridge crests at zero-crossings with
                // broad smooth basins between — watershed divides over gully floors.
                // smooth_r rounds the sharp V at each crest (like noise Smooth param).
                double n = Noise::Noise3D(pnt);
                double a = fabs(n);
                const double smooth_r = 0.15;
                if (a < smooth_r) {
                    double t = a / smooth_r;
                    a = smooth_r * 0.5 * t * t * (3.0 - t);
                }
                v = 1.0 - a;
                v = v * v;  // sharpen: concentrate carving near ridge crests
            } else {
                // Cell mode: standard Voronoi dome
                const double VMAX = 0.87;
                double d = Noise::Voronoi3D(pnt) + 0.5;
                d = d < 0.0 ? 0.0 : d;
                v = 1.0 - (d / VMAX);
            }
            v = v < 0.0 ? 0.0 : v > 1.0 ? 1.0 : v;
            vsum   += oct_amp * hw * v;
            vtotal += oct_amp * hw;
            f       *= drain_delf;
            oct_amp *= falloff;
        }

        double dc = (vtotal > 0.0) ? vsum / vtotal : 0.0;
        dc = dc * dc * (3.0 - 2.0 * dc);
        w_drain = dc * env;
    }

    // ── Combine ────────────────────────────────────────────────
    // Erosion carves the cliff face by depth * ampl * w_elev.
    // Drainage modulates within that zone: channels carve deeper,
    // ridges carve less — but never below the full erosion depth.
    // Combined weight blends between w_elev alone and w_elev*w_drain
    // based on drain_mix amplitude.  Result stays in [0, depth*ampl].
    double w_combined = w_elev * (ampl + drain_mix * 0.25 * w_drain);
    double z = base - depth * w_combined;

    if (S0.pvalid()) S0.p.z = z;
    else             S0.s   = z;

    Td.sediment = -(base - z);  // total erosion depth (negative = removed material)
}

//-------------------------------------------------------------
// TNerode::buildImage() used for image generation
//-------------------------------------------------------------
// Uses TheNoise.get_point() 4D torus coords for seamless TILE tiling,
// matching the same mechanism used by noise() and craters() in image mode.
// Voronoi4D / Noise4D are used so all four torus components contribute,
// eliminating the banding that flat 3D coords produce.
// Modulates the downstream (right) node value eval'd before this call.
//-------------------------------------------------------------
void TNerode::buildImage(){
	// Capture downstream value BEFORE INIT clears S0.s
	double base = S0.s;
	INIT;
	double args[10];
	TNarg *arg = (TNarg*) left;
	int n = getargs(arg, args, 10);

	double depth      = n > 0 ? args[0] : 0.3;
	int    nchannels  = n > 4 ? (int)(args[4] + 0.5) : 0;
	double drain_mix  = n > 5 ? args[5] : 0.5;
	double ampl       = n > 6 ? args[6] : 1.0;
	int    orders     = n > 7 ? (int)(args[7] + 0.5) : 3;
	double drain_delf = n > 8 ? args[8] : 2.0;
	double falloff    = n > 9 ? args[9] : 0.5;

	// 4D torus point set by image builder for seamless tiling
	Point4D pt = TheNoise.get_point();
	double f = pow(2.0, (double)nchannels);
	double vsum = 0.0, vtotal = 0.0, oct_amp = 1.0;
	const double VMAX = 0.87;

	for (int oct = 0; oct < orders; oct++) {
		// Scale all 4 torus components — octave seed via w offset
		double pnt[4] = { pt.x*f, pt.y*f, pt.z*f, pt.w*f + oct*0.37 };
		double v;
		if (options & NEG) {
			// Gully mode: smoothed |Noise4D|
			double nn = Noise::Noise4D(pnt);
			double a = fabs(nn);
			const double smooth_r = 0.15;
			if (a < smooth_r) { double t = a/smooth_r; a = smooth_r*0.5*t*t*(3.0-t); }
			v = 1.0 - a;
			v = v * v;
		} else {
			// Cell mode: Voronoi4D dome
			double d = Noise::Voronoi4D(pnt) + 0.5;
			d = d < 0.0 ? 0.0 : d;
			v = 1.0 - (d / VMAX);
		}
		v = v < 0.0 ? 0.0 : v > 1.0 ? 1.0 : v;
		vsum   += oct_amp * v;
		vtotal += oct_amp;
		f      *= drain_delf;
		oct_amp *= falloff;
	}

	double dc = (vtotal > 0.0) ? vsum / vtotal : 0.0;
	dc = dc * dc * (3.0 - 2.0 * dc);
	double erode_val = depth * (ampl * (1.0 - dc) + drain_mix * dc);

	// Modulate downstream node value (captured before INIT)
	S0.s = (base != 0.0) ? erode_val * base : erode_val;
}
