// SceneClass.cpp

#include "SceneClass.h"
#include "TerrainData.h"
#include "AdaptOptions.h"
#include "RenderOptions.h"
#include "LightClass.h"
#include "ImageMgr.h"
#include "Effects.h"
#include "GLglue.h"
#include "Octree.h"
#include "ModelClass.h"
#include "FileUtil.h"
#include "FrameClass.h"
#include "TerrainClass.h"
#include "MapNode.h"
#include "KeyIF.h"
#include <math.h>
#include <stdio.h>
#include <GLSLMgr.h>

#define JPEG

#ifdef JPEG
  extern void writeJpegFile(char *,double d);
  extern void showJpegFile(char *);

#else
  static void writeJpegFile(char *s,double d) {}
  static void showJpegFile(char *s) {}
#endif

Scene *TheScene=0;
View *TheView=0;
int scene_objects;
int rendered_objects;
int show_visits=0;
double mark_start=0;
double mark_stop=0;

#define DEBUG_TIMING
#define VERIFY_OPEN

extern int hits,visits;
extern int test3;

static VFV initializers[32];
static int inits=0;
static VFV finishers[32];
static int fins=0;
static TerrainData Td;

extern int place_visits,place_hits,place_misses,place_gid;
extern int tnoise_visits,tnoise_hits,tnoise_visits;
extern int recalced,tcreated, tdeleted, tcount;

const char *dflt_movie_name="movie";

int scene_rendered=0;
static double timing_start=0;

extern	void rebuild_scene_tree();
extern	void select_tree_node(NodeIF *n);

//********* functions called from GLglue.cpp (GLUT) ************

//-------------------------------------------------------------
// mouse_down() GLUT mouse press event
//-------------------------------------------------------------
void mouse_down(int x, int y){
    TheScene->select(x,y);
}
//-------------------------------------------------------------
// resize_window() GLUT window resize event
//-------------------------------------------------------------
void resize_window(int width, int height)
{
    TheScene->resize(width,height);
    TheScene->set_changed_detail();
}
//*************************************************************

char tabs[256];
int tabcnt=0;
//-------------------------------------------------------------
// reset_tabs() reset tab string
//-------------------------------------------------------------
void reset_tabs()
{
	tabcnt=0;
	tabs[0]=0;
}
//-------------------------------------------------------------
// inc_tabs() increment tab string
//-------------------------------------------------------------
void inc_tabs()
{
	tabs[0]=0;
	tabcnt++;
	int i;
	for(i=0;i<tabcnt;i++)
		tabs[i]='\t';
	tabs[i]=0;
}

//-------------------------------------------------------------
// dec_tabs() decrement tab string
//-------------------------------------------------------------
void dec_tabs()
{
	int i;
	tabcnt--;
	tabs[0]=0;
	for(i=0;i<tabcnt;i++)
		tabs[i]='\t';
	tabs[i]=0;
}

void init_for_cycle()
{
    recalced=0;
}

void init_for_rebuild()
{
    place_visits=place_hits=place_misses=0;
    tnoise_visits=tnoise_hits=0;
	recalced=tcreated=tdeleted=tcount=0;
}
void init_for_open()
{
    init_for_rebuild();
	place_gid=0;
}
void add_initializer(VFV func)
{
	initializers[inits++]=func;
}

void exec_initializers()
{
	VFV func;
	for(int i=0;i<inits;i++){
		func=initializers[i];
		(*func)();
	}
}

void add_finisher(VFV func)
{
	finishers[fins++]=func;
}

void exec_finishers()
{
	VFV func;
	for(int i=0;i<fins;i++){
		func=finishers[i];
		(*func)();
	}
}

const char *vstg[]={
	"OFF",
	"IN",
	"OUT"
};
const char *pstg[]={
	"FG1",
	"FG0",
	"BG0",
	"BG1",
	"BG2",
	"BG3",
	"BG4",
	"BG5",
	"BG6",
	"BGS",
	"FGS"
};

static int info_enabled=0;
static int info_scale=1;
static int adapt_info=0;
static int render_info=1;

static double mind,maxd;
static double minz=10*FEET;

static LinkedList<Object3D*>passlist;
static char grps[4];

#define SET_SHOWINFO \
if(!Render.display(OBJINFO)) \
	info_enabled=0; \
else if(adapt_mode() && adapt_info) \
	info_enabled=1; \
else if(render_mode() && render_info) \
	info_enabled=1; \
else \
	info_enabled=0;

//-------------------------------------------------------------
// total_objs() return number of objects in tree
//-------------------------------------------------------------
static int obj_cnt=0;
static void total_objs_(Object3D *obj)
{
	obj_cnt++;
}
static int total_objs()
{
	obj_cnt=0;
	TheScene->objects->visit(&total_objs_);
	printf("total objects %d\n",obj_cnt);
	return obj_cnt;
}

//************************************************************
// Scene class
//************************************************************
Scene::Scene(Model *m)
{
	model=m;
	TheScene=this;
	TheView=this;
	view=0;

	playback_rate=20;   // movie playback(frames/sec)
	redraw_rate=20;     // normal refresh(frames/sec)
	animate_rate=20;    // animation refresh(frames/sec)
	refresh_rate=redraw_rate;

	exprs.parent=&vars;
	strcpy(filename,"Startup.spx");
	strcpy(moviename,dflt_movie_name);
	cells=0;
	cycles=0;
	views=0;
	movie=0;
	status=0;
	vmode=VLOG;
	rseed=0;
	viewobj=selobj=localobj=groupobj=focusobj=rootobj=0;
}

Scene::~Scene()
{
	free();
	save_prefs();
	prefs.free();
	vars.free();
	exprs.free();
	TheScene=0;
	DFREE(model);
	DFREE(views);
	DFREE(movie);
	frame_files.free();
}

//-------------------------------------------------------------
// ObjectNode::Model and TreeNode methods
//-------------------------------------------------------------
TreeNode *Scene::buildTree()		{
	TreeNode *root=model->buildTree(this);
	return root;
}
void Scene::freeTree(TreeNode *t)	{
	model->freeTree(t);
}
void Scene::printTree(TreeNode *t,char *s) {
	model->printTree(t,s);
}
int  Scene::treeNodes(TreeNode *t) {
	return model->countNodes(t);
}
int Scene::getPrototype(int t ,char *s){
	return model->getPrototype(t,s);
}
int Scene::setPrototype(NodeIF *n, NodeIF *s){
	return model->setPrototype(n,s);
}

//-------------------------------------------------------------
// Scene::getInstance() return prototype or randomly selected object
//-------------------------------------------------------------
NodeIF *Scene::getInstance(int type){
	char sbuff[1024];
	sbuff[0]=0;
	NodeIF  *newobj=0;

	LinkedList<ModelSym*>flist;
	model->getFileList(type,flist);
	double rval=2*URAND(lastn);
    rval=fmod(rval,1.0);
	if(flist.size){
		int menu_id = fabs(rval) * flist.size;
		cout<<"lastn:"<<PERM(lastn)<<" rval="<<rval<<" id="<<menu_id+1<<":"<<flist.size<<endl;

		ModelSym* sym=flist[menu_id];
		model->getFullPath(sym,sbuff);
		newobj=open_node(this,sbuff);
	}
	else{
		model->getPrototype(type,sbuff);
		newobj=parse_node(sbuff);
	}
	return newobj;
}

NodeIF* Scene::getPrototype(NodeIF *obj, int type){
	char sbuff[1024];
	NodeIF *newobj=0;
	if (model->getPrototype(type,sbuff)){
		newobj=parse_node(obj,sbuff);
		newobj->setParent(obj);
	}
	return newobj;
}

NodeIF* Scene::makeObject(NodeIF *obj, int type){

	cout<<"Scene::makeObject "<<obj->getParent()->typeName()<<" "<<obj->typeName()<<" "<<type<<endl;
	
	NodeIF *m=getPrototype(obj,type);
	NodeIF *n=m->getInstance(obj);
	
	if(n){
		n->setType(type);
		n->setParent(obj);
	}
	if(m!=n)
		delete m;
	return n;
}

bool Scene::containsViewobj(ObjectNode *obj){
	return ((Orbital *)obj)->containsViewobj();
}

//-------------------------------------------------------------
// Scene::setSeed() set a random seed
//-------------------------------------------------------------
void Scene::setSeed(double s)
{
	rseed=s;
	add_expr("rseed",rseed);
    cout << "Random Seed=" << rseed << endl;
}
//-------------------------------------------------------------
// Scene::make() build the startup scene
//-------------------------------------------------------------
void Scene::setRandomSeed()
{
	double seed=getRandValue();
	setSeed(seed);
}
/*
//-------------------------------------------------------------
// Scene::getPrototype() return prototype
//-------------------------------------------------------------
NodeIF *Scene::getPrototype(int type){
	char sbuff[1024];
	sbuff[0]=0;
	NodeIF  *newobj=0;

	model->getPrototype(getParent(),type,sbuff);
	newobj=parse_node(sbuff);

	return newobj;
}
*/

//-------------------------------------------------------------
// Scene::setProgram() set shader variables
//-------------------------------------------------------------
bool Scene::setProgram(){
	GLSLMgr::loadVars();
	GLSLVarMgr vars;

	float colormip=color_mip;
	
	vars.newFloatVar("bumpmip",bump_mip);
	vars.newFloatVar("freqmip",freq_mip);
	if(Raster.filter_aniso())
		vars.newFloatVar("texmip",tex_mip+0.5);
	else
		vars.newFloatVar("texmip",tex_mip-0.5);

	vars.newFloatVar("colormip",4*colormip);

 	vars.setProgram(GLSLMgr::programHandle());
	vars.loadVars();
	return true;
}

//-------------------------------------------------------------
// Scene::get_prefs() read application scope variables file
//-------------------------------------------------------------
void Scene::read_prefs(){
	char base[256];
	char dir[256];
  	File.getBaseDirectory(base);
 	sprintf(dir,"%s/Resources/UI/prefs.spx",base);
 	prefs_mode=true;
	model->parse(dir);
 	prefs_mode=false;
 	get_prefs();
}

//-------------------------------------------------------------
// Scene::save_prefs() save application scope variables file
//-------------------------------------------------------------
void Scene::save_prefs(){
	char base[256];
	char dir[256];
  	File.getBaseDirectory(base);
 	sprintf(dir,"%s/Resources/UI/prefs.spx",base);
 	set_prefs();

    FILE *fp=fopen(dir,"wb");
    if(fp){
    	fprintf(fp,"Scene() {\n");
    	inc_tabs();
    	prefs.save(fp);
    	dec_tabs();
    	fprintf(fp,"}\n");
		fclose(fp);
    }
}

