#include "SceneClass.h"
#include "MapNode.h"
#include "NoiseClass.h"
#include "Fractal.h"
#include "AdaptOptions.h"

//**************** extern API area ************************

extern Map     *TheMap;
extern Point    MapPt;
//extern int hits,misses,visits;

extern NameList<LongSym*> NOpts;

#define smallest(x,y) x<y?x:y
#define largest(x,y)  x>y?x:y

//double last_fractal=0;

static 		TerrainData Td;
const unsigned int 	MAXLVLS=63;

//**************** static and private *********************

#define NBRHT(n) mapdata[n]->fractal(chnl)

extern double   ptable[],Hscale;

//************************************************************
// TNfractal class
//************************************************************
//************************************************************
// TNfractal class
//************************************************************
TNfractal::TNfractal(int t, TNode *l, TNode *r) : TNfunc(l,r)
{
    options=t;
    chnl=0;
}

void TNfractal::init()
{
	if(isEnabled()){
		Td.add_fractal(this);
	}
	TNfunc::init();

}
//-------------------------------------------------------------
// TNfractal::eval() evaluate the node
//-------------------------------------------------------------
//  options:    optional feature
//  SQR         square slope to exaggerate slope bias
//  SS          smoothstep base to fractal switch point (3 orders)
//  V2		    use v1,v0 diagonal (2 points)
//  V4		    use 4 corners (4 point diamond-squares)
//  VS		    use two corners but randomly switch diagonal (thresh)
//              if Random>thresh use ave(v1,v0) else use ave(v3,v2)
//  VR		    random switch between VS and V4 modes (thresh)
//              if abs(Random)>thresh mode=V4 else mode=VS
//
// n:    number of variable arguments
// args: name   default
//	0	 begin	0     cell level to begin fractal
//	1	 orders	0     cell level to stop fractal = begin+orders
//	2	 ampl	0.0   magn of random perturbation to apply
//	3	 sbias	0.0   slope bias (exaggerates areas of high slope)
//	4	 smax	4.0   slope max (truncates max slope used for bias)
//	5	 sdrop	0.0   slope drop (exaggerates steepness)
//	6	 thresh	1.0   multiply by tbias if slope < than thresh
//	7	 tbias	1.0   low slope multiplier (damping factor)
//	8	 hmax	1.0   ht(fractional) for mode switch
//	9	 hval	1.0   multiplier to apply if ht > hmax
//
//-------------------------------------------------------------
void TNfractal::eval()
{
	//if(!isEnabled() /*|| Td.get_flag(FVALUE)*/){
	if(!isEnabled() || chnl>3){
		if(right)
			right->eval();
		return;
	}
    
 	Td.clr_flag(FVALUE);
	static int init=0;
	static double facts[64];
	double margin_scale=1;
	if(!init){
		int i;
		for(i=0;i<63;i++){
			facts[i]=smoothstep(0,25,i,1,0.025);
		}
		init=1;
	}
	int n=0;
	double base=0;
	double args[20];

  	TNarg *arg=(TNarg*)left;

	if(arg)
		n=getargs(arg,args,20);

	INIT;
	if(right){
	   	Td.set_flag(FVALUE);

		right->eval();
		if(S0.pvalid())
			base=S0.p.z;
		else
			base=S0.s;
	}
	//if(S0.get_flag(INMARGIN))
	//	margin_scale=0.1;
	//if(Td.texht)
	//	base+=Td.texht;

	double slope,delta,t,f,drop=0;
	double t1,t2;
	unsigned int l1,l2;
	double rand1=0;
	double rand2=0;
	double rand3=0;
	int mdopt=0;
	double sfact=1;

	unsigned int begin=(unsigned int)(2.0*args[0]);
	unsigned int orders=(unsigned int)(2.0*args[1]);
	double ampl=n>2?args[2]:0.1;
	double sbias=n>3?args[3]:0.2;
	double smax=n>4?args[4]:2.0;
	double sdrop=n>5?args[5]:0.1;
	double thresh=n>6?args[6]:0.0;
	double tbias=n>7?args[7]:0.0;
	double hmax=n>8?args[8]:1.0;
	double hval=n>9?args[9]:0.0;

	double hscale=1000*TheMap->hscale;

	ampl/=hscale;
	smax/=hscale;
	bool inmargin=false;

	//cout<<Hscale<<endl;

	unsigned int level=(int)Td.level;

	l1=begin;
	l2=begin+orders;

	l1=l1>MAXLVLS?MAXLVLS:l1;
	l2=l2>MAXLVLS?MAXLVLS:l2;

	t1=CELLSIZE(l2);
	t2=CELLSIZE(l1)-t1;

	if(level>=l1){
		if(level<l2){
			double rf=1<<(level/2);
			rand1=Random(MapPt.x*rf,MapPt.y*rf)+0.5;
			rand2=RAND(1)+0.5;
			switch(options &FMODE){
			case V2:
				mdopt=0;
				break;
			case VR:
				rand3=2*RAND(2);
				if(rand3>0.5 || rand3<-0.5)
					mdopt=rand2>0?1:0;
				else
					mdopt=2;
				break;
			case V4:
		    	mdopt=2;
				break;
			default:
				rand3=2*RAND(2);
				if(rand3>0)
					mdopt=1;
				else
					mdopt=0;

				rand3=0.5+RAND(3);
				//if(rand3>thresh)
				if(rand3>1.0)
					Td.set_flag(FNOREC);
				break;
			}
			if(mdopt==1 && mdcnt<4)
				mdopt=0;
		}

		double fractal_ave=0;
		double fractal_slope=0;

		if(mdcnt>=2){ // calculate average ht in cell
			double f1=0.5,f2=0.5;
			double s1,s2,s3,s4;
			s1=mdctr->span(mapdata[0]);
			s2=mdctr->span(mapdata[1]);
			f1=s2/(s1+s2);
			if(mdcnt==4){
				s3=mdctr->span(mapdata[2]);
				s4=mdctr->span(mapdata[3]);
				f2=s4/(s3+s4);
			}
			switch(mdopt){
			case 1:
				if(mdcnt==4){
					fractal_ave=f2*NBRHT(2)+(1-f2)*NBRHT(3);
					break;
				}
			case 0:
				fractal_ave=f1*NBRHT(0)+(1-f1)*NBRHT(1);
				break;
			case 2:
				if(mdcnt==4)
					fractal_ave=0.5*(f1*NBRHT(0)+(1-f1)*NBRHT(1)+f2*NBRHT(2)+(1-f2)*NBRHT(3));
				else
					fractal_ave=f1*NBRHT(0)+(1-f1)*NBRHT(1);
				break;
			}
			if(sbias){ // calculate slope
				switch(mdopt){
				case 1:
					if(mdcnt==4){
						fractal_slope=fabs(NBRHT(2)-NBRHT(3))/mapdata[2]->span(mapdata[3]);
						break;
					}
				case 0:
					fractal_slope=fabs(NBRHT(0)-NBRHT(1))/mapdata[0]->span(mapdata[1]);
					break;
				case 2:
					CELLSLOPE(fractal(chnl),fractal_slope);
					break;
				}
				fractal_slope*=margin_scale*TheMap->hscale;
			}
		}
		f=fractal_ave;
	
		if(level<l2){
			t=(CELLSIZE(level)-t1)/t2;
			if(options & SS)
				sfact=facts[level-l1];
			slope=sfact*fractal_slope;
			if(options & SQR)
				slope=slope*slope;
			slope=smallest(slope,smax);

			if(slope<=(thresh*smax))
				slope*=tbias;
            if(TheMap->hmax>TheMap->hmin){
				double fht=(base-TheMap->hmin)/(TheMap->hmax-TheMap->hmin);
				slope*=rampstep(fht,0,hmax,1,hval);
            }

			delta=sfact*ampl*rand1*slope*sbias;
			if(sdrop)
				drop=sfact*rand2*sdrop*slope*sbias;
			f+=t*(delta-drop);
		}
	}
	else {
		f=base;
	}
   	Td.set_flag(FVALUE);
	if(options & NEG)
	    f=-f;
	Td.fracval[chnl]=f;
	
	if(S0.pvalid())
		S0.p.z=f;
	else
		S0.s=f;
}

//-------------------------------------------------------------
// TNfractal::save() evaluate the node
//-------------------------------------------------------------
void TNfractal::save(FILE *f)
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
// TNfunc::valueString() write value string
//-------------------------------------------------------------
void TNfractal::valueString(char *s)
{
    TNfunc *func=expr?expr:this;
	setStart(s);
    if(CurrentScope->tokens() && token[0])
        strcat(s,token);
    else{
		func->propertyString(s);
	}
	setEnd(s);
	if(right)
		right->valueString(s);
}

//-------------------------------------------------------------
// TNfractal::optionString() get option string
//-------------------------------------------------------------
void TNfractal::optionString(char *c)
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
// TNfractal::propertyString() get property string
//-------------------------------------------------------------
void TNfractal::propertyString(char *s)
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
void TNfractal::applyExpr()
{
    if(expr){
    	options=((TNfractal*)expr)->options;
        DFREE(left);
        left=expr->left;
        left->setParent(this);
        expr=0;
    }
    if(right)
        right->applyExpr();
}
