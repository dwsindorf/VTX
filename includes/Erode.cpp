
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

extern int hits,misses,visits;
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
	S0.hardness=0;
	if(arg){
	    arg->eval();
		S0.hardness=S0.s;
	}
	INIT;
	if(right){
		right->eval();
	}
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
// TNerode::eval() evaluate the node
//-------------------------------------------------------------
void TNerode::eval() {
    INIT;

    TNarg *arg = (TNarg*) left;
    double args[11];
    int n = getargs(arg, args, 10);
    Td.hardness = 0.5;
     Td.sediment = 0;
     Td.erosion = 1;
     Td.set_flag(EVALUE);

    if(!isEnabled() || CurrentScope->rpass()){
    	if(right)
    		right->eval();
    	return;
    }

    int idx=0;
    unsigned int begin=30;
   // unsigned int begin = (unsigned int) (2.0 * args[idx++]);
  //  unsigned int orders = (unsigned int) (2.0 * args[idx++]);
   // unsigned int begin=1;
    double fill_ampl = n > idx ? args[idx++] : 1.0;
    double transport = n > idx ? args[idx++] : 0.0;
    double upper = n > idx ? args[idx++] : 0.2; // upper height threshold
    double dt = n > idx ? args[idx++] : 0.1; // lower threshold
    double lower=upper-dt;

    double slope_ampl = n > idx ? args[idx++] : 1.0;
    double slope_drop = n > idx ? args[idx++] : 0.1; // slope drop
    double slope_min = n > idx ? args[idx++] : 0.25; // min slope
    double ds = n > idx ? args[idx++] : 3; // lower threshold
    double slope_max = slope_min+ds; // max slope

    double ave=0;
    if(transport>0)
        ave=transport*ave_sediment();
    if (right) {
        right->eval();
        //Td.p.z= S0.pvalid()?S0.p.z:S0.s;
        Td.rock = S0.pvalid()?S0.p.z:S0.s;
    }
    //double z = Td.p.z;//+ave;
   double z = Td.rock;//+ave;

    int l1 = begin;
//    int l2=begin+orders;
//    l1=l1>MAXLVLS?MAXLVLS:l1;
//    l2=l2>MAXLVLS?MAXLVLS:l2;
    int level = (int) Td.level;

    double drop=0;
    double sed = 0;//ave;
    double s=0;
    slope_min/=TheMap->hscale;
    slope_max/=TheMap->hscale;
   // if (level >= l1 /*&& level<l2*/) {
    	//if(level<l2){
       // CELLSLOPE(Ht(),s);
        CELLSLOPE(solid(),s);
       s*=TheMap->hscale;//*INV2PI;

         if (options & SQR)
             s = s * s;
         if (options & SS)
             drop=slope_ampl*smoothstep(slope_min, slope_max, s,0,slope_drop);
         else
             drop=slope_ampl*rampstep(slope_min, slope_max, s,0,slope_drop);
        drop*=TheMap->hscale;
       sed-=drop;
        //double ave = ave_sediment();
        //sed = ave;

        //double absz = fabs(z);
        //z+=drop;

       // if (thresh < lim) {
       // if (z < upper){
        	//sed=fill_ampl*(upper-z);
            sed+=fill_ampl*rampstep(lower, upper, z,lower-z, 0);
        //}
        //else
        //    sed = 0;
 

//            if (z - drop < thresh)
//                sed = thresh - drop;
//            else
//                sed = -drop;
//        } else {
//            drop = rampstep(0, absz, drop, drop, absz);
//            sed = -drop;
//        }
//		double delta=0.5*(sed-ave);
//		for(int i=0;i<mdcnt;i++){
//			mapdata[i]->setSediment(NBSED(i)+delta/mdcnt);
//			mapdata[i]->invalidate();
//		}
//		sed=sed-delta;
    	//}
        z += sed;
//        drop=s;

   //}
   if (S0.pvalid())
       S0.p.z=z;
   else
       S0.s=z;
    Td.sediment = sed;

}

