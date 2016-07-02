
#include "Octree.h"
#include "RenderOptions.h"
#include "SceneClass.h"
#include "ImageClass.h"
#include "TextureClass.h"
#include "TerrainClass.h"
#include "TerrainData.h"
#include "AdaptOptions.h"

//**************** extern API area ************************

//#define DEBUG_RENDER

//#define DEBUG_STARS

extern double Radius,Theta,Phi,Density;
extern int hits,visits,misses;

static OctTree *octree=0;
static MinMax mx1,mx2;
const double TWOPI=2.0*PI;

static double gdist=0;
static double sdist=0;
static double smax;

static double rnoise=0;
static double pnoise=0;
static double max_ext;
static int mcreated=0,mdeleted=0, mtotal=0;

static 	TerrainData Td;


extern const char *vstg[];

static void star_info(StarTree *tree)
{
   if(!tree->nodes)
       return;
   Point p=tree->origin/LY;
   double stars_per_cell=tree->fgpts+tree->bgpts;
   stars_per_cell=tree->nodes?stars_per_cell/tree->nodes:0;

   TheScene->draw_string(DATA_COLOR,"%-4.0f %-4.0f %-4.0f %-3s %-6.3f %-7.1f %-5.1f %-5.1f %-4.1f %-3.1f",
		p.x/1000.0,
		p.y/1000.0,
		p.z/1000.0,
		vstg[tree->vtype],
		fabs(tree->z1),
   		tree->nodes/1000.0,
   		tree->fgpts/1000.0,
    	tree->bgpts/1000.0,
     	stars_per_cell,
    	tree->resolution_scale
 		);
}

static void init_star_specs()
{
	if(!Render.display(OCTINFO))
		return;
	TheScene->draw_string(HDR1_COLOR,"------- galaxy info ---------------------------");
	TheScene->draw_string(HDR1_COLOR,"X    Y    Z    view       cells   fg    bg    s/c  rs");
	TheScene->draw_string(HDR1_COLOR,"-----------------------------------------------");
	mcreated=mdeleted=0;
	//mx1.reset();
	//mx2.reset();
}

void show_star_specs()
{
	if(!Render.display(OCTINFO))
		return;
	TheScene->draw_string(HDR1_COLOR,"----------------------------------------------");
	TheScene->draw_string(DATA_COLOR,"total %-6.1f new  + %-5.1f - %-5.1f",
	    mtotal/1000.0,mcreated/1000.0,mdeleted/1000.0);
	//mx1.print("ext");
	//mx2.print("rf");

	TheScene->draw_string(HDR1_COLOR,"----------------------------------------------");
}

//**************** static and private *********************

static Color star_colors[]={
	Color(1.0, 1.0, 1.0),
	Color(1.0, 1.0, 0.9),
	Color(1.0, 1.0, 0.8),
	Color(1.0, 1.0, 0.7),
	Color(1.0, 1.0, 0.6),
	Color(1.0, 1.0, 0.5),
	Color(1.0, 1.0, 0.4),
	Color(1.0, 1.0, 0.3),
	Color(1.0, 0.8, 0.6),
	Color(1.0, 0.7, 0.6),
	Color(1.0, 0.7, 1.0),
	Color(1.0, 0.7, 0.9),
	Color(0.4, 0.8, 1.0),
	Color(0.7, 0.7, 1.0)
};

static int ncolors=sizeof(star_colors)/sizeof(Color);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// node classes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//************************************************************
// OctNode class
//************************************************************
//-------------------------------------------------------------
// OctNode::OctNode()	constructor
//-------------------------------------------------------------
OctNode::OctNode(uint l, int x, int y, int z)
{
	set_level(l);
	lx=x;
	ly=y;
	lz=z;
	child=0;
	clr_visible();
	clr_clipped();
	set_density(0);
	set_flags(0);
	octree->nodes++;
	mcreated++;
	mtotal++;
	info.slvl=0;	   // subdivision level target
	info.tests=0;       // test flags
	info.visible=0;       // visibility flag
	info.clipped=0;       // visibility flag
	info.flags=0;       // class flag

}

//-------------------------------------------------------------
// OctNode::~OctNode()	destructor
//-------------------------------------------------------------
OctNode::~OctNode()
{
	for(int i=0;i<children();i++){
		if(child[i])
			delete child[i];
		child[i]=0;
	}
	FREE(child);
	octree->nodes--;
	mdeleted++;
	mtotal--;
}

//-------------------------------------------------------------
// OctNode::distance()	select nodes
//-------------------------------------------------------------
Point OctNode::point()
{
	return center()*octree->gscale;
}

//-------------------------------------------------------------
// OctNode::locate(Point &p) return smallest node containing point
//-------------------------------------------------------------
OctNode *OctNode::locate(Point &p)
{
	return this;
}

//-------------------------------------------------------------
// OctNode::calc_density(Point &p) return density
//-------------------------------------------------------------
double OctNode::calc_density(Point &p)
{
	return 1.0;
}

