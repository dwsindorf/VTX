#include "States.h"
#include "SceneClass.h"
#include "TerrainClass.h"

static char* def_liquid_func="noise(GRADIENT|SCALE,18,3,1,0.5,2,0.11,1,0,0)";
static char* def_solid_func="noise(GRADIENT|NABS|SCALE|SQR,15.2,8.6,0.1,0.4,1.84,0.73,-0.34,0,0.0,1e-06)";
static char* def_ocean_func="noise(GRADIENT,8,14,0.59,0.5,2,0.8,4,1,0,1e-06)";

//ocean.expr=-0.4*LAT+noise(GRADIENT|NNORM|SCALE,5,9.5,1,0.5,2,0.23,1,0,0,1e-06);
Array<OceanState*> OceanState::oceanTypes(NUM_OCEAN_TYPES);
char *OceanState::oceanNames[]={"Water","SO2","CO2","CH4","N2"};
static double gas_temps[]={-196,-163,-78,-10,100};
static double solid_temps[]={-210,-182,-79,-72,0,};

static char *H2O_liq_str="liquid(Color(0.67,0.93,0.93),Color(0.00,0.16,0.16),373,300,0.95,100,1,10,noise(GRADIENT|SCALE,17.4,3,1,0.5,2.08,0.11,1,0,0))";
static char *H2O_sol_str="solid(Color(1.00,1.00,1.00,0.80),Color(0.40,0.67,0.80),273,1,0.95,0.8,0.1,0.05,noise(GRADIENT|NABS|SCALE|SQR,15.2,8.6,0.1,0.4,1.84,0.73,-0.34,0,0.005,1e-06))";
static char *SO2_liq_str="liquid(Color(0.9,0.9,0.8,0.522),Color(0.447,0.435,0.512),263,1000,0.8,71,1,10,noise(GRADIENT|SCALE,19,3,1,0.5,2,0.11,1,0,0))";
static char *SO2_sol_str="solid(Color(1.000,0.95,0.8,0.769),Color(0.784,0.700,0.716),201,5,0.95,0.8,0.6,0.1,noise(GRADIENT|NABS|SCALE|SQR,15,8.6,0.1,0.4,1.8,0.2,0.6))";
static char *CO2_liq_str="liquid(Color(0,1,1,0.2),Color(0.1,0.1,0.5),195,300,0.95,100,1,10)";
static char *CO2_sol_str="solid(Color(1,1,1,0.6),Color(0.400,0.675,0.8),195,1.0,0.95,0.8,0.6,0.2)";
static char *CH4_liq_str="liquid(Color(0,1,1,0.2),Color(0.1,0.1,0.5),110,300,0.95,100,1,10)";
static char *CH4_sol_str="solid(Color(1,1,1,0.6),Color(0.400,0.675,0.8),91,1.0,0.95,0.8,0.6,0.2)";
static char *N2_liq_str="liquid(Color(0,1,1,0.2),Color(0.1,0.1,0.5),77,300,0.95,100,1,10)";
static char *N2_sol_str="solid(Color(1,1,1,0.6),Color(0.400,0.675,0.8),63,1.0,0.95,0.8,0.6,0.2)";

//************************************************************
// OceanState
//************************************************************
OceanState::OceanState(TNode *l,LiquidState *m1, SolidState *m2): TNfunc(l,0){
	TNarg *arg=left;
	TNarg *node=arg->left;
	expr[0]=0;
	
	setName("Ocean");
	liquid=m1;
	solid=m2;
	ocean_expr=0;
	rseed=0;

	if(node->typeValue() == ID_STRING){		
		setName(((TNstring*)node)->value);
		left=arg->next();
		if(left){
			left->setParent(this);
			arg->right=0;
		}
		delete arg;	
	}
	TNarg &args=*((TNarg *)left);
	if(args[0])
		args[0]->valueString(expr);
	else
		strcpy(expr,def_ocean_func);
	if(args[1]){
		args[1]->eval();
		rseed=S0.s;
	}
}

void  OceanState::getOceanFunction(char *buff){
	strcpy(buff,expr);
}

