#include "SceneClass.h"
#include "ViewClass.h"
#include "MapClass.h"
#include "MapLink.h"
#include "MapNode.h"
#include "AdaptOptions.h"
#include "RenderOptions.h"
#include "matrix.h"
#include "Effects.h"
#include "GLSLMgr.h"
#include "FileUtil.h"
#include "TerrainClass.h"
static bool debug_call_lists=false;


#define DEBUG_TRIANGLES 0
//#define DEBUG_RENDER
#define DRAW_VIS (!Render.draw_nvis()&&!Raster.draw_nvis())

#define RENDERLIST(i,j,func) \
	if(use_call_lists && tid_lists[i]){ \
		if(callLists[i][j]==0) { \
			callLists[i][j]=glGenLists(1); \
			if(debug_call_lists) \
				cout << "new list ["<<i<<"]["<<j<<"] id:"<<callLists[i][j]<<endl;\
			glNewList(callLists[i][j], GL_COMPILE); \
			npole->func; \
			glEndList(); \
		} \
		glCallList(callLists[i][j]); \
	} \
	else \
		npole->func;

#define RENDER_TRIANGLES(i,j) \
	if(use_call_lists && tid_lists[i]){ \
		if(callLists[i][j]==0) { \
			callLists[i][j]=glGenLists(1); \
			if(debug_call_lists) \
				cout << "new list ["<<i<<"]["<<j<<"] id:"<<callLists[i][j]<<endl;\
			glNewList(callLists[i][j], GL_COMPILE); \
			render_triangle_list(); \
			glEndList(); \
		} \
		glCallList(callLists[i][j]); \
	} \
	else \
		render_triangles();

//**************** extern API area ************************

int hits,misses,visits;
int             recalced=0;
int      		tcreated = 0, tdeleted = 0, tcount=0;
int 			tscnt=0;

extern void     set_info(char *c,...);
extern void     init_tables();

Map            *TheMap = 0;
double          Rscale, Gscale, Pscale, Hscale,Fscale;
double          ptable[PLVLS];
extern double 	Theta, Phi, Height,MinHt,MaxHt;
extern Point	MapPt;
extern int 		test2;

static TerrainData Td;
static int      pcreated = 0, pdeleted = 0, pcount=0;
static int      vis = 0, invis=0, pvis=0, mcycles=0;
static LPoint   lp;
static int		total_nodes=0,total_wnodes=0,total_dnodes=0,total_rpasses=0,total_tpasses=0;
static int		total_dbytes=0,total_cnodes=0,total_ndata=0,total_links=0;
static int		map_nodes=0,map_vnodes=0,map_pvnodes=0,map_nvnodes=0;
static int 		vcount;

extern int 		rcycles;

static void terrain_string(TerrainProperties *t)
{
   	TheScene->draw_string(DATA_COLOR,"%-4d %-4d %-8d",
		t->type(),
		t->textures.size,
		TheMap->idcnts[t->type()]
   		);
}
static void show_terrain_info()
{
	if(!Render.display(TRNINFO))
		return;
	TheScene->draw_string(HDR1_COLOR,"------- terrain info ---------------------------");
	TheScene->draw_string(HDR1_COLOR,"id  texs vis");
	TheScene->draw_string(HDR1_COLOR,"-----------------------------------------------");
	for(int i=0;i<Td.properties.size;i++){
		terrain_string(Td.properties[i]);
	}
}

//#define DEBUG_INFO
static void map_info(MapNode *n)
{
	total_nodes++;
	MapData *d=&n->data;
	map_nodes++;
	if(n->visible())
		map_vnodes++;
	else if(n->partvis())
		map_pvnodes++;
	else
		map_nvnodes++;

	while(d){
		if(d->water())
			total_wnodes++;
		else if(d->data2())
			total_dnodes++;
		total_ndata++;
		total_dbytes+=d->memsize();
		d=d->data2();
	}
	d=n->cdata;
	if(d){
		total_cnodes++;
		while(d){
			if(d->water())
				total_wnodes++;
			else if(d->data2())
				total_dnodes++;
			total_ndata++;
			total_dbytes+=d->memsize();
			d=d->data2();
		}
	}
}

static void init_map_specs()
{
	rcycles=0;
    total_nodes=total_wnodes=total_ndata=total_dnodes=total_dbytes=total_links=0;
	total_rpasses=total_tpasses=total_cnodes=0;
	pcreated=pdeleted=pcount=0;
	vis=invis=pvis=0;
	if(!Render.display(MAPINFO))
		return;

	TheScene->draw_string(HDR1_COLOR,"------- map info ------------------------------");
	TheScene->draw_string(HDR1_COLOR,"name   cycs  ids  cells  vis   pvis  nvis  %% vis");
	TheScene->draw_string(HDR1_COLOR,"-----------------------------------------------");
}

static void map_string(Map *map)
{
   	if(map_nodes)
   	TheScene->draw_string(DATA_COLOR,"%-8s %-4d %-4d %-5.1f %-5.1f %-5.1f %-5.2f %-3.1f",
		map->object->name(),
		map->cycles,
		map->tids,
   		0.001*map_nodes,
   		0.001*map_vnodes,
   		0.001*map_pvnodes,
   		0.001*map_nvnodes,
   		map_nodes?100.0*(map_vnodes+map_pvnodes)/map_nodes:0.0
   		);
}

static void show_node_specs()
{
	if(!Render.display(NODEINFO))
		return;
	TheScene->draw_string(HDR1_COLOR,"---------------- cell data ----------------");

	TheScene->draw_string(DATA_COLOR,"new   + %-7d - %-7d cycles: %-4d",
	    pcreated,pdeleted,pcount);
	if(recalced && total_nodes)
		TheScene->draw_string(DATA_COLOR,"recalc  %-7d  %% %2.1f",recalced,(100.0*recalced)/total_nodes);
	TheScene->draw_string(DATA_COLOR,"total + %-7d - %-7d cycles: %-4d",
		tcreated,tdeleted,tcount);

	TheScene->draw_string(DATA_COLOR,"cycles: polygons %-4.1f k tids %d passes %d",
		 0.001*rcycles,
		 total_tpasses,
		 total_rpasses
		 );

	TheScene->draw_string(DATA_COLOR,"nodes(k) %-4.1f (%4.1f %%c) links %-4.1f",
		0.001*total_nodes,
		100.0*total_cnodes/total_nodes,
		0.001*total_links
		);
	TheScene->draw_string(DATA_COLOR,"dnodes(k) water %-4.1f land %-4.1f (%-1.1f ave/node)",
		 0.001*total_wnodes,
		 0.001*total_dnodes,
		 (double)total_ndata/(total_nodes+total_cnodes)
		 );
	int total_mem=total_dbytes+total_nodes*sizeof(MapNode)+total_links*sizeof(MapLink);
	TheScene->draw_string(DATA_COLOR,"mbytes total:%1.2f data:%-1.2f bytes/dnode %2.1f",
		1e-6*total_mem,
		1e-6*total_dbytes,
		((double)total_dbytes)/total_ndata
		);
	TheScene->draw_string(HDR1_COLOR,"-----------------------------------------------");
	//pcreated=pdeleted=0;
}

//************************************************************
// external API functions
//************************************************************
//-------------------------------------------------------------
// void init_test_params()	reset debug parameters
//-------------------------------------------------------------
void init_test_params()
{
	hits=misses=visits=0;
}

//-------------------------------------------------------------
// cell_size()		return cell size
//-------------------------------------------------------------
double cell_size(int i)
{
	return PI*TheMap->radius*ptable[i];
}

