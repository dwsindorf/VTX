
#include "TerrainData.h"
#include "TerrainClass.h"
#include "ImageClass.h"
#include "ImageMgr.h"
#include "TextureClass.h"
#include "MapNode.h"
#include "AdaptOptions.h"
#include "Effects.h"
#include "GLSLMgr.h"
#include "GLSLVars.h"
#include "RenderOptions.h"

#define DEBUG_LOD          // turn on to get lod info

//#define DEBUG_NOISE  // show noise normalization activity

int tnoise_visits=0;
int tnoise_hits=0;
int tnoise_misses=0;

// notes:
// Timing (330 MHz Ultra10) per point
//    2.5 us    terrain tree traverse (earth)
//    1.0 us    MapPt calculation (spherical->rectangular)
//    9.7 us    noise calculation (noise3)

extern double Rand();
extern double zslope();
extern double eslope();
extern void inc_tabs();
extern void dec_tabs();

extern double Theta, Phi, Height, Drop, Margin,Impact,Radius,Density,MaxHt,MinHt,FHt;
extern char   tabs[];
extern double Hscale,Rscale;

static double  Mheight;

double Red=0, Green=0, Blue=0, Alpha=1;
double Pz=0, Px=0, Py=0;
int addtabs=0;

char tmps[256];

// noise symbols

static LongSym ntypes[]={
	{"VORONOI",	    VORONOI},
	{"GRADIENT",	GRADIENT},
	{"SIMPLEX",	    SIMPLEX},
};
NameList<LongSym*> NTypes(ntypes,sizeof(ntypes)/sizeof(LongSym));

static LongSym nopts[]={
	{"UNS",			UNS},
	{"NOREC",		NOREC},
	{"SQR",		    SQR},
	{"NEG",		    NEG},
	{"NABS",		NABS},
	{"SS2",			SS2},
	{"RO1",		    RO1},
	{"RO2",		    RO2},
	{"RO3",		    RO3},
	{"RO4",		    RO4},
	{"RO5",		    RO5},
	{"FS",		    FS},
	{"BP",		    BP},
	{"SCALE",	    SCALE},
	{"NLOD",	    NLOD},
	{"NNORM",	    NNORM},
	{"SS",		    SS},
	{"V2",		    V2},
	{"V4",		    V4},
	{"VR",		    VR}
};
NameList<LongSym*> NOpts(nopts,sizeof(nopts)/sizeof(LongSym));

extern double Pscale,Gscale,Radius,MaxSize;
extern int hits,visits,misses;

static TerrainData Td;

#define INITORDER   12
#define HIGHORDERS   5
#define TOPORDER    INITORDER+HIGHORDERS

#define WATER_COLOR	Color(0.0,1.0,1.0)

static double SeaLevel=0,WaterDepth=0,WaterHeight=0;

// globals symbols

enum  {	 // NOTE: must keep same order as in gtypes[] below
	ZHT,
	MAXHT,
	MINHT,
	FHT,
	BMPHT,
	THETA,
	PHI,
	LAT,
	EQU,
	SLOPE,
	RADIUS,
	DENSITY,
	RED,
	GREEN,
	BLUE,
	ALPHA,
	INTENSITY,
	PX,
	PY,
	PZ,
	SEALEVEL,
	WDEPTH,
	WHT,
	MHT,
	GPI,
	GRPD,
	GDPR,
	RAND,
	LVL,
	GTOL,
	LTOG,
	MAXSIZE,
	IMPACT,
	ESLOPE,
	SED,
	DROP,
	MARGIN,
	ZBASE,
	DEPTH,
	PILLOW,
	CIRRUS,
	CUMULUS,
	CTYPES,
};
enum {		// CTypes cloud options
};

static LongSym gtypes[]={
	{"HT",			ZHT},
	{"MXHT",		MAXHT},
	{"MNHT",		MINHT},
	{"FHT",		    FHT},
	{"BMPHT",		BMPHT},
	{"THETA",		THETA},
	{"PHI",			PHI},
	{"LAT",			LAT},
	{"EQU",			EQU},
	{"SLOPE",		SLOPE},
	{"RADIUS",		RADIUS},
	{"DENSITY",		DENSITY},
	{"RED",			RED},
	{"GREEN",		GREEN},
	{"BLUE",		BLUE},
	{"ALPHA",		ALPHA},
	{"INTENSITY",	INTENSITY},
	{"PX",			PX},
	{"PY",			PY},
	{"PZ",			PZ},
	{"SEALEVEL",	SEALEVEL},
	{"WDEPTH",		WDEPTH},
	{"WHT",			WHT},
	{"MHT",			MHT},
	{"PI",			GPI},
	{"RPD",			GRPD},
	{"DPR",			GDPR},
	{"RAND",		RAND},
	{"LVL",			LVL},
	{"GTOL",		GTOL},
	{"LTOG",		LTOG},
	{"MAXSIZE",		MAXSIZE},
	{"IMPACT",		IMPACT},
	{"ESLOPE",		ESLOPE},
	{"SED",			SED},
	{"DROP",		DROP},
	{"MARGIN",		MARGIN},
	{"BASE",		ZBASE},
	{"DEPTH",       DEPTH},
	{"PILLOW",      PILLOW},
	{"CIRRUS",		CIRRUS},
	{"CUMULUS",		CUMULUS},
	{"CTYPES",      CTYPES}
};
NameList<LongSym*> NGlobals(gtypes,sizeof(gtypes)/sizeof(LongSym));

//~~~~~~~~~~~~~~~~~ terminal nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~

//************************************************************
// TNstring class
//************************************************************
TNstring::~TNstring()
{
	FREE(value);
	value=0;
}
void TNstring::eval()
{
	INIT;
	S0.string=value;
	S0.set_strvalid();
}
void TNstring::save(FILE *f)
{
	fprintf(f,"\"%s\"",value);
}
void TNstring::valueString(char *s)
{
	setStart(s);
	sprintf(s+start,"\"%s\"",value);
	setEnd(s);
}

//************************************************************
// TNconst class
//************************************************************
void TNconst::eval()
{
	INIT;
	S0.s=value;
	S0.set_svalid();
	S0.set_constant();
	//if(CurrentScope->shader())
	//	S0.set_inactive();
}
void TNconst::valueString(char *s){
	setStart(s);
	sprintf(s+start,"%g",value);
	setEnd(s);
}

