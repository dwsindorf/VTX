#include "PreviewRenderer.h"
#include "SceneClass.h"
#include "TerrainClass.h"
#include "MapNode.h"
#include "NoiseClass.h"
#include "UniverseModel.h"
#include "AdaptOptions.h"
#include "Effects.h"

extern double	Hscale,Gscale;
#define NO_OVL
Renderer *renderer=0;
static 	TerrainData Td;
//************************************************************
// PreviewMap class
//************************************************************
#define TEST_STRINGS

//-------------------------------------------------------------
// PreviewMap::set_resolution() adjust cell size 
//-------------------------------------------------------------
void  PreviewMap::set_resolution()				    
{
	/*
    Point p=renderer->cpoint-renderer->epoint;
    Point v=object->point-renderer->epoint;
    double dp=v.dot(p)/p.length()/v.length();
    double d=rampstep(0,1,fabs(dp),1,0.5);
	*/
	resolution=object->resolution();
}

//-------------------------------------------------------------
// PreviewMap::set_scene()	 set near/far clipping planes, horizon etc.
//-------------------------------------------------------------
void PreviewMap::set_scene()
{
	double gndlvl=0,ht,horizon=0;
	double d,r;	
	make_current();
	gndlvl=TheScene->gndlvl;
    if(renderer->moved() || need_adapt()){ 
		gndlvl=elevation(renderer->theta,renderer->phi);
	}

	ht=renderer->radius-(radius+gndlvl);
	if(ht<renderer->minh)   // prevent underground views 
		ht=renderer->minh;

	renderer->radius=radius+gndlvl+ht;
	renderer->minr=radius+gndlvl+renderer->minh;

	// estimate horizon distance

	r=renderer->radius-gndlvl+50*MILES;
	d=radius;
	d=r*r-d*d;
	renderer->elevation=renderer->radius-radius;
	if(d>0.0)
		horizon=sqrt(d);
	renderer->height=ht;
	renderer->gndlvl=gndlvl;
	renderer->znear=0.05*ht;
	renderer->zfar=horizon;
	dmin=renderer->znear;
	dmax=horizon;

}

//************************************************************
// PreviewObject class
//************************************************************
PreviewObject::PreviewObject() : ObjectNode() { 
	tnode=0;
	map=0;
	size=0.004;
	set_local_group();
}

PreviewObject::~PreviewObject(){ 
	DFREE(map);

	set_preview_object(0);
}

//-------------------------------------------------------------
// PreviewObject::resolution() cell size 
//-------------------------------------------------------------
double  PreviewObject::resolution()				    
{
	double r=renderer->resolution*renderer->cellsize;
	double d=rampstep(0,90,fabs(renderer->pitch),r,sqrt(r));
	return d;
}

//-------------------------------------------------------------
// PreviewObject::render() render object
//-------------------------------------------------------------
void PreviewObject::render()
{
	if(!((PreviewRenderer*)renderer)->lighting())
		map->lighting=0;
	else
		map->lighting=1;

    TerrainMgr *mgr=getMgr();
	ExprMgr *exmgr=0;
 	TheScene->set_adapt_mode();
    mgr->set_preview(1);
	if(mgr->parent){
		exmgr=(ExprMgr*)mgr->parent;
		exmgr->set_preview(1);
		exmgr->eval();
	}
	map->radius=size;

	Hscale=map->hscale;
	Gscale=1/Hscale/size;
	mgr->init();

	glClearColor(0.0f, 0.0f,0.0f,1.0f);
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	map->adapt();
	map->adapt();
	renderer->erase();

	TheScene->set_render_mode();
	mgr->init_render();
	Td.albedo=0.4;
	Td.shine=10;
	//Raster.set_draw_nvis(1);
	map->render();
	//Raster.set_draw_nvis(0);
    mgr->set_preview(0);
	if(exmgr){
		exmgr->set_preview(0);
		exmgr->eval();
		mgr->init();
	}
}

//-------------------------------------------------------------
// PreviewObject::rebuild() make
//-------------------------------------------------------------
void PreviewObject::rebuild(){
	if(map)
	    delete map;
	map=new PreviewMap(size);
	map->object=this;
	TerrainMgr *terrain=getMgr();
	Scope *scope=CurrentScope;
	if(terrain){
		terrain->init();
		terrain->setPreviewMode();
	}
	map->make();
	CurrentScope=scope;
}

//-------------------------------------------------------------
// PreviewObject::getMgr() get Terrainmgr
//-------------------------------------------------------------
TerrainMgr *PreviewObject::getMgr()
{
	NodeIF *obj=tnode;
	if(!obj)
		return 0;
	if(obj->typeValue() & ID_SPHEROID)
		return &((Spheroid*)obj)->terrain;
	while(obj && !(obj->typeValue() & ID_TMGR)){
		obj=obj->getParent();
	}
	if(!obj || !(obj->typeValue() & ID_TMGR))
		return 0;
	return (TerrainMgr*)obj;
}