//************************************************************
// Map class
//************************************************************
LinkedList<Triangle*> Map::triangle_list;
bool Map::use_call_lists=true;
bool Map::use_triangle_lists=true;
int Map::tesslevel=2;
Map::Map(double r)
{
	static int init_flag=1;
	if(init_flag){
		add_initializer(init_map_specs);
		add_finisher(show_node_specs);
		init_flag=0;
	}
	TheMap=this;
	radius=r;
	dmin=0;
	dmax=0;
	texture=0;

	Point pt(0,0,1);
	pt=pt.rectangular(); // initialize sin lut
	object=0;

	init_tables();
	flags=0;
	npole=spole=last=0;
	size=links=vnodes=0;
	hscale=5e-4;  // translates local hts to scene dimensions
	hmax=-lim;
	hmin=lim;
	hrange=0;
	last=current=0;
	frontface=GL_FRONT;
	lighting=1;
	grid_=0;
	gmax=0.3;
	cmax=0.005;
	cmin=cmax/10;
	symmetry=1;
	tids=0;
	tid=0;
	minext=lim;
	ntexs=1;
    mcreated=mdeleted=mcount=cycles=0;
    for(int i=0;i<LISTTYPES;i++){
		tid_lists[i]=true;
		for(int j=0;j<MAXLISTS;j++)
			callLists[i][j]=0;
    }
    set_render_ftob(); // default ordering
}

Map::~Map()
{
    make_current();
	free();
}
//-------------------------------------------------------------
// Map::clearLists() clear call lists
//-------------------------------------------------------------
void Map::clearLists() {
	//cout << "Map::clearLists()"<<endl;
	for (int i = 0; i < LISTTYPES; i++) {
		for (int j = 0; j < MAXLISTS; j++) {
			if (callLists[i][j] > 0) {
				glDeleteLists(callLists[i][j], 1);
				if(debug_call_lists)
					cout << "del list ["<<i<<"]["<<j<<"] id:"<<callLists[i][j]<<endl;
			}
			callLists[i][j] = 0;
		}
	}
}
//-------------------------------------------------------------
// Map::clearLists() clear call lists
//-------------------------------------------------------------
void Map::enablelists(bool b){
	for(int i=0;i<LISTTYPES;i++){
		tid_lists[i]=b;
	}
}

//-------------------------------------------------------------
// void init_tables()	make static arrays
//-------------------------------------------------------------
void Map::init_tables()
{
	extern void render_setup();
	extern void render_finish();
	static int flag=0;
	double pmax;
	const double f=1.0/sqrt(2.0);
	int i;

	if(flag)
		return;

	add_initializer(render_setup);
	add_finisher(render_finish);

	pmax=1.0;
	for(i=0;i<PLVLS;i++){
		ptable[i]=pmax;
		pmax=f*pmax;
	}
	flag=1;
}

double Map::cell_size()						{ return last?last->cell_size():1;}
void Map::visit(void  (MapNode::*f)())		{ npole->visit(f);}
void Map::visit(void (*f)(MapNode*))		{ npole->visit(f);}
void Map::visit_all(void  (MapNode::*f)())	{ npole->visit_all(f);}
void Map::visit_all(void (*f)(MapNode*))	{ npole->visit_all(f);}

//-------------------------------------------------------------
// Map::locate()	return *MapNode closest to theta & phi
//-------------------------------------------------------------
MapNode *Map::locate(double t, double p)
{
	return npole->locate(t,p);
}

//-------------------------------------------------------------
// Map::elevation()	return elevation from surface
//-------------------------------------------------------------
double hts;
void heights(MapNode *n){
	double h=n->data.Ht();
	if(h>hts)
		hts=h;
}
double Map::elevation(double t, double p)
{
	hts=-lim;

	MapNode *n=npole->locate(t,p);
	if(n->cdata)
		hts=n->cdata->Ht();
	else
		hts=n->data.Ht();

	n->CWcycle(heights);
	return Rscale*hts;
}

//-------------------------------------------------------------
// Map::free()		free all allocated memory
//-------------------------------------------------------------
void Map::free()
{
	TheMap=this;

	if(npole){
		npole->free();
		delete npole;
		npole=0;
	}
	clearLists();
	triangles.free();

}

//-------------------------------------------------------------
// Map::make_current()		set global *Map to current map
//-------------------------------------------------------------
void Map::make_current()
{
	TheMap=this;
	Hscale=hscale;
	Rscale=radius*hscale; // for scaling proportional to size
	Fscale=1000*hscale; // for scaling independent of size

	Gscale=1/Rscale;
	Pscale=DPR*atan(hscale);
}
//#define DEBUG_SET_SCENE
//-------------------------------------------------------------
// Map::set_scene()	 set near/far clipping planes, horizon etc.
//-------------------------------------------------------------
void Map::set_scene()
{
	double gndlvl=0,ht=0,horizon=0,oldlvl,oldht;
	double d,r;
	make_current();
	gndlvl=oldlvl=TheScene->gndlvl;
	oldht=TheScene->height;

    if(TheScene->moved() || need_adapt()){
		gndlvl=elevation(TheScene->theta,TheScene->phi);
		TheScene->gndlvl=gndlvl;
	}

	ht=TheScene->radius-(radius+gndlvl);
	if(ht<TheScene->minh) {  // prevent underground views
		ht=TheScene->minh;
	}

	TheScene->radius=radius+gndlvl+ht;
	TheScene->minr=radius+gndlvl+TheScene->minh;

	// estimate horizon distance

	r=TheScene->radius-gndlvl+50*MILES;
	d=radius;
	d=r*r-d*d;
	TheScene->elevation=TheScene->radius-radius;
	//cout << TheScene->elevation/radius << endl;
	if(d>0.0)
		horizon=sqrt(d);

	TheScene->height=ht;
	TheScene->gndlvl=gndlvl;
	TheScene->radius=radius+gndlvl+ht;

	TheScene->znear=0.25*ht;
	//TheScene->zfar=horizon;
	TheScene->zfar=TheScene->radius;

	dmin=TheScene->znear;
	dmax=horizon;

//	if(oldlvl!=gndlvl)
//		cout << "gndlvl old:"<<oldlvl/FEET<< " new:"<<gndlvl/FEET<<endl;
//	if(oldht!=ht)
//		cout << "height old:"<<oldht/FEET<< " new:"<<ht/FEET<<endl;

	object->setvis(OUTSIDE);

#ifdef DEBUG_SET_SCENE
	static char tmp[256];
	sprintf(tmp,"radius %g ht %g glvl %g zn %g ft zf %g miles ratio:%g",
	    (TheScene->radius-radius)/FEET,ht/FEET,gndlvl/FEET,TheScene->znear/FEET,TheScene->zfar/MILES,TheScene->zfar/TheScene->znear);
	cout << tmp << endl;
#endif
}

//-------------------------------------------------------------
// Map::height()	return height closest to t,p
//-------------------------------------------------------------
double Map::height(double t, double p)
{
	return elevation(t,p)+radius;
}

//-------------------------------------------------------------
// Map::reset()	reset ogl mode_state
//-------------------------------------------------------------
void Map::reset()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
	reset_texs();
	set_textures(0);
	set_lighting(0);
}

//-------------------------------------------------------------
// Map::reset()	reset ogl state
//-------------------------------------------------------------
void Map::reset_texs()
{
	GLSLMgr::clearTexs();
	ntexs=1;

}

//-------------------------------------------------------------
// Map::make()	built initial map
//-------------------------------------------------------------
void Map::make()
{
	MapNode *p,*r,*d,*c;
	last=0;
	p=makenode(last,0.0,   90.0);
	r=makenode(last,180.0, 90.0);
	d=makenode(last,0.0,  -90.0);
	c=makenode(last,180.0,-90.0);

	p->rnode=p->lnode=r;
	p->dnode=d;
	npole=p;

	r->lnode=r->rnode=p;
	r->dnode=c;

	d->rnode=d->lnode=c;
	d->unode=p;

	c->lnode=c->rnode=d;
	c->unode=r;

	p->link=new MapLink(d,c,r);

	spole=d;
	last=0;
	MaxHt=-lim;
	MinHt=lim;
	hmax=MaxHt;
	hmin=MinHt;
}