void TNconst::save(FILE *f)
{
	fprintf(f,"%g",value);
}

//************************************************************
// TNglobal class
//************************************************************
void TNglobal::init()
{
	switch(gtype){
	case FHT:
		Td.set_flag(HT2PASS);
		break;
	case ZHT:
		//if(!Render.draw_shaded())
			Td.set_flag(HT2PASS);
		break;
	}
}
void TNglobal::eval()
{
	INIT;
	switch(gtype){
	case MAXHT:
		S0.s=MaxHt;
		break;
	case MINHT:
		S0.s=MinHt;
		break;
	case FHT:
		S0.s=FHt=(Height-MinHt)/(MaxHt-MinHt);
		S0.s=clamp(S0.s,0,1);
		break;
	case ZHT:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=Height;
		break;
	case BMPHT:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=0;
		break;
	case PZ:
		S0.s=Pz;
		break;
	case PX:
		S0.s=Px;
		break;
	case PY:
		S0.s=Py;
		break;
	case SEALEVEL:
		S0.s=SeaLevel;
		break;
	case WDEPTH:
		S0.s=WaterDepth;
		break;
	case WHT:
		S0.s=WaterHeight;
		break;
	case MHT:
		S0.s=Mheight;
		break;
	case THETA:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=Theta/180-1;
		break;
	case PHI:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=Phi/180;
		break;
	case LAT:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=2*fabs(Phi/180);
		S0.s=clamp(S0.s,0,1);
		break;
	case EQU:
		S0.s=2*fabs(Phi/180);
		S0.s=clamp(1-S0.s,0,1);
		break;
	case RADIUS:
		S0.s=Radius;
		break;
	case DENSITY:
		//Td.set_flag(SNOISEFLAG);
		S0.s=Density;
		break;
	case GPI:
		S0.s=PI;
		break;
	case GRPD:
		S0.s=RPD;
		break;
	case GDPR:
		S0.s=DPR;
		break;
	case RAND:
		S0.s=RANDID;
		break;
	case LVL:
		S0.s=0.5*S0.level;
		break;
	case GTOL:
		S0.s=Gscale;
		break;
	case LTOG:
		S0.s=1/Gscale;
		break;
	case MAXSIZE:
		S0.s=MaxSize;
		break;
	case IMPACT:
		S0.s=Impact;
		break;
	case SED:
		S0.s=S0.sediment;
		break;
	case SLOPE:
		if(CurrentScope->shader()){
			S0.set_inactive();
			S0.set_constant();
		}
		else
			S0.s=zslope();
		break;
	case ESLOPE:
		S0.s=eslope();
		break;
	case RED:
		S0.set_constant();
		S0.s=Red;
		break;
	case GREEN:
		S0.set_constant();
		S0.s=Green;
		break;
	case BLUE:
		S0.set_constant();
		S0.s=Blue;
		break;
	case ALPHA:
		S0.s=Alpha;
		break;
	case INTENSITY:
		S0.s=(Red+Blue+Green)/3.0;
		break;
	case DROP:
		S0.s=Drop;
		break;
	case MARGIN:
		S0.s=Margin;
		break;
	case ZBASE:
		S0.s=S0.height;
		break;
	case DEPTH:
		S0.s=Td.depth;
		break;
	case PILLOW:
		S0.s=1;
		break;
	case CIRRUS:
		S0.s=4;
		break;
	case CUMULUS:
		S0.s=10;
		break;
	case CTYPES:
		S0.s=16;
		break;
	}
	S0.set_svalid();
}

//-------------------------------------------------------------
// TNglobal::save() save the node
//-------------------------------------------------------------
void TNglobal::save(FILE *f)
{
	fprintf(f,"%s",gtypes[gtype].name());
}

//-------------------------------------------------------------
// TNglobal::valueString() node string
//-------------------------------------------------------------
void TNglobal::valueString(char *s)
{
	eval();
	if(S0.inactive())
		return;
	setStart(s);
	sprintf(s+strlen(s),"%s",gtypes[gtype].name());
	setEnd(s);
}

//~~~~~~~~~~~~~~~~~ unary nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//************************************************************
// TNcolor class
//************************************************************
//-------------------------------------------------------------
// TNcolor::init() init the node
//-------------------------------------------------------------
void TNcolor::init(){
	right->init();
	//Td.set_flag(HT2PASS);
}
const char *TNcolor::symbol(){
	return "Color";
}

//-------------------------------------------------------------
// TNcolor::initProgram() set shader defines
//-------------------------------------------------------------
bool TNcolor::initProgram(){

	//char nstr[256];
	char defs[256];
	char red[256]={0};
	char green[256]={0};
	char blue[256]={0};
	char alpha[256]={0};
	defs[0]=0;
	CurrentScope->set_shader(1);
	TNarg *arg=(TNarg*)right;
	Td.clr_flag(SNOISEFLAG);
	Td.set_flag(NOISE1D);
	arg->eval();
	arg->valueString(red); // red
	arg=arg->next();
	arg->eval();
	arg->valueString(green); // green
	arg=arg->next();
	arg->eval();
	arg->valueString(blue); // blue
	arg=arg->next();
	if(arg){
		arg->eval();
		arg->valueString(alpha); // alpha
	}

	if(!strlen(red))
		strcpy(red,"0.0");
	if(!strlen(green))
		strcpy(green,"0.0");
	if(!strlen(blue))
		strcpy(blue,"0.0");
	if(!strlen(alpha))
		strcpy(alpha,"1.0");

	sprintf(defs,"#define NCC vec4(RED=%s,GREEN=%s,BLUE=%s,%s)\n",
			red,green,blue,alpha);
	strcat(GLSLMgr::defString,defs);
	CurrentScope->set_shader(0);
	defs[0]=0;
	Td.clr_flag(NOISE1D);

	return true;
}
//-------------------------------------------------------------
// TNcolor::eval() evaluate the node
//-------------------------------------------------------------
void TNcolor::eval()
{
	if(!isEnabled()){
		if((TNarg*)left)
			((TNarg*)left)->eval();
		return;
	}
	INIT;
//    if(CurrentScope->zpass())
//        return;
 	TNarg *arg=(TNarg*)right;
    if(CurrentScope->rpass()){
    	Td.clr_flag(SNOISEFLAG);
    	//right->init();
    	arg->eval(); // red
    	arg=arg->next();
    	arg->eval(); // green
    	arg=arg->next();
    	arg->eval(); // blue
    	arg=arg->next();
    	if(arg)
    		arg->eval(); // alpha
    	if(Td.get_flag(SNOISEFLAG))
    		TerrainData::add_TNcolor(this);
    	else
    		S0.clr_cvalid();
    	TerrainData::tp->set_color(true);
    	Td.clr_flag(SNOISEFLAG);
    	return;
    }

	int cflag=1;
	Alpha=1;

	arg->eval();
	Red=S0.s;
	cflag=S0.constant()?cflag:0;
	arg=arg->next();
	arg->eval();
	cflag=S0.constant()?cflag:0;
	Green=S0.s;
	arg=arg->next();
	arg->eval();
	cflag=S0.constant()?cflag:0;
	Blue=S0.s;
	arg=arg->next();
	if(arg){
		arg->eval();
		Alpha=S0.s;
		cflag=S0.constant()?cflag:0;
	}
	S0.c=Color(Red,Green,Blue,Alpha);
	S0.set_cvalid();
	S0.clr_svalid();
	if(cflag)
	    S0.set_constant();
}

