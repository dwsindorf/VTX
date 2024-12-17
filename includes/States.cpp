#include "States.h"
#include "SceneClass.h"
#include "TerrainClass.h"

//water(noise(GRADIENT|SCALE,17.7,3,-0.34,0.5,2.08,0.22,1,0,0),Color(0.827,0.863,0.996),Color(0.027,0.247,0.220),762.618,100,1)
//ice(noise(GRADIENT|NABS|SCALE|SQR,15.2,8.6,0.1,0.4,1.84,0.73,-0.34,0,0),Color(1.000,1.000,1.000,0.173),Color(0.400,0.667,0.808),15.8451,15.278,0.625)
//Ocean("H2O")[liquid(Color(0,1,1,0.2),Color(0.1,0.1,0.5),0.00,500.00,0.80,10.00,noise(GRADIENT|NABS|SQR|RO1,0.7,10,1,0.5,2.22,1,1,0,0,1e-06)),
//solid(Color(1,1,1,0.6),Color(0.4,0.675,0.8),100.00,1.00,0.80,10.00,noise(GRADIENT|NABS|SCALE|SQR|RO1,15.2,8.6,0.1,0.4,1.84,0.73,-0.34,0,0))]

static char* def_liquid_func="noise(GRADIENT|SCALE,17.4,3,1,0.5,2.08,0.11,1,0,0)";
static char* def_solid_func="noise(GRADIENT|NABS|SCALE|SQR,15.2,8.6,0.1,0.4,1.84,0.73,-0.34,0,0)";
static char* def_ocean_func="noise(GRADIENT|RO1,2,10,1,0.5,2,0.7,1,0,0,1e-06)";
//noise(GRADIENT|RO1,2,10,1,0.5,2,1,1,0,0,1e-06)
Array<OceanState*> OceanState::oceanTypes(6);

static char *H2O_liq_str="liquid(Color(0,1,1,0.2),Color(0.1,0.1,0.5),100,500,0.95,100,1)";
static char *H2O_sol_str="solid(Color(1,1,1,0.6),Color(0.400,0.675,0.8),0,1.0,0.95,0.8,0.6)";

void OceanState::setDefaults(){
	char str[1024];
	sprintf(str,"Ocean(\"%s\")[%s,%s]","H2O",H2O_liq_str,H2O_sol_str);
	cout<<str<<endl;
	if(oceanTypes.size==0){
		OceanState *test=(OceanState*)TheScene->parse_node((char*)str);
		if(test){
			oceanTypes.add(test);
			test->valueString(str);
			cout<<str<<endl;
		}
	}
}
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
	
	double vals[6];
	TNarg *a=args.index(2);
	int n=getargs(a,vals,6);
	temp=C2K(vals[0]);
	clarity=vals[1]*FEET;
	mix=vals[2];
	specular=vals[3];
	shine=vals[4];
	expr[0]=0;
}

LiquidState::LiquidState(TNode *a) : MaterialState(a){
	TNarg &args=*((TNarg *)right);
	if(args[8]){
		args[8]->valueString(expr);
	}
	else
		setExpr(def_liquid_func);
}
SolidState::SolidState(TNode *a) : MaterialState(a){
	TNarg &args=*((TNarg *)right);
	if(args[8])
		args[8]->valueString(expr);
	else
		setExpr(def_solid_func);

}
void MaterialState::print(char *s){
	char str[512];
	sprintf(str,"%s Temp:%-4.0fC Color1(%1.2f,%1.2f,%1.2f,%1.2f) Color2(%1.2f,%1.2f,%1.2f,%1.2f)",s,K2C(temp),
			color1.red(),color1.green(),color1.blue(),color1.alpha(),
			color2.red(),color2.green(),color2.blue(),color2.alpha());
	cout<<str<<endl;
	cout<<expr<<endl;
}
void MaterialState::valueString(char *s){
	char col1[256]={0};
	char col2[256]={0};
	
	TNarg &args=*((TNarg *)right);
	args[0]->valueString(col1);
	args[1]->valueString(col2);
	double arg[6];
	TNarg *a=args.index(2);
	int n=getargs(a,arg,6);
	if(strlen(name)>0)
		sprintf(s,"%s(\"%s\",%s,%s,%1.2f,%1.2f,%1.2f,%1.2f",symbol(),name,col1,col2,arg[0],arg[1],arg[3],arg[4],arg[5]);
	else
		sprintf(s,"%s(%s,%s,%1.2f,%1.2f,%1.2f,%1.2f",symbol(),col1,col2,arg[0],arg[1],arg[3],arg[4],arg[5]);
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

OceanState::OceanState(TNode *l,LiquidState *m1, SolidState *m2): TNfunc(l,0){
	TNarg *arg=left;
	TNarg *node=arg->left;
	if(node->typeValue() == ID_STRING){		
		setName(((TNstring*)node)->value);
		left=arg->next();
		if(left){
			left->setParent(this);
			arg->right=0;
		}
		delete arg;	
	}
	liquid=m1;
	solid=m2;
}

void OceanState::saveNode(FILE *f){
	char str[2048];
	str[0]=0;
	valueString(str);
	cout<<str<<endl;
	fprintf(f,"%s",str);
}
void OceanState::print(){
	liquid->print("liquid");
	solid->print("solid ");
}
void OceanState::valueString(char *s){
	char str1[256];
	char str2[256];
	liquid->valueString(str1);
	solid->valueString(str2);
	sprintf(s,"Ocean(\"%s\")[%s,\n%s]",nodeName(),str1,str2);
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