//-------------------------------------------------------------
// Map::render()	render map (all modes)
//-------------------------------------------------------------
void Map::render()
{
	reset();
	make_current();
	if(TheScene->buffers_mode()){
		if(Render.draw_zvals())
			render_zvals();
		//else if(Render.draw_raster())
		else if(Render.draw_raster() && (object==TheScene->viewobj))
			render_raster();
		else if(Render.draw_normals())
			render_normals();
		return;
	}

	double ext=TheScene->wscale*radius/TheScene->vpoint.length();
//	if(ext<2){   // extent< 1 pixel
//		render_as_point();
//		return;
//	}
	glDisable(GL_CULL_FACE);
	if(Render.cullface() && !Render.showall()){
		glEnable(GL_CULL_FACE);
		if(Render.backface() && frontface==GL_FRONT)
			glCullFace(GL_FRONT);
		else
		if(Render.frontface() && frontface==GL_FRONT)
			glCullFace(GL_BACK);
		else
			glCullFace(GL_FRONT);
	}
	glFrontFace(GL_CW);
	set_textures(Render.textures());
	set_lighting(lighting && Render.lighting());
    if(Render.draw_lines())
		render_lines();
	else if(Render.draw_points())
		render_points();
	else if(Render.draw_solid())
		render_solid();
	else if(Render.draw_shaded())
		render_shaded();

	if(TheScene->test_view()||TheScene->light_view()){
	    if(TheScene->bounds==&vbounds)
	    	render_bounds();
	}
}

//-------------------------------------------------------------
// Map::render_raster()	render raster image
// TODO: Make this work for geometry program generated terrain
//-------------------------------------------------------------
void Map::render_raster()
{
	glPolygonMode(GL_FRONT,GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	if(frontface==GL_FRONT)
		glCullFace(GL_BACK);
	else
		glCullFace(GL_FRONT);
	glShadeModel(GL_SMOOTH);
	if(Raster.render_type()!=0)
		glDisable(GL_LIGHTING);
	else
		glEnable(GL_LIGHTING);

	glDisable(GL_DITHER);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);

	set_textures(0);
	set_lighting(0);
	set_colors(0);

	npole->init_render();

	if(Raster.surface==1 || !waterpass() || !Raster.show_water()){
#ifdef DEBUG_RENDER
		cout<< "Map::render_raster - LAND "<<object->name()<<endl;
#endif
		Raster.surface=1;
		for(tid=ID0;tid<tids;tid++){
			tp=Td.properties[tid];
			Td.tp=tp;
			if(!visid(tid))
				continue;
			RENDERLIST(RASTER_LISTS,tid,render_vertex());
		}
	}

	//if ((Raster.surface == 2 || Raster.top()) && waterpass()) {
	if (Raster.surface == 2  && waterpass() && Raster.show_water()) {
#ifdef DEBUG_RENDER
		cout<< "Map::render_raster - WATER "<<object->name()<<endl;
#endif
		Raster.surface = 2;
		tid = 0;
		RENDERLIST(RASTER_LISTS,tid,render_vertex());
	}
	//glFlush();
}

//-------------------------------------------------------------
// Map::render_raster()	render raster image
//-------------------------------------------------------------
void Map::render_normals()
{
	make_current();

	glPolygonMode(GL_FRONT,GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	if(frontface==GL_FRONT)
		glCullFace(GL_BACK);
	else
		glCullFace(GL_FRONT);
	glShadeModel(GL_SMOOTH);
	if(Raster.render_type()!=0)
		glDisable(GL_LIGHTING);
	else
		glEnable(GL_LIGHTING);

	glDisable(GL_DITHER);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);

	set_textures(0);
	set_lighting(0);
	set_colors(0);
	Render.pushmode(SHOW_NORMALS);
	npole->init_render();

	Raster.surface=1;
	for(tid=ID0;tid<tids;tid++){
		tp=Td.properties[tid];
		Td.tp=tp;
		if(!visid(tid))
			continue;
		RENDERLIST(NORMAL_LISTS,tid,render_vertex());
		 //npole->render_vertex();
	}
	if (waterpass()) {
		Raster.surface = 2;
		tid = 0;
		RENDERLIST(NORMAL_LISTS,tid,render_vertex());
		 //npole->render_vertex();
	}
	Render.popmode();
	//glFlush();
}

//-------------------------------------------------------------
// Map::render_zvals()	render without color to validate z buffer
//-------------------------------------------------------------
void Map::render_zvals()
{
	make_current();

	//glFlush();
//#ifndef WINDOWS
//    if(!mask())
//	    glDrawBuffer(GL_NONE); // this doesn't work in windows
//#endif
	int cmask[4];
	glGetIntegerv(GL_COLOR_WRITEMASK,cmask);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	if(frontface==GL_FRONT)
		glCullFace(GL_BACK);
	else if(frontface==GL_BACK)
		glCullFace(GL_FRONT);
	else
		glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_BACK,GL_FILL);

    if(!mask())	{
		glShadeModel(GL_FLAT);
		glDisable(GL_LIGHTING);
	}
	else
		glEnable(GL_LIGHTING);
	glDisable(GL_DITHER);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);

	set_textures(0);
	set_lighting(0);
	set_colors(0);
	Render.pushmode(SHOW_ZVALS);
	npole->init_render();
	texture=0;
	Raster.surface=1;

	for(tid=ID0;tid<tids;tid++){
		tp=Td.properties[tid];
		Td.tp=tp;
	    if(!visid(tid))
	        continue;
	    //setProgram();
	    //RENDERLIST(ZVAL_LISTS,tid,render_vertex());
	     npole->render_vertex();
	}
	texture=0;
	if(waterpass()){
		Raster.surface=2;  // water pass
		tid=0;
		//setProgram();
		//RENDERLIST(ZVAL_LISTS,tid,render_vertex());
		npole->render_vertex();
    }

	//glFlush();
	glColorMask(cmask[0], cmask[1], cmask[2], cmask[3]);
	Render.popmode();
//	glDrawBuffer(GL_BACK);
//#ifdef WINDOWS
//    if(!mask())
//	glClear(GL_COLOR_BUFFER_BIT);  // for windows
//#endif
}

//-------------------------------------------------------------
// Map::render_select()	select render
// TODO: better method for selection
//        - current method returns point for closest node vertex hit
//        - doesn't find intersection with triangle or hit layer
//        - try: use unproject to get 3d position of screen point
//               and zbuffer(screen point)
//-------------------------------------------------------------
void Map::render_select()
{
	make_current();

	glFlush();

	reset();
	Raster.reset_idtbl();
	/*
	glShadeModel(GL_FLAT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DITHER);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);
*/
	glDisable(GL_BLEND);
	set_textures(0);
	set_lighting(0);

	double ext=TheScene->wscale*radius/TheScene->vpoint.length();

	if(ext<4){   // small extent
		glDisable(GL_CULL_FACE);
		glPointSize(2.0f);
		Point p=npole->point();
		glBegin(GL_POINTS);
		glVertex3dv((double*)(&p));
		glEnd();
		glFlush();
		return;
	}

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	if(frontface==GL_FRONT)
		glCullFace(GL_BACK);
	else
		glCullFace(GL_FRONT);
	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_BACK,GL_FILL);
	npole->init_render();

	Raster.surface=1;
	for(tid=ID0;tid<tids;tid++){
		tp=Td.properties[tid];
		Td.tp=tp;
	    if(!visid(tid))
	        continue;
	    npole->render_vertex();
	}
	if(waterpass()){
		Raster.surface=2;  // water pass
		npole->render_vertex();
    }
	glFlush();
}