//-------------------------------------------------------------
// TNcolor::addToken() add to token list
//-------------------------------------------------------------
void TNcolor::addToken(LinkedList<TNode*>&l)
{
	if(right)
		right->addToken(l);
	//TNvector::addToken(l);
}

//-------------------------------------------------------------
// index operator
//-------------------------------------------------------------
TNode *TNcolor::operator[](int i)
{
	TNarg &arg=*((TNarg*)right);
	return arg[i];
}


//************************************************************
// Class TNclist
//************************************************************
TNclist::~TNclist()
{
	colors.free();
}

//-------------------------------------------------------------
// TNclist::init() initialize
//-------------------------------------------------------------
void TNclist::init()
{
	colors.free();
	TNarg *arg=(TNarg*)right;
	while(arg){
		arg->eval();
		colors.add(new Color(S0.c));
		arg=arg->next();
	}
}
//-------------------------------------------------------------
// TNclist::save() save the node
//-------------------------------------------------------------
void  TNclist::save(FILE *f)
{
	colors.ss();
	Color *c;
	fprintf(f,"%s(",symbol());
	inc_tabs();
	fprintf(f,"\n%s",tabs);

	int i=0;
	while((c=colors++)>0){
		char tmp[64];
		c->toString(tmp);
		fprintf(f,"%s",tmp);
		i++;
		if(size()>1 && i<size())
			fprintf(f,",");
		fprintf(f,"\n%s",tabs);
	}
	fprintf(f,")");
	dec_tabs();
}

//-------------------------------------------------------------
// TNclist::set_color() set color at index (i)
//-------------------------------------------------------------
void  TNclist::set_color(int i,Color c)
{
	if(i>=0 && i <colors.size){
		delete colors.base[i];
		colors.base[i]=new Color(c);
	}
}

//-------------------------------------------------------------
// TNclist::get_color() get color at index (i)
//-------------------------------------------------------------
Color  TNclist::get_color(int i)
{
	Color c;
	c=*colors[i];
	return c;
}

//-------------------------------------------------------------
// TNclist::color() interpolate color
//-------------------------------------------------------------
Color  TNclist::color(double s)
{
	Color c;

    int n1,n2,m=colors.size-1;
    s=clamp(s,0,1);
    n1=(int)(s*m);

    if(n1>=m)
        return *colors.base[m];
    n2=n1+1;

    double f=s*m-n1;

    return colors.base[n1]->blend(*colors.base[n2],f);
}

//************************************************************
// TNdensity class
//************************************************************
//-------------------------------------------------------------
// TNdensity::eval() evaluate the node
//-------------------------------------------------------------
void TNdensity::eval()
{
	if(CurrentScope->zpass()){
		TNvector::eval();
		S0.density=S0.s;
	}
}

//************************************************************
// TNpoint class
//************************************************************
//-------------------------------------------------------------
// TNpoint::eval() evaluate the node
//-------------------------------------------------------------
void TNpoint::eval()
{
	INIT;
    Px=Py=Pz=0;
	Td.clr_flag(SNOISEFLAG|GNOISEFLAG);

	TNarg *arg=(TNarg*)right;
    if(CurrentScope->rpass()){
    	snoise=0;
    	Td.clr_flag(SNOISEFLAG);
    }
    if(arg){
		arg->eval();
		if(!arg->next())
			Pz=S0.s;
		else{
			Px=S0.s;
			arg=arg->next();
			if(arg){
				arg->eval();
				Py=S0.s;
				arg=arg->next();
				if(arg){
					arg->eval();
					Pz=S0.s;
				}
			}
		}
    }
	if(CurrentScope->rpass() && Td.get_flag(SNOISEFLAG)){
		TerrainProperties *tp=TerrainData::tp;
		TerrainData::add_TNpoint(this);
		snoise=tp->noise.at();
		snoise->type|=GEOM;
	}
	if(CurrentScope->eval_mode() && Td.get_flag(SNOISEFLAG)){
		Td.set_flag(GNOISEFLAG);
	}
	S0.p=Point(Px,Py,Pz);
	S0.set_pvalid();
	S0.clr_svalid();
}

//-------------------------------------------------------------
// TNpoint::setProgram() set shader uniform variables
// - called AFTER shader program is created
//-------------------------------------------------------------
bool TNpoint::setProgram(){
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;
	if(!snoise)
		return false;
	double scale=snoise->scale;
	double bumpmin=1e-5;
	double minscale=32;
	minscale=minscale<scale?minscale:scale;
	double bumpdelta=pow(10,-(0.25*minscale+4-2*TheScene->bump_mip));
	bumpdelta=bumpdelta<1e-9?1e-9:bumpdelta;

	//cout << "scale:"<< snoise->scale<< " delta:"<< bumpdelta << endl;

	GLSLVarMgr vars;
	vars.newFloatVar("bump_delta",bumpdelta);
	//vars.newFloatVar("bump_delta",1e-7);
	vars.newFloatVar("bump_ampl",2e-3);

	//cout << " delta:"<<bumpdelta<<endl;

	vars.setProgram(program);
	vars.loadVars();
	return true;
}

