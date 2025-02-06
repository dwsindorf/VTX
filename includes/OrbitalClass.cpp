 // OrbitalClass.cpp
#include "SceneClass.h"
#include "RenderOptions.h"
#include "Effects.h"
#include "matrix.h"
#include "MapClass.h"
#include "MapNode.h"
#include "Octree.h"
#include "TerrainClass.h"
#include "ImageClass.h"
#include "ImageMgr.h"
#include "GLSLMgr.h"
#include "AdaptOptions.h"
#include "FileUtil.h"
#include "UniverseModel.h"

extern	void rebuild_scene_tree();
extern	void select_tree_node(NodeIF *n);
extern void setCenterText(char *text);

extern double Theta,Phi,Radius,Sfact;

#define TEST
//#define GEOMETRY_TEST
#define WRITE_STAR_DATA
//#define DEBUG_RENDER
#define DEBUG_TEMP 1
#define DEBUG_AVE_TEMP
//#define DEBUG_GENERATE

static int debug_temp=DEBUG_TEMP;

extern const char *pstg[];

static void show_render_state(Object3D *obj){
#ifdef DEBUG_RENDER
	const char *side="";
	if(obj==TheScene->viewobj)
		side="viewobj";
	else if(obj->getParent() == TheScene->viewobj){
		if(obj->inside())
			side="inside";
		else if(obj->outside())
			side= "outside";
	}
	cout<<pstg[TheScene->bgpass]<<" " << obj->name() << " " << side << endl;
#endif
}

#ifndef WRITE_STAR_DATA
#include "star_data.h"
#endif

extern double Gscale,Hscale,Rscale,Temp,Tave,Tsol,Tgas,Sfact;
extern void inc_tabs();
extern void dec_tabs();
extern char tabs[];
extern void d2f(double doubleValue, float  &floatHigh, float &floatLow,double scale);
//#define DEBUG_OBJS
//#define DEBUG_BASE_OBJS

//#define DEBUG_SAVE  // show name on save

#define TDGET(name,value) if(exprs.get_local(name,Td)) tree_mgr->value=Td.s
#define TCGET(name,value) if(exprs.get_local(name,Td)) tree_mgr->value=Td.c
#define TSET(s,x) exprs.set_var(s, tree_mgr->x, tree_mgr->x != tree_defs->x)

static TerrainSym *ts=0;
static TerrainData Td;
enum {GAS=0,LIQUID=1,SOLID=2};
static int       def_ocean_state=LIQUID;
static int       def_ocean_auto=0;

static Color 	def_haze_color=WHITE;
static double  	def_haze_value=0;
static double  	def_haze_min=100*FEET;
static double  	def_haze_max=10*MILES;
static double  	def_haze_height=0.1;
static Color 	def_fog_color=WHITE;
static double  	def_fog_min=10*FEET;
static double  	def_fog_max=1000*FEET;
static double  	def_fog_vmin=0;
static double  	def_fog_vmax=1000*FEET;
static double  	def_fog_value=0;
static double  	def_fog_glow=0.2;
static double  	def_temp_factor=0.1;
static double  	def_season_factor=0.5;

static Color 	def_twilite_color=BLACK;
static double	def_twilite_value=1;
static double	def_twilite_max=0.2;
static double	def_twilite_min=-0.2;
static Color	def_shadow_color=Color(0,0,0,0.7);
static double	def_symmetry=1;
static double	def_hscale=5e-4;

//************************************************************
// Orbital class
//************************************************************

double NodeIF::r[RANDS];
double NodeIF::s[RANDS];

LinkedList<double>Orbital::seedlist;

void Orbital::pushSeed(){
	TheNoise.rseed=rseed+TheScene->rseed;
	seedlist.push(rseed);
}
void Orbital::popSeed(){
	rseed=seedlist.pop();
	TheNoise.rseed=rseed+TheScene->rseed;
}
void Orbital::setSeed(double val){
	rseed=val;
	TheNoise.rseed=rseed+TheScene->rseed;
}
void Orbital::setDefault(){
	setSeed(0);
}

// constructors

Orbital::Orbital(double s) : ObjectNode(s)
{
	set_defaults();
}

Orbital::Orbital(Orbital *m) : ObjectNode(m)
{
	set_defaults();
	if(m)
		exprs.parent=&(m->exprs);
}

Orbital::Orbital(Orbital *m, double s) : ObjectNode(m,s)
{
	set_defaults();
	if(m)
		exprs.parent=&(m->exprs);
}

Orbital::Orbital(Orbital *m, double s, double r)
		: ObjectNode(m,s)
{
	set_defaults();
	orbit_radius=r;
	if(m)
		exprs.parent=&(m->exprs);
	set_cargs(2);
}

Orbital::~Orbital(){ }

void Orbital::set_defaults()
{
	orbit_skew=orbit_angle=orbit_phase=orbit_radius=0.0,orbit_eccentricity=0;
	year=day=tilt=pitch=rot_angle=rot_phase=0.0;
	detail=1;
	set_color(WHITE);
	name_str[0]=0;
	exprs.setParent(this);
	exprs.setFlag(NODE_HIDE);
	ambient=Color(1,1,1,DEFAULT_AMBIENT);
	emission=Color(1,1,1,0);
	diffuse=Color(1,1,1,1);
	specular=Color(1,1,1,DEFAULT_SPECULAR);
	albedo=DEFAULT_ALBEDO;
	shine=DEFAULT_SHINE;
	sunset=0.2;
	shadow_intensity=1;
	rseed=0;
	Gscale=1;
}

void Orbital::set_lighting(){
    if(TheScene->render_mode()){
        Lights.defaultLighting();
        Lights.setDiffuse(1.0);
    	Lights.setEmission(emission);
    	Lights.setSpecular(specular.alpha());
    	Lights.setShininess(shine);
    }
}

void Orbital::set_wscale(){
	float wscale=0.5*TheScene->window_width*size/tan(RPD*TheScene->fov/2);
	GLSLMgr::wscale=wscale;
}

void Orbital::set_tilt() {}
void Orbital::orient() {}
void Orbital::setMatrix() {TheScene->set_matrix(this);}

void Orbital::set_rotation() {}
void Orbital::init_view() { TheScene->maxht=0;}
Bounds *Orbital::bounds(){return 0;}
TNode *Orbital::set_terrain(TNode *r)	{return 0;}
Color Orbital::color()				{ return _color;}
void Orbital::set_color(Color c)	{ _color=c;}
int Orbital::getChildren(LinkedList<NodeIF*>&l){
	int n=exprs.getChildren(l);
    n+=ObjectNode::getChildren(l);
	return n;
}
void Orbital::getDateString(char *s) {*s=0;}
void Orbital::getTempString(char *s) {*s=0;}
double Orbital::getTemperature() {return 0;}

void Orbital::free() {}
void Orbital::set_surface(TerrainData &d){}
void Orbital::set_mode(int d) {}
void Orbital::set_debug(int d) {}
TNode  *Orbital::add_expr(int t, char *s, TNode *r)
{
	TNode  *node=exprs.add_expr(s,r);
	return node;
}
TNode  *Orbital::add_image(TNode *r)
{
	TNode  *node=exprs.add_image(r);
	return node;
}
TNinode  *Orbital::get_image(char *s)
{
	return exprs.get_image(s);
}
TNinode  *Orbital::replace_image(TNinode *i, TNinode *r){
	return exprs.replace_image(i,r);
}

TNvar *Orbital::addExprVar(const char *name,const char *expr){
	char buff[2048];
	strcpy(buff,expr);
	char *var_name;
	MALLOC(32, char, var_name);
	strcpy(var_name, name);
	TNode *val=TheScene->parse_node(buff);
	TNvar *var=(TNvar*)exprs.add_expr(var_name,val);
	if(!var)
		cout<<"parser error:"<<expr<<endl;
	return (TNvar *)var;
}

void Orbital::setOrbitFrom(Orbital *p){
	rot_angle=p->rot_angle;
	orbit_angle=p->orbit_angle;
	rot_phase=p->rot_phase;
	orbit_phase=p->orbit_phase;
	year=p->year;
	day=p->day;
	tilt=p->tilt;
	pitch=p->pitch;
	orbit_skew=p->orbit_skew;
	orbit_eccentricity=p->orbit_eccentricity;
	orbit_radius=p->orbit_radius;
	origin=p->origin;
}

//-------------------------------------------------------------
// Orbital::NodeIF methods
//-------------------------------------------------------------
const char *Orbital::typeName() {
	return name();
}
//-------------------------------------------------------------
char *Orbital::nodeName() {
	return name_str;
}
//-------------------------------------------------------------
//-------------------------------------------------------------
NodeIF *Orbital::addChild(NodeIF *c){
   if(c->typeClass()&ID_OBJECT)
   	   ObjectNode::addChild(c);
   else if(c->typeClass()==ID_TERRAIN)
	   exprs.addChild(c);
   return c;
}

//-------------------------------------------------------------
NodeIF *Orbital::addAfter(NodeIF *b,NodeIF *c){
	if(c->typeClass()&ID_OBJECT)
		ObjectNode::addAfter(b,c);
	else if(c->typeClass()==ID_TERRAIN){
		if(b->expanded() || b->isEmpty())
			b->addChild(c);
		else
			exprs.addChild(c);
	}
	return c;
}

//-------------------------------------------------------------
NodeIF *Orbital::removeChild(NodeIF *c){
	if(c->typeClass()&ID_OBJECT){
  		ObjectNode::removeChild(c);
  		if(TheScene->viewobj==c){
  		    TheScene->viewobj=(ObjectNode*)c->getParent();
  		    TheScene->selobj=TheScene->vobj=TheScene->viewobj;
  		    TheScene->focusobj=0;
  		    TheScene->change_view(ORBITAL);
  		    TheScene->views_reset();
  		}
  	}
	else if(c->typeClass()==ID_TERRAIN)
        exprs.removeChild(c);
	return c;
}
//-------------------------------------------------------------
NodeIF *Orbital::replaceChild(NodeIF *c,NodeIF *n){
	if(c->typeClass()&ID_OBJECT){
  		ObjectNode::replaceChild(c,n);
		if(TheScene->viewobj==c){
			TheScene->viewobj=(ObjectNode*)n;
			TheScene->selobj=TheScene->viewobj;
		}
  	}
   	else if(c->typeClass()==ID_TERRAIN)
        exprs.replaceChild(c,n);
	return n;
}

//-------------------------------------------------------------
// Orbital::setName() set user name
//-------------------------------------------------------------
void Orbital::setName(char *s) {
	strncpy(name_str,s,maxstr);
	exprs.set_var("name",name_str,1);
}

//-------------------------------------------------------------
// Orbital::getName() get user name
//-------------------------------------------------------------
char *Orbital::getName() {
	if(exprs.get_local("name",Td)){
		return Td.string;
	}
    return 0;
}

//-------------------------------------------------------------
// Orbital::select() select children
//-------------------------------------------------------------
void Orbital::select()
{
	visit(&Object3D::select);
}

//-------------------------------------------------------------
// Orbital::adapt() adapt children
//-------------------------------------------------------------
void Orbital::adapt()
{
	visit(&Object3D::adapt);
}

//-------------------------------------------------------------
// Orbital::render() render children
//-------------------------------------------------------------
void Orbital::render()
{
	visit(&Object3D::render);
}

//-------------------------------------------------------------
// Orbital::set_lights() set light sources
//-------------------------------------------------------------
void Orbital::set_lights()
{
	visit(&Object3D::set_lights);
}

//-------------------------------------------------------------
// Orbital::init() set common variables
//-------------------------------------------------------------
void Orbital::init()
{
	exprs.init();
	get_vars();
}

//-------------------------------------------------------------
// Orbital::get_vars() reserve interactive variables
//-------------------------------------------------------------
void Orbital::get_vars()
{
	if(exprs.get_local("name",Td))
		strncpy(name_str,Td.string,maxstr);

	VGET("tilt",tilt,0);
	VGET("pitch",pitch,0);
	VGET("skew",orbit_skew,0);
	VGET("eccentricity",orbit_eccentricity,0);
	VGET("phase",orbit_phase,0);
	VGET("rot_phase",rot_phase,0);

	VGET("day",day,0);
	VGET("year",year,0);
	VGET("resolution",detail,1);
	if(exprs.get_local("color",Td))
		set_color(Td.c);
	else
		set_color(WHITE);
	if(exprs.get_local("ambient",Td))
		ambient=Td.c;
	else
		ambient=Color(1,1,1,DEFAULT_AMBIENT);
	if(exprs.get_local("emission",Td))
		emission=Td.c;
	else
		emission=Color(1,1,1,0);
	if(exprs.get_local("diffuse",Td))
		diffuse=Td.c;
	else
		diffuse=Color(1,1,1,1);
	if(exprs.get_local("specular",Td))
		specular=Td.c;
	else{
		specular=Color(1,1,1,DEFAULT_SPECULAR);
	}
	VGET("albedo",albedo,DEFAULT_ALBEDO);
	VGET("shine",shine,DEFAULT_SHINE);
	VGET("sunset",sunset,0.2);
	VGET("rseed",rseed,0.0);
}

//-------------------------------------------------------------
// Orbital::set_vars() set common variables
//-------------------------------------------------------------
void Orbital::set_vars()
{
	exprs.set_var("name",name_str,name_str[0]!=0);

	VSET("tilt",tilt,0);
	VSET("pitch",pitch,0);
	VSET("skew",orbit_skew,0);
	VSET("eccentricity",orbit_eccentricity,0);
	VSET("phase",orbit_phase,0);
	VSET("rot_phase",rot_phase,0);
	VSET("day",day,0);
	VSET("year",year,0);
	VSET("resolution",detail,1);
	CSET("color",color(),WHITE);
	CSET("ambient",ambient,Color(1,1,1,DEFAULT_AMBIENT));
	CSET("emission",emission,Color(1,1,1,0));
	CSET("specular",specular,Color(1,1,1,DEFAULT_SPECULAR));
	CSET("diffuse",diffuse,Color(1,1,1,1));
	VSET("albedo",albedo,DEFAULT_ALBEDO);
	VSET("shine",shine,DEFAULT_SHINE);
	VSET("sunset",sunset,0.2);
	VSET("rseed",rseed,0.0);
}

//-------------------------------------------------------------
// Orbital::randomize() randomize noise seed
//-------------------------------------------------------------
bool Orbital::randomize()
{
	if(!canRandomize())
		return false;
	setRseed(getRandValue());
	cout<<typeName()<<" "<<getRseed()<<endl;

	invalidate();
	visitChildren(&Object3D::invalidate);
	return true;
 }

//-------------------------------------------------------------
// Orbital::save() 	archiving routine
//-------------------------------------------------------------
void Orbital::save(FILE *fp)
{
	switch(cargs()){
	case 0:
		fprintf(fp,"() {\n");
	    break;
	case 1:
		fprintf(fp,"(%g) {\n",size);
	    break;
	case 2:
		fprintf(fp,"(%g,%g) {\n",size,orbit_radius);
	    break;
	}
    inc_tabs();
	set_vars();
#ifdef DEBUG_SAVE
	printf("save %-12s\n",name());
	exprs.save(fp);
	printf("\n");
#else
	exprs.save(fp);
#endif
}

//-------------------------------------------------------------
// Orbital::locate() set point to global coordinates
//-------------------------------------------------------------
void Orbital::locate()
{
	visit(&Object3D::locate);
}

//-------------------------------------------------------------
// Orbital::visit() visit the objects children & apply function
//-------------------------------------------------------------
void Orbital::visit(void (Object3D::*func)())
{
	Object3D *obj;

	children.ss();
	while(obj=children++){
		TheScene->pushMatrix();
		(obj->*func)();
		TheScene->popMatrix();
	}
}

//-------------------------------------------------------------
// Orbital::visit() visit the objects children & apply function
//-------------------------------------------------------------
void Orbital::visit(void (*func)(Object3D*))
{
	Object3D *obj;
	(*func)(this);
	if(children.size){
		children.ss();
		while(obj=children++){
			TheScene->pushMatrix();
			obj->visit(func);
			TheScene->popMatrix();
		}
	}
}

//-------------------------------------------------------------
// Orbital::animate() adjust orbit and rotation with time
//-------------------------------------------------------------
void Orbital::animate()
{
	double secs=0,cycles=0;

	if(year){
		secs=24*3600*year; // earth normal days
		cycles=TheScene->ftime/secs;
		// angle around focal point (as if a circular orbit)
		orbit_angle=P360(orbit_phase+360*cycles);

		double r=orbit_radius;
		double e=orbit_eccentricity;

        double c=cos(RPD*orbit_angle);
        double b=(c+e)/(1+c*e);//converts circular angle to elliptical angle
        double x=DPR*acos(b); // angle around COM

        if(orbit_angle>180) //fixes transition direction change in cosine
        	x=360-x;

		orbit_angle=x;
		c=cos(RPD*orbit_angle); // recalculate rotation angle
		orbit_distance=r*(1-e*e)/(1-e*c);

	}
	if(day){
		secs=day*3600;
		// back off 1 cycle per year (if year==day orbital is tidal locked)
		cycles-=TheScene->ftime/secs;
		rot_angle=P360(rot_phase+360*cycles);
	}
}


//-------------------------------------------------------------
// Orbital::set_ref() move to orbital position
//-------------------------------------------------------------
void Orbital::set_ref()
{
	animate();
	if(orbit_skew)
		TheScene->rotate(orbit_skew,1.0,0.0,0.0);
	if(orbit_angle)
		TheScene->rotate(orbit_angle,0.0,1.0,0.0);
	if(orbit_radius)
		TheScene->translate(0,0,orbit_distance);
}

//-------------------------------------------------------------
// Orbital::set_view()   		set eye matrix
//-------------------------------------------------------------
void Orbital::set_view()
{
	set_ref();
	set_tilt();
	set_rotation();
}

bool Orbital::isViewobj(){
	return TheScene->viewobj == this;
}

bool Orbital::containsViewobj(){
	if(isViewobj())
		return true;
	children.ss();
	Orbital *obj;
	while(obj=children++){
		if(obj->containsViewobj())
			return true;
	}
	return false;
}

//-------------------------------------------------------------
// Scene::getInstance() return prototype or randomly selected object
//-------------------------------------------------------------
NodeIF *Orbital::getInstance(){
	return TheScene->getInstance(type());
}

//************************************************************
// Universe class
//************************************************************
Universe::Universe(double s) : Orbital(1.0)
{
#ifdef DEBUG_OBJS
	printf("Universe\n");
#endif
	all_exclude();
	setFlag(NODE_HIDE);
	set_far();
}

//-------------------------------------------------------------
// Universe::save() archive the object and it's children
//-------------------------------------------------------------
void Universe::save(FILE *fp)
{
	fprintf(fp,"%s%s {\n",tabs,name());
    inc_tabs();
	ObjectNode::save(fp);
    dec_tabs();
	fprintf(fp,"%s}\n",tabs);
}

//-------------------------------------------------------------
NodeIF *Universe::addChild(NodeIF *c){
   	return ObjectNode::addChild(c);
}

//-------------------------------------------------------------
NodeIF *Universe::addAfter(NodeIF *b,NodeIF *c){
	//ObjectNode::addAfter(b,c);
	return ObjectNode::addChild(c);
}

NodeIF *Universe::replaceChild(NodeIF *c,NodeIF *n){
	((Orbital*)n)->setOrbitFrom((Orbital*)c);
	if(c->hasChild(TheScene->viewobj)){
		Orbital * m=(Orbital *)c;
		m->children.remove(TheScene->viewobj);
		n->addChild(TheScene->viewobj);
	}
	Orbital *m=(Orbital*)Orbital::replaceChild(c,n);
	cout<<"Universe::replaceChild"<<endl;
	return m;
}

//************************************************************
// DensityCloud class
//************************************************************
DensityCloud::DensityCloud(double s) : Orbital(s)
{
#ifdef DEBUG_OBJS
	printf("DensityCloud\n");
#endif
	tree=0;
	noise_expr=0;
	color_expr=0;
	set_far();
	all_exclude();
	exprs.setParent(this);

}
DensityCloud::DensityCloud(Orbital *m,double s) : Orbital(m,s)
{
#ifdef DEBUG_OBJS
	printf("DensityCloud\n");
#endif
	tree=0;
	noise_expr=0;
	color_expr=0;
	all_exclude();
	set_far();
	exprs.setParent(this);
}

DensityCloud::~DensityCloud()
{
	if(tree)
		delete tree;
	tree=0;
#ifdef DEBUG_OBJS
	printf("~DensityCloud\n");
#endif
}

void DensityCloud::setMatrix(){
	TheScene->set_matrix(0);
}
//-------------------------------------------------------------
// DensityCloud::animate() adjust rotation with time
//-------------------------------------------------------------
void DensityCloud::animate()
{
	// units are 1000 years
	const double tscale=3600*360*1000;
	if(day){
		double secs=day*tscale;
		double cycles=TheScene->ftime/secs;
		rot_angle=360*(cycles-floor(cycles));
	}
}

//-------------------------------------------------------------
// DensityCloud::cells() return cells in tree
//-------------------------------------------------------------
int DensityCloud::cells()
{
	return tree?tree->nodes:0;
}

//-------------------------------------------------------------
// DensityCloud::scale() return znear zfar
//-------------------------------------------------------------
int DensityCloud::scale(double &n, double &f)
{
    int t=tree->scale(n,f);
    setvis(t);
	return t;
}

//-------------------------------------------------------------
// DensityCloud::set_ref() move to orbital position
//-------------------------------------------------------------
void DensityCloud::set_ref()
{
	if(tilt)
		TheScene->rotate(tilt,1,0,0);	// rotate about x
	if(pitch)
		TheScene->rotate(pitch,0,0,1);	// rotate about z
}

//-------------------------------------------------------------
// DensityCloud::adapt() adapt grid
//-------------------------------------------------------------
void DensityCloud::adapt()
{
 	if(included()){
		set_ref();
		TheScene->pushMatrix();
		TheScene->set_matrix(0);
		//setMatrix();
		tree->adapt();
		TheScene->popMatrix();
	}
	Orbital::adapt();
}

//-------------------------------------------------------------
// DensityCloud::render() render
//-------------------------------------------------------------
void DensityCloud::render()
{
 	if(included()){
		set_ref();
		TheScene->pushMatrix();
		TheScene->set_matrix(0);
		//setMatrix();
		render_object();
		TheScene->popMatrix();
	}
	Orbital::render();
}

//-------------------------------------------------------------
// DensityCloud::set_surface() set voxel properties
//-------------------------------------------------------------
void DensityCloud::set_surface(TerrainData &data)
{

	Point ps=data.p.spherical();

	TheNoise.theta=ps.x;
	TheNoise.phi=ps.y;
	Theta=ps.x;
	Phi=ps.y;
	Radius=ps.z;
	CurrentScope=&exprs;

	TheNoise.offset=TheNoise.scale=0.5;

	Point pr=data.p+0.5;
	TheNoise.set(pr);
	INIT;

	S0.c=data.c;
	S0.p=Point(0,0,0);
	if(exprs.zpass()){
		noise_expr->eval();
		S0.density=S0.s;
	}
	else{
		if(color_expr){
			color_expr->eval();
			S0.density=S0.s;
			S0.c=color_list->color(tree->color_bias*S0.s);
			//cout<<S0.c.alpha()<<endl;

			//S0.c=WHITE*(1-tree->color_mix)+S0.c*(tree->color_mix);
			//c=c*(1-galaxy->color_mix)+S0.c*(galaxy->color_mix);
		}
	}
	data=S0;
}

//-------------------------------------------------------------
// DensityCloud:adapt_object() adapt_object object
//-------------------------------------------------------------
void DensityCloud::adapt_object()
{
	exprs.set_zpass();
	exprs.eval();
	tree->adapt();
}

//-------------------------------------------------------------
// DensityCloud::render_object() render object
//-------------------------------------------------------------
void DensityCloud::render_object()
{
	 exprs.set_cpass();
	 tree->render();
}

//------------------------------------------------------------
// DensityCloud::set_vars() set local variables
//-------------------------------------------------------------
void DensityCloud::set_vars()
{
    Orbital::set_vars();
    exprs.set_var("origin",origin/LY,origin.length()>0);

	DensityTree *tree_defs=defaults();
	DensityTree *tree_mgr=tree;

	TSET("density",density_value);
	TSET("scatter",dispersion);
	TSET("diffusion",diffusion);
	TSET("radius",outer_radius);
	TSET("radius.outer",outer_radius);
	TSET("radius.inner",inner_radius);
	TSET("gradient",gradient);
	TSET("compression",compression);
	TSET("twist.rotation",twist_angle);
	TSET("twist.radius",twist_radius);
	TSET("noise.cutoff",noise_cutoff);
	TSET("noise.vertical",noise_vertical);
	TSET("noise.amplitude",noise_amplitude);
	TSET("center.radius",center_radius);
	TSET("center.bias",center_bias);
	TSET("shape.exterior",shape_exterior);
	TSET("shape.interior",shape_interior);
	TSET("shape.sharpness",shape_sharpness);
	TSET("shape.solidity",shape_solidity);
	TSET("color.mix",color_mix);
	TSET("color.bias",color_bias);
}

//-------------------------------------------------------------
// DensityCloud::get_vars() reserve interactive variables
//-------------------------------------------------------------
void DensityCloud::get_vars()
{
	Orbital::get_vars();
	if(exprs.get_local("origin",Td)){
		origin=(Td.p)*LY;
		tree->origin=origin;
	}
	DensityTree *tree_mgr=tree;
	TDGET("density",density_value);
	TDGET("scatter",dispersion);
	TDGET("diffusion",diffusion);
	TDGET("radius",outer_radius);
	TDGET("radius.outer",outer_radius);
	TDGET("radius.inner",inner_radius);
	TDGET("gradient",gradient);
	TDGET("compression",compression);
	TDGET("twist.rotation",twist_angle);
	TDGET("twist.radius",twist_radius);
	TDGET("noise.cutoff",noise_cutoff);
	TDGET("noise.vertical",noise_vertical);
	TDGET("noise.amplitude",noise_amplitude);
	TDGET("center.radius",center_radius);
	TDGET("center.bias",center_bias);
	TDGET("shape.exterior",shape_exterior);
	TDGET("shape.interior",shape_interior);
	TDGET("shape.sharpness",shape_sharpness);
	TDGET("shape.solidity",shape_solidity);
	TDGET("color.mix",color_mix);
	TDGET("color.bias",color_bias);

	char *var_name;
	TNode *val;
	TNvar *var;
	CurrentScope=&exprs;

	var=exprs.getVar((char*)"noise.expr");
	if(!var){
		var=addExprVar("noise.expr",tree->def_noise_expr);
		var->eval();
	}
	noise_expr=var->right;

	var=exprs.getVar((char*)"color.expr");
	if(!var){
		var=addExprVar("color.expr",tree->def_color_expr);
		var->eval();
	}
	if(var){
 		var->eval();
		color_expr=var->right;
	}

	var=exprs.getVar((char*)"color.list");
	if(!var){
        val=(TNode*)TheScene->parse_node((char*)tree->def_color_list);
		MALLOC(15,char,var_name);
		strcpy(var_name,"color.list");
		var=(TNvar*)exprs.add_expr(var_name,val);
		var->eval();
	}
	if(var){
 		var->init();
		color_list=(TNclist*)var->right;
	}
}

//------------------------------------------------------------
// DensityCloud::setDefaultNoise() set default noise
//-------------------------------------------------------------
void DensityCloud::setDefaultNoise()
{
	setNoiseFunction((char*)tree->def_noise_expr);
}

//------------------------------------------------------------
// DensityCloud::getNoiseFunction() return noise function
//-------------------------------------------------------------
int DensityCloud::getNoiseFunction(char *buff)
{
	TNvar *var=exprs.getVar((char*)"noise.expr");
	TNode *expr=var->getExprNode();
	if(!expr)
		expr=var->right;

	buff[0]=0;
	expr->valueString(buff);
	return 1;
}

//------------------------------------------------------------
// DensityCloud::setNoiseFunction() set noise function
//-------------------------------------------------------------
void DensityCloud::setNoiseFunction(char *expr)
{
	TNvar *var=exprs.getVar((char*)"noise.expr");
	var->setExpr(expr);
}

//------------------------------------------------------------
// DensityCloud::applyNoiseFunction() set & apply noise function
//-------------------------------------------------------------
void DensityCloud::applyNoiseFunction()
{
	TNvar *var=exprs.getVar((char*)"noise.expr");
	var->applyExpr();
	noise_expr=var->right;
}

//------------------------------------------------------------
// DensityCloud::getColorFunction() return color function
//-------------------------------------------------------------
int DensityCloud::getColorFunction(char *buff)
{
	TNvar *var=exprs.getVar((char*)"color.expr");
	if(!var)
		return 0;
	TNode *expr=var->getExprNode();
	if(!expr)
		expr=var->right;

	buff[0]=0;
	expr->valueString(buff);
	return 1;
}

//------------------------------------------------------------
// DensityCloud::setColorFunction() set noise function
//-------------------------------------------------------------
void DensityCloud::setColorFunction(char *expr)
{
	TNvar *var=exprs.getVar((char*)"color.expr");
	if(var)
		var->setExpr(expr);
}

//------------------------------------------------------------
// DensityCloud::applyColorFunction() set & apply color function
//-------------------------------------------------------------
void DensityCloud::applyColorFunction()
{
	TNvar *var=exprs.getVar((char*)"color.expr");
	if(var){
		var->applyExpr();
		color_expr=var->right;
	}
}

//------------------------------------------------------------
// DensityCloud::setColor() set color on array
//-------------------------------------------------------------
void DensityCloud::setColor(int i,Color c)
{
	color_list->set_color(i,c);
}

//------------------------------------------------------------
// DensityCloud::getColor() get color from array
//-------------------------------------------------------------
Color DensityCloud::getColor(int i)
{
	return color_list->get_color(i);
}


//-------------------------------------------------------------
// DensityCloud::save() archive the object and it's children
//-------------------------------------------------------------
void DensityCloud::save(FILE *fp)
{
	fprintf(fp,"%s%s",tabs,name());
	if(tree){
		fprintf(fp,"(%g) {\n",tree->size/tree->units);
    	inc_tabs();
		set_vars();
		exprs.save(fp);
		ObjectNode::save(fp);
    	dec_tabs();
		fprintf(fp,"%s}\n",tabs);
	}
}

//************************************************************
// Nebula class
//************************************************************
DensityTree Nebula::_defaults(10.0,0.1*LY);

Nebula::Nebula(double s) : DensityCloud(1.0)
{
#ifdef DEBUG_OBJS
	printf("Nebula\n");
#endif
	tree=new DensityTree(s,0.1*LY);
	tree->object=this;
}
Nebula::Nebula(Orbital *m,double s) : DensityCloud(m,1.0)
{
#ifdef DEBUG_OBJS
	printf("Nebula\n");
#endif
	tree=new DensityTree(s,0.1*LY);
	tree->object=this;
}
Nebula::~Nebula()
{
#ifdef DEBUG_OBJS
	printf("~Nebula\n");
#endif
}

//-------------------------------------------------------------
// Nebula::init()     initialize
//-------------------------------------------------------------
void Nebula::init()
{
	DensityCloud::init();
}

//-------------------------------------------------------------
// Nebula::adapt() adapt grid
//-------------------------------------------------------------
void Nebula::adapt()
{
	DensityCloud::adapt();
}

//-------------------------------------------------------------
// Nebula::render() render grid
//-------------------------------------------------------------
void Nebula::render()
{
	DensityCloud::render();
}

//************************************************************
// Galaxy class
//************************************************************