//-------------------------------------------------------------
// Scene::set_prefs() set application scope variables
//-------------------------------------------------------------
void Scene::set_prefs(){
    PCSET("contour_color",contour_color,Color(1,1,1));
	PCSET("phi_color",phi_color,Color(0,1,1));
	PCSET("theta_color",theta_color,Color(0,1,0));
	PCSET("text_color",text_color,Color(0,1,1));
	PVSET("enable_contours",enable_contours,1);
	PVSET("enable_grid",enable_grid,1);
	PVSET("grid_spacing",grid_spacing,1);
	PVSET("contour_spacing",contour_spacing,1000);
	PVSET("autogrid",autogrid(),0);
}

//-------------------------------------------------------------
// Scene::get_prefs() get application scope variables
//-------------------------------------------------------------
void Scene::get_prefs(){
	prefs.eval();
	PCGET("contour_color",contour_color,Color(1,1,1));
	PCGET("phi_color",phi_color,Color(0,1,1));
	PCGET("theta_color",theta_color,Color(0,1,0));
	PCGET("text_color",text_color,Color(0,1,1));
	PVGET("enable_contours",enable_contours,1);
	PVGET("enable_grid",enable_grid,1);
	PVGET("grid_spacing",grid_spacing,1);
	PVGET("contour_spacing",contour_spacing,1000);

    if(prefs.get_local("autogrid",Td))
    	set_autogrid(Td.s);
    else
    	set_autogrid(0);

	syscolor[INFO_COLOR]=text_color;
}

//-------------------------------------------------------------
// Scene::adjust_grid() afjust contours based on height
//-------------------------------------------------------------
void Scene::adjust_grid() {
	if(!autogrid() || !viewobj)
		return;
	double ht=height/FEET;
	double cv=(int)(ht/1000.0+0.5);
	if(cv<10){
		contour_spacing=1000;
		grid_spacing=1;
	}
	else if(cv<50){
		contour_spacing=2000;
		grid_spacing=5;
	}
	else if(cv<100){
		contour_spacing=5000;
		grid_spacing=10;
	}
	else if(cv<200){
		contour_spacing=10000;
		grid_spacing=20;
	}
	else if(cv<400){
		contour_spacing=20000;
		grid_spacing=50;
	}
	else if(cv<2000){
		contour_spacing=20000;
		grid_spacing=100;
	}
	else if(cv<5000){
		contour_spacing=20000;
		grid_spacing=500;
	}
	else {
		contour_spacing=20000;
		grid_spacing=1000;
	}

	//cout <<" grid:"<<grid_spacing<<endl;
}

//-------------------------------------------------------------
// Scene::eval_exprs() evaluate and set scene scope variables
//-------------------------------------------------------------
void Scene::eval_exprs(){
	exprs.eval();
	VGET("rseed",rseed,0);
}

//-------------------------------------------------------------
// Scene::add_expr() add expression to expr list
//-------------------------------------------------------------
void Scene::add_expr(char *s, TNode *r)
{
	TNode  *n;
	if(prefs_mode){
		prefs.add_expr(s,r);
	}
	else{
		exprs.add_expr(s,r);
	}
}

double Scene::get_expr(char *s){
	TNvar *var=exprs.get_expr(s);
	if(var){
		var->eval();
		return S0.s;
	}
    return 0.0;
}
void Scene::add_expr(char *s,double v){
	char *n=newstr(s);
	TNconst *node = new TNconst(v);
	add_expr(n,node);
}
void Scene::remove_expr(char *s){
	TNvar *var=exprs.removeVar(s);
	if(var)
		delete var;
}

//-------------------------------------------------------------
// NodeIF methods
//-------------------------------------------------------------
int Scene::getChildren(LinkedList<NodeIF*>&l){
	objects->list.ss();
	NodeIF *obj;
	while((obj=(NodeIF *)objects->list++))
		l.add(obj);

	return objects->list.size;
}

//-------------------------------------------------------------
// Scene::NodeIF methods
//-------------------------------------------------------------
bool Scene::hasChildren(){
	return objects->list.size?true:false;
}
//-------------------------------------------------------------
NodeIF *Scene::addChild(NodeIF *c){

  Object3D *obj=objects->list[0];  // get first (only) object
  if(obj)
	  obj->addChild(c);
  return c;
}
//-------------------------------------------------------------
NodeIF *Scene::addAfter(NodeIF *b,NodeIF *c){
	//ObjectNode::addAfter(b,c);
	addChild(c);
	return c;
}

//-------------------------------------------------------------
NodeIF *Scene::removeChild(NodeIF *c){
  Object3D *obj=objects->list[0];  // get first (only) object
  if(obj)
	  obj->removeChild(c);
   return c;
}

//-------------------------------------------------------------
NodeIF *Scene::replaceChild(NodeIF *c,NodeIF *n){
  Object3D *obj=objects->list[0];  // get first (only) object
  if(obj)
	  return obj->replaceChild(c,n);
  return 0;
}

//-------------------------------------------------------------
// Scene::show_status() display data in status panel
//-------------------------------------------------------------
void Scene::show_status(int which)
{
	switch(which){
	case INFO_POSITION:
	case INFO_ELEVATION:
	    if(cartesion()){
			set_position("X: %-3.1f Y: %-3.1f Z: %-3.1f ly",
				spoint.x/LY,spoint.y/LY,spoint.z/LY);
		} else {
			if(height>1*LY)
			    set_position("T: %-3.1f P: %-2.1f HT: %-3.0f ly",
				theta,phi,height/LY);
			else if(height>0.1*MM)
			    set_position("T: %-3.1f P: %-2.1f HT: %-3.1f mm",
				theta,phi,height/MM);
			else if(height>10*MILES)
			    set_position("T: %-3.1f P: %-2.1f HT: %-5.0f m",
				theta,phi,height/MILES);
			else
			    set_position("T: %-3.1f P: %-2.1f HT: %-5.0f ft",
				theta,phi,height/FEET);
		}
		break;
	case INFO_HSTRIDE:
	case INFO_VSTRIDE:
	    if(cartesion()){
			set_status("LS: %-3.5f",gstride/LY);
		} else {
			if(delh>LY || delv>LY)
			    set_status("H: %-5.1f V: %-5.1f ly",delh/LY,delv/LY);
			else if(delh>0.1*MM || delv>0.1*MM)
			    set_status("H: %-5.1f V: %-5.1f mm",delh/MM,delv/MM);
			else if(delh>10*MILES || delv>10*MILES)
			    set_status("H: %-5.0f V: %-5.0f ml/s",delh/MILES,delv/MILES);
			else
			    set_status("H: %-5.0f V: %-5.0f ft/s",delh/FEET,delv/FEET);
		}
		break;
	case INFO_TIME:
		set_status("TM: %-6.0f",time);
		break;
	case INFO_CELLSIZE:
		set_status("RES: %-2.1f",extent());
		break;
	}
}

//-------------------------------------------------------------
// Scene::show_info() display data in info panel
//-------------------------------------------------------------
void Scene::show_info(int which)
{
	int svisits=visits;
	int shits=hits;
	if(tnoise_visits){
		svisits=tnoise_visits;
		shits=tnoise_hits;
    }
    switch(which){
    case INFO_TIME:
		static char b[256];
		b[0]=0;
		if(svisits && show_visits)
			sprintf(b,"V: %2.1f (%2.1f)",svisits/1000.0,100.0*shits/svisits);
		else
			sprintf(b,"A:%-3.0f R:%-3.0f N:%-2.0f C:%-2d",adapt_time*1000,render_time*1000,0.001*cells,cycles);
		set_info("%s",b);
		break;
	}
}

//-------------------------------------------------------------
// Scene::init() initialize all
//-------------------------------------------------------------
void Scene::init()
{
	gl_init();
	reset();
	set_quality(NORMAL);
	vmode=VLOG;
	playback_rate=20;   // movie playback(frames/sec)
	redraw_rate=20;     // normal refresh(frames/sec)
	animate_rate=20;    // animation refresh(frames/sec)
	refresh_rate=redraw_rate;
	inter_frames=5;
	inter_index=0;
	frame_index=0;
	set_intrp(1);
	Render.set_multitexs(0);
	set_movie((char*)dflt_movie_name);
	read_prefs();
}

//-------------------------------------------------------------
// Scene::reset() reset to standard conditions
//-------------------------------------------------------------
void Scene::reset()
{
    reset_tabs();
	status=0;
	set_render_mode();
	reset_time();
	backcolor=BLACK;
	objpath.reset();
	elevation=0.0;
	resolution=2.0;
	height=0.0;
	selobj=viewobj=shadowobj=localobj=groupobj=focusobj=rootobj=0;
	Renderer::reset();
	exprs.free();
	vars.free();
	exprs.parent=&vars;
	//images.free();
	Lights.free();
	DFREE(views);
	views=new FrameMgr(*objects);
	DFREE(movie);
	movie=new FrameMgr(*objects);
//	cells=0;
	cycles=0;
	bgpass=0;
	info_enabled=0;
	nstrings=0;
	self=0;
	set_sidestep(0);
	set_sidestep(1);
	view=this;
}

//-------------------------------------------------------------
// Scene::reset() reset view to standard conditions (GLOBAL)
//-------------------------------------------------------------
void Scene::init_view()
{
	Renderer::init_view();
	bgpass=0;
	size=1;
	minr=0;
	minh=10*FEET;
	maxr=size;
}

//-------------------------------------------------------------
// Scene::wrect() set display window rect
//-------------------------------------------------------------
void Scene::wrect(int l,int w ,int b,int h)
{
	aspect=(double)w/(double)h;
	getViewport(viewport);
	wscale=0.5*viewport[3]/tan(RPD*fov/2);
	//cout<<aspect<<" "<<wscale<<" "<<aspect/wscale<<endl;
}

//-------------------------------------------------------------
// Scene::save() save a scene
//-------------------------------------------------------------
void Scene::save()
{
	save(filename);
}

//-------------------------------------------------------------
// Scene::save() print a scene
//-------------------------------------------------------------
void Scene::show()
{
 	save(stdout);
}