//-------------------------------------------------------------
// TNpoint::initProgram() set shader defines
//-------------------------------------------------------------
bool TNpoint::initProgram(){
	//char nstr[256];
	char defs[256];
	char x[256]={0};
	char y[256]={0};
	char z[256]={0};
	defs[0]=0;
	CurrentScope->set_shader(1);
	TNarg *arg=(TNarg*)right;
	Td.clr_flag(SNOISEFLAG);
	if(!arg->next()){
		arg->eval();
		arg->valueString(z); // z
		sprintf(defs,"#define NPZ %s\n",z);
	}
	else{
		arg->eval();
		arg->valueString(x); // x
		arg=arg->next();
		arg->eval();
		arg->valueString(y); // y
		arg=arg->next();
		arg->eval();
		arg->valueString(z); // z
		if(!strlen(x))
			strcpy(x,"0.0");
		if(!strlen(y))
			strcpy(y,"0.0");
		if(!strlen(z))
			strcpy(z,"0.0");

		sprintf(defs,"#define NPNT vec3(PX=%s,PY=%s,PZ=%s)\n",x,y,z);
	}

	strcat(GLSLMgr::defString,defs);
	CurrentScope->set_shader(0);
	return true;
}

//-------------------------------------------------------------
// TNpoint::symbol() return node symbol
//-------------------------------------------------------------
const char *TNpoint::symbol()
{
	TNarg *arg=(TNarg*)right;
	if(arg && arg->right)
		return "Point";
	return "Z";
}

//-------------------------------------------------------------
// index operator
//-------------------------------------------------------------
TNode *TNpoint::operator[](int i)
{
	TNarg &arg=*((TNarg*)right);
	return arg[i];
}

//-------------------------------------------------------------
// TNpoint::valueString() value string
//-------------------------------------------------------------
void TNpoint::valueString(char *s)
{
	//setStart(s);
	TNvector::valueString(s+strlen(s));
	//setEnd(s);
}

//-------------------------------------------------------------
// TNpoint::addToken() add to token list
//-------------------------------------------------------------
void TNpoint::addToken(LinkedList<TNode*>&l)
{
	//l.add(this); // if F.E. has dialog for this
	//TNvector::addToken(l);
	if(right)
		right->addToken(l);
}

//************************************************************
// Class TNzbase
//************************************************************

//-------------------------------------------------------------
// TNzbase::eval() evaluate the node
//-------------------------------------------------------------
void TNzbase::eval()
{
	right->eval();
	S0.height=S0.s;
	S0.s=0;
	S0.clr_svalid();
	S0.p.z=S0.height;
	S0.set_pvalid();
}
//************************************************************
// TNgloss class
//************************************************************
//-------------------------------------------------------------
// TNgloss::eval() evaluate the node
//-------------------------------------------------------------
void TNgloss::eval()
{
	INIT;
  	TNarg *arg=(TNarg*)right;
	if(arg){
	    arg->eval();
		S0.albedo=S0.s;
		arg=arg->next();
		if(arg){
			arg->eval();
			S0.shine=S0.s;
			arg=arg->next();
			if(arg){
				SINIT;
				arg->eval();
				if(S0.cvalid())
					S0.emission=S0.c;
				S0.clr_cvalid();
			}
		}
	}
	S0.clr_svalid();
}

//************************************************************
// TNnoise class
//************************************************************
TNnoise::TNnoise(int t, TNode *a) : TNvector(a)
{
	type=t;
	mx=-lim;
	mn=lim;
	ma=1.0;
	mb=0;
	id=-1;
	scale=1.0;

	if(t & NNORM)
	    flags=0;
	else
	    flags=TNN_NORM;
}


//-------------------------------------------------------------
// TNnoise::addToken() add to token list
//-------------------------------------------------------------
void TNnoise::addToken(LinkedList<TNode*>&l)
{
	l.add(this); // if F.E. has dialog for this
}

//-------------------------------------------------------------
// TNnoise::save() evaluate the node
//-------------------------------------------------------------
void TNnoise::save(FILE *f)
{
	char buff[256];
	buff[0]=0;
	valueString(buff);
	fprintf(f,"%s",buff);
}

//-------------------------------------------------------------
// TNnoise::optionString() get option string
//-------------------------------------------------------------
void TNnoise::optionString(char *c)
{
    c[0]=0;
    if(type==0)
        return;
	int ntype=type&NTYPES;
	int nopt=type&(NOPTS|0xf0);

	int i;
	int mask=0;
	// get noise type
	if(!ntype) // default
		strcpy(c,"GRADIENT");
	else{
		for(i=0;i<NTypes.size;i++){
			if(ntype==NTypes[i]->value){
				strcpy(c,NTypes[i]->name());
				break;
			}
		}
	}
	for(i=0;i<NOpts.size;i++){
	    int bit=NOpts[i]->value;
    	if((nopt & bit) && !(bit & mask)){
    	    if(c[0])
   				strcat(c,"|");
   			strcat(c,NOpts[i]->name());
   		}
	}
	int rof=type & ROFF;
	if(rof){
	    if(c[0])
	        strcat(c,"|");
	    sprintf(c+strlen(c),"RO%d",rof);
	}
}