GLuint Galaxy::star_image[3]={0,0,0};
StarTree Galaxy::_defaults(1000);
Galaxy::Galaxy(double s) : DensityCloud(s*LY)
{
#ifdef DEBUG_OBJS
	printf("Galaxy\n");
#endif
	tree=new StarTree(s);
	tree->object=this;
	all_exclude();
}
Galaxy::Galaxy(Orbital *m,double s) : DensityCloud(m,s*LY)
{
#ifdef DEBUG_OBJS
	printf("Galaxy\n");
#endif
	tree=new StarTree(s);
	tree->object=this;
	all_exclude();
	validate();
}
Galaxy::~Galaxy()
{
#ifdef DEBUG_OBJS
	printf("~Galaxy\n");
#endif
}

//-------------------------------------------------------------
// Galaxy::get_focus() set selection focus
//-------------------------------------------------------------
Point Galaxy::get_focus(void *obj)
{
    Point p=tree->point((StarNode*)obj);
    return p;
}

//-------------------------------------------------------------
// Galaxy::set_focus() set selection focus
//-------------------------------------------------------------
void Galaxy::set_focus(Point &p)
{
	System *system=new System(0);
	system->set_system(p);

	int stars=system->stars();
	int planets=system->planets();

    Point vp=Point(0,0,0)-TheScene->spoint;
    TheScene->set_istring("System dist:%g ly Stars:%d Planets:%d",p.distance(vp)/LY,stars,planets);
    delete system;
}

//-------------------------------------------------------------
// Galaxy::move_focus() move selection focus
//-------------------------------------------------------------
void Galaxy::move_focus(Point &selm)
{
	//selm.print();
	TheScene->set_changed_view();
	TheScene->views_mark();

	TheScene->spoint=Point(0,0,0)-selm;

	TheScene->viewtype=GLOBAL;
	TheScene->viewobj=0;
	TheScene->spoint.print();
	TheScene->set_moved();
	TheScene->set_viewobj((ObjectNode*)0);
	TheScene->views_mark();
}

//-------------------------------------------------------------
// Galaxy::selection_pass() select for select pass
//-------------------------------------------------------------
int Galaxy::selection_pass()
{
    clr_selected();

    if(TheScene->far_pass()){
     	if(TheScene->containsViewobj(this) && tree->inside())
        	set_selected();
    }
    return selected();
}

//-------------------------------------------------------------
// Galaxy::adapt_pass() select for adapt pass
//-------------------------------------------------------------
int Galaxy::adapt_pass()
{
    if(TheScene->far_pass()){
     	TheScene->set_clrpass(1);
        set_selected();
    }
    else
        clr_selected();
    return selected();
}
//-------------------------------------------------------------
// Galaxy::render_pass() select for render pass
//-------------------------------------------------------------
int Galaxy::render_pass()
{
    return adapt_pass();
}

//-------------------------------------------------------------
// Galaxy::rebuild() rebuild map
//-------------------------------------------------------------
void Galaxy::rebuild()
{
	if(invalid()){
		tree->rebuild();
		validate();
	}
	ObjectNode::rebuild();
}

//-------------------------------------------------------------
// Galaxy::set_ref() move to orbital position
//-------------------------------------------------------------
void Galaxy::set_ref()
{
  	if(TheScene->far_pass()){
		TheScene->translate(origin.x,origin.y,origin.z);
		if(tilt)
		    TheScene->rotate(tilt,1,0,0);	// rotate about x
	    if(pitch)
		    TheScene->rotate(pitch,0,0,1);	// rotate about z
 		if(orbit_angle)
			TheScene->rotate(orbit_angle,0,1,0);	// rotate about y
    }
}

//-------------------------------------------------------------
// Galaxy::select() load names for the object and it's children
//-------------------------------------------------------------
void Galaxy::select()
{
 	if(isEnabled() && TheScene->far_pass()){
		set_ref();
		set_point();
		if(included()){
			//GLSLMgr::setFBORenderPass();
			//glUseProgramObjectARB(0);
		    //set_tilt();
		    //set_rotation();
			TheScene->pushMatrix();
		    //TheScene->set_matrix(0);
			setMatrix();
			tree->select();
		    TheScene->popMatrix();
		}
	}
	Orbital::select(); // select children
}
//-------------------------------------------------------------
// Galaxy::adapt() adapt grid
//-------------------------------------------------------------
void Galaxy::adapt()
{
 	if(isEnabled() && TheScene->far_pass()){
		set_ref();
		set_point();
		if(included()){
		    TheScene->pushMatrix();
			//TheScene->set_matrix(0);
		    setMatrix();
			adapt_object();
			//volume.set_zpass();
			//tree->adapt();
		    TheScene->popMatrix();
		}
	}
	Orbital::adapt(); // render children
}
//-------------------------------------------------------------
// Galaxy::setStarTexture() set point sprite image texture
// generate a texture image from a bitmap file
//-------------------------------------------------------------
void Galaxy::setStarTexture(int id,char *name){
	if(star_image[id]>0)
		return;

	int height,width;

	char base[256];
	char dir[256];

  	File.getBaseDirectory(base);
 	sprintf(dir,"%s/Resources/Sprites/Stars/%s",base,name);

	Image *image = images.open(name,dir);
	
	if (!image)
		return;
	height = image->height;
	width = image->width;
	unsigned char *pixels = (unsigned char*) malloc(height * width);
	unsigned char* rgb = (unsigned char*) image->data;
	for (int i = 0,index=0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int k=(height-i-1)*width+j;  // mirror y !
			double a = rgb[k * 3] / 255.0;
			pixels[index]=(unsigned char) (a * 255.0);
			index++;
		}
	}
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glGenTextures(1, &star_image[id]); // Generate a unique texture ID
	glBindTexture(GL_TEXTURE_2D, star_image[id]);
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width,
			height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
	delete image;
	::free(pixels);
}

//-------------------------------------------------------------
// Galaxy::setProgram() set shader program
// - When rendering to FBO GL_POINT_SPRITE_COORD_ORIGIN needs to
//   be set to GL_LOWER_LEFT (otherwise rendering is done on the cpu
//   which is much slower)
//-------------------------------------------------------------
bool Galaxy::setProgram(){
	StarTree *stars=(StarTree *)tree;

	char frag_shader[128]="galaxy.frag";
	char vert_shader[128]="galaxy.vert";
	char defs[128]="";
	if(stars->render_fg())
		strcpy(defs+strlen(defs),"#define FGSTARS\n");
	if(stars->inside())
		strcpy(defs+strlen(defs),"#define INSIDE\n");
	if(TheScene->inside_sky())
		strcpy(defs+strlen(defs),"#define INSIDE_SKY\n");

	GLSLMgr::setDefString(defs);
	GLSLMgr::loadProgram(vert_shader,frag_shader);
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program){
		cout << "Error building program "<< endl;
		return false;
	}
	GLSLVarMgr vars;
	vars.newIntVar("startex",0);

	if(stars->render_fg())
		vars.newFloatVar("pointsize",stars->fgpt1);
	else
		vars.newFloatVar("pointsize",stars->bgpt1);
	vars.newFloatVar("color_mix",2.0*stars->color_mix);
	vars.setProgram(program);
	vars.loadVars();

	GLSLMgr::pass=0;
	if(TheScene->inside_sky()||Raster.do_shaders)
		GLSLMgr::setFBOReadWritePass();
	else
		GLSLMgr::setFBORenderPass();

	GLSLMgr::loadVars();
	GLSLMgr::setProgram();
	return true;
}

//-------------------------------------------------------------
// Galaxy::render() render galaxy
//-------------------------------------------------------------
void Galaxy::render()
{
	if(TheScene->far_pass() && isEnabled() && included()){
		render_object();
		if(TheScene->focusobj==this) // highlight selected star
		    tree->draw_selpt();
	}
	else
		Orbital::render(); // render children
}
//-------------------------------------------------------------
// Galaxy::render() render galaxy
//-------------------------------------------------------------
void Galaxy::render_object()
{
	set_ref();
	set_point();
	TheScene->pushMatrix();
	//TheScene->set_matrix(0);
	setMatrix();
	StarTree *stars=(StarTree *)tree;
	tree->sort_nodes=true;
	tree->sortNodes();

	stars->set_render_fg(stars->inside());
	stars->set_render_bg(true);
	if(Render.draw_shaded()){
		glActiveTextureARB(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_POINT_SPRITE);
		glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		setStarTexture(0,"galaxy-sprites");
		glBindTexture(GL_TEXTURE_2D, star_image[0]);
		setProgram();
	}
	else{
		glActiveTextureARB(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_POINT_SPRITE);
		glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		if(stars->inside())
			setStarTexture(1,"star0");
		else
			setStarTexture(1,"star1");
		glBindTexture(GL_TEXTURE_2D, star_image[1]);
	}
	DensityCloud::render_object();
	glActiveTextureARB(GL_TEXTURE0);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	TheScene->popMatrix();
}

//-------------------------------------------------------------
// Galaxy::init_view()     initialize
//-------------------------------------------------------------
void Galaxy::init_view()
{
	if(TheScene->focusobj==this){
		cout<<"Galaxy::init_view"<<endl;
		addNewSystem();
		return;
	}

	TheScene->gndlvl=0;
	TheScene->minr=0.0;
	TheScene->maxr=size;
	TheScene->spoint=Point(0,0,0);

	//cout <<"stride:"<< TheScene->gstride/LY << endl;
	TheScene->hstride=1;
	TheScene->zoom=1;
	//TheScene->radius=TheScene->height=0;
	if(TheScene->changed_view()){
		if(!TheScene->changed_position()){
			if(tree->inside())
				TheScene->gstride=TheScene->vstride=0.1*LY;
			else
				TheScene->gstride=TheScene->vstride=0.1*size;

			TheScene->phi=0;
			TheScene->theta=0;
		}
		TheScene->view_tilt=0;
		TheScene->view_angle=0;
		TheScene->view_skew=0.0;
		TheScene->heading=90;
		TheScene->pitch=-90;
	}
}

//-------------------------------------------------------------
// Galaxy::addNewSystem() make a new star system
//-------------------------------------------------------------
void Galaxy::addNewSystem()
{
	int ssave=lastn;
	fixed_rands=true;
	System::galaxy_origin=TheScene->selm;
	
	cout<<"New System"<<endl;

	System  *system=System::newInstance();

	addChild(system);
    invalidate();

	TheScene->regroup();
    TheScene->rebuild_all();
    rebuild_scene_tree();
    select_tree_node(system);
    
    TheScene->focusobj=system;
    system->init_view();
    lastn=ssave;
    fixed_rands=false;
}
//------------------------------------------------------------
// Galaxy::set_vars() set local variables
//-------------------------------------------------------------
void Galaxy::set_vars()
{
	DensityCloud::set_vars();
	StarTree *tree_mgr=(StarTree*)tree;
	StarTree *tree_defs=(StarTree*)defaults();
	TSET("near.size",fgpt1);
	TSET("far.size",bgpt1);
	TSET("far.density",bg_density);
	TSET("far.random",bg_random);
	TSET("far.distance",fgfar);
	TSET("nova.density",nova_density);
	TSET("nova.size",nova_size);
	TSET("variability",fg_random);

}

//------------------------------------------------------------
// Galaxy::get_vars() get local variables
//-------------------------------------------------------------
void Galaxy::get_vars()
{
	DensityCloud::get_vars();
	StarTree *tree_mgr=(StarTree*)tree;
	TDGET("near.size",fgpt1);
	TDGET("far.size",bgpt1);
	TDGET("far.density",bg_density);
	TDGET("far.distance",fgfar);
	TDGET("far.random",bg_random);

	TDGET("nova.density",nova_density);
	TDGET("nova.size",nova_size);
	TDGET("variability",fg_random);
}

//************************************************************
// System class
//************************************************************
Point System::galaxy_origin;
bool System::building_system=false;
System *System::TheSystem=0;

System::System(double s) : Orbital(s)
{
#ifdef DEBUG_OBJS
	printf("System\n");
#endif
	all_exclude();
	m_planets=m_stars=0;
	TheSystem=this;
}
System::System(Orbital *m,double s) : Orbital(m,s)
{
#ifdef DEBUG_OBJS
	printf("System\n");
#endif
	all_exclude();
	m_planets=m_stars=0;
	TheSystem=this;
}

//-------------------------------------------------------------
// System::select_pass() select for scene pass
//-------------------------------------------------------------
int System::select_pass()
{
    clr_selected();
    return 0;
}

//-------------------------------------------------------------
// System::scale() set znear, zfar
//-------------------------------------------------------------

int System::scale(double&n, double&f) {
    n=1e-5;
    f=size;
	//TheScene->height=TheScene->radius;
    setvis(OFFSCREEN);
	return OFFSCREEN;
}

//-------------------------------------------------------------
// System::set_ref() move to orbital position
//-------------------------------------------------------------
void System::set_ref()
{
  	if(TheScene->far_pass()){
 	    //printf("System::set_ref()\n");
        TheScene->translate(origin.x,origin.y,origin.z);
		if(tilt)
			TheScene->rotate(tilt,1,0,0);	// rotate about x
		if(pitch)
			TheScene->rotate(pitch,0,0,1);	// rotate about z
		if(orbit_angle)
			TheScene->rotate(orbit_angle,0,1,0);	// rotate about y
	}
}

//-------------------------------------------------------------
// System::locate() set point to global coordinates
//-------------------------------------------------------------
void System::locate()
 {
 	set_ref();

 	TheScene->pushMatrix();
 	setMatrix();
 	set_point();
 	TheScene->popMatrix();
 	visit(&Object3D::locate);
 }

//-------------------------------------------------------------
// System::get_vars() reserve interactive variables
//-------------------------------------------------------------
void System::get_vars()
{

	Orbital::get_vars();
	if(exprs.get_local("origin",Td))
		origin=(Td.p)*LY;
}

//-------------------------------------------------------------
// System::set_vars() set common variables
//-------------------------------------------------------------
void System::set_vars()
{
    Orbital::set_vars();
    exprs.set_var("origin",origin/LY,origin.length()>0);
}

//-------------------------------------------------------------
// System::init_view()     initialize
//-------------------------------------------------------------
void System::init_view()
{
	get_system_stats();

	TheScene->maxr=size;
	TheScene->minr=10;
	TheScene->minh=0.01;
	TheScene->scale(0.001,size);
	TheScene->zoom=1;
	TheScene->hstride=TheScene->vstride=1;
	TheScene->gstride=0.1*LY;
	//if(TheScene->changed_file())  // exit for open
	//    return;
	TheScene->view_tilt=0;
	TheScene->view_skew=0.0;
	TheScene->gndlvl=0;
	TheScene->height=3*size;//51.120;
	//TheScene->height=30;
	//TheScene->radius=30;
	TheScene->radius=TheScene->height;
	TheScene->theta=0;
	TheScene->heading=90;
	TheScene->pitch=-90.0;
	if(m_stars==1){
		TheScene->phi=0;
		TheScene->view_angle=0;
	}
	else{
		TheScene->phi=90;
		TheScene->view_angle=-90;

	}
	cout<<"stars:"<<m_stars<<" planets:"<<m_planets<<" size:"<<size<<" ht:"<<TheScene->radius<<endl;

}

//-------------------------------------------------------------
// System::save() archive the object and it's children
//-------------------------------------------------------------
void System::save(FILE *fp)
{
	fprintf(fp,"%s%s",tabs,name());
	fprintf(fp,"(%g) {\n",size);
    inc_tabs();
	set_vars();
#ifdef DEBUG_SAVE
	printf("save %-12s\n",name());
	exprs.save(fp);
	printf("\n");
#else
	exprs.save(fp);
#endif
	ObjectNode::save(fp);

    dec_tabs();
	fprintf(fp,"%s}\n",tabs);
}

//-------------------------------------------------------------
// System::render() render the object and it's children
//-------------------------------------------------------------
void System::render()
{
	//GLSLMgr::setFBOReset();

	if(local_group()){
		set_ref();
	   	Orbital::render(); // render children
    }
}

//-------------------------------------------------------------
// System::adapt() adapt the object and it's children
//-------------------------------------------------------------
void System::adapt()
{
	set_ref();
	Orbital::adapt(); // adapt children
}

NodeIF *System::replaceChild(NodeIF *c,NodeIF *n){
	TheScene->views_reset();

	((Orbital*)n)->setOrbitFrom((Orbital*)c);
	if(c->hasChild(TheScene->viewobj)){
		Orbital * m=(Orbital *)c;
		m->children.remove(TheScene->viewobj);
		n->addChild(TheScene->viewobj);
	}
	Orbital *m=(Orbital*)Orbital::replaceChild(c,n);
	return m;
}

NodeIF *System::addChild(NodeIF *child){
	NodeIF *n=Orbital::addChild(child);
	adjustOrbits();
	return n;
}
NodeIF *System::addAfter(NodeIF *b,NodeIF *c){
	NodeIF *n=Orbital::addAfter(b,c);
	adjustOrbits();
	return n;
}

NodeIF *System::removeChild(NodeIF *child){
	NodeIF *n=Orbital::removeChild(child);
	TheScene->views_reset();
	adjustOrbits();
	return n;
}

NodeIF *System::replaceNode(NodeIF *n){
	Point p=origin;

	NodeIF *node=Orbital::replaceNode(n);
	((System*)n)->origin=p;
	if(TheScene->viewobj==n)
		TheScene->change_view(ORBITAL);
	return node;
}
//-------------------------------------------------------------
// System::randomize() generate a new random star system
//-------------------------------------------------------------
bool System::randomize(){
	Orbital::randomize();
	return true;
}

//-------------------------------------------------------------
// System::planets() return number of planets in system
//-------------------------------------------------------------
int System::planets()
{
	return m_planets;
}

//-------------------------------------------------------------
// System::planets() return number of stars in system
//-------------------------------------------------------------
int System::stars()
{
	return m_stars;
}

//-------------------------------------------------------------
// System::get_system_stats() identify system children
//-------------------------------------------------------------
void System::get_system_stats()
{
	Orbital *obj;
	children.ss();
	m_stars=0;
	m_planets=0;
	size=0;

	while(obj=(Orbital*)children++){
        if(obj->type()!=ID_STAR && obj->type()!=ID_PLANET)
        	continue;
        double r=(obj->orbit_radius)*(1+obj->orbit_eccentricity)+3*obj->size;
		if(obj->type()==ID_STAR){
			Star *star=(Star*)obj;
	        size=r>size?r:size;

			//cout<<"size:"<<star->size<<" radius:" <<star->orbit_radius<<" e:"<<star->orbit_eccentricity<<endl;
			m_stars++;
		}
		else if(obj->type()==ID_PLANET){
			m_planets++;
		}
	}
}
//-------------------------------------------------------------
// System::set_system() set system prototype
//-------------------------------------------------------------
void System::set_system(Point p)
{
	origin=p;
	double nseed=0.5+Random(p);
	setRseed(nseed);

	lastn=rseed*123457;

	double ps=pow(URAND(lastn),2);
	m_stars=1+ps*3;
	m_stars=m_stars<1?1:m_stars;
	double pp=pow(URAND(lastn++),2);
	if(m_stars==1)
		m_planets=0.9+pp*5;
	else
		m_planets=0.5+pp*2;
}

NodeIF *System::getInstance(){
	double f=1000*LY;
	System::galaxy_origin=Point(f*SRand(),0.01*f*SRand(),0.1*f*SRand());
	System *system=newInstance();
	system->setNewViewObj(true);
	return system;
}

double System::adjustOrbits(){
	if(building_system)
		return 0;
	double year=0;
	double phase=0;
	
	Star *star;
	Object3D *child;

	ValueList<Object3D*> sorted;

	children.ss();
	while ((child = children++)){
		if(child->type()==ID_STAR){
			sorted.add(child);
		}
	}
	int stars=sorted.size;
	if(stars==1){
		star=sorted.first();
		star->orbit_radius=0;
		star->orbit_eccentricity=0;
		star->orbit_distance=star->orbit_radius;
		star->locate();
		size=4*star->size;
		return 0;
	}
	sorted.sort();
	sorted.se();

	while ((star = sorted--)){
		star->orbit_phase=phase;
		phase+=360.0/stars;
	}
	sorted.se();
	double radius=0;
	double max_radius=0;

	if(stars>=2){ //binary
		double r,m1,m2,m;
		Star *star1=sorted--; //biggest
		Star *star2=sorted--; // smallest
		m1=star1->size;
		m2=star2->size;
		m=m1+m2;// min distance
		r=m*(10+RAND(lastn++));   // random distance between stars
		star1->orbit_radius=r*m2/m;
		star2->orbit_radius=r-star1->orbit_radius;

		double year=0.5*r*(1+0.2*URAND(lastn++));
		star1->day=0.1*year*(1+0.5*RAND(lastn++));
		star2->day=0.1*year*(1+0.5*RAND(lastn++));
		r=URAND(lastn++);
		double e=r;//*m1/m;
		//cout<<"e:"<<e<<endl;

		star1->year=year;
		star2->year=year;
		star2->orbit_eccentricity=e*m1/m;
		star1->orbit_eccentricity=e*m2/m;
		radius=star2->orbit_radius*(1+e);
		star1->orbit_distance=star1->orbit_radius;
		star2->orbit_distance=star2->orbit_radius;
		max_radius=radius>max_radius?radius:max_radius;

		star1->locate();
		star2->locate();
	}
	if(stars>2){
		while ((star = sorted--)) { // back-to-front
			star->orbit_eccentricity=0;
			radius+=10;
			star->orbit_radius=radius*(1.2+URAND(lastn++));
			star->year=star->orbit_radius*(1+0.5*URAND(lastn++));
			star->orbit_distance=star->orbit_radius;
			star->locate();
			max_radius=radius>max_radius?radius:max_radius;
		}
	}

	star=(Star*)sorted.first();
	size=star->orbit_radius+4*star->size;
	return max_radius;
}
//-------------------------------------------------------------
// System::setInstance() generate a new random star system
//-------------------------------------------------------------
System *System::newInstance(){
	setCenterText(" New System");

	System  *system=TheScene->getPrototype(0,TN_SYSTEM);
    
	system->origin=galaxy_origin;

	system->set_system(system->origin);
	system->children.free();

	double year=0;
	double phase=0;
	int stars=system->m_stars;
	int planets=system->m_planets;
	building_system=true;
	for(int i=0;i<stars;i++){
		Star *star=Star::newInstance();	
		if(stars==1)
			star->orbit_radius=0;
		lastn++;
		system->addChild(star);
	}
	building_system=false;
	double star_radius=system->adjustOrbits();
	Planetoid::planet_orbit=1.2*star_radius;
	for(int i=0;i<planets;i++){
		Planet::planet_cnt=i+1;
		Planet *planet=TheScene->getPrototype(0,ID_PLANET);
		planet->setRands();
		
		planet->orbit_radius=1.3*(Planetoid::planet_orbit+20+50*r[4]);
		planet->orbit_distance=planet->orbit_radius;

		planet->newInstance(GN_RANDOM);
		Planetoid::planet_orbit=planet->orbit_radius;
		lastn++;
		system->addChild(planet);
	}
    system->setProtoValid(true);
    Planetoid::planet_orbit=0;
    Render.invalidate_textures();
    images.invalidate();
	images.makeImagelist();

    return system;
}

//************************************************************
// Spheroid class
//************************************************************
Spheroid::Spheroid(Orbital *m, double s) :
	Orbital(m,s,0)
{
    hscale=5e-4;
	terrain.parent=&exprs;   // Scope parent
	terrain.setParent(this); // NodeIF parent
	exprs.setParent(this);
#ifdef DEBUG_BASE_OBJS
	//printf("    Spheroid\n");
#endif
}
Spheroid::Spheroid(Orbital *m, double s, double r) :
	Orbital(m,s,r)
{
    hscale=5e-4;
	map=new Map(s);
	map->object=this;
	set_geometry();
	validate();
	terrain.parent=&exprs;   // Scope parent
	terrain.setParent(this); // NodeIF parent
	exprs.setParent(this);
#ifdef DEBUG_BASE_OBJS
	printf("Spheroid\n");
#endif
	set_clip_children();
}

Spheroid::~Spheroid()
{
	if(map)
		delete map;
	map=0;
#ifdef DEBUG_BASE_OBJS
	printf("    ~Spheroid\n");
#endif
}
TNode *Spheroid::add_expr(int t, char *s, TNode *r)
{
	if(t)
		return terrain.add_expr(s,r);  // eval for each node
	else
		return Orbital::add_expr(t, s,r);
}
int Spheroid::cells()		{ return map?map->size:0;}
int Spheroid::cycles()		{ return map?map->cycles:0;}

void Spheroid::addTerrainVar(const char *name,const char *expr){
	char buff[2048];
	strcpy(buff,expr);
	char *var_name;
	MALLOC(32, char, var_name);
	strcpy(var_name, name);
	TNode *var=TheScene->parse_node(buff);
	terrain.add_expr(var_name,var);
}

//-------------------------------------------------------------
// Spheroid::get_focus() set selection focus
//-------------------------------------------------------------
Point Spheroid::get_focus(void *v)
{
    if(v){
        int id=(int)v;
        if(Raster.valid_id(id)){
            MapNode *n=Raster.get_data(id);
            return Point(n->theta(),n->phi(),n->height());
        }
    }
    return Object3D::get_focus(v);
}
//-------------------------------------------------------------
// Spheroid::move_focus() move selection focus
//-------------------------------------------------------------
void Spheroid::move_focus(Point &selm)
{
	if(TheScene->viewobj==this){
		double dh=TheScene->elevation-selm.z;
		if(dh<10*FEET)
		    dh=10*FEET;

		Point m=selm;
		m.z+=radius();
		m=m.rectangular();
    	Point p=m.mm(TheScene->ModelMatrix.values())+point;

		double dz=p.length();
		double minht=10*FEET;

		TheScene->theta=selm.x;
		TheScene->phi=selm.y;
		TheScene->gndlvl=selm.z;
		TheScene->clr_moved();  	     // don't need new gndlvl from Map.set_scene
		TheScene->set_changed_detail();  // but still need to turn on Map.adapt

	    double ht=0;

	    // called from change_view

	    if(TheScene->changed_position()){
			if(TheScene->viewtype==ORBITAL)
			 	ht=1.5*radius();
			else
			 	ht=rampstep(500*FEET,1000*MILES,dz,20*FEET,10*MILES);
		}

 		// called from move_selected

 		else {
 			minht=rampstep(500*FEET,10*MILES,dz,20*FEET,200*FEET);
			if(dh<minht)
				ht=minht;
			else
		    	ht=dh;
		}
		TheScene->height=ht;
		TheScene->elevation=TheScene->height+TheScene->gndlvl;
		TheScene->radius=radius()+TheScene->elevation;
		//map->vbounds.set_valid(0);
	}
	else{
	    TheScene->phi=5;
	    TheScene->theta=0;
		TheScene->viewobj=this;
		init_view();
		//selm=Point(theta,phi,radius);
	}
}
//-------------------------------------------------------------
// Spheroid::set_focus() set selection focus
//-------------------------------------------------------------
void Spheroid::set_focus(Point &selm)
{
    if(TheScene->viewobj !=this){
       Object3D::set_focus(selm);
       return;
    }
    Point p1,p2,v;
	double d;

	GLboolean lflag;
	glGetBooleanv(GL_LIGHTING,&lflag);
	glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);

	Point m=selm;
	m.z+=radius();
	m=m.rectangular();
    Point p=m.mm(TheScene->ModelMatrix.values())+point;
    v=TheScene->project(p.x,p.y,p.z);
    //v.z=0;
    d=p.length();
	const char *s1="miles";
	const char *s2="miles";
    if(d>=4*MILES)
		d=d/MILES;
	else {
		d=d/FEET;
		s1="ft";
	}
    double g=selm.z-TheScene->elevation;
	if(fabs(g)>4*MILES)
		g=g/MILES;
	else{
		g=g/FEET;
		s2="ft";
	}
    TheScene->set_istring("dist %5.1f %s dht %5.1f %s elev %4.0f ft",
		d,s1,g,s2,selm.z/FEET);

	glLineWidth(2.0f);
	glColor3d(1,1,1);

    p1=TheScene->unProject(v.x,v.y-8,v.z);
    p2=TheScene->unProject(v.x,v.y+8,v.z);

	glBegin(GL_LINES);
	glVertex3dv(&(p1.x));
	glVertex3dv(&(p2.x));
	glEnd();

    p1=TheScene->unProject(v.x-8,v.y,v.z);
    p2=TheScene->unProject(v.x+8,v.y,v.z);

	glBegin(GL_LINES);
	glVertex3dv(&(p1.x));
	glVertex3dv(&(p2.x));
	glEnd();

	glLineWidth(1.0f);

	glFlush();
	if(lflag==GL_TRUE)
		glEnable(GL_LIGHTING);
}

//-------------------------------------------------------------
// Spheroid::scale() 	set znear, zfar
//-------------------------------------------------------------
//#define DEBUG_SCALE
int  Spheroid::scale(double &zn, double &zf)
{
    int v=getvis();
    int t;

    if(TheScene->viewobj==this){
        if(TheScene->adapt_mode() || !map->vbounds.valid()){
		    set_scene();
		    zn=TheScene->znear;
		    zf=TheScene->zfar;
#ifdef DEBUG_SCALE
	        cout<<"adapt  zn:"<<zn/FEET<<" zf:"<<zf/MILES<<" ratio:"<<zf/zn<<endl;
#endif
		}
		else{
			zn=0.25*map->vbounds.zn;
			zf=map->vbounds.zf;
#ifdef DEBUG_SCALE
	        cout<<"render zn:"<<zn/FEET<<" zf:"<<zf/MILES<<" ratio:"<<zf/zn<<endl;
#endif
		}
		t=OUTSIDE;
    }

    else{
		t=Object3D::scale(zn,zf);
		//zn*=0.9;

		// calculate distance to sky, tangent to horizon

		if(t!=OFFSCREEN){
		    double s=TheScene->epoint.distance(point);
		    double a,b,r,h;
		    h=2*hscale;
		    r=(1-h)*size;
		    if(s>r){
			    a=sqrt(s*s-r*r);
			    b=sqrt(size*size-r*r);
			    zf=a+b;
		    }
	    }
	}
	map->dmin=zn;
	map->dmax=zf;
	if(TheScene->select_mode())
		t=v;
	setvis(t);
	return t;
}

//-------------------------------------------------------------
// Spheroid::selection_pass() select for scene pass
//-------------------------------------------------------------
int Spheroid::selection_pass()
{
    clr_selected();
    if(TheScene->viewobj==this){
        if(TheScene->select_node())
        	clear_pass(FG0);
        else
            clear_pass(BG1);
    }
    else if(!offscreen() && local_group())
        clear_pass(BG1);
    return selected();
}

//-------------------------------------------------------------
// Spheroid::adapt_pass() select for scene pass
//-------------------------------------------------------------
int Spheroid::adapt_pass()
{
	clr_selected();

    if(!local_group() || !view_group() || offscreen())
        clear_pass(BG2);
    else
        clear_pass(BG1);
    return selected();
}

//-------------------------------------------------------------
// Spheroid::render_pass() select for scene pass
//-------------------------------------------------------------
int Spheroid::render_pass()
{
	clr_selected();

    if(!local_group() || offscreen() || !isEnabled())
		return 0;
	//if(view_group()){
    if(view_group())
		clear_pass(BG2);
    else
    	clear_pass(BG3);
	//}
	//else
	//	clear_pass(BG2);
		//clear_pass(BG5);
    return selected();
}

//-------------------------------------------------------------
// Spheroid::get_vars() reserve interactive variables
//-------------------------------------------------------------
void Spheroid::get_vars()
{
    Orbital::get_vars();


	if(exprs.get_local("shadow",Td))
		shadow_color=Td.c;
	else
		shadow_color=def_shadow_color;

	if(exprs.get_local("hscale",Td)){
	    hscale=Td.s;///size;
	}
	else
		hscale=def_hscale;
	//map->hscale=hscale;

	VGET("symmetry",symmetry,def_symmetry);
	map->symmetry=symmetry;
}

//-------------------------------------------------------------
// Spheroid::set_vars() set common variables
//-------------------------------------------------------------
void Spheroid::set_vars()
{
    Orbital::set_vars();

    CSET("shadow",shadow_color,def_shadow_color);
	VSET("symmetry",symmetry,def_symmetry);
	VSET("hscale",hscale,def_hscale);
}