bool  OceanState::setOceanFunction(char *e){
	TNode *n=TheScene->parse_node(e);
	if(n){
		DFREE(ocean_expr);
		strcpy(expr,e);
		ocean_expr=n;
		return true;
	}
	else
		return false;
}		

char *OceanState::getOceanExpr(){
	if(!ocean_expr)
		ocean_expr=TheScene->parse_node(expr);
	return expr;		
}
void OceanState::setOceanExpr(){
	if(!ocean_expr)
		ocean_expr=TheScene->parse_node(expr);
}
double  OceanState::evalOceanFunction(){
	double t=1;
	if(!ocean_expr)
		ocean_expr=TheScene->parse_node(expr);
    double oldseed=Noise::rseed;
    //if(rseed>0)
    	Noise::rseed=rseed;

	ocean_expr->eval();
	Noise::rseed=oldseed;
	t=S0.s;
	return t;
}

void OceanState::saveNode(FILE *f){
	char str[2048];
	str[0]=0;
	valueString(str);
	fprintf(f,"%s",str);
}

void OceanState::valueString(char *s){
	char str1[512];
	char str2[512];
	str1[0]=0;
	str2[0]=0;
	liquid->valueString(str1);
	solid->valueString(str2);
	sprintf(s,"Ocean(\"%s\",%s,%g)[\n%s,\n%s]",nodeName(),expr,rseed,str1,str2);
}

bool OceanState::randomize(){
	setRseed(getRandValue());
	return true;
}
char *OceanState::getDfltOceanLiquidExpr(){
	return def_liquid_func;	
}
char *OceanState::getDfltOceanSolidExpr(){
	return def_solid_func;	
}
char *OceanState::getDfltOceanExpr(){
	return def_ocean_func;	
}
NodeIF *OceanState::newInstance(){
	char buff[1024];
	char lbuff[1024];
	char sbuff[1024];
	buff[0]=lbuff[0]=sbuff[0]=0;
	LiquidState::newInstance(lbuff);
	SolidState::newInstance(sbuff);
	double seed=getRandValue();
    sprintf(buff,"Ocean(\"Sea\",%s,%g)[\n%s,\n%s]",def_ocean_func,seed,lbuff,sbuff);
    cout<<"OceanState::newInstance\n"<<buff<<endl;
    OceanState *c=TheScene->parse_node(buff);
 	return c;
}

OceanState *OceanState::makeDefaultState(char *n,char *f,char *liq,char *sol){
	char str[1024];
	str[0]=0;
	if(f)
		sprintf(str,"Ocean(\"%s\",%s)[%s,%s]",n,f,liq,sol);
	else
		sprintf(str,"Ocean(\"%s\",%s)[%s,%s]",n,def_ocean_func,liq,sol);
	OceanState *s=(OceanState*)TheScene->parse_node((char*)str);
	return s;
}
//#define TEST
void OceanState::setDefaults(){
	char str[1024];
	//cout<<str<<endl;
	if(oceanTypes.size==0){
		oceanTypes.add(makeDefaultState(oceanNames[H2O],0,H2O_liq_str,H2O_sol_str));
		oceanTypes.add(makeDefaultState(oceanNames[SO2],0,SO2_liq_str,SO2_sol_str));
		oceanTypes.add(makeDefaultState(oceanNames[CO2],0,CO2_liq_str,CO2_sol_str));
		oceanTypes.add(makeDefaultState(oceanNames[CH4],0,CH4_liq_str,CO2_sol_str));
		oceanTypes.add(makeDefaultState(oceanNames[N2],0,N2_liq_str,N2_sol_str));
	}
#ifdef TEST	
		OceanState *test=OceanState::newInstance();
		if(test){
			str[0]=0;
			test->valueString(str);
			cout<<str<<endl;
		}
#endif	
}
//************************************************************
// MaterialState
//************************************************************