//-------------------------------------------------------------
// Map::render_as_point()	render as a single point
//-------------------------------------------------------------
void Map::render_as_point()
{
	Color c;
	Point p=TheScene->vpoint;

	double ext=TheScene->wscale*radius/p.length();

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_FOG);
	glShadeModel(GL_FLAT);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glDisable(GL_POINT_SPRITE);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, 0);

	//float f=(float)rampstep(0,1,sqrt(ext),2,5);
	float f=(float)rampstep(0,1,sqrt(ext),2,5);
//	glPointSize(10);
	glPointSize(f);
	c=object->color().lighten(0.6);
	glColor4d(c.red(),c.green(),c.blue(),1);

	p=npole->point();
	glFlush();
	glBegin(GL_POINTS);
	glVertex3d(p.x,p.y,p.z);
	glEnd();

}

//-------------------------------------------------------------
// Map::render_points()	render map  (points mode)
//-------------------------------------------------------------
void Map::render_points()
{
	glDisable(GL_LIGHTING);

	glPolygonMode(GL_FRONT,GL_POINT);
	glPolygonMode(GL_BACK,GL_POINT);

	glDisable(GL_BLEND);

	Render.show_points();
	if(Render.draw_refs())
	    render_refs();
	if(object->allows_selection() && Render.draw_bounds())
	    render_bounds();
	render_surface();

}

//-------------------------------------------------------------
// Map::render_lines()	render map  (lines mode)
//-------------------------------------------------------------
void Map::render_lines()
{
	glDisable(GL_LIGHTING);
	if(Render.smooth_shading())
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	glPolygonMode(GL_FRONT,GL_LINE);
	glPolygonMode(GL_BACK,GL_LINE);

	glDisable(GL_BLEND);

	Render.show_lines();
	if(Render.draw_refs())
	    render_refs();
	if(object->allows_selection() && Render.draw_bounds())
	    render_bounds();

	render_surface();

}

//-------------------------------------------------------------
// Map::render_solid()	render map  (surface mode)
//-------------------------------------------------------------
void Map::render_solid()
{
	if(!Render.lighting() || !lighting)
		glDisable(GL_LIGHTING);
	else
		glEnable(GL_LIGHTING);

	if(Render.smooth_shading())
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_BACK,GL_FILL);
	glEnable(GL_BLEND);

	Render.show_solid();
	if(object->allows_selection() && Render.draw_bounds())
	    render_bounds();

	render_surface();
}

//-------------------------------------------------------------
// Map::setProgram() set shader program for ids render pass
//#define USE_SHADER_ONLY_FOR_GEOM
//#define GEOM_SHADER
//-------------------------------------------------------------
bool Map::setProgram(){
	if(TheScene->viewobj != object)
		return false;
	bool geom=tp->has_geometry() && tp->tnpoint;
#ifdef USE_SHADER_ONLY_FOR_GEOM
	if(!Render.geometry() || !tp->has_geometry() || !tp->tnpoint)
		return false;
#endif
	GLSLMgr::input_type=GL_TRIANGLES;
	GLSLMgr::output_type=GL_TRIANGLE_STRIP;

	//if(Render.draw_ids())
		sprintf(GLSLMgr::defString,"#define COLOR\n");
#ifdef GEOM_SHADER
	tesslevel=0;  //  0=single triangle mode
	GLSLMgr::max_output=(tesslevel+1)*(tesslevel+3);
	sprintf(GLSLMgr::defString+strlen(GLSLMgr::defString),"#define TESSLVL %d\n",tesslevel);
#endif
	if(tp->tnpoint){
		sprintf(GLSLMgr::defString+strlen(GLSLMgr::defString),"#define NVALS %d\n",tp->noise.size);
		tp->tnpoint->snoise->initProgram();
		tp->tnpoint->initProgram();
	}
	else
		sprintf(GLSLMgr::defString+strlen(GLSLMgr::defString),"#define NVALS 0\n");

#ifdef GEOM_SHADER
	GLSLMgr::loadProgram("map.gs.vert","map.frag","map.geom");
#else
	GLSLMgr::loadProgram("map.vert","map.frag");
#endif
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;

	GLSLMgr::setProgram();
	if(tp->tnpoint){
		tp->tnpoint->snoise->setProgram();
		tp->tnpoint->setProgram();
	}

	GLSLMgr::setFBORenderPass();

	GLSLVarMgr vars;
	Point pv=TheScene->xpoint;
	vars.newFloatVec("pv",pv.x,pv.y,pv.z);
	vars.newIntVar("tessLevel",tesslevel);

	vars.setProgram(program);
	vars.loadVars();
	TheScene->setProgram();
	set_resolution();
	return true;
}

