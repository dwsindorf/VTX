
#include "SceneClass.h"
#include "MapNode.h"
#include "MapLink.h"
#include "NoiseClass.h"
#include "AdaptOptions.h"
#include "RenderOptions.h"
#include "GLSLMgr.h"
#include "Effects.h"
#include "matrix.h"
#include "Perlin.h"

extern double Theta, Phi, Height,Rscale,Margin;
extern Point MapPt;


#define VCLIP     // enable viewobj clip  test
#define PCLIP     // enable parent sphere clip  test
#define ZCLIP     // enable dmax clip  test

//static double noise3(double vec[3]){
//	return Perlin::noise3(vec);
//}
//#define USE_GL_POLYGON

extern double INV2PI;
#define DRAW_VIS (!Render.draw_nvis()&&!Raster.draw_nvis())

// uncomment to get indicated behavior

#define USEMAXA 		    // set to balance alevels

//**************** extern API area ************************

extern int hits,misses,visits;
extern double   ptable[];

MapData        *mdcycle;
MapData        *mdctr;
MapData        *mapdata[MAX_NDATA];
int      		mdcnt = 0;
int      		rccnt = 0;
int 			rcycles=0;

static 			TerrainData Td;

//************************************************************
// external API functions
//************************************************************

double zslope()
{
	if(CurrentScope->rpass())
		return 0;
	static double s=0;
	if(!Td.get_flag(SFIRST)){
	    CELLSLOPE(Z(),s);
    	s*=TheMap->hscale*INV2PI;
		Td.set_flag(SFIRST);
    }
	return s;
}

//**************** static and private *********************

static MapNode  *last_grid = 0;

const double SQRT2=sqrt(2.0);

#define CPOINT(n) n->cdata->mpoint()
#define DPOINT(n) n->data.mpoint()

#define CELLSIZE(i) PI*TheMap->radius*ptable[i]

static LPoint   lp;
static idu      idb;
static MapNode *Neighbor[9];

//========  hash table utility classes ====================

#ifdef HASH_NORMALS
extern HashTable normals;
#endif

Color sctbl[16]={
	Color(0,1,1),		// cyan (water)
	Color(1,0,0),		// red
	Color(0,1,0),		// green
	Color(0,0,1),		// blue
	Color(1,1,0),	    // yellow
	Color(1,0,0.5),		// violet
	Color(0.5,0.5,0.0),	// dark yellow
	Color(0.5,0.0,0.0),	// dark red
	Color(0.0,0.5,0.0),	// dark green
	Color(1.0,0.5,0.0),	// orange
	Color(1.0,0.5,1.0),	// pale violet
	Color(0.5,1.0,0.5),	// pale green
	Color(0.5,1.0,1.0),	// pale cyan
	Color(0.5,0.5,1.0),	// pale blue
	Color(0.0,0.5,0.5),	// dark cyan
	Color(1,1,1)		// white
};

//************************************************************
// Triangle class
//************************************************************
#define TOP    1
#define BOTTOM 2

Triangle::Triangle(MapData *a,MapData *b,MapData *c){
	d1=a;
	d2=b;
	d3=c;
	distance=0;
	type=TheMap->tid;
	Point pt=d1->mpoint()+d2->mpoint()+d3->mpoint();
	pt=pt/3.0;
	distance=TheScene->vpoint.distance(pt);
	backfacing=false;
	if(TheMap->frontface==GL_FRONT){
		Point c = d1->mpoint();
		Point norm = c.normal(d2->mpoint(), d3->mpoint());
		Point vp=TheScene->vpoint-c;
		if(norm.dot(vp)<=0.0)
			backfacing=true;
	}
//	if(MapNode::TheNode->nodraw() && !MapNode::TheNode->margin() && d1->X()==0 && d1->Y()==0){
	MapNode *n=MapNode::TheNode;
	if(n && n->nodraw() && !n->margin() && d1->X()==0 && d1->Y()==0){
		//cout <<".";
		visible=false;
	}
	else
		visible=true;
}

double Triangle::size(){
	Point c=center();
	Point p1=d1->mpoint();
	Point p2=d2->mpoint();
	Point p3=d3->mpoint();
	double s1=c.distance(p1);
	double s2=c.distance(p2);
	double s3=c.distance(p3);

	double size=s1>s2?s1:s2;
	size=size>s3?size:s3;
	return size;
}

double Triangle::extent(){
	Point c=center();
	double depth=TheScene->vpoint.distance(c);
	double ext=TheScene->wscale*(size()/depth); // perspective scaling
	return ext;
}

double Triangle::density(){
	double s1=d1->density();
	double s2=d2->density();
	double s3=d3->density();
	return (s1+s2+s3)/3.0;
}

double Triangle::depth(){
	double s1=d1->depth();
	double s2=d2->depth();
	double s3=d3->depth();
	return (s1+s2+s3)/3.0;
}

double Triangle::height(){
	double s1=d1->Z();
	double s2=d2->Z();
	double s3=d3->Z();
	return (s1+s2+s3)/3.0;
}

Point Triangle::vertex(){
	Point p1=d1->point();
	Point p2=d2->point();
	Point p3=d3->point();
	Point p=p1+p2;
	p=p+p3;
	return p/3.0;
}

Point Triangle::center(){
	Point p1=d1->mpoint();
	Point p2=d2->mpoint();
	Point p3=d3->mpoint();
	Point p=p1+p2;
	p=p+p3;
	return p/3.0;
}

Point Triangle::normal(){
	Point p;
	if(!Render.avenorms()){
		p=d1->point().normal(d2->point(), d3->point());
	}
	else{
		Point p1=d1->normal_;
		Point p2=d2->normal_;
		Point p3=d3->normal_;
		p=p1+p2;
		p=p+p3;
	}
	p=p.normalize();
	return p;
}

void Triangle::init(){
	Phi = d1->phi()/180;
	Theta = d1->theta()/180.0-1;
    MapPt = d1->mpoint();

	for(int j=0;j<TheMap->tp->textures.size;j++){
		Texture *t=TheMap->tp->textures[j];
		t->svalue=Phi;
		t->tvalue=Theta;
	}

}

FColor Triangle::color(){
	FColor c1(d1->color());
	FColor c2(d2->color());
	FColor c3(d3->color());
	FColor c=c1+c2;
	c=c+c3;
	c=c/3.0;
	double alpha=c1.alpha()+c2.alpha()+c3.alpha();
	c.set_alpha(alpha/3.0);
	return c;
}

//************************************************************
// MapNode class
//************************************************************

MNV MapNode::Dmid=&MapNode::vertex;
MNV MapNode::Dstart=&MapNode::vertex;
MND MapNode::Cmid=&MapNode::vertex;
MND MapNode::Cstart=&MapNode::vertex;
MapNode *MapNode::TheNode=0;
MapNode::MapNode(MapNode *parent, uint t, uint p): data(t,p)
{
	info.alvl=info.flvl=parent?parent->info.alvl:0;
	info.tlvl=parent?parent->info.tlvl:TLMAX;

	link=0;
	lnode=rnode=unode=dnode=0;
	init_flags();
	cdata=0;
	TheMap->mcreated++;
	TheScene->cells++;
	TheMap->size++;
	mdctr=&data;
	gval=0;
}

MapNode::MapNode(MapNode *parent, MapData *c) : data(c)
{
	info.alvl=info.flvl=parent->info.alvl;
	info.tlvl=parent->info.tlvl;

	link=0;
	lnode=rnode=unode=dnode=0;
	init_flags();
	info.cdata=0;
	gval=0;

	//c->invalidate_normal();
	c->invalidate();
	cdata=0;
	TheScene->cells++;
	//TheMap->nodes++;
	mdctr=&data;
	if(parent->margin())
	    set_margin(1);
	if(parent->hidden())
	    set_hidden(1);
}

MapNode::~MapNode()
{
	if(TheMap){
	    TheMap->mdeleted++;
	    //if(visible())
	    //	TheMap->vnodes--;
		TheScene->cells--;
		TheMap->size--;
		if(TheMap->last==this)
			TheMap->last=0;
		if(TheMap->current==this)
			TheMap->current=0;
	}
	if(cdata){
		TheMap->mdeleted++;
		TheMap->size--;
		delete cdata;
		cdata=0;
	}
}