//-------------------------------------------------------------
// TNnoise::valueString() node value substring
//-------------------------------------------------------------
void TNnoise::valueString(char *s)
{
	if(CurrentScope->tokens() && token){
        strcat(s,token);
        return;
	}
	else if(CurrentScope->texture()){
		if(id>=0)
			return;
	}
	else if(CurrentScope->shader()){
		char tmp[64];
		if(id>=0){
			if(Td.get_flag(NOISE1D))
				sprintf(tmp,"Noise1D(%d)",id);
			else
				sprintf(tmp,"Noise(%d)",id);
			strcat(s,tmp);
		}
		return;
	}
	char opts[64];
	optionString(opts);
	if(opts[0])
		sprintf(s+strlen(s),"%s(%s",symbol(),opts);
	else
		sprintf(s+strlen(s),"%s(",symbol());
	TNarg *arg=(TNarg*)right;
	if(opts[0] && arg)
		strcat(s,",");
	while(arg){
		arg->valueString(s);
		arg=arg->next();
		if(arg)
			strcat(s,",");
	}
	strcat(s,")");
}
//-------------------------------------------------------------
// TNnoise::propertyString() node value substring
//-------------------------------------------------------------
void TNnoise::propertyString(char *s)
{
	valueString(s);
}
//-------------------------------------------------------------
// TNnoise::init() evaluate the node
//-------------------------------------------------------------
void TNnoise::init()
{
	if(type & FS){
		Td.set_flag(SNOISEFLAG);
		//Td.add_TNnoise(this);
	}
}

//-------------------------------------------------------------
// TNnoise::eval() evaluate the node
//-------------------------------------------------------------
void TNnoise::eval()
{
	double f=0,ampl=1.0,offset=0.0;
	double args[16];

	int n;
	INIT;
	S0.clr_constant();
	if(type & FS)
		Td.set_flag(SNOISEFLAG);

	if(CurrentScope->texture()){
		if(type & FS)
			S0.set_inactive();
		return;
	}
	if(CurrentScope->shader()){
		if(!(type & FS))
			S0.set_inactive();
		return;
	}
	if(CurrentScope->rpass()){
		if(type & FS)
			Td.add_TNnoise(this);
		return;
	}

	n=getargs(right,args,16);
	INIT;
	S0.set_svalid();
	S0.clr_constant();
	if(norm() && !normalized()){
		TheNoise.get_minmax(ma,mb,type,n,args);
#ifdef DEBUG_NOISE
		char buff[256];
		buff[0]=0;
    	valueString(buff);
    	sprintf(tmps,"new %-30s %-12g %-12g",buff,ma,mb);
    	cout << tmps << endl;
#endif
    	//cout <<"ma:"<<ma<<" mb:"<<mb<<endl;
		set_normalized();
    }
    if(type & FS){
     	scale=args[0]; // start level
    	S0.s=0;
    	return; // pixel shader noise
    }
	int dolod=(type&NLOD)?0:Adapt.lod();
	if(dolod){
		int orders=(int)args[1];
        int lmax=(int)(Td.level*0.5-args[0]+4);
#ifdef DEBUG_LOD
		tnoise_visits+=(int)orders;
#endif
		if(lmax>0 && orders>lmax){
	    	args[1]=lmax;
#ifdef DEBUG_LOD
	    	tnoise_hits+=(int)(orders-lmax);
#endif
		}
	}

    if(n>5)
    	ampl=args[5];
    if(n>8)
    	offset=args[8];

    if(args[1]>0){
	    f=TheNoise.eval(type,n,args);
    }
	S0.s=f*ampl*ma+mb+offset;

}

// debug routine to print out shader noise value
static void printFvar(GLhandleARB program,char *v,int nid){
	char str[256];
	sprintf(str,"nvars[%d].%s",nid,v);
	float d[1]={0};
	int loc=glGetUniformLocationARB(program,str);
	glGetUniformfv(program,loc,d);
	printf("%-20s %-10g\n",str,d[0]);
}
//-------------------------------------------------------------
// TNnoise::setProgram() set shader uniform variables
// - called AFTER shader program is created
//-------------------------------------------------------------
bool TNnoise::setProgram(){
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;
	double args[16];
	int n=getargs(right,args,16);

	eval();

	double start_order = args[0];
	double orders = args[1];
	float bias = n>2?args[2]:1.0;
	float H = n>3?args[3]:Noise::dflt_H;
	float L = n>4?args[4]:Noise::dflt_L;
	float ampl=(n>5)?args[5]:1.0;
	float clamp=n>6?args[6]:1.0;
	float smooth=n>7?args[7]:0.0;
	float offset=n>8?args[8]:0.0;

	bool sqr = (type & SQR)?true:false;
	bool invert = (type & NEG)?true:false;
	bool absval = (type & NABS)?true:false;
	bool uns = (type & UNS)?true:false;
	int vnoise=(type & NTYPES)>>8; // GRADient=0, VORONai=1, SIMPLEX=2

	int nid=id;//TerrainProperties::nid;

	char str[256];

	GLSLVarMgr vars;
#define MAXORDERS 28

	start_order=start_order>MAXORDERS?MAXORDERS:start_order;
	//double nfreq = pow(L,start_order-1);
	double nfreq = pow(L,start_order);

	//float logf=log2(nfreq)+TheScene->freq_mip;
	float logf=log2(nfreq);

	double bumpmin=1e-5;
	double minscale=32;
	minscale=minscale<scale?minscale:scale;
	double bumpdelta=pow(10,-(0.25*minscale+4-2*TheScene->bump_mip));
	bumpdelta=bumpdelta<1e-9?1e-9:bumpdelta;


	sprintf(str,"nvars[%d].fact",nid);    	glUniform1fARB(glGetUniformLocationARB(program,str),pow(nfreq,-H));
	sprintf(str,"nvars[%d].delta",nid);    	glUniform1fARB(glGetUniformLocationARB(program,str),pow(L,-H));
	sprintf(str,"nvars[%d].freq",nid);    	glUniform1fARB(glGetUniformLocationARB(program,str),nfreq);
	sprintf(str,"nvars[%d].orders",nid);    glUniform1fARB(glGetUniformLocationARB(program,str),orders);
	sprintf(str,"nvars[%d].bias",nid);      glUniform1fARB(glGetUniformLocationARB(program,str),bias);
	sprintf(str,"nvars[%d].H",nid);         glUniform1fARB(glGetUniformLocationARB(program,str),H);
	sprintf(str,"nvars[%d].L",nid);         glUniform1fARB(glGetUniformLocationARB(program,str),L);
	sprintf(str,"nvars[%d].smoothing",nid);    glUniform1fARB(glGetUniformLocationARB(program,str),smooth);
	sprintf(str,"nvars[%d].clamp",nid);     glUniform1fARB(glGetUniformLocationARB(program,str),clamp);
	sprintf(str,"nvars[%d].ampl",nid);      glUniform1fARB(glGetUniformLocationARB(program,str),ampl*ma);
	sprintf(str,"nvars[%d].offset",nid);    glUniform1fARB(glGetUniformLocationARB(program,str),mb+offset);
	sprintf(str,"nvars[%d].sqr",nid);       glUniform1iARB(glGetUniformLocationARB(program,str),sqr);
	sprintf(str,"nvars[%d].invert",nid);    glUniform1iARB(glGetUniformLocationARB(program,str),invert);
	sprintf(str,"nvars[%d].absval",nid);    glUniform1iARB(glGetUniformLocationARB(program,str),absval);
	sprintf(str,"nvars[%d].uns",nid);       glUniform1iARB(glGetUniformLocationARB(program,str),uns);
	sprintf(str,"nvars[%d].vnoise",nid);    glUniform1iARB(glGetUniformLocationARB(program,str),vnoise);
	sprintf(str,"nvars[%d].logf",nid);      glUniform1fARB(glGetUniformLocationARB(program,str),logf);

//#define TEST
#ifdef TEST
	printf("\n");
	printFvar(program,"freq",nid);
	printFvar(program,"orders",nid);
	/*
	printFvar(program,"fact",nid);
	printFvar(program,"delta",nid);

	printFvar(program,"bias",nid);
	printFvar(program,"H",nid);
	printFvar(program,"L",nid);
	printFvar(program,"smoothing",nid);
	printFvar(program,"clamp",nid);
	printFvar(program,"ampl",nid);
	printFvar(program,"offset",nid);
	printFvar(program,"delta",nid);
*/
#endif
	vars.newFloatVar("rscale",Rscale);

	vars.setProgram(program);
	vars.loadVars();
	return true;
}

