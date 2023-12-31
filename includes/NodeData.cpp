#include "SceneClass.h"
#include "MapNode.h"
#include "MapLink.h"
#include "NoiseClass.h"
#include "AdaptOptions.h"
#include "RenderOptions.h"
#include "Effects.h"
#include "matrix.h"
#include "Sprites.h"
#include "Plants.h"
//**************** extern API area ************************

extern int     hits, misses, visits;
extern double  Rscale, Gscale, Pscale, Height,MaxHt,MinHt,FHt;
extern double  zslope();
extern int test_flag;
extern Point MapPt;


static TerrainData Td;
//**************** static and private *********************

// uncomment to get indicated behavior

// show hash table diagnostics
#define DEBUG_NORMALS  0   // .. for normals
#define DEBUG_POINTS   0   // .. for points

//========  hash table utility classes ====================

#define NORMALS_HASHSIZE 1024
#define POINTS_HASHSIZE  1024*32

#ifdef HASH_NORMALS
HashTable normals(NORMALS_HASHSIZE,DEBUG_NORMALS);
#endif

#ifdef HASH_POINTS
static HashTable points(POINTS_HASHSIZE,DEBUG_POINTS);
#endif

void invalidate_normals()
{
#ifdef HASH_NORMALS
	normals.init();
	Render.validate_normals();
#endif
}
//-------------------------------------------------------------
// render_setup()		do render setup operations
//-------------------------------------------------------------
void render_setup()
{
#ifdef HASH_NORMALS
	if(Render.invalid_normals())
		normals.init();
	Render.validate_normals();
#endif
#ifdef HASH_POINTS
	if(Render.invalid_points())
		points.init();
#endif
	Render.validate_points();
}

//-------------------------------------------------------------
// render_setup()		do render finsh operations
//-------------------------------------------------------------
void render_finish()
{
#ifdef HASH_NORMALS
	Render.validate_normals();
#endif
}

//************************************************************
// MapData class
//************************************************************
int MapData::gid=0;
//-------------------------------------------------------------
// MapData::MapData()	constructor
//-------------------------------------------------------------
MapData::MapData(uint t, uint p) : lt(t),lp(p)
{
	flags.l=0;
	data=0;
	set_id(gid++);
	set_type(ID0);
	invalidate_normal();
}

//-------------------------------------------------------------
// MapData::MapData()	constructor (from cdata)
//-------------------------------------------------------------
MapData::MapData(MapData *p)
{
	data=p->data;
	lt=p->lt;
	lp=p->lp;
	p->data=0;
	flags.l=p->flags.l;
	set_id(gid++);
#ifndef HASH_POINTS
	point_=p->point_;
#endif
	//invalidate_normal();
}

//-------------------------------------------------------------
// MapData::~MapData()
//-------------------------------------------------------------
MapData::~MapData()
{
	free();
}

void MapData::free()
{
   // if(!data)
   //     return;
#ifdef HASH_POINTS
	points.remove(id());
#endif
#ifdef HASH_NORMALS
	normals.remove(id());
#endif
	if(data && links()){
		setLinks(0);
        MapData *md=data2();
		DFREE(md);
	}
	FREE(data);
}

//-------------------------------------------------------------
// MapData::invalidate_normal()
//-------------------------------------------------------------
void  MapData::invalidate_normal()
{
#ifdef HASH_NORMALS
	normals.remove(id());
#else
	normal_=Point(0,0,0);
	set_normal_valid(0);
#endif
}
//-------------------------------------------------------------
// MapData::validate_normal()
//-------------------------------------------------------------
void  MapData::validate_normal()
{
#ifndef HASH_NORMALS
	if(normal_.length()>0){
		normal_=normal_.normalize();
		set_normal_valid(1);
	}
#endif
}

//-------------------------------------------------------------
// MapData::invalidate_point()
//-------------------------------------------------------------
void  MapData::invalidate_point()
{
#ifdef HASH_POINTS
	points.remove(id());
#endif
}

//-------------------------------------------------------------
// MapData::invalidate()
//-------------------------------------------------------------
void  MapData::invalidate()
{
#ifdef HASH_NORMALS
	normals.remove(id());
#endif
#ifdef HASH_POINTS
	points.remove(id());
#endif
}
//-------------------------------------------------------------
// MapData::point()	return rectangular point from theta,phi,height
//-------------------------------------------------------------
Point  MapData::point()
{
#ifdef HASH_POINTS
	Hdata *hdata=points.hash(id(),this);
	if(hdata->adrs !=this){
		hdata->adrs=this;
		hdata->point=TheMap->point(theta(),phi(),Ht());
	}
	if(TheScene->eyeref())
		return hdata->point-TheScene->xpoint;
	return hdata->point;
#else
	if(TheScene->eyeref())
		return point_-TheScene->xpoint;
	return point_;
#endif
}