//-------------------------------------------------------------
// OctNode::size() return cell size in global units
//-------------------------------------------------------------
double OctNode::size()
{
	return cellsize()*octree->gscale;
}

//-------------------------------------------------------------
// OctNode::contains(Point &p) return true if node contains point
//-------------------------------------------------------------
int OctNode::contains(Point &p)
{
	double s=cellsize();
	Point  c=center();

	if(p.z>c.z+s || p.z < c.z-s)
		return 0;
	if(p.x>c.x+s || p.x < c.x-s)
		return 0;
	if(p.y>c.y+s || p.y < c.y-s)
		return 0;
	return 1;
}

//-------------------------------------------------------------
// OctNode::center() return center point
//-------------------------------------------------------------
Point OctNode::center()
{
	return Point(lx,ly,lz);
}

//-------------------------------------------------------------
// OctNode::randomize() set object point
//-------------------------------------------------------------
Point OctNode::randomize()
{
	Point p=center();
	double x=Random(p.x,p.y,p.z);
	double y=RAND(1);
	double z=RAND(2);
	return p+(Point(x,y,z)*(2.0*cellsize()));
}

void OctNode::setValue(){
	if(isActive()){
		Point p=center();
		Point mpt=p*octree->gscale;
		double d=mpt.distance(TheScene->vpoint);
		_value=d;
		OctTree::unsorted.add(this);
	}
}

//-------------------------------------------------------------
// OctNode::value() return distance to center point from eye
//-------------------------------------------------------------
double OctNode::value()
{
	return _value;
}

//-------------------------------------------------------------
// OctNode::visit()	apply OctNode::function to this and child nodes
//-------------------------------------------------------------
void OctNode::visit(void (OctNode::*func)())
{
	(this->*func)();
	for(int i=0;i<children();i++)
		if(child[i])
			child[i]->visit(func);
}

//-------------------------------------------------------------
// OctNode::visit()	apply function to this and child nodes
//-------------------------------------------------------------
void OctNode::visit(void (*func)(OctNode*))
{
	(*func)(this);
	for(int i=0;i<children();i++)
		if(child[i])
			child[i]->visit(func);
}

//-------------------------------------------------------------
// OctNode::split()	split node into 8 children
//-------------------------------------------------------------
void OctNode::split()
{
    if((octree->nodes>octree->maxnodes))
	   return;
	MALLOC(8,OctNode*, child);
	uint l=level()-1;
	uint s=1<<l;
	child[0]=octree->build(l,lx-s,ly+s,lz+s);
	child[1]=octree->build(l,lx+s,ly+s,lz+s);
	child[2]=octree->build(l,lx+s,ly-s,lz+s);
	child[3]=octree->build(l,lx-s,ly-s,lz+s);
	child[4]=octree->build(l,lx-s,ly+s,lz-s);
	child[5]=octree->build(l,lx+s,ly+s,lz-s);
	child[6]=octree->build(l,lx+s,ly-s,lz-s);
	child[7]=octree->build(l,lx-s,ly-s,lz-s);

	octree->init_node(this);

}

//-------------------------------------------------------------
// OctNode::combine()	delete all children
//-------------------------------------------------------------
void OctNode::combine()
{
	if(child)
		delete child;
	child=0;
	octree->init_node(this);
}

//-------------------------------------------------------------
// OctNode::clipped() return true if cell is clipped
//-------------------------------------------------------------
int OctNode::clip_test()
{
    Point p=center()*octree->gscale;
	p=p.mm(TheScene->viewMatrix);
	double x,y,z;
	z=p.mz(TheScene->lookMatrix);	// z (depth z projection)

	double esize=4*cellsize()*octree->gscale;

    if(-z<TheScene->znear-esize)
        return 1;

    double margin=fabs(esize/z);
	double t=(1+margin);

	y=p.my(TheScene->projMatrix)/z;
    if(y < -t || y >t)
        return 1;
    t*=TheScene->aspect;
	x=p.mx(TheScene->projMatrix)/z;
    if(x < -t|| x >t)
        return 1;
    if(-z>TheScene->zfar+esize)
        return 1;

	return 0;
}

//-------------------------------------------------------------
// OctNode::set_tests()	set flags
//-------------------------------------------------------------
void OctNode::set_tests()
{
	clr_tests();
	double s=cellsize();

	// test for minimum size

	if(!child && s>=octree->maxsize){
		set_stest();
		return;
	}

	if(Adapt.clip_test() && clip_test())
		set_clipped();
	else
		clr_clipped();


    double d=randomize().distance(TheScene->vpoint*octree->wscale);

	// test if inside cell

	//if(!child && d<=2*s && s>octree->minsize){
	if(!child && d<=s && s>octree->minsize){
		set_stest();
		return;
	}
	double ext,mxext,mnext;

	ext=1.2*TheScene->wscale*s/d;

	mxext=octree->normal_ext;
	if(!visible()||clipped())
		mxext=octree->hidden_ext*slevel();
	//else if(density()<octree->diffusion)
	//else if(density()<=0)
	//	mxext=octree->smooth_ext;
	else if(TheScene->viewtype==SURFACE)
		mxext*=2;
	mnext=mxext*octree->combine_factor;

	if((!child) && (ext>=mxext) && (s>octree->minsize) )
		set_stest();
	else if(ext < mnext && s<octree->maxsize)
		set_ctest();
}

