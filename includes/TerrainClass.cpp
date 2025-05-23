
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
#include "UniverseModel.h"

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

extern double Theta, Phi, Height, Drop, Margin,Impact,Radius,Density,MaxHt,MinHt,FHt,Randval,Srand,Level,Range,Temp,Tave,Tsol,Tgas;
extern char   tabs[];
extern double Hscale,Rscale;

static double  Mheight;

double Red=0, Green=0, Blue=0, Alpha=1;
double Pz=0, Px=0, Py=0;
int addtabs=0;

char tmps[1024];

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
	{"TA",	    	TA},
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
	LNG,
	PHI,
	LAT,
	EQU,
	TEMP,
	TAVE,
	TSOL,
	TGAS,
	SLOPE,
	RADIUS,
	DENSITY,
	RANGE,
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
	IRAND,
	URAND,
	SRAND,
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
	{"LONG",		LNG},
	{"PHI",			PHI},
	{"LAT",			LAT},
	{"EQU",			EQU},
	{"TEMP",		TEMP},
	{"TAVE",		TAVE},
	{"TSOL",		TSOL},
	{"TGAS",		TGAS},
	{"SLOPE",		SLOPE},
	{"RADIUS",		RADIUS},
	{"DENSITY",		DENSITY},
	{"RANGE",		RANGE},
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
	{"IRAND",		IRAND},
	{"URAND",		URAND},
	{"SRAND",		SRAND},
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
//	FREE(value);
//	value=0;
}
void TNstring::eval()
{
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
	case LNG:
		if(CurrentScope->texture())
			S0.set_inactive();
		//S0.s=Theta/180-1;
		S0.s=cos(0.5*RPD*(Theta+90));
		S0.s=fabs(S0.s);//fabs(sin(PI*S0.s));
		//cout<<Theta<<" "<<S0.s<<endl;
		break;
	case PHI:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=(Phi+90)/180;
		S0.s=clamp(S0.s,0,1);
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
	case TEMP:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=Temp;///100.0;
		break;
	case TAVE:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=Tave;
		break;
	case TSOL:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=Tsol;
		break;
	case TGAS:
		if(CurrentScope->texture())
			S0.set_inactive();
		S0.s=Tgas;
		break;
	case RADIUS:
		S0.s=Radius;
		break;
	case DENSITY:
		//Td.set_flag(SNOISEFLAG);
		S0.s=Density;
		break;
	case RANGE:
		//Td.set_flag(SNOISEFLAG);
		S0.s=Range;
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
	case IRAND:
		S0.s=Randval;
		//S0.set_constant();
		break;
	case URAND:
		S0.s=Srand+0.5;
		//S0.set_constant();
		break;
	case SRAND:
		S0.s=Srand;
		//S0.set_constant();
		break;
	case LVL:
		S0.s=Level;
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
		if(CurrentScope->shader())
			S0.set_constant();
		break;
	case ZBASE:
		S0.s=S0.height;
		break;
	case DEPTH:
		S0.s=Td.depth;
		if(CurrentScope->shader())
			S0.set_constant();
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
    if(!isEnabled())
    	return false;
	//char nstr[MAXSTR];
	char defs[1024];
	char red[1024]={0};
	char green[1024]={0};
	char blue[1024]={0};
	char alpha[1024]={0};
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
int TNcolor::comps()
{
	return numargs((TNarg*)right);
}
//-------------------------------------------------------------
// TNcolor::eval() evaluate the node
//-------------------------------------------------------------
void TNcolor::eval()
{

 	TNarg *arg=(TNarg*)right;
	INIT;
	Alpha=1;

    if(CurrentScope->rpass()){
		Td.clr_flag(SNOISEFLAG);
    	if(!isEnabled()){
    		TerrainData::tp->set_color(false);
    	}
    	else{
			//right->init();
			arg->eval(); // red
			Red=S0.s;
			arg=arg->next();
			arg->eval(); // green
			Green=S0.s;
			arg=arg->next();
			arg->eval(); // blue
			Blue=S0.s;
			arg=arg->next();
			if(arg){
				arg->eval(); // alpha
				Alpha=S0.s;
			}
			if(Td.get_flag(SNOISEFLAG))
				TerrainData::add_TNcolor(this);
			else{
				Td.clr_flag(SNOISEFLAG);
			}
			S0.set_cvalid();
			S0.c=Color(Red,Green,Blue,Alpha);
			TerrainData::tp->set_color(true);
			TerrainData::tp->color=S0.c;
    	}
    	return;
    }
	if(!isEnabled()){
		S0.clr_cvalid();
		return;
	}

	int cflag=1;

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
	while((c=colors++)){
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
	if(i<0)
		return;
	if(i <colors.size){
		delete colors.base[i];
		colors.base[i]=new Color(c);
	}
	else
		colors.add(new Color(c));
}

//-------------------------------------------------------------
// TNclist::get_color() get color at index (i)
//-------------------------------------------------------------
Color  TNclist::get_color(int i)
{
	Color *cp;
	cp=colors[i];
	if(cp)
		return *cp;
	return Color(1.0,1.0,1.0,0);
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
	TNarg *arg=(TNarg*)right;
	INIT;
	if(CurrentScope->zpass()){
		arg->eval();
		S0.density=S0.s;
	}
	if(CurrentScope->rpass()){
		Td.clr_flag(SNOISEFLAG);
		arg->eval();
		if(Td.get_flag(SNOISEFLAG))
			TerrainData::add_TNdensity(this);
		else
			Td.clr_flag(SNOISEFLAG);	
	}
	
}

//-------------------------------------------------------------
// TNdensity::init() init the node
//-------------------------------------------------------------
void TNdensity::init(){
	right->init();
}
//-------------------------------------------------------------
// TNdensity::initProgram() set shader defines
//-------------------------------------------------------------
bool TNdensity::initProgram(){
    if(!isEnabled())
    	return false;
    char defs[MAXSTR];
	defs[0]=0;
	char argstr[MAXSTR];
	argstr[0]=0;
	CurrentScope->set_shader(1);
	TNarg *arg=(TNarg*)right;
	Td.clr_flag(SNOISEFLAG);
    Td.set_flag(NOISE1D);
    arg->eval();
    arg->valueString(argstr); // red
    sprintf(defs,"#define ND %s\n",argstr);
    strcat(GLSLMgr::defString,defs);
    CurrentScope->set_shader(0);
    Td.clr_flag(NOISE1D);
    return true;
}
//************************************************************
// TNpoint class
//************************************************************
//-------------------------------------------------------------
// TNpoint::eval() evaluate the node
//-------------------------------------------------------------
void TNpoint::eval()
{
	if(!isEnabled())
		return;

	INIT;
    Px=Py=Pz=0;
	Td.clr_flag(SNOISEFLAG);

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
			//S0.clr_pvalid();
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
		//return;
	}
	if(CurrentScope->eval_mode() && Td.get_flag(SNOISEFLAG)){
		Td.set_flag(GNOISEFLAG);
		//return;
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
	if(!isEnabled())
		return false;

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
	//char nstr[MAXSTR];
	char defs[MAXSTR];
	char x[MAXSTR]={0};
	char y[MAXSTR]={0};
	char z[MAXSTR]={0};
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
	mx=0;
	mn=0;
	ma=1.0;
	mb=0;
	id=-1;
	scale=1.0;
	rate=1e-6;

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
	char buff[MAXSTR];
	buff[0]=0;
	valueString(buff);
	fprintf(f,"%s",buff);
}

//-------------------------------------------------------------
// TNnoise::getNtype() get noise type string
//-------------------------------------------------------------
void TNnoise::getNtype(char *c)
{
	int ntype=type&NTYPES;
	for(int i=0;i<NTypes.size;i++){
		if(ntype==NTypes[i]->value){
			strcpy(c,NTypes[i]->name());
			break;
		}
	}
}

//-------------------------------------------------------------
// TNnoise::getOpts() get opts string
//-------------------------------------------------------------
void TNnoise::getOpts(char *c)
{
	int nopt=type&(NOPTS|0xf0);
	int mask=0;

 	for(int i=0;i<NOpts.size;i++){
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
// TNnoise::optionString() get option string
//-------------------------------------------------------------
void TNnoise::optionString(char *c)
{
    c[0]=0;
    if(type==0)
        return;
    getNtype(c);
    getOpts(c);
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
	clr_normalized();
}

std::string TNnoise::randomize(char *src,double f,double t){
	char opts[256];
	double args[16];
	opts[0]=0;
	TNnoise *noise=TheScene->parse_node(src);
	std::string str("noise(");
	if(t<0.1){
		noise->optionString(opts);
	    str+=opts;
	}
	else if(t<0.5){
		str+=Noise::getNtype(r[5]);
		noise->getOpts(opts);
		if(strlen(opts)){
			str+="|";
			str+=opts;
		}
	}
	else{
		str+=Noise::getNtype(r[5]);
		str+=Noise::getNopts(r[3]);
	}
	int n=getargs(noise->right,args,16);
	char tmp[64];
	for(int i=0;i<n;i++){
		args[i]+=f*s[i]*fabs(args[i]);
		sprintf(tmp,",%1.2f",args[i]);		
		str+=tmp;
	}
	str+=")";
	//cout<<str.c_str()<<endl;
	return str;
	
//
}
//-------------------------------------------------------------
// TNnoise::eval() evaluate the node
//-------------------------------------------------------------
void TNnoise::eval()
{
	double f=0,ampl=TheNoise.maxampl,offset=0.0;
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
    if(n>5)
    	ampl*=args[5];
    if(n>8)
    	offset=args[8];
	if(norm() && !normalized()){
		TheNoise.get_minmax(ma,mb,type,n,args);
		mx=ampl*ma;
		mn=-ampl*ma;

#ifdef DEBUG_NOISE
		char buff[MAXSTR];
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

    if(args[1]>0 && ampl>0)
	  f=TheNoise.eval(type,n,args);
    if(ampl==0)
    	S0.s=offset;
    else
	    S0.s=f*ampl*ma+mb+offset;

}

// debug routine to print out shader noise value
static void printFvar(GLhandleARB program,char *v,int nid){
	char str[MAXSTR];
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
	float clamp=n>6?args[6]:4.0;
	float smooth=n>7?args[7]:0.0;
	float offset=n>8?args[8]:0.0;
	float rate=n>9?args[9]:2e-6;

	bool sqr = (type & SQR)?true:false;
	bool invert = (type & NEG)?true:false;
	bool absval = (type & NABS)?true:false;
	bool uns = (type & UNS)?true:false;
	bool animate=(type & TA)?true:false;
	int vnoise=(type & NTYPES)>>8; // GRADient=0, VORONai=1, SIMPLEX=2

	int nid=id;//TerrainProperties::nid;

	char str[MAXSTR];

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
	double seed=TheNoise.rseed;
	if(animate){
		double t=animate?rate*TheScene->time:0;
		seed+=t;
	}
	else
		vars.newFloatVar("rseed",seed);
	
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
	sprintf(str,"nvars[%d].seed",nid);    glUniform1fARB(glGetUniformLocationARB(program,str),seed);

	sprintf(str,"nvars[%d].sqr",nid);       glUniform1iARB(glGetUniformLocationARB(program,str),sqr);
	sprintf(str,"nvars[%d].invert",nid);    glUniform1iARB(glGetUniformLocationARB(program,str),invert);
	sprintf(str,"nvars[%d].absval",nid);    glUniform1iARB(glGetUniformLocationARB(program,str),absval);
	sprintf(str,"nvars[%d].animate",nid);    glUniform1iARB(glGetUniformLocationARB(program,str),animate);

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
	Planetoid *obj=(Planetoid*)(TheMap->object);
	if(CurrentScope->rpass() || !isEnabled()){
		if(right){
			INIT;
			right->eval();
		}
		return;
	}
    double newseed=obj->ocean->getRseed();	
    double oldseed=Noise::rseed;
    if(newseed>0)
    	Noise::rseed=newseed;
	S0.set_flag(WATERFLAG);

	double dz=0;
	static TerrainData water;
	static TerrainData s2;

	extern Point MapPt;

	TNarg &arg=*((TNarg*)left);

	S0.datacnt=0;
	SeaLevel=Raster.sea_level*Gscale;
	double temp=obj->calcLocalTemperature(true);

	// drop sealevel as liquid turns to gas
	double gf=obj->liquidToGas(temp);
	double vf=0.5*obj->ocean->liquid->volatility;
	SeaLevel-=vf*gf;

	water.InitS();

	TNarg *args=(TNarg*)left;
	int n=numargs(args);

    if(left && !obj->gas()){
    	arg[0]->eval();
    	double lvl1=S0.s;
    	double slvl=lvl1;
    	if(n==2 && !obj->liquid()){
    		arg[1]->eval();
    		double lvl2=S0.s;
    		double f=obj->solidToLiquid(temp);
    		slvl=f*lvl2+(1-f)*lvl1;
    	}
		SeaLevel+=slvl;
	}
    Noise::rseed=oldseed;

	INIT;

	if(right)
		right->eval();
	bool geom=Td.get_flag(GNOISEFLAG);
		
	dz=SeaLevel-S0.p.z;  // water-terrain
	WaterDepth=dz<0?0:dz;
	if(geom)
		WaterDepth=abs(WaterDepth);
	WaterHeight=dz/Gscale;

	double cmix=WaterDepth/Gscale;
	water.p.z=SeaLevel;

	water.clr_svalid();
	water.set_pvalid();
	water.clr_cvalid();
	water.set_water();
	water.depth=WaterDepth/Gscale;
	double mf=fabs(dz)/Gscale;
	double mv=rampstep(0,2*Raster.water_clarity,mf,0,1);
 
	if(mv>=1){
		water.clr_flag(INMARGIN);
		S0.clr_flag(INMARGIN);
	}
	else{
	   	water.set_flag(INMARGIN);
	    S0.set_flag(INMARGIN);
	}
    if(dz>=0 || geom){  // terrain is below water
		if(S0.datacnt<MAX_TDATA)
			S0.datacnt++;
	    for(int i=1;i<S0.datacnt;i++){
			S0.data[S0.datacnt-i]=S0.data[S0.datacnt-i-1];
	    }
		s2.copy(S0);
		S0.data[0]=&s2;
		S0.copy(water);
	 }
	 else {    // terrain is above water with water as surface 2
		 for(int i=0;i<rccnt;i++){
	   		if(mapdata[i] && mapdata[i]->water()){
				if(S0.datacnt<MAX_TDATA)
					S0.data[S0.datacnt++]=&water;
				else
					S0.data[MAX_TDATA-1]=&water;
				break;
	    	}
		 }
	 }
}
//-------------------------------------------------------------
// TNwater::saveNode() archive the node
//-------------------------------------------------------------
void TNwater::saveNode(FILE *f)
{
	Planetoid *orb=(Planetoid *)getOrbital(this);
	orb->ocean->setName(nodeName());
	orb->ocean->saveNode(f);
}

bool TNwater::randomize(){
	Planetoid *orb=(Planetoid *)getOrbital(this);
	orb->ocean->randomize();
	orb->invalidate();
	TheScene->rebuild();
	return true;
}

//-------------------------------------------------------------
// TNwater::save() archive the node
//-------------------------------------------------------------
void TNwater::save(FILE *f)
{
	fprintf(f,"%s(",symbol());
	if(left)
		left->save(f);
	fprintf(f,")\n%s",tabs);
	if(right)
		right->save(f);
}
void TNwater::setNoiseExprs(OceanState *s){
	char str[1024];
	str[0]=0;
	sprintf(str,"ocean(%s,%s)\n",s->getOceanLiquidExpr(),s->getOceanSolidExpr());
	setExpr(str);
	applyExpr();
}
void TNwater::getNoiseExprs(OceanState *s){
	TNarg &arg=*((TNarg*)left);
	char tmp[256];
	tmp[0]=0;
	arg[0]->valueString(tmp);
	s->setOceanLiquidExpr(tmp);
	tmp[0]=0;
	arg[1]->valueString(tmp);
	s->setOceanSolidExpr(tmp);
}
//-------------------------------------------------------------
// TNwater::replaceNode
//-------------------------------------------------------------

NodeIF *TNwater::replaceNode(NodeIF *c){
	TerrainData arglist[8];
	if(getParent()==0){
	   cout<<"TNwater::replaceNode parent=0"<<endl;
	   return 0;
	}
	Planetoid *orb=(Planetoid *)getOrbital(this);
	if(!orb){
		cout<<"TNwater::replaceNode orb=0"<<endl;
		return 0;
	}
	if(c->typeValue()!=ID_OCEAN && c->typeValue()!=ID_WATER){
		cout<<"TNwater::replaceNode wrong type "<<typeValue()<<":"<<c->typeValue()<<endl;
		return 0;
	}
	if(c->typeValue()==ID_OCEAN){
		orb->setOcean((OceanState *)c);
	}

	return this;
}

NodeIF *TNwater::getInstance(){

	int last=lastn;
	lastn=getRandValue();//*1115;
	setRands();	
	Planetoid *orb=(Planetoid *)getOrbital(this);
	OceanState *state=OceanState::newInstance();

	setNoiseExprs(state);
	orb->setOcean(state);

	replaceNode(state);
	orb->ocean->rseed=getRandValue();

	lastn=last;

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
	TNfog *fog=(TNfog*)c;
	delete left; // delete old expr
	left=(TNarg*)fog->left;
	TNarg *args=(TNarg*)left; // first arg (noise expr)

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
	//cout<<"~TNsnow()"<<endl;
	DFREE(image);
	DFREE(texture);
}

//-------------------------------------------------------------
// TNsnow::init() initialize the node
//-------------------------------------------------------------
void TNsnow::init()
{
#define NALPHA 4
	if(right)
    	right->init();

	if(!texture){
		if(!image){
			image=images.open("snow");
			//image=images.load("snow",BMP|JPG);
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
			}
			image->set_accessed(true);
		}
		if(!image)
			return;
		int opts=ACHNL|BORDER|BUMP|TEX|INTERP|TBIAS;
	    texture=new Texture(image,opts,this);
	    texture->orders=1;
	    texture->scale=0.5;
	}
}

//-------------------------------------------------------------
// TNsnow::initProgram() set shader #defines for texture
//-------------------------------------------------------------
bool TNsnow::initProgram(){
	char defs[1024];
    TNarg *arg=(TNarg*)left;
	double args[7]={0};
	defs[0]=0;

	if(!isEnabled() || !Render.textures())
		return false;
	int id=texture->num_coords;//texture->tid;

	sprintf(defs,"#define TX%d\n",id);
	//sprintf(defs+strlen(defs),"#define PTEST\n");

	texture->cid=Texture::num_coords;
	int n=getargs(arg,args,7);
	double bias=args[0];
	double ht=args[1];
	double lat=args[2];
	double slope=args[3];
	double depth=args[4];
	double bmpht=args[5];
    texture->bump_damp=depth;
    texture->bias=bias;

    Planetoid *orb=(Planetoid *)getOrbital(this);
  
    texture->bump_bias=bmpht;
    texture->height_bias=ht;
    texture->slope_bias=slope;
    texture->tilt_bias=1;
    texture->phi_bias=-lat;
    texture->texamp=1;
 
	sprintf(defs+strlen(defs),"#define C%d CS%d\n",id,texture->num_coords++);
	strcat(GLSLMgr::defString,defs);
	return true;
}

//-------------------------------------------------------------
// TNsnow::eval() evaluate the node
//-------------------------------------------------------------
void TNsnow::eval()
{
	if(!right)
		return;
	// note: when called from mapnode constructor evaluate texture only
    if(!CurrentScope->tpass())
    	right->eval(); // otherwise process downstream stack

 	if(!texture )
		return;

	if(!isEnabled()||!Render.textures())
		texture->enabled=false;
	else
		texture->enabled=true;

	if(texture->enabled && CurrentScope->rpass()){
		Td.add_texture(texture);
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
		if(arg[2]!=0){
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

	if(arg[2]){
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