//************************************************************
// TNbinary classes
//************************************************************

//************************************************************
// TNwater class
//************************************************************
//-------------------------------------------------------------
// TNwater::eval() evaluate the node
//-------------------------------------------------------------
void TNwater::eval()
{
	if(CurrentScope->rpass()){
		if(right){
			INIT;
			right->eval();
		}
		return;
	}

	S0.set_flag(WATERFLAG);

	double dz=0;
	static TerrainData water;
	static TerrainData s2;

	S0.datacnt=0;
	SeaLevel=Raster.water_level*Gscale;
	water.InitS();

    if(left){
		left->eval();
		SeaLevel+=S0.s;
	}
	INIT;

	if(right)
		right->eval();
	dz=(SeaLevel-S0.p.z);  // water-terrain
	// FIXME - HACK S0.p.z always zero if ONLY geometry noise is present
	//       - this work work if both real and geometry terrain exist
	if(Td.get_flag(SNOISEFLAG)) // shader noise
		dz=fabs(dz); // otherwise water not rendered if sealevel <0
	double dw=fabs(dz);
	WaterDepth=dw;
	WaterHeight=dw/Gscale;

	double cmix=WaterDepth/Gscale;
	double f=rampstep(0,2*Raster.water_clarity,cmix,1,0);
	water.p.z=SeaLevel;

	water.clr_svalid();
	water.set_pvalid();
	water.clr_cvalid();
	water.set_water();
	water.depth=WaterDepth/Gscale;

	//if(CurrentScope->zpass())
	//	return;

	// S0 = terrain surface

	S0.clr_flag(INMARGIN);
    if(dz>=0){  // terrain is below water
		if(S0.datacnt<MAX_TDATA)
			S0.datacnt++;
	    for(int i=1;i<S0.datacnt;i++){
			S0.data[S0.datacnt-i]=S0.data[S0.datacnt-i-1];
	    }
		dz*=Rscale;
		//m=Raster.water_clarity;
		s2.copy(S0);
		S0.data[0]=&s2;
		S0.copy(water);
		//if(dz<m)
	    if(f<1)
			S0.set_flag(INMARGIN);
	 }
	 else {    // terrain is above water with water as surface 2
		 for(int i=0;i<rccnt;i++){
	   		if(mapdata[i]->water()){
				if(S0.datacnt<MAX_TDATA)
					S0.data[S0.datacnt++]=&water;
				else
					S0.data[MAX_TDATA-1]=&water;
				break;
	    	}
		 }
		 /*
		 if(water.p.x||S0.p.x||water.p.y||S0.p.y){
	    	double dx=fabs(water.p.x-S0.p.x);
	    	double dy=fabs(water.p.y-S0.p.y);
	    	double dmax=dx>dy?dx:dy;
	    	if(fabs(dz)<0.5*dmax){
		      	S0.data[S0.datacnt++]=&water;
        		return;
	        }
	    }
		*/
	 }
}
//-------------------------------------------------------------
// TNwater::saveNode() archive the node
//-------------------------------------------------------------
void TNwater::saveNode(FILE *f)
{
	Planetoid *orb=(Planetoid *)getOrbital(this);
	fprintf(f,"%s(",symbol());
	char tmp[64];
	if(left)
		left->save(f);
	else
		fprintf(f,"1.0",tmp);
	Color color=orb->water_color1;
	color.toString(tmp);
	fprintf(f,",%s",tmp);
	color=orb->water_color2;
	color.toString(tmp);
	fprintf(f,",%s",tmp);
	fprintf(f,",%g,%g,%g",orb->water_clarity/FEET,orb->water_shine,orb->water_specular);
	fprintf(f,")\n");
}
//-------------------------------------------------------------
// TNwater::replaceNode
//-------------------------------------------------------------
NodeIF *TNwater::replaceNode(NodeIF *c){
	TerrainData arglist[6];
	Planetoid *orb=(Planetoid *)getOrbital(this);
	if(!orb)
		return 0;
	if(c->typeValue()!=typeValue())
		return 0;
	delete left; // delete old expr
	TNfunc *node=(TNfunc*)c;
	left=(TNarg*)node->left;

	TNarg *args=(TNarg*)left; // first arg (noise expr)
	//if(!args)
	//	return 0;

	int n=getargs(args,arglist,6);

	if(n>1)
		orb->water_color1=arglist[1].c;
	if(n>2)
		orb->water_color2=arglist[2].c;
	if(n>3)
		orb->water_clarity=arglist[3].s*FEET;
	if(n>4)
		orb->water_shine=arglist[4].s;
	if(n>5)
		orb->water_specular=arglist[5].s;

	// remove color expression from tree since it is no longer needed
	args->right=0;
	delete c;
	return this;
}