//-------------------------------------------------------------
// OctNode::adapt()	adapt nodes
//-------------------------------------------------------------
void OctNode::adapt()
{
	int i,j;
	set_tests();

	if(stest())
		split();
	else if(ctest()){
		for(i=0,j=0;i<children();i++){
			if(child[i]){
				child[i]->adapt();
				if(child[i]->child==0 && child[i]->ctest()){
					delete child[i];
					child[i]=0;
				}
			}
			if(!child[i])
				j++;
		}
		if(j==8){
			delete child;
			child=0;
		}
	}
}

//-------------------------------------------------------------
// OctNode::render()	render nodes
//-------------------------------------------------------------
void OctNode::render()
{
}

//-------------------------------------------------------------
// OctNode::select()	select nodes
//-------------------------------------------------------------
void OctNode::select()
{
}

//************************************************************
// DensityNode class
//************************************************************
//-------------------------------------------------------------
// DensityNode::DensityNode()	constructor
//-------------------------------------------------------------
DensityNode::DensityNode(uint l, int x, int y, int z) : OctNode(l,x,y,z)
{
}

//-------------------------------------------------------------
// DensityNode::render()	render nodes
//-------------------------------------------------------------
void DensityNode::render()
{
	if(child || !visible())
		return;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// tree classes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//************************************************************
// OctTree class
//************************************************************
Point OctTree::last_view;
LinkedList<OctNode*> OctTree::unsorted;
ValueList<OctNode*> OctTree::sorted;
double OctTree::max_value=0;
bool OctTree::sort_nodes=true;
//-------------------------------------------------------------
// OctTree::OctTree()	constructor
//-------------------------------------------------------------
OctTree::OctTree(double s, double u)
{
	flags=0;
	levels=0;
	units=u;
	double t=1;
	while(t<s){
		t*=2;
		levels++;
	}

	size=s*units;
	fullsize=(double)(1<<levels);
	gscale=size/fullsize;    // converts from int to global
	nscale=1/fullsize;		 // converts from int to fraction (0..1)
	dispersion=2;
	diffusion=0.05;
	shape_exterior=0.0001;
	shape_interior=0;
	shape_sharpness=0.5;
	shape_solidity=1.0;
	resolution_value=0.15;
	resolution_scale=1;
	density_value=1;
	density_scale=1;

	nodes=0;
	maxnodes=20000000;
	maxsize=fullsize/8.0;
	minsize=1;
	//minsize=0.5;
	wscale=1.0/gscale;
	root=0;
	object=0;
	max_value=0;
	make_current();
}

//-------------------------------------------------------------
// OctTree::~OctTree()	destructor
//-------------------------------------------------------------
OctTree::~OctTree()
{
	make_current();
	delete root;
	root=0;
}

const char *OctTree::typeName() { return "";}

//-------------------------------------------------------------
// OctTree::rebuild()	rebuild nodes
//-------------------------------------------------------------
void OctTree::rebuild()
{
	make_current();
	delete root;
	root=0;
}

//-------------------------------------------------------------
// OctTree::make_current()	set current
//-------------------------------------------------------------
void OctTree::make_current()
{
	octree=this;
}

//-------------------------------------------------------------
// OctTree::init_node() virtual node builder function
//-------------------------------------------------------------
void OctTree::init_node(OctNode *octnode)
{
}

//-------------------------------------------------------------
// OctTree::adapt()	adapt grid
//-------------------------------------------------------------
void OctTree::adapt()
{
//printf("StarTree::adapt(%g)\n",TheScene->vpoint.length()/LY);
	make_current();
	scale();
	if(!root)
		root=build(levels-1,0,0,0);
	//if(!changed_view())
	//	return;
	//mcreated=mdeleted=0;

//	max_ext=object->resolution()*TheScene->resolution*TheScene->cellsize/resolution();
	max_ext=5*pow(TheScene->cellsize*object->resolution()*TheScene->resolution,0.2)/resolution();
//cout << "ext1:"<< max_ext << endl;
//	max_ext=4*pow(TheScene->cellsize,0.2)*object->resolution()*TheScene->resolution/resolution();
//	max_ext=1.25*sqrt(TheScene->cellsize*object->resolution()*TheScene->resolution/resolution());
//cout << "ext2:"<< max_ext << endl;
    normal_ext=max_ext;
	hidden_ext=normal_ext*4;
	smooth_ext=normal_ext*1.5;
	//combine_factor=1.0/3;
	combine_factor=1.0/2;

 	visit(&OctNode::adapt);
}

//-------------------------------------------------------------
// OctTree::sortNodes() get distance sorted node array
//-------------------------------------------------------------
void OctTree::sortNodes(){
	if(sort_nodes){
#ifdef DEBUG_RENDER
		double start=clock();
#endif
		unsorted.reset();
		make_current();
		scale();
		visit(&OctNode::setValue);
		sorted.set(unsorted);
		sorted.sort();
#ifdef DEBUG_RENDER
		if(sorted.size>0){
			double sort_time=clock();
			cout << "sort time:"<< (sort_time - start)/CLOCKS_PER_SEC  << " nodes:"<< unsorted.size << endl;
		}
#endif
	}
}
//-------------------------------------------------------------
// OctTree::render() render grid
//-------------------------------------------------------------
void OctTree::render() {
#ifdef DEBUG_RENDER
	double start = clock();
#endif
	make_current();
	scale();
	if (sort_nodes) {
		if (sorted.size>0) {
			sorted.se();
			OctNode *node = sorted.at();
			while ((node = sorted--)>0) {
				node->render();
			}
#ifdef DEBUG_RENDER
		double render_time = clock();
		cout << "render time:" << (render_time - start) / CLOCKS_PER_SEC
				<< " nodes:" << sorted.size << endl;
#endif
		}
	} else
		visit(&OctNode::render);
}

//-------------------------------------------------------------
// OctTree::select()	select nodes
//-------------------------------------------------------------
void OctTree::select()
{
	make_current();
	scale();
	visit(&OctNode::select);
}

//-------------------------------------------------------------
// OctTree::init() init grid
//-------------------------------------------------------------
void OctTree::init()
{
}

//-------------------------------------------------------------
// OctTree::build() virtual node builder function
//-------------------------------------------------------------
OctNode *OctTree::build(uint l, int x, int y, int z)
{
	return new OctNode(l,x,y,z);
}

//-------------------------------------------------------------
// OctTree::scale() set znear zfar
//-------------------------------------------------------------
void OctTree::scale()
{
}

//-------------------------------------------------------------
// OctTree::inside() return true viewpoint is inside shape
//-------------------------------------------------------------
int OctTree::inside()
{
//	Point gp=TheScene->gpoint+TheScene->spoint-origin;
	Point gp=TheScene->gpoint-origin;
	if(gp.length()<=size)
		return 1;
	return 0;
}

//-------------------------------------------------------------
// OctTree::scale() 	set znear, zfar
//-------------------------------------------------------------
int  OctTree::scale(double &zn, double &zf)
{
	//Point gp=TheScene->gpoint+TheScene->spoint-origin;
	Point gp=TheScene->gpoint-origin;
	//Point gp=TheScene->gpoint-origin; // course znear zfar

	double gd=gp.length();

	if(gd<=size){
		zn=0.1*LY;
		zf=gd+2*size;
		return INSIDE;
	}
	else{
		zn=0.25*(gd-size);
		zf=gd+2*size;
		return OUTSIDE;
	}
}

//-------------------------------------------------------------
// OctTree::normalize() return center point
//-------------------------------------------------------------
void OctTree::normalize(Point &p)
{
	p=p*nscale;
}

//-------------------------------------------------------------
// OctTree::changed_view() return true if vpoint enters new octnode
//-------------------------------------------------------------
int OctTree::changed_view()
{
	Point p=TheScene->vpoint*wscale;
	p.x=floor(p.x);
	p.y=floor(p.y);
	p.z=floor(p.z);

	if(p!=last_view){
		last_view=p;
		return 1;
	}
	return 0;
}

//-------------------------------------------------------------
// OctTree::test_clipped() return true if cell is clipped
//-------------------------------------------------------------
int OctTree::test_clipped(Point &mp)
{

	Point p=mp.mm(TheScene->viewMatrix);
	double x,y,z;
	z=p.mz(TheScene->lookMatrix);	// z (depth z projection)

	if(-z >TheScene->zfar || -z< TheScene->znear )
		return 1;
	y=p.my(TheScene->projMatrix);   // screen y (0..2)
	if(y<z || y>-z)
		return 1;

	double t=z*TheScene->aspect;

	x=p.mx(TheScene->projMatrix);	// screen x (0..2)
	if(x<t || x>-t)
		return 1;

	return 0;
}

//-------------------------------------------------------------
// OctTree::test_surface() test if point is in shape
//-------------------------------------------------------------
double OctTree::test_surface(Point &p)
{
	return 1.0;
}

//-------------------------------------------------------------
// OctTree::point() return node point (eye ref)
//-------------------------------------------------------------
Point OctTree::point(OctNode *n)
{
    make_current();
    Point gp=TheScene->epoint-TheScene->spoint-origin;
    //Point gp=TheScene->gpoint-origin;
    selpt=n->point();
	return gp-selpt;
}

//-------------------------------------------------------------
// OctTree::draw_selpt() draw selection point
//-------------------------------------------------------------
void OctTree::draw_selpt()
{
}

//************************************************************
// DensityTree class
//************************************************************

const char *DensityTree::def_noise_expr="noise(NLOD,2,7,1,0.2)+0.1*noise(NLOD,14,5,0.5)\n";
const char *DensityTree::def_color_expr="1.0*DENSITY\n";
const char *DensityTree::def_color_list=
	"Colors("
		"Color(0.502,0.000,0.251),"
		"Color(0.000,0.000,1.000),"
		"Color(0.000,0.725,1.000),"
		"Color(0.502,1.000,1.000),"
		"Color(1.000,0.300,0.000),"
		"Color(1.000,0.702,0.200),"
		"Color(1.000,1.000,0.400),"
		"Color(1.000,1.000,0.702)"
	")\n";

//-------------------------------------------------------------
// DensityTree::DensityTree()	constructor
//-------------------------------------------------------------
DensityTree::DensityTree(double s, double u) : OctTree(s,u)
{

	twist_angle=1.3;
	compression=0.0;
	noise_saturation=2;
	noise_cutoff=0.4;
	noise_vertical=0.1;
	twist_radius=1.4;
	outer_radius=0.8;
	inner_radius=0.3;
	gradient=0.9;
	center_radius=0.3;
	center_bias=1;
	color_mix=0.5;
	color_bias=1.3;
}
DensityTree::~DensityTree()
{
}

//-------------------------------------------------------------
// DensityTree::inside() return true viewpoint is inside shape
//-------------------------------------------------------------
int DensityTree::inside()
{
//	Point gp=TheScene->gpoint+TheScene->spoint-origin;
	Point gp=TheScene->gpoint-origin;
	if(test_surface(gp)>=0)
		return 1;
	return 0;
}

//-------------------------------------------------------------
// DensityTree::scale() set zn zf etc
//-------------------------------------------------------------
void DensityTree::scale()
{
	Point gp=TheScene->vpoint;
	gdist=gp.length();
	rnoise=pnoise=0;
	sdist=test_surface(gp);

	if(sdist>=0){
		vtype=INSIDE;
		//z1=LY;
		//z2=size;
	}
	else{
		vtype=OUTSIDE;
		//z1=-sdist;
		//z2=gdist+size;
	}
	z1=sdist/size;
}

//-------------------------------------------------------------
// DensityTree::scale() 	set znear, zfar
//-------------------------------------------------------------
int  DensityTree::scale(double &zn, double &zf)
{
	return OctTree::scale(zn,zf);
}

//-------------------------------------------------------------
// DensityTree::build() virtual node builder function
//-------------------------------------------------------------
OctNode *DensityTree::build(uint l, int lx, int ly, int lz)
{
	DensityNode *node=new DensityNode(l,lx,ly,lz);
	init_node(node);
	return node;
}

//-------------------------------------------------------------
// DensityTree::init() init grid
//-------------------------------------------------------------
void DensityTree::init()
{
}

//-------------------------------------------------------------
// DensityTree::init_node() virtual node builder function
//-------------------------------------------------------------
void DensityTree::init_node(OctNode *octnode)
{
    DensityNode *node=(DensityNode*)octnode;

	// reject cells outside sphere bounds

	if((node->center()*nscale).length()>0.6) {
	    node->set_slevel(4);
		return;
	}

	// calculate density

    double r,dns=1,R,rn=0,sm,ds;

	Point p=node->randomize();
	Point gp=p*gscale;
	p=p*nscale;

	rnoise=noise_amplitude;
	pnoise=rnoise*noise_vertical;
    if(shape_exterior){
		ds=-test_surface(gp);
		double sm1=shape_exterior*size;
		double sm2=2*node->size();
		sm=sm1>sm2?sm1:sm2;
		if(ds>sm){
			int slevel=(int)(clamp(1,4,ds/sm));
			node->set_slevel(slevel);
			return;
		}
    }
	R=p.length();

	r=sqrt(p.x*p.x+p.z*p.z);
	if(twist_angle){
		double mr=0.5*twist_radius;
		double f=mr-r;
		if(f<0)
			f=0;
		else
			f=f*f*f;
		f=(mr-f)/mr;
		double a=TWOPI*twist_angle*f;
		double c=cos(a);
		double s=sin(a);
		double x=p.x*c-p.z*s;
		double z=p.x*s+p.z*c;
		p.x=x;
		p.z=z;
	}

	Td.p=p;
	Td.density=0;
	object->set_surface(Td);
	if(noise_amplitude){
		rn=noise_amplitude*S0.density;
	}

	rnoise=rn;
	pnoise=noise_vertical*rn;

	gp=p*size;
	ds=test_surface(gp);
	if(ds>0 && shape_interior && vtype==OUTSIDE){
	    sm=shape_interior*size;
	    if(sm>0)
			dns=1-ds/sm;
	    else
	    	dns=0;
	}
	else if(ds<=0 && shape_exterior){
	    sm=-shape_exterior*size;
	    if(sm<0)
			dns=1-ds/sm;
	    else
	    	dns=0;
	}

	dns=clamp(dns,0,1);

	if(dns>0){
		rn+=0.5;

		double nf=1;

		// reduce noise and increase density in core

		if(cf){
			double cb=cf+cf*rnoise;
			if(r<cb){
				nf=smoothstep(0.1*cb,cb,r,0,1);
				dns+=center_bias*(1-nf);
			}
		}
		double rd=noise_saturation*(rn-noise_cutoff);
		if(rd>0)
			rn=diffusion+rd;
		else
			rn=diffusion*(1+2*rd);
	    rn=clamp(rn,0,1);
		dns*=smoothstep(0,1,nf,1,rn);
	}

	if(gradient>0){  // reduce density with distance;
	    double g;
		if(center_radius)
		    g=gradient*rampstep(cf,outer_radius,R,0,-2);
		else
		    g=gradient*rampstep(0,outer_radius,R,0,-2);
	    dns*=exp(g);
	}
	if(dns<0)
	    dns=0;
	node->set_density(dns);
	node->set_visible();
}

//-------------------------------------------------------------
// DensityTree::render() render grid
//-------------------------------------------------------------
void DensityTree::render()
{
 	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	//glDisable(GL_TEXTURE_2D);
	//glDisable(GL_TEXTURE_1D);
	glDisable(GL_FOG);
	glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	OctTree::render();
    glEnable(GL_DEPTH_TEST);
	glDisable(GL_POINT_SMOOTH);
	glPointSize(1.0f);
}

//-------------------------------------------------------------
// StarTree::make_current()	set active
//-------------------------------------------------------------
void DensityTree::make_current()
{
	OctTree::make_current();

    // static constants for test_surface

    smax=rmax=0.5*outer_radius*size;

	cf=0;
}

//-------------------------------------------------------------
// DensityTree::test_surface() test if point is in shape
//-------------------------------------------------------------
double DensityTree::test_surface(Point &p)
{
    rmax=0.5*outer_radius*size;
	smax=rmax;
	double sr=rmax+rnoise*rmax;
	return sr-p.length();
}

//************************************************************
// StarNode class
//************************************************************
//-------------------------------------------------------------
// OctNode::OctNode()	constructor
//-------------------------------------------------------------
StarNode::StarNode(uint l, int x, int y, int z) : DensityNode(l,x,y,z)
{
}

//-------------------------------------------------------------
// StarNode::select()	select nodes
//-------------------------------------------------------------
int StarNode::stars(Point &mpt)
{
	double dns=density()*octree->density_scale;
	int nmax=octree->vtype==OUTSIDE?200:1000;
    double s=cellsize()*octree->nscale;
	//double rf=nmax;//rampstep(0.25,2.0,ext,100,nmax);
	double sf=nmax*octree->density()*dns*sqrt(s);
	int nstars=(int)sf;
//	double rem=sf-nstars;
//	if(rem>0){
//		double r=RAND(2)+0.5;
//		if(rem>r)
//			nstars++;
//	}
	return nstars;
}

//-------------------------------------------------------------
// StarNode::point()	select nodes
//-------------------------------------------------------------
Point StarNode::point()
{
    Point max;
    brightest(max);
    return max;
}

//-------------------------------------------------------------
// StarNode::select()	select nodes
//-------------------------------------------------------------
void StarNode::select()
{
	if(!isActive())
		return;
	double dns=density()*octree->density_scale;
	if(dns<=0)
		return;
	Point vmax;
	if(brightest(vmax)){
 		//glPointSize(2.0f*fgscale);
 		glLoadName((long)this);
		glBegin(GL_POINTS);
		glVertex3dv((double*)(&vmax));
		glEnd();
	}
}

//-------------------------------------------------------------
// StarNode::brightest()	select nodes
//-------------------------------------------------------------
int StarNode::brightest(Point &vmax)
{
	if(!isActive())
		return 0;

	int i;
	Point v=randomize();
	Point mpt=v*octree->gscale;
	StarTree *galaxy=(StarTree*)octree;

	int nstars=stars(mpt);

	double d=mpt.distance(TheScene->vpoint);
	if(d>galaxy->fgfar)
	    return 0;

	int j=2;
	double ptmax=0;
	double delta=octree->dispersion*size();
	for(i=0;i<nstars;i++){
 		v.x=mpt.x+delta*RAND(j++);
		v.y=mpt.y+delta*RAND(j++);
		v.z=mpt.z+delta*RAND(j++);

  		double r=(RAND(j++)+0.8);

		double d=v.distance(TheScene->vpoint);
		double s=r*rampstep(0,galaxy->fgfar,d,1,0);
		double pts=galaxy->fgscale*rampstep(1,0,s*s,galaxy->fgpt1,galaxy->fgpt2);

		RAND(j++);
		if(pts>ptmax){
			ptmax=pts;
		    vmax=v;
		}
	}
    return 1;
}

//-------------------------------------------------------------
// StarNode::render()	render nodes
//-------------------------------------------------------------
void StarNode::render()
{
	if(!isActive())
		return;
	double r,delta,s,pts,alpha;
	int i,j=2;

	double dns=density()*octree->density_scale;
	if(dns<=0)
		return;
	StarTree *galaxy=(StarTree*)octree;

	Point v=randomize();
	int seed=lastn;
	Point mpt=v*octree->gscale;
	double d=mpt.distance(TheScene->vpoint);

	int nstars=stars(mpt);
	Radius=mpt.distance(galaxy->origin)/galaxy->size;

    // render bg stars
	if(TheScene->viewtype!=SURFACE && galaxy->render_bg() && d > galaxy->fgfar && galaxy->bgpt1>0){
		Color c=WHITE;
		Density=Td.density=dns;
		Td.p=v*octree->nscale;
		Td.c=c;
		octree->object->set_surface(Td);

		//dns=S0.density;
		c=Td.c;
		delta=octree->dispersion*size();
	    s=0.5*galaxy->bgpt1;//rampstep(galaxy->fgfar,galaxy->bgfar,d,galaxy->bgpt1,galaxy->bgpt2);
	    //s=rampstep(galaxy->fgfar,galaxy->bgfar,d,galaxy->bgpt1,galaxy->bgpt2);
		pts=galaxy->bgscale*s*galaxy->ptscale;
		alpha=rampstep(0,galaxy->diffusion,dns,0,1);
		double mix=galaxy->inside()?0.5:1.0;
		alpha*=mix*galaxy->color_mix;

		double sf=RAND(j++);

		double nd=0.5*galaxy->nova_density;
		if(nd>0 && sf>0.5-nd){
			pts*=galaxy->nova_size*(1+2*galaxy->variability*RAND(j++));
			nstars=1;
			alpha+=0.5;
		}
		glPointSize((GLfloat)pts);
		glColor4d(c.red(),c.green(),c.blue(),alpha*c.alpha());


		galaxy->bgpts+=nstars;
		glBegin(GL_POINTS);
		for(i=0;i<nstars;i++){
 			v.x=mpt.x+delta*RAND(j++);
			v.y=mpt.y+delta*RAND(j++);
			v.z=mpt.z+delta*RAND(j++);
			glVertex3dv((double*)(&v));
		}
		glEnd();
	}
	// render fg stars
	lastn=seed;
	j=2;
	if(galaxy->render_fg() && d<galaxy->fgfar && galaxy->fgpt1>0){
		delta=octree->dispersion*size();
		galaxy->fgpts+=nstars;
		for(i=0;i<nstars;i++){

			// calculate position

 			v.x=mpt.x+delta*RAND(j++);
			v.y=mpt.y+delta*RAND(j++);
			v.z=mpt.z+delta*RAND(j++);

			// calculate random star size

			r=(2.5*galaxy->variability*RAND(j++)+0.1);
			r=r*r;

			// calculate point size

			d=v.distance(TheScene->vpoint);
			s=rampstep(LY,galaxy->fgfar,d,1,0);
		    pts=galaxy->fgscale*rampstep(1,0,s*s,galaxy->fgpt1*r,galaxy->fgpt2);
			glPointSize((GLfloat)(pts));

			// calculate color

		    r=RAND(j++)+0.5;
		    r=r*r;
		    int cindx=(int)(r*ncolors);
		    cindx=(int)clamp(cindx,0,ncolors-1);
		    Color c=star_colors[cindx];
		    r=RAND(j+1)+0.75;
		    c=c.lighten(r);
			alpha=3*galaxy->color_mix*rampstep(0,galaxy->fgfar,d,1,0.1);

		    glColor4d(c.red(),c.green(),c.blue(),alpha);
			glBegin(GL_POINTS);
			glVertex3dv((double*)(&v));
			glEnd();
		}
	}
}

//************************************************************
// StarTree class
//************************************************************

double StarTree::fgpt2=0.125;
double StarTree::bgpt2=0.25;
double StarTree::bgscale=1.0;
double StarTree::fgscale=1.0;
//-------------------------------------------------------------
// StarTree::StarTree()	constructor
//-------------------------------------------------------------
StarTree::StarTree(double s) : DensityTree(s,LY)
{
	static int init_flag=1;
	if(init_flag){
#ifdef DEBUG_STARS
		Render.set_display(OCTINFO);
#endif
		add_initializer(init_star_specs);
		add_finisher(show_star_specs);
		init_flag=0;
	}
	// defaults to spiral galaxy
	fgpt1=2.5;
	bgpt1=1;
	fgfar=25*LY;    // max distance to render fg stars
	twist_angle=1.7;
	compression=0.9;
	noise_saturation=1;
	noise_cutoff=0.05;
	noise_vertical=0.05;
	noise_amplitude=1;
	twist_radius=1.5;
	outer_radius=0.8;
	inner_radius=0.8;
	gradient=1;
	center_radius=0.3;
	center_bias=0;
	nova_density=0.01;
	nova_size=2.0;
	variability=1.0;

	set_render_fg(1);
	set_render_bg(1);
	ptscale=1;
}

const char *StarTree::typeName()
{
	if(center_radius)
		return "Spiral";
	else
		return "Elyptical";
}

//-------------------------------------------------------------
// StarTree::scale() 	set znear, zfar
//-------------------------------------------------------------
int  StarTree::scale(double &zn, double &zf)
{
	make_current();
	rnoise=noise_amplitude;
	pnoise=rnoise*noise_vertical;

	return DensityTree::scale(zn,zf);
}

//-------------------------------------------------------------
// StarTree::scale() set znear zfar etc
//-------------------------------------------------------------
void StarTree::scale()
{
	DensityTree::scale();

   // adjust Point size to screen size
    GLfloat sizes[2];
	GLfloat step;

	glGetFloatv(GL_POINT_SIZE_RANGE,sizes);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY,&step);

    //ptscale=sqrt(TheScene->wscale/2000);
    //ptscale=1;

	bgpt2=0.25*bgpt1;

    z1=-sdist/size;
	//resolution_scale=rampstep(0,-0.25*size,sdist,1,20);
	double rf=rampstep(0,-0.5*size,sdist,0,2);
	resolution_scale=1+20*(1-exp(-rf));

	//double maxpt=pow(TheScene->cellsize*object->resolution()*TheScene->resolution,0.2);

	ptscale=rampstep(0,-0.2*size,sdist,1.5,10);
	bgscale=1.0;
	fgscale=Render.startex()?10.0:1.0;
	bgscale=1.0;
	if(Render.draw_shaded())
		bgscale=Render.startex()?1.0:0.5;
	else
		bgscale=Render.startex()?2.0:1.0;
	//ptscale*=Render.startex()?8.0:1.0;
	lastn=0;
}