//-------------------------------------------------------------
// Scene::save() save a scene
//-------------------------------------------------------------
void Scene::save(char *fn)
{
    FILE *fp=fopen(fn,"wb");
    if(fp){
		save(fp);
		fclose(fp);
    }
}

//-------------------------------------------------------------
// Scene::save() archive the scene
//-------------------------------------------------------------
void Scene::save(FILE *fp)
{
	views_mark();
	ViewFrame *last_view=views->at();
	ViewFrame *frame;
	last_view->save(this);

	//cout<< "save ht:"<<height/FEET<<" last:"<<last_view->point.z/FEET<<endl;

	fprintf(fp,"Scene() {\n");
	inc_tabs();
	exprs.save(fp);

	views->ss();
	frame=views->incr();
	int i=0;
	//cout<< "frame "<<i++<<" ht:"<<frame->point.z/FEET<<endl;
	views->save(fp,frame);
	frame->restore(this);
	while((frame=views->incr())){
	    views->save(fp,this,frame);
		//cout<< "frame "<<i++<<" ht:"<<frame->point.z/FEET<<endl;
		frame->restore(this);
	}
	objects->save(fp);
	dec_tabs();
	fprintf(fp,"}\n");

	views->setFrame(last_view);
	last_view->restore(this);
}

//-------------------------------------------------------------
// Scene::open() restore a scene
//-------------------------------------------------------------
void Scene::open(char *fn)
{
	timing_start=clock();
	int stat=status;
	images.clear_flags();
	Noise::resetStats();
	delete_tmpfiles();

	reset();
	init_for_open();
	model->parse(fn);
	eval_exprs();

    ViewFrame *frame;
    frame=views->first();
	if(frame){
		if(!frame->vobj && viewobj){
			frame->vobj=(ObjectNode*)views->getViewExpr(viewobj);
		}
		views->getObjects();
		frame=views->se();
		if(!viewobj)
			viewobj=frame->vobj;
		frame->restore(this);
	}
	//cout<<viewobj->typeName() << "  ht:"<<height/FEET<<" gndlvl:"<<gndlvl/FEET<<endl;

	scene_objects=total_objs();

	status=stat;
	set_changed_detail();
	set_changed_position();
	set_changed_file();

	clr_changed_view();
	clr_moved();
	clr_changed_marker();
	selobj=focusobj=0;
	objects->visitAll(&Object3D::clr_groups);
	objects->visitAll(&Object3D::init);
	if(viewobj){
	    radius=height+gndlvl+viewobj->radius();
		viewobj->init_view();
		set_obj_path();
	}
	else{
		set_global_view();
		viewobj=0;
	}
	if(viewtype==ORBITAL){
		set_sidestep(1);
		set_vstep(1);
	}
	else{
		set_sidestep(0);
		set_vstep(1);
	}

	locate_objs();
	auto_stride();
	clr_automv();
    double tm=clock();
	//objects->visitAll(&Object3D::init);
    init_time=(double)(clock() - tm)/CLOCKS_PER_SEC;
#ifdef VERIFY_OPEN
	char path[256];
	File.makeFilePath(File.system,(char*)"verify.spx",path);
    save(path);
#endif
	clr_changed_view();
	clr_moved();
	set_changed_detail();
	Render.invalidate_textures();
	GLSLMgr::clearTexs();
	views_mark();
}

//-------------------------------------------------------------
// Scene::rebuild_all() rebuild
//-------------------------------------------------------------
void Scene::rebuild_all(){
	//Noise::resetStats();
	//timing_start=clock();

	extern void init_test_params();
	init_test_params();
	init_for_rebuild();
	Render.invalidate_textures();
	Adapt.set_maxcycles(50);
	GLSLMgr::clearTexs();
	Renderer::rebuild_all();
}

//-------------------------------------------------------------
// Scene::movie_add() add a frame
//-------------------------------------------------------------
void Scene::movie_add()
{
    movie->add(this);
}

//-------------------------------------------------------------
// Scene::movie_clip() clip views
//-------------------------------------------------------------
void Scene::movie_clip()
{
    movie->clip();
    set_frame(movie->first());
    set_moved();
}
//-------------------------------------------------------------
// Scene::movie_restart() reset views
//-------------------------------------------------------------
void Scene::movie_restart()
{
    movie->restart();
    set_frame(movie->last());
    set_moved();
}

//-------------------------------------------------------------
// Scene::movie_clr() remove current view
//-------------------------------------------------------------
void Scene::movie_clr()
{
	if(movie->size()<2)
		return;
	ViewFrame *last;
	if(movie->atend()){
		last=movie->pop();
		movie->se();
	}
	else
		last=movie->pop();
    set_frame(movie->at());
	delete last;
    set_moved();
}

//-------------------------------------------------------------
// Scene::movie_reset() reset movie
//-------------------------------------------------------------
void Scene::movie_reset()
{
	ViewFrame *last=movie->pop();
    movie->free();
	movie->add(last);
	movie->ss();
    set_frame(last);
    set_moved();
}

//-------------------------------------------------------------
// Scene::movie_stop() stop movie
//-------------------------------------------------------------
void Scene::movie_stop()
{
	if(jsave()){
		stop();
		get_movie_frames();
	}
	else
		stop();
	set_intrp(0);
}

//-------------------------------------------------------------
// Scene::movie_rewind() rewind movie
//-------------------------------------------------------------
void Scene::movie_rewind()
{
    stop();
	set_frame(movie->ss());
	set_moved();
}

//-------------------------------------------------------------
// Scene::movie_record() record flight path
//-------------------------------------------------------------
void Scene::movie_record()
{
    stop();
	movie->free();
	//movie_rewind();
	mark_start=clock();
	set_record();
}

//-------------------------------------------------------------
// Scene::movie_play() play flight path
//-------------------------------------------------------------
void Scene::movie_play()
{
	mark_start=clock();

	if(movie->atend())
		set_frame(movie->ss());
	set_moved();
	set_play();
}
//-------------------------------------------------------------
// Scene::movie_interpolate() interpolate flight path
//-------------------------------------------------------------
void Scene::movie_interpolate()
{
    set_intrp(!intrp());
}

//-------------------------------------------------------------
// Scene::movie_record_video() play flight path
//-------------------------------------------------------------
void Scene::movie_record_video()
{
	set_jplay(0);
    if(!jsave()){
    	delete_frames();
		set_jsave(1);
    }
    else
        stop();
}

//-------------------------------------------------------------
// Scene::movie_play_video() play flight path
//-------------------------------------------------------------
void Scene::movie_play_video()
{
	if(!reverse() && frame_index>=frame_files.size-1)
		frame_index=0;
	else if(reverse() && frame_index==0)
		frame_index=frame_files.size-1;

	set_jplay(1);
	set_play();
	clr_changed_render();
}

//-------------------------------------------------------------
// Scene::frames_ss() set first movie frame
//-------------------------------------------------------------
void Scene::frames_ss()
{
	frame_index=0;
	NameSym *sym=frame_files[frame_index];
	if(sym)
		showJpegFile(sym->name());
	TheScene->set_changed_movie();
}

//-------------------------------------------------------------
// Scene::frames_ss() set last movie frame
//-------------------------------------------------------------
void Scene::frames_se()
{
	frame_index=frame_files.size-1;
	NameSym *sym=frame_files[frame_index];
	if(sym)
		showJpegFile(sym->name());
	TheScene->set_changed_movie();
}

//-------------------------------------------------------------
// Scene::frames_decr() decr movie frame
//-------------------------------------------------------------
void Scene::frames_decr()
{
	if(frame_files.size==0||frame_index<1)
		return;
	frame_index--;
	NameSym *sym=frame_files[frame_index];
	if(sym)
		showJpegFile(sym->name());
	set_changed_movie();
}

//-------------------------------------------------------------
// Scene::frames_incr() incr movie frame
//-------------------------------------------------------------
void Scene::frames_incr()
{
	if(frame_files.size==0||frame_index>=frame_files.size-1)
		return;
	frame_index++;
	NameSym *sym=frame_files[frame_index];
	if(sym)
		showJpegFile(sym->name());
	set_changed_movie();
}

//-------------------------------------------------------------
// Scene::save_frame() save frame (jpeg)
//-------------------------------------------------------------
void Scene::save_frame()
{
	frame_index++;
	char dir[MAXSTR];
	if(make_movie_dir(dir,moviename)){
		char fn[MAXSTR];
		sprintf(fn,"frame_%-0.5d.jpg",frame_index);
		File.addToPath(dir,fn);
    	printf("saving %s\n",dir);
    	writeJpegFile(dir,Render.image_quality());
	}
}

//-------------------------------------------------------------
// Scene::make_movie_dir() make movie base directory
//-------------------------------------------------------------
int Scene::make_movie_dir(char *path)
{
	char basedir[MAXSTR];
	path[0]=0;
	File.getBaseDirectory(basedir);
	return File.makeSubDirectory(basedir,File.movies,path);
}

//-------------------------------------------------------------
// Scene::make_movie_dir() make movie sub directory
//-------------------------------------------------------------
int Scene::make_movie_dir(char *path,char *s)
{
	char dir[MAXSTR];
	if(!make_movie_dir(dir))
	    return 0;
	return File.makeSubDirectory(dir,moviename,path);
}
//-------------------------------------------------------------
// Scene::set_movie() set new movie name
//-------------------------------------------------------------
void Scene::set_movie(char *s)
{
    movie_stop();
	movie->free();
	strcpy(moviename,s);
	get_movie_frames();
	frame_index=0;
}

//-------------------------------------------------------------
// Scene::delete_movie_dir() delete movie directory
//-------------------------------------------------------------
void Scene::delete_movie_dir(char *s)
{
	char dir[MAXSTR];
	make_movie_dir(dir);
	File.addToPath(dir,s);
	File.deleteDirectory(dir);
}

//-------------------------------------------------------------
// Scene::delete_tmpfiles() tmp files
//-------------------------------------------------------------
void Scene::delete_tmpfiles()
{
	char dir[MAXSTR];
	FileUtil::getBitmapsDir(dir);
	File.addToPath(dir,"tmp");
	File.deleteDirectoryFiles(dir,(char*)"*.*");
	FileUtil::getShadersDir(dir);
	File.deleteDirectoryFiles(dir,(char*)"*.debug");
}
//-------------------------------------------------------------
// Scene::delete_frames() delete movie frames
//-------------------------------------------------------------
void Scene::delete_frames()
{
	char dir[MAXSTR];
	make_movie_dir(dir);
	File.addToPath(dir,moviename);
	File.deleteDirectoryFiles(dir,(char*)"*.jpg");
	frame_index=0;
	frame_files.free();
}

