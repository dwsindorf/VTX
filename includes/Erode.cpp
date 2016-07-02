
#include "NodeData.h"
#include "Erode.h"

//**************** extern API area ************************
extern NameList<LongSym*> NOpts;

extern int hits,misses,visits;
static 			TerrainData Td;

//**************** static and private *********************

#define NBSED(n) mapdata[n]->sediment()
#define smallest(x,y) x<y?x:y
#define largest(x,y)  x>y?x:y

extern double   ptable[];

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
	s1=mdctr->span(mapdata[0]);
	s2=mdctr->span(mapdata[1]);
	f1=s2/(s1+s2);
	if(mdcnt==4){
		s3=mdctr->span(mapdata[2]);
		s4=mdctr->span(mapdata[3]);
		f2=s4/(s3+s4);
		lastz=0.5*(f1*NBSED(0)+(1-f1)*NBSED(1)+f2*NBSED(2)+(1-f2)*NBSED(3));
	}
	else
		lastz=f1*NBSED(0)+(1-f1)*NBSED(1);
	return lastz;
}

double eslope()
{
    double s=0;
    CELLSLOPE(rock(),s);
    return TheMap->hscale*s*INV2PI;
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
// TNerode::eval() evaluate the node
//-------------------------------------------------------------
void TNerode::eval()
{        
	Td.hardness=0.5;
	Td.sediment=0;
	Td.erosion=1;

  	TNarg *arg=(TNarg*)left;
	double args[8];
	int	n=getargs(arg,args,8);
	INIT;

	if(right){
		right->eval();
	    Td.rock=S0.pvalid()?S0.p.z:S0.s;
	}
    double z=Td.rock;
    
	unsigned int begin=(unsigned int)(2.0*args[0]);
	double ampl=n>2?args[2]:0.0;
	double sbias=n>3?args[3]:1.0;
	double gmax=n>4?args[4]:3;
	double thresh=n>5?args[5]:lim; // turn on threshold

	int l1=begin;
	int level=(int)Td.level;

   	Td.set_flag(EVALUE);
	double sed=0;
	
	if(level>=l1){
		double ave=ave_sediment();
		sed=ave;
		
		//if(level<l2){
		    double absz=fabs(z);
		    double slope=eslope();
			//if(options & NSQR)
			//	slope=slope*slope;
			double grad=rampstep(0,gmax,sbias*slope);
			double drop=ampl*grad;

			if(thresh < lim){
			    if(z-drop<thresh)
					sed=thresh-z;
			    else
					sed=-drop;
			}
			else{
				drop=rampstep(0,absz,drop,drop,absz);
			    sed=-drop;
			}
			double delta=0.5*(sed-ave);
			for(int i=0;i<mdcnt;i++){
				mapdata[i]->setSediment(NBSED(i)+delta/mdcnt);
				mapdata[i]->invalidate();		    
			}
			sed=sed-delta;
		//}
	}
	Td.sediment=sed;
}

