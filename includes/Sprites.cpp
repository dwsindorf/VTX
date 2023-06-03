
//************************************************************
// classes SpritePoint, SpriteMgr
//************************************************************
#include "SceneClass.h"
#include "RenderOptions.h"
#include "Sprites.h"
#include "Util.h"
#include "MapNode.h"
#include "ModelClass.h"
#include "AdaptOptions.h"
#include "FileUtil.h"

extern double Hscale, Drop, MaxSize,Height;
extern double ptable[];

extern void inc_tabs();
extern void dec_tabs();
extern char   tabs[];
extern int addtabs;

static double cval=0;
static double mind=0;
static double htval=0;
static int ncalls=0;
static int nhits=0;
static double thresh=0.25;

static int cnt=0;

static TerrainData Td;
static SpriteMgr *s_mgr=0; // static finalizer
static int hits=0;
#define DEBUG_PMEM
//#define DEBUG
#ifdef DEBUG
static void show_stats()
{
	if(s_mgr)
		s_mgr->dump();
	cout<<"calls="<<ncalls<< " hits="<<nhits<<endl;
}
#endif
//************************************************************
// SpriteMgr class
//************************************************************
//	arg[0]  levels   		scale levels
//	arg[1]  maxsize			size of largest craters
//	arg[2]  mult			size multiplier per level
//	arg[3]  density			density or dexpr
//
//-------------------------------------------------------------
SpriteMgr::SpriteMgr(int i) : PlacementMgr(i)
{
#ifdef DEBUG
	if(!s_mgr)
		add_finisher(show_stats);
#endif
	s_mgr=this;
	//set_ntest(0);
}
SpriteMgr::~SpriteMgr()
{
  	if(finalizer()){
  		s_mgr=0;
#ifdef DEBUG_PMEM
  		printf("SpriteMgr::free()\n");
#endif
	}
}

//-------------------------------------------------------------
// SpriteMgr::init()	initialize global objects
//-------------------------------------------------------------
void SpriteMgr::init()
{
#ifdef DEBUG_PMEM
  	printf("SpriteMgr::init()\n");
#endif
	PlacementMgr::init();
	ncalls=0;
	nhits=0;
	cnt=0;
	ss();
  	//reset();
}

void SpriteMgr::eval(){
	//reset();
	PlacementMgr::eval();
}

//-------------------------------------------------------------
// SpriteMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *SpriteMgr::make(Point4DL &p, int n)
{
    return new SpritePoint(*this,p,n);
}

//************************************************************
// class SpritePoint
//************************************************************
SpritePoint::SpritePoint(PlacementMgr&mgr, Point4DL&p,int n) : Placement(mgr,p,n)
{
	ht=0;
	aveht=0;
	dist=1e16;
	visits=0;
	hits=0;
	mind=1e16;
	flags.s.active=false;
}

//-------------------------------------------------------------
// SpritePoint::set_terrain()	impact terrain
//-------------------------------------------------------------
bool SpritePoint::set_terrain(PlacementMgr &pmgr)
{
	double d=pmgr.mpt.distance(center);
	d=d/radius;
	SpriteMgr &mgr=(SpriteMgr&)pmgr;
 
	if(d>=thresh)
		return false;
	visits++;
    flags.s.active=true;
   // if(visits==1)
    aveht+=Height;
  
	cval=lerp(d,0,thresh,0,1);
	if(d<dist){
		ht=Height;
		dist=d;
		mind=d;	
		hits++;
	}
//	if(visits==1)
//		c=Color(0,0,1);
//	else
//		c=Color(1-cval,0,0);
	::hits++;
//	glColor4d(c.red(), c.green(), c.blue(), c.alpha());
	return true;
}

void SpritePoint::reset(){
	flags.s.active=0;
	visits=0;
	hits=0;
	dist=1e6;
	aveht=0;
}
void SpritePoint::dump(){
	if(flags.s.valid && flags.s.active){
		Point4D p(point);
		p=center;
		char msg[256];
		char vh[32];
		sprintf(vh,"%d:%d",visits,hits);
		sprintf(msg,"%-3d %-2d %-8s dist:%-0.4f ht:%-1.6f x:%-1.5f y:%-1.5f z:%1.5f",cnt++,flags.l,vh,dist,ht,p.x,p.y,p.z);
		cout<<msg<<endl;
	}
}
//==================== SpriteData ===============================
SpriteData::SpriteData(SpritePoint *pnt){
	Point4D	p(pnt->center);
	//p=p.normalize();
	Point pp=Point(p.x,p.y,p.z);

	Point ps=pp.spherical();
	
	ht=pnt->ht;//aveht/pnt->visits;
	aveht=pnt->aveht/pnt->visits;
	center=TheMap->point(ps.x, ps.y, ht);
	radius=pnt->radius;
	id=pnt->type;
	
	Point t=Point(-center.x,center.y,-center.z); // why the 180 rotation around y axis ????

	distance=TheScene->vpoint.distance(t);
}

void SpriteData::print(){
	char msg[256];
	Point ps=center.spherical();
	double h=TheMap->radius*TheMap->hscale;
	
	sprintf(msg,"t:%-1.3g p:%-1.3g ht:%-1.4f dist:%g",ps.x,ps.y,h*ht/FEET,distance/FEET);
	cout<<msg<<endl;
	
}
//===================== Sprite ==============================
//-------------------------------------------------------------
// Sprite::Sprite() Constructor
//-------------------------------------------------------------
Sprite::Sprite(Image *i, int l, TNode *e)
{
	options=l;
    image=i;
	expr=e;
}