void MapNode::Rvertex()              { info.cdata=0;Raster.vertex(this);}
void MapNode::Rvertex(MapData*d)     { info.cdata=1;Raster.vertex(this);}
MapData * MapNode::surface_data(MapData *m)
{
	MapData *d=m;
    if(Raster.surface==1){
        if(d->water())
            d=d->data2();
        while(d && (d->type()!=TheMap->tid))
            d=d->data2();
    }
    else{
        while(d && !d->water())
            d=d->data2();
    }
    return d;
}

//-------------------------------------------------------------
// MapNode::Vgroup() get vertical-horizontal neighbors to node
//-------------------------------------------------------------
void MapNode::Vgroup(MapData**p)
{
	MapNode *n;
	p[0]=p[1]=p[2]=p[3]=0;
	n=CWup();
	if(n)
		p[GUP]=surface_data(&(n->data));
	n=CCWdown();
	if(n)
		p[GDN]=surface_data(&(n->data));
	n=CWright();
	if(n)
		p[GRT]=surface_data(&(n->data));

	n=CCWleft();
	if(n)
		p[GLT]=surface_data(&(n->data));
}

//-------------------------------------------------------------
// MapNode::Pgroup() get lattice neighbors to cnode
//-------------------------------------------------------------
void MapNode::Pgroup(MapData**p)
{
	p[0]=p[1]=p[2]=p[3]=0;
	p[GLT]=surface_data(&data);
	MapNode *n=CWright();
	if(n){
		p[GUP]=surface_data(&(n->data));
		n=n->CWdown();
		if(n)
			p[GRT]=surface_data(&(n->data));
	}
	n=CCWdown();
	if(n){
		p[GDN]=surface_data(&(n->data));
		if(!p[GRT]){
			n=n->CWright();
			if(n)
				p[GRT]=surface_data(&(n->data));
		}
	}
}

//-------------------------------------------------------------
// MapNode::Dgroup() get diagonal (cnode) neighbors to node
//-------------------------------------------------------------
void MapNode::Dgroup(MapData**p)
{
	p[0]=p[1]=p[2]=p[3]=0;
	MapNode *n;
	if(cdata)
		p[GRT]=surface_data(cdata);
	n=CWup();
	if(n){
		if(n->cdata)
			p[GUP]=surface_data(n->cdata);
		n=n->CCWleft();
		if(n && n->cdata)
			p[GLT]=surface_data(n->cdata);
	}
	n=CCWleft();
	if(n && n->cdata)
		p[GDN]=surface_data(n->cdata);
}

//-------------------------------------------------------------
// MapNode::Cgroup() get vertical-horizontal neighbors to cnode
//-------------------------------------------------------------
void MapNode::Cgroup(MapData**p)
{
	MapNode *n;
	p[0]=p[1]=p[2]=p[3]=0;

	n=CWup();
	if(n && n->cdata)
		p[GUP]=surface_data(n->cdata);
	n=CCWdown();
	if(n && n->cdata)
		p[GDN]=surface_data(n->cdata);

	n=CWright();
	if(n && n->cdata)
		p[GRT]=surface_data(n->cdata);

	n=CCWleft();
	if(n && n->cdata)
		p[GLT]=surface_data(n->cdata);
}

//-------------------------------------------------------------
// MapNode::Lgroup() get neighbors to a lattice node
//-------------------------------------------------------------
void MapNode::Lgroup(MapData**p)
{
	if(cdata){
		Dgroup(p);
		if(p[GUP]&&p[GLT]&&p[GDN])
			return;
	}
	Vgroup(p);
}

//-------------------------------------------------------------
// MapNode::cell_size()		return cell size
//-------------------------------------------------------------
double MapNode::cell_size()
{
	return CELLSIZE(elevel());
}

//-------------------------------------------------------------
// MapNode::clr_cchecks()   set changed surface geometry flags
//-------------------------------------------------------------
void MapNode::clr_cchecks()
{
	if(unode) {
		unode->clr_ccheck();
		if(unode->lnode)
			unode->lnode->clr_ccheck();
		if(unode->rnode)
			unode->rnode->clr_ccheck();
	}
	if(dnode) {
		dnode->clr_ccheck();
		if(dnode->lnode)
			dnode->lnode->clr_ccheck();
		if(dnode->rnode)
			dnode->rnode->clr_ccheck();
	}
	if(rnode) {
		rnode->clr_ccheck();
		if(rnode->unode)
			rnode->unode->clr_ccheck();
		if(rnode->dnode)
			rnode->dnode->clr_ccheck();
	}
	if(lnode) {
		lnode->clr_ccheck();
		if(lnode->unode)
			lnode->unode->clr_ccheck();
		if(lnode->dnode)
			lnode->dnode->clr_ccheck();
	}
}

//-------------------------------------------------------------
// MapNode::free()		delete the node
//-------------------------------------------------------------
void MapNode::free()
{
	if(link){
		link->free();
		delete link;
		link=0;
	}
	remove();
}

//-------------------------------------------------------------
// MapNode::remove()	remove all links to node
//-------------------------------------------------------------
void MapNode::remove()
{
	if(unode)
		unode->dnode=dnode;
	if(dnode)
		dnode->unode=unode;
	if(rnode)
		rnode->lnode=lnode;
	if(lnode)
		lnode->rnode=rnode;
}

//-------------------------------------------------------------
// MapNode::adapt()		adaptive resizing function
//-------------------------------------------------------------
void MapNode::adapt()
{
	TheMap->current=this;

	if(!link){
		if(stest())
			link=split();
		return;
	}
	if(link){
		link->adapt();
		if(link->empty()){
			delete link;
			link=0;
		}
	}
}

//-------------------------------------------------------------
// MapNode::split()	split cell into 4 equal-sized child cells
//-------------------------------------------------------------
#define MKLINKS(u,a,rn,ln) \
		a->rn=u;\
		a->ln=u->ln;\
		u->ln->rn=a;\
		u->ln=a;\
		u=a;
#define IFNEW(u,r,rn) \
	while(u && u!=r && u->flvl() > alvl())\
		u=u->rn;\
	if(u && u==r)

#define CD4 \
	mdcnt=0;  \
	mapdata[mdcnt++]=&data; \
	mapdata[mdcnt++]=&ctr->data; \
	mapdata[mdcnt++]=&rt->data; \
	mapdata[mdcnt++]=&dn->data; \
	rccnt=mdcnt

#define MD2(l,r) \
	mdcnt=0; \
	mapdata[mdcnt++]=&l->data; \
	mapdata[mdcnt++]=&r->data; \
	rccnt=mdcnt

#define MD4(c,x) \
	if(x && !x->cdata && x->alvl()<alvl()) { \
		for(i=0;i<rccnt;i++) \
			tmpmd[i]=mapdata[i]; \
		tmpctr=mdctr; \
		tmprc=rccnt; \
		tmpdc=mdcnt; \
		x->split(); \
		for(i=0;i<tmprc;i++) \
			mapdata[i]=tmpmd[i]; \
		rccnt=tmprc; \
		mdcnt=tmpdc; \
		mdctr=tmpctr; \
	} \
	mapdata[mdcnt++]=&c->data; \
	if(x && x->cdata && x->alvl()==alvl()-1) { \
		mapdata[mdcnt++]=x->cdata;\
	} \
	rccnt=mdcnt

#define MAPDATA(l,r,x) \
	MD2(l,r); \
	MD4(c,x);


