#include "States.h"

static const char *def_ocean_expr="noise(GRADIENT|NNORM|SCALE|RO1,5,9.5,1,0.5,2,0.4,1,0,0)";

static char* def_liquid_func="noise(GRADIENT|NABS|SQR|RO1,0.7,10,1,0.5,2.22,1,1,0,0,1e-06)";
static char* def_solid_func="noise(GRADIENT|NABS|SCALE|SQR|RO1,15.2,8.6,0.1,0.4,1.84,0.73,-0.34,0,0)";
static char* def_ocean_func="noise(GRADIENT|SCALE|RO1,5,9.5,1,0.5,2,0.4,1,0,0)";
Array<OceanState*> OceanState::oceanTypes(6);

static MaterialState H2O_liquid(C2K(0),Color(0,1,1,0.2),Color(0.1,0.1,0.5),500*FEET,0.95,0.8,10);
static MaterialState H2O_solid(C2K(100),Color(1,1,1,0.6),Color(0.400,0.675,0.8),1*FEET,0.95,0.8,10);
static OceanState Water("water",&H2O_liquid,&H2O_solid);

MaterialState::MaterialState(double t,Color c1, Color c2,double c, double m, double sp, double sh){
	color1=c1;color2=c2,clarity=c;mix=m,specular=sp;shine=sh;temp=t;
	expr[0]=0;
}
void MaterialState::print(char *s){
	char str[512];
	sprintf(str,"%s Temp:%-4.0fC Color1(%1.2f,%1.2f,%1.2f,%1.2f) Color2(%1.2f,%1.2f,%1.2f,%1.2f)",s,K2C(temp),
			color1.red(),color1.green(),color1.blue(),color1.alpha(),
			color2.red(),color2.green(),color2.blue(),color2.alpha());
	cout<<str<<endl;
	cout<<expr<<endl;
}
OceanState::OceanState(char *n, MaterialState *m1, MaterialState *m2){
	strcpy(name,n);
	liquid=*m1;
	solid=*m2;
	liquid.setExpr(def_liquid_func);
	solid.setExpr(def_solid_func);
	
}
void OceanState::print(){
	cout<<name<<endl;
	liquid.print("liquid");
	solid.print("solid ");
}
void OceanState::init(){
	if(oceanTypes.size==0){
		oceanTypes.add(&Water);
	}
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