//-------------------------------------------------------------
// Map::render_ids()	render using id psuedo-colors
//-------------------------------------------------------------
void Map::render_ids()
{
	GLSLMgr::beginRender();
	//glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);

	npole->visit(&MapNode::clr_flags);
	Raster.set_idmode(1);
	int mode=Render.mode();
	Render.show_ids();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
	//reset();
	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_BACK,GL_FILL);
	glEnable(GL_CULL_FACE);
	if(frontface==GL_FRONT)
		glCullFace(GL_BACK);
	else if(frontface==GL_BACK)
		glCullFace(GL_FRONT);
	else
		glDisable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glShadeModel(GL_FLAT);

	texture=0;
	npole->init_render();

	Raster.set_all();

	Raster.surface=1;
	//set_resolution();
	//tesslevel=0; // use single triangle for id geom shader
	for(tid=ID0;tid<Td.properties.size;tid++){
		tp=Td.properties[tid];
		Td.tp=tp;
		setProgram();
	    npole->render_ids();
	}
	if(S0.get_flag(WATERFLAG)){
		Raster.surface=2;
		tid=WATER;
		tp=Td.properties[tid];
		Td.tp=tp;
		setProgram();
		npole->render_ids();
    }

	glFinish();
	glFlush();
	Raster.read_ids();
	Raster.set_idmode(0);
	Render.set_mode(mode);
	Raster.surface=1;
	//set_resolution();
	GLSLMgr::endRender();

}
//-------------------------------------------------------------
// Map::render_shaded()	render map  (shaded mode)
//-------------------------------------------------------------
void Map::render_shaded()
{

	GLSLMgr::beginRender();
	glPushAttrib(GL_CURRENT_BIT);

	if(!Render.lighting() || !lighting)
		glDisable(GL_LIGHTING);
	else
		glEnable(GL_LIGHTING);

	if(Render.smooth_shading())
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	if(Raster.lines()){
		glPolygonMode(GL_FRONT,GL_LINE);
		glPolygonMode(GL_BACK,GL_LINE);
	}
	else{
		glPolygonMode(GL_FRONT,GL_FILL);
		glPolygonMode(GL_BACK,GL_FILL);
	}
	glDisable(GL_BLEND);

	if(object->allows_selection() && Render.draw_bounds())
	    render_bounds();
	Render.show_shaded();

	glColor4d(1,1,1,1);

	Raster.surface=1;
    Raster.set_all();
	set_textures(1);

	npole->init_render();
	Lights.setAmbient(Td.ambient);
	Lights.setDiffuse(Td.diffuse);
	if(!waterpass() || !Raster.show_water() || !Render.show_water()){
#ifdef DEBUG_RENDER
		cout <<"Map::render_shaded - LAND "<<object->name()<<endl;
#endif
		for(int i=0;i<tids-1;i++){
			tid=i+ID0;
			tp=Td.properties[tid];
			Td.tp=tp;
			if(!visid(tid) || !tp)
				continue;
			total_tpasses++;
			texpass=0;
			double shine=Td.shine;
			Color specular=Td.specular;
			Lights.mixSpecular(specular);

			Lights.setShininess(shine*tp->shine);
			specular.set_alpha(specular.alpha()*tp->albedo);
			Lights.mixSpecular(specular);
			if(tp->glow.alpha())
				Lights.setEmission(tp->glow);
			else
				Lights.setEmission(Td.emission);
			glColor4d(tp->color.red(),tp->color.green(),tp->color.blue(),tp->color.alpha());
			if(!object->setProgram())
				continue;

			texture=0;
			total_rpasses++;
			if(render_triangles()){
				RENDER_TRIANGLES(SHADER_LISTS,tid);
			}
			else {
				RENDERLIST(SHADER_LISTS,tid,render());
			}

			Render.show_shaded();
			reset_texs();
		}
	}

	bool show_water=waterpass() &&  Render.show_water() && (TheScene->viewtype!=SURFACE || Raster.show_water());
	//if(!TheScene->inside_sky()&& waterpass() && Render.show_water()){
	if(show_water){
#ifdef DEBUG_RENDER
		cout <<" Map::render_shaded - WATER "<<object->name()<<endl;
#endif
		tid=WATER;
		tp=Td.properties[tid];
		Td.tp=tp;
		Raster.surface=2;
		Lights.setSpecular(Raster.water_specular);
		Lights.setShininess(Raster.water_shine);

		TerrainData::tp->set_color(true);
		object->setProgram();
		texture=0;
		total_rpasses++;
		npole->init_render();
		set_colors(1);
		set_textures(0);
		if(render_triangles()){
			RENDER_TRIANGLES(SHADER_LISTS,tid);
		}
		else {
		    RENDERLIST(SHADER_LISTS,tid,render());
		}
		Render.show_shaded();
		Raster.surface=1;
	}
	set_textures(0);
	Render.show_shaded();
	GLSLMgr::endRender();
	glPopAttrib();

//	if(TheScene->viewobj==object){
//    	double minz,maxz;
//    	cout<< "Render ";
//    	Raster.getLimits(minz,maxz);
//	}
	//cout << "Map::render time:"<< (clock()-d0)/CLOCKS_PER_SEC << " ms" <<endl;

}
//-------------------------------------------------------------
// Map::render_water()	render water (surface mode)
//-------------------------------------------------------------
void Map::render_water(){
	Raster.surface=2;
	total_tpasses++;

	Lights.setSpecular(Raster.water_specular);
	Lights.setShininess(Raster.water_shine);
	if(Render.water_lines()){
		glPolygonMode(GL_FRONT,GL_LINE);
		glPolygonMode(GL_BACK,GL_LINE);
	}
	npole->init_render();
	texture=0;
	set_colors(1);
	set_textures(0);

	texpass=0;
	total_rpasses++;
	npole->render();
}


//-------------------------------------------------------------
// Map::render_surface()	render map  (surface mode)
//-------------------------------------------------------------
void Map::render_surface()
{
	render_texs();
	if(Render.bumps() && visbumps())
		render_bumps();
	if(waterpass() && Render.show_water())
		render_water();
	Raster.surface=1;
}

//-------------------------------------------------------------
// Map::render_texs()	render textures and colors (surface mode)
//-------------------------------------------------------------
void Map::render_texs(){
	glColor4d(1,1,1,1);

	Raster.surface=1;
    Raster.set_all();

	npole->init_render();

    double shine=Td.shine;
	Color emission=Td.emission;

    Lights.setAmbient(Td.ambient);
	for(tid=ID0;tid<tids;tid++){
		tp=Td.properties[tid];
		Td.tp=tp;
	    if(!visid(tid))
	        continue;
		total_tpasses++;
		set_colors(1);
        texpass=0;
 		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);

		Lights.setShininess(shine*tp->shine);
		Color specular=Td.specular;
		specular.set_alpha(specular.alpha()*tp->albedo);
		Lights.mixSpecular(specular);

		Lights.mixDiffuse(Td.diffuse);
		double emission_value=tp->glow.alpha();
		if(emission_value)
			Lights.setEmission(tp->glow);
		else
			Lights.setEmission(emission);

		int stexs=tp->textures.size;
		texture=stexs?tp->textures[0]:0;

		// if first texture has alpha<1 need separate pass for color
		// otherwise object will be transparant

		int aflag=texture && texture->alpha_enabled() && !transparant();

        //cout<<" Map::render_texs:"<<stexs<<endl;

		if(aflag || Render.colors())
			set_textures(0);
		else
			set_textures(Render.textures());

		if(!aflag && textures() && texture && Render.textures()){
			total_rpasses++;
		    texture->begin();
			npole->render();
		}
		else{
			set_textures(0);
			total_rpasses++;
			npole->render();
		}
		set_colors(0);

		if(!Render.textures()|| !stexs)
		    continue;
		set_textures(1);
		int n=aflag?0:1;
		for(texpass=n;texpass<stexs;texpass++){
			texture=tp->textures[texpass];
			if(!texture->tex_active)
				continue;
			double scale=texture->scale;
			for(int i=0;i<(int)texture->orders;i++){
				texture->begin();
				total_rpasses++;
				npole->render();
				texture->scale*=texture->orders_delta;
			}
			texture->scale=scale;
		}
		reset_texs();
	}
}

//-------------------------------------------------------------
// Map::render_bumps()	render bumpmaps (surface mode)
//-------------------------------------------------------------
void Map::render_bumps(){

	Raster.surface=1;
    Raster.set_all();

	npole->init_render();

 	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);

	// apply bumpmap textures

    Raster.set_all();
	//Raster.set_draw_nvis(1);
	int rsave=Render.mode();

	Render.show_raster();
	TheScene->set_buffers_mode();

	glAccum(GL_LOAD,0.5f);

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_BLEND);

	set_textures(1);
	set_colors(0);
	set_lighting(0);
	glColor4d(1,1,1,1);

	Raster.set_render_type(BUMPS);
	Raster.set_bumps(1);
	Raster.surface=1;
	npole->init_render();

    //Raster.set_draw_nvis(1);

	for(tid=ID0;tid<tids;tid++){
		tp=Td.properties[tid];
		Td.tp=tp;
		if(!visid(tid))
			continue;
		total_tpasses++;
		for(texpass=0;texpass<tp->textures.size;texpass++){
			texture=tp->textures[texpass];
			if(!texture->bump_active)
				continue;
			double scale=texture->scale;
			for(int i=0;i<(int)texture->orders;i++){
				texture->begin();

				// render bump texture (unshifted)

				Raster.bmppass=1;
				total_rpasses++;
				npole->render();

				glAccum(GL_ACCUM,(float)(texture->bumpamp*Raster.bump_ampl));

				// render bump texture (shifted)

				Raster.bmppass=2;
				total_rpasses++;
				npole->render();
				glAccum(GL_ACCUM,(float)(-texture->bumpamp*Raster.bump_ampl));
				texture->scale*=texture->orders_delta;
			}
			texture->scale=scale;
		}
	}
	Raster.set_bumps(0);
	glAccum(GL_RETURN,2.0f);

	Raster.bmppass=0;
	Raster.set_render_type(0);

	Render.set_mode(rsave);
	TheScene->set_render_mode();
	//Raster.set_draw_nvis(0);

	if(Render.lighting() && lighting)
		glEnable(GL_LIGHTING);

	if(Render.smooth_shading())
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	glEnable(GL_BLEND);
}