MaterialState::MaterialState(): TNunary(0){}
MaterialState::MaterialState(TNode *r) : TNunary(r){
	name[0]=0;
	TNarg *arg=right;
	TNarg *node=arg->left;
	if(node->typeValue() == ID_STRING){		
		setName(((TNstring*)node)->value);
		right=arg->next();
		right->setParent(this);
		arg->right=0;
		delete arg;	
	}
	TNarg &args=*((TNarg *)right);
	
	args[0]->eval();
	color1=S0.c;
	args[1]->eval();
	color2=S0.c;
	
	double vals[7];
	TNarg *a=args.index(2);
	int n=getargs(a,vals,6);
	temp=vals[0];
	clarity=vals[1];
	mix=vals[2];
	shine=vals[3];
	specular=vals[4];
	trans_temp=vals[5];
	expr[0]=0;
}
void MaterialState::valueString(char *s){
	char col1[256]={0};
	char col2[256]={0};
	color1.toString(col1);
	color2.toString(col2);
	
	if(strlen(name)>0)
		sprintf(s,"%s(\"%s\",%s,%s,%g,%g,%g,%g,%g,%g",symbol(),name,col1,col2,temp,clarity,mix,shine,specular,trans_temp);
	else
		sprintf(s,"%s(%s,%s,%g,%g,%g,%g,%g,%g",symbol(),col1,col2,temp,clarity,mix,shine,specular,trans_temp);
	if(strlen(expr)>0)
		sprintf(s+strlen(s),",%s",expr);

	sprintf(s+strlen(s),")",expr);

}
void MaterialState::saveNode(FILE *f){
	char str[1024];
	str[0]=0;
	valueString(str);
	fprintf(f,"%s",str);
}

//************************************************************
// LiquidState
//************************************************************
LiquidState::LiquidState(TNode *a) : MaterialState(a){
	TNarg &args=*((TNarg *)right);
	if(args[7])
		args[7]->valueString(expr);
	else
		setExpr(def_liquid_func);
}
void LiquidState::newInstance(char *buff){
	char cstr[256];
	cstr[0]=0;

	Color tc=Color(0.5+3*r[4],0.5+r[5],0.6*r[5]);
	Color mix=Color(0.4+2*r[7],0.5*r[8],0.1+0.3*r[9]);
	
	Color c1=tc.darken(0.7+0.1*s[2]);
	Color c2=tc.lighten(0.5+0.1*s[3]);

	std::string water="liquid(";

	Color b=Color("Color(0.8,0.9,1)");
	b=b.mix(mix,0.8*r[2]);
	b.set_alpha(0.2);
	b.toString(cstr);
	water+=cstr;
	water+=",";
	b=Color("Color(0.2,0.4,0.5)");
	b=b.mix(c2,0.8*r[2]);
	b=b.darken(0.3*r[4]);
	b.toString(cstr);
	water+=cstr;
	water+=",373,300,0.95,100,1,10,";
	water+=def_liquid_func;
	water+=")";
	strcpy(buff,water.c_str());
	//cout<<buff<<endl;
}
NodeIF *LiquidState::newInstance(){
	char buff[1024];
	buff[0]=0;
	newInstance(buff);
	NodeIF *c=TheScene->parse_node(buff);
	return c;	
}

//************************************************************
// SolidState
//************************************************************
SolidState::SolidState(TNode *a) : MaterialState(a){
	TNarg &args=*((TNarg *)right);
	if(args[7])
		args[7]->valueString(expr);
	else
		setExpr(def_solid_func);
}

void SolidState::newInstance(char *buff){
	char cstr[256];
	cstr[0]=buff[0]=0;

	Color tc=Color(0.5+3*r[4],0.5+r[5],0.6*r[5]);
	Color c2=tc.lighten(0.5+0.1*s[3]);

	std::string ice="solid(";
	Color b=Color("Color(1.0,1.0,1.0,0.173)");
	b=b.mix(c2,0.1*r[2]);
	b.toString(cstr);
	ice+=cstr;
	ice+=",";
	b=Color("Color(0.4,0.8,0.8)");
	b=b.mix(tc,0.5*r[2]);
	b.toString(cstr);
	ice+=cstr;
	ice+=",273,1.0,0.95,40,1,0.2,";
	ice+=def_solid_func;
	ice+=")";
	
	strcpy(buff,ice.c_str());
	cout<<buff<<endl;

}
NodeIF *SolidState::newInstance(){
	char buff[1024];
	buff[0]=0;
	newInstance(buff);
	NodeIF *c=TheScene->parse_node(buff);
	return c;	
}