//-------------------------------------------------------------
// Spheroid::set_tilt() 	set axial tilt
//-------------------------------------------------------------
void Spheroid::set_tilt()
{
	//TODO: fix set_tilt for clouds
	if(tilt){
		// tilt and freeze pole orientation
		// axis always points in same direction globally
		// but rotation axis transfers from x to z during orbit
		TheScene->rotate(tilt,cos(orbit_angle*RPD),0,sin(orbit_angle*RPD));
	}
}

//-------------------------------------------------------------
// Spheroid::set_rotation() rotate about axis (day)
//-------------------------------------------------------------
void Spheroid::set_rotation()
{
	if(rot_angle)
	    TheScene->rotate(-rot_angle,0,1,0);	// rotate ccw
}

//-------------------------------------------------------------
// Spheroid::rebuild() rebuild map
//-------------------------------------------------------------
void Spheroid::rebuild()
{
	if(invalid()){
		delete map;
		map=new Map(size);
		map->object=this;
		set_geometry();
		terrain.set_init_mode(1);
		exprs.eval();
		terrain.init();
		terrain.init_render();
		terrain.set_eval_mode(0);
		map->make();
	}
	ObjectNode::rebuild();
}
//-------------------------------------------------------------
// Spheroid::getChildren
//-------------------------------------------------------------
bool Spheroid::hasChildren()
{
	return true;
}
//-------------------------------------------------------------
// Spheroid::getChildren
//-------------------------------------------------------------
int Spheroid::getChildren(LinkedList<NodeIF*>&l)
{
	int n=exprs.getChildren(l);
	l.add(&terrain);
    n+=ObjectNode::getChildren(l);
	return n+1;
}
//-------------------------------------------------------------
// Spheroid::replaceChild
//-------------------------------------------------------------
NodeIF *Spheroid::replaceChild(NodeIF *c,NodeIF *n)
{
	if(c->typeClass()&ID_OBJECT || c->typeClass()==ID_TERRAIN){
		((Orbital*)n)->setOrbitFrom((Orbital*)c);
  		return Orbital::replaceChild(c,n);
	}
	if(c->typeValue()==ID_TNMGR)
		return terrain.replaceChild(c,n);
	return c;
}

//-------------------------------------------------------------
// Spheroid::replaceNode()  replace current instance with another
//-------------------------------------------------------------
NodeIF *Spheroid::replaceNode(NodeIF *n){
	NodeIF *newnode=NodeIF::replaceNode(n);
	if(TheScene->viewobj==n)
		TheScene->change_view(ORBITAL);
	return newnode;
}

//-------------------------------------------------------------
// Spheroid::locate() set point to global coordinates
//-------------------------------------------------------------
void Spheroid::locate()
 {
 	set_ref();

 	TheScene->pushMatrix();
 	setMatrix();
 	set_point();
 	TheScene->popMatrix();
 	visit(&Object3D::locate);
 }

//-------------------------------------------------------------
// Spheroid::select() load names for the object and it's children
//-------------------------------------------------------------
void Spheroid::select()
{
	set_ref();
	if(included()){
		TheScene->pushMatrix();

		set_tilt();
		set_rotation();
		setMatrix();
		if(TheScene->select_object()){
			int id=Raster.set_id();
			Raster.set_data((MapNode*)this);
			glLoadName(id);
			map->render_zvals();  // use fastest render (nothing drawn)
		}
		else{
			terrain.init_render();
			terrain.set_eval_mode(0);
			map->render_select();  // use fastest render (nothing drawn)
		}
		TheScene->popMatrix();
	}
	Orbital::select(); // select children
}

//-------------------------------------------------------------
// Spheroid::render() render the object and it's children
//-------------------------------------------------------------
void Spheroid::render()
{
	if(!isEnabled())
		return;

	set_ref();
	if(included()){
		TheScene->pushMatrix();
		set_tilt();
		set_rotation();
		orient();
		setMatrix();
		render_object();
		TheScene->popMatrix();
	}
	Orbital::render(); // render children
}

//-------------------------------------------------------------
// Spheroid::force_adapt()   adapt needed test
//-------------------------------------------------------------
bool Spheroid::force_adapt() {
	if(TheScene->viewobj==this)
		return false;
	if(TheScene->viewobj!=getObjParent())
	    return true;
	if(!local_group())
		return true;
	return false;
}

//-------------------------------------------------------------
// Spheroid::adapt() adapt the object and it's children
//-------------------------------------------------------------
void Spheroid::adapt()
{
	set_ref();
	if(included()){
		TheScene->pushMatrix();
		set_tilt();
		orient();
		set_rotation();
		setMatrix();
		adapt_object();
		TheScene->popMatrix();
	}
	Orbital::adapt(); // adapt children
}

//-------------------------------------------------------------
// Spheroid::save()   archive the object
//-------------------------------------------------------------
void Spheroid::save(FILE *fp)
{
	fprintf(fp,"%s%s",tabs,name());
	terrain.validateTextures();
	Orbital::save(fp);
	terrain.save(fp);
	ObjectNode::save(fp);
    dec_tabs();
	fprintf(fp,"%s}\n",tabs);
}

//-------------------------------------------------------------
// Spheroid::adapt_object()   object level adapt
//-------------------------------------------------------------
void Spheroid::adapt_object()
{
	//cout<<name()<<" adapt_object seed:"<<rseed<<endl;

	exprs.eval();
	set_geometry();
	terrain.init();
	terrain.setAdaptMode();
	set_wscale();
	//if(rseed)
	pushSeed();
	map->adapt();
	//if(rseed)
	popSeed();
}

//-------------------------------------------------------------
// Spheroid::render_object()   object level render
//-------------------------------------------------------------
void Spheroid::render_object()
{
	first=1;
	terrain.init_render();
	Td.albedo=albedo;
	Td.shine=shine;
	Td.ambient=ambient;
	Td.emission=emission;
	Td.diffuse=diffuse;
	Td.specular=specular;
	//pushSeed();
	map->render();
	//popSeed();
	first=0;
}

//-------------------------------------------------------------
// Spheroid::map_color()   modulate render color
//-------------------------------------------------------------
void Spheroid::map_color(MapData* d,Color &c)
{
	if(!d->colors())
	     c=color();
}

//-------------------------------------------------------------
// Spheroid::set_scene()  set scene near/far clipping planes, etc.
//-------------------------------------------------------------
void Spheroid::set_scene()
{
	map->set_scene();
}

//-------------------------------------------------------------
// Spheroid::height(double t, double p)
//-------------------------------------------------------------
double Spheroid::height(double t, double p)
{
	return map->elevation(t,p);
}

//-------------------------------------------------------------
// Spheroid::set_geometry()  set map geometry
//-------------------------------------------------------------
void Spheroid::set_geometry()
{
	map->radius=size;
	map->symmetry=symmetry;
	map->hscale=hscale;
	Hscale=hscale;
	Gscale=1/hscale/size;
	Rscale=size*hscale;
}

//-------------------------------------------------------------
// Spheroid::bounds()   set bounds box
//-------------------------------------------------------------
Bounds *Spheroid::bounds()
{
	if(!allows_selection())
		return 0;
	if(!map->vbounds.valid()){
		map->make_visbox();
    }
	return &map->vbounds;
}

//-------------------------------------------------------------
// Spheroid::init()     initialize
//-------------------------------------------------------------
void Spheroid::init()
{
	//Scene::timer.showTime(typeName());
	Gscale=1/hscale/size;
	exprs.init();
	get_vars();
	set_geometry();
	// causes problems for open_node?
	//Scene::timer.showTime(typeName());
	terrain.init();
	terrain.init_render();
	terrain.set_eval_mode(0);
	map->make();

}

//-------------------------------------------------------------
// Spheroid::max_height() 	return ht uncertainty
//-------------------------------------------------------------
double  Spheroid::max_height()
{
	//if(TheScene->viewobj==this  && map->hrange==0){
	//	map->find_limits();
	//    cout<<name() << " hrange:"<<map->hrange<<" hscale:"<<hscale<<endl;
	//}
    if(map->hrange)
    	return map->hrange*hscale;
    return 0.1*hscale;
}

//-------------------------------------------------------------
// Spheroid::far_height()   set parameters for change in viewtype
//-------------------------------------------------------------
double Spheroid::far_height()
{
	//TheScene->gstride=0.1*LY;

	Orbital *obj;
	children.ss();

	double mht=2*size;
	double ht;

	while(obj=(Orbital*)children++){
		if(obj->type()==ID_RING){
		    ht=0.9*(obj->size-size);
			mht=ht<mht?ht:mht;
		}
		else if(obj->type()==ID_MOON){
		    ht=0.9*(obj->orbit_radius-size);
			mht=ht<mht?ht:mht;
		}
		else if(!TheScene->shadows_mode() && obj->type()==ID_SKY){
		    ht=1.1*(obj->size-size);
			mht=ht<mht?ht:mht;
		}
	}
	return mht;
}

//-------------------------------------------------------------
// Spheroid::init_view()   set parameters for change in viewtype
//-------------------------------------------------------------
void Spheroid::init_view()
{
	TheScene->gstride=0.1*LY;

	children.ss();

	double mht=far_height();

	TheScene->maxht=mht;
	TheScene->zoom=size;

	TheScene->minh=3*FEET;
	TheScene->maxr=2*size;
	TheScene->hstride=1;
	TheScene->vstride=0.02;

	if(TheScene->changed_file())  // exit for open
	   return;

	TheScene->gndlvl=map->elevation(TheScene->theta,TheScene->phi);

	switch(TheScene->viewtype){
	case ORBITAL:
		if(TheScene->changed_object()){
			TheScene->phi=5;
			TheScene->theta=0;
		}
		TheScene->height=2*size;
		break;

	case SURFACE:
		if(TheScene->changed_object()){
			TheScene->view_tilt=-15;
			TheScene->heading=0;
			TheScene->phi=0;
			TheScene->height=5000*FEET;
		}
	    else if(TheScene->changed_view()){
			TheScene->height=5000*FEET;
	    }
	    else if(TheScene->changed_position()){
			if(TheScene->height>200*FEET)
			    TheScene->height=200*FEET;
	    }
	    else{
			TheScene->height=100*FEET;
		}
		break;
	}
	TheScene->elevation=TheScene->height+TheScene->gndlvl;
	TheScene->radius=TheScene->elevation+map->radius;

}

//-------------------------------------------------------------
// Spheroid::set_terrain()  set terrain expr node
//-------------------------------------------------------------
TNode *Spheroid::set_terrain(TNode *root)
{
	return terrain.set_root(root);
}

//-------------------------------------------------------------
// Spheroid::inside_sky()  test if viewpoint is inside sky
//-------------------------------------------------------------
int Spheroid::inside_sky()
{
	Object3D *sky=get_sky();
	if(!sky)
		return 0;
	return sky->inside()?1:0;
}

//-------------------------------------------------------------
// Spheroid::get_sky()  test if sky is a child
//-------------------------------------------------------------
Sky *Spheroid::get_sky()
{
	Object3D *obj;

	Node<Object3D*> *ptr=children.ptr;
	children.ss();

	while(obj=children++){
		if(obj->type()==ID_SKY){
			children.ptr=ptr;
		    return (Sky*)obj;
		}
	}
	children.ptr=ptr;
	return 0;
}

//************************************************************
// Star class (stars)
//************************************************************
//--------------- static values and functions ---------------
// data from https://people.highline.edu/iglozman/classes/astronotes/hr_diagram.htm
#define TRUE_COLORS
Color Star::star_color[Star::ntypes]={
#ifdef TRUE_COLORS
		Color(1.000,0.824,0.624),  // M
		Color(1.000,0.922,0.882),  // K
		Color(1.000,0.984,0.882),  // G
		Color(1.000,1.000,1.000),  // F
		Color(0.827,0.875,1.000),  // A
		Color(0.627,0.753,1.000),  // B
		Color(0.573,0.710,1.000)}; // O
#else // exagerate colors
		Color(1.000,0.7,0.5),  // M
		Color(1.000,0.8,0.4),  // K
		Color(1.000,0.9,0.7),  // G
		Color(0.9,1.000,1.000),  // F
		Color(0.8,0.8,1.000),  // A
		Color(0.6,0.8,1.000),  // B
		Color(0.5,0.7,1.000)}; // O
#endif	
enum type{
	ST_RED=0,ST_YELLOW=2,ST_WHITE=3, ST_BLUE=5
};
char Star::star_class[Star::ntypes]={'M','K','G','F','A','B','O'};
double Star::star_temp[Star::ntypes]={2000,3500,5000,6000,7500,10000,30000};//surface temperature
double Star::star_luminocity[Star::ntypes]={0.04,0.4,1.2,6,40,5e4,1e6}; //brightness vs sun
double Star::star_radius[Star::ntypes]={0.5,0.9,1.1,1.4,2,7,16}; //radius vs sun
double Star::star_frequency[Star::ntypes]={76,12,8,3,0.6,0.01,0.0001};  //% of main sequence stars

int Star::star_id=0;
int Star::num_temps=0;
int Star::expand_factor=1000;
double *Star::probability=0;
double *Star::temps=0;

//-------------------------------------------------------------
// Star::make_temps_table()  generate a random instance
//-------------------------------------------------------------
// generate a table of star temperatures based on stellar frequency and luminocity data
// The idea is that when picking a star from the background (brightest in select region) 
// the star type will be a product of it's stellar frequency and relative brightness
// (e.g. close in red stars, further out blue stars)
//-------------------------------------------------------------
//#define SHOW_PROBABILITIES
void Star::make_temps_table(){
	num_temps=expand_factor;
	MALLOC(num_temps,double,temps);
	
	MALLOC(ntypes,double,probability);
	int index=0;
	double sum=0;
	for(int i=0;i<ntypes;i++){
		double f=star_frequency[i]*sqrt(star_luminocity[i]); // brightness falls off as 1.0/r*2
		probability[i]=f;
		sum+=f;
	}
	for(int i=0;i<ntypes;i++){
		probability[i]/=sum;
#ifdef SHOW_PROBABILITIES
		cout<<i<<" "<<probability[i]*100<<endl;
#endif
	}
	for(int i=0;i<ntypes;i++){
		int m=probability[i]*num_temps;
		double start_temp=star_temp[i];
		double end_temp=(i==(ntypes-1)?60000:star_temp[i+1]);
		double f=0;
		double delta=1.0/m;
		for(int j=0;j<m;j++){
			double t=(1-f)*start_temp+f*end_temp;
			temps[index++]=t;
			f+=delta;			
		}	
	}
	// note: for some reason not all table entries at the end are getting filled
	if(index<num_temps){
		double t=temps[index-1];
		while(index<num_temps){
			temps[index++]=t;
			t+=1000;
		}
	}
#ifdef SHOW_PROBABILITIES2
	for(int i=0;i<num_temps;i++){
		cout<<i<<" "<<temps[i]<<endl;
	}
#endif
}

//-------------------------------------------------------------
// void Star::star_info(..) 
//-------------------------------------------------------------
// return predicted temperature and type string based on radiance color
//-------------------------------------------------------------
void Star::star_info(Color col, double *t, char *m){
	int min_index=0;
	Color c=col;
	c=col;
	c.set_alpha(1.0);
	double min_diff=10;
	//col.print();
	for(int i=0;i<ntypes;i++){
		double d=c.difference(star_color[i]);
		//cout<<" "<<star_class[i]<<" "<<d<<endl;
		if(d<min_diff){
			min_index=i;
			min_diff=d;
		}
	}
	double temp=star_temp[min_index];
	double f=0;
	if(min_index<ntypes-2){
		double temp2=star_temp[min_index+1];
		double diff2=c.difference(star_color[min_index+1]);
		f=lerp(min_diff,0,diff2,0,1);
		temp=(1-f)*star_temp[min_index]+f*star_temp[min_index+1];
	}
	*t=temp*col.alpha();
	sprintf(m,"%c%d",star_class[min_index],(int)(9*f*col.alpha()));
}

//-------------------------------------------------------------
// double Star::star_size(double temp)
//-------------------------------------------------------------
// - star size from temperature
// - approximate linear fit to data 
//-------------------------------------------------------------
double Star::star_size(double temp){
	static double sun_radius=0.4327; // in 10^6 miles
	return (0.27*temp/1000)*sun_radius;
}

//-------------------------------------------------------------
// void Star::random(..) generate random star
//-------------------------------------------------------------
// - return temperature radius and color
//-------------------------------------------------------------
void Star::type(int index,double &temp, double &radius, Color &color){
   temp=temps[index];
   radius=star_size(temp);
   color=star_color[index];
   //char str[128];
   // c.toString(str);
	   //cout<<"r:"<<z<<" t:"<<temp<<" "<<str<<endl;
}
void Star::random(double &temp, double &radius, Color &color){
   double p=URAND(lastn++);
   int index=p*num_temps;
   index=index>=num_temps?num_temps-1:index;
   temp=temps[index];
   int m=0;
   Color c1=WHITE,c2=Color(0.2,0.5,1.0);
   double f=1;
   while(m<ntypes){
	   if(temp>star_temp[m])
		   m++;
	   else{
		   double t1=star_temp[m-1];
		   double t2=star_temp[m];
		   c1=star_color[m-1];
		   c2=star_color[m];
		   f=(temp-t1)/(t2-t1);
		   break;
	   }   
   }
   Color c=c1.mix(c2,f);
   color=c;
   double z=Star::star_size(temp);
   radius=z*(1+0.2*RAND(lastn++));
   //char str[128];
   // c.toString(str);
   //cout<<"r:"<<z<<" t:"<<temp<<" "<<str<<endl;
}
//-------------------------------------------------------------
// TNinode *Star::image(Color tc) generate a 1D texture image from color
//-------------------------------------------------------------
TNinode *Star::image(Color tc){
	char buff[2048];

	sprintf(buff,"bands(\"S%d\",TMP|CLAMP,16",star_id);
	Color colors[5];
	colors[0]=tc.lighten(0.75);
	colors[1]=tc.lighten(0.5);
	colors[2]=tc;
	colors[3]=tc.darken(0.5);
	colors[4]=tc.darken(0.75);

	for(int i=0;i<5;i++){
		Color c=colors[i];
		strcat(buff,",");
		c.toString(buff+strlen(buff));
	}
	strcat(buff,");\n");
	//cout<<buff<<endl;
	TNinode *n=(TNinode*)TheScene->parse_node(buff);
	if(!n)
		return 0;
	n->init();
	return n;
}

//-------------------------------------------------------------
// TNtexture *Star::texture() generate a random texture
//-------------------------------------------------------------
TNtexture *Star::texture(){
 char *ntype[]={"GRADIENT","SIMPLEX","VORONOI"};
 double offset[]={0,0.5,0.5};
 int nt=(int)(3*URAND(lastn++));
 nt=nt>2?2:nt;
 char buff[256];
 char noise_expr[128];
 sprintf(noise_expr,"noise(%s|FS|NABS|SQR|UNS|TA|RO1,0.9,8.9,0.9,0.01,2.06,1,1,0,%g,1e-6)",ntype[nt],offset[nt]);
 sprintf(buff,"Texture(\"S%d\",BORDER|S|TEX,%s,0.5,2,1,0,1,2,1,0.9,0,0,0,0)",star_id,noise_expr);
 TNtexture *nc=(TNtexture*)TheScene->parse_node(buff);
 return nc;
}
//--------------- end static values and functions ---------------
Star::Star(Orbital *m, double s, double r) : Spheroid(m,s,r)
{
	year=0;
	set_color(WHITE);
	emission=Color(1.0,1.0,1.0,1.0);
	specular=Color(1.0,1.0,1.0,1.0);
	diffuse=Color(1.0,1.0,1.0,1.0);
#ifdef DEBUG_OBJS
	printf("Star\n");
#endif
	shadows_exclude();
	temperature=0;
	startype[0]=0;
	if(temps==0){
		make_temps_table();	
	}
}
Star::~Star()
{
#ifdef DEBUG_OBJS
	printf("~Star\n");
#endif
}

//-------------------------------------------------------------
// Star::setRadiance()  set radiance from color
//-------------------------------------------------------------
void Star::setRadiance(Color c){
	emission=c;
	star_info(emission,&temperature,startype);
	//cout<<startype<<" "<<temperature<<endl;
}
//-------------------------------------------------------------
// Star::getStarData()  return temperature and type string
//-------------------------------------------------------------
void Star::getStarData(double *d, char *m){
	setRadiance(emission);
	*d=temperature;
	strcpy(m,startype);
}

NodeIF *Star::getInstance(int gtype){
    Star *star=newInstance(gtype);
    star->randomize();
	return star;

}
//-------------------------------------------------------------
// Star::setInstance()  generate a random instance
//-------------------------------------------------------------
NodeIF *Star::getInstance(){
	lastn=Rand()*1715;
	Star *star=newInstance();
	star->setNewViewObj(true);
	images.invalidate();
	images.makeImagelist();

	return star;
}
//-------------------------------------------------------------
// Star::setInstance()  generate a random instance
//-------------------------------------------------------------
Star *Star::newInstance(){
	return newInstance(0);
}
//-------------------------------------------------------------
// Star::setInstance()  generate a random instance
//-------------------------------------------------------------
Star *Star::newInstance(int gtype){
	Star *star;

	star=(Star *)TheScene->getPrototype(0,TN_STAR|gtype);
	star_id=lastn;
	star->setRseed(URAND(lastn++));

	double t=0,r=0;
	Color c;
	double cf;
	switch(gtype){
	default:
	case GN_RANDOM:
		random(t,r,c);
	    cf=1.2+URAND(lastn++);
		c=c.intensify(cf);
		break;
	case GN_RED_STAR:
		type(ST_RED,t,r,c);
		break;
	case GN_YELLOW_STAR:
		type(ST_YELLOW,t,r,c);
		break;
	case GN_WHITE_STAR:
		type(ST_WHITE,t,r,c);
		break;
	case GN_BLUE_STAR:
		type(ST_BLUE,t,r,c);
		break;
	}
	star->temperature=t;
	star->emission=c;
	star->size=r;
	
    star->children.ss();

    TNinode *img=image(c);
    Render.invalidate_textures();
    star->add_image(img);
    TNode *root=star->terrain.get_root();
    TNtexture *stex=texture();
    if(stex){
    	star->terrain.set_root(stex);
    	star->terrain.init();
    }
    else
    	cout<<"error building star texture"<<endl;

	Halo *halo=star->children++;
	halo->size=1.01*r;
	halo->ht=halo->size-r;
	halo->color1=c;
	halo->color2=c;
	halo->gradient=0.8;
	halo->density=0.9;
	
	Corona *outer=star->children++;
	outer->size = pow(1 + r, 2);
	outer->ht = outer->size - r;
	outer->color1 = c;
	outer->color1.set_alpha(0.8);
	outer->color2 = c.mix(Color(1, 0, 0), 0.2);
	outer->setName("outer");

	outer->setProtoValid(true);

	Corona *inner=star->children++;
	inner->size=1.1*r;
	inner->ht=inner->size-r;
	inner->color1=c.lighten(0.2);
	inner->color2=c;
	inner->setProtoValid(true);
	inner->setName("inner");

	inner->setNoiseFunction("noise.expr=noise(GRADIENT|FS|NABS|NLOD|SQR|TA|RO1,0,7,1,0.16,2.17,1,1,0,1,1e-06)");
	inner->applyNoiseFunction();

    star->setProtoValid(true);
    star->setRadiance(star->emission);
    star->setName(star->startype);
    return star;
}



double Star::max_height(){
	return 1e-10;
}
//-------------------------------------------------------------
// Star::get_vars()  reserve interactive variables
//-------------------------------------------------------------
void Star::get_vars()
{
	Spheroid::get_vars();
	if(exprs.get_local("emission",Td))
		emission=Td.c;
	else
		emission=Color(1,1,1,1);
	if(exprs.get_local("specular",Td))
		specular=Td.c;
	else
		specular=Color(1,1,1,1);
	if(exprs.get_local("diffuse",Td))
		diffuse=Td.c;
	else
		diffuse=Color(1,1,1,1);
	setRadiance(emission);
	exprs.set_var("temperature",temperature);
}

//-------------------------------------------------------------
// Star::set_vars() set variables
//-------------------------------------------------------------
void Star::set_vars()
{
	Spheroid::set_vars();
    CSET("specular",specular,Color(1.0,1.0,0.9,1.0));
    CSET("diffuse",diffuse,Color(1.0,1.0,0.9,1.0));
    CSET("emission",emission,Color(1.0,1.0,0.9,1.0));
}

//-------------------------------------------------------------
// Star::set_lighting() set lighting
//-------------------------------------------------------------
void Star::set_lighting(){
    if(map)
		 map->lighting=0;
}

//-------------------------------------------------------------
// Star::set_lights() set lights
//-------------------------------------------------------------
void Star::set_lights()
{
	Point pv;

	if(!local_group())
		return;

	Light *l=Lights.newLight();

	if(!l)
		return;

	l->size=size;
	if(isEnabled())
		l->enable();
	else
		l->disable();
	set_ref();

	TheScene->pushMatrix();

	setMatrix();
	set_point();

	l->point=point;  // 	position of light in "eye" frame

	// OGL will set position of light from current state of ModelMatrix

	l->setPosition(Point(0,0,0));

	TheScene->popMatrix();

   // double intensity=emission.alpha();
    double intensity=temperature;
	l->setIntensity(intensity);

	l->Diffuse=diffuse;
	l->Specular=specular;
	l->color=emission;
	l->diffuse=diffuse.alpha();//*l->intensity;
	l->specular=specular.alpha();//*l->intensity;//0.2;
	if(map)
		map->lighting=0;
}

//-------------------------------------------------------------
// Star::map_color()   modulate render color
//-------------------------------------------------------------
void Star::map_color(MapData* d,Color &c)
{
	if(!d->colors()){
	     c=emission;
	     c.set_alpha(1.0);
	}
}
//-------------------------------------------------------------
// Star::setProgram() set shader program;
//-------------------------------------------------------------
bool Star::setProgram(){
	
	pushSeed();
	char frag_shader[128]="star.frag";
	char vert_shader[128]="star.vert";
	char defs[128]="";
	GLSLMgr::setDefString(defs);

	TerrainProperties *tp=map->tp;
	tp->initProgram();

	GLSLMgr::loadProgram(vert_shader,frag_shader);
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;

	GLSLVarMgr vars;

	set_wscale();

 	tp->setProgram();

	Point p=point.mm(TheScene->invViewMatrix);
	p=p.mm(TheScene->viewMatrix);
	vars.newFloatVec("center",p.x,p.y,p.z);
	vars.newFloatVar("rseed",TheNoise.rseed);
	
	if(TheScene->inside_sky()||Raster.do_shaders)
		GLSLMgr::setFBOReadWritePass();
	else
		GLSLMgr::setFBORenderPass();

	vars.setProgram(program);
	vars.loadVars();
	TheScene->setProgram();
	popSeed();

	return true;
}

//-------------------------------------------------------------
// Star::render() render the object and it's children
//-------------------------------------------------------------
void Star::render() {
	if (!isEnabled())
		return;

	if (included()) {
		ValueList<Object3D*> back;
		ValueList<Object3D*> front;

		Object3D *obj;
		children.ss();
		while ((obj = children++)) {
			if (obj->type() == ID_HALO)
				front.add(obj);
			else if (obj->type() == ID_CORONA) {
				Corona *corona = obj;
				if (corona->internal || (TheScene->viewobj != this))
					front.add(obj);
				else
					back.add(obj);
			}
		}
		set_ref();

		if (back.size > 0) {
			back.sort();
			back.se();
			while ((obj = back--)) {
				obj->render();
			}
		}
		TheScene->pushMatrix();
		set_tilt();
		set_rotation();
		setMatrix();
		render_object();
		TheScene->popMatrix();

		if (front.size > 0) {
			front.sort();
			front.ss();
			while ((obj = front++)) {
				obj->render();
			}
		}
	}
}

//-------------------------------------------------------------
// Star::render_object()   object level render
//-------------------------------------------------------------
void Star::render_object()
{
	
 	first=1;
 	pushSeed();
	show_render_state(this);
	if(TheScene->adapt_mode())
		map->render();
	else {
		Raster.init_lights(map->lighting);
		terrain.init_render();
		map->render();
	}
	popSeed();
}

//-------------------------------------------------------------
// Star::adapt_pass() select for scene pass
//-------------------------------------------------------------
int Star::adapt_pass()
{
	return Spheroid::adapt_pass();
}

//-------------------------------------------------------------
// Star::render_pass() select for scene pass
//-------------------------------------------------------------
int Star::render_pass()
{
	clr_selected();

    if(!local_group() || offscreen() || !isEnabled())
		return 0;
    if(TheScene->viewobj==this)
		clear_pass(FG0);		
	else if(local_group())
		clear_pass(BG3);
	//else
	//	clear_pass(BG4);
    return selected();
}

//************************************************************
// Planetoid class (moons & planets)
//************************************************************
Planetoid::Planetoid(Orbital *m, double s, double r) :
	Spheroid(m,s,r)
{
	ocean=0;

	ocean_level=0;
	
	ocean_state=def_ocean_state;
	ocean_auto=true;
	terrain_type=GN_ROCKY;

	fog_color=def_fog_color;
	fog_glow=def_fog_glow;
	fog_value=def_fog_value;
	fog_vmax=def_fog_vmax;
	fog_vmin=def_fog_vmin;
	fog_max=def_fog_max;
	fog_min=def_fog_min;

	temperature=100;
	surface_temp=0;
	season_factor=0.5;
	heat_factor=0.5;
	temp_factor=0.1;
	last_dt=0;
	last_temp=0;
	seasonal=true;

#ifdef DEBUG_BASE_OBJS
	printf("Planetoid\n");
#endif
}
Planetoid::~Planetoid()
{
#ifdef DEBUG_BASE_OBJS
	printf("  ~Planetoid\n");
#endif
}

void Planetoid::setOcean(OceanState *s){
	char str[1024];
	str[0]=0;
	s->valueString(str);
	OceanState *state=TheScene->parse_node(str);
	if(ocean)
		delete ocean;
	ocean=state;
	ocean->setOceanExpr();
	//cout<<"setOcean:"<<ocean<<" "<<str<<endl;
}

OceanState *Planetoid::getOcean() {
	return ocean;
}


void Planetoid::setOceanExpr(char *expr){
	if(!water())
		return;
	ocean->setOceanFunction(expr);
}

void Planetoid::setOceanExpr(){
  if(water()) { 
	  TNvar *var=exprs.getVar((char*)"ocean.expr");
	  if(!var)
		  getOceanExpr();
	  else{
	  	var->setExpr(ocean->getOceanExpr());
	 	var->applyExpr();
	  }
  } else 
	  exprs.hide_var("ocean.expr");
}

void Planetoid::getOceanExpr(){		
	TNvar *var=exprs.getVar((char*)"ocean.expr");
	if(!var){
		var=addExprVar("ocean.expr",ocean->getOceanExpr());
		var->eval();
	}
	else{
		char buff[256]={0};
		var->eval();
		var->valueString(buff);
		ocean->setOceanFunction(buff);
	}
}
#define ODEF OceanState::oceanTypes[OceanState::Types::H2O]

//-------------------------------------------------------------
// get_vars()  reserve interactive variables
//-------------------------------------------------------------
#define MDGET(name,value,u) if(exprs.get_local(name,Td)) if(water()) ocean->value=Td.s/u
#define MVGET(name,value) if(exprs.get_local(name,Td)) if(water())ocean->value=Td.s
#define MCGET(name,value) if(exprs.get_local(name,Td)) if(water())ocean->value=Td.c
#define MNGET(name,value) if(exprs.get_local(name,Td)) if(water())strncpy(ocean->value,Td.string,maxstr)

