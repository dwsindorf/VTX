
//************************************************************
// classes Sprite, SpriteMgr
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

static double dval=0;
static double cval=0;
static double hval=0;
static double htval=0;
static int ncalls=0;

static TerrainData Td;
static SpriteMgr *s_mgr=0; // static finalizer
static void show_stats()
{
	if(s_mgr)
		s_mgr->dump();
	cout<<"calls="<<ncalls<<endl;
}

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
	set_ntest(0);
	c=Color(1,1,1);
	if(!s_mgr)
		add_finisher(show_stats);
	s_mgr=this;
}
SpriteMgr::~SpriteMgr()
{
  	if(finalizer()){
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
}

void SpriteMgr::eval(){
	PlacementMgr::eval();
	dval=cval;
}

//-------------------------------------------------------------
// SpriteMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *SpriteMgr::make(Point4DL &p, int n)
{
    return new Sprite(*this,p,n);
}

//************************************************************
// class Sprite
//************************************************************
Sprite::Sprite(PlacementMgr&mgr, Point4DL&p,int n) : Placement(mgr,p,n)
{
//	SpriteMgr &m=(SpriteMgr&)mgr;
//
//	double d=m.mpt.distance(center);
//	d=d/radius;
//
	ht=0;
	dist=1e6;
	visits=0;
//	
//	d=d>1?1:d;
}

//-------------------------------------------------------------
// Sprite::set_terrain()	impact terrain
//-------------------------------------------------------------
bool Sprite::set_terrain(PlacementMgr &pmgr)
{
	double d=pmgr.mpt.distance(center);
	d=d/radius;
	SpriteMgr &mgr=(SpriteMgr&)pmgr;
	
	//d=d>1?1:d;
    //cval=0;
	if(d>=1.0)
		return false;
	cval=d;
	if(d<dist){
		ht=htval;
		dist=d;
	}
	visits++;

	return true;
}

void Sprite::dump(){
	if(flags.s.active)
	cout<<visits<<" d:"<<dist<<" ht:"<<ht<<endl;
}

//************************************************************
// TNsprite class
//************************************************************
TNsprite::TNsprite(char *s, TNode *l, TNode *r) : TNplacements(SPRITES,l,r,0)
{
    mgr=new SpriteMgr(type);
//	TNarg &args=*((TNarg *)left);
//	TNode *arg=args[3];
//	if(arg && (arg->typeValue() != ID_CONST))
//		mgr->dexpr=arg;
	set_collapsed();
	setName(s);
	FREE(s);
	image=0;
}

//-------------------------------------------------------------
// TNsprite::init() initialize the node
//-------------------------------------------------------------
void TNsprite::init()
{
	SpriteMgr *smgr=(SpriteMgr*)mgr;
	Image *timage=images.load(sprites_file,JPG);
	if(!timage){
		printf("TNsprites ERROR image %s not found\n",sprites_file);
		return;
	}
	if(texture==0)
		texture=new Texture(image,0,this);
	smgr->init();
	TNplacements::init();
}

//-------------------------------------------------------------
// TNsprite::eval() evaluate the node
//-------------------------------------------------------------
void TNsprite::eval()
{
	//if(!right)
	//	return;

	if(TheScene->adapt_mode()){
		if(right)
			right->eval();
		//return;
	}			
	ncalls++;
	//double z=S0.p.z;
	SpriteMgr *smgr=(SpriteMgr*)mgr;
	//smgr->init();
	htval=Height;

	TNplacements::eval();  // evaluate common arguments
	cval=0;
	smgr->eval();
	
	S0.set_cvalid();
	if(fabs(dval)>0)
		S0.c=Color(1-dval,0,0);
	else
		S0.c=Color(1,1,1);
	S0.set_pvalid();
	S0.p.z=Height;
	//}
//	INIT;
//	right->eval();
//	double z=S0.p.z;
//	//cout<<z<<endl;
//	TNplacements::eval();  // evaluate common arguments
//	smgr->ht=z;
//	smgr->eval();
//	right->eval();
//	S0.set_cvalid();
//	Color red(1,0,0);
//	S0.c=red.mix(WHITE, smgr->radius);

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
