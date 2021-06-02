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
#include "GLSLMgr.h"
#include "AdaptOptions.h"
#include "FileUtil.h"
#include "UniverseModel.h"

extern	void rebuild_scene_tree();
extern	void select_tree_node(NodeIF *n);

//#define GEOMETRY_TEST
#define WRITE_STAR_DATA
//#define DEBUG_RENDER

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

int test2=0;
int test3=0;
int test4=0;
int test5=0;
#ifndef WRITE_STAR_DATA
#include "star_data.h"
#endif

extern double Gscale,Hscale,Rscale;
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

static Color	def_water_color1=Color(0,1,1,0.2);
static Color	def_water_color2=Color(0,0,0.5);
static double	def_water_clarity=500*FEET;
static double	def_water_mix=0.95;
//static double	def_water_saturation=0.75;
//static double 	def_water_reflectance=1.0;
static double 	def_water_specular=0.8;
static double 	def_water_shine=10;
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
	orbit_skew=orbit_angle=orbit_phase=orbit_radius=0.0;
	year=day=tilt=pitch=rot_angle=rot_phase=0.0;
	detail=1;
	set_color(WHITE);
	name_str[0]=0;
	exprs.setParent(this);
	exprs.setFlag(NODE_HIDE);
	ambient=Color(1,1,1,0);
	emission=Color(1,1,1,0);
	diffuse=Color(1,1,1,0);
	specular=Color(1,1,1,0);
	albedo=DEFAULT_ALBEDO;
	shine=DEFAULT_SHINE;
	sunset=0.2;
	rseed=0;
	Gscale=1;
}

void Orbital::set_lighting(){
    if(TheScene->render_mode()){
        Lights.defaultLighting();
        Lights.setDiffuse(1.0);
    	Lights.setEmission(emission);
    	Lights.setSpecular(albedo);
    	Lights.setShininess(shine);
    }
}

void Orbital::set_wscale(){
	float wscale=0.5*TheScene->window_width*size/tan(RPD*TheScene->fov/2);
	GLSLMgr::wscale=wscale;
}