MapLink *MapNode::split()
{
	MapNode *a,*c,*l,*r,*u,*d,*rt,*dn,*ctr=0;
	MapData *md=0;
	int i,tmpdc,tmprc;
	MapData *tmpmd[MAX_NDATA];
	MapData *tmpctr;

	rt=CWright();
	dn=CCWdown();
	mdcnt=rccnt=0;
	for(i=0;i<MAX_NDATA;i++)
		mapdata[i]=0;

	if(!dn || !rt)
		return 0;

	clr_nflags();
	clr_cchecks();
	ctr=dn->CCWright(); // always true
	inc_alevel();
	uint lvl=alvl();

	uint th=data.ltheta();
	uint ph=data.lphi();
	uint dt=lp.t.MAX>>(lvl+1);
	uint t=lp.t.MASK&(th+dt);
	uint p=lp.p.MASK&(ph-dt);

	// make center node (diamonds stage)

	if(cdata==0){
		CD4;
		cdata=new MapData(t,p);
		init_map_data(cdata);
		dec_alevel();
		TheMap->mcreated++;
		if(TheMap)
			TheMap->size++;
		clr_bchecked();
		return 0;
	}

	// make child lattice nodes (squares stage)

	c=new MapNode(this, cdata);
	md=cdata;
	cdata=0;
	// find d(down) node
	d=dn->rnode;
	IFNEW(d,ctr,rnode){
		MAPDATA(dn,ctr,dn);

		mapdata[rccnt++]=&data;
		a=TheMap->makenode(this,t,dn->data.lphi());
		MKLINKS(d,a,rnode,lnode);
	}

	// find r(right) node
	r=rt->dnode;
	IFNEW(r,ctr,dnode){
		MAPDATA(rt,ctr,rt);

		a=TheMap->makenode(this,rt->data.ltheta(),p);
		MKLINKS(r,a,dnode,unode);
	}

	// find u(up) node
	u=rnode;
	IFNEW(u,rt,rnode){
		a=CWup();
		MAPDATA(this,rt,a);
		a=TheMap->makenode(this,t,ph);
		MKLINKS(u,a,rnode,lnode);
	}

	// find l(left) node
	l=dnode;
	IFNEW(l,dn,dnode){
		a=CCWleft();
		MAPDATA(this,dn,a);
		a=TheMap->makenode(this,th,p);
		MKLINKS(l,a,dnode,unode);
	}

	u->clr_nflags();
	l->clr_nflags();
	r->clr_ccheck();
	d->clr_ccheck();

	// set node links

	c->dnode=d; d->unode=c;
	c->unode=u; u->dnode=c;
	c->lnode=l; l->rnode=c;
	c->rnode=r; r->lnode=c;
	if(md)
		delete md;

	clr_bchecked();
	if(need_recalc())
	   recalc2();
	return new MapLink(l,c,u);
}

int    MapNode::find_neighbors(){
    mdcnt=0;
    for(int i=0;i<MAX_NDATA;i++)
    	mapdata[i]=0;
	if(rnode && lnode){
		mapdata[mdcnt++]=&(rnode->data);
		mapdata[mdcnt++]=&(lnode->data);
		if(dnode && !unode)
			mapdata[mdcnt++]=&(dnode->data);
	}
	if(dnode && unode){
		mapdata[mdcnt++]=&(dnode->data);
		mapdata[mdcnt++]=&(unode->data);
		if(rnode && !lnode)
			mapdata[mdcnt++]=&(rnode->data);
	}
	return mdcnt;
}


int MapNode::count_neighbors(){
    int cnt=0;
	if(rnode && lnode){
		cnt+=2;
		if(dnode && !unode)
			cnt++;
	}
	if(dnode && unode){
		cnt+=2;
		if(rnode && !lnode)
			cnt++;
	}
	return cnt;
}

//-------------------------------------------------------------
// MapNode::recalc2() recalculate surface
//-------------------------------------------------------------
void MapNode::recalc2()
{
	mdcnt=find_neighbors();
	rccnt=mdcnt;
	if(mdcnt>2){
		Td.init();
		Td.level=elevel();
		Td.p=Point(data.tbase(),data.pbase(),1);
		Td.c=TheMap->object->color();
		mdctr=&data;
		mdctr->invalidate();
		TheMap->object->set_surface(Td);
		data.free();
		data.init_terrain_data(Td,0);
		if(Td.get_flag(INMARGIN))
	    	set_margin(1);
		if(Td.get_flag(HIDDEN))
		    set_hidden(1);
		set_need_recalc(0);
	}
	//else{
	//	set_need_recalc(1);
	//}
}

//-------------------------------------------------------------
// MapNode::test_backfacing()	test if cell is back facing
//-------------------------------------------------------------
void MapNode::test_backfacing()
{
	MapNode *dn,*rt,*ctr;
	MapData *m,*d,*c,*r,*n;

	//if(bchecked())
	//	return;
	if(!dnode || !rnode)
		return; // not a lattice node

	if(TheMap->frontface==GL_FRONT_AND_BACK)
	    return;
	n=surface_data(&data);
	//n=&data;

	dn=CCWdown();

	d=surface_data(&(dn->data));
	//d=&(dn->data);

	rt=CWright();

	r=surface_data(&(rt->data));
	//r=&(rt->data);

	ctr=rt->CWdown();

	c=surface_data(&(ctr->data));
	//c=&(ctr->data);

	if(cdata)
		m=cdata;
	else
		m=&(ctr->data);
	m=surface_data(m);

	if(!n||!m||!r||!d||!c)
		return;
	clr_backfacing();
	Point norm;
	double dp;
	int front=(TheMap->frontface==GL_FRONT);

    set_bchecked();
	Point p=n->mpoint();

	Point pr=r->mpoint();
	Point pd=d->mpoint();
	Point pm=m->mpoint();

	Point vr=pr-p;
	Point vd=pd-p;
	Point vm=pm-p;

	Point vp=TheScene->vpoint-p;

	norm=vr.cross(vm);
	dp=norm.dot(vp);

	if((front&&(dp>0)) || (!front && (dp<0)))
		set_backfacing(DBF);

	norm=vm.cross(vd);
	dp=norm.dot(vp);

	if((front&&(dp>=0)) || (!front && (dp<0)))
		set_backfacing(DBF);

	if(!cdata)
		return;

	vr=pr-pm;
	vd=c->mpoint()-pm;
	vp=TheScene->vpoint-pm;
	norm=vr.cross(vd);
	dp=norm.dot(vp);
	if((front&&(dp>=0)) || (!front && (dp<0)))
		set_backfacing(CBF);
}
//-------------------------------------------------------------
// MapNode::clipchk()	return 1 if vertex is clipped by the current
//						view window, else return 0
//-------------------------------------------------------------
int MapNode::clipchk(Point pnt)
{
	double x,y,z,t,dist;
	double size=CELLSIZE(elevel());
	double esize=size*2;

    if(!TheMap->object->local_group())
        return 1;

	Point p=pnt.mm(TheScene->viewMatrix);
	z=-p.mz(TheScene->lookMatrix);	// z (depth z projection)

    if(z<TheScene->znear-esize)
        return 1;

    double m=esize/z;

	t=1+m;

	y=p.my(TheScene->projMatrix)/z;
    if(y < -t || y >t)
        return 1;

    t*=TheScene->aspect;
	x=p.mx(TheScene->projMatrix)/z;
    if(x < -t|| x >t)
        return 1;

#ifdef ZCLIP
	dist=TheScene->epoint.distance(p);
	if(TheMap->dmax>0&& dist>TheMap->dmax+esize)
        return 1;
#endif
    if(TheScene->viewobj==TheMap->object){
        return 0;
    }

    if(TheMap->object->inside())
        return 0;

    double h,r,d1,d2;
    Object3D *vobj=TheScene->viewobj;
    Object3D *pobj=TheMap->object->getObjParent();

#ifdef VCLIP

    // test for intersection with viewobj

    if(vobj && vobj->allows_selection()){

        h=2*vobj->getMap()->hscale;
//        Object3D *obj=TheMap->object;
//        double zf=vobj->getMap()->dmax; // add contribution from radius of parent
//        double s=obj->size;
//		double d=TheScene->epoint.distance(obj->point);
		//h+=10*s*zf/d;

        r=vobj->size*(1-h);
		if(p.intersect_sphere(TheScene->epoint,vobj->point,r,d1,d2)>0){
	    	if(d1>0 && d1<1)
	        	return 1;
	    }
	    if(vobj==pobj)
	        return 0;
    }
#endif
#ifdef PCLIP
    // test for intersection with own parent object

    if(pobj && pobj->allows_selection() && pobj->getMap()){
        h=2*pobj->getMap()->hscale;
        r=pobj->size*(1-h);
		if(p.intersect_sphere(TheScene->epoint,pobj->point,r,d1,d2)>0){
	    	if(d1>0 && d1<1)
	        	return 1;
	    }
    }
#endif
	return 0; // visible
}

//-------------------------------------------------------------
// Maxa()   utility - find max level
//-------------------------------------------------------------
static double maxa=0;
static void Maxa(MapNode *n)
{
	double f=n->elevel();
	if(f>maxa)
		maxa=f;
}