//-------------------------------------------------------------
// Sprite::collect() collect valid sprite points
//-------------------------------------------------------------
void Sprite::collect()
{
	SpriteMgr *smgr=(SpriteMgr*)expr->mgr;
	smgr->ss();
	sprites.free();
	SpritePoint *s=(SpritePoint *)smgr->next();
	LinkedList<SpriteData*> list;
	while(s){
		if(s->flags.s.valid && s->flags.s.active){
			list.add(new SpriteData(s));
		}
		s=smgr->next();
	}
	sprites.set(list); // copy pointers to array
	sprites.sort();
	cout<<" collected "<<sprites.size<<" sprites"<<endl;
	for(int i=0;i<sprites.size;i++){
		cout<<i<<" ";
		sprites[i]->print();	
	}
	//expr->collect();
}
//-------------------------------------------------------------
// Sprite::eval() evaluate TNtexture string
//-------------------------------------------------------------
void Sprite::eval()
{
	int mode=CurrentScope->passmode();
	CurrentScope->set_spass();
	expr->eval();
	CurrentScope->set_passmode(mode);
}

bool Sprite::setProgram(){
	return false;
}
bool Sprite::initProgram(){
	return false;
}
void Sprite::begin(){
	
}
void Sprite::end(){
	
}

//===================== TNsprite ==============================
//************************************************************
// TNsprite class
//************************************************************
TNsprite::TNsprite(char *s, TNode *l, TNode *r) : TNplacements(SPRITES,l,r,0)
{
    mgr=new SpriteMgr(type);
	set_collapsed();
	setName(s);
	FREE(s);
	image=0;
	sprite=0;
}

//-------------------------------------------------------------
// TNsprite::~TNsprite() destructor
//-------------------------------------------------------------
TNsprite::~TNsprite()
{
	DFREE(sprite);
}

//-------------------------------------------------------------
// TNsprite::init() initialize the node
//-------------------------------------------------------------
void TNsprite::init()
{
	SpriteMgr *smgr=(SpriteMgr*)mgr;
	image=images.load(sprites_file,JPG);
	if(!image){
		printf("TNsprites ERROR image %s not found\n",sprites_file);
		return;
	}
	if(sprite==0)
		sprite=new Sprite(image,SPRITE,this);
	smgr->init();
	TNplacements::init();
}

//-------------------------------------------------------------
// TNsprite::eval() evaluate the node
//-------------------------------------------------------------
void TNsprite::eval()
{	
	SINIT;
	S0.set_flag(TEXFLAG);
	S0.clr_constant();
	S0.set_cvalid();
	S0.clr_svalid();

	if(CurrentScope->rpass()){
		Td.add_sprite(sprite);
		return;
	}	

	if(!CurrentScope->spass()){
		return;	
	}

	SpriteMgr *smgr=(SpriteMgr*)mgr;
	//smgr->init();
	htval=Height;
	//cout<<"ht:"<<htval<<endl;

	ncalls++;

	TNplacements::eval();  // evaluate common arguments
	cval=0;
	hits=0;
	smgr->eval();  // calls SpritePoint.set_terrain
   
	Color c =Color(1,1,0);

	S0.clr_constant();
	S0.set_cvalid();
	S0.clr_svalid();

	if(hits>0){ // inside target radius
		if(fabs(cval)>0)
			c=Color(1-cval,0,0);
		nhits++;
	}
	glColor4d(c.red(), c.green(), c.blue(), c.alpha());
    Td.c=c;
	S0.c=c;

}
//-------------------------------------------------------------
// TNinode::setName() set name
//-------------------------------------------------------------
void TNsprite::setName(char *s)
{
	strcpy(sprites_file,s);
}
//-------------------------------------------------------------
// TNinode::setName() set name
//-------------------------------------------------------------
void TNsprite::setType(char *s)
{
	strcpy(sprites_type,s);
}

//-------------------------------------------------------------
// TNtexture::valueString() node value substring
//-------------------------------------------------------------
void TNsprite::valueString(char *s)
{
	sprintf(s+strlen(s),"%s(\"%s\",",symbol(),sprites_file);
	//char opts[64];
	//opts[0]=0;
	//if(optionString(opts))
	//	sprintf(s+strlen(s),"%s,",opts);
	TNarg *arg=(TNarg*)left;
	while(arg){
		arg->valueString(s+strlen(s));
		arg=arg->next();
		if(arg)
			strcat(s,",");
	}
	strcat(s,")");
}

//-------------------------------------------------------------
// TNsprite::save() archive the node
//-------------------------------------------------------------
void TNsprite::save(FILE *f)
{
	char buff[1024];
	buff[0]=0;
	valueString(buff);
	//if(addtabs)
	    fprintf(f,"\n%s",tabs);
	fprintf(f,"%s",buff);
	if(right)
		right->save(f);
}

//-------------------------------------------------------------
// TNsprite::setSpritesTexture() set sprite image texture
//-------------------------------------------------------------
void TNsprite::setSpritesTexture(){

}
void TNsprite::getSpritesFilePath(char *dir){
	char base[256];
  	File.getBaseDirectory(base);
 	sprintf(dir,"%s/Textures/Sprites/%s",base,sprites_file);
}