//-------------------------------------------------------------
// Map::render_refs()	draw meridian reference lines
//-------------------------------------------------------------
void Map::render_refs()
{
	MapNode *node;
	glDisable(GL_LIGHTING);

	node=npole->locate(0.0,0.0);

	// north longitude: theta=0 to theta=180 (blue)

	glColor3d(0,0,1);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);

	while(node->unode){
		node->vertex();
		node=node->unode;
	}

	node=npole->locate(180.0,0.0);
	while(node->unode){
		node->vertex();
		node=node->unode;
	}
	glEnd();

	// north longitude: theta=0 to theta=180 (gray)

	node=npole->locate(0.0,0.0);

	glColor3d(0.5,0.5,0.5);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);

	while(node->dnode){
		node->vertex();
		node=node->dnode;
	}

	node=npole->locate(180.0,0.0);
	while(node->dnode){
		node->vertex();
		node=node->dnode;
	}
	glEnd();


	// latitude: equator  theta=0 to 180 (red)
	node=npole->locate(0.0,0.0);
	glBegin(GL_LINE_LOOP);
	glColor3d(1,0,0);
	node->vertex();
	node=node->rnode;
	while(node&& node->theta() <180.0){
		node->vertex();
		node=node->rnode;
	}
	// latitude: equator  theta=180 to 0 (yellow)
	glColor3d(1,1,0);
	while(node&& node->theta() >0.0){
		node->vertex();
		node=node->rnode;
	}
	glEnd();

	node=npole->locate(0.0,0.0);
	glColor3d(1,1,1);
	glDisable(GL_POINT_SMOOTH);

	glPointSize(4.0f);
	glBegin(GL_POINTS);
	node->vertex();
	glEnd();
	glLineWidth(1.0f);
	glPointSize(1.0f);
}

//-------------------------------------------------------------
// Map::render_bounds()	draw bounds box
//-------------------------------------------------------------
void Map::render_bounds()
{
	GLboolean lflag;
	Point p,p1,p2;

	Point *box;
	if(!vbounds.valid())
	    return;

	box=rbounds.box();

	glGetBooleanv(GL_LIGHTING,&lflag);
	glDisable(GL_LIGHTING);

	glLineWidth(2.0f);
	glColor4d(0,0,1,1);

	// draw bottom of box

	glBegin(GL_LINE_LOOP);

	glVertex3dv((double*)(box+0));
	glVertex3dv((double*)(box+1));
	glVertex3dv((double*)(box+2));
	glVertex3dv((double*)(box+3));
	glEnd();

	// draw top of box

	glColor4d(1,0,1,1);

	glBegin(GL_LINE_LOOP);

	glVertex3dv((double*)(box+4));
	glVertex3dv((double*)(box+5));
	glVertex3dv((double*)(box+6));
	glVertex3dv((double*)(box+7));
	glEnd();

	// connect top/bottom

	glBegin(GL_LINES);
	glVertex3dv((double*)(box+0));
	glVertex3dv((double*)(box+4));
	glEnd();

	glBegin(GL_LINES);
	glVertex3dv((double*)(box+1));
	glVertex3dv((double*)(box+5));
	glEnd();

	glBegin(GL_LINES);
	glVertex3dv((double*)(box+2));
	glVertex3dv((double*)(box+6));
	glEnd();

	glBegin(GL_LINES);
	glVertex3dv((double*)(box+3));
	glVertex3dv((double*)(box+7));
	glEnd();

	glColor4d(0,1,1,1);
	p2=TheScene->epoint;

    p1=TheScene->vcpoint;
    p2=TheScene->vpoint;

	glBegin(GL_LINES);
	glVertex3dv((double*)(&p1));
	glVertex3dv((double*)(&p2));
	glEnd();

	glLineWidth(1.0f);
	glPointSize(1.0f);
	if(lflag==GL_TRUE)
		glEnable(GL_LIGHTING);
}

//-------------------------------------------------------------
// Map::make_visbox()	set screen bounds to span all visible nodes
//-------------------------------------------------------------
static GLdouble vmat[16];
static GLdouble imat[16];
//-------------------------------------------------------------
static void evalbox(MapNode *n)
{
	if(n->visible()){
		Point pt=n->point();
    	Point p1=pt.mm(vmat);
    	if(p1.z<0)
    	TheMap->rbounds.eval(p1);
    }
}
//-------------------------------------------------------------
void Map::make_visbox()
{
    int i;
    Point *r,*v;
	make_current();
	//vischk();

	if(!bchecked()){
		set_bchecked();

	    // local to eye

		CMmmul(TheScene->lookMatrix,TheScene->viewMatrix,vmat,4);

	    rbounds.reset();
		npole->visit(evalbox);

	    r=rbounds.make();

	    if(idtest && TheScene->viewobj==object){
			rbounds.zn=zn;
			rbounds.zf=zf;
	    }
	    else{
			rbounds.zn=-rbounds.bmax().z;
			rbounds.zf=-rbounds.bmin().z;
	    }

		// eye to local

	    minv(vmat,imat,4);
	    for(i=0;i<rbounds.size();i++)
		    r[i]=r[i].mm(imat);

		//vbounds.zn=0.5*rbounds.zn;
	   // cout << "rbounds zn:"<<rbounds.zn/FEET<<" zf:"<<rbounds.zf/FEET<<endl;
		//if(TheScene->viewobj==object)
		//    cout << "rbounds.zn:" << rbounds.zn/FEET << " rbounds.zf:"<< rbounds.zf/FEET << " zn:"<<zn/FEET<<" zf:"<<zf/FEET<<endl;
		vbounds.zn=0.5*rbounds.zn;
		vbounds.zf=1.1*rbounds.zf;

		// quick fix for zf clipped by water surface :
		// make sure zf-zn >= max water transparency depth

		double df=(vbounds.zf-rbounds.zn)*Rscale;
		if(waterpass() && df <2000*FEET){
		    double zm=vbounds.zf+(2000*FEET-df);
		    vbounds.zf=zm;
		}
		vbounds.set_valid(1);
	}

	// local to global

	r=rbounds.box();
	v=vbounds.box();
    for(i=0;i<rbounds.size();i++)
        v[i]=r[i].mm(TheScene->viewMatrix);
}

//-------------------------------------------------------------
// Map::adapt()	adapt map to change of view
//-------------------------------------------------------------

#define ADAPT_NEEDED ( object->force_adapt() \
					|| Adapt.always() \
					|| need_adapt() \
					|| TheScene->view->changed_model() \
					|| TheScene->viewobj!=object && TheScene->view->changed_time() \
					)

#define ADAPT_COMPLETE ( converged \
				    || size>Adapt.maxcells()*1000 \
				    || (created<5 && deleted<5 && vnodes>=vmin && mcount>2) \
				    || mcount>max_cycles \
					)

#define ADAPT_CYCLE \
			mcount++; \
			created=mcreated; \
		    deleted=mdeleted; \
			npole->visit(&MapNode::set_tests); \
			cycles++; \
			npole->adapt(); \
			find_limits();\
			created=mcreated-created; \
			deleted=mdeleted-deleted; \
			pcount++; \
			pcreated+=created; \
			pdeleted+=deleted; \
			tcount++; \
			tcreated+=created; \
			tdeleted+=deleted; \
			if(created || deleted) {  \
			    clr_vchecked(); \
			    if(vnodes>=vmin) {\
					dc=(double)created/size; \
					dd=(double)deleted/size; \
					if(dc<sfact && dd<sfact) \
					    converged=1; \
				}\
			}\
			else \
				converged=1; \
			if(ADAPT_COMPLETE) \
				break;