//-------------------------------------------------------------
// MapNode::set_tests()	set split/combine flags in adapt
//-------------------------------------------------------------
void MapNode::set_tests()
{
	double depth;
	double size;
	double	f,ext,max,min;

	clr_split();
	clr_combine();

	if(TheMap->fixed_grid()){
		if(alevel()<TheMap->grid_size())
			set_split();
		else if(alevel()>TheMap->grid_size())
			set_combine();
		return;
	}
	if(!dnode || !rnode)
		return;
#ifdef USEMAXA
	max=elevel();
	if(max>=PLVLS-1){
		set_combine();
		return;
	}
	maxa=max;
	CWcycle(Maxa);
	if(maxa>max+1){
		set_split();
		return;
	}
#endif
	// find extent of node (pixels)

	Point mpnt = DPOINT(this);
	Point dpnt;
	Point rpnt;
	if(cdata){
		dpnt=CPOINT(this);
		size=mpnt.distance(dpnt);
	}
	else{
		dpnt=DPOINT(CCWdown());
		rpnt=DPOINT(CWright());
		size=mpnt.distance(dpnt);
		double s2=mpnt.distance(rpnt);
		if(s2<4*size)  // improves problem at poles
			size = s2;
		//size=size>s2?size:s2;
	}
	f=CELLSIZE(elevel());

    // return if cellsize if smallest possible
 /*
 	if(TheMap->idmap() && !visible() && f>TheMap->smin){
		if(clipchk())
			set_clipped();
		else {
			clr_clipped();
			clr_masked();
			set_partvis();
			set_nodraw(0);
			set_tlevel(Adapt.normal());
		}
	}
 */
	if((uint)alvl()>=lp.t.MAXBIT-1)
	    return;

	if(f>TheMap->smin)
		size=8*f;
	else if(size >30*f)  // prevent runaway slope
		size=f;
    depth=TheScene->vpoint.distance(mpnt);

	MapData *d=data.surface1();

	// if a sub-surface layer has an X or Y component
	// use surface closest to viewpoint for cell depth
	if(TheScene->viewobj==TheMap->object){
		while(d){
			d=d->next_surface();
			if(d && d->dims()>1){
				double mx=TheScene->epoint.distance(d->mpoint());
				depth=mx<depth?mx:depth;
			}
		}
	}
	double min_depth=3*FEET;
	depth=depth<min_depth?min_depth:depth;

	ext=TheScene->wscale*(size/depth); // perspective scaling

	if(ext<TheMap->minext)
		TheMap->minext=ext;

	max=TheMap->resolution*Adapt.tlevel(tlevel());
	//min=max/15.0;
	min=max/5.0;
	if(ext<Adapt.minext()){
	    return;
	}
	if(ext > max)
		set_split();
	//else if(ext < min  && size<TheMap->smin)
	else if(ext < min)
		set_combine();

}

//-------------------------------------------------------------
// MapNode::vischeck()	test if cell is clipped or backfacing
//-------------------------------------------------------------
void MapNode::vischk()
{
	clr_aflags();
    set_nodraw(0);
    set_mchecked();
	clr_masked();
	if(Adapt.clip_test()){
	    clr_clipped();
	    // tag node as clipped only is all surfaces
	    // are clipped.
		MapData *d=data.surface1();
		bool clp=false;
		while(d){
			Point p=d->mpoint();
			if(!clipchk(p)){
				clp=false;
				break;;
			}
			d=d->next_surface();
		}
		if(clp){
			set_clipped();
			set_masked();
			return;
		}
	}

	if(Adapt.back_test()){
		test_backfacing();
		if(backfacing()){
			cout << "." << endl;
		    //set_backfacing();
		    return;
		}
	}

	if(visible()){
		TheMap->vnodes++;
	}
}

//-------------------------------------------------------------
// MapNode::pvischeck()	test if cell is neighbored by visible cells
//-------------------------------------------------------------
static int pvflag;
static int level=0;
const int maxlvl=2;
static void testvis(MapNode *n)
{
	if(n->visible()){
		pvflag=1;
		return;
    }
	level++;
	if(level<maxlvl)
		n->CWcycle(testvis);
}

void MapNode::pvischk()
{
	set_nodraw(0);
	level=0;
	if(!visible()){
	    set_nodraw(1);
		pvflag=0;
		CWcycle(testvis);
	    if(pvflag){
			set_partvis();
			set_nodraw(0);
		}
	}
}
//-------------------------------------------------------------
// MapNode::sizechk()  set "target" cellsize level (tlevel) based
//                     on visibility and curvature tests
//-------------------------------------------------------------
void MapNode::sizechk()
{
	// if using the pixel mask buffer

	if(TheMap->idmap()){
		if(invisible()&& !partvis())
			set_tlevel(Adapt.hidden());
		else if(!invisible()&& partvis())
			set_tlevel(Adapt.sharp());
		else if(Adapt.curve_test()){
		   if(!cchecked())
			   curvechk();
	    }
	    else
			set_tlevel(Adapt.normal());
	}
	// not using the pixel mask buffer
	else{
		if(hidden())
		   set_tlevel(Adapt.smooth());
		else
		if(masked()){// clipped by frame
		    if(partvis())
				set_tlevel(Adapt.normal());
			else
				set_tlevel(Adapt.hidden());
		}
		else  {						// frontfaced
			if(Adapt.curve_test() && !cchecked())
				curvechk();
			else
				set_tlevel(Adapt.normal());
		}
	}
}

//-------------------------------------------------------------
// MapNode::balance()	balance tlevels
//-------------------------------------------------------------
void	MapNode::balance()
{
	unsigned int tmp=tlevel()+1;
	if(unode && unode->tlevel()>tmp)
		{unode->set_tlevel(tmp);unode->balance();}
	if(dnode && dnode->tlevel()>tmp)
		{dnode->set_tlevel(tmp);dnode->balance();}
	if(lnode && lnode->tlevel()>tmp)
		{lnode->set_tlevel(tmp);lnode->balance();}
	if(rnode && rnode->tlevel()>tmp)
		{rnode->set_tlevel(tmp);rnode->balance();}
}

//-------------------------------------------------------------
// MapNode::curvechk()	do curvature tests
//-------------------------------------------------------------

#define CCHK(x,y,z) { \
		g1=x.distance(y); \
		g2=y.distance(z); \
		g3=x.distance(z); \
		err=1-g3/(g1+g2); \
		curv=err>curv?err:curv; }

#define TEST_COLOR(x)  \
	if(x && \
	    (data.type()!=x->data.type() \
	    || color().difference(x->color())>TheMap->gmax\
	    || fabs(density()-x->density())>TheMap->gmax\
	    )) { \
		set_tlevel(Adapt.sharp());return 0;}
#define TEST_CCOLOR()  \
	if(data.type()!=cdata->type() \
	    || color().difference(cdata->color())>TheMap->gmax \
	    || fabs(density()-cdata->density())>TheMap->gmax \
	    )\
	   { \
		set_tlevel(Adapt.sharp());return 0;}

int MapNode::curvechk()
{
	if(invisible()||hidden())
		return 1;

	double  g1=0,g2=0,g3=0,err,curv=0;
	MapNode *r=0,*d=0;
	double cmin,cmax;
	int done=0;

	set_ccheck();

	if(!Adapt.curve_test()){
		set_tlevel(Adapt.normal());
		return 1;
	}

	r=CWright();
	TEST_COLOR(r);
	d=CCWdown();
	TEST_COLOR(d);

	Point p=DPOINT(this);

	if(cdata){
		TEST_CCOLOR();
		MapNode *a=0;
		if(r && d)
			CCHK(DPOINT(d),CPOINT(this),DPOINT(r));
		if(r)
			a=r->CWdown();
		if(!a && d)
			a=d->CCWright();
		if(a){
			TEST_COLOR(a);
			CCHK(p,CPOINT(this),DPOINT(a));
		}
		done=1;
	}
	if(!done){
		if(rnode && rnode->rnode){
			TEST_COLOR(rnode->rnode);
			Point rr=DPOINT(rnode->rnode);
			CCHK(p,DPOINT(rnode),rr);
			if(lnode && lnode->lnode){
				TEST_COLOR(lnode->lnode);
				CCHK(DPOINT(lnode->lnode),p,rr);
			}
		}
		if(lnode && (lnode !=rnode) && rnode){
			TEST_COLOR(lnode);
			CCHK(DPOINT(lnode),p,DPOINT(rnode));
		}
		if(dnode && dnode->dnode){
			TEST_COLOR(dnode->dnode);
			Point dd=DPOINT(dnode->dnode);
			CCHK(p,DPOINT(dnode),dd);
			if(unode && unode->unode){
				TEST_COLOR(unode->unode);
				CCHK(DPOINT(unode->unode),p,dd);
			}
		}
		if(unode && dnode){
			TEST_COLOR(unode);
			CCHK(DPOINT(unode),p,DPOINT(dnode));
		}
	}
	cmax=TheMap->cmax;
	cmin=TheMap->cmin;

	if(curv > cmax )
		set_tlevel(Adapt.sharp());
	else if(curv <=cmin)
		set_tlevel(Adapt.smooth());
	else
		set_tlevel(Adapt.normal());

	return 0;
}