//************************************************************
// TNfog class
//************************************************************
//-------------------------------------------------------------
// TNfog::eval() evaluate the node
//-------------------------------------------------------------
void TNfog::eval()
{

	S0.density=0.0;
	//SINIT

	if(right){
		SINIT;
		right->eval();
	}
	if(left && !CurrentScope->zpass()&& !CurrentScope->rpass()){
		SPUSH;
		left->eval();
		S0.density=clamp(S0.s,0.0,1);
		SPOP;
	}
}
//-------------------------------------------------------------
// TNfog::save() archive the node
//-------------------------------------------------------------
void TNfog::saveNode(FILE *f)
{
	Planetoid *orb=(Planetoid *)getOrbital(this);
	fprintf(f,"%s(",symbol());
	char tmp[64];
	if(left)
		left->save(f);
	else
		fprintf(f,"1.0",tmp);
	Color color=orb->fog_color;
	color.toString(tmp);
	fprintf(f,",%s",tmp);

	fprintf(f,")\n");
}
//-------------------------------------------------------------
// TNfog::replaceNode
//-------------------------------------------------------------
NodeIF *TNfog::replaceNode(NodeIF *c){
	TerrainData arglist[2];
	Planetoid *orb=(Planetoid *)getOrbital(this);
	if(!orb)
		return 0;
	if(c->typeValue()!=typeValue())
		return 0;
	//TNfunc *newobj=(TNfunc*)c;
	//newobj->right=right;
	//if(right)
	//	right->setParent(newobj);
	//NodeIF *p=getParent();
	//if(p)
	//	p->replaceChild(this,newobj);

	//if(!TNfunc::replaceNode(c))
	//	return 0;

	TNfog *fog=(TNfog*)c;
	delete left; // delete old expr
	left=(TNarg*)fog->left;
	TNarg *args=(TNarg*)left; // first arg (noise expr)
	//if(!args)
	//	return 0;

	int n=getargs(args,arglist,2);

	if(n<2)
		return this;
	orb->fog_color=arglist[1].c;
	// remove color expression from tree node since it is no longer needed
	args->right=0;
	delete c;
	return this;
}

//************************************************************
// TNsnow class
//************************************************************
TNsnow::TNsnow(TNode *l, TNode *r) : TNfunc(l,r) {
	texture=0;
	image=0;
}

//-------------------------------------------------------------
// TNsnow::~TNsnow() destructor
//-------------------------------------------------------------
TNsnow::~TNsnow()
{
	DFREE(texture);
}

//-------------------------------------------------------------
// TNsnow::init() initialize the node
//-------------------------------------------------------------
void TNsnow::init()
{
#define NALPHA 4
	if(!texture){
		if(!image){
			image=images.load("snow",BMP|JPG);
			if(!image){
				Color *color=0;
				MALLOC(2*NALPHA,Color,color);
				int i=0,j;
				for(j=0;j<NALPHA;j++)
					color[i++]=Color(1,1,1,0);
				for(j=0;j<NALPHA;j++)
					color[i++]=Color(1,1,1,1);
				image=new Image(color,2*NALPHA,1);
				image->set_alpha(1);
				images.save("snow",image);
				cout <<"building Snow Image"<<endl;
			}
			else
				cout <<"loading Snow Image"<<endl;
		}
		if(!image)
			return;
		int opts=BORDER|CLAMP|TEX;
	    texture=new Texture(image,opts,this);
	    texture->orders=1;
	    texture->scale=1;
	    texture->bump_active=true;
	    texture->s_data=true;
	}
	if(right)
    	right->init();
}

//-------------------------------------------------------------
// TNsnow::initProgram() set shader #defines for texture
//-------------------------------------------------------------
//+Texture("snow",BORDER|BUMP|LINEAR|S,BMPHT+HT-5*SLOPE+10*LAT*LAT,1,0,1,-1.77776,1,1.73848,1,0.9,0,0,0,-0.07408)

bool TNsnow::initProgram(){
	char defs[1024];
    TNarg *arg=(TNarg*)left;
	double args[7]={0};
	defs[0]=0;

	if(!isEnabled() || !Render.textures())
		return false;
	int id=TheMap->tp->textures.size-1;

	sprintf(defs,"#define TX%d\n",id);

	texture->cid=Texture::num_coords;
	int n=getargs(arg,args,7);
	double bias=args[0];
	double ht=args[1];
	double lat=args[2];
	double slope=args[3];
	double depth=args[4];
	double bmpht=args[5];
    texture->bump_damp=depth;
    texture->bias=bias+2*slope-ht;

	sprintf(defs+strlen(defs),"#define X%d",id);
	sprintf(defs+strlen(defs)," %g*HT",2*ht);
	if(bmpht>0)
		sprintf(defs+strlen(defs),"+%g*BMPHT",bmpht);
	sprintf(defs+strlen(defs),"\n");
	//cout << "TNsnow :"<<defs << endl;

	sprintf(defs+strlen(defs),"#define C%d CS%d\n",id,texture->num_coords++);
	strcat(GLSLMgr::defString,defs);
	return true;
}

//-------------------------------------------------------------
// TNsnow::eval() evaluate the node
//-------------------------------------------------------------
void TNsnow::eval()
{
	if(!texture || !right)
		return;
	if(!isEnabled()||!Render.textures())
		texture->enabled=false;
	else
		texture->enabled=true;
	if(CurrentScope->rpass()){
		right->eval();
		Td.add_texture(texture);
		return;
	}

	if(CurrentScope->zpass()){
		right->eval();
		return;
	}

	if(!CurrentScope->eval_mode()){
  		TNarg *arg=(TNarg*)left;
  		double args[6]={0};

		int n=0;
		n=getargs(arg,args,6);
		double slope=args[3];
		double lat=args[2];
		double amp=-10*slope*zslope();
		double f=fabs(Phi/180);
		amp+=20*lat*f*f;
		texture->s=amp;

		S0.clr_svalid();
	}
	else {
		right->eval();
	}
}