void Planetoid::get_ocean_vars()
{
	getOceanExpr();
	cout<<"get ocean vars:"<<name()<<" have water:"<<water()<<endl;

//	if(exprs.get_local("water.level",Td))
//		ocean_level=Td.s;
//	else 
	if(exprs.get_local("ocean.level",Td))
		ocean_level=Td.s;
	else
		ocean_level=0;
	VGET("ocean.state",ocean_state,def_ocean_state);
	VGET("ocean.auto",ocean_auto,def_ocean_auto);
	
	MNGET("ocean.name",name_str);
	MVGET("ocean.rseed",rseed);

	MVGET("ocean.solid",solid->temp);
	MVGET("ocean.liquid",liquid->temp);
	MVGET("ocean.solid.trans",solid->trans_temp);
	MVGET("ocean.liquid.trans",liquid->trans_temp);
	
	MCGET("water.color1",liquid->color1);
	MCGET("water.color2",liquid->color2);
	MDGET("water.clarity",liquid->clarity,FEET);
	MVGET("water.mix",liquid->mix);
	MVGET("water.albedo",liquid->specular);
	MVGET("water.shine",liquid->shine);
	MVGET("water.volatility",liquid->volatility);

	MCGET("ocean.liquid.color1",liquid->color1);
	MCGET("ocean.liquid.color2",liquid->color2);
	MDGET("ocean.liquid.clarity",liquid->clarity,FEET);
	MVGET("ocean.liquid.mix",liquid->mix);
	MVGET("ocean.liquid.albedo",liquid->specular);
	MVGET("ocean.liquid.shine",liquid->shine);
	MVGET("ocean.liquid.volatility",liquid->volatility);

	MCGET("ocean.solid.color1",solid->color1);
	MCGET("ocean.solid.color2",solid->color2);
	MDGET("ocean.solid.clarity",solid->clarity,FEET);
	MVGET("ocean.solid.mix",solid->mix);
	MVGET("ocean.solid.albedo",solid->specular);
	MVGET("ocean.solid.shine",solid->shine);
	MVGET("ocean.solid.volatility",solid->volatility);
		
}
#define MCSET(name,value) exprs.set_var(name,ocean->value,water()&& (ODEF->value != ocean->value))
#define MVSET(name,value) exprs.set_var(name,ocean->value,water() && (ODEF->value != ocean->value))
#define MUSET(name,value,u) { ts=exprs.set_var(name,ocean->value,water() &&(ODEF->value != ocean->value)); ts->units=u;}
#define MNSET(name,value) exprs.set_var(name,ocean->value,water())

#define WCSET(name,value) exprs.set_var(name,ocean->value,0)
#define WVSET(name,value) exprs.set_var(name,ocean->value,0)
#define WUSET(name,value,u) { ts=exprs.set_var(name,ocean->value,0); ts->units=u;}

void Planetoid::set_ocean_vars(){
	
	setOceanExpr();	
	
	WCSET("water.color1",liquid->color1);
	WCSET("water.color2",liquid->color2);
	WVSET("water.clarity",liquid->clarity);
	WVSET("water.mix",liquid->mix);
	WVSET("water.albedo",liquid->specular);
	WVSET("water.shine",liquid->shine);
	WVSET("water.volatility",liquid->volatility);
	WVSET("water.level",liquid->shine);
	WVSET("water.reflectivity",liquid->shine);

	VSET("ocean.state",ocean_state,def_ocean_state);
	VSET("ocean.auto",ocean_auto,def_ocean_auto);
	USET("ocean.level",ocean_level,0,"ft");
	
	MVSET("ocean.rseed",rseed);
		
	MVSET("ocean.solid",solid->temp);
	MVSET("ocean.liquid",liquid->temp);
	MVSET("ocean.solid.trans",solid->trans_temp);
	MVSET("ocean.liquid.trans",liquid->trans_temp);

	MCSET("ocean.liquid.color1",liquid->color1);
	MCSET("ocean.liquid.color2",liquid->color2);
	MVSET("ocean.liquid.mix",liquid->mix);
	MVSET("ocean.liquid.albedo",liquid->specular);
	MVSET("ocean.liquid.shine",liquid->shine);
	MVSET("ocean.liquid.volatility",liquid->volatility);
	MUSET("ocean.liquid.clarity",liquid->clarity*FEET,"ft");
	
    MCSET("ocean.solid.color1",solid->color1);
    MCSET("ocean.solid.color2",solid->color2);
	MUSET("ocean.solid.clarity",solid->clarity*FEET,"ft");
	MVSET("ocean.solid.mix",solid->mix);
	MVSET("ocean.solid.albedo",solid->specular);
	MVSET("ocean.solid.shine",solid->shine);
	MVSET("ocean.solid.volatility",solid->volatility);

}

void Planetoid::get_vars()
{
	Spheroid::get_vars();
	checkForOcean();
	if(ocean==0)
		setOcean(ODEF);
    if(water())
		get_ocean_vars();

	VGET("season.factor",season_factor,def_season_factor);
	VGET("temp.factor",temp_factor,def_temp_factor);
	VGET("season.enable",seasonal,true);
	
	VGET("fog.value",fog_value,def_fog_value);
	VGET("fog.glow",fog_glow,def_fog_glow);
	if(exprs.get_local("fog.color",Td)){
		fog_color=Td.c;
		fog_value=fog_color.alpha();
		fog_color.set_alpha(1.0);
	}
	VGET("fog.min",fog_min,def_fog_min);
	VGET("fog.max",fog_max,def_fog_max);
	VGET("fog.vmin",fog_vmin,def_fog_vmin);
	VGET("fog.vmax",fog_vmax,def_fog_vmax);
}

//-------------------------------------------------------------
// Planetoid::set_vars() set common variables
//-------------------------------------------------------------

void Planetoid::set_vars()
{
    Spheroid::set_vars();
	checkForOcean();

	set_ocean_vars();
	
	VSET("season.factor",season_factor,def_season_factor);
	VSET("temp.factor",temp_factor,def_temp_factor);
	VSET("season.enable",seasonal,true);

	CSET("fog.color",fog_color,def_fog_color);
	VSET("fog.value",fog_value,def_fog_value);
	VSET("fog.glow",fog_glow,def_fog_glow);

	USET("fog.min",fog_min,def_fog_min,"ft");
	USET("fog.max",fog_max,def_fog_max,"ft");
	USET("fog.vmin",fog_vmin,def_fog_vmin,"ft");
	USET("fog.vmax",fog_vmax,def_fog_vmax,"ft");
	
}
//-------------------------------------------------------------
// Planetoid::setProgram() set shader program;
//-------------------------------------------------------------
bool Planetoid::setProgram(){
	TerrainProperties *tp=map->tp;

	char defs[512]="";
	if(Render.lighting())
		sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),Lights.size);
	else
		sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),0);
    bool do_shadows=Raster.shadows() && (Raster.twilight() || Raster.night());
	if(do_shadows && !TheScene->light_view()&& !TheScene->test_view() &&(Raster.farview()||TheScene->viewobj==this))
		sprintf(defs+strlen(defs),"#define SHADOWS\n");
	if(Raster.hdr())
		sprintf(defs+strlen(defs),"#define HDR\n");
	if(TheScene->viewobj==this)
	    sprintf(defs+strlen(defs),"#define VIEWOBJ\n");
	if(Render.geometry() && tp->has_geometry())
		sprintf(defs+strlen(defs),"#define TESSLVL %d\n",Map::tessLevel());

	if(TheScene->enable_contours)
		sprintf(defs+strlen(defs),"#define CONTOURS\n");
	if(TheScene->enable_grid)
		sprintf(defs+strlen(defs),"#define GRID\n");


	double twilite_min=-0.2;
	double twilite_max=0.2;
	double twilite_dph=0.1;
	float haze_zfar = 0.0;
	float haze_grad = 0.5;
	Color haze(1,1,1);
	double sky_alpha = 0.8*Raster.sky_alpha;

	Sky *sky=get_sky();
	if(sky){
		twilite_min=sky->twilite_min;
		twilite_max=sky->twilite_max;
		twilite_dph=sky->twilite_dph;
	}
	double clarity=0;
	if(water()){
		clarity=ocean->liquid->clarity;
		if(TheScene->viewobj!=this || TheScene->viewtype !=SURFACE)
			clarity*=20;
	}
	GLSLMgr::setDefString(defs);

	//GLSLMgr::init3DNoiseTexture();
	tp->initProgram();
	set_wscale();
	int tl=1; // tesslevel
	if(Render.geometry() && tp->has_geometry()){
		GLSLMgr::input_type=GL_TRIANGLES;
		//GLSLMgr::output_type=GL_TRIANGLE_STRIP_ADJACENCY; // this also works without any code changes
		GLSLMgr::output_type=GL_TRIANGLE_STRIP;
		GLSLMgr::loadProgram("planetoid.gs.vert","planetoid.frag","planetoid_indexed.geom");
	}
	else{
		GLSLMgr::loadProgram("planetoid.vert","planetoid.frag");
	}
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;

	GLSLVarMgr vars;
	shadow_intensity=shadow_color.alpha();
	if(TheScene->inside_sky())
		shadow_intensity*=lerp(Raster.ldp,-1,0,shadow_intensity,1);
	//cout<<Raster.ldp<<" "<<shadow_intensity<<endl;
	Point p=point.mm(TheScene->invViewMatrix);
	vars.newFloatVec("object",p.x,p.y,p.z); // global
	p=p.mm(TheScene->viewMatrix);
	vars.newFloatVec("center",p.x,p.y,p.z);
	vars.newFloatVec("Diffuse",diffuse.red(),diffuse.green(),diffuse.blue(),diffuse.alpha());
	vars.newFloatVec("Shadow",shadow_color.red(),shadow_color.green(),shadow_color.blue(),shadow_intensity);
	vars.newBoolVar("lighting",Render.lighting());
	vars.newFloatVar("sky_alpha",sky_alpha);
    vars.newFloatVar("haze_zfar",haze_zfar);
    vars.newFloatVar("haze_grad",haze_grad);
	vars.newFloatVar("fog_vmin",fog_vmin);
	vars.newFloatVar("fog_vmax",fog_vmax);
	vars.newFloatVar("fog_znear",fog_min);
	vars.newFloatVar("fog_zfar",fog_max);
	vars.newFloatVar("rseed",rseed);
	
	double glow=TheScene->viewobj==this?1:2;
	vars.newFloatVar("glow",glow);

	vars.newFloatVar("nscale",shadow_color.alpha());

    vars.newFloatVec("Haze",haze.red(),haze.green(),haze.blue(),haze.alpha());

	vars.newFloatVar("twilite_min",twilite_min);
	vars.newFloatVar("twilite_max",twilite_max);
	vars.newFloatVar("twilite_dph",twilite_dph);

	vars.newFloatVar("hdr_min",Raster.hdr_min);
	vars.newFloatVar("hdr_max",Raster.hdr_max);
	Point pv=TheScene->xpoint;
	vars.newFloatVec("pv",pv.x,pv.y,pv.z);
	if(TheScene->enable_contours){
		Color c = TheScene->contour_color;
		vars.newFloatVec("contour_color",c.red(),c.green(),c.blue(),1.0);
		vars.newFloatVar("contour_spacing",TheScene->contour_spacing);
	}
	if(TheScene->enable_grid){
		Color c = TheScene->phi_color;
		vars.newFloatVec("phi_color",c.red(),c.green(),c.blue(),1.0);
		c = TheScene->theta_color;
		vars.newFloatVec("theta_color",c.red(),c.green(),c.blue(),1.0);
		vars.newFloatVar("grid_spacing",TheScene->grid_spacing*MILES/RPD/size);
	}
	if(water()){
		Color c=ocean->liquid->color1;
		vars.newFloatVec("WaterColor1",c.red(),c.green(),c.blue(),c.alpha());
		c=ocean->liquid->color2;
		vars.newFloatVec("WaterColor2",c.red(),c.green(),c.blue(),c.alpha());
		c=ocean->solid->color1;
		vars.newFloatVec("IceColor1",c.red(),c.green(),c.blue(),c.alpha());
		c=ocean->solid->color2;
		vars.newFloatVec("IceColor2",c.red(),c.green(),c.blue(),c.alpha());
	
		vars.newFloatVar("ice_clarity",ocean->solid->clarity*FEET);
		vars.newFloatVar("water_clarity",clarity*FEET);
	
		vars.newFloatVar("water_shine",ocean->liquid->shine);
		vars.newFloatVar("ice_shine",ocean->solid->shine);
		vars.newFloatVar("water_specular",ocean->liquid->specular);
		vars.newFloatVar("ice_specular",ocean->solid->specular);
	}
	vars.newFloatVar("albedo",albedo);

	tp->setProgram();

	if(TheScene->inside_sky()){
		if(TheScene->viewobj==this)
			GLSLMgr::setFBORenderPass();
		else
			GLSLMgr::setFBOWritePass();
	}
	else{
		//GLSLMgr::pass=0;
		if(Raster.do_shaders)
			GLSLMgr::setFBOReadWritePass();
		else
			GLSLMgr::setFBORenderPass();
	}

	vars.setProgram(program);
	vars.loadVars();
	TheScene->setProgram();

	return true;
}


void Planetoid::animate(){
	Orbital::animate();
}

//-------------------------------------------------------------
// Planetoid::adapt_pass() select for scene pass
//-------------------------------------------------------------
int Planetoid::adapt_pass()
{
	clr_selected();

    if(!local_group() || !view_group() || offscreen()|| !isEnabled())
        clear_pass(BG4);
    else{
        if(TheScene->viewobj==this)
            clear_pass(FG0);
        else
            clear_pass(BG2);
    }
    return selected();
}

//-------------------------------------------------------------
// Planetoid::render_pass() select for scene pass
//-------------------------------------------------------------
int Planetoid::render_pass()
{
	clr_selected();

    if(!local_group() || offscreen()|| !isEnabled())
		return 0;
	if(view_group() || near_group()){
		if(TheScene->viewobj==this/* || TheScene->orbital_view()*/)
			select_pass(FG0);
		else
			clear_pass(BG2);
		}
	else
		clear_pass(BG3);
	if(selected() && map->visbumps() && Render.bumps())
	    	Raster.set_bumptexs(1);
	
    return selected();
}

//-------------------------------------------------------------
// Planetoid::shadow_pass() select for scene pass
//-------------------------------------------------------------
int Planetoid::shadow_pass()
{
	clr_selected();

    if(!view_group())
		return 0;
	if(TheScene->viewobj==this)
		select_pass(FGS);
	else if(type()==ID_MOON || type()==ID_PLANET){
		Planetoid *parent=(Planetoid*)getParent();
		if(TheScene->viewobj==parent){
			//if(TheScene->radius>0.5*orbit_radius)
			//	select_pass(FGS);
			//else
				select_pass(BGS);
			//cout << "radius:"<<orbit_radius<<" Scene radius:"<<TheScene->radius<<endl;
		}
		else
			select_pass(BGS);
	}
	else
		select_pass(FGS);
    return selected();
}

//-------------------------------------------------------------
// Planetoid::render() render the object and it's children
//-------------------------------------------------------------
void Planetoid::render() {
	if (!isEnabled())
		return;
	LinkedList<Object3D*> inlist;
	LinkedList<Object3D*> outlist;
	LinkedList<Object3D*> shells;

	Object3D *obj;
	children.ss();

	while ((obj = children++)) {
		if(!obj->included())
			continue;
		shells.add(obj);
	}
	shells.ss();

	while ((obj = shells++)) {
		if (obj->inside() && TheScene->viewobj == this)
			inlist.add(obj);
		else
			outlist.add(obj);
	}

	ValueList<Object3D*> insiders(inlist);
	ValueList<Object3D*> outsiders(outlist);

	set_ref();

	if (included()) {
		TheScene->pushMatrix();
		set_tilt();
		set_rotation();
		setMatrix();
		render_object();
		TheScene->popMatrix();
		if (!isViewobj()){
			TheScene->set_frontside();
			//Orbital::render(); // render children
			if (outsiders.size > 0) {
				outsiders.ss();
				while ((obj = outsiders++)) { // front-to-back
					TheScene->pushMatrix();
					obj->render();
					TheScene->popMatrix();
				}
			}
			return;
		}
	}
	if (isViewobj()) {
		TheScene->set_backside();
		if (insiders.size > 0) {
			// sort objects by radius
			insiders.se();
			while ((obj = insiders--)) { // back-to-front
				TheScene->pushMatrix();
				obj->render();
				TheScene->popMatrix();
			}
		}
		TheScene->set_frontside();
		if (outsiders.size > 0) {
			outsiders.ss();
			while ((obj = outsiders++)) { // front-to-back
				TheScene->pushMatrix();
				obj->render();
				TheScene->popMatrix();
			}
		}
	} else
		Orbital::render(); // render children
}

//-------------------------------------------------------------
// Planetoid::init_render()   render setup
//-------------------------------------------------------------
void Planetoid::init_render()
{
	checkForOcean();
 	visit(&Object3D::init_render);

	double dp=0;
	double dpmin=1,dpmax=-1;
	Raster.blend_factor=Raster.darken_factor=0;

	if(TheScene->viewobj==this){
		Point lp;
		Point vp=TheScene->vpoint;
		for(int i=0;i<Lights.numLights();i++){
			Point c=point.mm(TheScene->invViewMatrix);
			Point cv=vp-c;
			cv=cv.normalize();
		    Point l=Lights[i]->point.mm(TheScene->invViewMatrix);
		    Point cl=l-c;
		    cl=cl.normalize();
			double dpl=-cv.dot(cl);
			dpmax=dpl>dpmax?dpl:dpmax;
			dpmin=dpl<dpmin?dpl:dpmin;	
		}
		if(dpmin<=0 && dpmax<=0)
			dp=dpmax;
		else
			dp=dpmin;

		Raster.set_ldp(dp);
		if(!TheScene->inside_sky()){
			Raster.haze_value=0;
			double f=rampstep(def_twilite_max,def_twilite_min,dp,0,def_twilite_value);
			Raster.blend_factor=f;
			Raster.darken_factor=def_twilite_value*f;
		}
	}	

 	glDisable(GL_FOG);

 	// Sky.init_render sets Raster.sky_color and Raster.twilite_color

	//if(TheScene->viewobj==this)
  	//	Raster.sky_color=shadow_color;

	Raster.shadow_value=shadow_color.alpha();
 	Raster.shadow_color=shadow_color;
 	Raster.shadow_color.set_alpha(1);

    double clarity=0;// set water factors
 	if(water()){
		clarity=ocean->liquid->clarity;
		if(TheScene->viewobj!=this || TheScene->viewtype !=SURFACE)
			clarity*=20;
		Raster.water_clarity=clarity*FEET;
		Raster.water_mix=ocean->liquid->mix;
		Raster.ice_clarity=ocean->solid->clarity*FEET;
		Raster.ice_mix=ocean->solid->mix;
 	}
	Td.clarity=clarity;
    // set fog factors

	Raster.fog_value=0;

	double df=sqrt(Raster.darken_factor);
	if(fog_value){
		Color c=fog_color;
		c=c.darken((1-fog_glow)*Raster.darken_factor);
		Raster.modulate(c);
		Raster.fog_min=fog_min;
		Raster.fog_max=fog_max;
		Raster.fog_vmin=fog_vmin;
		Raster.fog_vmax=fog_vmax;
		Raster.fog_value=fog_value;
		Raster.fog_color=c;
	}
	else
		Raster.fog_value=0;
}

//-------------------------------------------------------------
// Planetoid::adapt_object()   adapt setup
//-------------------------------------------------------------

void Planetoid::adapt_object()
{
	if(!isEnabled())
		return;

	checkForOcean();
	calcAveTemperature();
	Tave=temperature;
	Tsol=ocean->solid->temp;
	Tgas=ocean->liquid->temp;	
	
	double p=TheScene->phi;
	Phi=TheScene->phi;
	Theta=TheScene->theta;

	debug_temp=DEBUG_TEMP;
	double t=calcLocalTemperature(false);
	
	surface_temp=t;  // K
	
    double dt=fabs(last_temp-surface_temp);
    setDateString();
	if(water() && dt>0.25){
		if(debug_temp>0){
			char date[256]={0};
			getDateString(date);
			cout<<"rebuild "<<date<<" "<<t<<" "<<last_temp<<" "<<dt<<endl;
		}
		invalidate();
		TheScene->set_changed_detail();
		TheScene->rebuild();
		last_temp=surface_temp;
	}
	debug_temp=0;

	Raster.frozen=ocean_state==SOLID?true:false;
    if(water()){
		Td.clarity=ocean->liquid->clarity*FEET;
		Raster.sea_level=ocean_level;
		Raster.water_color1=ocean->liquid->color1;
		Raster.water_color2=ocean->liquid->color2;
		Raster.water_clarity=Td.clarity;
	
		Raster.ice_color1=ocean->solid->color1;
		Raster.ice_color2=ocean->solid->color2;
		Raster.ice_clarity=ocean->solid->clarity*FEET;
    }
    Spheroid::adapt_object();
}


//-------------------------------------------------------------
// Planetoid::render_object()   object level render
//-------------------------------------------------------------
void Planetoid::render_object()
{
 	first=1;
	set_lighting();
	Raster.init_lights(1);
	show_render_state(this);
	Temp=surface_temp;
	
    if(TheScene->bgpass==BG4){
		map->set_mask(1);
		Color c=Raster.blend_color;
	    glColor4d(c.red(),c.green(),c.blue(),1);
		map->render_as_point();
		//map->render_zvals();
	    map->set_mask(0);
	}
	else /*if(water())*/{
		if(water()){
			Raster.sea_level=ocean_level;
			Color c= ocean->solid->color1;
			Raster.modulate(c);
			c.set_alpha(ocean->solid->color1.alpha());
			Raster.ice_color1=c;
			c=ocean->solid->color2;
			Raster.modulate(c);
			Raster.ice_color2=c;
			c=ocean->liquid->color1;
			Raster.modulate(c);
			c.set_alpha(ocean->liquid->color1.alpha());
			Raster.water_color1=c;
			c=ocean->liquid->color2;
			Raster.modulate(c);
			Raster.water_color2=c;
		}
		Spheroid::render_object();
	}

}

//-------------------------------------------------------------
// Planetoid::set_surface() set properties
//-------------------------------------------------------------
void Planetoid::set_surface(TerrainData &data)
{
	Spheroid::set_surface(data);
	if(data.type()==WATER){
		Temp=calcLocalTemperature(true);
		data.ocean=oceanState();
	}
	else{
		data.ocean=0;
	}
}

//-------------------------------------------------------------
// Planetoid::map_color()   modulate render color
//-------------------------------------------------------------
void Planetoid::map_color(MapData*d,Color &c)
{
	if(!terrain.get_root())
		c=color();

	static Point sv;
	static double a=0;
	static double wf=1;

	if(first){
		if(TheScene->viewobj==this)
			wf=rampstep(0,TheScene->maxht,TheScene->height,0.0,1);
		else
			wf=1;
 		sv=Lights[0]->point.mm(TheScene->invViewMatrix);
		sv=sv.normalize();
		a=1-Raster.sky_color.alpha();
		a=sqrt(a);
    	if(TheScene->viewobj==this)
        	Raster.shadow_darkness=1;
    	else
        	Raster.shadow_darkness=rampstep(0,1,Raster.sky_color.alpha(),1,0.7);
		first=0;
	}

	if(Raster.surface==2){ // affects orbital view only
		double g;
		double h=d->ocean();
		Color c1,c2;
        if(h>1.5){
        	c1=Raster.ice_color1;
        	c2=Raster.ice_color2;
        	g=Raster.ice_clarity;
        }
        else{
        	c1=Raster.water_color1;
        	c2=Raster.water_color2;
        	g=Raster.water_clarity;
        }
		double f=rampstep(0,g,d->depth(),0,wf);
		double alpha=c2.alpha()*(1-f)+c2.alpha()*f;
		if(TheScene->viewtype==ORBITAL && h>1.5)
			c=c2*(1-f)+c1*f;
		else{
			c=c1*(1-f)+c2*f;
	       	if(h<1){
				double df=pow(1-h,0.25);
				c=c.darken(0.2*df);
	       	}
		}
		c.set_alpha(alpha);
	}
	if(TheScene->viewobj==this)
		return;
	if(TheScene->bgpass==BG4)
		return;

 	if(TheScene->inside_sky()){
		Point p=d->mpoint();
		double dp=sv.dot(p)/p.length();
		double f=rampstep(0.2,1,dp,a,1);
		c.set_alpha(f);
	}
}

void Planetoid::set_lights(){
	if(TheScene->adapt_mode()){
		set_tod();
	}
}
void Planetoid::set_tod(){
	Point light_point=Lights[0]->point.mm(TheScene->InvModelMatrix.values());
	Point ls=light_point.spherical();
	light_theta=ls.y;
	//cout<<"light_theta:"<<light_theta<<endl;
}
//-------------------------------------------------------------
// Planetoid::set_lighting()  set light attributes
//-------------------------------------------------------------
void Planetoid::set_lighting(){
    Orbital::set_lighting();
    Lights.setAttenuation(point);
    if(water()){
		Raster.water_shine=ocean->liquid->shine;
		Raster.water_specular=ocean->liquid->specular;
		Raster.ice_shine=ocean->solid->shine;
		Raster.ice_specular=ocean->solid->specular;
    }
	Lights.modDiffuse(Raster.sky_color);
	Lights.modSpecular(Raster.blend_color);
}

//-------------------------------------------------------------
// Planetoid::tilt_bias() get seasonal light variation
//-------------------------------------------------------------
double Planetoid::tilt_bias(){
	double a=orbit_angle;
	if(type()==ID_MOON)
		a=((Planetoid *)getParent())->orbit_angle;
	return season_factor*RPD*tilt*cos(RPD*(a));
}

bool Planetoid::tidalLocked(){
	double f=day/year/24;
	if(fabs(1-f)<1e-4)
		return true;
	return false;
}
void Planetoid::setTidalLocked(){
	year=day/24;
	rot_phase=0;
}

static char date[256]={0};
void Planetoid::getDateString(char *s){
	strcpy(s,date);
}
//-------------------------------------------------------------
// Planetoid::getDate() get date string
//-------------------------------------------------------------
void Planetoid::setDateString(){
	static char *months[]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
	double angle=orbit_angle;
	if(type()==ID_MOON)
		orbit_angle=((Planetoid *)getParent())->orbit_angle;
	angle/=360;
	double month=fmod(angle*12,12);
	double day=(angle*360-((int)month)*30)+1;
	double dt=calc_delt();
	double dd=-dt*24;
	//double dd=fabs(dt*24);
	char *locked=tidalLocked()?"L":" ";

    char *apm=fabs(dd)<12?"AM":"PM";
    int dap=fabs(dd);
    int min=60*(fabs(dd)-dap);
     
    dap=dap>12?dap-12:dap;

	sprintf(date,"%-3s %-2d %2d:%02d %s %s",months[(int)month],(int)day,dap,min,apm,locked);
}

//-------------------------------------------------------------
// Planetoid::getDate() get temperature string
//-------------------------------------------------------------
void Planetoid::getTempString(char *s){
	double at=temperature;//getTemperature();
	double lt=surface_temp;//getTemperature();
	int tmode=TheScene->tempmode();
	switch(tmode){
	case 0:
		sprintf(s,"%3.1f(%3.1f) K",at,lt);
		break;
	case 1:
		sprintf(s,"%3.1f(%3.1f) C",K2C(at),K2C(lt));
		break;
	case 2:
		sprintf(s,"%3.1f(%3.1f) F",K2F(at),K2F(lt));
		break;
	}

}

//-------------------------------------------------------------
// Planetoid::calcAveTemperature() calculate ave surface temperature
// -increases for: %green house gas, pressure, temp of star
// -decreases for: distance from star, albedo
//-------------------------------------------------------------
// data      mercury  venus   earth    mars     titan
//------------------------------------------------------------
// dist      36       67      93       141      886
// albedo    0.12     0.75    0.30     0.16     0.4
// pressure  0        92      1        0.01     1.4
// %ghg      0        96      1        95        5
// Tobs      176      475     15      -60      -179
// Tcalc     174      496     21      -42      -180
// delta     -2       21      6        18       -1
//-------------------------------------------------------------
void Planetoid::calcAveTemperature() {

	if(System::TheSystem==0){
		return;
	}
	Orbital *obj;
	
	heat_factor = 1;
	children.ss();
	Sky *sky = (Sky*) getChild(ID_SKY);
	if(sky)
		heat_factor=sky->getHeatCalacity();
	//cout<<g<<endl;
	//g = 0.4 * pow(g, 0.25);
	char str[1024];
	str[0]=0;
	
	double radius=orbit_radius;
	if(typeValue()==ID_MOON)
		radius=((Orbital*)getParent())->orbit_radius;

	System::TheSystem->children.ss();
	temperature = 0;
	double Tp=0;
	double Tg=0;
	
	while ((obj = (Orbital*) System::TheSystem->children++)) {
		if (obj->type() == ID_STAR) {
			double d = radius-((Star*)obj)->size;//point.distance(obj->point);// - obj->size - size;
			double Ts = ((Star*) obj)->temperature;
			//cout<<"distance:"<<d<<endl;
			double f = 0.452; // constant relating star to planet temp
			double a = pow(1 - albedo, 0.25);
			Tp= Ts * f * a / sqrt(d);
			Tg = Tp * heat_factor; //Tp*pow(tg,0.25);
			temperature += Tp + Tg;
		}
	}

#ifdef DEBUG_AVE_TEMP
	double tc=temperature;
	double tf=tc*9.4/5.0+32;
	sprintf(str,"Temp:%-3.0f Tp:%-2.0f Tg:%-2.1f g:%f",temperature,Tp,Tg,heat_factor);
	//cout<<str<<endl;
#endif
	//cout<<"ocean state="<<ocean_state<<" temp:"<<temp<<" solid:"<<ocean_solid_temp<<" liquid:"<<ocean_liquid_temp<<endl;
}

//-------------------------------------------------------------
// Orbital::calcLocalTemperature() calculate temperature at surface
// if planet tilt exists adjusts latitude bias as a function of "season"
// - if ocean exists this function is called for every vertex to set water state
// - also called once per render pass to set texture latitude bias
// limitations:
// - doesn't work for moons (orbits planet not star)
// - doesn't work tidal locked planets (need theta dependence)
// another idea to try
// use light-normal dot product averaged over day to estimate temperature
// - for per vertex case need to sample at least 4 light vector values day phase (0,90,180,270)
// o scene->pushMatrix()
// o for each phase
//   - add/subtract calculated delta time to scene->time
//   - call scene->animate() and scene->setLights()
//   - calculate light/normal dp (add to sum)
//    * could use surface normal vs face normal
// o determine temperature from dp sum
//    * at equator sum should be ~0 at poles dp sum varies by season
//-------------------------------------------------------------
double Planetoid::calcLocalTemperature(bool w){
	double tave=temperature; // K
	double s,ds=0,f,df=1,dt,hf=1;
	double g=RPD*Phi;
	double t=tave;
	if(seasonal){
		g+=tilt_bias();
		if(tidalLocked()){
			dt=-cos(RPD*dlt()); // ~ time of day
			df=t*dt*day/year/24;
			hf=df*lerp(heat_factor,0,2.25,0.5,0);
			t+=hf;
		}
	}
	s=sin(g);
	ds=tave*temp_factor*s*s;
	t-=ds; // C
	Sfact=K2C(t)/K2C(temperature)-1;

	Temp=t;
	if(w && ocean->ocean_expr){
		f=ocean->evalOceanFunction();
		double gf=t > ocean->liquid->temp?liquidToGas(t):1;		
		t+=gf*f;
	}
	if(debug_temp>1){
		char date[256]={0};
		getDateString(date);
		cout<<date<<" A:"<<orbit_angle<<" Phi:"<<Phi<<" Ta:"<<K2C(tave)<<" f:"<<f<<" sf:"<<Sfact<<" Temp:"<<K2C(t)<<endl;
	}
	if(ocean_auto){
		if (t <= ocean->solid->temp)
			ocean_state = SOLID;
		else if (t <= ocean->liquid->temp)
			ocean_state = LIQUID;
		else
			ocean_state = GAS;
	}
	Raster.frozen=ocean_state==SOLID?true:false;
	Temp=t;
	return t;
}