//-------------------------------------------------------------
// Scene::movie_save() save a movie (flight path)
//-------------------------------------------------------------
void Scene::movie_save(char *path)
{
    FILE *fp=fopen(path,"wb");
    if(!fp)
        return;

	ViewFrame *last_view=movie->first();

	fprintf(fp,"Scene() {\n");
	inc_tabs();
	exprs.save(fp);

	movie->ss();
	ViewFrame *frame=movie->incr();
	movie->save(fp,frame);
	frame->restore(this);
	while((frame=movie->incr())){
	    movie->save(fp,this,frame);
		frame->restore(this);
	}

	objects->save(fp);
	dec_tabs();
	fprintf(fp,"}\n");
	fclose(fp);

	movie->setFrame(last_view);
	last_view->restore(this);
	set_moved();
}

//-------------------------------------------------------------
// Scene::movie_save() save a movie (flight path)
//-------------------------------------------------------------
void Scene::movie_save()
{
    if(!movie->size())
        return;
    char path[MAXSTR];
	char fn[MAXSTR];

    if(!make_movie_dir(path,moviename))
        return;

	sprintf(fn,"%s.spx",moviename);
	File.addToPath(path,fn);

	movie_save(path);

}

//-------------------------------------------------------------
// Scene::movie_open() open a movie path
//-------------------------------------------------------------
void Scene::movie_open(char *path)
{
	if(!File.fileExists(path)){
	    printf("could not open path file %s\n",path);
	    return;
	}
	frame_files.free();
    movie->free();
    movie_stop();
	int stat=status;
	reset();

    set_mopen(1);
	model->parse(path);
    set_mopen(0);

    ViewFrame *frame=movie->first();
	if(!frame->vobj && viewobj)
	    frame->vobj=(ObjectNode*)movie->getViewExpr(viewobj);
	movie->getObjects();

	frame=movie->ss();

    if(!viewobj)
	    viewobj=frame->vobj;

	set_frame(frame);
	views->add(this);

	scene_objects=total_objs();

	status=stat;

	set_changed_detail();
	set_changed_position();
	set_changed_file();

	clr_changed_view();
	clr_moved();
	clr_changed_marker();
	selobj=focusobj=0;

	objects->visitAll(&Object3D::clr_groups);

	if(viewobj){
	    radius=height+gndlvl+viewobj->radius();
		viewobj->init_view();
		set_obj_path();
	}
	else{
		set_global_view();
		viewobj=0;
	}
	locate_objs();
	auto_stride();
	clr_automv();
	clr_autotm();
	objects->visitAll(&Object3D::init);
	clr_changed_view();
	clr_moved();
	set_changed_detail();
	get_movie_frames();
	frame_index=0;

	//movie_rewind();
}

//-------------------------------------------------------------
// Scene::movie_open() open a default movie path
//-------------------------------------------------------------
void Scene::movie_open()
{
	char path[MAXSTR];
	if(!make_movie_dir(path,moviename))
	    return;
	char fn[MAXSTR];
	sprintf(fn,"%s.spx",moviename);
	File.addToPath(path,fn);

	movie_open(path);
}

//-------------------------------------------------------------
// Scene::jpeg_save() save an image file
//-------------------------------------------------------------
void Scene::jpeg_save(char *s)
{
	char path[MAXSTR];
	char fn[MAXSTR];
	sprintf(fn,"%s.jpg",s);
	File.makeFilePath(File.images,fn,path);
	writeJpegFile(path,Render.image_quality());
}

//-------------------------------------------------------------
// Scene::jpeg_show() show an image file
//-------------------------------------------------------------
void Scene::jpeg_show(char *s)
{
	char path[MAXSTR];
	char fn[MAXSTR];
	sprintf(fn,"%s.jpg",s);
	File.makeFilePath(File.images,fn,path);
	showJpegFile(path);
	set_changed_render();
}

//-------------------------------------------------------------
// Scene::open() restore a scene
//-------------------------------------------------------------
void Scene::open()
{
	open(filename);
}

//-------------------------------------------------------------
// Scene::parse_node() parse a node string
//-------------------------------------------------------------
NodeIF *Scene::parse_node(NodeIF *n,char *s)
{
	return model->parse_node(n,s);
}

//-------------------------------------------------------------
// Scene::parse_node() parse a node string
//-------------------------------------------------------------
NodeIF *Scene::parse_node(char *s)
{
	return model->parse_node(0,s);
}

//-------------------------------------------------------------
// Scene::save_node() parse a node string
//-------------------------------------------------------------
void Scene::save_node(NodeIF *n,char *s){
	model->save_node(n,s);
}

//-------------------------------------------------------------
// Scene::open_node() parse a node string
//-------------------------------------------------------------
NodeIF *Scene::open_node(NodeIF *n,char *s)
{
	return model->open_node(n,s);
}

//-------------------------------------------------------------
// Scene::make() build the startup scene
//-------------------------------------------------------------
void Scene::make()
{
	model->make_libraries(0);
    char path[256]={0};
    FILE *fp;
	File.makeFilePath(File.saves,(char*)"Default.spx",path);
	fp=fopen(path, "wb");
	if(fp){
		model->make(fp);
		fclose(fp);
	}
}

//-------------------------------------------------------------
// Scene::free() free resources
//-------------------------------------------------------------
void Scene::free()
{
    extern void destroy_parser();
    destroy_parser();
    Renderer::free();
}

//-------------------------------------------------------------
// Scene::set_viewobj() set the viewobj by parsing an expression
//-------------------------------------------------------------
void Scene::set_viewobj(char *s)
{
    vobj=(ObjectNode*)s;   // called from sx.y (viewobj;)
}

//-------------------------------------------------------------
// Scene::inside_sky() return view if current view is inside sky
//-------------------------------------------------------------
bool Scene::inside_sky(){
	if(viewobj && ((Orbital*)viewobj)->inside_sky())
		return true;
	return false;
}
//-------------------------------------------------------------
// Scene::eval_view() restore a scene (called from open->yacc)
//-------------------------------------------------------------
void Scene::eval_view()
{
	TerrainData data;

	if(vars.get_local("time",data))
		time=data.s;
	if(vars.get_local("delt",data))
		delt=data.s;
	if(vars.get_local("theta",data))
		theta=data.s;
	if(vars.get_local("phi",data))
		phi=data.s;
	if(vars.get_local("heading",data))
		heading=data.s;
	if(vars.get_local("pitch",data))
		pitch=data.s;
	if(vars.get_local("fov",data))
		fov=data.s;
	if(vars.get_local("tilt",data))
		view_tilt=data.s;
	if(vars.get_local("angle",data))
		view_angle=data.s;
	if(vars.get_local("skew",data))
		view_skew=data.s;
	if(vars.get_local("gndlvl",data))
		gndlvl=data.s;
	if(vars.get_local("height",data))
		height=data.s;
	if(vars.get_local("origin",data))
		spoint=data.p;
	if(vars.get_local("speed",data)){
		speed=data.s;
		//vstride=gstride=speed;
	}
	ViewFrame *frame;
	if(mopen())
	    frame=movie->add(this);
	else{
	    frame=views->add(this);
		if(vars.get_local("message",data))
			frame->log_message(data.string);
	}

	vars.free();
}
//-------------------------------------------------------------
// Scene::move_selected() jump to selection point
//-------------------------------------------------------------
void Scene::move_selected()
{
	if(!focusobj)
	    return;

	vobj=viewobj;

    if(focusobj==selobj){
		if(focusobj==viewobj){
    		if(changed_marker())
    			views->add(this);
	    	clr_changed_view();
	    	clr_changed_position();
	    	focusobj->move_focus(selm);
	    	views->add(this);
		}
		else
	    	change_view(ORBITAL);
    }
    else
        focusobj->move_focus(selm);
    clr_changed_marker();
}

//-------------------------------------------------------------
// Scene::change_view() change the viewtype from GUI
//-------------------------------------------------------------
void Scene::change_view(int v)
{
//	if(focusobj && focusobj->type()==ID_GALAXY){
//		v=GLOBAL;
//	}
    //if(viewtype!=v){
        set_changed_view();
        reset_stride();
    //}
    //else
    //    clr_changed_view();

	if(changed_marker())
    	views->add(this);

	viewtype=v;

	//vobj=viewobj;

	set_changed_detail();
	if(focusobj != viewobj)
		objects->visitAll(&Object3D::clr_groups);

	switch(viewtype){
	case GLOBAL:
		pitch=0;
		heading=0;
		view_angle=0;
		view_skew=0.0;
		view_tilt=0;
		break;
	case ORBITAL:
		pitch=-90;
		view_angle=0;
	    if(changed_view())
			heading=90;
		view_skew=0.0;
		view_tilt=0;
		set_sidestep(1);
		set_vstep(1);
		break;
	case SURFACE:
	    if(changed_view())
			heading=0;
		view_angle=0.0;
		view_skew=0.0;
		pitch=0.0;
		view_tilt=-15;
	    if(changed_view())
			heading=0;
		set_vstep(1);
		set_sidestep(0);
		break;
	}
	if(focusobj){
		if(!changed_view() && focusobj == viewobj){
			set_changed_position();
			focusobj->move_focus(selm);
			views->add(this);
			clr_changed_marker();
			selobj=0;
			focusobj=0;
			return;
		}
		focusobj->init_view();
	    set_changed_object();
	    set_changed_position();
	    focusobj->move_focus(selm);
	    viewobj=focusobj;
	}
	else if(selobj){
		viewobj=selobj;
		viewobj->init_view();
	}
	else if(viewobj){
		if(changed_view() || viewtype != SURFACE)
		    viewobj->init_view();
		else
			height/=2;
	}
	else{
		init_view();
		set_view();
	}
	if(viewobj)
		set_obj_path();
	vobj=viewobj;
	views->add(this);
	selobj=0;
	focusobj=0;
	set_moved();
	clr_automv();
	clr_changed_marker();
	clr_changed_object();
}


//-------------------------------------------------------------
// Scene::change_view() change the viewtype from GUI
//-------------------------------------------------------------
void Scene::change_view(int v,double ht)
{
	change_view(v);
	radius+=ht-height;
	height=ht;
	auto_stride();
}