//-------------------------------------------------------------
// MapNode::gradchk()	do edge test
//-------------------------------------------------------------
int MapNode::gradchk()
{
    return 0;
}
#define TEST_VISIT
//-------------------------------------------------------------
// MapNode::visit()	visit terminal nodes only and apply function
//-------------------------------------------------------------
void MapNode::visit(void (MapNode::*func)())
{
#ifdef TEST_VISIT
	visit_all(func);
#else
	if(link)
		link->visit(func);
	if(!link || !TheMap->leaf_cycle())
		(this->*func)();
#endif
}
void MapNode::visit(void (*func)(MapNode*))
{
#ifdef TEST_VISIT
	visit_all(func);
#else
	if(link)
		link->visit(func);
	if(!link || !TheMap->leaf_cycle())
		(*func)(this);
#endif
}

//-------------------------------------------------------------
// MapNode::visit_all()	visit all nodes and apply function
//-------------------------------------------------------------
void MapNode::visit_all(void (MapNode::*func)())
{
	MapNode *node;
	if(link)
		link->visit_all(func);

	(this->*func)();

	// visit mid-point nodes on left and upper edges

	if(dnode){
		node=rnode;
		while(node && node->dnode==0){
			(node->*func)();
			node=node->rnode;
		}
		node=dnode;
		while(node && node->rnode==0){
			(node->*func)();
			node=node->dnode;
		}
	}
}
void MapNode::visit_all(void (*func)(MapNode*))
{
	MapNode *node;
	if(link)
		link->visit_all(func);

	(*func)(this);

	// visit mid-point nodes on left and upper edges

	if(dnode){
		node=rnode;
		while(node && node->dnode==0){
			(*func)(node);
			node=node->rnode;
		}
		node=dnode;
		while(node && node->rnode==0){
			(*func)(node);
			node=node->dnode;
		}
	}
}
#define MOVETO(r,u,func) \
	while(n && n->r==0){ \
		(n->*func)(); \
		n=n->u; \
	}

//-------------------------------------------------------------
// MapNode::CWcycle()	visits all border nodes of the cell by
//						traversing a clockwise path around the
//						cell's outer edge.
//-------------------------------------------------------------
void MapNode::CWcycle(void  (MapNode::*func)())
{
	MapNode *n;

	if(dnode==0 || rnode==0)
		return;

	n=rnode;
	MOVETO(dnode,rnode,func);
	(n->*func)();

	n=n->dnode;
	MOVETO(lnode,dnode,func);
	(n->*func)();

	n=n->lnode;
	MOVETO(unode,lnode,func);
	(n->*func)();

	n=n->unode;
	MOVETO(rnode,unode,func);
	if(n!=this)
		(n->*func)();
}

//-------------------------------------------------------------
// MapNode::CWcycle()	visits all border nodes of the cell by
//						traversing a clockwise path around the
//						cell's outer edge.
//-------------------------------------------------------------
void MapNode::CWcycle(void  (*func)(MapNode*))
{
	MapNode *n;

	if(dnode==0 || rnode==0)
		return;

	n=rnode;
	while(n && n->dnode==0){
		(*func)(n);
		n=n->rnode;
	}
	(*func)(n);

	n=n->dnode;
	while(n && n->lnode==0){
		(*func)(n);
		n=n->dnode;
	}
	(*func)(n);
	n=n->lnode;
	while(n && n->unode==0){
		(*func)(n);
		n=n->lnode;
	}

	(*func)(n);

	n=n->unode;
	while(n && n->rnode==0){
		(*func)(n);
		n=n->unode;
	}
	if(n!=this)
		(*func)(n);
}

//-------------------------------------------------------------
// MapNode::visit_cycle() apply function to nodes in render cycle order
// - called by add_triangles
//-------------------------------------------------------------
// move to corner node while applying function
#define AMOVE(r,u) \
	while(n && n->r==0){ \
		(*func)(&n->data); \
		n=n->u; \
	}
//  move to corner node without applying function
#define VMOVE(r,u) \
	while(n && n->r==0){ \
		n=n->u; \
	}
void MapNode::visit_cycle(void  (*func)(MapData*))
{
	MapNode *n;
	if(dnode==0 || rnode==0)
		return;
	TheMap->current==this;
	n=rnode;
	if(cdata){
		(*func)(cdata);
		(*func)(&data);
		AMOVE(dnode,rnode);
		(*func)(&n->data);
	}
	else{ // move to first corner before creating triangles
		(*func)(&data);
		VMOVE(dnode,rnode);
		(*func)(&n->data);
	}
	n=n->dnode;
	AMOVE(lnode,dnode);
	(*func)(&n->data);

	n=n->lnode;
	AMOVE(unode,lnode);
	(*func)(&n->data);
	//if(cdata || n!=this){
	if(cdata){
		n=n->unode;
		AMOVE(rnode,unode);
		(*func)(&n->data);
	}
}

//-------------------------------------------------------------
// MapNode::ave_point()	return ave point in cell;
//-------------------------------------------------------------
static Point _apt;
static int _apcnt;
void add_pt(MapNode *n){
   _apt=_apt+Point(n->theta(),n->phi(),n->height());
   _apcnt++;
}

Point MapNode::ave_point()
{
    _apt=Point(0,0,0);
    _apcnt=0;
    CWcycle(add_pt);
    return _apt*(1.0/_apcnt);
}

//-------------------------------------------------------------
// MapNode::locate()	return *MapNode closest to theta & phi
//-------------------------------------------------------------
MapNode *MapNode::locate(double t, double p)
{
	MapNode *n;
	MapLink *l;

	n=this;
	l=n->link;
	n=this;
	l=n->link;

	while(l){
		if(l->rt && l->rt->theta() <= t){
			if(l->ctr && (l->ctr->phi()>=p))
				n=l->ctr;
			else
				n=l->rt;
			l=n->link;
		}
		else if(l->dn && (l->dn->phi()>=p)){
			n=l->dn;
			l=n->link;
		}
		else
			l=l->lwr;
	}
	return n;
}
//-------------------------------------------------------------
// MapNode::neighbors()	set neighbor nodes
//-------------------------------------------------------------
int MapNode::neighbors()
{
	int     n=0;

	// get direct neighbors

	if(unode)
		Neighbor[n++]=unode;
	if(dnode)
		Neighbor[n++]=dnode;
	if(lnode)
		Neighbor[n++]=lnode;
	if(rnode)
		Neighbor[n++]=rnode;

	// get corner neighbors

	if(unode && unode->lnode)
		Neighbor[n++]=unode->lnode;
	else if(lnode && lnode->unode)
		Neighbor[n++]=lnode->unode;
	if(lnode && lnode->dnode)
		Neighbor[n++]=lnode->dnode;
	else if(dnode && dnode->lnode)
		Neighbor[n++]=dnode->lnode;

	if(unode && unode->rnode)
		Neighbor[n++]=unode->rnode;
	else if(rnode && rnode->unode)
		Neighbor[n++]=rnode->unode;
	if(rnode && rnode->dnode)
		Neighbor[n++]=rnode->dnode;
	else if(dnode && dnode->rnode)
		Neighbor[n++]=dnode->rnode;

	return n;
}