void Planetoid::checkForOcean(){	
	have_water=terrain.hasChild(ID_WATER,true);
}
bool Planetoid::water(){	
	return have_water;
}
//-------------------------------------------------------------
// Planetoid::liquid() return true if not ocean_auto and state=liquid
//-------------------------------------------------------------
bool Planetoid::liquid(){
	int state=ocean_state;
	if(ocean_auto)
		return false;
	return state==LIQUID?true:false;
}
//-------------------------------------------------------------
// Planetoid::solid() return true if not ocean_auto and state=solid
//-------------------------------------------------------------
bool Planetoid::solid(){
	int state=ocean_state;
	if(ocean_auto)
		return false;
	return state==SOLID?true:false;
}
//-------------------------------------------------------------
// Planetoid::gas() return true if not ocean_auto and state=gas
//-------------------------------------------------------------
bool Planetoid::gas(){
	int state=ocean_state;
	if(ocean_auto)
		return false;
	return state==GAS?true:false;
}

//-------------------------------------------------------------
// Planetoid::solidToLiquid() return smooth transition between solid and liquid states (1..0)
//-------------------------------------------------------------
double Planetoid::oceanState(){
	double ds;
	double trans;
	if(Temp<ocean->liquid->temp){
		trans=ocean->solid->temp+ocean->solid->trans_temp;
		ds=smoothstep(ocean->solid->temp,trans,Temp,1.0,0)+1;		
	}
	else{
		trans=ocean->liquid->temp+ocean->liquid->trans_temp;
		ds=smoothstep(Temp,ocean->liquid->temp,trans,1,0);
	}
	return ds;
}
//-------------------------------------------------------------
// Planetoid::solidToLiquid() return smooth transition between solid and liquid states (1..0)
//-------------------------------------------------------------
double Planetoid::solidToLiquid(double t){
	if(!ocean_auto){
		return ocean_state==SOLID?1:0;
	}
	Temp=t;//calcLocalTemperature(true);//Temp;//
	double trans=ocean->solid->temp+ocean->solid->trans_temp;
	double dt=smoothstep(ocean->solid->temp,trans,Temp,1.0,0);
	return dt;
}

//-------------------------------------------------------------
// Planetoid::liquidToGas() return smooth transition between liquid and gas states (0..1)
//-------------------------------------------------------------
double Planetoid::liquidToGas(double t){
	double f=0;
	if(!ocean_auto){
		return ocean_state==GAS?1:0;
	}
	if (Temp > ocean->liquid->temp){
		double trans=ocean->liquid->temp+ocean->liquid->trans_temp;
		//f=smoothstep(Temp,ocean->oceanGasTemp(),trans,0.0,1);
		f=(Temp-ocean->liquid->temp)/ocean->liquid->trans_temp;
	}
	return f;
}

double Planetoid::dlt(){
	if(Lights.numLights()==0)
		return 0;
	//double tl=light_point.spherical().x;
	double tv=Theta;//TheScene->theta;
	double dv=tv-light_theta;
	return dv;
}
//-------------------------------------------------------------
// Planetoid::calc_delt() determine (normalized) position of the sun.
//-------------------------------------------------------------
double Planetoid::calc_delt()
{
	double dv=dlt();
	double wt=unwrap(last_dt,dv);
	double del=P360(wt)/360;
	last_dt=wt;
	return del;
}

//-------------------------------------------------------------
// Planetoid::calc_time() determine time-of-day (units=seconds).
//-------------------------------------------------------------
double Planetoid::calc_time(double t)
{
	double dt=calc_delt();
	double del=dt+t;
	double tm=del*day*3600;
	return tm;
}

//-------------------------------------------------------------
// Planetoid::set_time() set Scene time using TOD.
//-------------------------------------------------------------
void Planetoid::set_time(double t)
{
	double tm=calc_time(t)+TheScene->time;
	TheScene->ftime=tm;
	TheScene->set_time(tm);
}

//-------------------------------------------------------------
// Planetoid::set_view_info()
//-------------------------------------------------------------
void Planetoid::set_view_info()
{
	static char *hsyms[8]={"E","NE","N","NW","W","SW","S","SE"};
	if(TheScene->viewobj==this){
		double h=P360(TheView->heading);
		h=h<0?360+h:h;
		double indx=fmod(8*h/360+0.5,8.0);
		// heading 0 = E, 180 = W, 90 = N, 270=S
		//cout<<"heading:"<<TheView->heading<<" angle:"<<h<<" indx:"<<indx<<endl;
		TheView->set_vstring("%s",hsyms[(int)indx]);
	}
}


int Planetoid::planet_id=0;
int Planetoid::planet_cnt=0;
int Planetoid::moon_cnt=0;
int Planetoid::layer_cnt=0;
int Planetoid::num_layers=1;
int Planetoid::set_layers=0;
static bool use_2d_tmps=true;
static bool use_1d_tmps=false;

int Planetoid::tcount=0;
double Planetoid::planet_orbit=0;

static NameList<ImageSym*> icy_list;
static NameList<ImageSym*> rocky_list;
static NameList<ImageSym*> gassy_list;

static NameList<ImageSym*> hmaps_list;
static NameList<ImageSym*> image_list;
static NameList<ImageSym*> erode_list;

void Planetoid::makeLists(){
	icy_list.free();
	rocky_list.free();
	gassy_list.free();
	hmaps_list.free();
	image_list.free();
	erode_list.free();
	LinkedList<ImageSym *> list;
	//images.getImageInfo(TMP|SPX, list);
	images.getImageInfo(TMP|SPX, list);
	list.ss();
	ImageSym *is=0;
	char *name;
	while(is=list++){
		name=is->name();
		if(name[0]=='H')
			hmaps_list.add(is);
		else if(name[0]=='L')
			image_list.add(is);
		else if(name[0]=='I')
			icy_list.add(is);
		else if(name[0]=='G')
			gassy_list.add(is);
		else if(name[0]=='P')	
			rocky_list.add(is);
		else if(name[0]=='E')	
			erode_list.add(is);
	}
	list.reset();
	images.getImageInfo(HTMAP, list);
	list.ss();
	while(is=list++){
		//erode_list.add(is);
		hmaps_list.add(is);
	}
	icy_list.sort();
	gassy_list.sort();
	rocky_list.sort();
	hmaps_list.sort();
	image_list.sort();
	erode_list.sort();
	cout<<"Planetoid::makeLists()"
			<<" P:"<<rocky_list.size
			<<" I:"<<icy_list.size
			<<" G:"<<gassy_list.size
			<<" L:"<<image_list.size
			<<" H:"<<hmaps_list.size
			<<" E:"<<erode_list.size
			<<endl;
//	bands_list.ss();
//	int i=0;
//	while(is=image_list++){
//		cout<<i++<<" "<<is->name()<<endl;
//	}
}

static char *getIcyTexName(){
	int index=NodeIF::r[5]*icy_list.size;
	char *name=icy_list[index]->name();
	//cout<<"Tex Name="<<name<<" index="<<index<<":"<<icy_list.size<<endl;
	return name;
}
static char *getGassyTexName(){
	int index=NodeIF::r[5]*gassy_list.size;
	char *name=gassy_list[index]->name();
	//cout<<"Tex Name="<<name<<" index="<<index<<":"<<gassy_list.size<<endl;
	return name;
}
static char *getRockyTexName(){
	int index=NodeIF::r[5]*rocky_list.size;
	char *name=rocky_list[index]->name();
	//cout<<"Tex Name="<<name<<" index="<<index<<":"<<rocky_list.size<<endl;
	return name;
}

static char *getRandLayerTexName(){
	int index=NodeIF::r[6]*image_list.size;
	char *name=image_list[index]->name();
	//cout<<"LTex Name="<<name<<" index="<<index<<":"<<image_list.size<<endl;
	return name;
}

static char *getRandHmapTexName(){
	int index=NodeIF::r[7]*hmaps_list.size;
	char *name=hmaps_list[index]->name();
	//cout<<"HTex Name="<<name<<" index="<<index<<":"<<hmaps_list.size<<endl;
	return name;
}
static char *getRandEmapTexName(){
	int index=NodeIF::r[7]*erode_list.size;
	char *name=erode_list[index]->name();
	//cout<<"HTex Name="<<name<<" index="<<index<<":"<<hmaps_list.size<<endl;
	return name;
}
Sky *Planetoid::newSky(int g){
	Sky *sky=Sky::newInstance(g);
	sky->size=1.05*size;
	sky->ht=sky->size-size;
	return sky;
}

Ring *Planetoid::newRing(){
	Ring *ring=Ring::newInstance();
	ring->inner_radius=(1.2+0.3*r[4])*(size);
	ring->width=(0.8+0.3*s[5])*(size);
	ring->size=ring->inner_radius+ring->width;
	ring->set_geometry();
	return ring;
}

CloudLayer *Planetoid::newClouds(bool is3d){
	CloudLayer *clouds=CloudLayer::newInstance(is3d);
	if(is3d)
		clouds->size=1.005*size;
	else
		clouds->size=1.01*size;
	clouds->ht=clouds->size-size;
	return clouds;
}
//************************************************************
// Planet class
//************************************************************
Planet::Planet(Orbital *m, double s, double r) : Planetoid(m,s,r)
{
#ifdef DEBUG_OBJS
	printf("Planet\n");
#endif
}
Planet::~Planet()
{
#ifdef DEBUG_OBJS
	printf("~Planet\n");
#endif
}

NodeIF *Planet::getInstance(NodeIF *prev,int gtype){
	planet_cnt++;
	Planet *planet=(Planet*)prev;
	setRseed(getRandValue());
	//lastn=getRandValue()*1234;
	setRands();
	setParent(prev->getParent());
	orbit_radius=planet->orbit_radius;
	newInstance(gtype);
	return this;
}

void Planet::newInstance(int gtype){
	//initInstance();
    double rs=2*s[4];
	cout<<"Planet::newInstance "<<UniverseModel::typeSymbol(gtype).c_str()<<" s:"<<rs<<endl;
	switch(gtype){
	default:
	case GN_RANDOM:
		if(rs<0)
			newInstance(GN_GASSY);
		else if(rs<0.15)
			newInstance(GN_ICY);
		else if(rs<0.3)
			newInstance(GN_OCEANIC);
		else if(rs<0.5)
			newInstance(GN_ROCKY);
		else if(rs<0.7)
			newInstance(GN_VOLCANIC);
		else
			newInstance(GN_CRATERED);
		return;		
	case GN_GASSY:
		makeLists();
		planet_id=planet_cnt+lastn;
		size=0.03*(1+0.7*s[1]);
		newGasGiant(this,gtype);
		break;
	case GN_CRATERED:
	case GN_VOLCANIC:
	case GN_ICY:
	case GN_OCEANIC:
	case GN_ROCKY:
		makeLists();
		planet_id=planet_cnt+lastn;
		size=0.001*(0.4+10*r[1]);
		newRocky(this,gtype);
		break;
	}
	
	symmetry=1;
	set_geometry();
	//setProtoValid(true);
	setNewViewObj(true);
}

enum orbital_features{
	RND_TEXNAME,
	RND_HTEXNAME,
	RND_LTEXNAME,
	RND_ETEXNAME,
	RND_NOISEFUNC,
	RND_NOISEFUNC2,
	RND_NOISEFUNC3,
	RND_NOISEOPTS,
	RND_SNOW,
	RND_FRACTAL,
	RND_FRACTAL_NOISE,
	RND_MOUNTAINS,
	RND_HRIDGES,
	RND_HCRATERS,
	RND_HROCKS,
	RND_HERODED,
	RND_HROUNDED,
	RND_HVORONI,
	RND_CANYONS,
	RND_CRATERS,
	RND_MIN,
	RND_HTWIST,
	RND_VOLCANOS,
	RND_BANDS,
	RND_HMAP_IMAGE,
	RND_EMAP_IMAGE,
	RND_HMAP_TEX,
	RND_HMAP_TWIST,
	RND_ERODE_TEX,
	RND_GLOBAL_TEX,
	RND_GLOBAL_NOISE,
	RND_TEX_NOISE,
	RND_GLOBAL_DUAL_TEX,
	RND_LOCAL_TEX,
	RND_LOCAL_IMAGE,
	RND_OCEAN_EXPR,
	RND_MAP,
	RND_LAYER,
	RND_LAYER_VAR,
	RND_CLOUDS_2D,
	RND_CLOUDS_3D,
	RND_SKY,
};
#define DESSERT_THEME     Color(0.9,0.8,0.7),Color(0.9,0.82,0.68),Color(0.62,0.52,0.44),Color(1,0.9,0.64)
#define DRK_BROWN_THEME   Color(0,0,0),Color(0.675,0.4,0.1),Color(1,0.875,0.275),Color(0.4,0.251,0.1)
#define LGT_BROWN_THEME   Color(0,0,0),Color(1,0.529,0),Color(1,0.863,0.635),Color(0.655,0.471,0.024)
#define GRAY_THEME        Color(0,0,0),Color(0.1,0.1,0.1),Color(0.5,0.5,0.5),Color(1,1,1)    
#define BLUE_THEME        Color(0.0,0.0,0.0),Color(0.012,0.384,0.422),Color(0.769,0.937,0.976),Color(1,1,1)    
#define RED_THEME         Color(0.5,0.3,0.09),Color(1,0.89,0.49),Color(0.302,0.078,0.078),Color(0.202,0.02,0.02)    
#define GREEN_THEME       Color(0,0,0),Color(0.2,0.3,0.2),Color(0.5,0.7,0.6),Color(0.7,0.8,0.5)    
Color Planetoid::themes[THEMES*4]={
		DESSERT_THEME,
		DRK_BROWN_THEME,
		LGT_BROWN_THEME,
		GRAY_THEME,
		BLUE_THEME,
		RED_THEME,
		GREEN_THEME
};
Color Planetoid::mix;
Color Planetoid::tc;
int Planetoid::ncolors=6;
Color Planetoid::colors[COLORS];

static Planetoid *planetoid=0;

void Planetoid::initInstance(){
	setRands();

}

void Planetoid::setColors(){
//	if(terrain_type==GN_ICY){
//		setIceColors();
//		return;
//	}
	double use_theme=r[7];
	if(terrain_type!=GN_ICY && use_theme>0.95){
		int index=(int)THEMES*r[8];
		ncolors=4;
		for(int i=0;i<ncolors;i++){
			tc=themes[ncolors*index+i];
			tc.set_red(tc.red()+0.05*s[i]);
			tc.set_green(tc.green()+0.05*s[i]);
			tc.set_blue(tc.blue()+0.01*s[i+2]);
			tc=tc.darken(0.2*r[i+3]);
			tc=tc.lighten(0.2*r[i+4]);
			colors[i]=tc;		
		}
		cout<<"THEME "<<index<<endl;
	}
	else{
		ncolors=6;
		mix=Color(0,0,0);
		double hue,sat,val;
        //cout<< UniverseModel::typeSymbol(terrain_type).c_str();
		switch(terrain_type){
		default:
			hue=0.2+0.2*s[3];
			sat=0.3+0.2*s[4];
			val=0.8+0.3*s[5];
			break;
		case GN_OCEANIC:
			if(tcount==0){
				hue=0.2+0.2*s[3];
				mix=Color(0.0,0.0,0);			
			}
			else{
				hue=0.1+0.1*r[3];
				mix=Color(0.2,0.2,0);
			}
			sat=0.3+0.1*s[4];
			val=0.8+0.3*s[5];
			mix=Color(0.2,0.2,0);
			break;
		case GN_ICY:
			hue=0.6+0.1*s[3];
			sat=0.8+0.2*s[4];
			val=0.8+0.2*s[5];
			mix=Color(0.7,1,1);
			break;
		case GN_GASSY:
			hue=0.6+s[3];
			sat=0.5+0.2*s[4];
			val=0.8+0.2*s[5];
			mix=Color(0.5,0.5,0);
			break;
		case GN_VOLCANIC:
			hue=0.1+0.2*s[3];
			sat=0.8+0.2*s[4];
			val=0.8+0.3*s[5];			
			break;
		case GN_CRATERED:
			hue=0.1+0.1*r[3];
			sat=0.2+0.3*s[4];
			val=0.8+0.3*s[5];			
			break;
		}
#ifdef DEBUG_GENERATE
		cout<<UniverseModel::typeSymbol(terrain_type).c_str()<<" H:"<<hue<<" S:"<<sat<<" V:"<<" "<<val<<endl;
#endif
		//Color tv=Color(base+0.1*r[3],0.5+0.2*r[4],0.5+0.3*r[5]);
		//Color tm =Color(base+0.2*s[3]+0.2,0.5+0.2*s[4],0.5+0.3*s[5]);
		tc=Color::HSVtoRGB(hue,sat,val);
		//tc.print();
//		tc=Color(0.5+3*r[4],0.4+2*r[5],0.6*r[5]);
//		mix=Color(0.5+2*r[7],0.5*r[8],0.2+0.3*r[9]);
		colors[0]=mix;
		colors[1]=tc.darken(0.5+0.2*s[6]);
		colors[2]=tc.lighten(0.5+0.1*s[7]);
		colors[3]=tc;
		colors[4]=tc.darken(0.5+0.3*s[8]);
		colors[5]=tc.lighten(0.5+0.3*s[9]);
	}
	for(int i=0;i<ncolors;i++){
		//cout<<" "<<r[i];
		colors[i]=colors[i].desaturate(0.2*r[i]+0.1);
	}
	cout<<endl;


}

void Planetoid::setIceColors(){
	// bands("neptune",ACHNL,64,0.14286,0.19354,Color(1,1,1,0.192),Color(0.031,0.216,0.251),
	//Color(0.012,0.38,0.522),Color(0.769,0.933,0.973),Color(1,1,1));
	colors[0]=Color(0.031,0.216,0.251);
	colors[1]=Color(0.012,0.38,0.522);
	colors[2]=Color(0.6,0.8,1);
	colors[3]=Color(1,1,1);
	ncolors=4;
	for(int i=0;i<3;i++){
		tc=colors[i];
		tc.set_red(tc.red()+0.2*s[i]);
		tc.set_green(tc.green()+0.2*s[i+1]);
		tc.set_blue(tc.blue()+0.2*s[i+2]);
		colors[i]=tc;		
	}
	//cout<<"setIceColors "<<endl;
	
}
//-------------------------------------------------------------
// Planetoid::randFeature() return text or random feature
//-------------------------------------------------------------
#define AMPL  r[12]
#define PROB  r[13]
#define MAXS  r[14]
static bool keep_rands=false;
static double size_scale=1;

#ifdef DEBUG_GENERATE
#define PRNT_FEATURE(s) cout<<"RND_"<<s<<":"<<buff<<endl;
#else
#define PRNT_FEATURE(s)
#endif
std::string Planetoid::randFeature(int type) {
	int nt;
	double mx,mr,md;
	if(!keep_rands)
		setRands();
	std::string str("");
	char buff[4096];
	switch(type){
	case RND_TEXNAME:			
		str="\"";
		if(TheScene->use_tmps==ALL_TMPS){
			if(planetoid->terrain_type==GN_ICY)
				str+=getIcyTexName();
			else if(planetoid->terrain_type==GN_GASSY)
				str+=getGassyTexName();
			else
				str+=getRockyTexName();								
		}
		else{
			if(planetoid->terrain_type==GN_ICY)
				str+="I";
			else if(planetoid->terrain_type==GN_GASSY)
				str+="G";
			else
				str+="P";					
			str+=std::to_string(planet_id);
		}
		str+="\"";
		break;
	case RND_HTEXNAME:
		str="\"";
		if(TheScene->use_tmps!=NO_TMPS){
			str+=getRandHmapTexName();
		}
		else{
			str+="H";
			str+=std::to_string(planet_id);
		}
		str+="\"";
		break;
	case RND_ETEXNAME:
		str="\"";
		if(TheScene->use_tmps!=NO_TMPS){
			str+=getRandEmapTexName();
		}
		else{
			str+="E";
			str+=std::to_string(planet_id);
		}
		str+="\"";
		break;
	case RND_LTEXNAME:
		str="\"";
		if(TheScene->use_tmps!=NO_TMPS)
			str+=getRandLayerTexName();
		else{
			str+="L";
			str+=std::to_string(planet_id);
		}
		str+="\"";
		break;
	case RND_NOISEFUNC:
		str=Noise::getNtype(r[5]);
		break;
	case RND_NOISEFUNC2:
		str=Noise::getNtype(r[4]);
		break;
	case RND_NOISEFUNC3:
		str=Noise::getNtype(0.6*r[3]);
		break;
	case RND_NOISEOPTS:
		str=Noise::getNopts(r[9]);
		break;
	case RND_SNOW:
		str="snow(";
		str+=std::to_string(-0.8+0.3*s[0]);
		str+=",0.3,0.8,-0.25,0.5,0.1)";
		break;
	case RND_FRACTAL_NOISE:
		str="20+3*noise(";
		str+=randFeature(RND_NOISEFUNC3);
		str+="|NLOD,8,5,0,0.5,2.2,1,4,0,0.0,1e-06)";
		break;
	case RND_FRACTAL:
		str="fractal(SQR|SS,";
		//str+=std::to_string((int)(18)); //start
		str+=randFeature(RND_FRACTAL_NOISE);
		str+=",4,0.1,0.0,1,0.1,0,0,1,0)";
		break;
	case RND_CRATERS:
		mx=MAXS*size_scale+0.1*s[2];
		md=0.1+0.5*AMPL*size_scale;
		sprintf(buff,"craters(ID%d,%d,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,0.8,0.6,0.05,0,0,%1.2f,%1.2f)",
		(int)(5+0.5*s[0]), // id
		(int)(8+0.5*s[0]), // levels
		(mx),              // max size
		(0.3+0.5*s[3]),    // delta size
		(0.1+0.6*PROB),    // probability
		(md),              // depth
		(0.3+0.3*r[6]),    // impact
		(0.9+0.3*s[7]),    // vnoise
		(0.9+0.3*s[7]),    // rnoise
		(0.2+0.1*s[9]),    // rise
		(md),              // drop
		(0.5+0.2*s[11]),   // noise bias
		(0.4+0.2*s[10]));  // noise min

		str+=buff;
		PRNT_FEATURE("CRATERS")
		break;
	case RND_VOLCANOS:
		mx=MAXS*size_scale+0.1*s[2];
		mr=0.1+0.5*AMPL*size_scale;
		sprintf(buff,"craters(ID%d,%d,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,%1.2f,0.05,0,%1.2f,%1.2f)",
		(int)(2+0.5*s[0]), // rand id
		(int)(8+0.5*s[0]), // levels
		(mx),              // max size
		(0.3+0.2*s[3]),    // delta size
		(0.1+0.6*PROB),    // probability
		(mr),              // depth
		(0.2+0.1*s[6]),    // impact
		(0.9+0.3*s[7]),    // noise
		(0.9+0.3*s[7]),    // noise
		(mr),              // rise
		(0.1*r[9]),        // drop
		(0.2+0.2*s[10]),   // rim
		(0.05+0.1*s[10]),  // floor
		(0.3+0.2*s[11]),   // noise bias
		(0.1+0.1*s[10]));  // noise min
		str+=buff;
		PRNT_FEATURE("VOLCANOS")
		break;
	case RND_OCEAN_EXPR:
		str="noise(GRADIENT|NNORM|SCALE|RO1,5,9.5,1,0.5,2,0.4,1,0,0)";
		break;
	case RND_HCRATERS:
		str="craters(ID";
		str+=std::to_string(9-layer_cnt);
		str+=",2,";
		str+=std::to_string(1+0.3*s[4]); // size
		str+=",0.5,";  // delta size
		str+=std::to_string(0.5+0.5*PROB);  // probability
		str+=",0.9,1,1,1,";
		str+=std::to_string(0.1+r[3]);  // height
		str+=",0.5,0.75,0.5,0.2)";
		break;
	case RND_HERODED:
		str="noise("+randFeature(RND_NOISEFUNC3)+"|SQR|UNS,1,6.9,0,0.56,2,1,0)";
		str=TNnoise::randomize(str.c_str(),0.3,0.0);
		break;
	case RND_HROUNDED:
		str="noise("+randFeature(RND_NOISEFUNC3)+"|NABS|NEG|SQR|UNS,1,5,0.6,0.1)";
		str=TNnoise::randomize(str.c_str(),0.3,0.0);
		break;
	case RND_HRIDGES:
		str="noise("+randFeature(RND_NOISEFUNC3)+"|NABS|NEG|SQR|UNS,0,5,-0.3,0.1,2.22,1.5,1,0,-0.4)";
		str=TNnoise::randomize(str.c_str(),0.3,0.0);
		break;	
	case RND_HROCKS:
		str+="pow(";
		str+="noise("+randFeature(RND_NOISEFUNC3)+"|NABS|NLOD|SCALE|SQR|UNS,0,4.2,0.41,0.41,2.8,1,0.4,0,0.5)*";
		str+="noise("+randFeature(RND_NOISEFUNC)+"|SCALE|UNS,1,3.3,1,0.5,2,1,4,0,-0,1e-06)";
		str+=",2)";
		break;
	case RND_HVORONI:
		str="noise(VORONOI|RO1,2,3,0.59,0.27,2.1,1,1,1,0,1e-06)";
		str=TNnoise::randomize(str.c_str(),0.3,0.0);
		break;
	case RND_CANYONS:
		str="noise("+randFeature(RND_NOISEFUNC3)+"|NABS|NEG|NNORM|SQR|RO1|SCALE,0.5,14.8,0.17,0.51,2.5,1,1,0,2,1e-06)";
		str=TNnoise::randomize(str.c_str(),0.3,0.0);
		str=std::string("min(")+str.c_str();
		str=str+",0)";
		sprintf(buff,str.c_str());
		PRNT_FEATURE("CANYONS")
		break;
	case RND_MOUNTAINS:
		str="noise("+randFeature(RND_NOISEFUNC3)+"|NABS|SQR,1.5,16,";
		str+=std::to_string(0.0+0.5*s[7]); //homogeneity
		str+=",";
		str+=std::to_string(0.3+0.1*r[7]); //atten
		str+=",";
		str+=std::to_string(2.0+0.3*r[6]); //frequency
		str+=",1,4,0,0.5,1e-06)";
		str=TNnoise::randomize(str.c_str(),0.3,0.0);
		sprintf(buff,str.c_str());
		PRNT_FEATURE("MOUNTAINS")
		break;
	case RND_BANDS:
		str="bands(";
		str+=randFeature(RND_TEXNAME);
		str+=",TMP|NORM|REFLECT,64,";
		str+=std::to_string(0.1*r[5]); // random
		str+=",";
		str+=std::to_string(0.5*r[6]); // mix
        buff[0]=0;
		for(int i=0;i<ncolors;i++){
			Color c=colors[i];
			strcat(buff,",");
			c.toString(buff+strlen(buff));
        }
	    str+=buff;
	    str+=");\n";
	    break;
	case RND_GLOBAL_NOISE:
		str="noise(GRADIENT|SQR,";
		str+=std::to_string(r[8]); // start
		str+=",4.5,";
		str+=std::to_string(0.1+s[8]); // homogeneity
		str+=",0.29,";
		str+=std::to_string(2.5+0.5*s[6]); // frequency
		str+=",2,4,0,-0,1e-06)";
		break;
	case RND_TEX_NOISE:
		keep_rands=true;
		sprintf(buff,"noise(SIMPLEX,%1.2f,12,%1.2f,0.5,%1.2f,%1.2f,%1.2f,4,0,0,1e-6)",
		1.0+s[7],     // start
		0.5+0.2*s[8], // homogeneity
		2.3+0.5*s[9], // frequency
		0.02+0.01*s[10]   // amplitude
		);
		str=buff;
		break;
	case RND_GLOBAL_DUAL_TEX:
		keep_rands=true;
		mr=tcount>1?-1:1;
		sprintf(buff,"Texture(%s,A|S|NORM|TEX|TBIAS,%s,%s%s,%1.1f,2,1,%1.2f,1,2,1,0,%1.2f,%0.2f,%1.2f,%1.2f)",
		randFeature(RND_TEXNAME).c_str(), // image name
		randFeature(RND_TEX_NOISE).c_str(), // noise
		tcount>0?"-":"",
		randFeature(RND_LAYER_VAR).c_str(),	// +- n	
		pow(2,8+2*s[0]),  // start
		(mr*r[7]),       // offset
		mr*(0.05+0.1*s[8]),     // phi bias
		(mr*0.02),        // ht bias
		0.1*(1+0.3*s[9]),  // bmp bias
		-0.1*(1+0.5*s[10]) // slope bias
		);
		PRNT_FEATURE("GLOBAL_DUAL_TEX")
		str=std::string(buff);
		keep_rands=false;
		break;
	case RND_GLOBAL_TEX:
		//keep_rands=true;
		sprintf(buff,"Texture(%s,S|NORM|TEX,%s,%1.2f,2,1,%1.2f,1,2,1,0,%1.2f,%1.2f,%1.2f,%1.2f)",
		randFeature(RND_TEXNAME).c_str(), // image name
		randFeature(RND_TEX_NOISE).c_str(), // noise
		pow(2,8+4*s[0]),  // start
		0.5*r[7],         // offset
		0.05+0.1*s[8],     // phi bias
		(0.03+0.01*s[9]), // ht bias
		0.05*(1+0.5*s[9]),     // bmp bias
		-0.1*(1+0.5*s[10]) // slope bias
		);
		str=buff;
		PRNT_FEATURE("GLOBAL_TEX")
		//keep_rands=false;
		break;
	case RND_LOCAL_IMAGE:
		//keep_rands=true;
		str="image(";
		str+=randFeature(RND_LTEXNAME);
		str+=",TMP|ACHNL|NORM,256,256,";
		if(s[7]>0.0)
			str+=randFeature(RND_HCRATERS);
		else
			str+=randFeature(RND_HRIDGES);
		str+=",";
		str+=randFeature(RND_TEXNAME);
		str+=");\n";
		//keep_rands=false;
		break;
	case RND_LOCAL_TEX:
		//keep_rands=true;
		str="Texture(";
		str+=randFeature(RND_LTEXNAME);
		str+=",BORDER|BUMP|NORM|RANDOMIZE|TEX,4096";
		str+=",0.1,2,0,7,";
		str+=std::to_string(2+0.5*r[1]);    // freq
		str+=",0.4,0,0,";
		str+=std::to_string(0.2*tcount);  // ht bias
		str+=",0,";
		str+=std::to_string(tcount); // slope bias
		str+=")\n";
		//keep_rands=false;
		break;	
	case RND_EMAP_IMAGE:
		//keep_rands=true;
		str="image(";
		str+=randFeature(RND_ETEXNAME);
		str+=",TMP|GRAY|NORM,256,256,";
		if(r[7]>0.8)
			str+=randFeature(RND_HRIDGES);
		else if(r[7]>0.2)
			str+=randFeature(RND_HERODED);
		else
			str+=randFeature(RND_HVORONI);	
		str+=");\n";
		//keep_rands=false;
		break;
	case RND_HMAP_IMAGE:
		keep_rands=true;
		str="image(";
		str+=randFeature(RND_HTEXNAME);
		str+=",TMP|GRAY|NORM|HMAP,256,256,";
		if(r[7]>0.8)
			str+=randFeature(RND_HCRATERS);
		else if(r[7]>0.6)
			str+=randFeature(RND_HRIDGES);
		else if(r[7]>0.4)
			str+=randFeature(RND_HROCKS);
		else if(r[7]>0.2)
			str+=randFeature(RND_HERODED);
		else
			str+=randFeature(RND_HVORONI);	
		str+=");\n";
		keep_rands=false;
		break;
	case RND_ERODE_TEX:
		//keep_rands=true;
		sprintf(buff,"Texture(%s,BUMP|RANDOMIZE,%1.2f,%1.2f,0,0,%d,%1.2f,0.5,0,0,%1.2f,0,%1.2f)",
		randFeature(RND_ETEXNAME).c_str(), // image name
		pow(2,8+2*s[4]),   	// start,  // start
		0.6+0.2*s[5],    	// bump ampl
		(int)(6+2*s[7]),   	// num orders
		2.3+0.2*s[8],      	// orders freq
		0.0+0.1*r[9],  		// ht bias
		0.8+0.25*s[5]  		// slope bias
		);
		str=buff;
		PRNT_FEATURE("EMAP_TEX")
		//keep_rands=false;
		break;
		//Texture("MtAdamWA",BUMP|HMAP|LINEAR|RANDOMIZE|S,noise,2.27,0.14,0,0,5.97,2.21,0.9,0,1.09,0.7,0,0.5,0,0
	case RND_HMAP_TEX:
		keep_rands=true;
		sprintf(buff,"Texture(%s,HMAP|LINEAR|S|RANDOMIZE|BUMP,%s,%1.2f,%1.2f,0,0,%1.2f,%1.2f,%1.2f,0,%1.2f,%1.2f,0,0,0)",
		randFeature(RND_HTEXNAME).c_str(), // image name
		randFeature(RND_HMAP_TWIST).c_str(),
		pow(2,1+0.5*s[4]),  // start
		0.0+0.2*s[5],  // bump ampl
		5+2*s[6],      // orders
		2.3+0.2*s[7],  // freq
		0.5+0.1*r[8],  // orders ampl
		1+0.25*s[9],  // H ampl
		0.5+0.25*s[10]  // offset
		);
		str=buff;
		PRNT_FEATURE("HMAP_TEX")
		keep_rands=false;
		break;
	case RND_HMAP_TWIST:
		str="noise(";
		str+=randFeature(RND_NOISEFUNC2);
		str+=randFeature(RND_NOISEOPTS);
		str+="|SCALE,0.5,12,";
		str+=std::to_string(-0.2+0.3*s[8]); // homogeneity
		str+=",";
		str+=std::to_string(0.5+0.1*s[7]); // attenuation
		str+=",";
		str+=std::to_string(2.5+0.3*s[6]); // frequency
		str+=",0.3,4,0,0)";
		break;
	case RND_MAP:
		str="map(noise(";
		str+=randFeature(RND_NOISEFUNC2);
		str+=randFeature(RND_NOISEOPTS);
		str+=",";
		str+=std::to_string(r[4]); // start
		str+=",14,";
		str+=std::to_string(r[4]); // homogeneity
		str+=",0.5,2.3))";
		//GRADIENT,0,14.13,1,0.5,2,1,4,0,0,1e-06
		//str="map(noise(1,5))";
		break;
	case RND_LAYER:
		// morph, width,drop,ramp
		//str="layer(MESH,0)[";
		str="layer(MESH,";
		str+=std::to_string(r[4]); // morph
		str+=",";
		str+=std::to_string(0.1+0.2*r[5]); // width (could be noise modulated)
		str+=",";
		str+=std::to_string(0.5*r[6]); // drop
		str+=",";
		str+=std::to_string(r[7]); // eamp
		str+=")[";
		break;
	case RND_LAYER_VAR:
		str="n";
		str+=std::to_string(layer_cnt);
		break;
	case RND_CLOUDS_2D:
		str="Color(1,1,1,twist(0.1,noise(";
		str+=randFeature(RND_NOISEFUNC3); // perlin or simplex
		str+="|FS|NABS|SQR|TA,1,8,1,";
		str+=std::to_string(0.5+0.4*r[6]);    // offset
		str+=",";
		str+=std::to_string(2+r[1]);    // freq
		str+=",1,1,0,0.4,1e-5)))";
		break;
	case RND_CLOUDS_3D:
		str="clouds(HT,-0.2*HT*(1-0.1*noise(GRADIENT|NABS|SQR,9,4,0.1,0.6)),2+60*HT)";
		str+="Z(0.1*noise(GRADIENT|SQR,4.3,12,0.2,0.5,2,1,1,0,0,1e-06))+Color(1+4*HT,RED,RED,1)";
		break;
	}
	return str;
}