//-------------------------------------------------------------
// Scene::change_view() change the viewtype from GUI
//-------------------------------------------------------------
void Scene::change_view(int v,Point p)
{
	change_view(v);

	theta=p.x;
	phi=p.y;
	height=radius=p.z;
	set_autovh();
	auto_stride();
}

//-------------------------------------------------------------
// Scene::set_view() archive the scene
//-------------------------------------------------------------
void Scene::set_view(int vt)
{
	viewtype=vt;
}

//-------------------------------------------------------------
// Scene::reset_view() reset to standard conditions
//-------------------------------------------------------------
void Scene::reset_view()
{
    height=gndlvl=elevation=radius=0.0;
    phi=theta=0;
	heading=pitch=0;
	view_tilt=view_angle=view_skew=0.0;
}

//-------------------------------------------------------------
// Scene::add_orbital() add orbital to objects
//-------------------------------------------------------------
void Scene::add_orbital(Orbital *orb)
{
	objects->add(orb);
	orb->exprs.parent=&exprs;
}


//-------------------------------------------------------------
// Scene::view_expr() add expression to view list
//-------------------------------------------------------------
void Scene::view_expr(char *s, TNode *r)
{
	TNode  *n=vars.add_expr(s,r);
	vars.eval_node(n);
}

//-------------------------------------------------------------
// Scene::subr_expr() add subr to expr list
//-------------------------------------------------------------
TNsubr *Scene::subr_expr(char *s, TNode *r)
{
    extern TNsubr *subr_node(char *a,TNode *b);
	return subr_node(s,r);
}


//-------------------------------------------------------------
// Scene::set_viewobj() set the viewobj explicitly
//-------------------------------------------------------------
void Scene::set_viewobj(ObjectNode *obj)
{
    if(!obj){
		focusobj=selobj=viewobj=0;
    }
	else if(obj !=viewobj){
		viewobj=obj;
		set_changed_object();
	}
    ObjectNode *p=0;
    if(viewobj->type()==ID_MOON)
    	p=viewobj->getParent()->getParent();
    else if(viewobj->type()==ID_PLANET || viewobj->type()==ID_STAR)
    	p=viewobj->getParent();
    if(p && p->type()==ID_SYSTEM)
    	System::TheSystem=(System *)p;
	set_obj_path();
}

//-------------------------------------------------------------
// Scene::locate_objs() locate objects
//-------------------------------------------------------------
void Scene::locate_objs()
{
	set_view();
	objects->visit(&Object3D::locate);
}

//-------------------------------------------------------------
// Scene::set_lights() light the scene
//-------------------------------------------------------------
void Scene::set_lights()
{
	Lights.free();
    if(far_pass())
        return;
	setMatrixMode(GL_MODELVIEW);
    loadIdentity();
	objects->visit(&Object3D::set_lights);
}

//-------------------------------------------------------------
// Scene::set_matrix() archive the scene
//-------------------------------------------------------------
void Scene::set_matrix(Object3D *obj)
{
	setMatrix();
	if(obj && obj==viewobj){
	    InvModelMatrix.setValues(invViewMatrix);
	    ModelMatrix.setValues(viewMatrix);
	}
}

//-------------------------------------------------------------
// Scene::views_mark() mark current view
//-------------------------------------------------------------
void Scene::views_mark()
{
	vobj=viewobj;
    views->add(this);
}


//-------------------------------------------------------------
// Scene::has_movie_path() return 1 if path exist in movie dir
//-------------------------------------------------------------
int Scene::has_movie_path()
{
	if(movie && movie->size()>1)
		return 1;
	return 0;
}

//-------------------------------------------------------------
// Scene::get_movie_frames() return 1 if frames exist in movie dir
//-------------------------------------------------------------
int Scene::get_movie_frames()
{
	LinkedList<ModelSym*> list;
	char dir[256];
	char path[256];

	make_movie_dir(dir);
	sprintf(path,"%s%s%s",dir,File.separator,moviename);
	frame_files.free();
	File.getFileNameList(path,"*.jpg",list);
	ModelSym *sym;
	list.ss();
	while((sym=list++))
		frame_files.add(new NameSym(sym->path));
	frame_files.sort();
	list.free();

	frame_files.ss();
	//frame_index=0;
	set_changed_movie();
	return frame_files.size;
}

//-------------------------------------------------------------
// Scene::has_movie_frames() return 1 if frames exist in movie dir
//-------------------------------------------------------------
int Scene::has_movie_frames()
{
	if(frame_files.size>0)
		return 1;
	return get_movie_frames();
}

//-------------------------------------------------------------
// Scene::set_frame() restore last view
//-------------------------------------------------------------
void Scene::set_frame(ViewFrame *v)
{
    if(!v)
        return;
	v->restore(this);
	if(vobj && vobj!=viewobj){
	    viewobj=vobj;
        set_obj_path();
    }
    else if(vobj){
		elevation=height+gndlvl;
		radius=viewobj->radius()+elevation;
		clr_moved();
		set_changed_detail();
		set_changed_time();
		//set_moved();
		printf("view radius %g ht %g gndlvl %g time:%g\n",
		   (radius-viewobj->radius())/FEET,height/FEET,gndlvl/FEET,v->time);
    }
	else
		set_changed_detail();
}
//-------------------------------------------------------------
// Scene::record_movie_from_path() make image files from path
//-------------------------------------------------------------
void Scene::record_movie_from_path() {
	if(movie->size()==0)
		movie_open();
	else{
		movie_rewind();
		movie_save();
	}
	set_intrp(1);
	movie_record_video();
	movie_play();
}

//-------------------------------------------------------------
// Scene::views_clr() remove current view
//-------------------------------------------------------------
void Scene::views_clr()
{
	if(views->size()<2)
		return;
	ViewFrame *last;
	if(views->atend()){
		last=views->pop();
		views->se();
	}
	else{
		last=views->pop();
	}
	set_frame(views->at());
	delete last;
    set_moved();
}

//-------------------------------------------------------------
// Scene::views_reset() reset views
//-------------------------------------------------------------
void Scene::views_reset()
{
	views->add(this);
	ViewFrame *last=views->pop();
    views->free();
	views->add(last);
	views->ss();
    set_frame(last);
    set_moved();
}

//-------------------------------------------------------------
// Scene::views_clip() reset views
//-------------------------------------------------------------
void Scene::views_clip()
{
    views->clip();
    set_frame(views->last());
    set_moved();
}

//-------------------------------------------------------------
// Scene::views_restart() reset views
//-------------------------------------------------------------
void Scene::views_restart()
{
    if(views->size()<2 || views->atstart())
       return;
    views->restart();
    set_frame(views->last());
    set_moved();
}

//-------------------------------------------------------------
// Scene::views_rewind() set views to start
//-------------------------------------------------------------
void Scene::views_rewind()
{
    set_moved();
    set_frame(views->ss());
}

//-------------------------------------------------------------
// Scene::views_incr() incr view
//-------------------------------------------------------------
void Scene::views_incr()
{
     set_moved();
   if(views->size()<=1 || views->atend())
       return;
    ViewFrame *frame=views->incr();
    frame=views->at();
    if(frame)
       set_frame(frame);
}

//-------------------------------------------------------------
// Scene::views_incr() decr view
//-------------------------------------------------------------
void Scene::views_decr()
{
    set_moved();
    if(views->size()<=1 || views->atstart())
       return;
    ViewFrame *frame=views->decr();
    frame=views->at();
    if(frame)
    	set_frame(frame);
}

//-------------------------------------------------------------
// Scene::views_undo() restore last view
//-------------------------------------------------------------
void Scene::views_undo()
{
    set_moved();
    if(views->size()<=1)
       return;
    ViewFrame *last=views->pop();
    delete last;
    last=views->last();
    set_frame(last);
}

//-------------------------------------------------------------
// Scene::select() return the object selected at point(x,y)
//-------------------------------------------------------------
void Scene::select(int x, int y)
{
    selx=x;
    sely=y;
    self=1;
    selp=Point2D(x,y);
}

//-------------------------------------------------------------
// Scene::select() return the object selected at point(x,y)
//-------------------------------------------------------------
void Scene::select()
{
	self=0;
	selobj=0;
	focusobj=0;

	void *obj=0;

	if(surface_view()){
#define USE_IDS // fetch selected node from last idtable image
#ifdef USE_IDS
		if(Raster.idvalues()){
		    viewobj->set_geometry();
			MapNode *n=Raster.pixelID(selx,sely);
			if(n){
				selobj=focusobj=viewobj;
				selm=Point(n->theta(),n->phi(),n->max_height());
		        return;
			}
		}
#endif
	    bgpass=FG0;
	    set_select_node();
	    pass_group();

	    obj=select_pass();
        if(obj){
           selobj=focusobj=viewobj;
           selm=viewobj->get_focus(obj);
           return;
        }
	}
	set_select_object();
	Raster.reset_idtbl();

	bgpass=BG1;
	pass_group();
	// for orbitals will be id of ObjectNode* in Raster.idtbl
	int id=(int)select_pass();
	focusobj=(ObjectNode*)Raster.get_data(id);

	if(focusobj){
	    if(focusobj==viewobj){
			bgpass=FG0;
			set_select_node();
			pass_group();
			obj=select_pass();
	    	if(obj)
            	selm=focusobj->get_focus(obj);
        }
        else
            selm=focusobj->get_focus(0);
	    selobj=focusobj;
	    return;
	}
	//if(local_view())
	//	return;
	bgpass=BGFAR;
	// star pass
	Raster.reset_idtbl();

	set_select_object();
	scale(minz, size);
	set_view();
	pass_init();
	set_clrpass(0);
	pass_group();
    focusobj=0;
	if(passlist.size){
		focusobj=(ObjectNode*)passlist.first();
		set_select_node();
		id=(int)select_pass();
		obj=(ObjectNode*)Raster.get_data(id);
		if(obj){
			selm=focusobj->get_focus(obj);
		}
		else
			focusobj=0;
	}
	bgpass=0;
}