//-------------------------------------------------------------
// PreviewObject::set_objects() set object
//-------------------------------------------------------------
void PreviewObject::set_preview_object(NodeIF *n)
{
	if(tnode){
		if(tnode->typeValue() & ID_SPHEROID)
			delete (Spheroid*)tnode;
		else 
			delete (TNode*)tnode;
	}
	tnode=n;
}
//-------------------------------------------------------------
// PreviewObject::set_objects() set object
//-------------------------------------------------------------
NodeIF *PreviewObject::get_preview_object()
{
	return tnode;
}

//-------------------------------------------------------------
// PreviewObject::set_surface() set surface
//-------------------------------------------------------------
void PreviewObject::set_surface(TerrainData &d) 	{
	TerrainMgr *terrain=getMgr();
	if(terrain)
	terrain->set_surface(d);
} 

//************************************************************
// PreviewRenderer class
//************************************************************
PreviewRenderer::PreviewRenderer(): Renderer() { 
	backcolor=Color(0,0.7,1);
	object=new PreviewObject();
	size=0.001;
	set_object_size(0.005);
	zoom=0.5;
	minh=200*FEET;
	radius=1.2*size;
	znear=minh;
	zfar=maxr;
	view_tilt=0;
	pitch=-90;
	view_angle=0;
	heading=90;
	rflags=DEFAULTS;
	//resolution=3;
	objects->add(object);
	original_node=0;
	root=0;

	lpoint=Point(-100,0,100);
	renderer=this;
	set_sidestep(1);
	viewtype=ORBITAL;
	set_show_istring(0);
	set_quality(DRAFT);
}

PreviewRenderer::~PreviewRenderer()
{ 
	Renderer::free();
}
//-------------------------------------------------------------
// PreviewRenderer::set_object_size() set object size
//-------------------------------------------------------------
void PreviewRenderer::set_object_size(double f)
{
	if(f!=size){
		object->size=f;
		double dr=f-size;
		gndlvl+=dr;
		radius*=f/size;
		size=f;
		minh=200*FEET;
		minr=size;
		maxr=5*size;
		vstride=hstride=0.5*size;
		znear=minh;
		zfar=maxr;
	}
}

//-------------------------------------------------------------
// PreviewRenderer::set_preview_object() set object node
//-------------------------------------------------------------
int PreviewRenderer::set_preview_object(NodeIF *n)
{
	set_changed_map(1);
	if(!n || n==object->get_preview_object())
		return 0;

	Object3D *obj=0;
	object->set_preview_object(n);
	if(n->typeValue() & ID_SPHEROID)
		obj=(Object3D*)n;
	else 
		obj=((TNode*)n)->getObject();
	if(obj)
		set_object_size(obj->size);

	original_node=0;
	TerrainMgr *mgr=object->getMgr();

	if(!mgr ){
		object->set_preview_object(0);
		return 0;
	}
    root=mgr->get_root();
	if(!root){
		object->set_preview_object(0);
		return 0;
	}
	NodeIF *node=root->getValue();
	if(!node){
		object->set_preview_object(0);
		return 0;
	}
	original_node=node;
	gndlvl=0;
	init_view();	
	return 1;
}

//-------------------------------------------------------------
// PreviewRenderer::get_preview_object() get object node
//-------------------------------------------------------------
NodeIF *PreviewRenderer::get_preview_object(){
	return object->get_preview_object();
}

//-------------------------------------------------------------
// PreviewRenderer::save_state() save scene state
//-------------------------------------------------------------
void PreviewRenderer::save_state(){
	last_view->reset(TheScene);
	for(int i=0;i<4;i++)
		last_view->viewport[i]=TheScene->viewport[i];
	last_view->aspect=TheScene->aspect;
	last_view->wscale=TheScene->wscale;
	last_view->znear=TheScene->znear;
	last_view->zfar=TheScene->zfar;
	last_view->minr=TheScene->minr;
	last_view->maxr=TheScene->maxr;
	last_view->minh=TheScene->minh;
	last_view->viewtype=TheScene->viewtype;
}
static int overlap;
//-------------------------------------------------------------
// PreviewRenderer::set_state() set scene state
//-------------------------------------------------------------
void PreviewRenderer::set_state(){
	TheScene->View::reset(this);
	for(int i=0;i<4;i++)
		TheScene->viewport[i]=viewport[i];
	TheScene->aspect=aspect;
	TheScene->wscale=wscale;
	TheScene->znear=znear;
	TheScene->zfar=zfar;
	TheScene->minr=minr;
	TheScene->maxr=maxr;
	TheScene->minh=minh;
	TheScene->viewtype=viewtype;
#ifdef NO_OVL
	overlap=Adapt.overlap_test();
	//lod=Adapt.lod();
	//Adapt.set_lod(0);
	Adapt.set_overlap_test(0);
#endif
	vobj=TheScene->viewobj;

	TheScene->viewobj=object;
}