void Planet::newRocky(Planet *planet,int gtype){
	cout<<"new Rocky Planet"<<endl;
	Planetoid::newRocky(planet,gtype);
	planet->day=24*(4+3*s[5]);
	planet->year=planet->day*(1+3*r[3]);
	planet->tilt=50*r[8];
	planet->hscale=0.002;
}
void Planetoid::pushInstance(Planetoid *planet){
	nsave=lastn;
	lastn+=ncount*131;
	planet->initInstance();

	planet_id=planet_cnt+lastn;
	ncount++;
}
void Planetoid::popInstance(Planetoid *planet){
	lastn=nsave;
}

std::string newHmapTex(Planetoid *planet){
	std::string str;
	if(TheScene->use_tmps==NO_TMPS){
		char buff[2048];
		std::string str=Planetoid::randFeature(RND_HMAP_IMAGE);
		strcpy(buff,str.c_str());	
		TNinode *himg=(TNinode*)TheScene->parse_node(buff);
		himg->init();
		planet->add_image(himg);
	}
	if(planet->terrain_type==GN_ICY){
		Planetoid::r[9]*=2;
		Planetoid::r[6]*=2;
	}
	return Planetoid::randFeature(RND_HMAP_TEX);
}
std::string newErodeTex(Planetoid *planet){
	std::string str;
	if(TheScene->use_tmps==NO_TMPS){
		char buff[2048];
		std::string str=Planetoid::randFeature(RND_EMAP_IMAGE);
		strcpy(buff,str.c_str());	
		TNinode *himg=(TNinode*)TheScene->parse_node(buff);
		himg->init();
		planet->add_image(himg);
	}
	return Planetoid::randFeature(RND_ERODE_TEX);
}
std::string newGlobalTex(Planetoid *planet){
	if(TheScene->use_tmps!=ALL_TMPS){
		char buff[2048];	
		std::string str = Planetoid::randFeature(RND_BANDS);
		strcpy(buff, str.c_str());
		TNinode *img = (TNinode*) TheScene->parse_node(buff);
		img->init();
		planet->add_image(img);
	}
	return Planetoid::randFeature(RND_GLOBAL_TEX);
}

std::string newDualGlobalTex(Planetoid *planet){
	char buff[2048];
	Planetoid::pushInstance(planet);
	//planet->setColors();
	std::string str;
//	if(planet->terrain_type==GN_ICY){
//		Planetoid::r[5]*=0.25;
//		Planetoid::r[6]*=0.25;
//	}
	if(TheScene->use_tmps!=ALL_TMPS){
		str = Planetoid::randFeature(RND_BANDS);
		strcpy(buff, str.c_str());
		TNinode *img = (TNinode*) TheScene->parse_node(buff);
		img->init();
		planet->add_image(img);	
	}
	str=Planetoid::randFeature(RND_GLOBAL_DUAL_TEX);
	Planetoid::popInstance(planet);
	return str;
}

std::string newLocalTex(Planetoid *planet){
	char buff[2048];	
	Planetoid::pushInstance(planet);
	std::string str;
	if(TheScene->use_tmps!=ALL_TMPS){
		str = Planetoid::randFeature(RND_BANDS);
		strcpy(buff, str.c_str());
		TNinode *img = (TNinode*) TheScene->parse_node(buff);
		img->init();
		planet->add_image(img);	
	}
	if(TheScene->use_tmps==NO_TMPS){	
		str=Planetoid::randFeature(RND_LOCAL_IMAGE);
		strcpy(buff,str.c_str());	
		TNinode *himg=(TNinode*)TheScene->parse_node(buff);
		himg->init();
		planet->add_image(himg);
	}
    str=Planetoid::randFeature(RND_LOCAL_TEX);
    Planetoid::popInstance(planet);
	return str;
}

std::string Planetoid::newOcean(Planetoid *planet){	
	cout<<"new Ocean"<<endl;
	double level=s[0]*FEET;
	pushInstance(planet);

	TNwater *water=TheScene->getPrototype(0,TN_WATER);
	planet->terrain.set_root(water);
	OceanState *state=OceanState::newInstance();

	char buff[2045];
	buff[0]=0;
	
	water->setNoiseExprs(state);
	water->propertyString(buff);

	planet->terrain.set_root(0);
	planet->ocean_level=level;
	planet->ocean_auto=1;
	state->setOceanFunction((char*)randFeature(RND_OCEAN_EXPR).c_str());

	popInstance(planet);
	
	return std::string(buff);
}

std::string Planetoid::newLayer(Planetoid *planet){	
	//cout<<"new Layer "<<layer_cnt<<endl;
	std::string str;
	pushInstance(planet);
	planet->setColors();
	tcount=0;
	layer_cnt++;
	std::string var_name=randFeature(RND_LAYER_VAR);

	str=randFeature(RND_LAYER);
	// -- layer start
	// 1) pushinstance
	// 2) for each layer
	// 3) add layer header
	if(s[2]>0.5)
		str+=newGlobalTex(planet);
	else{
		planet->addTerrainVar(var_name.c_str(),randFeature(RND_GLOBAL_NOISE).c_str()); // need new variable
		tcount=0;
		str+=newDualGlobalTex(planet);
		str+="+";
		tcount=1;
		planet->setColors();
		str+=newDualGlobalTex(planet);
	}
	for(tcount=-1;tcount<2;tcount+=2){
		str+="+";
		str+=newLocalTex(planet);
	}
	tcount=0;
	str+="+";
	str+=newErodeTex(planet);
	str+="+";
	str+=newHmapTex(planet);
	str+="+Z(";
	switch(planet->terrain_type){
	case GN_OCEANIC:
		PROB*=0.1; // probability
		AMPL*=0.1; // ampl/rise/drop
		MAXS*=0.1; // max size
		if(r[10]>0.6)
			str+=randFeature(RND_VOLCANOS);
		if(r[10]>0.8)
			str+=randFeature(RND_CRATERS);
		break;
	case GN_ROCKY:
		PROB*=0.25; // probability
		AMPL*=0.25; // rise/drop
		MAXS*=0.25; // max size
		if(r[10]>0.7)
			str+=randFeature(RND_VOLCANOS);
		if(r[11]>0.8)
			str+=randFeature(RND_CRATERS);
		break;
	case GN_VOLCANIC:
		keep_rands=true;
		AMPL=AMPL+0.5;
		MAXS=MAXS+0.2;
		str+=randFeature(RND_VOLCANOS);
		//keep_rands=false;
		break;
	case GN_CRATERED:
		keep_rands=true;
		AMPL=AMPL+0.5;
		MAXS=MAXS+0.2;		
			str+=randFeature(RND_CRATERS);
		//keep_rands=false;
		break;
	case GN_ICY:
		if(r[11]>0.3)
			str+=randFeature(RND_CRATERS);
		break;
	}
	str+=randFeature(RND_MOUNTAINS);
	str+="+";
	str+=randFeature(RND_CANYONS);
    // ]
	// popinstance
	str+=")";
	str+="]";
	popInstance(planet);
	//cout<<"new Layer generated:"<<str.c_str()<<endl;

	return str;
	
}
void Planetoid::newRocky(Planetoid *planet, int gtype){
	char surface[5000];
	char buff[10000];
	int nsave=lastn;
	std::string str;
	ncount=0;
	Sky *sky=0;
	str="";
	planet->terrain_type=gtype;
	planetoid=planet;
	
	//cout<<"new rocky "<<planet_cnt<<endl;
	// decrease ht and max size of craters for larger planets
	size_scale=lerp(planet->size,0.001,0.01,1,0.5);  
	//cout<<" "<<planet->size<<" "<<size_scale<<endl;
	
    switch(gtype){
    default:
    case GN_VOLCANIC:
    case GN_CRATERED:
    case GN_ROCKY:
    	if(r[2] > 0.7 && planet->size >= 0.001){
    		sky= planet->newSky(GN_THIN);
   			planet->addChild(sky);
    		if (sky->pressure >= 0.5) {
    			CloudLayer *clouds=planet->newClouds(false);
    			planet->addChild(clouds);
    		}
     	}
    	break;
    case GN_ICY:
		//str+=newOcean(planet);
    	break;
    case GN_OCEANIC:
		str+=newOcean(planet);
		sky= planet->newSky(GN_MED);
		planet->addChild(sky);
		CloudLayer *clouds;
		clouds = planet->newClouds(false);
		if(sky->pressure >=1){
			clouds = planet->newClouds(true);
			planet->addChild(clouds);				
		}
		planet->calcAveTemperature();
		if(planet->temperature<400){
			str+=randFeature(RND_SNOW);
		}
 		planet->shadow_color.set_alpha(0.5);
   		break;
    }
	//if(num_layers<3)
		str+=randFeature(RND_FRACTAL);
	
    if(set_layers==0){
		int max_layers=TheScene->generate_quality;	
		num_layers=num_layers>max_layers?max_layers:num_layers;
		num_layers+=1;
    }
    else
    	num_layers=set_layers;
    //cout<<"layers="<<num_layers<<" "<<set_layers<<endl;
	str+=randFeature(RND_MAP);
	
	layer_cnt=0;
	for(int i=0;i<num_layers;i++)
		str+=newLayer(planet);


	strcpy(buff,str.c_str());
	cout<<"text generated length:"<<strlen(buff)<<endl;
	
	TNode *root=TheScene->parse_node(buff);
	if(root){
	    planet->terrain.set_root(root);
	    planet->terrain.init();
	}
	else
	    cout<<"error building planet surface"<<endl;
	
	if(planet->typeValue()==ID_PLANET){
		int moons=r[7]*2.1;
		int max_moons=TheScene->generate_quality;
		moons=moons>max_moons?max_moons:moons;
		moon_cnt=0;
		for(int i=0;i<moons;i++){
			((Planet *)planet)->addMoon(gtype);			
		}
	}
	char pname[64];
	sprintf(pname,"%s-l%d",UniverseModel::typeSymbol(planet->terrain_type).c_str(),num_layers);
	planet->setName(pname);
	planet_id=planet_cnt+lastn;
	planet->get_vars();

}

void Planet::addMoon(int gtype){
	Planet *moon=TheScene->getPrototype(this,ID_MOON);
	moon->setParent(this);
	double max_size=0.2*size;
	pushInstance(this);
	moon->size=max_size*(1-0.6*r[6]);
	moon->orbit_radius=size*(2+moon_cnt+5*r[5]);
	moon->orbit_distance=moon->orbit_radius;
	moon->orbit_phase=360*r[7];
	moon->tilt=60*r[8];
	moon_cnt++;
	cout<<"new Moon size:"<<(moon->size/size)<<endl;
	newRocky(moon,gtype);
	addChild(moon);
	popInstance(this);
}
void Planet::newGasGiant(Planet *planet, int gtype){
	cout<<"building new gas giant"<<endl;
	planet->setName("Gaseous");

	planet->day=100+90*r[2];
	planet->year=planet->day*(1+3*r[3]);
	planet->detail=4;
	
	planet->terrain_type=GN_GASSY;
	planetoid=planet;
	planet->setColors();

	char buff[4096];
	if(TheScene->use_tmps!=ALL_TMPS){
		std::string str=randFeature(RND_BANDS);
		strcpy(buff,str.c_str());
	
		TNinode *img=(TNinode*)TheScene->parse_node(buff);
		img->init();
		Render.invalidate_textures();
		planet->add_image(img);
	}
	std::string  noise_func=randFeature(RND_NOISEFUNC);
	
	char noise_expr1[2048];
	char noise_expr2[2048];
	char noise_expr3[2048];
	double twist=0.2+0.1*r[6];
	sprintf(noise_expr1,"%g*twist(%g,noise(%s|FS|NABS|SQR|TA|UNS|RO1,2.4,6,1,0.55,2.25,0.3,1,0,0,1e-6))",twist,0.2+0.1*s[2],noise_func.c_str());
	double storms=0.4+0.1*s[3];

	// storms
	sprintf(noise_expr2,"%g*max(EQU*noise(SIMPLEX|FS|SCALE|SQR|RO1,%g,3,1,0.5,2,0.5,1,0,0.3,1e-06),0)",storms,1+0.25*r[4]);
	double ripple=1+0.1*s[4];
     sprintf(noise_expr3,"%g*noise(%s|FS|SQR|TA|RO1,1,2.5,1,0.5,2,0.05,1,0,0,6.61114e-07)",ripple,noise_func.c_str());
    double scale=0.5+r[5];
    double ampl=1.5+0.5*r[6];
    double bump=-0.02;
    double offset=r[7];
    double levels=2+r[8];
    double delf=2.2+s[6];
    double dela=0.4+r[9];
    char  *tname=randFeature(RND_TEXNAME).c_str();
  
    sprintf(buff,"Texture(%s,S|TEX|BUMP,%g*PHI+%s+%s+%s,%g,%g,%g,%g,%g,%g,%g,0,0,0,0,0)",
    		tname,0.5+0.5*s[3],noise_expr1,noise_expr2,noise_expr3,
			scale,bump,ampl,offset,levels,delf,dela);

	TNtexture *tex=(TNtexture*)TheScene->parse_node(buff);
	if(tex){
	    planet->terrain.set_root(tex);
	    planet->terrain.init();
	 }
	 else
	    cout<<"error building planet texture"<<endl;

	Sky *sky=planet->newSky(GN_MED);
	planet->addChild(sky);

	bool has_rings=r[6]>0.8;
	if(has_rings){
		Ring *ring=planet->newRing();
		planet->addChild(ring);
	}
	int moons=r[7]*4;
	int max_moons=TheScene->generate_quality;
	moons=moons>max_moons?max_moons:moons;

	moon_cnt=0;
	for(int i=0;i<moons;i++){
		planet->addMoon(GN_RANDOM);		
	}
	Render.invalidate_textures();
	images.invalidate();
	images.makeImagelist();

}

//************************************************************
// Moon class
//************************************************************
Moon::Moon(Planet *m, double s, double r) : Planetoid(m,s,r)
{
#ifdef DEBUG_OBJS
	printf("Moon\n");
#endif
}
Moon::~Moon()
{
#ifdef DEBUG_OBJS
	printf("~Moon\n");
#endif
}

NodeIF *Moon::getInstance(NodeIF *prev,int gtype){
	lastn=getRandValue()*1234;
	setRands();
	setParent(prev->getParent());
	size=0.1*((Planetoid *)prev)->size;
	size*=(1.00+0.9*s[7]);
	newInstance(gtype);
	return this;
}

void Moon::newInstance(int gtype){
	cout<<"Moon::newInstance "<<UniverseModel::typeSymbol(gtype).c_str()<<endl;
	makeLists();
	//initInstance();
	setRseed(r[0]);
	newRocky(this,gtype);
	setProtoValid(true);
	setNewViewObj(true);
}

//-------------------------------------------------------------
// Spheroid::set_tilt() 	set axial tilt
//-------------------------------------------------------------
void Moon::set_tilt()
{
	//return Spheroid::set_tilt();
	//TODO: fix set_tilt for clouds
	if(tilt){
		// tilt and freeze pole orientation
		// axis always points in same direction globally
		// but rotation axis transfers from x to z during orbit
		Planetoid *planet=(Planetoid *)getParent();
		TheScene->rotate(-tilt,cos(planet->orbit_angle*RPD),0,sin(planet->orbit_angle*RPD));
	}
}
//************************************************************
// Shell class
//************************************************************
Shell::Shell(Orbital *m) : Spheroid(m,0.0,0.0)
{
#ifdef DEBUG_BASE_OBJS
	printf("Shell\n");
#endif
    if(m){
        size=m->size;
        symmetry=((Spheroid*)m)->symmetry;
    }
	set_geometry();
	select_exclude();
	shadows_exclude();
	set_cargs(0);
}
Shell::Shell(Orbital *m, double s) : Spheroid(m,s,0.0)
{
#ifdef DEBUG_BASE_OBJS
	printf("Shell\n");
#endif
	if(m){
		ht=s-m->size;
		symmetry=((Spheroid*)m)->symmetry;
	}
	set_geometry();
	select_exclude();
	shadows_exclude();
	set_cargs(1);
}
Shell::~Shell()
{
#ifdef DEBUG_BASE_OBJS
	printf("  ~Shell\n");
#endif
}

//-------------------------------------------------------------
// Shell::select() render names for the object and it's children
//-------------------------------------------------------------
void Shell::select()
{
    Orbital::select();
}

//-------------------------------------------------------------
// Shell::selection_pass() clear selection
//-------------------------------------------------------------
int Shell::selection_pass()
{
   return Orbital::selection_pass();
}

//-------------------------------------------------------------
// Shell::render_object()   object level render
//-------------------------------------------------------------
void Shell::render_object()
{
	if(Raster.auximage()) // skip surface fog/water effects
		return;
    
	if(TheScene->render_mode()){
	    set_lighting();
		Spheroid::render_object();
	}
}

//-------------------------------------------------------------
// Shell::adapt_object()   object level adapt
//-------------------------------------------------------------
void Shell::adapt_object()
{
	Spheroid::adapt_object();
}

//-------------------------------------------------------------
// Shell::set_geometry() set map properties
//-------------------------------------------------------------
void Shell::set_geometry()
{
	Spheroid::set_geometry();
	Object3D *parent=(Object3D*)getParent();
	if(parent){
		double dht=size-parent->size;
		if(dht!=ht){
			size=parent->size+ht;
		}
		symmetry=((Spheroid*)parent)->symmetry;
		map->symmetry=symmetry;
		map->radius=size;
	}
	map->frontface=GL_BACK;
	map->lighting=0;
	map->set_transparant(1);
}

NodeIF *Shell::replaceNode(NodeIF *n){
	NodeIF *p=getParent();
	Shell *shell=(Shell *)n;
	shell->size=size;
	shell->ht=ht;
    shell->setParent(p);
    shell->set_geometry();
	p->replaceChild(this,n);
	return n;
}
//-------------------------------------------------------------
// Shell::scale() 	set znear, zfar
//-------------------------------------------------------------
int  Shell::scale(double &zn, double &zf)
{
    int t=Object3D::scale(zn,zf);

    // calculate distance to sky, tangent to horizon
    // TODO:need to calc max_height from parent's terrain z min/max

	double s=TheScene->epoint.distance(point);
	if(parent){
		double a=0,b=0,r,h;
		h=parent->max_height();
		r=(1-h)*parent->size;
		if(s>r)
			a=sqrt(s*s-r*r);
		if(size>r)
			b=sqrt(size*size-r*r);
		zf=a+b+size;
		double zmax=s+size;
		zf=zf>zmax?zmax:zf;
	}
	//zf*=2;
	map->dmin=zn;
	map->dmax=zf;
	setvis(t);
	return t;
}
//-------------------------------------------------------------
// Shell::dpvars() calculate gradient variables
// - produces alpha gradient from horizon (a=1) to top of shell (a=0)
//-------------------------------------------------------------
void Shell::dpvars(double &dht, double &dpmin, double &dpmax){
	double r=parent->size+TheScene->gndlvl;
	double d=point.length()+1*MILES;
	double s=size; // radius of shell
	double h=s-r;
	double theta=asin(r/d);
	double l1=sqrt(d*d-r*r);
	double l2=sqrt(s*s-r*r);
	double L=l1+l2;
	double D=L*cos(theta);
	double x=D-d;   // may be negative because l2 length constrained to r
	double dp=x/s;  // cos(phi)
	double ht=TheScene->height;
	double phi=asin(h/l1);
	double cmax=cos(theta+phi); // cos @dpmax

	dht=1;

	if(TheScene->viewobj==parent)
		dht=ht/h;
    //cout<<name() <<" out:"<<l1/d<<" "<<cmax<<" in:"<<h/(h+r)<<" "<< dp<<endl;
    if(TheScene->backside()||inside()){
	//if(inside()){
	  	dpmax=dp;
	  	dpmin=h/(h+r);
	  	//cout <<name()<< " inside dpmin:"<<dpmin<<" dpmax:"<<dpmax<<endl;
	}
	else{
	  	dpmax=sqrt(d*d-s*s)/d;
	  	dpmin=l1/d;
	  	//dpmax=cmax;
	}
}


//************************************************************
// Sky class
//************************************************************
Sky::Sky(Orbital *m, double s) : Shell(m,s)
{
#ifdef DEBUG_OBJS
	printf("Sky\n");
#endif
	set_geometry();
	year=day=0;
	twilite_max=1;
	twilite_min=-0.5;
	haze_color=def_haze_color;
	density=0.05;
	detail=8;
}
Sky::~Sky()
{
#ifdef DEBUG_OBJS
	printf("~Sky\n");
#endif
}
double Sky::getHeatCalacity() {
	double g = pow(pressure, 1.5) * ghg_fraction;
	return  0.4 * pow(g, 0.25);
}

//-------------------------------------------------------------
// Sky::getInstance()  generate a random instance
//-------------------------------------------------------------
bool Sky::randomize(){	
	setRseed(getRandValue());
	setRands();
	Color hsv=Color(0.5+0.3*s[2]+0.2*s[3],0.5,0.9);
	Color c=hsv.HSVtoRGB();
	set_color(c);
	twilite_color=c.darken(0.8);
	twilite_color=twilite_color.blend(Color(1,0,0),0.3);
	Color sc=haze_color;
	haze_color=haze_color.mix(c,0.5);

	invalidate();
	TheScene->set_changed_detail();
	return true;
}

NodeIF *Sky::getInstance(NodeIF *prev,int gtype){
	cout<<"Sky::getInstance"<<endl;
	Sky *sky=newInstance(gtype);	
	sky->setParent(prev->getParent());
	return sky;
}

//-------------------------------------------------------------
// Sky::newInstance()  generate a random instance
//-------------------------------------------------------------
Sky *Sky::newInstance(int gtype){
	Sky *sky=TheScene->getPrototype(0,ID_SKY);
	sky->randomize();
	sky->detail=8;
	double f=pow(r[3],3);
	sky->ghg_fraction=r[4]*r[3]*r[5];
	switch(gtype){
	case GN_THIN:
		sky->pressure=lerp(f,0,1,0.01,0.5);
		sky->density=lerp(f,0,1,0.02,0.1);
		sky->haze_grad=lerp(f,0,1,0.8,1);
		sky->_color.set_alpha(lerp(f,0,1,0.7,1));
		sky->haze_color.set_alpha(lerp(f,0,1,0.1,0.5));
		break;
	case GN_MED:
		sky->pressure=lerp(f,0,1,0.5,2);
		sky->density=lerp(f,0,1,0.1,0.5);
		sky->haze_grad=lerp(f,0,1,0.5,0.2);
		sky->haze_color.set_alpha(lerp(f,0,1,0.5,0.8));
		break;
	case GN_DENSE:
		sky->pressure=lerp(f,0,1,2,10);
		sky->density=lerp(f,0,1,0.5,2);
		sky->haze_grad=lerp(f,0,1,0.1,0.01);
		break;
	}
	sky->set_vars();
	return sky;
}

//-------------------------------------------------------------
// Sky::scale() 	set znear, zfar
//-------------------------------------------------------------
int  Sky::scale(double &zn, double &zf)
{
    int t=Shell::scale(zn,zf);
    if(/*inside() && */parent && parent==TheScene->viewobj){
    	double r=parent->size+TheScene->gndlvl;
    	double d=point.length()+1*MILES;
      	double l1=1e-4;

      	if(d>r)
      		l1=sqrt(d*d-r*r);

    	haze_zfar=l1; // rough estimate of distance to horizon

    	double theta=asin(r/size);

    	twilite_dph=-cos(theta);
    	//cout<<twilite_dph<<" "<<twilite_min<<" "<<twilite_max<<endl;

    	Raster.haze_zfar=haze_zfar;
    	Raster.twilite_dph=twilite_dph;
    	Raster.twilite_max=twilite_max;
    	Raster.twilite_min=twilite_min;
    	
    	Raster.setParams();

    	Point p=point.mm(TheScene->invViewMatrix);
    	p=p.mm(TheScene->viewMatrix);

    	Raster.center=p;

    }
	return t;
}

//-------------------------------------------------------------
// Sky::adapt_pass() select for scene pass
//-------------------------------------------------------------
int Sky::adapt_pass()
{
    clr_selected();

	if(!local_group() || !view_group() || offscreen()|| !isEnabled())
        clear_pass(BG2);
    else
		clear_pass(FG1);

    return selected();
}

//-------------------------------------------------------------
// Sky::render_pass() select for scene pass
//-------------------------------------------------------------
int Sky::render_pass()
{
    clr_selected();

	if(!view_group() || offscreen() || !isEnabled())
	    return 0;
	if(getParent()==TheScene->viewobj){
		if(near_group()){
			if(outside())
				clear_pass(FG1);
			else{
				// hides stars in transparent bands of rings
				Orbital *parent=getParent();
				if (parent->type()==ID_MOON){
					if(parent->getParent()->hasChild(ID_RING))
						clear_pass(BG3);
				}
				clear_pass(BG1);
			}
		}
		else
			clear_pass(BG2);
	}
	else
		clear_pass(BG2);
    return selected();
}

//-------------------------------------------------------------
// Sky::get_vars()    reserve interactive variables
//-------------------------------------------------------------
void Sky::get_vars()
{

	Spheroid::get_vars();
	twilite_color=def_twilite_color;
	VGET("twilight.value",twilite_value,def_twilite_value);
	if(exprs.get_local("twilight",Td)){
		twilite_color=Td.c;
		twilite_value=twilite_color.alpha();
		//twilite_color.set_alpha(1);
	}
	VGET("twilight.min",twilite_min,def_twilite_min);
	VGET("twilight.max",twilite_max,def_twilite_max);
	VGET("resolution",detail,2);
	VGET("density",density,0.01);
	VGET("pressure",pressure,1); // earth normal 1ATM
	VGET("ghg_fraction",ghg_fraction,0.01); //  earth: mostly h2o (20%) co2 (0.03%)

	VGET("haze.value",haze_value,def_haze_value);
	if(exprs.get_local("haze.color",Td)){
		haze_color=Td.c;
		//haze_value=haze_color.alpha();
		//haze_color.set_alpha(1.0);
	}
	VGET("haze.min",haze_min,def_haze_min);
	VGET("haze.max",haze_max,def_haze_max);
	VGET("haze.height",haze_height,def_haze_height);
	VGET("haze.grad",haze_grad,1.0);
	VGET("sky.grad",sky_grad,1.0);

	if(exprs.get_local("night.color",Td))
		night_color=Td.c;
	else
		night_color=Color(0,0,0,0);
	if(exprs.get_local("halo.color",Td))
		halo_color=Td.c;
	else
		halo_color=Color(1,1,1,1);
	specular=Color(1,1,1,1);
}

//-------------------------------------------------------------
// Sky::set_vars()    set common variables
//-------------------------------------------------------------
void Sky::set_vars()
{
	Spheroid::set_vars();
	CSET("night.color",night_color,Color(0,0,0,0));
	CSET("halo.color",halo_color,Color(1,1,1,1));
	VSET("resolution",detail,2);
	VSET("density",density,0.01);
	VSET("pressure",pressure,1);
	VSET("ghg_fraction",ghg_fraction,0.01);
	CSET("twilight",twilite_color,def_twilite_color);
	VSET("twilight.value",twilite_value,def_twilite_value);
	VSET("twilight.min",twilite_min,def_twilite_min);
	VSET("twilight.max",twilite_max,def_twilite_max);
	CSET("haze.color",haze_color,def_haze_color);
	VSET("haze.value",haze_value,def_haze_value);
	USET("haze.min",haze_min,def_haze_min,"miles");
	USET("haze.max",haze_max,def_haze_max,"miles");
	VSET("haze.height",haze_height,def_haze_height);
	VSET("haze.grad",haze_grad,1.0);
	VSET("sky.grad",sky_grad,1.0);
}