//-------------------------------------------------------------
// Scene::select_pass() selection pass
//-------------------------------------------------------------
void *Scene::select_pass()
{
	GLuint selz=0xffffffff;
	GLuint selectBuff[256];
	GLuint *hit;
	GLint n=0,i;

	glSelectBuffer(256, selectBuff);

	glRenderMode(GL_SELECT);

	setMatrixMode(GL_PROJECTION);
    loadIdentity();

    int pts=8;
    if(bgpass==FG0)
       pts=1;
    else if(bgpass==BGFAR)
       pts=8;
    pickMatrix(selx,sely,pts);
	perspective(fov, aspect, znear, zfar);
	lookat(eye,center,normal);
	setMatrixMode(GL_MODELVIEW);
    loadIdentity();

	glInitNames();
	glPushName(0);

	int rsave=Render.mode();
	Render.show_zvals();

	objects->visit(&Object3D::select);
	Render.set_mode(rsave);

	n = glRenderMode(GL_RENDER);
	glPopName();

	void *obj=0;

	for(hit=selectBuff,i=0;i<n;i++,hit+=4){
		if(i==0 || hit[1]<(GLuint)selz){
			selz=hit[1];
			obj=(void*)hit[3];
		}
	}
	//cout << "hits="<<n<<" obj:"<<obj<<endl;
    return obj;
}

//-------------------------------------------------------------
// show_log_info() show log message
//-------------------------------------------------------------
void Scene::show_log_info()
{
	char msg[1024];
	ViewFrame *v=views->at();
	if(!v || !v->message())
		return;
	strncpy(msg,v->message(),1023);
	if(showlog() && msg && function()==VIEWS){
 		char *line = strtok(msg,"\n\r");
		while (line != NULL){
   			draw_string(HIST_COLOR,"%s",line);
			line = strtok(NULL,"\n\r");
		}
	}
}

//-------------------------------------------------------------
// show_select_info() show selection message
//-------------------------------------------------------------
void Scene::show_select_info()
{
    if(!focusobj)
	    set_istring(0);
    else{
        focusobj->set_focus(selm);
    }
}

//-------------------------------------------------------------
// show_view_info() show view message
//-------------------------------------------------------------
void Scene::show_view_info()
{
    if(!viewobj || viewtype!=SURFACE)
	    set_vstring(0);
    else
    	viewobj->set_view_info();
}

//-------------------------------------------------------------
// Scene::animate() do all animation tasks
//-------------------------------------------------------------
void Scene::animate()
{
	if(movie_mode()){
	    if(recording()){
			View::animate();
			vobj=viewobj;
			movie->add(this);
	    	return;
	    }
	    else if(playing()&&jplay()){
			if(reverse()){
				if(frame_index>=1){
					frame_index--;
					NameSym *sym=frame_files[frame_index];
					if(sym)
						showJpegFile(sym->name());
				}
				else
					pause();
			}
			else{
				if(frame_index<frame_files.size-1){
					frame_index++;
					NameSym *sym=frame_files[frame_index];
					if(sym)
						showJpegFile(sym->name());
				}
				else
					pause();
			}
       	}
	    else if(playing() && movie->size()){
	        if(jsave() && intrp()){
		    	if(movie->atend() && inter_index==inter_frames)
	        		movie_stop();
		    	if(inter_index==inter_frames){
		    		inter_index=0;
		    		movie->incr();
       			    set_frame(movie->at());
		    	}
		    	else{
		    		ViewFrame *f1=movie->at();
		    		ViewFrame *f2=movie->next();
		    		if(f2->vobj==f1->vobj){
		    			double f=((double)inter_index)/inter_frames;
		    			ViewFrame *f3=movie->lerp(f1,f2,f);
		    			set_frame(f3);
		    			delete f3;
		    		}
		    		else{
		    		    inter_index=0;
		    		    movie->incr();
       			    	set_frame(movie->at());
		    		}
		    	}
		    	inter_index++;
		    	ftime=time;
	        }
	        else{
	        	if(movie->atend())
	        		stop();
		    	movie->incr();
       			set_frame(movie->at());
       			ftime=time;
       		}
       		return;
	    }
	}
	View::animate();
}

//-------------------------------------------------------------
// Scene::adapt() adapt the scene to current conditions
//-------------------------------------------------------------
static void init_objvis(Object3D *obj)
{
    double zn,zf;
	obj->scale(zn,zf);
}
void Scene::adapt()
{
	// adapt background objects from global perspective
	set_adapt_mode();
	cycles=0;
	erase();
	Adapt.set_more_cycles(0);
	Raster.reset();

	bgpass=BGFAR;
	scale(minz, size);
	locate_objs();

	// adapt far objects

	pass_init();
	pass_group();
	if(passobjs()){
		objects->visit(&Object3D::adapt);
		pass_reset();
	}

	// adapt system objects

	bgpass=0;
	set_view();
	scale(minz, size);
	project();

	info_enabled=0;
	locate_objs();

	// set initial object visibility codes

	objects->visit(init_objvis);
	SET_SHOWINFO;
	pass_init();
	for(bgpass=BGMAX;bgpass>=FGMAX;bgpass--){
		pass_group();
		if(passobjs()){
			project();
			objects->visit(&Object3D::adapt);
			pass_reset();
		 }
	}
	if(inside_sky())
		Raster.set_hazepass(true);
}

//-------------------------------------------------------------
// Scene::render() render the scene
//-------------------------------------------------------------
#if defined(WIN32) || defined(WIN7)
   bool swap_on_update=true;
#else
   bool swap_on_update=true;
#endif
void Scene::render()
{
    scene_rendered=0;
	if(!suspended())
	    animate();
	int update_needed=
		      changed_time()
			||changed_detail()
 			//||Adapt.more_cycles()
			||moved()
			||changed_render();

	if(!suspended() && (update_needed || self)){
		setContext();
		init_for_cycle();
		scene_rendered=1;
		double start=clock();
		nstrings=0;
		show_mode_info();
		exec_initializers();

		GLSLMgr::clearTexs();

		set_lights();
		Raster.manageBuffers();
		rendered_objects=0;

		//set_viewtype();  // set dynamic viewtype

 		if(Render.display(OBJINFO)){
 		    switch(viewtype){
 		    case SURFACE:
				draw_string(HDR1_COLOR,"------- objects <SURFACE %1.2g> ------",
				                   height/maxht);
				break;
			case ORBITAL:
			    if(maxht)
					draw_string(HDR1_COLOR,"------- objects <ORBITAL %1.2g> ------",
				                   height/maxht);
				else
					draw_string(HDR1_COLOR,"------- objects <ORBITAL > ----------");
				break;
			case GLOBAL:
				draw_string(HDR1_COLOR,"------- objects <GLOBAL>  ---------");
				break;
			}
		}
	    set_adapt_mode();
	    SET_SHOWINFO;
	    if(info_enabled)
		    draw_string(HDR1_COLOR,"------- adapt -------------------");
		if(changed_detail() || moved())
			set_action("Adapting");
		adapt();

		set_action("Rendering");
    	adapt_time=(double)(clock() - start)/CLOCKS_PER_SEC;

		rendered_objects=0;
		bgpass=0;

    	if(self){
    	    set_select_object();
    		select();
    	}
		erase();

		set_render_mode();
		set_ambient();
		adjust_grid();

		info_enabled=Render.display(OBJINFO)&&render_info;
		if(info_enabled && adapt_info)
			draw_string(HDR1_COLOR,"------- render -------------------");
	 	GLSLMgr::setFBOReset();

		render_shadows();

		glFlush();

		if(movie_mode() && jsave())
			save_frame();

		setMatrixMode(GL_PROJECTION);
        loadIdentity();
		setMatrixMode(GL_MODELVIEW);
        loadIdentity();
		show_select_info();
		show_view_info();
		show_log_info();
    	render_time=(double)(clock() - start)/CLOCKS_PER_SEC;
    	render_time-=adapt_time;
    	adapt_time+=build_time;
#ifdef DEBUG_TIMING
    	if(timing_start){
			build_time=(double)(clock() - timing_start)/CLOCKS_PER_SEC;
			printf("build tm: %-2.1f\n",build_time);
			Noise::showStats();
			timing_start=0;
    	}
        show_info(INFO_TIME);
#endif
    	build_time=0;
		exec_finishers();

		output_text(); // on-screen info

		set_action("Ready");
    	if(/*changed_detail() ||*/ automv())
    		set_moved();
    	else
    		clr_moved();
        clr_changed_flags();
        clr_changed_position();
        Render.validate_textures();
    	auto_stride();
        if(swap_on_update)
	       swap_buffers();
	}
	if(!swap_on_update) // swap on refresh
		swap_buffers();

	clr_eyeref();
	if(autotm())
		set_changed_time();
}

//-------------------------------------------------------------
// shadow_group() shadow group
//-------------------------------------------------------------
void Scene::shadow_group(int smode)
{
	set_shadows_mode();
	pass_init();
	switch(smode){
	case 1:     		// all objects (1 pass)
		bgpass=BGS;
		pass_group();
		bgpass=FGS;
		set_clrpass(1);
		pass_group();
		break;
	case 0:     		// non-near objects (2 passes)
		set_clrpass(1);
		bgpass=BGS;
		pass_group();
		if(vsobs==0)     // Don't render shadows for offscreen objects
			clr_passobjs();
		break;
	case 2:				// near objects (2 passes)
		set_clrpass(1);
		bgpass=FGS;
		pass_group();
		break;
	}
    bgpass=0;

    if(passobjs()){
		bounds=shadowobj->bounds();
	    Raster.shadow_vmin=bounds->zn;
	    Raster.shadow_vmax=bounds->zf;
	    Raster.shadow_vzn=znear;
	    Raster.shadow_vzf=zfar;
	    Raster.shadow_vsize=shadowobj->size;
	}
	set_render_mode();
}

