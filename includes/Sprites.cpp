#include "SceneClass.h"
#include "RenderOptions.h"
#include "Sprites.h"
#include "Util.h"
#include "MapNode.h"
#include "ModelClass.h"
#include "AdaptOptions.h"
#include "FileUtil.h"
#include "GLSLMgr.h"
#include "Effects.h"

// BUGS/problems
// 1) need to disable neighbor test in PlacementMgr (FIXED)
//    - hash table full problem
//    - if hash table almost full closer sprites get overridden by more distant ones
//    - solution was to render nodes based on distance (closest last)
// 2) can only have 1 level (FIXED)
// 3) at maximum probability minimum density low if neighbors disabled
//    - compensated somewhat by adding additional levels
// 4) Occasionally some sprites disappear reappear as viewpoint moves
//   - but still see colored placement circles in debug mode
// 5) draw order sometimes fails (back to front)
//   - defined by distance from viewpoint
// 6) need to add small level offset (roff) otherwise sprites will overlap
//   - but this causes more jitter in sprite ht and "floating" sprites
//   - possibly because spread in xy uses same ht as first level
// 7) sometimes see grid like artifact (sprites arranged in lines)
// TODO
// 1) change sprite pointsize with distance (DONE)
// 2) set ht offset based on sprite actual size and distance (DONE)
// 3) implement method for multiple sprite types and multiple terrain types
// 4) create sprite wxWidgets gui
// 5) allow sprite images to be in different directory than 2d textures
// 6) implement multiple sprite lookup from single image
//    - Could be same sprite different views depending on camera angle
//    - or different sprites of similar types (trees etc)
// 7) implement random horizontal flip of sprite image (DONE)
//    - in fragment shader use vec2(1-l_uv.x,l_uv.y) based on random variable
// 8) implement environmental density (ht,slope) as in Texture class
// 9) implement lighting
//    - star distance + time of day (DONE)
//    - shadows (DONE)
//    - haze 
// coverage table  
// window 640x480 sprite size=1e-5 thresh=5 min_pts=2 neighbors=true
// -----------------------------------------------------------------------
// levels   #		ms      % coverage (est)
// 1		364		14      25
// 2		708		29      50
// 3		1052	48      75
// 4		1381	64      100
// 8        2590    133     >100
// 16  		4722	289     >100
// GUI idea: pull down menu max-coverage 25,50,75,100 ->1,2,3,4 levels
//           - may want to multiply actual levels by 1/level_mult
//************************************************************
// classes SpritePoint, SpriteMgr
//************************************************************

extern double Hscale, Drop, MaxSize,Height;
extern double ptable[];
extern Point MapPt;

extern void inc_tabs();
extern void dec_tabs();
extern char   tabs[];
extern int addtabs;

static double cval=0;
static double mind=0;
static double htval=0;
static int ncalls=0;
static int nhits=0;
static double thresh=5;    // move to argument ?
static double ht_offset=0.9; // move to argument ?

static double roff_value=0;
static double roff2_value=PI;

static double min_pts=2;

static int cnt=0;
static int tests=0;
static int fails=0;

static TerrainData Td;
static SpriteMgr *s_mgr=0; // static finalizer
static int hits=0;
//#define DEBUG_PMEM
#define TEST
//#define SHOW
#define MIN_VISITS 1
#define TEST_NEIGHBORS 1
#define TEST_PTS 
//#define DUMP
#ifdef DUMP
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
#ifdef DUMP
	if(!s_mgr)
		add_finisher(show_stats);
#endif
	type|=SPRITES;
	s_mgr=this;
	roff=roff_value;
	roff2=roff2_value;
	level_mult=1;
	set_ntest(TEST_NEIGHBORS);
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
  	reset();
}

void SpriteMgr::eval(){
	PlacementMgr::eval();
	//reset();
}

void SpriteMgr::reset(){
	PlacementMgr::reset();
	tests=fails=0;
}
//-------------------------------------------------------------
// SpritePoint::set_terrain()	impact terrain
//-------------------------------------------------------------
bool SpriteMgr::valid()
{ 
#ifndef TEST_PTS
	return true;
#endif
	if(TheScene->adapt_mode())
		return true;
	Point pv=MapPt;
	double d=MapPt.length();
	double r=TheMap->radius*size;
	double f=TheScene->wscale*r/d;
    double pts=f;
    tests++;
	//return true;
    if(pts<2*min_pts){
    	fails++;
    	return false;
    }

	return true;
}