//-------------------------------------------------------------
// PreviewRenderer::restore_state() restore scene state
//-------------------------------------------------------------
void PreviewRenderer::restore_state(){
	TheScene->View::reset(last_view);
	for(int i=0;i<4;i++)
		TheScene->viewport[i]=last_view->viewport[i];
	TheScene->aspect=last_view->aspect;
	TheScene->wscale=last_view->wscale;
	TheScene->znear=last_view->znear;
	TheScene->zfar=last_view->zfar;
	TheScene->maxr=last_view->maxr;
	TheScene->minr=last_view->minr;
	TheScene->minh=last_view->minh;
	TheScene->viewtype=last_view->viewtype;
#ifdef NO_OVL
	//Adapt.set_lod(lod);
	Adapt.set_overlap_test(overlap);
#endif
	TheScene->viewobj=vobj;
}

//-------------------------------------------------------------
// PreviewRenderer::render() render
//-------------------------------------------------------------
void PreviewRenderer::render()
{
	NodeIF *node=object->get_preview_object();
	TerrainMgr *mgr=object->getMgr();
	static int rcount=0;

	if(!mgr)
		return;
	TNode    *mroot=mgr->get_root();

	if(!node || !original_node || !mroot)
		return;

	save_state();
	View *view=TheScene->view;
	
	set_state();
	TheScene->view=this;
	int update_needed=
			changed_detail()
			||moved()
			||changed_render()
			||changed_map()
			||changed_view()
			||rcount<3;
			;
	if(update_needed){
		setContext();

		rcount++;
		double start=clock();
		mroot->setValue(node);
		
		setMatrixMode(GL_MODELVIEW);
		loadIdentity();
		nstrings=0;

		TheScene->calc_offset();
		TheScene->model_ref();

		auto_stride();
		animate();
		auto_stride();

		setMatrixMode(GL_MODELVIEW);
		loadIdentity();

		View::set_view();
		project();
		TheScene->set_matrix(object);

		set_light();

		if(!object->map || changed_map()){
			object->rebuild();
			set_changed_map(0);
		}

		View::set_view();
		project();
		TheScene->set_matrix(object);
		Raster.init_lights(lighting());

		object->render();
		glFlush();
		double render_time=(double)(clock() - start)/CLOCKS_PER_SEC;
		if(show_istring()){
			draw_string(DATA_COLOR,"time %-2.1f ms cells %d",render_time*1000,object->map->size);
			output_strings();
		}
		glFlush();
		clr_moved();
		clr_changed_detail();
		clr_changed_view();
		clr_changed_flags();
	}
	swap_buffers();

	mroot->setValue(original_node); // restore old value;
	restore_state();
	TheScene->view=view;
}


//-------------------------------------------------------------
// PreviewRenderer::set_oblique_view() set oblique view
//-------------------------------------------------------------
void PreviewRenderer::set_oblique_view()	{ 
	double ht=radius-size;
	view_tilt=rampstep(0,size,ht,-15,0);
	pitch=rampstep(0,size,ht,0,-90);
	set_changed_view();
	set_moved();
}
//-------------------------------------------------------------
// PreviewRenderer::set_overhead_view() set overhead view
//-------------------------------------------------------------
void PreviewRenderer::set_overhead_view()	{ 
	view_tilt=0;
	pitch=-90;
	heading=90;
	set_sidestep(1);
	set_changed_view();
	set_moved();
}

//-------------------------------------------------------------
// PreviewRenderer::init_view() set znear,zfar etc
//-------------------------------------------------------------
void PreviewRenderer::init_view()
{
	double ht,horizon=0;
	double d,r;	
	ht=radius-(size+gndlvl);
	if(ht<minh)   // prevent underground views 
		ht=minh;

	radius=size+gndlvl+ht;
	minr=size+gndlvl+minh;
	r=radius-gndlvl+50*MILES;
	d=size;
	d=r*r-d*d;
	elevation=radius-size;
	if(d>0.0)
		horizon=sqrt(d);
	height=ht;
	znear=0.25*ht;
	zfar=horizon;
	auto_stride();
}
//-------------------------------------------------------------
// Scene::change_view() change the viewtype from GUI
//-------------------------------------------------------------
void PreviewRenderer::change_view(int v,double ht)
{
	viewtype=v;
	if(v==SURFACE){
		view_tilt=rampstep(0,size,ht,-15,0);
		pitch=rampstep(0,size,ht,0,-90);
	}
	else{
		view_tilt=0;
		pitch=-90;
	}
	set_changed_view();
	set_moved();
	radius=size+ht;
	auto_stride();
	//if(object->map)
	//	object->map->set_scene();
	clr_automv();
	init_view();
	//set_changed_map(1);
}

//-------------------------------------------------------------
// PreviewRenderer::set_light() set light
//-------------------------------------------------------------
void PreviewRenderer::set_light(){

	Lights.free();

	Light *l=Lights.newLight();

	if(!l)
		return;
	if(lighting())
		l->enable();		
	l->setPosition(lpoint);
	l->defaultLighting();
	Raster.set_light(0);
}

//-------------------------------------------------------------
// PreviewRenderer::set_quality() set overall render quality
//-------------------------------------------------------------
void PreviewRenderer::set_quality(int q)
{
    switch(q){
    case HIGH:
		resolution=1;
		break;		
	case NORMAL:
		resolution=3;
		break;		
	case DRAFT:
		resolution=5;
		break;
	}
    quality=q;
	set_changed_map(1);
	set_changed_detail();
}