//-------------------------------------------------------------
// Scene::render_shadows() render objects with shadows
//-------------------------------------------------------------
void Scene::render_shadows()
{
	if(!viewobj || !viewobj->allows_shadows()){
		Raster.init_render();
		render_objects();
		Raster.apply();
		return;
	}
	Raster.init_render();

    int single_pass=0;
    set_shadows_mode();
    double ht=viewobj->far_height();
    set_render_mode();
    // TODO: if single_pass can't see clouds and sky - fix this for shadows and effects
    //if(ht<height)
    //    single_pass=1;

    int bgs=Raster.shadows() && Raster.bgshadows() && !light_view() && !test_view();

    locate_objs();
	set_lights();

	bounds=viewobj->bounds();
	
	Raster.hdr_min_delta=0;
	Raster.hdr_max_delta=0;

	if(test_view()){   					 // only for viewobj
		shadow_group(2);
		setview_test();
		render_bgpass();
		Raster.apply();
		return;
	}

	pass_init();
	shadowobj=viewobj;
	if(shadowobj->parent && shadowobj->parent->allows_shadows())
		shadowobj=shadowobj->parent;
	if(shadows_mode())
		shadow_group(single_pass);

	if(passobjs() && (bgs || light_view())){

    	// render background objects with shadows

		Point p=cpoint;
		Raster.set_farview(0);
		if(shadowobj != viewobj)
		    cpoint=shadowobj->point;
		shadowobj->init_render();
		Raster.set_farview(1);
		Raster.setView();
	    if(single_pass && light_view()){
		    render_light_view();
		    cpoint=p;
		    return;
	    }
		project();
		Raster.renderBgShadows();
	    cpoint=p;
	}

 	// ----- get shadow map for near objects ----

	if(single_pass){
		//shadow_group(2);
		Raster.init_render();
	    render_objects();
	    if(bgs){
	    	Raster.applyBgShadows();// add shadows
	    }
		Raster.apply();
		return;
    }
	set_view();
	viewobj->init_render();
	shadowobj=viewobj;
	bounds=viewobj->bounds();
	if(Raster.shadows()&& Raster.twilight()){
		if(surface_view()){ // surface views
			shadow_group(2);
			Raster.set_farview(vsobs>1);
			Raster.shadow_vsize=shadowobj->size;
		}
		else{
			shadow_group(1);
			Raster.set_farview(1);
			//Raster.shadow_vsize=0.5*(zfar-znear);
			ObjectNode *sobj=(ObjectNode *)shadowobj;
			if(shadowobj->type()==ID_MOON)
				sobj=(ObjectNode *)sobj->getParent();

			Raster.shadow_vsize=znear+2*sobj->far_height();
			Raster.shadow_vmax=Raster.shadow_vsize;
		}
		Raster.init_render();
		if(light_view() && viewobj){
			viewobj->set_selected();
			render_light_view();
			Raster.apply();
			return;
		}
		project();
		if(Raster.shadows() && !test_view())
			Raster.renderFgShadows();
	}
	project();

	Raster.render();
	project();

	render_objects();
	if(shadows_mode())
		shadow_group(2);
	project();
	Raster.apply();
}

//-------------------------------------------------------------
// Scene::render_objects() render the scene objects
//-------------------------------------------------------------
void Scene::render_objects()
{
	set_render_mode();

	// render background

	bgpass=BGFAR; // stars,galaxies

	pass_init();
	pass_group();
	if(passobjs()){
		project();
		locate_objs();
		objects->visit(&Object3D::render);
		GLSLMgr::clrDepthBuffer();
	}
	pass_init();
	bgpass=0;
	locate_objs();
	set_lights();

	// render foreground

	if(viewobj)
		viewobj->init_render();
	info_enabled=Render.display(OBJINFO);

	for(bgpass=BGMAX;bgpass>=FGMAX;bgpass--){
	 	pass_group();
		if(passobjs()){
			//set_view();
			project();
			//cout << bgpass << endl;
			render_bgpass();
			//if(bgpass>FGMAX && !Raster.farview())
			if(bgpass>FGMAX)
				GLSLMgr::clrDepthBuffer();
			pass_reset();
		}
	}
}

//-------------------------------------------------------------
// Scene::render_raster()	render to generate raster image
//-------------------------------------------------------------
void Scene::render_raster()
{
	Raster.set_draw_nvis(1);
	int rsave=Render.mode();
	int msave=Render.markers();
	Render.set_markers(0);
    Render.show_raster();
	set_buffers_mode();
	//viewobj->render();
	pass_reset();
	viewobj->set_selected();
	objects->visit(&Object3D::render);

	Render.set_mode(rsave);
	Render.set_markers(msave);
	set_render_mode();
	Raster.set_draw_nvis(0);
}

//-------------------------------------------------------------
// Scene::render_zvals()	render to validate zbuffer
//-------------------------------------------------------------
void Scene::shadows_zvals()
{
	Raster.set_draw_nvis(1);
	int rsave=Render.mode();
	int msave=Render.markers();
	Render.set_markers(0);

	Render.show_szvals();
	set_buffers_mode();

	objects->visit(&Object3D::render);
	Render.set_mode(rsave);
	Render.set_markers(msave);
	//set_render_mode();
	Raster.set_draw_nvis(0);
}

//-------------------------------------------------------------
// Scene::render_normals()	render normals only
//-------------------------------------------------------------
void Scene::shadows_normals()
{
	Raster.set_draw_nvis(1);
	int rsave=Render.mode();
	int msave=Render.markers();
	Render.set_markers(0);

	Render.show_normals();
	set_buffers_mode();

	objects->visit(&Object3D::render);
	Render.set_mode(rsave);
	Render.set_markers(msave);
	//set_render_mode();
	Raster.set_draw_nvis(0);
}

//-------------------------------------------------------------
// Scene::set_view() set OpenGL view matrix
//-------------------------------------------------------------
void Scene::set_view()
{
	ObjectNode *obj;

	View::clr_autoldm();
	View::calc_offset();

	if(viewobj){
		objpath.ss();
		while((obj=objpath++))
			obj->set_ref();
		viewobj->set_view();
	}
	else if(rootobj){
		rootobj->set_ref();
	}

	View::model_ref();
	View::set_view();
	View::set_autoldm();
}

//-------------------------------------------------------------
// Scene::setview_test() set transverse view
//-------------------------------------------------------------
void Scene::setview_test()
{
	double s,d,w,t,h,a,zn,zf;
	Point e,c,n,v;

	w=bounds->width();
	h=bounds->height();
	d=bounds->depth();

    double a1=(90-view_tilt)/90;
    double a2=1-a1;
    Point vh=(Point(0,0,0)-bounds->heightEdge()).normalize();
    Point vd=bounds->depthEdge().normalize();

    v=vd*a2+(vh*a1);
    n=vd*a1+(vh*a2);

    s=bounds->extent();
    if(s<45)
        s=rampstep(0,45,view_tilt,1.2*s,1.5*s);
    else
        s=rampstep(45,90,view_tilt,1.5*s,0.5*s);

	t=45;
	s*=1.5;
    a=TheScene->aspect;

 	c=bounds->center();
	e=c+(v*s);
 	d=e.distance(c);
    zn=0.1*d;
    zf=2*d;

	perspective(t, a, zn, zf, e, c, n);
}

//-------------------------------------------------------------
// Scene::render_light_view() render from light position
//-------------------------------------------------------------
void Scene::render_light_view()
{

	if(Raster.last_light())
		Raster.set_light(Lights.size-1);
	else
		Raster.set_light(0);
	if(changed_model() || !Raster.more_views() || Raster.shadow_count==0)
		Raster.init_view();
	//cout << "light view:"<<Raster.shadow_vcnt<<endl;

	Raster.render();
	if (Render.draw_shaded())
		GLSLMgr::setFBOWritePass();
    glDisable(GL_BLEND);
	Raster.set_light_view();

	objects->visit(&Object3D::render);

	Raster.next_view();
}

//-------------------------------------------------------------
// Scene::set_obj_path() back traverse the object tree
//-------------------------------------------------------------
void Scene::set_obj_path()
{
	if(viewobj){
		elevation=height+gndlvl;
		radius=viewobj->radius()+elevation;
	}
	regroup();
}

//-------------------------------------------------------------
// Scene::regroup() set object groups
//-------------------------------------------------------------
void Scene::regroup()
{
	objects->visitAll(&Object3D::clr_groups);
	if(!viewobj)
		return;

	ObjectNode *obj=viewobj->parent;
	groupobj=viewobj->bgfar()?0:viewobj;
	rootobj=viewobj->bgfar()?viewobj:viewobj->parent;
	while(rootobj && !rootobj->bgfar())
		rootobj=rootobj->parent;
	objpath.reset();
	while(obj){
		if(!obj->bgfar())
			groupobj=obj;
		objpath.push(obj);
		obj=obj->parent;
	}
	localobj=shadowobj=0;
	if(groupobj){
		groupobj->visitAll(&Object3D::set_local_group);
	    if(viewobj!=groupobj){
	        localobj=viewobj;
			if(viewobj->parent && viewobj->parent->allows_selection())
		    	localobj=viewobj->parent;
		    localobj->visitAll(&Object3D::set_view_group);
			viewobj->visitAll(&Object3D::set_near_group);
		}
	}
}

//-------------------------------------------------------------
// Scene::surface_view() return 1 if surface view
//-------------------------------------------------------------
int Scene::surface_view()
{
    if(viewtype==GLOBAL || !viewobj)
        return 0;
	return (viewtype == SURFACE)?1:0;
}

//-------------------------------------------------------------
// Scene::orbital_view() return 1 if orbital view
//-------------------------------------------------------------
int Scene::orbital_view()
{
    if(viewtype==GLOBAL || !viewobj)
        return 0;
	return (viewtype == ORBITAL)?1:0;
}

//-------------------------------------------------------------
// Scene::set_viewtype() set current view type
//-------------------------------------------------------------
void Scene::set_viewtype()
{
    if(viewtype==GLOBAL || !viewobj || maxht==0)
        return;

    if(height<maxht)
    	viewtype=SURFACE;
    else
    	viewtype=ORBITAL;
}

//-------------------------------------------------------------
// Scene::set_global_view() set global view (from open)
//-------------------------------------------------------------
void Scene::set_global_view()
{
	set_moved();
	size=1000;
	minr=0.0;
	maxr=size;
	zoom=vstride=hstride=1;
	auto_stride();
}

//-------------------------------------------------------------
// Scene::findObject() find named object
//-------------------------------------------------------------
static Object3D *found_obj=0;
static char *tname=0;
static void test_name(Object3D *obj){
    char *s=obj->getName();
    if(s !=0 && (strcmp(tname,s)==0))
        found_obj=obj;
}
Object3D *Scene::findObject(char *name)
{
    tname=name;
    found_obj=0;
	TheScene->objects->visit(test_name);
	return found_obj;
}