//-------------------------------------------------------------
// Sky::set_ref()   set orientation
//-------------------------------------------------------------
void Sky::set_ref()
{
	if(parent){
		rot_angle=((Orbital*)parent)->rot_angle;
		orbit_angle=((Orbital*)parent)->orbit_angle;
		tilt=((Orbital*)parent)->tilt;
		day=((Orbital*)parent)->day;
		year=((Orbital*)parent)->year;
		//cout<<"Sky::set_ref "<<rot_angle<<endl;
	}
	//animate();
}


//-------------------------------------------------------------
// Sky::adapt_object()   object level adapt
//-------------------------------------------------------------
void Sky::adapt_object()
{
	if(!isEnabled())
		return;

	if(inside())
		map->frontface=GL_BACK;
	else
		map->frontface=GL_FRONT_AND_BACK;
		//map->frontface=GL_FRONT;
	exprs.eval();
	map->make_current();

	terrain.init();
	terrain.set_eval_mode(0);
	set_wscale();

	map->adapt();

}

//-------------------------------------------------------------
// Sky::init_render()  render setup (called only for viewobj)
//-------------------------------------------------------------
void Sky::init_render()
{
	if(parent && parent==TheScene->viewobj){

		Raster.haze_grad=haze_grad;
		Point lp=Lights[0]->point.mm(TheScene->invViewMatrix);
		lp=lp.normalize();
		Point vp=TheScene->vpoint;
		double dp=lp.dot(vp)/vp.length();
		double a=rampstep(-0.4,-0.6,dp,1,0);

		double ht=TheScene->height;
		double depth=size-parent->size;
		double dht=ht/depth;

		// shader sets Raster.sky_alpha (used to whiten moons in daytime)
		if(inside())
			Raster.sky_alpha = (1-dht)*rampstep(twilite_min-0.3,twilite_min,dp,0,1.0);
		else
			Raster.sky_alpha=0;

		// legacy ogl sky transparancy algorithm sets sky_color.alpha
		a*=smoothstep(0.2,1,dht,1.1,0);

		double f=rampstep(twilite_max,twilite_min,dp,0,twilite_value);
		Raster.blend_factor=f;
		Raster.darken_factor=twilite_value*f;

		Color c=color();
	    Raster.sky_color=c;
	    if(Render.twilite()){
	        Raster.sky_color.set_alpha(a*Raster.sky_color.alpha());
	        Raster.blend_color=twilite_color;
	        //Raster.blend_color.set_alpha(f*twilite_color.alpha());
	    }
	    else
	        Raster.sky_color.set_alpha(1);
		//if(haze_value){
		double df=sqrt(Raster.darken_factor);
		c=haze_color;
		c=c.darken(df);
		//Raster.modulate(c);
		if(Raster.haze()){
			Raster.haze_min=haze_min;
			Raster.haze_max=haze_max;
			Raster.haze_value=haze_color.alpha();
			//Raster.haze_value=haze_value;
			Raster.haze_color=c;
			}
		//}
	}
	else{
		Raster.sky_alpha=0;
		Raster.sky_color.set_alpha(0);
		Raster.blend_color.set_alpha(0);
	}
}

//-------------------------------------------------------------
// Sky::render_object()   lower level render
//-------------------------------------------------------------
void Sky::render_object()
{
	if(!TheScene->render_mode() || !isEnabled())
		return;
	if(Raster.auximage()) // skip surface fog/water effects
		return;
	show_render_state(this);

	glDepthMask(GL_FALSE);
	if(TheScene->backside() || inside()){
		map->frontface=GL_BACK;
		Shell::render_object();
	}
	else if(outside()){
		map->frontface=GL_FRONT;
		Shell::render_object();
	}
	glDepthMask(GL_TRUE);
}

//-------------------------------------------------------------
// Sky::setProgram() set shader program
//-------------------------------------------------------------
bool Sky::setProgram(){
	char defs[128]="";

	Orbital *parent=getParent();
	if(parent->type()==ID_STAR)
		sprintf(defs,"#define NLIGHTS 0\n");
	else
		sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),Lights.size);
	if(!inside() && TheScene->backside())
		sprintf(defs+strlen(defs),"#define BACK\n");
	if((TheScene->backside()||inside())&&Render.haze())
		sprintf(defs+strlen(defs),"#define HAZE\n");
	GLSLMgr::setDefString(defs);
	if(TheScene->backside()||inside()){
		if(inside()){
			GLSLMgr::pass=0;
			GLSLMgr::setFBOWritePass();
		}
		GLSLMgr::loadProgram("sky.vert","sky.back.frag");
	}
	else{
		GLSLMgr::loadProgram("sky.vert","sky.front.frag");
	}

	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;

	double dht,dpmin,dpmax;
	dpvars(dht,dpmin,dpmax);

	//cout <<"sky dpmin:"<<dpmin<<" dpmax:"<<dpmax<<endl;

	GLSLVarMgr vars;
	Color c = haze_color;
	vars.newFloatVec("Haze",c.red(),c.green(),c.blue(),c.alpha());
	c = color();
	vars.newFloatVec("Sky",c.red(),c.green(),c.blue(),c.alpha());
	c = halo_color;
	vars.newFloatVec("Halo",c.red(),c.green(),c.blue(),c.alpha());
	c = twilite_color;
	//c.print();
	vars.newFloatVec("Twilite",c.red(),c.green(),c.blue(),c.alpha());
	c = night_color;
	vars.newFloatVec("Night",c.red(),c.green(),c.blue(),c.alpha());

	vars.newBoolVar("lighting",Render.lighting());
	Point p=point.mm(TheScene->invViewMatrix);
	p=p.mm(TheScene->viewMatrix);

	Raster.center=p;

	vars.newFloatVec("center",p.x,p.y,p.z);
	vars.newFloatVar("sky_grad",sky_grad);
	vars.newFloatVar("haze_grad",haze_grad);
	vars.newFloatVar("haze_zfar",haze_zfar);
	vars.newFloatVar("day_grad",shine);
	vars.newFloatVar("density",density);
	vars.newFloatVar("dht",dht);
	vars.newFloatVar("dpmin",dpmin);
	vars.newFloatVar("dpmax",dpmax);

//	double r=parent->size;
//	double s=size; // radius of shell
//	double theta=asin(r/s);
//
//	double twilite_dph=-cos(theta);

	vars.newFloatVar("twilite_dph",twilite_dph);
	vars.newFloatVar("twilite_max",twilite_max);
	vars.newFloatVar("twilite_min",0.5*twilite_min);
	//cout << twilite_min << " "<< twilite_max << endl;

	Point pv=TheScene->xpoint;
	vars.newFloatVec("pv",pv.x,pv.y,pv.z);

	glEnable(GL_BLEND);
 	vars.setProgram(program);
	vars.loadVars();

	GLSLMgr::loadVars();
	return true;
}

//-------------------------------------------------------------
// Sky::map_color()   modulate render color
//-------------------------------------------------------------
void Sky::map_color(MapData*d,Color &c)
{
	if(Render.draw_shaded()){
		c=WHITE;
		c.set_alpha(1.0);
		return;
	}
	double dp1,dp2,f;
	Point p=d->mpoint();
	double ds=p.length();
	static Point lp;
	static Point lv;
	static Point pv;
	static Point vp;
	static double amax;
	static double dmin;
	static double dhaze;
	static double dmax=size;
	static double dht=0;
	static double depth=0;
	static double ht=0;
    static double a,b,r,s;

	if(first){

 		lp=Lights[0]->point.mm(TheScene->invViewMatrix);
		vp=TheScene->vpoint;
		lv=lp-vp;
		lv=lv.normalize();
		lp=lp.normalize();
		vp=vp.normalize();
		amax=color().alpha();
		//dmax=size;

		// calculate distance to sky, tangent to horizon

		r=parent->size;
		s=TheScene->height+r;
		a=sqrt(s*s-r*r);
		b=sqrt(size*size-r*r);
		dmax=a+b;
		ht=TheScene->height;
		depth=size-r;
		dht=ht/depth;
		first=0;
	}

	if(!terrain.get_root())
		c=color();

	if(map->frontface==GL_BACK){
		pv=(p-TheScene->vpoint);
		double dist=pv.length();

		if(TheScene->bgpass==BG1){
			// add ground haze

			//dhaze=0.9*rampstep(0,1,haze_height,dht*dmax,dmax);
			//dhaze=a+b*rampstep(0,1,haze_height,dht,1);
			dhaze=a+b*0.8*haze_height;
			f=rampstep(dhaze,dmax,dist,0,1);

			c=haze_color;
			//double a1=c.alpha();
			//c=c.blend(haze_color,f*haze_value);

			// add night transparancy

			dp1=lp.dot(p)/ds;
			double a1=rampstep(-0.4,-1,dp1,1,0.1);

			c.set_alpha(haze_value*f*a1);
		}
		else{
			// add ground sky tint

			dhaze=a+0.9*b*rampstep(0,1,haze_height,dht,1);
			f=rampstep(0,dhaze,dist,0,1);
			c=c.darken(twilite_value*0.5*(1-density)*(1-f));
			f=rampstep(dhaze,dmax,dist,0,1);
			c=c.blend(haze_color,f*haze_value);

			// increase transparency with ht

			//dmin=rampstep(0,1,dht,0,a+0.5*b);
			dmin=rampstep(0,1,dht,a,a+0.5*b);
			f=rampstep(0.2,1,dht,1,0);
			double df=rampstep(dmax,dmin,dist,1,f);

			// add twilight color

			dp1=lp.dot(p)/ds;
			dp2=lv.dot(pv)/dist;
			f=rampstep(twilite_max,twilite_min,dp1,0,twilite_value);
			f*=rampstep(twilite_min,-1,dp1,1,0);
			if(Render.twilite())
				c=c.blend(Raster.blend_color,f*rampstep(0,1,dp2,1,0));

			// decrease transparency towards star

			double dps=dp2*fabs(dp2);
			//double dps=fabs(dp2);
			double a2=rampstep(0.5,1,dps,0,1);
			//double a2=rampstep(0.7,1,dps,0,1);

			// add night transparancy

			double a1=rampstep(-0.4,-0.6,dp1,1,0);

			double alpha=a1>a2?a1:a2;
			c=c.blend(Lights[0]->color,a2);
			c.set_alpha(alpha*amax*df);
		}
	}
	else{
		dp1=vp.dot(p)/ds;
		dp2=lv.dot(p)/ds;
		//double da=rampstep(1,1.1,size/parent->size,-0.2,0.4);
		double da=rampstep(1,1.01,size/parent->size,-0.2,0.4);
		double f1=rampstep(0.8,da,dp1,1,0.0);
		double f2=rampstep(0.2,-0.3,dp2,1,0.3);
		f=rampstep(0,0.8*f2,dp1*dp1,1,density);
		//f=f2*rampstep(0.0,0.8,dp1*dp1,1,density);
		c.set_alpha(c.alpha()*f*f1*f2);
	}
}

//************************************************************
// CloudLayer class
//************************************************************
#define TOP    1
#define BOTTOM 2

#define ROT_IN_SHADER

#define CLOUD_MAX 15.1
#define CLOUD_ROWS 4

#define CLOUDS_BILLBOARDS  0
#define CLOUDS_POINTS      1
#define CLOUDS_GS_SHADER   2 // use geometry shader for 3d clouds
#define CLOUDS_NO_SHADER   3

int clouds_mode=CLOUDS_GS_SHADER;
//int clouds_mode=CLOUDS_POINTS;

char *CloudLayer::dflt_sprites_file="CloudSprites";
CloudLayer::CloudLayer(Orbital *m, double s) : Shell(m,s)
{
#ifdef DEBUG_OBJS
	printf("CloudLayer\n");
#endif
	map->frontface=GL_BACK;
	map->gmax=0.2;
	hscale=0;
	//hscale=5e-4;
	clouds=0;
	cmin=16;
	cmax=4;
	smax=2;
	crot=1;
	sprites_dim=4;
	sprites_file[0]=0;
	sprites_image=0;
	emission=Color(1,1,1,0.05);
}
CloudLayer::~CloudLayer()
{
#ifdef DEBUG_OBJS
	printf("~CloudLayer\n");
#endif
	if(sprites_image>0)
		glDeleteTextures(1,&sprites_image);
}

//-------------------------------------------------------------
// CloudLayer::adapt_pass() select for scene pass
//-------------------------------------------------------------
int CloudLayer::adapt_pass()
{
    clr_selected();

	if(!local_group() || !view_group() || offscreen()|| !isEnabled())
        clear_pass(BG2);
    else if(threeD())
		clear_pass(FG1);
		//clear_pass(FG0);
    else
    	clear_pass(BG0);

    return selected();
}

//-------------------------------------------------------------
// CloudLayer::render_pass() select for scene pass
//-------------------------------------------------------------
int CloudLayer::render_pass()
{
    clr_selected();

	if(!view_group() || offscreen()|| !isEnabled())
	    return 0;
	if(getParent()==TheScene->viewobj){
		if(near_group()){
			if(outside())
				clear_pass(FG1);
			else
				clear_pass(BG1);
		}
		else
			clear_pass(BG2);
	}
	else
		clear_pass(BG2);

    return selected();
}

//-------------------------------------------------------------
// CloudLayer::shadow_pass() select for shadow pass
//-------------------------------------------------------------
int CloudLayer::shadow_pass()
{
	return Shell::shadow_pass();
}

//-------------------------------------------------------------
// CloudLayer::scale() 	set znear, zfar
//-------------------------------------------------------------
//#define DEBUG_CLOUDS

int CloudLayer::scale(double &zn, double &zf)
{
	int t = Shell::scale(zn, zf);
#ifdef DEBUG_CLOUDS
	cout <<"zn:"<< zn/MILES << " zf:"<< zf/MILES << " Rz:"<< zf/zn << endl;
#endif
	if (!TheScene->render_mode())
		return t;

	if (map->triangles.size > 10 && threeD() && Render.draw_shaded()) {
		zn = map->triangles[0]->distance;
		zf = map->triangles[map->triangles.size - 1]->distance;
#ifdef DEBUG_CLOUDS
		cout << "tn:"<< zn/MILES << " tf:"<< zf/MILES << " Rt:"<< zf/zn<< endl;
#endif
		map->dmin = zn;
		map->dmax = zf;
	}
	return t;
}

//-------------------------------------------------------------
// CloudLayer::get_vars()    reserve interactive variables
//-------------------------------------------------------------
void CloudLayer::get_vars()
{
 	Spheroid::get_vars();
	VGET("cmin",cmin,16);
	VGET("cmax",cmax,4);
	VGET("smax",smax,3);
	VGET("crot",crot,1);
	VGET("num_sprites",num_sprites,1);
	VGET("diffusion",diffusion,0.25);
	VGET("sprites_dim",sprites_dim,4);

	if(exprs.get_local("sprites_file",Td))
		strncpy(sprites_file,Td.string,256);
	else
		strncpy(sprites_file,dflt_sprites_file,256);
}

//-------------------------------------------------------------
// CloudLayer::set_vars()    set common variables
//-------------------------------------------------------------
void CloudLayer::set_vars()
{
	Spheroid::set_vars();
	VSET("cmin",cmin,16);
	VSET("cmax",cmax,4);
	VSET("smax",smax,3);
	VSET("crot",crot,0.5);
	VSET("num_sprites",num_sprites,1);
	VSET("diffusion",diffusion,0.25);
	VSET("sprites_dim",sprites_dim,4);

	exprs.set_var("sprites_file",sprites_file,strcmp(sprites_file,dflt_sprites_file));
}

void CloudLayer::setSpritesFile(char *s,GLuint dim)
{
	if(strcmp(s,sprites_file)!=0 || (dim!=sprites_dim)){
		if(sprites_image>0)
			glDeleteTextures(1,&sprites_image);

		sprites_image=0;
		strcpy(sprites_file,s);
		sprites_dim=dim;
	}
}
char *CloudLayer::getSpritesFile(GLuint &dim){
	dim=sprites_dim;
	return sprites_file;
}

void CloudLayer::getSpritesDir(int dim,char*dir){
	char base[256];
	char dimdir[32];
  	File.getBaseDirectory(base);
  	switch(dim){
  	case 1: strcpy(dimdir,"1x"); break;
  	case 2: strcpy(dimdir,"4x"); break;
  	case 3: strcpy(dimdir,"9x"); break;
  	default:
  	case 4: strcpy(dimdir,"16x"); break;
  	}
 	sprintf(dir,"%s/Resources/Sprites/Clouds/%s",base,dimdir);
}

void CloudLayer::getSpritesFilePath(char *dir){
	char base[256];
	char dimdir[32];
  	File.getBaseDirectory(base);
  	switch(sprites_dim){
  	case 1: strcpy(dimdir,"1x"); break;
  	case 2: strcpy(dimdir,"4x"); break;
  	case 3: strcpy(dimdir,"9x"); break;
  	default:
  	case 4: strcpy(dimdir,"16x"); break;
  	}
 	sprintf(dir,"%s/Resources/Sprites/Clouds/%s/%s",base,dimdir,sprites_file);
}

//-------------------------------------------------------------
// CloudLayer::setSpritesTexture() set point sprite image texture
// Generates a texture image from a bitmap file or compiled data
//-------------------------------------------------------------
void CloudLayer::setSpritesTexture(){
	if(sprites_image>0)
		return;

	char dir[256];
	getSpritesFilePath(dir);

	cout<<"building cloud sprites texture :"<<dir<<" "<<sprites_dim<<endl;

	Image *image = images.open(sprites_file,dir);
	if (!image){
		cout<<"could not open image file:"<<sprites_file<<endl;
		return;
	}
	int height = image->height;
	int width = image->width;
	unsigned char *pixels = (unsigned char*) malloc(height * width);
	unsigned char* rgb = (unsigned char*) image->data;
	for (int i = 0,index=0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int k=(height-i-1)*width+j;
			double a = rgb[k * 3] / 255.0;
			pixels[index]=(unsigned char) (a * 255.0);
			index++;
		}
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glGenTextures(1, &sprites_image); // Generate a unique texture ID
	glBindTexture(GL_TEXTURE_2D, sprites_image);
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -1);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width,
			height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
}

//-------------------------------------------------------------
// CloudLayer::set_ref()   set orientation
//-------------------------------------------------------------
void CloudLayer::set_ref()
{
	animate();
}
void CloudLayer::set_tilt()
{
	//Orbital::set_tilt();
	// use planet tilt parent reference
	((Orbital*)parent)->set_tilt();
}

//-------------------------------------------------------------
// CloudLayer::animate() adjust rotation with time
//-------------------------------------------------------------
void CloudLayer::animate()
{
	double cycles=0,secs=0;
	double yr=((Spheroid*)parent)->year;
	if(yr){
		secs=24*3600*yr; // earth normal days
		cycles=TheScene->ftime/secs;
		orbit_angle=P360(orbit_phase+360*cycles);
	}
	double cd=((Spheroid*)parent)->day-day;
	if(cd){
		 // 360 degrees per 3600 sec/hr per day (hrs)
		secs=cd*3600;
		cycles-=TheScene->ftime/secs;
		rot_angle=P360(rot_phase+360*cycles);
	}
}


//-------------------------------------------------------------
// CloudLayer::render() render the object and it's children
//-------------------------------------------------------------
void CloudLayer::render()
{
	if(!isEnabled())
		return;
	if(!TheScene->render_mode())
		return;
	if(Raster.auximage()) {// skip surface fog/water effects
		return;
	}
	if(included()){
		show_render_state(this);
		map->lighting=1;
		glDepthMask(GL_FALSE);

		TheScene->pushMatrix();
		set_ref();
		set_tilt();
		set_rotation();

		setMatrix();
		bool v3d=threeD()&& Render.draw_shaded();
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

		if(v3d ){
			Raster.surface=1;
		    Raster.set_all();
		    map->make_current();
		    switch(clouds_mode){
		    case CLOUDS_BILLBOARDS:
		    case CLOUDS_GS_SHADER:
		    	glEnable(GL_POINT_SPRITE);
				setSpritesTexture();
				break;
		    case CLOUDS_POINTS:
				setSpritesTexture();
		    	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
				break;
		    case CLOUDS_NO_SHADER:
		    	glDisable(GL_POINT_SPRITE);
				break;
		    }
		}
		else{
			if(threeD())
				map->frontface=GL_FRONT_AND_BACK;
			//else if(TheScene->backside() || inside())
			else if(inside())
				map->frontface=GL_BACK;
			else //if(outside())
				map->frontface=GL_FRONT;
		}
		Shell::render_object();
		map->frontface=GL_FRONT;
		TheScene->popMatrix();
		glDepthMask(GL_TRUE);
    	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	}
}

//-------------------------------------------------------------
// CloudLayer::setProgram() set shader program
//-------------------------------------------------------------
bool CloudLayer::setProgram(){
	bool v3d=threeD();
	if(v3d && map->tid!=TOP)
		return false;
	bool texs_enabled=Render.textures();

	if(Raster.lines()){
		texs_enabled=false;
		glPolygonMode(GL_FRONT,GL_LINE);
		glPolygonMode(GL_BACK,GL_LINE);
	}
	else{
		glPolygonMode(GL_FRONT,GL_FILL);
		glPolygonMode(GL_BACK,GL_FILL);
	}

	char defs[512]="";
	sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),Lights.size);
	if(inside())
		sprintf(defs+strlen(defs),"#define BACK\n");

	// FIXME: 3d noise textures seem to be broken (7/5/12)
	//if(!v3d)
	//	sprintf(defs+strlen(defs),"#define N3D\n");

	Sky *sky=((Spheroid*)parent)->get_sky();
	Color haze(1,1,1);
	Color halo(1,1,1);
	Color twilight(0,0,0);

	double twilite_min=-0.2;
	double twilite_max=0.2;
	double twilite_dph=0;
	double haze_grad=1;
	double haze_zfar=0;
	double day_grad=2;

	if(sky){
		twilite_min=sky->twilite_min;
		twilite_max=sky->twilite_max;
		twilite_dph=sky->twilite_dph;
		halo=sky->halo_color;
		twilight=sky->twilite_color;
		if(Render.haze() && parent == TheScene->viewobj){
			haze=sky->haze_color;
			haze_grad = sky->haze_grad;
			haze_zfar = sky->haze_zfar;
			day_grad=sky->shine;
			if(sky->inside())
				sprintf(defs+strlen(defs),"#define HAZE\n");
		}
	}

	GLSLVarMgr vars;
	GLSLMgr::setDefString(defs);
	TerrainProperties *tp=map->tp;

	tp->initProgram();
	if(Td.clouds)
		Td.clouds->initProgram();

	//GLSLMgr::init3DNoiseTexture();

	double gndlvl=parent->size+TheScene->gndlvl;
	float sky_ht=sky?sky->size-gndlvl:size-gndlvl;

	vars.newBoolVar("lighting",Render.lighting());
	vars.newBoolVar("textures",texs_enabled);
	vars.newFloatVar("haze_zfar",haze_zfar);
	vars.newFloatVar("haze_grad",haze_grad);
	vars.newFloatVar("day_grad",day_grad);
	vars.newFloatVec("Haze",haze.red(),haze.green(),haze.blue(),haze.alpha());
	vars.newFloatVec("Halo",halo.red(),halo.green(),halo.blue(),halo.alpha());
	vars.newFloatVec("Twilight",twilight.red(),twilight.green(),twilight.blue(),twilight.alpha());


	vars.newFloatVar("sky_ht",sky_ht);
	vars.newFloatVar("eye_ht",TheScene->height);
	vars.newFloatVar("twilite_min",twilite_min);
	vars.newFloatVar("twilite_max",twilite_max);
	vars.newFloatVar("twilite_dph",twilite_dph);

	Point p=point.mm(TheScene->invViewMatrix);
	Point center=p.mm(TheScene->viewMatrix);
	vars.newFloatVec("center",center.x,center.y,center.z);
	vars.newFloatVec("Diffuse",diffuse.red(),diffuse.green(),diffuse.blue(),diffuse.alpha());
	vars.newFloatVar("ROWS",sprites_dim);
	vars.newFloatVar("INVROWS",1.0/sprites_dim);
	vars.newFloatVar("rseed",rseed);

	//UniformBool *backfacing=vars.newBoolVar("backfacing",0);
	Point cp=TheScene->vpoint;

	double dht,dpmin,dpmax;
	dpvars(dht,dpmin,dpmax);

	vars.newFloatVar("dpmin",dpmin);
	vars.newFloatVar("dpmax",dpmax);
	vars.newFloatVar("ldp",Raster.ldp);
	
	//cout<<"CloudLayer::setProgram "<<Raster.ldp<<endl;

	//cout <<"clouds dpmin:"<<dpmin<<" dpmax:"<<dpmax<<endl;

	set_wscale();
	if(v3d){
		switch(clouds_mode){
		case CLOUDS_BILLBOARDS:
			glBindTexture(GL_TEXTURE_2D, sprites_image);
			vars.newIntVar("sprites",0);
			GLSLMgr::loadProgram("clouds.bb.vert","clouds.3d.frag");
			break;
		case CLOUDS_POINTS:
			glBindTexture(GL_TEXTURE_2D, sprites_image);
			vars.newIntVar("sprites",0);
			GLSLMgr::loadProgram("clouds.ps.vert","clouds.ps.frag");
			break;
		case CLOUDS_GS_SHADER:
			glBindTexture(GL_TEXTURE_2D, sprites_image);
			vars.newIntVar("sprites",0);
			GLSLMgr::input_type=GL_POINTS;
			GLSLMgr::output_type=GL_TRIANGLE_STRIP;
			GLSLMgr::tesslevel=0;
			//GLSLMgr::max_output=4;  // special case
			GLSLMgr::loadProgram("clouds.gs.vert","clouds.3d.frag","clouds.geom");
			break;
		case CLOUDS_NO_SHADER:
			GLSLMgr::loadProgram("clouds.test.vert","clouds.test.frag");
			break;
		}
	}
	else
		GLSLMgr::loadProgram("clouds.2d.vert","clouds.2d.frag");

	GLhandleARB program=GLSLMgr::programHandle();

	if(!program)
		return false;

    tp->setProgram();
    if(Td.clouds)
    	Td.clouds->setProgram();

 	vars.setProgram(program);

	vars.loadVars();
	GLSLMgr::setFBOWritePass();
	TheScene->setProgram();

	double resolution= sqrt(map->resolution/16);

	if(v3d){
		glEnable(GL_BLEND);
		map->frontface=GL_BACK;
		glDisable(GL_CULL_FACE);
		float pts=cmin;// max=64;
		int n=map->triangles.size;
		double ext=TheScene->wscale*size/cp.length();
		double dht=TheScene->height/size;
		double df=lerp(dht,0.0,1,0.5,0.1);
		//double dht=(TheScene->height-size)/size;
		double maxext= df*ext;
		if(map->callLists[SHADER_LISTS][1]==0){
			map->callLists[SHADER_LISTS][1]=glGenLists(1);
			glNewList(map->callLists[SHADER_LISTS][1], GL_COMPILE);
			//cout << "new list 3d clouds  ["<<SHADER_LISTS<<"]["<<1<<"] id:"<<map->callLists[SHADER_LISTS][1]<<endl;
			switch(clouds_mode){
			case CLOUDS_BILLBOARDS:
				break;
			case CLOUDS_POINTS:
				glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
				glBegin(GL_POINTS);
				break;
			case CLOUDS_GS_SHADER:
				glBegin(GL_POINTS);
				break;
			case CLOUDS_NO_SHADER:
				glBegin(GL_TRIANGLES);
				break;
			}

			int tcnt1=0;
			int tcnt2=0;
			Point ec=center;
			ec=ec.normalize();

			for(int i=0;i<n;i++){
				Triangle *triangle=map->triangles[n-i-1]; // back to front render order
				if(!triangle->visible || triangle->backfacing)
					continue;
				Point v=triangle->normal();
				if(triangle->type==BOTTOM && inside())
					p=Point(0,0,0)-p;

				FColor c=triangle->color();
				Point p=triangle->vertex();
				Point m=triangle->center();
				double ext=triangle->extent();
				double rsize=ext/TheView->viewport[3];
				double tsize=smax*ext;
				Point p1=m*10000;
				p1=p1-p1.floor();
				p1=p1*256;

				int ns=num_sprites;

				double thickness=triangle->depth();
				double angle=crot*(0.5+Random(p1.x,p1.y,p1.z));
				double y=crot>0.5?0.5+RAND(1):0.5; // random reflection
				double z=0.5+RAND(2); // random point size
				double d=triangle->density(); // sprite type
				double max=lerp(thickness,0,0.1,0.25,1); // increase detail at top-bottom surface boundary
				pts=lerp(z*resolution*max,0,1,cmin,cmin*(1+cmax));

				//pts=pts<tsize?tsize:pts;
				Point viewdir = p.mm(TheScene->viewMatrix);
				viewdir=viewdir.normalize();

				// for orbital view decrease size of clouds near horizon
				Point vn=v-p;                           // line from normal (unit-less) to vertex
	            Point norm =vn.normalize();
	            Point vp = TheScene->vpoint - p;        // line from eye to vertex
	            vp = vp.normalize();
	            double dpv = norm.dot(vp);              // in surface views max for side of clouds facing eye
	            if(dpv<0)
	            	continue;
	            Point dm=TheScene->vpoint - m;          // line from eye to vertex
	            m=m.normalize();                        // line from vertex to planet center
	            dm=dm.normalize();
	            double dpm=dm.dot(m);                   // in orbital view dp = 0 at horizon 1 directly below

				double mx=maxext*dpv;
				if(pts>mx)
					pts=mx;

				double ht=TheScene->height;
				double depth=ht-parent->size;
				double dht=ht/depth;                    // eye ht as fraction of planet radius

				double maxs=lerp(dht,0,2,smax,dpm);     // reduces scatter at planet horizon in orbital view
				double scatter=lerp(dpv,0,1,0.0,maxs);  //

				//c=Color(scatter/maxs,0,1-scatter/maxs);

				ht=m.length()-gndlvl;
				depth = TheScene->vpoint.distance(m);
				d=clamp(d,0,sprites_dim*sprites_dim-0.9);
				double zht=triangle->height();
				double transmission=clamp(0.5+diffusion*20*zht,0,1);

				double a=c.alpha();
				c=c*transmission; // note: Color * operator doesn't preserve alpha (sets it to 1.0)
				c=c.clamp();
				glColor4d(c.red(), c.green(), c.blue(), a*transmission);
				glNormal3dv(v.values());

				double ts=sqrt(2.0)*pts/TheView->viewport[3];  // center offset

				glVertexAttrib4d(GLSLMgr::CommonID1, ht, depth,transmission, d); // Constants1

				switch(clouds_mode){
				case CLOUDS_BILLBOARDS: // billboards
					glVertexAttrib4d(GLSLMgr::TexCoordsID,angle*2*PI-PI/4, scatter*ts,0,0);
					glBegin(GL_TRIANGLE_FAN);
					for(int i=0;i<4;i++)
						glVertex4d(p.x,p.y,p.z,(double)i);
					glEnd();
					for(int i=0;i<ns-1;i++){
						double r1=0.5+RAND(i+3);
						double r2=0.5+RAND(i+3+ns);
						double r3=0.5+RAND(i+3+2*ns);
						double ra=2*PI*(i/ns+0.2*r2);
						double rr=rsize*(1+0.3*r3);
						glVertexAttrib4d(GLSLMgr::TexCoordsID,crot*r1*2*PI-PI/4, scatter*ts,ra,scatter*rr);
						glBegin(GL_TRIANGLE_FAN); // big overhead: glBegin called for each point
						for(int j=0;j<4;j++)
							glVertex4d(p.x,p.y,p.z,(double)j);
						glEnd();
					}
					break;
				case CLOUDS_POINTS: // point-sprites (non directional)
					glVertexAttrib4d(GLSLMgr::TexCoordsID, d, angle*2*PI, pts, y);
					glVertex3dv(p.values());
					break;
				case CLOUDS_GS_SHADER: // geometry shader
					glVertexAttrib4d(GLSLMgr::TexCoordsID,angle*2*PI-PI/4, scatter*ts,0,0);
					glVertex3d(p.x,p.y,p.z);
					for(int i=0;i<ns-1;i++){
						double r1=0.5+RAND(i+3);
						double r2=0.5+RAND(i+3+ns);
						double r3=0.5+RAND(i+3+2*ns);
						double ra=2*PI*(i/ns+r2);
						double rr=rsize*(1+0.3*r3);
						//double rr=0.5*ts*(1+r3);
						glVertexAttrib4d(GLSLMgr::TexCoordsID,crot*r1*2*PI-PI/4, scatter*ts,ra,scatter*rr);
						glVertex3d(p.x,p.y,p.z);
					}
					break;
				case CLOUDS_NO_SHADER: // OGL triangles (no shader)
					glVertex3dv(triangle->d1->point().values());
					glVertex3dv(triangle->d2->point().values());
					glVertex3dv(triangle->d3->point().values());
					break;
				}
			}
			switch(clouds_mode){
			case CLOUDS_BILLBOARDS:
				break;
			default:
				glEnd(); // all but billboards render all primitives in single glBegin loop
				break;
			}
			glEndList();
		}
		glCallList(map->callLists[SHADER_LISTS][1]);
		return false;
	}
	else{
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		if(map->frontface==GL_BACK)
			glCullFace(GL_FRONT);
		else
			glCullFace(GL_BACK);
		return true;
	}
}