//-------------------------------------------------------------
// MapData::mpoint() return point centered at model origin
//-------------------------------------------------------------
Point  MapData::mpoint()
{
#ifdef HASH_POINTS
	Hdata *hdata=points.hash(id(),this);
	if(hdata->adrs !=this){
		hdata->adrs=this;
		hdata->point=TheMap->point(theta(),phi(),Ht());
	}
	return hdata->point;
#else
	return point_;
#endif
}

//-------------------------------------------------------------
// MapData::height()		return ht of node (global units)
//-------------------------------------------------------------
double MapData::height(){
    return Z()*Rscale;
}

//-------------------------------------------------------------
// MapData::pvector() phi direction vector
//-------------------------------------------------------------
Point MapData::pvector()
{
	Point pt=TheMap->point(theta(),phi()+1e-6,Ht())-point();
	if(TheScene->eyeref())
		return pt-TheScene->xpoint;
	else
	    return pt;
}

//-------------------------------------------------------------
// MapData::tvector() theta direction vector
//-------------------------------------------------------------
Point MapData::tvector()
{
	Point pt=TheMap->point(theta()+1e-6,phi(),Ht())-point();
	if(TheScene->eyeref())
		return pt-TheScene->xpoint;
	else
	    return pt;
}

//-------------------------------------------------------------
// MapData::init_terrain_data()	set node data after surface call
//-------------------------------------------------------------
#define TEST_SPRITES
#define TEST_PLANTS
//#define TEST_CRATERS
#define TEST_DENSITY

//#define TEST_COLOR