void Map::adapt()
{
	int created=0,deleted=0,dv=0;
	double dc=0,dd=0;
	int converged=1;
	make_current();
	int max_cycles=Adapt.maxcycles();

	//if(TheScene->automv())
	//	max_cycles=9;
   if( Adapt.never())
	   return;

	mcreated=mdeleted=mcount=cycles=0;
	if(!npole)
		make();
	if(Adapt.uniform())
		set_grid(Adapt.grid_depth());
	else
		set_grid(0);

	if(!object->local_group()){
	    smax=RT2*2*PI*radius;
	    smin=smax/4;
	}
	else{
	    smax=RT2*2*PI*radius/8;
	    smin=smax/4;
	}

    int vmin=TheScene->viewobj==object?32:0;

    idtest=false;
	if(TheScene->viewobj==object && Adapt.overlap_test())
		set_idmap(1);
	else
		set_idmap(0);
	idtest=idmap();
	//double sfact=0.3*object->resolution()*TheScene->cellsize;
	double sfact=Adapt.conv()*0.5*sqrt(object->resolution()*TheScene->cellsize);
	if(TheScene->changed_file()){
		sfact*=0.5;
		max_cycles*=2;
		//cout<<"new file"<<endl;
	}
	if(ADAPT_NEEDED  || TheScene->changed_render())
		clearLists();
    if(!test2)
	if(ADAPT_NEEDED) {
		set_end_adapt(0);
		converged=0;
		Raster.set_draw_nvis(1);
		minext=lim;
		set_resolution();
		npole->visit(&MapNode::clr_flags);
		hmax=-lim;
		hmin=lim;

		if(TheScene->view->changed_detail())
			npole->visit_all(&MapNode::clr_ccheck);
		clr_vchecked();
		clr_bchecked();
		//Raster.surface=1;
		reset();
		while(1){
			if(!fixed_grid()){
			    int vn=vnodes;
			    if(idtest)
			    	glClear(GL_DEPTH_BUFFER_BIT);

			    vischk(idtest);
				dv=vnodes-vn;
				dv=dv<0?-dv:dv;
				npole->visit(&MapNode::sizechk);
				npole->visit(&MapNode::balance);
			}
			ADAPT_CYCLE; // squares pass
			ADAPT_CYCLE; // diamonds pass

			if(Adapt.show_steps())
			    break;
		}
		if(visids()){
			idtest=true;
			clr_vchecked();
		}
		if(!converged)
		    set_need_adapt();
		else
		    clr_need_adapt();
		set_end_adapt(1);
	    if(idtest)
	    	glClear(GL_DEPTH_BUFFER_BIT);

		vischk(idtest);
        if(!Adapt.edges() || !idtest)
		    npole->visit(&MapNode::pvischk);
		// TODO: read depth buffer and find zn, zf
        if(idtest && object==TheScene->viewobj){
        	Raster.getLimits(zn,zf);
        }
		if(object->allows_selection()||idtest)
			make_visbox();
		Raster.set_draw_nvis(0);
		get_info();

		if(make_lists() || render_triangles()|| Render.avenorms())
			make_triangle_lists();
		else {
			triangles.free();
		}

	}
	if(need_adapt())
		Adapt.set_more_cycles(1);
	if(TheScene->viewobj==object){
		TheScene->cycles+=mcount;
		TheScene->cycles=mcount;
		Raster.set_waterpass(waterpass());
		Raster.set_fogpass(fog());
		//cout<<cycles<<" MinHt:"<<MinHt<<" MaxHt:"<<MaxHt<<endl;
	}
	if(Render.display(MAPINFO) || Render.display(NODEINFO)){
		mcycles=mcount;
		map_nodes=map_vnodes=map_pvnodes=map_nvnodes=0;
		npole->visit(map_info);
		total_links+=links;
		if(Render.display(MAPINFO))
			map_string(this);
	}
	if(object==TheScene->viewobj && Render.display(TRNINFO)){
		show_terrain_info();
	}
	find_limits();

}
//-------------------------------------------------------------
// Map::find_limits()	get ht min and max
//-------------------------------------------------------------
void Map::find_limits(){
	npole->visit(&MapNode::find_limits);
	hrange=hmax-hmin;
	//if(TheScene->viewobj==object)
	//cout<<cycles<<" MinHt:"<<MinHt<<" MaxHt:"<<MaxHt<<endl;
}
//-------------------------------------------------------------
// Map::vischk()	test all nodes for visibility
//-------------------------------------------------------------
void Map::vischk(bool idtest)
{
	if(vchecked())
		return;

	vnodes=0;
	if(idtest){
		glClear(GL_COLOR_BUFFER_BIT);
	    Raster.reset_idtbl();
		render_ids();
		Raster.setVisibleIDs();
    	if(Adapt.edges())
			npole->visit(&MapNode::pvischk);
	}
	else{
		npole->visit(&MapNode::vischk);
	}
	set_vchecked();
	clr_bchecked();
}

//-------------------------------------------------------------
// Map::makenode()	node generation factory method
//-------------------------------------------------------------
MapNode *Map::makenode(MapNode *parent, uint t, uint p)
{
	MapNode *a;
	TheMap=this;
	int lvl=parent?parent->elevel():0;
	Td.init();
	Td.p=Point(lp.theta(t),lp.phi(p),1);
	Td.c=object->color();
	Td.level=lvl;

	if(!parent){
		mdcnt=rccnt=0;
	    for(int i=0;i<MAX_NDATA;i++)
	    	mapdata[i]=0;
	}

	last=parent;
	a=new MapNode(parent, t, p);
	Td.clr_flag(FNOREC);


	object->set_surface(Td);

	a->data.init_terrain_data(Td,0);
	if(Td.get_flag(INMARGIN))
	    a->set_margin(1);
	if(Td.get_flag(HIDDEN))
	    a->set_hidden(1);

	if(Adapt.recalc()&&!Td.get_flag(FNOREC))
		a->set_need_recalc(1);
	return a;
}

MapNode *Map::makenode(MapNode *parent, double t, double p)
{
	return makenode(parent,lp.ltheta(t),lp.lphi(p));
}

//-------------------------------------------------------------
// Map::point()	return rectangular point from theta,phi,height
//-------------------------------------------------------------
Point  Map::point(double t, double p, double r)
{
    if(symmetry==1)
		return Point(t,p,radius+Rscale*r).rectangular();
    Point pt=Point(t,p,radius+Rscale*r).rectangular();
    pt.y*=symmetry;
	return pt;
}

//-------------------------------------------------------------
// Map::set_resolution() adjust cell size
//-------------------------------------------------------------
void  Map::set_resolution()
{
    double d=rampstep(0,90,fabs(TheScene->view_tilt),1,0.5);
    double dp=-1;
    if(object->offscreen())
        d=4;
    else if(object->outside()){
    	Point p=TheScene->cpoint-TheScene->epoint;
    	Point v=object->point-TheScene->epoint;
    	dp=v.dot(p)/p.length()/v.length();
    	d=rampstep(0,1,fabs(dp),1,0.5);
 	}
	resolution=d*TheScene->resolution*TheScene->cellsize*object->resolution();
}


//-------------------------------------------------------------
// Map::get_info() get visibility, texture etc. info from node tree
//-------------------------------------------------------------
static int eflag=0;
static Map *map=0;

static void node_info(MapNode *n)
{
	MapData *d=&n->data;

	int wflag=0;
	if(d->water()){
	    wflag=1;
	    d=d->data2();
	}

	eflag=0;
	if(!n->partvis() && !n->visible())
	    return;

	if(wflag){
	    TheMap->set_waterpass(1);
		TheMap->idcnts[WATER]++;
	}
	int lid=d->dtype();
	map->idcnts[lid]++;

	if(lid+1>map->tids)
	   map->tids=lid+1;

	if(n->fog())
		map->set_fog(1);

	if(d->textures())
		map->set_vistexs(1);
	if(d->bumpmaps())
		map->set_visbumps(1);
}