void Orbital::set_tilt() {}
void Orbital::set_rotation() {}
void Orbital::init_view() { TheScene->maxht=0;}
Bounds *Orbital::bounds(){return 0;}
TNode *Orbital::set_terrain(TNode *r)	{return 0;}
Color Orbital::color()				{ return _color;}
void Orbital::set_color(Color c)	{ _color=c;}
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
TNinode  *Orbital::get_image(char *s, int m)
{
	return exprs.get_image(s,m);
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
int Orbital::getChildren(LinkedList<NodeIF*>&l){
	int n=exprs.getChildren(l);
    n+=ObjectNode::getChildren(l);
	return n;
}
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
		ambient=Color(1,1,1,0);
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
		VGET("albedo",albedo,DEFAULT_ALBEDO);
		specular=Color(1,1,1,albedo);
	}
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
	VSET("phase",orbit_phase,0);
	VSET("rot_phase",rot_phase,0);
	VSET("day",day,0);
	VSET("year",year,0);
	VSET("resolution",detail,1);
	CSET("color",color(),WHITE);
	CSET("ambient",ambient,Color(1,1,1,0));
	CSET("emission",emission,Color(1,1,1,0));
	CSET("specular",specular,Color(1,1,1,DEFAULT_ALBEDO));
	CSET("diffuse",diffuse,Color(1,1,1,1));
	//VSET("albedo",albedo,DEFAULT_ALBEDO);
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
	while((obj=children++)>0){
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
		while((obj=children++)>0){
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
	if(day){
		 // 360 degrees per 3600 sec/hr per day (hrs)
		secs=day*3600;
		cycles=TheScene->ftime/secs;
		rot_angle=360*(cycles-floor(cycles))+rot_phase;
	}
	else if(rot_phase){
		rot_angle=rot_phase;
	}
	if(year){
		secs=24*3600*year; // earth normal days
		cycles=TheScene->ftime/secs;
		orbit_angle=P360(orbit_phase+360*cycles);
		//orbit_angle=orbit_phase+360*(cycles-floor(cycles));
	}
	//else
	    //orbit_angle=P360(orbit_angle);
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
		TheScene->translate(0,0,orbit_radius);
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
	while((obj=children++)>0){
		if(obj->containsViewobj())
			return true;
	}
	return false;
}

NodeIF *Orbital::getInstance(int type){
	return TheScene->getInstance(type);
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
	extern double Theta,Phi,Radius;

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
        val=(TNode*)TheScene->parse_node((char*)tree->def_noise_expr);
		MALLOC(15,char,var_name);
		strcpy(var_name,"noise.expr");
		var=(TNvar*)exprs.add_expr(var_name,val);
		var->eval();
	}
	noise_expr=var->right;

	var=exprs.getVar((char*)"color.expr");
	if(!var){
        val=(TNode*)TheScene->parse_node((char*)tree->def_color_expr);
		MALLOC(15,char,var_name);
		strcpy(var_name,"color.expr");
		var=(TNvar*)exprs.add_expr(var_name,val);
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
}
Galaxy::Galaxy(Orbital *m,double s) : DensityCloud(m,s*LY)
{
#ifdef DEBUG_OBJS
	printf("Galaxy\n");
#endif
	tree=new StarTree(s);
	tree->object=this;
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
    Point vp=Point(0,0,0)-TheScene->spoint;
    TheScene->set_istring("Star dist %g ly",p.distance(vp)/LY);
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
		    TheScene->set_matrix(0);
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
			TheScene->set_matrix(0);
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
	TheScene->set_matrix(0);
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
		newSubSystem();
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
// Galaxy::newSubSystem()     make a new star system
//-------------------------------------------------------------
NodeIF *Galaxy::newSubSystem()
{
	cout << "new system"<<endl;
	int ssave=lastn;

	System  *system=TheScene->getPrototype(0,TN_SYSTEM);
	system->origin=TheScene->selm;

	double nseed=Random(system->origin);
	system->setRseed(nseed);

	system->newSubSystem();

	addChild(system);
	TheScene->regroup();
    invalidate();
    TheScene->rebuild_all();
    rebuild_scene_tree();
    select_tree_node(system);

    TheScene->focusobj=system;
    system->init_view();
    lastn=ssave;
    return system;
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
System::System(double s) : Orbital(s)
{
#ifdef DEBUG_OBJS
	printf("System\n");
#endif
	all_exclude();
}
System::System(Orbital *m,double s) : Orbital(m,s)
{
#ifdef DEBUG_OBJS
	printf("System\n");
#endif
	all_exclude();
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
 	TheScene->set_matrix(this);
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
	TheScene->maxr=size;
	TheScene->minr=10;
	TheScene->minh=0.01;
	TheScene->scale(0.001,size);
	TheScene->zoom=1;
	TheScene->hstride=TheScene->vstride=1;
	TheScene->gstride=0.1*LY;
	if(TheScene->changed_file())  // exit for open
	    return;
	TheScene->view_tilt=0;
	TheScene->view_angle=0;
	TheScene->heading=90;
	TheScene->view_skew=0.0;
	TheScene->pitch=-90.0;
	TheScene->gndlvl=0;
	TheScene->height=51.120;
	//TheScene->height=30;
	//TheScene->radius=30;
	TheScene->radius=0;
	TheScene->theta=0;
	TheScene->phi=0;
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
	((Orbital*)n)->setOrbitFrom((Orbital*)c);
	if(c->hasChild(TheScene->viewobj)){
		Orbital * m=(Orbital *)c;
		m->children.remove(TheScene->viewobj);
		n->addChild(TheScene->viewobj);
	}
	Orbital *m=(Orbital*)Orbital::replaceChild(c,n);
	return m;
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
// System::newSubSystem() generate a new random star system
//-------------------------------------------------------------
NodeIF *System::newSubSystem(){

	children.free();

	randomize();
	lastn=rseed*123457;

	double ps=pow(URAND(lastn),2);
	int nstars=1+ps*3;
	double radius=0;
	double phase=0;
	for(int i=0;i<nstars;i++){
		Star *star=getInstance(TN_STAR);
		star->setRseed(URAND(lastn++));
		star->size=2*star->size*(1+1.5*RAND(lastn++));
		star->orbit_radius=radius*star->size*(1+URAND(lastn++));
		star->orbit_phase=phase;
		phase+=360.0/nstars;
		radius+=5;
		lastn++;
		addChild(star);
	}
	return this;
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
			zn=map->vbounds.zn;
			zf=map->vbounds.zf;
#ifdef DEBUG_SCALE
	        cout<<"render zn:"<<zn/FEET<<" zf:"<<zf/MILES<<" ratio:"<<zf/zn<<endl;
#endif
		}
		t=OUTSIDE;
    }

    else{
		t=Object3D::scale(zn,zf);

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
	TheScene->rotate(tilt,1,0,0);	// tilt and freeze pole orientation
	TheScene->rotate(-orbit_angle,0,cos(RPD*tilt),-sin(RPD*tilt));
	}
}

//-------------------------------------------------------------
// Spheroid::set_rotation() rotate about axis (day)
//-------------------------------------------------------------
void Spheroid::set_rotation()
{
	if(rot_angle)
	    TheScene->rotate(rot_angle,0,1,0);	// rotate ccw
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
// Spheroid::locate() set point to global coordinates
//-------------------------------------------------------------
void Spheroid::locate()
 {
 	set_ref();

 	TheScene->pushMatrix();
 	TheScene->set_matrix(this);
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
		TheScene->set_matrix(this);
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
	set_ref();
	if(included()){
		TheScene->pushMatrix();
		set_tilt();
		set_rotation();
		TheScene->set_matrix(this);
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
		set_rotation();
		TheScene->set_matrix(this);
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
	//	pushSeed();
	map->adapt();
	//if(rseed)
	//popSeed();
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
	map->render();
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
	Gscale=1/hscale/size;
	exprs.init();
	get_vars();
	set_geometry();
	// causes problems for open_node?
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
	   // cout<<name() << " hrange:"<<map->hrange<<" hscale:"<<hscale<<endl;
	//}
    if(map->hrange)
    	return map->hrange*hscale;
    return 0;
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

	while((obj=(Orbital*)children++)>0){
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

	TheScene->minh=6*FEET;
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

	while((obj=children++)>0){
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
Color StarData::star_colors[StarData::ntypes]={
    		Color(0.957,0.525,0.255),  // L
    		Color(1.000,0.824,0.624),  // M
    		Color(1.000,0.922,0.882),  // K
    		Color(1.000,0.984,0.882),  // G
    		Color(1.000,1.000,1.000),  // F
    		Color(0.827,0.875,1.000),  // A
    		Color(0.627,0.753,1.000),  // B
    		Color(0.573,0.710,1.000)}; // O
double StarData::star_temps[StarData::ntypes]={1000,2400,3700,5200,6000,7500,10000,30000};
char StarData::star_class[StarData::ntypes]={'L','M','K','G','F','A','B','O'};

void StarData::star_info(Color col, double *t, char *m){
	int min_index=0;
	Color c=col;
	c=col;
	c.set_alpha(1.0);
	double min_diff=10;
	col.print();
	for(int i=0;i<ntypes;i++){
		double d=c.difference(star_colors[i]);
		//cout<<" "<<star_class[i]<<" "<<d<<endl;
		if(d<min_diff){
			min_index=i;
			min_diff=d;
		}
	}
	double temp=star_temps[min_index];
	double f=0;
	if(min_index<ntypes-2){
		double temp2=star_temps[min_index+1];
		double diff2=c.difference(star_colors[min_index+1]);
		f=lerp(min_diff,0,diff2,0,1);
		temp=star_temps[min_index]+f*star_temps[min_index+1];
	}
	*t=temp*col.alpha();
	sprintf(m,"%c%d",star_class[min_index],(int)(10*f*col.alpha()));
}
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
	startemp=0;
	startype[0]=0;
}
Star::~Star()
{
#ifdef DEBUG_OBJS
	printf("~Star\n");
#endif
}

void Star::setRadiance(Color c){
	emission=c;
	//c.print();
	StarData::star_info(emission,&startemp,startype);
	cout<<startype<<" "<<startemp<<endl;
}
void Star::getStarData(double *d, char *m){
	*d=startemp;
	strcpy(m,startype);
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
	exprs.set_var("temperature",startemp);
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

	TheScene->set_matrix(this);
	set_point();

	l->point=point;  // 	position of light in "eye" frame

	// OGL will set position of light from current state of ModelMatrix

	l->setPosition(Point(0,0,0));

	TheScene->popMatrix();

   // double intensity=emission.alpha();
    double intensity=startemp;
	l->setIntensity(intensity);

	l->Diffuse=diffuse;
	l->Specular=specular;
	l->color=emission;
	l->diffuse=1.5*diffuse.alpha()*l->intensity;
	l->specular=specular.alpha()*l->intensity;//0.2;
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
	char frag_shader[128]="star.frag";
	char vert_shader[128]="star.vert";
	char defs[128]="";

	TerrainProperties *tp=map->tp;
	GLSLMgr::setDefString(defs);
	tp->initProgram();

	GLSLMgr::loadProgram(vert_shader,frag_shader);
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program){
		return false;
	}

	GLSLVarMgr vars;

	set_wscale();

 	tp->setProgram();

	Point p=point.mm(TheScene->invViewMatrix);
	p=p.mm(TheScene->viewMatrix);
	vars.newFloatVec("center",p.x,p.y,p.z);
	vars.newFloatVar("rseed",rseed);

	if(TheScene->inside_sky()||Raster.do_shaders)
		GLSLMgr::setFBOReadWritePass();
	else
		GLSLMgr::setFBORenderPass();

	vars.setProgram(program);
	vars.loadVars();
	TheScene->setProgram();

	return true;
}

//-------------------------------------------------------------
// Star::render() render the object and it's children
//-------------------------------------------------------------
void Star::render()
{
	if(!isEnabled())
		return;

	set_ref();
	if(included()){
		TheScene->pushMatrix();
		set_tilt();
		set_rotation();
		TheScene->set_matrix(this);
		render_object();
		TheScene->popMatrix();
	}
	Orbital::render(); // render children
}

//-------------------------------------------------------------
// Star::render_object()   object level render
//-------------------------------------------------------------
void Star::render_object()
{
 	first=1;
	show_render_state(this);

	if(TheScene->adapt_mode())
		map->render();
	else {
		Raster.init_lights(map->lighting);
		terrain.init_render();
		map->render();
	}
}

//-------------------------------------------------------------
// Star::adapt_pass() select for scene pass
//-------------------------------------------------------------
int Star::adapt_pass()
{
	clr_selected();

    if(!isEnabled())
		return 0;

    if(!local_group() || !view_group() || offscreen())
        clear_pass(BG2);
    else
        clear_pass(BG1);
    return selected();
}

//-------------------------------------------------------------
// Star::render_pass() select for scene pass
//-------------------------------------------------------------
int Star::render_pass()
{
	clr_selected();

//    if(!local_group() || offscreen() || !isEnabled())
//		return 0;
//	if(view_group()){
		clear_pass(BG2);
//	}
//	else
		//clear_pass(BG2);
//		clear_pass(BG5);
    return selected();
}

//************************************************************
// Planetoid class (moons & planets)
//************************************************************
Planetoid::Planetoid(Orbital *m, double s, double r) :
	Spheroid(m,s,r)
{
	//hscale=0.004;
	water_color1=def_water_color1;
	water_color2=def_water_color2;
	fog_color=def_fog_color;
	water_specular=def_water_specular;
	water_shine=def_water_shine;

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

//-------------------------------------------------------------
// Planetoid::replaceNode()  replace current instance with another
//-------------------------------------------------------------
NodeIF *Planetoid::replaceNode(NodeIF *n){
	NodeIF *newnode=NodeIF::replaceNode(n);
	if(TheScene->viewobj==n)
		TheScene->change_view(ORBITAL);
	return newnode;
}
//-------------------------------------------------------------
// Planetoid::get_vars()  reserve interactive variables
//-------------------------------------------------------------
void Planetoid::get_vars()
{
	Spheroid::get_vars();

	if(exprs.get_local("water.color1",Td))
		water_color1=Td.c;
	if(exprs.get_local("water.color2",Td))
		water_color2=Td.c;
	VGET("water.clarity",water_clarity,def_water_clarity);
	VGET("water.mix",water_mix,def_water_mix);
	//VGET("water.saturation",water_saturation,def_water_saturation);
	VGET("water.level",water_level,0.0);
	//VGET("water.reflectivity",water_reflectance,def_water_reflectance);
	VGET("water.albedo",water_specular,def_water_specular);
	VGET("water.shine",water_shine,def_water_shine);

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
// Planetoid::setProgram() set shader program;
//-------------------------------------------------------------
bool Planetoid::setProgram(){
	TerrainProperties *tp=map->tp;

	char defs[512]="";
	if(Render.lighting())
		sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),Lights.size);
	else
		sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),0);

	if(!TheScene->light_view()&& !TheScene->test_view() && Raster.shadows()&&(Raster.farview()||TheScene->viewobj==this))
		sprintf(defs+strlen(defs),"#define SHADOWS\n");
	if(Raster.hdr())
		sprintf(defs+strlen(defs),"#define HDR\n");
	if(test4)
	    sprintf(defs+strlen(defs),"#define TEST4 %d\n",test4);
	if(test3)
	    sprintf(defs+strlen(defs),"#define TEST3 %d\n",test3);
	if(test2)
	    sprintf(defs+strlen(defs),"#define TEST2 %d\n",test2);
	if(TheScene->viewobj==this)
	    sprintf(defs+strlen(defs),"#define VIEWOBJ\n");
	if(Render.geometry() && tp->has_geometry())
		sprintf(defs+strlen(defs),"#define TESSLVL %d\n",Map::tessLevel());

	if(TheScene->enable_contours)
		sprintf(defs+strlen(defs),"#define CONTOURS\n");
	if(TheScene->enable_grid)
		sprintf(defs+strlen(defs),"#define GRID\n");

	double twilite_min=-0.3;
	double twilite_max=0.4;
	double twilite_dph=0;
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
	Point p=point.mm(TheScene->invViewMatrix);
	vars.newFloatVec("object",p.x,p.y,p.z); // global
	p=p.mm(TheScene->viewMatrix);
	vars.newFloatVec("center",p.x,p.y,p.z);
	vars.newFloatVec("Diffuse",diffuse.red(),diffuse.green(),diffuse.blue(),diffuse.alpha());
	vars.newFloatVec("Shadow",shadow_color.red(),shadow_color.green(),shadow_color.blue(),shadow_color.alpha());
	vars.newBoolVar("lighting",Render.lighting());
	vars.newFloatVar("sky_alpha",sky_alpha);
    vars.newFloatVar("haze_zfar",haze_zfar);
    vars.newFloatVar("haze_grad",haze_grad);
	vars.newFloatVar("fog_vmin",fog_vmin);
	vars.newFloatVar("fog_vmax",fog_vmax);
	vars.newFloatVar("fog_znear",fog_min);
	vars.newFloatVar("fog_zfar",fog_max);
	vars.newFloatVar("rseed",rseed);

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

//-------------------------------------------------------------
// Planetoid::set_vars() set common variables
//-------------------------------------------------------------
void Planetoid::set_vars()
{
    Spheroid::set_vars();

    CSET("water.color1",water_color1,def_water_color1);
    CSET("water.color2",water_color2,def_water_color2);
	USET("water.level",water_level,0,"ft");
	USET("water.clarity",water_clarity,def_water_clarity,"ft");
	VSET("water.mix",water_mix,def_water_mix);
	//VSET("water.saturation",water_saturation,def_water_saturation);
	//VSET("water.reflectivity",water_reflectance,def_water_reflectance);
	VSET("water.albedo",water_specular,def_water_specular);
	VSET("water.shine",water_shine,def_water_shine);
	CSET("fog.color",fog_color,def_fog_color);
	VSET("fog.value",fog_value,def_fog_value);
	VSET("fog.glow",fog_glow,def_fog_glow);

	USET("fog.min",fog_min,def_fog_min,"ft");
	USET("fog.max",fog_max,def_fog_max,"ft");
	USET("fog.vmin",fog_vmin,def_fog_vmin,"ft");
	USET("fog.vmax",fog_vmax,def_fog_vmax,"ft");
}

//-------------------------------------------------------------
// Planetoid::adapt_pass() select for scene pass
//-------------------------------------------------------------
int Planetoid::adapt_pass()
{
	clr_selected();

    if(!local_group() || !view_group() || offscreen()|| !isEnabled())
        clear_pass(BG2);
    else{
        if(TheScene->viewobj==this)
            clear_pass(FG0);
        else
            clear_pass(BG1);
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
		if(TheScene->viewobj==this || TheScene->orbital_view())
			select_pass(FG0);
		else
			clear_pass(BG2);
	}
	else
		clear_pass(BG5);
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

	while ((obj = children++) > 0) {
		if(!obj->included())
			continue;
		shells.add(obj);
	}
	shells.ss();

	while ((obj = shells++) > 0) {
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
		TheScene->set_matrix(this);
		render_object();
		TheScene->popMatrix();
		if (!isViewobj()){
			TheScene->set_frontside();
			//Orbital::render(); // render children
			if (outsiders.size > 0) {
				outsiders.ss();
				while ((obj = outsiders++) > 0) { // front-to-back
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
			while ((obj = insiders--) > 0) { // back-to-front
				TheScene->pushMatrix();
				obj->render();
				TheScene->popMatrix();
			}
		}
		TheScene->set_frontside();
		if (outsiders.size > 0) {
			outsiders.ss();
			while ((obj = outsiders++) > 0) { // front-to-back
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
 	visit(&Object3D::init_render);

	double dp=0;
	if(TheScene->viewobj==this && !(TheScene->inside_sky())){
		Raster.blend_factor=Raster.darken_factor=0;
		Raster.haze_value=0;
		Point lp=Lights[0]->point.mm(TheScene->invViewMatrix);
		lp=lp.normalize();
		Point vp=TheScene->vpoint;
		dp=lp.dot(vp)/vp.length();
		double f=rampstep(def_twilite_max,def_twilite_min,dp,0,def_twilite_value);
		Raster.blend_factor=f;
		Raster.darken_factor=def_twilite_value*f;
	}

 	glDisable(GL_FOG);

 	// Sky.init_render sets Raster.sky_color and Raster.twilite_color

	//if(TheScene->viewobj==this)
  	//	Raster.sky_color=shadow_color;

	Raster.shadow_value=shadow_color.alpha();
 	Raster.shadow_color=shadow_color;
 	Raster.shadow_color.set_alpha(1);

    // set water factors

	//Raster.water_reflect=water_reflectance;
	Raster.water_clarity=water_clarity;
	Raster.water_mix=water_mix;
	//Raster.water_saturation=water_saturation;
	Td.clarity=water_clarity;

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
	//cout << "Planetoid::adapt_object" << endl;
    Td.clarity=water_clarity;
	Raster.water_level=water_level;
	Raster.water_color1=water_color1;
	Raster.water_color2=water_color2;
	Raster.water_clarity=water_clarity;
	// TODO: first check if any child is a fg object
	//if(TheScene->viewobj==this && !TheScene->view->changed_model())
//		map->render_zvals(); // set parant occlusion mask in zbuffer
	//map->set_render_ftob();

	pushSeed();
    Spheroid::adapt_object();
    popSeed();
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

    if(TheScene->bgpass==BG4){
		map->set_mask(1);
		Color c=Raster.blend_color;
	    glColor4d(c.red(),c.green(),c.blue(),1);
		map->render_zvals();
	    map->set_mask(0);
	}
	else{
		Raster.water_level=water_level;
		Color c= water_color1;
		Raster.modulate(c);
		c.set_alpha(water_color1.alpha());
		Raster.water_color1=c;
		c=water_color2;
		Raster.modulate(c);
		Raster.water_color2=c;
		//cout<<"Render planet:"<<name()<<endl;
		Spheroid::render_object();
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
	if(Render.draw_shaded() && TheScene->inside_sky())
		return;

	/*
	if(TheScene->viewobj==this){
		if(Raster.surface==2)
			c.set_alpha(c.alpha()*wf);
		return;
	}
	*/
	if(Raster.surface==2){
		Color c1=Raster.water_color1;
		Color c2=Raster.water_color2;

		double f=rampstep(0,Raster.water_clarity,d->depth(),0,wf);
		double alpha=c1.alpha()*(1-f)+c2.alpha()*f;
		c=c1*(1-f)+c2*f;
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

//-------------------------------------------------------------
// Planetoid::set_lighting()  set light attributes
//-------------------------------------------------------------
void Planetoid::set_lighting(){
    Orbital::set_lighting();
    Lights.setAttenuation(point);
	Raster.water_shine=water_shine;
	Raster.water_specular=water_specular;
	//Raster.sky_color.print();
	//Raster.blend_color.print();
	Lights.modDiffuse(Raster.sky_color);
	Lights.modSpecular(Raster.blend_color);
}

//-------------------------------------------------------------
// Planetoid::calc_delt() determine (normalized) position of the sun.
//-------------------------------------------------------------
double Planetoid::calc_delt()
{
	Point pl=Lights[0]->point.mm(TheScene->InvModelMatrix.values());

	double tl=pl.spherical().x;
	double tv=TheScene->theta;
	double del=P360(tl-tv)/360;
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
//	set_geometry();
//	exprs.eval();
//	terrain.init();
//	terrain.setAdaptMode();
//	set_wscale();
//
//	map->adapt();
}

//-------------------------------------------------------------
// Shell::set_geometry() set map properties
//-------------------------------------------------------------
void Shell::set_geometry()
{
	Spheroid::set_geometry();
	Object3D *parent=getParent();
	if(parent){
		double dht=size-parent->size;
		if(dht!=ht){
			size=parent->size+ht;
		}
		symmetry=((Spheroid*)parent)->symmetry;
		map->symmetry=symmetry;
		map->radius=size;
	}
	else
		cout<<"SHELL NO PARENT"<<endl;
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
	twilite_max=0.2;
	twilite_min=-0.2;
	haze_color=def_haze_color;
	density=0.01;
	detail=2;
}
Sky::~Sky()
{
#ifdef DEBUG_OBJS
	printf("~Sky\n");
#endif
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

    	Raster.haze_zfar=haze_zfar;
    	Raster.twilite_dph=twilite_dph;
    	Raster.twilite_max=twilite_max;
    	Raster.twilite_min=twilite_min;

    	Point p=point.mm(TheScene->invViewMatrix);
    	p=p.mm(TheScene->viewMatrix);

    	Raster.center=p;


    	//cout << "horizon:"<<l1/MILES << " gndlvl:"<< TheScene->gndlvl/MILES<<endl;
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
	VGET("haze.value",haze_value,def_haze_value);
	if(exprs.get_local("haze.color",Td)){
		haze_color=Td.c;
		haze_value=haze_color.alpha();
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
		tilt=((Orbital*)parent)->tilt;
		day=((Orbital*)parent)->day;
	}
	Spheroid::set_ref();
	//day=tilt=0;
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
	vars.newFloatVar("twilite_min",twilite_min);
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
	set_ref();
	if(included()){
		show_render_state(this);
		map->lighting=1;
		glDepthMask(GL_FALSE);

		TheScene->pushMatrix();
		set_tilt();
		set_rotation();

		TheScene->set_matrix(this);
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
			GLSLMgr::max_output=4;  // special case
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
				double transmission=clamp(1+diffusion*50*zht,0,1);

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
// CloudLayer::animate() adjust rotation with time
//-------------------------------------------------------------
void CloudLayer::animate()
{
	double cd=((Spheroid*)parent)->day-day;
	if(cd){
		 // 360 degrees per 3600 sec/hr per day (hrs)
		double secs=cd*3600;
		double cycles=TheScene->ftime/secs;
		rot_angle=360*(cycles-floor(cycles))+rot_phase;
	}
	else if(rot_phase){
		rot_angle=rot_phase;
	}

	//rot_angle=360*fmod((day*TheScene->ftime/3600+rot_phase),1.0);
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
	gradient=0.8;
	hscale=0;
	noise_expr=0;
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
	VGET("resolution",detail,3);
	VGET("gradient",gradient,0.8);

	if(exprs.get_local("color",Td))
	    set_color(Td.c);
	if(exprs.get_local("color2",Td))
		color2=Td.c;
	if(exprs.get_local("color1",Td))
		color1=Td.c;
	else if(parent){
		color1=parent->color();
		color1.set_alpha(0.95);
	}
	TNvar *var=exprs.getVar((char*)"noise.expr");
	if(var)
		noise_expr=var->right;
}

//-------------------------------------------------------------
// Corona::set_vars() set common variables
//-------------------------------------------------------------
void Corona::set_vars()
{
	VSET("resolution",detail,3);
	VSET("gradient",gradient,0.8);
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
	  	//dpmax=sqrt(d*d-s*s)/d;
	  	//dpmax=l2/s;
	  	dpmax=dp;
	  	dpmin=l1/d;
	}

//	dpvars(dht,dpmin,dpmax);

	TerrainProperties *tp=map->tp;

	tp->initProgram();

	//cout << dpmin << " " << dpmax << endl;
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
	if(TheScene->inside_sky()||Raster.do_shaders)
		GLSLMgr::setFBOReadWritePass();
	else
		GLSLMgr::setFBORenderPass();

	GLSLMgr::loadVars();
	tp->setProgram();

	glEnable(GL_BLEND);
	return true;
}

//-------------------------------------------------------------
// Corona::set_surface() set properties
//-------------------------------------------------------------
void Corona::set_surface(TerrainData &data)
{
	Spheroid::set_surface(data);
	 if(noise_expr){
	    noise_expr->eval();
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
		char *var_name;
		TNode *val = (TNode*) TheScene->parse_node(expr);
		MALLOC(15, char, var_name);
		strcpy(var_name, "noise.expr");
		var = (TNvar*) exprs.add_expr(var_name, val);
	} else
		var->setExpr(expr);
}

//------------------------------------------------------------
// Corona::applyNoiseFunction() set & apply noise function
//-------------------------------------------------------------
void Corona::applyNoiseFunction()
{
	TNvar *var=exprs.getVar((char*)"noise.expr");
	if(var){
		var->applyExpr();
		noise_expr=var->right;
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
	clear_pass(BG3);
    return selected();
}
//-------------------------------------------------------------
// Corona::adapt_pass() select for scene pass
//-------------------------------------------------------------
int Corona::adapt_pass()
{
	clr_selected();

    if(!local_group() || !view_group() || offscreen())
        clear_pass(BG2);
    else
        clear_pass(BG1);
    return selected();
}

//-------------------------------------------------------------
// Corona::render_object()   lower level render
//-------------------------------------------------------------
void Corona::render_object()
{
    first=1;
	show_render_state(this);
	if(Render.draw_solid() || Render.draw_shaded()){
		glDisable(GL_DEPTH_TEST);
		if(!Render.draw_nvis()){
			Render.set_draw_nvis(1);
			Shell::render_object();
			Render.set_draw_nvis(0);
		}
		else
			Shell::render_object();
		glEnable(GL_DEPTH_TEST);
	}
	else
		Shell::render_object();
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

	TheScene->set_matrix(0);

}

//-------------------------------------------------------------
// Corona::render() render the object and it's children
//-------------------------------------------------------------
void Corona::render()
{
	if(!isEnabled())
		return;

	set_ref();
	if(included()){
		TheScene->pushMatrix();
		set_tilt();
		set_rotation();
		orient();
		render_object();
		TheScene->popMatrix();
	}
	Orbital::render(); // render children
}

//-------------------------------------------------------------
// Corona::adapt() adapt the object and it's children
//-------------------------------------------------------------
void Corona::adapt()
{
	set_ref();
	if(included()){
		TheScene->pushMatrix();
		set_tilt();
		set_rotation();
		orient();
		adapt_object();
		TheScene->popMatrix();
	}
	Orbital::adapt(); // adapt children
}

//************************************************************
// Ring class
//************************************************************
Ring::Ring(Planet *m, double s, double r) : Spheroid(m,s)
{
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
		if(TheScene->orbital_view())
			select_pass(FG0);
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

	GLSLVarMgr vars;
	//vars.newFloatVar("emission",emission.alpha());
	vars.newFloatVec("Emission",emission.red(),emission.green(),emission.blue(),emission.alpha());
	vars.newFloatVec("Shadow",shadow_color.red(),shadow_color.green(),shadow_color.blue(),shadow_color.alpha());
	vars.newFloatVec("Diffuse",diffuse.red(),diffuse.green(),diffuse.blue(),diffuse.alpha());

	double twilite_min=-0.3;
	double twilite_max=0.4;
	double twilite_dph=0;

	vars.newFloatVar("twilite_min",twilite_min);
	vars.newFloatVar("twilite_max",twilite_max);
	vars.newFloatVar("twilite_dph",twilite_dph);
	vars.newBoolVar("lighting",Render.lighting());

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
	//c.set_alpha(c.alpha()*a);


/*


	if(first){
 		sv=Lights[0]->point.mm(TheScene->InvViewMatrix);
		sv=sv.normalize();
		a=1-Raster.sky_color.alpha();
		a=sqrt(a);
		first=0;
	}

	if(!terrain.get_root())
		c=color();
 	if(TheScene->viewobj&&((Orbital*)TheScene->viewobj)->inside_sky()){
		Point p=d->mpoint();
		double dp=sv.dot(p)/p.length();
		double f=rampstep(0.2,1,dp,a,1);
		c.set_alpha(f*c.alpha());
	}
	*/

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
	const double MINZN=1e-8;

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
