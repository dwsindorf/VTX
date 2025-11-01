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
#include "Craters.h"
#include "Rocks.h"

//**************** extern API area ************************

extern int     hits, misses, visits;
extern double  Rscale, Gscale, Pscale, Height,MaxHt,MinHt,FHt,Hardness,Phi;
extern double  zslope();
extern int test_flag;
extern Point MapPt,Mpt;

extern PlantMgr g_pm;
extern SpriteMgr g_sm;

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

//#define TEST_COLOR

void MapData::init_terrain_data(TerrainData &td,int pass)
{
	int nd=0;
	int nc=0;
	int dns=0;
	int ne=0;
	int ng=0;
	int nf=0;
	int md=0;
	int nw=0;

	int i;
	int a,b;
	double density=0;
	Td.pass=pass;

	MapData *s2=0;

	if(td.get_flag(HIDDEN))
		set_hidden(1);
    
	if(td.get_flag(FVALUE)){
	    nf=Td.fids;
	    //cout<<nf<<endl;
	}
	if(td.get_flag(MULTILAYER))
		md=1;
	if(td.hardness())
		ne=1;
	else if(td.depth)
		ne=1;
	if(td.get_flag(EVALUE))
	    ne=2;
	if(td.get_flag(DVALUE)){
		density=td.s;
		dns=1;
	}
	else if(td.density!=0.0){
		dns=1;
		density=td.density;
	}
	setRock(td.get_flag(ROCKBODY));
	if(td.water())
		nw=1;
    bool color_valid=false;
    bool sprites_density_test=false;
    
    Color c;
	if(td.cvalid()){
		color_valid=true;
		c=td.c;
		nc=1;
	}

	setLinks(0);
    if(td.datacnt && pass<td.datacnt){
        s2=new MapData(lt.lvalue(),lp.lvalue());
	    s2->init_terrain_data(*td.data[pass],pass+1);
	    setLinks(1);
    }
    
	Height=td.p.z;

	int ttype=td.type();
	TerrainProperties *tp=td.properties[ttype];
	if(!tp){
		cout<<"ERROR tp=0! "<<ttype<<endl;
		return;
	}

	int pm=CurrentScope->passmode();
	
	bool test_plants=TheScene->viewobj==TheMap->object && g_pm.test();
	bool test_plant_color=test_plants && g_pm.testColor();
	bool test_plant_density=test_plants && g_pm.testDensity();

	bool test_sprites=TheScene->viewobj==TheMap->object && g_sm.test();
	bool test_sprite_color = test_sprites&& g_sm.testColor();
	bool test_sprite_density = test_sprites&& g_sm.testDensity();
		
	if(test_sprite_color || test_plant_color )
		nc=1;
	if(test_sprite_density || test_plant_density)
		dns=1;

	a=b=0;

	setTextures(tp->textures.size?1:0);
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

	set_has_density(dns);
	set_has_ocean(nw);

	setEvals(ne);
	setFchnls(nf);
	setMdata(md);

	int n=nc+nd+nw+ne+dns+nf+md+links();
	a=tp->tsize();
	setMemory(n,a);

	setColor(td.c);
	setDensity(density);

	setZ(td.p.z);
	setX(Pscale*td.p.x);
	setY(Pscale*td.p.y);

	for(int i=0;i<nf;i++){
		setFractal(Td.fracval[i],i);
	}
	setMargin(td.margin);
	if(td.water()){
		setOcean(td.ocean-1);
	}
    if(td.water()||td.depth)
		setDepth(td.depth);
    else if(td.hardness())
		setHardness(td.s);
	else {
		setSolid(td.rock);
		setSediment(td.sediment);
		if(ne>0)
			TheMap->set_erosion(1);
	}
    Hardness=hardness();
	double h=Ht();
	Phi=phi();

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
	}
	if(h<TheMap->hmin){
		TheMap->hmin=h;
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
	Mpt=point();
#endif
	int mode=CurrentScope->passmode();

    if(test_sprites){
		CurrentScope->set_spass();
		MapPt=point();
		if(test_sprite_density){
			Td.density=0;
		}
		if(test_sprite_color){
			if(color_valid)
				Td.diffuse=c;
			else
				Td.diffuse=Color(1,1,1);
		}
		for(i=0;i<Td.sprites.size;i++){
			Sprite *sprite=Td.sprites[i];
			sprite->eval();
			if(test_sprite_density)
				setDensity(Td.density);
			if(test_sprite_color)
				setColor(Td.diffuse);
		}
	}

	if(test_plants){
		MapPt=point();
		if(test_plant_density&&!test_sprite_density)
			Td.density=0;
		if(test_plant_color&&!test_sprite_color){
			if(color_valid)
				Td.diffuse=c;
			else
				Td.diffuse=Color(1,1,1);
		}
		for(i=0;i<Td.plants.size;i++){
			Plant *plant=Td.plants[i];
			plant->eval();
			if(test_plant_density)
				setDensity(Td.density+density);
			if(test_plant_color){				
				setColor(Td.diffuse);
			}
		}
	}
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