//-------------------------------------------------------------
// MapNode::grid_walk()	return *MapNode closest to theta & phi
//-------------------------------------------------------------
MapNode *MapNode::grid_walk(double t, double p)
{
	int i,n;
	double d,x,y,dmin=10000;
	MapNode *closest=0;
	n=neighbors();

	for(i=0;i<n;i++){
		x=Neighbor[i]->phi()-p;
		y=Neighbor[i]->theta()-t;
		d=sqrt(x*x+y*y);
		if(d<dmin){
			closest=Neighbor[i];
			dmin=d;
		}
	}
	x=phi()-p;
	y=theta()-t;
	d=sqrt(x*x+y*y);

	if(d<dmin || this==last_grid)
//	if(d<dmin)
		return this;

	last_grid=this;
	return closest->grid_walk(t,p);
}

//-------------------------------------------------------------
// MapNode::init_render()	initialize render pass for all nodes
//-------------------------------------------------------------
void MapNode::init_render()
{
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	TheMap->texpass=0;

	Dmid=&MapNode::vertex;
	Dstart=&MapNode::vertex;
	Cmid=&MapNode::vertex;
	Cstart=&MapNode::vertex;

	if(TheScene->select_mode()){
		Dmid=&MapNode::vertexP;
		Dstart=&MapNode::vertexP;
		Cmid=&MapNode::vertexP;
		Cstart=&MapNode::vertexP;
	}
    else if(Render.draw_ids()){
//		Cstart=&MapNode::vertex;
//		Dstart=&MapNode::vertex;
//		Dmid=&MapNode::vertex;
//		Cmid=&MapNode::vertex;
		Cstart=&MapNode::IDvertex;
		Dstart=&MapNode::IDvertex;
		Dmid=&MapNode::IDvertex;
		Cmid=&MapNode::IDvertex;
	}
	else if(Render.draw_zvals() || Render.draw_szvals()){
		Cstart=&MapNode::IDvertex;
		Dstart=&MapNode::IDvertex;
		Dmid=&MapNode::IDvertex;
		Cmid=&MapNode::IDvertex;
//		Cstart=&MapNode::vertex;
//		Dstart=&MapNode::vertex;
//		Dmid=&MapNode::vertex;
//		Cmid=&MapNode::vertex;
	}
	else if(Render.draw_normals()){
		Cstart=&MapNode::vertexN;
		Dstart=&MapNode::vertexN;
		Dmid=&MapNode::vertexN;
		Cmid=&MapNode::vertexN;
	}
	else if(Render.draw_raster()){
		Cstart=&MapNode::Rvertex;
		Dstart=&MapNode::Rvertex;
		Dmid=&MapNode::Rvertex;
		Cmid=&MapNode::Rvertex;
	}
	else if(Render.draw_shaded()){
		Cstart=&MapNode::Svertex;
		Dstart=&MapNode::Svertex;
		Dmid=&MapNode::Svertex;
		Cmid=&MapNode::Svertex;
	}
	else if(!TheMap->lighting) {
		Cstart=&MapNode::vertexC;
		Dstart=&MapNode::vertexC;
		if(Render.smooth_shading()){
			Dmid=&MapNode::vertexC;
			Cmid=&MapNode::vertexC;
		}
	}
	else if(Render.draw_front()) {
		Cstart=&MapNode::vertexCN;
		Dstart=&MapNode::vertexCN;
		if(Render.smooth_shading()){
			Dmid=&MapNode::vertexCN;
			Cmid=&MapNode::vertexCN;
		}
	}
	else{
	   printf("unknown render mode!\n");
	}
}

//-------------------------------------------------------------
// MapNode::render_cycle()	render the cell using open-gl triangles
//-------------------------------------------------------------
void MapNode::render_cycle()
{
	MapNode *n;

	if(!Render.draw_ids())
		rcycles++;

	if(cdata)
		(this->*Cstart)(cdata);

	(this->*Dstart)();

	n=rnode;
	MOVETO(dnode,rnode,Dmid);
	(n->*Dmid)();

	n=n->dnode;
	MOVETO(lnode,dnode,Dmid);
	(n->*Dmid)();

	n=n->lnode;
	MOVETO(unode,lnode,Dmid);
	(n->*Dmid)();

	n=n->unode;
	MOVETO(rnode,unode,Dmid);
	if(cdata || n!=this)
		(n->*Dmid)();
}

//-------------------------------------------------------------
// MapNode::render()	render the cell using open-gl triangles
//-------------------------------------------------------------
void MapNode::render()
{
	MapData *d;

	TheMap->current=this;

	if(link && !TheMap->leaf_cycle())
		link->render();


	if(!rnode || !dnode){
	    return;
	}

	TheMap->current=this;

	if(cdata)
		d=cdata;
	else
		d=&data;

    d=surface_data(d);

	if(!d)
	   return;

//	if(DRAW_VIS && nodraw()){
//	    if(d->X()==0 && d->Y()==0 && !margin() && Render.show_water() && Raster.water_depth())
//	    	return;
//	}

    Phi = d->phi()/180;
    Theta = d->theta()/180.0-1;
    MapPt = d->mpoint();

	for(int i=0;i<TheMap->tp->textures.size;i++){
		Texture *t;
		t=TheMap->tp->textures[i];
		if(t->t2d()){
			t->svalue=Phi;
			t->tvalue=Theta;
		}
		else{
			t->svalue=t->s;
			t->tvalue=t->t;
		}
	}

#ifdef USE_GL_POLYGON
	glBegin(GL_POLYGON);
#else
	glBegin(GL_TRIANGLE_FAN);
#endif
	render_cycle();
	glEnd();
}
//-------------------------------------------------------------
// MapNode::render_ids()	render call for raster modes
//-------------------------------------------------------------
void MapNode::render_ids()
{
	TheMap->current=this;
	if(link)
		link->render_ids();
	TheMap->current=this;
	if(!rnode || !dnode){
		clr_masked();
	    return;
	}

	MapData *d;

	if(cdata)
		d=cdata;
	else
		d=&data;

    d=surface_data(d);
	if(!d)
	   return;
	MapPt = d->mpoint();
	set_mchecked();
	idb.l=Raster.set_id();
	Raster.set_data(this);
	glColor4ub(idb.c.red,idb.c.green,idb.c.blue,0);

#ifdef USE_GL_POLYGON
	glBegin(GL_POLYGON);
#else
	glBegin(GL_TRIANGLE_FAN);
#endif
	render_cycle();
	glEnd();
}

//-------------------------------------------------------------
// MapNode::render_vertex()	render call for raster modes
//-------------------------------------------------------------
void MapNode::render_vertex()
{
	TheMap->current=this;

	if(link)
		link->render_vertex();
	TheMap->current=this;
	if(!rnode || !dnode)
	    return;
	if(TheScene->select_node()){
		idb.l=Raster.set_id();
		Raster.set_data(this);
		glLoadName(idb.l);
	}

	MapData *d;

	if(cdata)
		d=cdata;
	else
		d=&data;

    d=surface_data(d);
	if(!d)
	   return;
	MapPt = d->mpoint();
#ifdef USE_GL_POLYGON
	glBegin(GL_POLYGON);
#else
	glBegin(GL_TRIANGLE_FAN);
#endif
	render_cycle();
	glEnd();
}