//-------------------------------------------------------------
// StarTree::draw_selpt() draw selected star
//-------------------------------------------------------------
void StarTree::draw_selpt()
{
	glPushAttrib(GL_CURRENT_BIT);

    Color c=Color(1,1,0);
	glDisable(GL_POINT_SMOOTH);
	glColor3d(c.red(),c.green(),c.blue());
  	glPointSize(5.0f*fgscale);
	glBegin(GL_POINTS);
	glVertex3dv((double*)(&selpt));
	glEnd();
	glPopAttrib();

}

//-------------------------------------------------------------
// StarTree::render() render grid
//-------------------------------------------------------------
void StarTree::render()
{
	fgpts=bgpts=0;
	DensityTree::render();
	if(object==TheScene->focusobj)
	    draw_selpt();
	if(Render.display(OCTINFO))
	    star_info(this);
}

//-------------------------------------------------------------
// StarTree::select()	select nodes
//-------------------------------------------------------------
void StarTree::select()
{
	glPushAttrib(GL_CURRENT_BIT);
	make_current();
	scale();
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_FOG);
	glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_POINT_SMOOTH);
  	glPointSize(4.0f*fgscale);
	visit(&OctNode::select);

    glEnable(GL_DEPTH_TEST);
	glPointSize(1.0f);
	glPopAttrib();
}