bool SpriteMgr::setProgram(){
	TerrainProperties *tp=Td.tp;
	char defs[1024]="";
	sprintf(defs+strlen(defs),"#define NSPRITES %d\n",tp->sprites.size);
	sprintf(defs+strlen(defs),"#define NLIGHTS %d\n",Lights.size);
	if(Render.haze())
		sprintf(defs+strlen(defs),"#define HAZE\n");

    bool do_shadows=Raster.shadows() && (Raster.twilight() || Raster.night());
	if(do_shadows && !TheScene->light_view()&& !TheScene->test_view() &&(Raster.farview()))
		sprintf(defs+strlen(defs),"#define SHADOWS\n");

	GLSLMgr::setDefString(defs);
	
	GLSLMgr::loadProgram("sprites.ps.vert","sprites.ps.frag");
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;
	glEnable(GL_TEXTURE_2D);
    //glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SPRITE);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	GLSLVarMgr vars;
	
	Planetoid *orb=(Planetoid*)TheScene->viewobj;
	Color diffuse=orb->diffuse;
	Color shadow=orb->shadow_color;
	Color haze=Raster.haze_color;
	
	vars.newFloatVec("Diffuse",diffuse.red(),diffuse.green(),diffuse.blue(),diffuse.alpha());
	vars.newFloatVec("Shadow",shadow.red(),shadow.green(),shadow.blue(),shadow.alpha());
	vars.newFloatVec("Haze",haze.red(),haze.green(),haze.blue(),haze.alpha());
	vars.newFloatVar("haze_zfar",Raster.haze_zfar);
	vars.newFloatVar("haze_grad",Raster.haze_grad);
	vars.newFloatVar("haze_ampl",Raster.haze_hf);
	
	double zn=TheScene->znear;
	double zf=TheScene->zfar;
	double ws1=1/zn;
	double ws2=(zn-zf)/zf/zn;

	vars.newFloatVar("ws1",ws1);
	vars.newFloatVar("ws2",ws2);

	vars.setProgram(program);
	vars.loadVars();
	GLSLMgr::setProgram();
	GLSLMgr::loadVars();
	for(int i=0;i<tp->sprites.size;i++){
		TerrainProperties::sid=i;
		tp->sprites[i]->setProgram();
	}
	
	double rand_flip_prob=0.5;
	glBegin(GL_POINTS);
	
	// render the sprites
	int scaleup=10000;
	int scaledn=100;
	int n=Sprite::sprites.size;
	int flip=0;
	for(int i=n-1;i>=0;i--){
		SpriteData *s=Sprite::sprites[i];
		int id=s->get_id();
		Point t=s->center;
		double f=s->pntsize;
		
		// 50% random reflection 
		// based on sprite hash table center position
		Point4DL pp=s->point;
		if(s->flip()){
			flip=Random(pp.x,pp.y,pp.z)>0?0:1;
			//if(i<10)
			//	printf("%d flip:%d x:%d y:%d z:%d dist:%-1.2f\n ",i,flip,pp.x,pp.y,pp.z,s->distance/FEET);	
		}
		glVertexAttrib4d(GLSLMgr::TexCoordsID,id, flip, f, 1);  // TODO: use w as lookup for grid image
		Point pn=Point(pp.x,pp.y,pp.z);
		pn=pn.normalize();
	    glVertex3dv(t.values());
	    glNormal3dv(pn.values());
	    //if(i<10)
		//printf("%d x:%-1.5g y:%-1.5g z:%-1.5g d:%-4.1f r:%-4.1f s:%-1.5f\n ",n-i,t.x,t.y,t.z,s->distance/FEET,s->radius/FEET,f);	
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);
	return true;
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
	// breaks visits >1 fixes sprite visibility in TEST
	//type|=SPRITES;//mgr.options;
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
	cval=0;
	
	if(d>thresh)
		return false;
	visits++;
    flags.s.active=true;
    aveht+=Height;
	//dist=d;

	cval=lerp(d,0,thresh,0,1);
	if(d<dist){
		ht=Height;
		dist=d;
		mind=d;	
		hits++;
	}
	::hits++;
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
	if(visits>=MIN_VISITS && flags.s.valid && flags.s.active){
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
SpriteData::SpriteData(SpritePoint *pnt,Point vp, double d, double ps){
	type=pnt->type;
	ht=pnt->ht;
	
	point=pnt->point;
	
	aveht=pnt->aveht/pnt->visits;
	center=vp;
	radius=pnt->radius;
    pntsize=ps;
 	distance=d;//TheScene->vpoint.distance(t);
	visits=pnt->visits;
}

void SpriteData::print(){
	char msg[256];
	Point ps=center.spherical();
	double h=TheMap->radius*TheMap->hscale;
	
	sprintf(msg,"visits:%-1d t:%-1.3g p:%-1.3g ht:%-1.4f aveht:%-1.4f dist:%g",visits,ps.x,ps.y,h*ht/FEET,h*aveht/FEET,distance/FEET);
	cout<<msg<<endl;
	
}
//===================== Sprite ==============================
ValueList<SpriteData*> Sprite::sprites;
//-------------------------------------------------------------
// Sprite::Sprite() Constructor
//-------------------------------------------------------------
Sprite::Sprite(Image *i, int l, TNode *e)
{
	type=l;
	sprite_id=get_id();
	texture_id=0;
    image=i;
	expr=e;
	valid=false;
}

void Sprite::reset()
{
	sprites.free();
	PlacementMgr::free_htable();
}

//-------------------------------------------------------------
// Sprite::collect() collect valid sprite points
//-------------------------------------------------------------
void Sprite::collect()
{
#ifdef SHOW_STATS	
	int trys=0;
	int visits=0;
	int bad_visits=0;
	int bad_valid=0;
	int bad_active=0;
	int bad_pts=0;
	double ave_pts=0;
#endif	
	PlacementMgr::ss();
	SpritePoint *s=(SpritePoint*)PlacementMgr::next();
	cout<<"pointsize tests:"<<tests<<" fails:"<<fails<<" "<<100.0*fails/tests<<" %"<<endl;

	while(s){
#ifdef SHOW_STATS
		trys++;
		
		if(s->visits<MIN_VISITS)
			bad_visits++;
		if(!s->flags.s.valid)
			bad_valid++;
		if(!s->flags.s.active)
			bad_active++;
#endif	
		if(s->visits>=MIN_VISITS && s->flags.s.valid && s->flags.s.active){
			Point4D	p(s->center);
			Point pp=Point(p.x,p.y,p.z);
			Point ps=pp.spherical();
			//double ht=s->aveht/s->visits;
			double ht=s->ht;
			
			Point center=TheMap->point(ps.x, ps.y,ht+ht_offset*s->radius/TheMap->radius);
			Point t=Point(-center.x,center.y,-center.z)-TheScene->xpoint; // why the 180 rotation around y axis ????
     
			double d=t.length();
			
			double r=TheMap->radius*s->radius;
			double f=TheScene->wscale*r/d;
		    double pts=f;
#ifdef SHOW_STATS		    
		    if(pts>=min_pts){
				sprites.add(new SpriteData((SpritePoint*)s,t,d,pts));
				ave_pts+=pts;
		    }
		    else
		    	bad_pts++;
#else
		    if(pts>=min_pts)
		    	sprites.add(new SpriteData((SpritePoint*)s,t,d,pts));
#endif
		}
		s=PlacementMgr::next();
	}
	PlacementMgr::end();
	sprites.sort();
#ifdef SHOW
	for(int i=0;i<sprites.size;i++){
		cout<<i<<" ";
		sprites[i]->print();	
	}
#endif
#ifdef SHOW_STATS
	double winsize=640*480;
	double usage=100.0*trys/PlacementMgr::hashsize;
	double badvis=100.0*bad_visits/trys;
	double badactive=100.0*bad_active/trys;
	double badpts=100.0*bad_pts/trys;
	double avepts=ave_pts/sprites.size;
	double proj=0.8*avepts;
	double coverage=100.0*proj*proj*sprites.size/winsize;
	double overlap=lerp(coverage,0,100,1,4);

	cout<<" sprites "<<sprites.size<<" %cov:"<<coverage/overlap<<" tests:"<<trys<<" %hash:"<<usage<<" %inactive:"<<badactive<<" %small:"<<badpts<<endl;
#else
	cout<<" sprites collected:"<<sprites.size<<endl;
#endif
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
	char str[MAXSTR];
#ifdef TEST
	int texid=sprite_id;
#else
	int texid=TerrainProperties::sid;
#endif
	glActiveTexture(GL_TEXTURE0+texid);
	if(!valid){
		glGenTextures(1, &texture_id); // Generate a unique texture ID
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -1);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		int w=image->width;
		int h=image->height;
		unsigned char* pixels=(unsigned char*)image->data;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		valid=true;
	}
	glBindTexture(GL_TEXTURE_2D, texture_id);
	GLhandleARB program=GLSLMgr::programHandle();
	sprintf(str,"samplers2d[%d]",texid);  
	//sprintf(str,"sprite");    	
	glUniform1iARB(glGetUniformLocationARB(program,str),texid);
	
	return true;
}
bool Sprite::initProgram(){
	return false;
}

//===================== TNsprite ==============================
//************************************************************
// TNsprite class
//************************************************************
TNsprite::TNsprite(char *s, int opts,  TNode *l, TNode *r) : TNplacements(opts|SPRITES|NNBRS|NOLOD,l,r,0)
{
	set_collapsed();
	setName(s);
	FREE(s);
	image=0;
	sprite=0;
#ifndef TEST
	set_id(TerrainProperties::sid++);
#endif
    mgr=new SpriteMgr(type);
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
	if(!image)
		image=images.load(sprites_file,JPG);
	if(!image){
		printf("TNsprites ERROR image %s not found\n",sprites_file);
		return;
	}
	if(sprite==0)
		sprite=new Sprite(image,type,this);
	smgr->init();
	TNplacements::init();
}
void TNsprite::set_id(int i){
	instance=i;
	TNplacements::set_id(i);
	if(sprite)
		sprite->set_id(i);
}
//-------------------------------------------------------------
// TNsprite::eval() evaluate the node
//-------------------------------------------------------------
//#define COLOR_TEST
//#define DENSITY_TEST
void TNsprite::eval()
{	
	SINIT;
#ifdef DENSITY_TEST
	Td.set_flag(DVALUE);
	Td.density=0;
#endif
	if(CurrentScope->rpass()){
#ifdef TEST
		int size=Td.tp->sprites.size;
		set_id(size);
#endif
		Td.add_sprite(sprite);
		return;
	}	
	if(!CurrentScope->spass())
		return;
	
	Color c =Color(1,1,1);
	double density=0;

	SpriteMgr *smgr=(SpriteMgr*)mgr;

	htval=Height;
	ncalls++;

	double arg[4];
	INIT;
	TNarg &args=*((TNarg *)left);
	TNode *dexpr;
	
	int n=getargs(&args,arg,3);

	if(n>0) mgr->levels=(int)arg[0]; 	// scale levels
	if(n>1) mgr->maxsize=arg[1];     	// size of largest craters
	if(n>2) mgr->mult=arg[2];			// scale multiplier per level
	
	if(mgr->dexpr==0){
		dexpr=args[3];
		if(dexpr){
		    dexpr->eval();
		    mgr->density=S0.s;
		}
	}
	
	MaxSize=mgr->maxsize;
	
	mgr->type=type;

	double hashcode=(mgr->levels+
		            1/mgr->maxsize+
					1/mgr->mult
					);
	mgr->id=(int)hashcode+mgr->type+SPRITES+instance+hashcode*TheNoise.rseed;
	//TNplacements::eval();  // evaluate common arguments
	
	cval=0;
	hits=0;
	smgr->eval();  // calls SpritePoint.set_terrain
   
	if(hits>0 && cval>0 && cval<=1/**/){ // inside target radius
		nhits++;
		double x=1-cval;
#ifdef COLOR_TEST
		if(get_id()==1)
			c=Color(x,1,1);
		else
			c=Color(1,1,x);
#endif
#ifdef DENSITY_TEST
		x=1/(cval);
		x=x*x;//*x*x;
		density=lerp(cval,0,0.2,0,.05*x);		
#endif
	}
#ifdef COLOR_TEST
	glColor4d(c.red(), c.green(), c.blue(), c.alpha());
	Td.diffuse=c;	
	Td.set_cvalid();
#endif	
#ifdef DENSITY_TEST
	Td.density=density;	
#endif
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
	char opts[64];
	opts[0]=0;
	if(optionString(opts))
		sprintf(s+strlen(s),"%s,",opts);
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
