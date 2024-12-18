#include "States.h"
#include "SceneClass.h"
#include "TerrainClass.h"

static char* def_liquid_func="noise(GRADIENT|SCALE,17.4,3,1,0.5,2.08,0.11,1,0,0)";
static char* def_solid_func="noise(GRADIENT|NABS|SCALE|SQR,15.2,8.6,0.1,0.4,1.84,0.73,-0.34,0,0)";
static char* def_ocean_func="noise(GRADIENT|RO1,2,10,1,0.5,2,0.7,1,0,0,1e-06)";
//noise(GRADIENT|RO1,2,10,1,0.5,2,1,1,0,0,1e-06)
Array<OceanState*> OceanState::oceanTypes(6);

static char *H2O_liq_str="liquid(Color(0,1,1,0.2),Color(0.1,0.1,0.5),100,300,0.95,100,1)";
static char *H2O_sol_str="solid(Color(1,1,1,0.6),Color(0.400,0.675,0.8),0,1.0,0.95,0.8,0.6)";

//#define TEST
void OceanState::setDefaults(){
	char str[1024];
	sprintf(str,"Ocean(\"%s\")[%s,%s]","H2O",H2O_liq_str,H2O_sol_str);
	//cout<<str<<endl;
	if(oceanTypes.size==0){
		OceanState *water=(OceanState*)TheScene->parse_node((char*)str);
		if(water){
			str[0]=0;
			water->valueString(str);
			cout<<str<<endl;
			oceanTypes.add(water);
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
	temp=vals[0];
	clarity=vals[1];
	mix=vals[2];
	shine=vals[3];
	specular=vals[4];
	expr[0]=0;
}

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
	b=b.mix(c1,0.8*r[2]);
	b=b.darken(0.3);
	b.toString(cstr);
	water+=cstr;
	water+=",100,300,0.95,100,1,";
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
	Color b=Color("Color(1.000,1.000,1.000,0.173)");
	b=b.mix(c2,0.1*r[2]);
	b.toString(cstr);
	ice+=cstr;
	ice+=",";
	b=Color("Color(0.4,0.8,0.8)");
	b=b.mix(tc,0.5*r[2]);
	b.toString(cstr);
	ice+=cstr;
	ice+=",100,0.3,40,1,";
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
NodeIF *OceanState::newInstance(){
	char buff[1024];
	char lbuff[1024];
	char sbuff[1024];
	buff[0]=lbuff[0]=sbuff[0]=0;
	LiquidState::newInstance(lbuff);
	SolidState::newInstance(sbuff);
    sprintf(buff,"Ocean(\"test\")[%s,\n%s]",lbuff,sbuff);
    cout<<"OceanState::newInstance"<<endl;

    NodeIF *c=TheScene->parse_node(buff);
	return c;
}
void MaterialState::valueString(char *s){
	char col1[256]={0};
	char col2[256]={0};
	color1.toString(col1);
	color2.toString(col2);
	
	if(strlen(name)>0)
		sprintf(s,"%s(\"%s\",%s,%s,%g,%g,%g,%g,%g",symbol(),name,col1,col2,temp,clarity,mix,shine,specular);
	else
		sprintf(s,"%s(%s,%s,%g,%g,%g,%g,%g",symbol(),col1,col2,temp,clarity,mix,shine,specular);
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
	fprintf(f,"%s",str);
}

void OceanState::valueString(char *s){
	char str1[512];
	char str2[512];
	str1[0]=0;
	str2[0]=0;
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