//************************************************************
// TNclouds class
//************************************************************
static TNnoise *s_noise=0;
static void findSNoise(NodeIF *obj)
{
	if(obj->typeValue()==ID_NOISE){
		if(((TNnoise*)obj)->type & FS){
			s_noise=(TNnoise*)obj;
			obj->setFlag(NODE_STOP);
		}
	}
}
static TNpoint *s_point=0;
static void findPoint(NodeIF *obj)
{
	if(obj->typeValue()==ID_POINT){
		s_point=(TNpoint*)obj;
		obj->setFlag(NODE_STOP);
	}
}

//-------------------------------------------------------------
// TNclouds::threeD() return true for volume clouds
//-------------------------------------------------------------
bool TNclouds::threeD()
{
	s_point=0;
	if(!right)
		return false;
	right->visitNode(findPoint);
    if(s_point==0){
     	right->eval();
    	return false;
    }
    return true;
}

//-------------------------------------------------------------
// TNclouds::eval() evaluate the node
//-------------------------------------------------------------
void TNclouds::init()
{
 	TerrainData::add_TNclouds(this);
}

//-------------------------------------------------------------
// TNclouds::eval() evaluate the node
//-------------------------------------------------------------
void TNclouds::eval()
{
	static TerrainData top;
	static TerrainData bottom;

	if(!right)
		return; // nothing to do
	s_point=0;
	right->visitNode(findPoint);
    if(s_point==0){
    	right->eval();
    	//return;
    }
    if(CurrentScope->rpass()){
    	INIT;
     	TerrainData::add_TNclouds(this);

		TNarg &arg=*((TNarg*)left);
		TNnoise *tn=0;//(TNnoise*)arg[2];
		if(arg[2]>0){
			s_noise=0;
			arg[2]->visitNode(findSNoise);
			tn=s_noise;
		}

     	INIT;

    	S0.c=Color(1,0,0);
     	right->eval();
     	bool htexpr=S0.pvalid();
     	if(tn)  // add scatter noise to layer 1
			tn->eval();
   	   // if height function exists, reserve levels for top and bottom layers
     	if(htexpr){
      		Td.add_id();
     		right->eval();
			if(tn)  // add scatter noise to layer 2
				tn->eval();
    	}
    	return;
    }
	INIT;
	S0.clr_flag(HIDDEN);
	right->eval();
	if(!S0.pvalid())
		return;
	
	if(S0.p.z<0)
		S0.set_flag(HIDDEN);

	Height=S0.p.z;

	right->eval();
	top.copy(S0);
	top.c.set_alpha(S0.c.alpha());
	double tht=Height;
	double bht=-0.25*Height;
    TNarg &arg=*((TNarg*)left);
	
	if(arg[0]){
		arg[0]->eval();
		Height=tht;
		bht=-0.25*Height;
		if(arg[1]){
			arg[1]->eval();
			bht=S0.s;
		}
	}
	if(S0.get_flag(HIDDEN))
		Td.depth=0.0;
	else
		Td.depth=fabs(tht-bht);

    top.next_id();

	bottom.copy(top);
	bottom.p.z=-top.p.z;
	bottom.c=S0.c;
	//bottom.c.set_alpha(S0.c.alpha());
    bottom.set_id(S0.tids);

//    Density=0.0;
//    if(arg[2]){
//    	Td.clr_flag(SNOISEFLAG);
//    	arg[2]->eval();
//    	if(!Td.get_flag(SNOISEFLAG))
//    		Density=S0.s;
//    }
	
	if(arg[0]){
		arg[0]->eval();
		top.p.z=Height=S0.s;
		right->eval();
		top.c=S0.c;
		if(arg[2]){
			arg[2]->eval();
			top.s=S0.s;
			top.set_svalid();
		}
	}
	if(arg[1]){
		arg[1]->eval();
		bottom.p.z=Height=S0.s;
		right->eval();
		bottom.c=S0.c;
		if(arg[2]){
			arg[2]->eval();
			bottom.s=S0.s;
			bottom.set_svalid();
		}
	}
	if(arg[2])
		S0.set_flag(DVALUE);
	else
		S0.clr_flag(DVALUE);

	if(top.p.z>bottom.p.z){
		S0.copy(top);
		Td.lower.copy(bottom);
	}
	else{
		S0.copy(bottom);
		Td.lower.copy(top);
	}
	top.set_cvalid();
    bottom.set_cvalid();

 	Td.begin();
    Td.zlevel[0].copy(S0);
    Td.zlevel[1].copy(Td.lower);
	Td.end();

}
//-------------------------------------------------------------
// TNclouds::initProgram() set shader #defines
//-------------------------------------------------------------
bool TNclouds::initProgram(){
	char defs[1024];
	char nstr[1024];
   
    TNarg &arg=*((TNarg*)left);
	defs[0]=0;
	nstr[0]=0;

	if(arg[2]>0){
		s_noise=0;
		arg[2]->visitNode(findSNoise);
		if(s_noise){
			CurrentScope->set_shader(1);
			arg[2]->valueString(nstr);
			sprintf(defs,"#define SCATTER %s\n",nstr);
			strcat(defs,GLSLMgr::defString);
			strcpy(GLSLMgr::defString,defs);
			CurrentScope->set_shader(0);
		}
	}
	return true;
}

//-------------------------------------------------------------
// TNclouds::setProgram() set shader vars
//-------------------------------------------------------------
bool TNclouds::setProgram(){
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;

	GLSLVarMgr vars;
	
	double zn=TheScene->znear;
	double zf=TheScene->zfar;
	double zmax=10.0*MILES;
	double args[6];
	int n=getargs(left,args,4);

	if(n>2){
		s_noise=0;
		zmax=args[2]*MILES;
		TNarg &arg=*((TNarg*)left);
		arg[2]->visitNode(findSNoise);
		if(s_noise)
			s_noise->setProgram();
	}
	vars.newFloatVar("znear",zn);
	vars.newFloatVar("zfar",zf);
	vars.newFloatVar("zmax",zmax);

 	vars.setProgram(program);
	vars.loadVars();

	return true;
}