//-------------------------------------------------------------
// pass_group() setup for pass
//-------------------------------------------------------------
static void pass_group_obj(Object3D *obj)
{
    int selected=0;
    if(TheScene->adapt_mode())
        selected=obj->adapt_pass();
    else if(TheScene->render_mode())
        selected=obj->render_pass();
    else if(TheScene->shadows_mode())
        selected=obj->shadow_pass();
    else if(TheScene->select_object()||TheScene->select_node())
        selected=obj->selection_pass();
    if(selected){
    	grps[0]=0;
        passlist.add(obj);
		double zn=lim;
		double zf=-lim;

		obj->scale(zn,zf);
		if(obj->local_group()){
		    strcat(grps,"L");
			if(obj->view_group())
		    	strcat(grps,"V");
			if(obj->near_group())
		    	strcat(grps,"N");
		}
		else
		    strcat(grps,"F");
		mind=zn<mind?zn:mind;
		maxd=zf>maxd?zf:maxd;
		double f=maxd>LY?LY:1;
 		if(info_enabled)
	    	TheScene->draw_string(DATA_COLOR,"%-10s %-3s %-4s %-12g %-12g",
				obj->name(),
				grps,
				vstg[obj->getvis()],
				zn/f,
				zf/f
				);
    }
}

void Scene::pass_group()
{
    Object3D *obj;
	objects->visit(pass_group_obj);
    if(bgpass==FGMAX && passlist.size)
        set_clrpass(1);
    if(clrpass() && passlist.size){
	    scale(mind,maxd);
		double f=maxd>LY?LY:1;
 		if(info_enabled&&info_scale)
			draw_string(DATA_COLOR,"%-10s %-3s %-4s %-12g %-12g %g",
			 "SCALE",
			 grps,
			 pstg[bgpass],
			 mind/f,
			 maxd/f,
			 maxd/mind
			 );
		mind=lim;
	 	maxd=-lim;
	 	passlist.ss();
		objects->visitAll(&Object3D::clr_selected);
		nsobs=passlist.size;
		while((obj=passlist++)){
        	obj->set_selected();
			if(obj->onscreen()){
				rendered_objects++;
        	    vsobs++;
			}
        }
        passlist.reset();
        set_passobjs();
        set_clrpass(0);
	}
}

//-------------------------------------------------------------
// pass_init() init passes
//-------------------------------------------------------------
void Scene::pass_init()
{
	pass_reset();
	nsobs=0;
	vsobs=0;
}

//-------------------------------------------------------------
// pass_reset() reset pass
//-------------------------------------------------------------
void Scene::pass_reset()
{
	set_clrpass(0);
	Raster.set_bumptexs(0);
	objects->visitAll(&Object3D::clr_selected);
    clr_passobjs();
	mind=lim;
	maxd=-lim;
}

//-------------------------------------------------------------
// Scene::render_bgpass() render objects (bgpass)
//-------------------------------------------------------------
void Scene::render_bgpass()
{
	objects->visit(&Object3D::render);
	glFlush();
}

//-------------------------------------------------------------
// Scene::set_quality() set overall render quality
//-------------------------------------------------------------
void Scene::set_quality(int q)
{
    switch(q){
    case BEST:
		cellsize=0.25*dflt_cellsize;
		break;

    case HIGH:
		cellsize=0.5*dflt_cellsize;
		break;

	case NORMAL:
		cellsize=dflt_cellsize;
		break;

	case DRAFT:
		cellsize=2*dflt_cellsize;
		break;
	}
	if(q<quality)
	    rebuild_all();
    quality=q;
	set_changed_detail();
}

//-------------------------------------------------------------
// show_mode_info() draw selection box
//-------------------------------------------------------------
void Scene::show_mode_info()
{
    Color c1=Color(1,0.2,0.2);
    Color c2=Color(0,1,0.5);

    const char *yes="[X] ";
    const char *no="[ ] ";
    char dmode='S';
	if(Render.draw_points())
		dmode='P';
	else if(Render.draw_lines())
		dmode='L';
	else if(Render.draw_solid())
		dmode='N';
	else if(Render.draw_shaded())
		dmode='S';

    int c=MENU_COLOR;
    switch(function()){
    default:
    case DFLT:
         if(help()){
            draw_string(DATA_COLOR,"[F1:All] F2:View F3:Movie F4:Stats F5:Colors F6:Test");
            draw_string(MENU_COLOR," 0: set display defaults");
            draw_string(MENU_COLOR," 1: set adapt defaults");
            draw_string(MENU_COLOR,"---- arrow keys -------------------------");
            draw_string(MENU_COLOR,"left: head left      right: head right");
            draw_string(MENU_COLOR,"  up: move forward    down: move back");
            draw_string(MENU_COLOR,"---- standard keys ----------------------");
            draw_string(MENU_COLOR," u move up             d move down");
            draw_string(MENU_COLOR," [ decrease detail     ] increase detail");
            draw_string(MENU_COLOR," , decrease speed      . increase speed ");
            draw_string(MENU_COLOR," < decrease time       > increase time (rate)");
            draw_string(MENU_COLOR," ; reset move rate     : reset time rate");
            draw_string(MENU_COLOR," R rebuild             W render water");
            draw_string(MENU_COLOR," P render points       L render lines");
            draw_string(MENU_COLOR," N render solid (ogl)  S render solid (shaders)");
            draw_string(MENU_COLOR,"---- render options ---------------------");
            draw_string(MENU_COLOR," [%c] Render mode       P:points L:lines N:triangles S:shaders",dmode);
            draw_string(MENU_COLOR," %ss shadows         %sf fog",Raster.shadows()?yes:no,Render.fog()?yes:no);
            draw_string(MENU_COLOR," %sb bumpmaps        %sx textures",Render.bumps()?yes:no,Render.textures()?yes:no);
            draw_string(MENU_COLOR," %sn smoothing       %sl lighting",Render.smooth_shading()?yes:no,Render.lighting()?yes:no);
            draw_string(MENU_COLOR," %sv ave normals     %sa antialiasing",Render.avenorms()?yes:no,Render.dealias()?yes:no);
            //draw_string(MENU_COLOR," %sw water depth     %sr water reflections",yes,yes);
           // draw_string(MENU_COLOR," %sT specular light  %si render nvis",spectex()?yes:no,Render.draw_nvis()?yes:no);
            draw_string(MENU_COLOR,"---- adapt options ---------------------");
            draw_string(MENU_COLOR," Scene detail          D:draft C:average B:better A:best");
            draw_string(MENU_COLOR," %so occlusion       %se LOD",Adapt.overlap_test()?yes:no,Adapt.lod()?yes:no);
        }
        break;
    case VIEWS:
        if(help()){
            draw_string(DATA_COLOR,"F1:All [F2:View] F3:Movie F4:Stats F5:Colors F6:Test");
            draw_string(MENU_COLOR," 1: right turn");
            draw_string(MENU_COLOR," 2: left turn");
            draw_string(MENU_COLOR," 3: look left");
            draw_string(MENU_COLOR," 4: look right");
            draw_string(MENU_COLOR," 5: look down");
            draw_string(MENU_COLOR," 6: look up");
            draw_string(MENU_COLOR," 7: tilt left");
            draw_string(MENU_COLOR," 8: tilt right");
            draw_string(MENU_COLOR," 9: overhead view");
            draw_string(MENU_COLOR," 0: surface view (reset)");
        }
        break;
    case MOVIE:
       {
        int i=movie->index()+1;
        int j=frame_index+1;
        int n=movie->size();
        int nf=inter_frames*n;
        if(help()){
            draw_string(DATA_COLOR,"F1:All F2:View [F3:Movie] F4:Stats F5:Colors F6:Test");
            draw_string(MENU_COLOR," 0: clear (all)");
            draw_string(MENU_COLOR," 1: stop");
            draw_string(MENU_COLOR,"------ path---------");
            draw_string(MENU_COLOR," 2: record");
            draw_string(MENU_COLOR," 3: rewind");
            draw_string(MENU_COLOR," 4: play");
            draw_string(MENU_COLOR," 5: save");
            draw_string(MENU_COLOR," 6: load");
            draw_string(MENU_COLOR,"------ movie -------");
            draw_string(MENU_COLOR," 7: record (from path)");
            draw_string(MENU_COLOR," 8: record (live)");
            draw_string(MENU_COLOR," 9: test");
        }
        if(recording())
            draw_string(RECD_COLOR,"Movie<%s> status: Recording path %d",moviename,i);
        else if(playing()){
            if(intrp()){
                if(jsave())
                    draw_string(RECD_COLOR,"Movie<%s> status: Recording video %d:%d frame %d:%d",moviename,i,n,j,nf);
                else
                    draw_string(PLAY_COLOR,"Movie<%s> status: Replaying path %d:%d",moviename,i,n);
            }
            else{
                if(jsave())
                    draw_string(RECD_COLOR,"Movie<%s> status: Recording video %d:%d frame %d:%d",moviename,i,n,j,n);
                else
                    draw_string(PLAY_COLOR,"Movie<%s> status: Replaying path %d:%d",moviename,i,n);
            }
        }
        else if(jsave())
            draw_string(RECD_COLOR,"Movie<%s> status: Recording video frame %d",moviename,j);
        else
            draw_string(PLAY_COLOR,"Movie<%s> status: Stopped %d:%d",moviename,i,n);
        }
        break;
    case INFO:
        if(help()){
            draw_string(DATA_COLOR,"F1:All F2:View F3:Movie [F4:Stats] F5:Colors F6:Test");
            draw_string(MENU_COLOR," 0: clear all");
            draw_string(MENU_COLOR," 1: objects");
            draw_string(MENU_COLOR," 2: maps");
            draw_string(MENU_COLOR," 3: cells");
            draw_string(MENU_COLOR," 4: raster");
            draw_string(MENU_COLOR," 5: stars");
            draw_string(MENU_COLOR," 6: placements");
            draw_string(MENU_COLOR," 7: terrain");
        }
        break;
    case NCOLS:
        if(help()){
            draw_string(DATA_COLOR,"F1:All F2:View F3:Movie F4:Stats [F5:Colors] F6:Test");
            draw_string(MENU_COLOR," 0: normal colors");
            draw_string(MENU_COLOR," 1: node size");
            draw_string(MENU_COLOR," 2: node type");
            draw_string(MENU_COLOR," 3: node visibility");
            draw_string(MENU_COLOR," 4: node layers");
            draw_string(MENU_COLOR," 5: node edges");
            draw_string(MENU_COLOR," 6: node margins");
            draw_string(MENU_COLOR," 7: node tests");
        }
        break;

    case TEST:
        if(help()){
            draw_string(DATA_COLOR,"F1:All F2:View F3:Movie F4:Stats F5:Colors [F6:Test]");
            draw_string(MENU_COLOR," 0: normal view");
            draw_string(MENU_COLOR," 1: laboratory view");
            draw_string(MENU_COLOR," 2: light source view");
        }
        break;
    }
}