//-------------------------------------------------------------
// MapNode::Tcolor() test color for output
//-------------------------------------------------------------
Color MapNode::Tcolor(MapData *d) {
    Color c = BLACK;

    double a = c.alpha();
    switch (Render.colors()) {
    case CSIZE:						// '1'
        c = Adapt.tcolor(alevel());
        break;

    case CNODES:					// '2'
    {
        double depth;
        double size = cell_size();
        int ext = 10;
        depth = TheScene->vpoint.distance(DPOINT(this));

        if (depth > 0)
            ext = (int) (TheScene->wscale * (size / depth)); // perspective scaling

        c = Adapt.tcolor(ext);
    }
        break;

    case VNODES:                  	// '3'
        if (partvis())
            c = Color(1, 1, 0);
        else if (hidden())
            c = Color(0, 0, 1);
        else if (invisible() && rnode && dnode)
            c = Color(1, 0, 0);
        else
            c = Color(1, 1, 1);
        break;

    case LNODES:                  	// '4'
        if (Raster.surface == 2)
            c = Color(0, 0, 1);
        else {
            c = sctbl[d->type() & 0xf];
            if (dual_terrain())
                c = c.darken(0.2);
            if (has_water())
                c = c.blend(Color(0, 0, 1), 0.25);
        }
        break;

    case ENODES:// '5'
    {
    	//cout<<Margin<<endl;

    	   c = Color(data.mdata(), 0, 0);
 //        if (Raster.surface == 2)
//            c = Color(0, 0, 1);
//        else {
//            if (dual_terrain()) {
//                if (d && d->water())
//                    d = d->data2();
//                if (!d)
//                    c = Color(0, 0, 1);
//                else if (d->next_surface()) {
//                    if (!d->next_surface())
//                        c = Color(1, 0, 0);
//                    else
//                        c = Color(0, 0, 1);
//                } else
//                    c = Color(0, 1, 0);
//            } else
//                c = Color(1, 1, 1);
//            if (has_water())
//                c = c.blend(Color(0, 0, 1), 0.25);
//            //if (margin())
//           //    c = c.blend(Color(1, 0, 0), 1);
//            if (data.margin())
//                c = c.blend(Color(0, 0, 1), 1);
        }
        break;
    case MNODES:                  	// '6'
    {
 	   if (data.margin())
  		   c = Color(0, 0, 1);
 	   else
 		   c= Color(1,1, 1);

 	/*
     double s = 2*sediment();
     c = Color(1, 1, 1);
     if (s > 0)
         c = c.blend(Color(0, 0, 1), s);
     else
         c = c.blend(Color(1, 0, 0), -s);
         */

    }
        break;
    case RNODES:
    {

        Point *np = 0;
        MapData *md;
        if (cdata)
            md = cdata;
        else
            md = &data;
        np = pnt_normal(md);

        c = Color(1, 1, 1);
        if (!np)
            c = Color(0, 1, 0);
        else {
            Point norm = md->mpoint().normalize();
            Point vp = TheScene->vpoint - md->mpoint();
            vp = vp.normalize();
            double dp = norm.dot(vp);
            dp=dp<0?0:dp;
            c = Color(dp, 0, 0);

//            if (dp < 0) {
//                if (cdata)
//                    c = Color(0, 0, 1);
//                else
//                    c = Color(1, 0, 0);
//            }
        }
    }
        break;
    case SNODES: {
        Point *np = 0;
        MapData *md;
        if (cdata)
            md = cdata;
        else
            md = &data;

        Texture *tex=TheMap->tp->textures[0];
        if(tex){
     		double phi = md->phi() / 180;
    		double theta = md->theta() / 180.0 - 1;
     		tex->s=phi;
    		tex->t=theta;
    		tex->svalue=0;
    		tex->tvalue=0;
    		tex->scale=1;


        	double x,y;
        	tex->getTexCoords(x,y);
        	int mode=tex->intrp();
        	FColor fc=tex->timage->color(mode, x+1,y);
        	fc=fc.grayscale();
        	c=Color(fc.red(),c.green(),c.blue(),1.0);
        }
/*
        np = pnt_normal(md);

        c = Color(1, 1, 1);
        if (!np)
            c = Color(0, 1, 0);
        else {
            Point norm = np->normalize();
            Point vp = md->mpoint();
            vp = vp.normalize();
            double dp = 1-norm.dot(vp);
            c = c.blend(Color(1, 0, 0), 2*dp);
        }
*/
    }
        break;
    }
    c.set_alpha(a);
    return c;
}

//-------------------------------------------------------------
// MapNode::Vcolor() color for output
//-------------------------------------------------------------
void MapNode::Vcolor(MapData *dd)
{
	MapData *d=surface_data(dd);
	if(!d)
		return;

	Color c=BLACK;

	if(TheMap->colors()){
		c=d->color();
		TheMap->object->map_color(d,c);
	}
	else if(TheMap->textures())
	    c=WHITE;

	if(Render.colors())
		c=Tcolor(d);
	else if(TheMap->textures() && d->textures())
		d->apply_texture();
	if(c.hasAlpha())
		glColor4d(c.red(),c.green(),c.blue(),c.alpha());
	else
		glColor4d(c.red(),c.green(),c.blue(),1);
}

//-------------------------------------------------------------
// MapNode::vertexZ()	 surface fast z vertex
//-------------------------------------------------------------
void MapNode::vertexZ(MapData*d)
{
	Point p=d->point();
	glVertex3dv((double*)(&p));
}

//-------------------------------------------------------------
// MapNode::vertex()	calls open-gl Vertex
//-------------------------------------------------------------
//#define DEBUG_EROSION
void MapNode::vertex(MapData*d)
{
	d=surface_data(d);
	if(!d)
		return;

#ifdef DEBUG_EROSION
	Point p;
	if(TheScene->render_mode() && Render.showsed()){
	    if(Render.showsed()==1)
			p=TheMap->point(d->theta(),d->phi(),d->Z());
	    else if(Render.showsed()==2)
			p=TheMap->point(d->theta(),d->phi(),d->sediment());
	    p=p-TheScene->xpoint;
	}
	else
		p=d->point();
#else
	Point p=d->point();
#endif
	glVertex3dv((double*)(&p));
}

//-------------------------------------------------------------
// MapNode::setVertexAttributes() set up cor shader noise
//-------------------------------------------------------------
void MapNode::setVertexAttributes(MapData*d){
	if(!d)
		return;
	double depth = TheScene->vpoint.distance(d->mpoint());

	Point pm=d->mpoint();
	pm=pm.normalize();  // this gets rid of the Z() component
	pm=pm*0.5+0.5;
	// set pm to Vertex1 in shaders
	// - pm contains just the (rectangularized) phi&theta values of the point

	GLSLMgr::input_type=GL_TRIANGLES;
	GLSLMgr::output_type=GL_TRIANGLE_STRIP;

	GLSLMgr::setVertexAttributes(pm,depth);

}

//-------------------------------------------------------------
// MapNode::IDvertex()	 for ID, normals or zbuffer
//-------------------------------------------------------------
void MapNode::IDvertex(MapData*d)
{
	d=surface_data(d);
	if(!d)
		return;
	Point p=d->point();

	setVertexAttributes(d);

	glVertex3dv((double*)(&p));  // contains Z() (global scale)
}

//-------------------------------------------------------------
// MapNode::normal() return effective vertex normal
//-------------------------------------------------------------
Point *MapNode::normal(MapData*d)
{
	Point *p=0;

	if(Render.avenorms())
		p=ave_normal(d);
	else
		p=pnt_normal(d);
	return p;
}
//-------------------------------------------------------------
// MapNode::vertexCN() calls gl Color, Normal and Vertex functions
//-------------------------------------------------------------
void MapNode::vertexCN(MapData*d)
{
	Point *p=normal(d);
	if(p)
		glNormal3dv((double*)p);
	Vcolor(d);
	vertex(d);
}

//-------------------------------------------------------------
// MapNode::vertexN()	calls open-gl Vertex and Normal functions
//-------------------------------------------------------------
void MapNode::vertexN(MapData*d)
{
	setVertexAttributes(surface_data(d));

	Point *p=normal(d);
	if(p){
		glNormal3dv((double*)p);
    }
	vertex(d);
}