//-------------------------------------------------------------
// DensityTree::build() virtual node builder function
//-------------------------------------------------------------
OctNode *StarTree::build(uint l, int lx, int ly, int lz)
{
	DensityNode *node=new StarNode(l,lx,ly,lz);
	init_node(node);
	return node;
}

//-------------------------------------------------------------
// StarTree::test_surface() test if point is in shape
//-------------------------------------------------------------

double StarTree::test_surface(Point &p)
{
	double sr=rmax+rnoise*rmax;
	double y=fabs(p.y);

	if(center_radius>0){
		double pn=pnoise*rmax;
		double r=sqrt(p.x*p.x+p.z*p.z);
		double cb=c1+cf*rnoise;
		double s;

		if(r>=sr){
			r=r-sr;
			return -sqrt(r*r+y*y);
		}
		if(r<=cb)
			s=rampstep(0,cb,r,a1,a2)+pn;
		else
			s=rampstep(cb,sr,r,a2,0)+pn;

	    return s-y;
	}
	else
		return sr-p.length();
}
//-------------------------------------------------------------
// StarTree::make_current()	set active
//-------------------------------------------------------------
void StarTree::make_current()
{
	DensityTree::make_current();
	cf=0;
	if(center_radius>0)
    	cf=0.5*outer_radius*center_radius;
	c1=rmax*center_radius;
	a1=c1*(1-compression);
	a2=0.1*a1;
}