void  Map::get_info()
{
	int i;
	set_waterpass(0);
	set_fog(0);
	set_visbumps(0);
	set_vistexs(0);
	map=this;

	for(i=0;i<Td.properties.size;i++){
		idcnts[i]=0;
	}
	vnodes=0;
	tids=0;
	npole->visit(node_info);
#ifdef DEBUG_INFO
	for(i=0;i<tids;i++){
	    if(idcnts[i]!=0)
	    	printf("%-8s %4d %4d\n",object->name(),i,idcnts[i]);
	}
#endif
}
//-------------------------------------------------------------
// Map::visid() return true if id is visible
//-------------------------------------------------------------
int Map::visid(int lid)
{
	return idcnts[lid]?1:0;
}

//=============================================================
// Triangle lists
// Applications
//   1. average normals
//   2. depth sorted vertexes
//=============================================================

int Map::render_triangles()  {
	return (render_lists() && TheScene->viewobj==object && Render.avenorms())?1:0;
}

#define WATER_AVES
#ifdef WATER_AVES
#define TID_START (TheMap->waterpass()?WATER:ID0)
#else
#define TID_START ID0;
#endif

inline void validate_norm(MapData *nd){
	MapData *d=nd->get_surface(TheMap->tid);
	if(d)
		d->validate_normal();
}
static void validate_norms(MapNode *n){
	if(n->cdata)
		validate_norm(n->cdata);
	validate_norm(&n->data);
}
inline void invalidate_norm(MapData *nd){
	MapData *d=nd->get_surface(TheMap->tid);
	if(d)
		d->invalidate_normal();
}
int tct=0;
static void invalidate_norms(MapNode *n){
	if(n->cdata)
		invalidate_norm(n->cdata);
	invalidate_norm(&n->data);
}

//-------------------------------------------------------------
// Map::invalidate_normals() invalidate normals
//-------------------------------------------------------------
void Map::invalidate_normals(){
	set_leaf_cycle(0);
	for (tid = TID_START; tid < tids; tid++) {
		npole->visit(invalidate_norms);
	}
}

//-------------------------------------------------------------
// Map::validate_normals() validate normals
//-------------------------------------------------------------
void Map::validate_normals(){
	set_leaf_cycle(0);
	for (tid = TID_START; tid < tids; tid++) {
		npole->visit(validate_norms);
	}
}
static MapData *tpiv;
static MapNode *tnode;
static MapData *t1;
static MapData *t2;

static void add_triangle(MapData *nd){
	MapData *d=nd->get_surface(TheMap->tid);
	if(!d){
		return;
	}
	switch(vcount){
	case 0:
		tpiv=d;
		break;
	case 1:
		if(d==tpiv){
			return;
		}
		t1=d;
		break;
	default:
		if(d==tpiv || d==t1){
			return;
		}
		t2=t1;
		t1=d;
		if(!tpiv->hidden() && !t2->hidden() && !t1->hidden()){
			TheMap->triangle_list.add(new Triangle(tpiv,t2,t1));
		}
		break;
	}
	vcount++;
}

static void add_triangles(MapNode *n){
	MapNode::TheNode=n;
	vcount=0;
	t1=t2=0;
	tpiv=0;
//	if(!n->visible()&& !n->partvis()){
//		return;
//	}

	n->visit_cycle(add_triangle);
}

//-------------------------------------------------------------
// Map::make_triangle_lists() set normals
//-------------------------------------------------------------
void Map::make_triangle_lists() {

	TheMap = this;
	Triangle *triangle;
#if DEBUG_TRIANGLES >0
	double d0=clock();
#endif
	if(Render.avenorms())
		invalidate_normals();

	set_leaf_cycle(1);
	triangles.free();
	triangle_list.reset();
	for (tid = TID_START; tid < tids; tid++) {
#if DEBUG_TRIANGLES >2
		int old_cnt=triangle_list.size;
		npole->visit_all(add_triangles);
		int new_cnt=triangle_list.size;
		cout << "Triangles added type:"<< tid << "="<<new_cnt-old_cnt<< endl;
#else
		npole->visit_all(add_triangles);
#endif
	}
	set_leaf_cycle(0);

#if DEBUG_TRIANGLES >1
	double d1=clock();
#endif
	triangle_list.ss();
	if(sort()){
	   triangles.set(triangle_list); // copy pointers to array
	   triangles.sort();
#if DEBUG_TRIANGLES >1
	   double d2=(double)clock();
	   cout << "Map::sort :"<< (double)(d2 - d1)/CLOCKS_PER_SEC<< endl;
#endif
	}
	else if(render_triangles())
		triangles.set(triangle_list); // copy pointers to array
	if(Render.avenorms()){
#if DEBUG_TRIANGLES >1
		double d2=clock();
#endif
		triangle_list.ss();
		while ((triangle = triangle_list++)>0) {
			if(!triangle->visible)
				continue;
			if(Raster.clean_edges() && triangle->backfacing)
				continue;
			Point c = triangle->d1->point();
			Point norm = c.normal(triangle->d2->point(), triangle->d3->point());
			triangle->d1->add_normal(norm);
			triangle->d3->add_normal(norm);
			triangle->d2->add_normal(norm);
		}
		validate_normals();
#if DEBUG_TRIANGLES >1
		double del1=(double)d1-d0;
		double del2=(double)clock()-d2;
		cout << "Map::ave_normals:"<< 1000.0*(del1+del2)/CLOCKS_PER_SEC << " ms n:"<< triangle_list.size << endl;
#endif
	}
	triangle_list.ss();
	if(!make_lists()&& !render_triangles())
		triangle_list.free();
#if DEBUG_TRIANGLES >0
	if(TheScene->viewobj==object){
	cout << "Map::triangles overhead:"<< (clock()-d0)/CLOCKS_PER_SEC << " ms"<< endl;
	}
#endif

}

//-------------------------------------------------------------
// Map::render_triangles() render using triangle lists
//-------------------------------------------------------------
void Map::render_triangle_list() {
	int n=triangles.size;
#if DEBUG_TRIANGLES >0
	double d0=clock();
#endif
	glBegin(GL_TRIANGLES);
	if(render_ftob()){
		for(int i=0;i<n;i++){
			Triangle *triangle=triangles[i];
			if(triangle->type!=tid)
				continue;
			if(DRAW_VIS && !triangle->visible)
				continue;
			triangle->init();
			(npole->*MapNode::Cstart)(triangle->d1);
			(npole->*MapNode::Cstart)(triangle->d2);
			(npole->*MapNode::Cstart)(triangle->d3);
		}
	}
	else{
		for(int i=n-1;i>=0;i--){
			Triangle *triangle=triangles[i];
			if(triangle->type!=tid)
				continue;
			if(DRAW_VIS && !triangle->visible)
				continue;
			triangle->init();
			(npole->*MapNode::Cstart)(triangle->d1);
			(npole->*MapNode::Cstart)(triangle->d2);
			(npole->*MapNode::Cstart)(triangle->d3);
		}
	}
	glEnd();
#if DEBUG_TRIANGLES >0
	cout << "Map::render_triangles:"<< (clock()-d0)/CLOCKS_PER_SEC << " ms number:"<< n<<endl;
#endif
}
//============================================================

//************************************************************
// RingMap class
//************************************************************
//-------------------------------------------------------------
// RingMap::point()	return rectangular point from theta,phi,height
//-------------------------------------------------------------
Point  RingMap::point(double t, double p, double r)
{
	return Point(t,0.0,radius+width*sin(RPD*fabs(p))).rectangular();
}

//-------------------------------------------------------------
// RingMap::set_scene()	 set near/far clipping planes, horizon etc.
//-------------------------------------------------------------
void RingMap::set_scene()
{
	Map::set_scene();
}