//-------------------------------------------------------------
// CloudLayer::threeD() return true for volume clouds
//-------------------------------------------------------------
bool CloudLayer::threeD()
{
	if(clouds/*&& clouds->threeD()*/)
		return true;
	return false;
}

//-------------------------------------------------------------
// CloudLayer::setClouds() set clouds expression
//-------------------------------------------------------------
void CloudLayer::setClouds(TNclouds *c){
	char t[512];
	t[0]=0;
	c->valueString(t);
	TNode *root=terrain.get_root();
	root->valueString(t);
	if(clouds){
		delete clouds;
		clouds=0;
	}
	TNclouds *nc=(TNclouds*)TheScene->parse_node(t);
	terrain.set_root(nc);
	terrain.init();
	TerrainData::add_TNclouds(nc);
	clouds = nc;
	t[0]=0;
	root->valueString(t);
	//cout<<t<<endl;
	invalidate();
}

//-------------------------------------------------------------
// CloudLayer::deleteClouds() delete clouds expression
//-------------------------------------------------------------
void CloudLayer::deleteClouds(){
	if(clouds){
		char t[512];
		t[0]=0;
		NodeIF *n=clouds->getParent();
		clouds->removeNode();
		n->valueString(t);
		//cout<<t<<endl;
		delete clouds;
	}
	clouds=0;
	TerrainData::add_TNclouds(clouds);
	invalidate();
}

//-------------------------------------------------------------
// CloudLayer::map_color()   modulate render color
//-------------------------------------------------------------
void CloudLayer::map_color(MapData*d,Color &c)
{

	if(!terrain.get_root())
		c=color();
	if(Render.draw_shaded())
		return;

	static Point lp;
	static Point lv;
	static Point vp;

	if(first){
 		lp=Lights[0]->point.mm(TheScene->invViewMatrix);
		vp=TheScene->vpoint;
		lv=lp-vp;
		lv=lv.normalize();
		lp=lp.normalize();
		vp=vp.normalize();
		first=0;
	}
	if(map->frontface==GL_BACK && Render.twilite()){
		Point p=d->mpoint();
		Point pv=(p-TheScene->vpoint);
		double ds=p.length();
		double dist=pv.length();
		double dp2=lv.dot(pv)/dist;
		double a=c.alpha();
		if(dp2>0 && a>0){
			double dp1=lp.dot(p)/ds;
			double a1=rampstep(-0.4,-0.6,dp1,1,0);
			double a2=rampstep(0.5,1,dp2,0,1);
			dp2=dp2*fabs(dp2);
			double f=rampstep(0.2,-0.2,dp1,0,1);
			c=c.blend(Raster.blend_color,f*dp2);
			//c=c.blend(Lights[0]->color,0.8*a2);
			c.set_alpha(a);
		}
	}
}
//-------------------------------------------------------------
// CloudLayer::force_adapt()   adapt needed test
//-------------------------------------------------------------
bool CloudLayer::force_adapt() {
	if(TheScene->viewobj==getObjParent() && day==0.0)
	    return false;
	return true;
}

//-------------------------------------------------------------
// CloudLayer::init() set common variables
//-------------------------------------------------------------
void CloudLayer::init()
{
	Gscale=1/hscale/size;
	exprs.init();
	get_vars();
	set_geometry();
	terrain.init();
	clouds = TerrainData::clouds;
	terrain.init_render();
	terrain.set_eval_mode(0);
	map->make();
}

//-------------------------------------------------------------
// CloudLayer::adapt_object()   object level adapt
//-------------------------------------------------------------
void CloudLayer::adapt_object()
{
	if(!isEnabled())
		return;
	if(threeD())
		map->set_conv(0.1);

	map->make_current();
	terrain.set_init_mode(1);
	exprs.eval();
	terrain.init();
	terrain.init_render();
	terrain.set_eval_mode(0);

	clouds = TerrainData::clouds;
	bool v3d=false;

	if(threeD()){
		map->frontface=GL_FRONT_AND_BACK;
		v3d=true;
	}
	else{
		if(inside())
			map->frontface=GL_BACK;
		else
			map->frontface=GL_FRONT;
	}

	terrain.setAdaptMode();
	if(v3d && Render.draw_shaded()){
		map->set_sort(1);
		map->set_make_lists(1);
		map->enablelists(false);
	}
	else{
		map->set_sort(0);
		map->enablelists(true);
		map->set_make_lists(0);
	}

	glDepthMask(GL_FALSE);

	set_wscale();

	map->adapt();
	map->frontface=GL_FRONT;

	glDepthMask(GL_TRUE);
}

//-------------------------------------------------------------
// CloudLayer::render_object()   lower level render
//-------------------------------------------------------------
void CloudLayer::render_object()
{
	show_render_state(this);
	Shell::render_object();
}

//-------------------------------------------------------------
// CloudLayer::getInstance()  generate a random instance
//-------------------------------------------------------------
NodeIF *CloudLayer::getInstance(NodeIF *prev){
	lastn=getRandValue()*1271;
	Orbital::setRands();
	if(((CloudLayer*)prev)->threeD())
		return newInstance(true);
	else
		return newInstance(false);
}

//-------------------------------------------------------------
// CloudLayer::newInstance()  generate a random instance
//-------------------------------------------------------------
CloudLayer *CloudLayer::newInstance(bool is3d){
	CloudLayer *clouds=TheScene->getPrototype(0,ID_CLOUDS);
	clouds->detail=16;
	if(is3d)
		clouds->day=-(4+3*s[4]);
	else
		clouds->day=(4+3*s[5]);
	char buff[2048];
	std::string str;
    
	if(is3d)
		str=Planetoid::randFeature(RND_CLOUDS_3D);
	else
		str=Planetoid::randFeature(RND_CLOUDS_2D);
	strcpy(buff,str.c_str());
		
	TNode *root=TheScene->parse_node(buff);
	if(root){
	    clouds->terrain.set_root(root);
	    clouds->terrain.init();
	}
	else
	    cout<<"error building cloudlayer"<<endl;
	return clouds;
}

//************************************************************
// Corona class
//************************************************************
Corona::Corona(Orbital *m, double s) : Shell(m,s)
{
#ifdef DEBUG_OBJS
	printf("Corona\n");
#endif
	map->frontface=GL_BACK;
	map->lighting=0;
	year=day=0;
	detail=3;
	gradient=1;
	hscale=0;
	noise_expr=0;
	density_expr=0;
	rate=1e-6;
	//inner_radius=m->size;
}
Corona::~Corona()
{
#ifdef DEBUG_OBJS
	printf("~Corona\n");
#endif
}


//-------------------------------------------------------------
// Corona::get_vars() reserve interactive variables
//-------------------------------------------------------------
void Corona::get_vars()
{
	if(exprs.get_local("name",Td))
		strncpy(name_str,Td.string,maxstr);
	VGET("resolution",detail,4);
	VGET("gradient",gradient,1);
	VGET("internal",internal,0);

	if(exprs.get_local("color",Td))
	    set_color(Td.c);
	if(exprs.get_local("color2",Td))
		color2=Td.c;
	if(exprs.get_local("color1",Td))
		color1=Td.c;
	else if(parent){
		color1=parent->color();
		color1.set_alpha(0.95);
		color2=((Star*)parent)->emission;
	}
	TNvar *var=exprs.getVar((char*)"noise.expr");
	if(var){
		noise_expr=var->right;
		applyNoiseFunction();
	}
	else{
		noise_expr=0;
		char dstr[256];
		sprintf(dstr,"Density(1.0)");
	    density_expr=TheScene->parse_node(dstr);
	}
}

//-------------------------------------------------------------
// Corona::set_vars() set common variables
//-------------------------------------------------------------
void Corona::set_vars()
{
	VSET("resolution",detail,4);
	VSET("gradient",gradient,1);
	VSET("internal",internal,0);

	exprs.set_var("color1",color1);
	exprs.set_var("color2",color2);
}

//-------------------------------------------------------------
// Corona::setProgram() set shader program
//-------------------------------------------------------------
bool Corona::setProgram(){
	char frag_shader[128]="corona.frag";
	char vert_shader[128]="corona.vert";
	GLSLMgr::setDefString("");
	

	double dpmin,dpmax;

	double r=parent->size;
	double d=point.length();
	double s=size; // radius of shell
	double h=s-r;
	double theta=asin(r/d);
	double l1=sqrt(d*d-r*r);
	double l2=sqrt(s*s-r*r);
	double L=l1+l2;
	double D=L*cos(theta);
	double x=D-d;   // may be negative because l2 length constrained to r
	double dp=x/s;  // cos(phi)

	if(inside()){
	  	dpmax=dp;
	  	dpmin=h/(h+r);
	}
	else{
	  	//dpmax=sqrt(d*d-s*s)/d;
	  	//dpmax=l2/s;
	  	dpmax=dp;
	  	dpmin=l1/d;
	}
	    
	TerrainProperties *tp=map->tp;

	tp->initProgram();
	
	GLSLMgr::loadProgram(vert_shader,frag_shader);
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;

	GLSLVarMgr vars;

	FColor outer=FColor(color2);
	FColor inner=FColor(color1);
	
	double t=animation?rate*TheScene->time:0.0;

	vars.newFloatVar("rseed",rseed+t);

	vars.newFloatVar("dpmax",dpmax);
    vars.newFloatVar("dpmin",dpmin);
    vars.newFloatVar("gradient",gradient);
    vars.newFloatVec("outer",outer.values(),4);
    vars.newFloatVec("inner",inner.values(),4);

	Point p=point.mm(TheScene->invViewMatrix);
	p=p.mm(TheScene->viewMatrix);
	vars.newFloatVec("center",p.x,p.y,p.z);
	vars.setProgram(program);
	vars.loadVars();
	if(TheScene->inside_sky()||Raster.do_shaders)
		GLSLMgr::setFBOReadWritePass();
	else
		GLSLMgr::setFBORenderPass();

	GLSLMgr::loadVars();

	tp->setProgram();
	TheScene->setProgram();

	glEnable(GL_BLEND);
	return true;
}

//-------------------------------------------------------------
// Corona::set_surface() set properties
//-------------------------------------------------------------
void Corona::set_surface(TerrainData &data)
{
	Spheroid::set_surface(data);
	 if(density_expr){
		 density_expr->right->eval();
	    if(!Td.get_flag(SNOISEFLAG))
	    	S0.density=S0.s;
	    //cout<<S0.density<<endl;
	 }
}
//-------------------------------------------------------------
// Corona::map_color()   modulate render color
//-------------------------------------------------------------
void Corona::map_color(MapData*n,Color &col)
{
	if(Render.draw_shaded())
		return;
    static double p1=0;
    static double p2=90;
    static Color  c1;
 	if(first){
 	   	c1=color1;
 	   	Point pv=point;
		double d=pv.length();
		double s=parent->size;
		double r=size;
		double ct=s/d;
		double st=sin(acos(ct));

		Point q=Point(s*st,-s*ct,0);
		Point e=Point(0,-d,0);
		Point v=q-e;
		v=v.normalize();
		double a=v.dot(v);
		double b=v.dot(e)*2;
		double c=e.dot(e)-(r*r);
		double x=0.5*(-b+sqrt(b*b-4*a*c))/a;
		Point p=e+(v*x);
		p=p.spherical();
	    p1=rampstep(r,s,d,0,-45);
		p2=1.02*p.y;
		first=0;
	}
	double f1=rampstep(p2,p1,n->phi(),1,0);
	double f2=f1*f1*f1;

	double f=gradient*f2+(1-gradient)*f1;

 	if(!terrain.get_root())
		col=color2;
 	double a=col.alpha();
    col=col.blend(c1,f);
 	col.set_alpha(a*f);
}

//------------------------------------------------------------
// Corona::getNoiseFunction() return noise function
//-------------------------------------------------------------
int Corona::getNoiseFunction(char *buff)
{
	buff[0]=0;
	TNvar *var=exprs.getVar((char*)"noise.expr");
	if(!var)
		return 0;
	TNode *expr=var->getExprNode();
	if(!expr)
		expr=var->right;

	if(!expr)
		return 0;

	expr->valueString(buff);
	return 1;
}

//------------------------------------------------------------
// Corona::setNoiseFunction() set noise function
//-------------------------------------------------------------
void Corona::setNoiseFunction(char *expr) {
	TNvar *var = exprs.getVar((char*) "noise.expr");
    if(var && strlen(expr)==0){
    	exprs.removeVar("noise.expr");
    }
    else if (!var) {
    	var=addExprVar("noise.expr",expr);
	} else
		var->setExpr(expr);
}

//------------------------------------------------------------
// Corona::applyNoiseFunction() set & apply noise function
//-------------------------------------------------------------
void Corona::applyNoiseFunction()
{
	TNvar *var=exprs.getVar((char*)"noise.expr");
	char dstr[256];
	if(var){
		var->applyExpr();
		noise_expr=var->right;
		char argstr[256];
		argstr[0]=0;
		noise_expr->valueString(argstr);
		sprintf(dstr,"Density(%s)",argstr);
		density_expr=TheScene->parse_node(dstr);
	}
	else{
		sprintf(dstr,"Density(1.0)");
		density_expr=TheScene->parse_node(dstr);
	}
}

//-------------------------------------------------------------
// Corona::render_pass() select for scene pass
//-------------------------------------------------------------
int Corona::render_pass()
{
    clr_selected();

	if(!local_group() || offscreen() || !isEnabled())
	    return 0;

	if(getParent()==TheScene->viewobj)
		clear_pass(FG0);	
	else if(local_group())
		clear_pass(BG3);	
		
    return selected();
}
//-------------------------------------------------------------
// Corona::adapt_pass() select for scene pass
//-------------------------------------------------------------
int Corona::adapt_pass()
{
	return Spheroid::adapt_pass();
}

//-------------------------------------------------------------
// Corona::getInstance() return prototype or randomly selected object
//-------------------------------------------------------------
NodeIF *Corona::getInstance(){
	return TheScene->getInstance(type());
}
//-------------------------------------------------------------
// Corona::render_object()   lower level render
//-------------------------------------------------------------
void Corona::render_object() {
	if (Raster.auximage()) // skip surface fog/water effects
		return;
	show_render_state(this);
	glDisable(GL_DEPTH_TEST);
	first = 1;
	if(density_expr){
		terrain.set_root(density_expr);
		terrain.init();
		terrain.init_render();
	}
	else
		terrain.init_render();
	map->render();
	first = 0;
	glEnable(GL_DEPTH_TEST);
	terrain.clr_root();

}

//-------------------------------------------------------------
// Corona::orient()   tilt
//-------------------------------------------------------------
void Corona::orient()
{
    double view[16];
    TheScene->getMatrix(view);

	Matrix A(view);
	Matrix B=A*TheScene->InvEyeMatrix;

	minv(B.values(),view,4);

	Point p;

	if(TheScene->eyeref())
	     p=(Point(0,0,0)-TheScene->xoffset).mm(view);
	else
	     p=TheScene->eye.mm(view);
	p=p.spherical();

	// rotate to orient axis to point to eye

    TheScene->rotate(p.x,0,1,0);	 // remove theta
	TheScene->rotate(90+p.y,0,0,1);	 // remove phi,rotate pole down

	//TheScene->set_matrix(0);
	//setMatrix();

}

void Corona::setMatrix(){
	TheScene->set_matrix(0);
}


//************************************************************
// Halo class
//************************************************************
Halo::Halo(Orbital *m, double s) : Shell(m,s)
{
#ifdef DEBUG_OBJS
	printf("Halo\n");
#endif
	if(m){
		size=m->size;
		ht=0;
	}
	density=0.1;
	gradient=0.9;
	detail=2;
	color1=WHITE;
	color2=WHITE;
	map->frontface=GL_BACK;
}
Halo::~Halo()
{
#ifdef DEBUG_OBJS
	printf("~Halo\n");
#endif
}
//-------------------------------------------------------------
// Halo::get_vars() reserve interactive variables
//-------------------------------------------------------------
void Halo::get_vars()
{
	VGET("resolution",detail,4);
	VGET("gradient",gradient,1);
	VGET("density",density,0.2);
	if(exprs.get_local("color1",Td))
		color1=Td.c;
	if(exprs.get_local("color2",Td))
		color2=Td.c;
}

//-------------------------------------------------------------
// Halo::set_vars() set common variables
//-------------------------------------------------------------
void Halo::set_vars()
{
	VSET("resolution",detail,4);
	VSET("gradient",gradient,1);
	VSET("density",density,0.2);
	exprs.set_var("color1",color1);
	exprs.set_var("color2",color2);
}
//-------------------------------------------------------------
// Halo::set_rotation()   tilt
//-------------------------------------------------------------
void Halo::orient()
{
	//Shell::set_rotation();
    double view[16];
    TheScene->getMatrix(view);

	Matrix A(view);
	Matrix B=A*TheScene->InvEyeMatrix;

	minv(B.values(),view,4);

	Point p;

	if(TheScene->eyeref())
	     p=(Point(0,0,0)-TheScene->xoffset).mm(view);
	else
	     p=TheScene->eye.mm(view);
	p=p.spherical();

	// rotate to orient axis to point to eye

    TheScene->rotate(p.x,0,1,0);	 // remove theta
	TheScene->rotate(90+p.y,0,0,1);	 // remove phi,rotate pole down

	//TheScene->set_matrix(0);
	//setMatrix();

}

bool Halo::setProgram(){
	
	glEnable(GL_BLEND);
	char frag_shader[128]="halo.frag";
	char vert_shader[128]="halo.vert";
	GLSLMgr::setDefString("");
	GLSLMgr::loadProgram(vert_shader,frag_shader);
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;

	double dpmin,dpmax;

	double r=parent->size;
	double d=point.length();
	double s=size; // radius of shell
	double h=s-r;
	double theta=asin(r/d);
	double l1=sqrt(d*d-r*r);
	double l2=sqrt(s*s-r*r);
	double L=l1+l2;
	double D=L*cos(theta);
	double x=D-d;   // may be negative because l2 length constrained to r
	double dp=x/s;  // cos(phi)

	if(inside()){
	  	dpmax=dp;
	  	dpmin=h/(h+r);
	}
	else{
	  	dpmax=dp;
	  	dpmin=l1/d;
	}

	//TerrainProperties *tp=map->tp;

	//tp->initProgram();

	//cout<<"Halo " << dpmin << " " << dpmax << endl;
	GLSLVarMgr vars;

	FColor outer=FColor(color2);
	FColor inner=FColor(color1);

	vars.newFloatVar("dpmax",dpmax);
    vars.newFloatVar("dpmin",dpmin);
    vars.newFloatVar("gradient",gradient);
    vars.newFloatVec("outer",outer.values(),4);
    vars.newFloatVec("inner",inner.values(),4);

	Point p=point.mm(TheScene->invViewMatrix);
	p=p.mm(TheScene->viewMatrix);
	vars.newFloatVec("center",p.x,p.y,p.z);
	vars.setProgram(program);
	vars.loadVars();
	//if(TheScene->inside_sky()||Raster.do_shaders)
		GLSLMgr::setFBOReadWritePass();
	//else
	//	GLSLMgr::setFBORenderPass();

	GLSLMgr::loadVars();
	//tp->setProgram();

	return true;
}
//-------------------------------------------------------------
// Halo::map_color()   modulate render color
//-------------------------------------------------------------
void Halo::map_color(MapData*n,Color &col)
{
	if(Render.draw_shaded())
		return;
	double f=rampstep(90,0,n->phi(),0,1);
	f=pow(f,1+gradient);
    col=color1.mix(color2,f);
    double a=f*color2.alpha()+(1-f)*color1.alpha();
   //cout<<color1.alpha()<<" "<<col.alpha()<<endl;

 	col.set_alpha(a*f);
}
//-------------------------------------------------------------
// Halo::render_object()   lower level render
//-------------------------------------------------------------
void Halo::render_object() {
	if (Raster.auximage()) // skip surface fog/water effects
		return;
	show_render_state(this);
	glDisable(GL_DEPTH_TEST);
	map->render();
	first = 0;
	glEnable(GL_DEPTH_TEST);

}
void Halo::setMatrix(){
	TheScene->set_matrix(0);
}

//-------------------------------------------------------------
// Halo::render_pass() select for scene pass
//-------------------------------------------------------------
int Halo::render_pass() {
	clr_selected();

	if (!local_group() || offscreen() || !isEnabled())
		return 0;
	if (getParent() == TheScene->viewobj)
		clear_pass(FG0);
	else
		clear_pass(BG3);
	return selected();

}

//************************************************************
// Ring class
//************************************************************
int Ring::ring_id=0;
Ring::Ring(Planet *m, double s, double r) : Shell(m,s)
{
	set_cargs(2);
	width=r;
	inner_radius=s;
	year=0;
	day=0;
	tilt=0;
	map=new RingMap(inner_radius,width);
	size=inner_radius+width;
	set_geometry();
	validate();
	select_exclude();
	fg_include();
	detail=3;
}
Ring::~Ring()
{
#ifdef DEBUG_OBJS
	printf("~Ring\n");
#endif
}

//-------------------------------------------------------------
// Ring::select() render names for the object and it's children
//-------------------------------------------------------------
void Ring::select()
{
    Orbital::select();
}

//-------------------------------------------------------------
// Ring::render_pass() select for scene pass
//-------------------------------------------------------------
int Ring::render_pass()
{
	clr_selected();

    if(!local_group() || offscreen()|| !isEnabled())
		return 0;
	if(view_group()){
		if(TheScene->orbital_view()){
			if(getParent()==TheScene->viewobj)
				select_pass(BG1);
			else
				select_pass(BG2);
		}
		else if(near_group() && !inside())
			select_pass(FG0);
		else
			clear_pass(BG2);
	}
	else
		clear_pass(BG5);
    return selected();
}

//-------------------------------------------------------------
// Ring::shadow_pass() select for scene pass
//-------------------------------------------------------------
int Ring::shadow_pass()
{
	clr_selected();

    if(!view_group())
		return 0;
	if(near_group())
		select_pass(FGS);
	else
		select_pass(BGS);
    return selected();
}

//-------------------------------------------------------------
// Ring::set_geometry() configure map
//-------------------------------------------------------------
void Ring::set_geometry()
{
	size=inner_radius+width;
	map->radius=inner_radius;
	map->frontface=GL_FRONT;
	map->object=this;
	map->set_transparant(1);
}

//-------------------------------------------------------------
// Ring::rebuild() rebuild map
//-------------------------------------------------------------
void Ring::rebuild()
{
	if(invalid()){
		delete map;
		map=new RingMap(inner_radius,width);
		set_geometry();
		terrain.set_init_mode(1);
		exprs.eval();
		terrain.init();
		terrain.setAdaptMode();
		map->make();
		validate();
	}
	ObjectNode::rebuild();
}

void Ring::set_tilt()
{
	// use planet tilt parent reference
	((Orbital*)parent)->set_tilt();
}

//-------------------------------------------------------------
// Ring::setProgram() set shader program
//-------------------------------------------------------------
bool Ring::setProgram(){
	char frag_shader[128]="ring.frag";
	char vert_shader[128]="ring.vert";
	char defs[128]="";

	TerrainProperties *tp=map->tp;
	sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),Lights.size);
	if(Raster.shadows()&&Raster.farview())
		sprintf(defs+strlen(defs),"#define SHADOWS\n");
	GLSLMgr::setDefString(defs);
	tp->initProgram();

	GLSLMgr::loadProgram(vert_shader,frag_shader);
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program){
		return false;
	}
    Planetoid *parent=getParent();
    double alpha=parent->shadow_color.alpha();
	GLSLVarMgr vars;
	//vars.newFloatVar("emission",emission.alpha());
	vars.newFloatVec("Emission",emission.red(),emission.green(),emission.blue(),emission.alpha());
	vars.newFloatVec("Shadow",shadow_color.red(),shadow_color.green(),shadow_color.blue(),alpha);
	vars.newFloatVec("Diffuse",diffuse.red(),diffuse.green(),diffuse.blue(),diffuse.alpha());

	double twilite_min=-0.3;
	double twilite_max=0.4;
	double twilite_dph=0;

	vars.newFloatVar("twilite_min",twilite_min);
	vars.newFloatVar("twilite_max",twilite_max);
	vars.newFloatVar("twilite_dph",twilite_dph);
	//vars.newBoolVar("lighting",Render.lighting());

	tp->setProgram();

	if(TheScene->inside_sky()||Raster.do_shaders)
		GLSLMgr::setFBOWritePass();
	else
		GLSLMgr::setFBORenderPass();

	vars.setProgram(program);
	vars.loadVars();
	TheScene->setProgram();

	return true;
}

//-------------------------------------------------------------
// Ring::map_color()   modulate render color
//-------------------------------------------------------------
void Ring::map_color(MapData*d,Color &c)
{
	static double a=0;
	static Point sv;
	if(!terrain.get_root())
		c=color();
	if(first){
		a=rampstep(1,0,Raster.sky_color.alpha(),0.5,1);
    	Raster.shadow_darkness=rampstep(0,1,Raster.sky_color.alpha(),1,0.5);
    	first=0;
    }
}

//-------------------------------------------------------------
// Ring::render_object() render the object
//-------------------------------------------------------------
void Ring::render_object()
{
	//if(TheScene->select_mode())
	//	return;
	if(Raster.auximage()) // skip surface fog/water effects
		return;
	first=1;
	//map->lighting=0;
	if(TheScene->render_mode()){
		show_render_state(this);
    	Orbital::set_lighting();
    	Lights.setAttenuation(point);
		Lights.modDiffuse(Raster.sky_color);
		//Raster.init_lights(0);

		Raster.init_lights(0);
		map->lighting=1;
		Color c=color();
		if(Raster.sky_color.alpha())
		    c=c.blend(Raster.sky_color,Raster.sky_color.alpha());
		if(emission.alpha())
		    c=c.blend(emission,emission.alpha());
		c.set_alpha(1);
		Lights.setEmission(c);
	}
	else{
		Raster.init_lights(0);
	    map->lighting=0;
	}
	Spheroid::render_object();
}

//-------------------------------------------------------------
// Ring::save() archive
//-------------------------------------------------------------
void Ring::save(FILE *f)
{
	orbit_radius=width;
	double s=size;
	size=s-width;
	Spheroid::save(f);
	orbit_radius=0;
	size=s;
}

//-------------------------------------------------------------
// Ring::scale() 	set znear, zfar
//-------------------------------------------------------------
int  Ring::scale(double &zn, double &zf)
{
	const double MINZN=1e-5;

	double x,y,z,w,y1,y2,x1,x2;
	double aspect=TheScene->aspect;
	double d=TheScene->epoint.distance(point);
	double rmax=1.3*size;
	double rmin=size-width;

	setvis(OFFSCREEN);

	if(d<rmin){
		zn=rmin-d;
		if(zn<MINZN)
	   		zn=MINZN;
	   	zf=rmax+d;
	   	setvis(INSIDE);
	   	return INSIDE;
	}
	if(d<rmax){
		zn=MINZN;
		if(zn<MINZN)
	   		zn=MINZN;
	   	zf=rmax+d;
	   	setvis(INSIDE);
		return INSIDE;
	}

	z=point.mz(TheScene->lookMatrix);	// screen z (before perspective)
	if(z>=0)
		return OFFSCREEN;

	w=fabs(rmax+width/tan(RPD*TheScene->fov/2)/z);
	x=point.mx(TheScene->projMatrix)/z;	// screen x (-1..1)
	x2=x+w;
	x1=x-w;
	if(x1>aspect || x2<-aspect)
		return OFFSCREEN;
	y=point.my(TheScene->projMatrix)/z;	// screen y (-1..1)
	y2=y+w; y1=y-w;
	if(y1>1 || y2<-1)
		return OFFSCREEN;

	zn=-z-rmax;
	zf=-z+rmax;
	if(zn<MINZN)
	   zn=MINZN;
	setvis(OUTSIDE);
	return OUTSIDE;
}
//-------------------------------------------------------------
// Ring::getInstance()  generate a random instance
//-------------------------------------------------------------
NodeIF *Ring::getInstance(){
	return newInstance();
}

//-------------------------------------------------------------
// Ring::setInstance()  generate a random instance
//-------------------------------------------------------------
Ring *Ring::newInstance(){
	Ring *ring;

	ring=TheScene->getPrototype(0,TN_RING);
	ring_id=lastn;
	ring->setRseed(URAND(lastn++));
	
	Color tc,mix;
	const int ncolors=8;
	Color colors[ncolors];
	const int nrands=10;
			
	double r[nrands];
	double s[nrands];
	for(int i=0;i<nrands;i++){
		r[i]=URAND(lastn++);
		s[i]=RAND(lastn++);
	}
		
	tc=Color(0.5+3*r[4],0.2+2*r[5],0.8*r[5],r[2]);
    mix=Color(0.4+2*r[7],0.9*r[8],0.2+0.3*r[9],r[3]);
	colors[0]=mix;
    colors[1]=tc.lighten(0.1+0.4*r[8]);
    colors[1].set_alpha(0.1*s[6]);
	colors[2]=tc.darken(0.75+0.2*s[7]);
	colors[2].set_alpha(0.1*r[3]);
	colors[3]=tc.lighten(0.5+0.1*s[8]);
	colors[4]=tc;
	colors[4].set_alpha(0);
	colors[5]=tc.darken(0.9+0.6*s[9]);
	colors[6]=tc.lighten(0.9+0.6*s[6]);
	colors[7]=tc.darken(0.9);
	colors[7].set_alpha(0);
	
	char buff[2048];

	sprintf(buff,"bands(\"R%d\",TMP|NORM|REFLECT,64,%g,%g",ring_id,0.2+0.1*s[0],0.4+0.2*s[1]);

	for(int i=0;i<ncolors;i++){
		Color c=colors[i];
		strcat(buff,",");
		c.toString(buff+strlen(buff));
	}
	strcat(buff,");\n");
	TNinode *img=(TNinode*)TheScene->parse_node(buff);
	img->init();
	Render.invalidate_textures();
	ring->add_image(img);
	
	sprintf(buff,"Texture(\"R%d\",BLEND|DECAL|REPLACE|S|TEX,PHI,0.5,%g,1,0,1,2,1,0,0,0,0,0)",ring_id,0.1+r[0]);

	TNtexture *tex=(TNtexture*)TheScene->parse_node(buff);
	if(tex){
		ring->terrain.set_root(tex);
		ring->terrain.init();
	}
	else
		cout<<"error building ring texture"<<endl;
	ring->emission=Color(1,1,1,0.8);
	ring->shadow_color=Color(0,0,0,0.8);
	ring->detail=3;
	return ring;
}