void MapData::init_terrain_data(TerrainData &td,int pass)
{
	int nd=0;
	int nc=0;
	int nf=0;
	int ne=0;
	int ng=0;
	int frac=0;
	int md=0;
	int nw=0;

	int i;
	int a,b;
	double density=0;
	Td.pass=pass;

	MapData *s2=0;

	if(td.get_flag(HIDDEN))
		set_hidden(1);

	if(td.get_flag(FVALUE))
	    frac=1;
	if(td.get_flag(MULTILAYER))
		md=1;
	if(td.depth)
		ne=1;
	if(td.get_flag(EVALUE))
	    ne=2;
	if(td.get_flag(DVALUE)){
		density=td.s;
		nf=1;
	}
	else if(td.density!=0.0){
		nf=1;
		density=td.density;
	}
	if(td.water())
		nw=1;

	if(td.cvalid())
		nc=1;

	setLinks(0);
    if(td.datacnt && pass<td.datacnt){
        s2=new MapData(lt.lvalue(),lp.lvalue());
	    s2->init_terrain_data(*td.data[pass],pass+1);
	    setLinks(1);
    }
    
	Height=td.p.z;

	int ttype=td.type();
	TerrainProperties *tp=td.properties[ttype];

#if defined TEST_CRATERS 
	nc=1;
#endif
	int pm=CurrentScope->passmode();
	bool do_sprites=Raster.sprites()&&Raster.adapt_sprites()&&tp->sprites.size>0&& TheScene->viewobj==TheMap->object;
#if defined TEST_SPRITES
	if(do_sprites){
#if defined TEST_COLOR
		nc=1;
#endif
#if defined TEST_DENSITY
		nf=1;
#endif
	}
#endif

#ifdef TEST_PLANTS
	bool do_plants=tp->plants.size>0&& TheScene->viewobj==TheMap->object;
	if(do_plants){
#ifdef TEST_COLOR
		nc=1;
#endif
#ifdef TEST_DENSITY
		nf=1;
#endif
	}
#endif

	a=b=0;

	setTextures(tp->textures.size?1:0);
	setSprites(tp->sprites.size?1:0);
	int nbumps=0;
	int nmaps=0;

	for(int i=0;i<tp->textures.size;i++){
		Texture *tx=tp->textures[i];
		if(tx->bump_active)
			nbumps++;
		if(tx->hmap_active)
			nmaps++;
	}
	setBumpmaps(nbumps?1:0);
	setHmaps(nmaps?1:0);
	
	if(td.p.y != 0.0)
	    nd=3;
	else if(td.p.x != 0.0)
	    nd=2;
	else if(td.p.z != 0.0)
	    nd=1;

	setDims(nd);
	setColors(nc);
	set_has_density(nf);
	set_has_ocean(nw);

	setEvals(ne);
	setFchnls(frac);
	setMdata(md);

	int n=nc+nd+nw+ne+nf+frac+md+links();
	a=tp->tsize();
	setMemory(n,a);

	setColor(td.c);
	setDensity(density);

	setZ(td.p.z);
	setX(Pscale*td.p.x);
	setY(Pscale*td.p.y);
	setFractal(td.fractal);
	setMargin(td.margin);
	if(td.water()){
		setOcean(td.ocean);
	}
	if(td.water()||td.depth)
		setDepth(td.depth);
	else{
		setSediment(td.sediment);
		setRock(td.rock);
		if(ne>0)
			TheMap->set_erosion(1);
	}
	double h=Ht();

	a=TSTART;

	for(i=0;i<tp->textures.size;i++){
		Texture *tex=tp->textures[i];
		tex->eval();
		if(tex->s_data){
			setTexture(tex->s,a);
			if(tex->a_data)
				setTexture(tex->avalue,a);
		}
		else if(tex->a_data){
			setTexture(tex->avalue,a);
		}
		if(md)
			tex->d_data=true;
	}
	if(h>TheMap->hmax){
		TheMap->hmax=h;
		//cout<<"hmax="<<h<<endl;
	}
	if(h<TheMap->hmin){
		TheMap->hmin=h;
		//cout<<"min="<<h<<endl;
	}
	if(td.get_flag(INEDGE))
		set_edge(1);
	if(td.get_flag(INMARGIN))
		set_margin(1);

	set_type(td.type());
    if(td.datacnt && pass<td.datacnt)
		setLink(s2);
#ifndef HASH_POINTS
	point_=TheMap->point(theta(),phi(),h);
#endif
	int mode=CurrentScope->passmode();
#ifdef TEST_SPRITES
	if(do_sprites){
		CurrentScope->set_spass();
		MapPt=point();
		Td.density=0;
		Td.diffuse=Color(1,1,1);
		for(i=0;i<tp->sprites.size;i++){
			Sprite *sprite=tp->sprites[i];
			sprite->eval();
	#ifdef TEST_DENSITY
			setDensity(Td.density);
	#endif
	#ifdef TEST_COLOR
			setColor(Td.diffuse);
	#endif
		}
	}
#endif
#ifdef TEST_PLANTS
	if(do_plants){
		CurrentScope->set_spass();
		MapPt=point();
		Td.density=0;
		Td.diffuse=Color(1,1,1);
		for(i=0;i<tp->plants.size;i++){
			Plant *plant=tp->plants[i];
			plant->eval();
	#ifdef TEST_DENSITY
			setDensity(Td.density);
	#endif
	#ifdef TEST_COLOR
			setColor(Td.diffuse);
	#endif
		}
	}
#endif
	CurrentScope->set_passmode(mode);
}
//-------------------------------------------------------------
// MapData::memsize() return storage size (bytes)
//-------------------------------------------------------------
int MapData::memsize()
{
	TerrainProperties *tp=Td.properties[type()];
#ifdef D64
    int n=sizeof(MapData)+2*(TSTART+tp->tsize());
#else
	int n=sizeof(MapData)+TSTART+TDSIZE*tp->tsize();
#endif
	return 4*n;
}

//-------------------------------------------------------------
// MapData::getTexIndex return index of texture
//-------------------------------------------------------------
int MapData::getTexIndex(Texture *tex, int &index)
{
	Array<Texture*> &texs=(TheMap->tp->textures);
	index=TSTART;
	Texture *tx=0;
	int i=0;
	for(i=0;i<texs.size;i++){ // find starting texture
		tx=texs[i];
		if(tx==tex)
			return i;
		if(tx->s_data||tx->a_data)
			index+=TDSIZE;
		if(tx->s_data&&tx->a_data)
			index+=TDSIZE;
	}
	return 0;
}

//-------------------------------------------------------------
// MapData::apply_texture() set current texture
//-------------------------------------------------------------
void MapData::apply_texture()
{
	Array<Texture*> &texs=(TheMap->tp->textures);
	int index;
	int i=getTexIndex(TheMap->texture, index);

    double p=phi()/180;
    double t=theta()/180.0-1;


	Texture *tx=texs[i];
	if(tx->t2d()){
		tx->s = p;
		tx->t = t;
	}
	else
		tx->s = texture(index)/tx->scale;

	tx->texCoords(GL_TEXTURE0);
}


//-------------------------------------------------------------
// MapData::span()	return MapData surface arc distance
//-------------------------------------------------------------
double MapData::span(MapData *d)
{
    double s1=lp.span(d->lp);
    double s2=lt.span(d->lt);
	return sqrt(s1*s1+s2*s2);
}