//-------------------------------------------------------------
// MapNode::Svertex()	node visit function for shader pass
//-------------------------------------------------------------
void MapNode::Svertex(MapData*dn) {
	MapData*d=surface_data(dn);

	bool lighting=TheMap->lighting;

	if(!d)
		return;

	Point pt=d->point();

	Point *n=0;
	static Point norm;

	if(lighting){
		n=normal(d);
	}

	Color c = d->color();

	if(Render.colors())
		c=Tcolor(d);
	else
		TheMap->object->map_color(d, c);

	TerrainProperties *tp=TerrainData::tp;

	if(n){
		norm=*n;
		glNormal3dv(norm.values());
	}

	glColor4d(c.red(), c.green(), c.blue(), c.alpha());

	if (d->type() != TheMap->tid){
		glVertex3dv((double*)(&pt));
		return;
	}
	// reduce max orders for low view angles
	double dpfactor=1;
	//if(norm){
		Point dv=TheScene->eye-pt;
		dv=dv.normalize();
		dpfactor=rampstep(0,1.0,norm.dot(dv),0.2,1);
	//}
	// reduce max orders with distance
	double depth = TheScene->vpoint.distance(d->mpoint());
	double dfactor=0.5*GLSLMgr::wscale/depth;

	double max_orders =log2(dfactor);
	double phi = d->phi() / 180;
	double theta = d->theta() / 180.0 - 1;
	double g=d->type()+1.5;

	double ht=d->Z()*Rscale;  // global units (MM)

	if(TheMap->object!=TheScene->viewobj)
		g = 0;
	if (GLSLMgr::CommonID1 >= 0){
		glVertexAttrib4d(GLSLMgr::CommonID1, ht, g,d->density(), theta);
	}

	setVertexAttributes(d);

	if(d->textures() || d->bumpmaps()){
		if (GLSLMgr::TexCoordsID >= 0){
			find_neighbors();
			Td.clr_flag(SFIRST);
			double zs=zslope();
			Point T=tangent(d);
			glVertexAttrib4d(GLSLMgr::TexCoordsID, T.x, T.y, zs, max_orders);
			//cout<<max_orders<<endl;
		}

		Texture *tx;
		int index = d->tstart();
		int num_attribs=0;
		double A[8]={1.0};
		int texid = 0;
		double s = 0;
		double t = 0;
		for (int j = 0; j < tp->textures.size; j++) {
			s=t=0;
			tx = tp->textures[j];
			if (!tx->tex_active && !tx->bump_active)
				continue;
			if (tx->t2d()) {
				tx->s = phi;
				tx->t = theta;
				if(tx->s_data){
					s = d->texture(index);
					tx->s+=s;
					tx->t+=s;
				}
				if(tx->a_data){
					t = d->texture(index);
					if(tx->d_data)
						t*=d->mdata();
					A[texid]=clamp(t,0.0,1.0);
					num_attribs++;
				}
				else if(tx->d_data){
					t=d->mdata();
					A[texid]=clamp(t,0.0,1.0);
					num_attribs++;
				}
			} else if(tx->cid>=0){
				if (tx->s_data)
					s = d->texture(index);
				else
					s=0;
				tx->s = s;

				if(tx->a_data){
					t = d->texture(index);
					if(tx->d_data)
						t*=d->mdata();
					A[texid]=clamp(t,0.0,1.0);
					num_attribs++;
				}
				else if(tx->d_data){
					t=d->mdata();
					//cout<<t<<endl;
					A[texid]=clamp(t,0.0,1.0);
					num_attribs++;
				}
			}
			if(tx->cid>=0)
				tx->texCoords(GL_TEXTURE0 + tx->cid);
			texid++;
		}
		if(GLSLMgr::attributes3ID >= 0 && num_attribs>0)
			glVertexAttrib4d(GLSLMgr::attributes3ID, A[0], A[1], A[2], A[3]);
		if(GLSLMgr::attributes4ID >= 0 && num_attribs>0)
			glVertexAttrib4d(GLSLMgr::attributes4ID, A[4], A[5], A[6], A[7]);
	}
	glVertex3dv((double*)(&pt));
}

//-------------------------------------------------------------
// MapNode::TanGroup() returns tangent vertexes
//-------------------------------------------------------------
bool MapNode::TanGroup(MapData **v)
{
	MapData *nd=ndata();
	MapData *d=surface_data(nd);
	if(nd!=&(data)){  // cnode
		Pgroup(v);
	}
	else{
		Vgroup(v);
		if(v[GUP]==0)
			v[GUP]=d;
		if(v[GDN]==0)
			v[GDN]=d;
		if(v[GRT]==0)
			v[GRT]=d;
		if(v[GLT]==0)
			v[GLT]=d;
	}
	if(v[GUP]&&v[GDN]&&v[GRT]&&v[GLT])
		return true;
	return false;
}

//-------------------------------------------------------------
// MapNode::TanGroup() returns tangent
//-------------------------------------------------------------
Point MapNode::tangent(MapData *d)
{
	double dphi=0.001; // a small offset

	Point p1=Point(d->theta(),d->phi()+dphi,0);
	Point p2=Point(d->theta(),d->phi()-dphi,0);
	Point V1 = p2 - p1;
	return V1.normalize();
}
//-------------------------------------------------------------
// MapNode::normal() calculates cell normal on-the-fly using
//					 hash table to avoid calculation replication
//-------------------------------------------------------------
extern int cntest;
Point *MapNode::pnt_normal(MapData *nd)
{
	MapData *r,*n,*d;
	MapNode *rt,*dn;
	if(!nd || !dnode || !rnode){
		return 0;
	}
	if(nd)
	    n=surface_data(nd);
	if(!n)
		return 0;

#ifdef HASH_NORMALS
	Hdata *hd=normals.hash(n->id(),n);
	if(hd->adrs != n){
#else
	if(!n->normal_valid())	{
#endif
		rt=CWright();
		if(nd==&data){
			r=rt->surface_data(&(rt->data));
			if(!r){
				return 0;
			}
			if(cdata)
				d=surface_data(cdata);
			else{
			    dn=rt->CWdown();
				d=dn->surface_data(&(dn->data));
			}
		}
		else{
			if(rt->cdata)
				r=rt->surface_data(rt->cdata);
			else
				r=rt->surface_data(&(rt->data));
			if(!r){
				return 0;
			}
			dn=rt->CWdown();
			d=dn->surface_data(&(dn->data));
		}
		if(!d){
			return 0;
		}
		Point c=n->point();
#ifdef HASH_NORMALS
		Point norm=c.normal(r->point(),d->point());
		hd->adrs=n;
		hd->point=norm;
		return &hd->point;
	}
	else
		return &hd->point;
#else
        n->normal_=c.normal(r->point(),d->point());
        n->set_normal_valid(1);
	}
    return &(n->normal_);
//        normal_=c.normal(r->point(),d->point());
//        return &(normal_);
#endif
}
//-------------------------------------------------------------
// MapNode::ave_normal() calculate average normal vector for the node
//-------------------------------------------------------------
Point *MapNode::ave_normal(MapData *nd)
{
	MapData *d=surface_data(nd);

	if(d  &&  d->normal_valid())
		return &(d->normal_);
#ifndef HASH_NORMALS
	return 0;

#else
	if(!dnode|| !rnode)
		return 0;
	Point ave;
	Hdata *hd=normals.hash(data.id(),&data);
	if(hd->adrs !=&data){
		hd->adrs=&data;
		MapData *sd=surface_data(d);
		if(!sd)
			return 0;
		Point c=sd->point();

		MapData *p[4]={0};
		if(d==&data)
			Lgroup(p);
		else
			Pgroup(p);

		if(p[GRT] && p[GDN])
			ave=ave+c.normal(p[GRT]->point(),p[GDN]->point());
		if(p[GUP] && p[GRT])
			ave=ave+c.normal(p[GUP]->point(),p[GRT]->point());
		if(p[GLT] && p[GUP])
			ave=ave+c.normal(p[GLT]->point(),p[GUP]->point());
		if(p[GDN] && p[GLT])
			ave=ave+c.normal(p[GDN]->point(),p[GLT]->point());

		hd->point=ave.normalize();
	}
	return &hd->point;
#endif
}

double MapNode::max_height() {
	double gz=getGZ();
	double z1=data.height();
	double z2=0;
	double ht=z1+gz;
    //if(TheScene->select_mode())
    //    cout<<" gz:"<<gz/FEET<<" z1:"<<z1/FEET;
	MapData *d=&data;
    if(data.water()){
    	d=data.data2();
    	z2=d->height();
    	if(TheScene->select_mode())
    		cout<<" z2:"<<z2/FEET;
    	if((z2+gz)>z1)
    		ht=z2+gz;
    	else
    		ht=z1;
    }
    if(TheScene->select_mode())
       cout<<" ht:"<<ht/FEET<<endl;

	return ht;
}

//-------------------------------------------------------------
// MapNode::init_data()	set MapData by calling terrain generator
//-------------------------------------------------------------
void MapNode::init_map_data(MapData *md)
{
	Td.init();
	//	Td.reset();
	//	Td.level=0;
	//	Td.fractal=0;
	Td.level=elevel();

	Td.p=Point(md->tbase(),md->pbase(),1);
	Td.c=TheMap->object->color();
	Td.clr_flag(FNOREC);

	mdctr=md;
	TheMap->object->set_surface(Td);
	md->init_terrain_data(Td,0);
	if(Td.get_flag(INMARGIN))
	    set_margin(1);
	if(Td.get_flag(HIDDEN))
	    set_hidden(1);

	if(Adapt.recalc()&&!Td.get_flag(FNOREC))
		set_need_recalc(1);
}

