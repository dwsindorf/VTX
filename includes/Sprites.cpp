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
// 6) need to add small level offset (roff2) otherwise sprites will overlap
//   - but this causes more jitter in sprite ht and "floating" sprites
//   - possibly because spread in xy uses same ht as first level ?
// 7) sometimes see grid like artifact (sprites arranged in lines)
//   - reduced by increasing roff2 (but decreases density)
// 8) drag and drop doesn't work (FIXED)
//   - preceding Sprite in file lacks "+" fixed by allowing alt form of constructor
// 9) with multiple sprites in file changing the size of one affects the coverage of others
//   - FIXED need to have separate hash table to each sprite type
// 10) density test doesn't seem to reject any instances (FIXED)
// TODO
// 1) change sprite point size with distance (DONE)
// 2) set ht offset based on sprite actual size and distance (DONE)
// 3) add support for multiple sprite types (DONE)
// 4) add support for multiple terrain types (DONE - TEST?)
// 5) allow sprite images to be in different directory than 2d textures (DONE)
// 6) implement multiple sprite lookup from single image (DONE)
// 7) implement random horizontal flip of sprite image (DONE)
//    - in fragment shader use vec2(1-l_uv.x,l_uv.y) based on random variable
// 8) implement environmental density (ht,slope) as in Texture class (DONE)
// 9) implement lighting
//    - star distance + time of day (DONE)
//    - shadows (~DONE)
//    - haze (~DONE)
//    - fog
// 10) create sprite wxWidgets gui (DONE)
// 11) add support for Noise expr in density (DONE)
// 12) allow only selected sprite to be rendered (checkbox)
//************************************************************
// classes SpritePoint, SpriteMgr
//************************************************************

extern double Hscale, Drop, MaxSize,Height,Phi;
extern double ptable[];
extern Point MapPt;
extern double  zslope();
extern NameList<LongSym*> POpts;

extern void inc_tabs();
extern void dec_tabs();
extern char   tabs[];
extern int addtabs;

static double mind=0;

static int ncalls=0;
static int nhits=0;
static double thresh=1.0;    // move to argument ?
static double ht_offset=0.5; // move to argument ?

static double min_render_pts=1; // for render
static double min_adapt_pts=2; //  for adapt - increase resolution only around nearby sprites

static int cnt=0;
static int tests=0;
static int pts_fails=0;
static int dns_fails=0;

static TerrainData Td;

#ifdef TEST_SPRITES
SpriteMgr g_sm(FINAL|DENSITY_TEST|COLOR_TEST);
#else
SpriteMgr g_sm(FINAL|DENSITY_TEST);
#endif

//static int hits=0;
//#define DEBUG_PMEM

//#define COLOR_TEST
//#define DENSITY_TEST

#define USE_AVEHT
#define SHOW
#define MIN_VISITS 1
#define TEST_NEIGHBORS 1
#define TEST_PTS 
//#define SHOW_STATS
//#define DUMP
#ifdef DUMP
static void show_stats()
{
	g_sm.dump();
	cout<<"calls="<<ncalls<< " hits="<<nhits<<endl;
}
#endif

void SpriteImageMgr::setImageBaseDir(){
	static char base[512];
	static char dir[512];
	File.getBaseDirectory(base);
 	sprintf(dir,"%s/Textures/Sprites",base);
 	//MALLOC(strlen(dir),char,base_dir);
 	strcpy(base_dir,dir);
}

SpriteImageMgr sprites_mgr; // global image manager

//************************************************************
// SpriteMgr class
//************************************************************
//	arg[0]  levels   		scale levels
//	arg[1]  maxsize			size of largest craters
//	arg[2]  mult			size multiplier per level
//	arg[3]  density			density or dexpr
//
//-------------------------------------------------------------
//ValueList<FileData*> SpriteMgr::sprite_dirs;
SpriteMgr::SpriteMgr(int i) : PlacementMgr(i)
{
#ifdef DUMP
	if(!s_mgr)
		add_finisher(show_stats);
#endif
	MSK_SET(type,PLACETYPE,SPRITES);
	level_mult=0.2;
	slope_bias=0;
	ht_bias=0;
	lat_bias=0;
	rand_flip_prob=0.5;
	variability=1;
	dexpr=0;
	instance=0;
	select_bias=0;
	sprites_rows=0;
	sprites_cols=0;
	set_ntest(TEST_NEIGHBORS);
	set_testpts(true);
	set_useaveht(false);
}
SpriteMgr::~SpriteMgr()
{
  	if(finalizer()){
 #ifdef DEBUG_PMEM
  		printf("SpriteMgr::free()\n");
#endif
	}
}

void SpriteMgr::setTests() {
	if(!test() || hits==0)
		return;
	double x=1-cval;
	if(testColor()) {
		S0.set_cvalid();
		if(fabs(x)>0)
			S0.c=Color(0,1-x,1);
		else
			S0.c=Color(0,1,1);
		Td.diffuse=Td.diffuse.mix(S0.c,0.5);
	}
	if(testDensity()) {
		x=1/(cval+1e-6);
		x=x*x; //*x*x;
		Td.density+=lerp(cval,0,0.2,0,0.05*x);
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
  	reset();
 }

void SpriteMgr::eval(){	
	PlacementMgr::eval(); 
}

bool SpriteMgr::setProgram(){
	TerrainProperties *tp=Td.tp;
	if(!Sprite::data.size)
		return false;
	char defs[1024]="";
	sprintf(defs+strlen(defs),"#define NSPRITES %d\n",Td.sprites.size);
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
   
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SPRITE);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,GL_LOWER_LEFT);
	glDisable(GL_POINT_SMOOTH);

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
	GLSLMgr::CommonID1=glGetAttribLocation(program,"CommonAttributes1"); // Constants1
	GLSLMgr::setProgram();
	GLSLMgr::loadVars();
	for(int i=0;i<Td.sprites.size;i++){
		TerrainData::sid=i;
		Td.sprites[i]->setProgram();
	}
	//glDisable(GL_DEPTH_TEST);
	glBegin(GL_POINTS);
	
	int n=Sprite::data.size;
	bool flip=false;
	SpriteData *s=(SpriteData*)Sprite::data[0];

	glNormal3dv(s->normal.values());

	for(int i=n-1;i>=0;i--){
		s=(SpriteData*)Sprite::data[i];
		int id=s->instance;//s->get_id();
		Point t=s->vertex;
		double pts=s->pts;
				
		// random reflection - based on sprite hash table center position

		int rval=s->rval;
		
		if(s->flip())
			flip=RANDVAL(rval)+0.5>s->rand_flip_prob?false:true;
		
		double x=RANDVAL(rval);
		double rv=s->variability*x;
		pts*=1+rv;
		
		double sel=0.1;	
		double r=0;
		double sid=0;
	    int rows=s->sprites_rows;
	    int cols=s->sprites_cols;
		double nn=(rows*cols);
		double sb=0;

		if(nn>1){ // random selection in multi-row sprites image
			r=2*RANDVAL(rval);
			r=clamp(r,-1,1);
			sid=s->get_id();
			sb=(1-s->select_bias)*r*(nn);
			sel=sid+sb+0.5;
			sel=sel>nn?sel-nn:sel;		
			sel=sel<0?nn+sel:sel;
			sel=clamp(sel,0,nn-1);
		}
		
		int sy=sel/cols;
		int sx=sel-sy*rows;
		
		sy=rows-sy-1; // invert y
	    	    
	   // cout<<"rows:"<<rows<<" cols:"<<cols<<" sel:"<<(int)sel<<" sy:"<<sy<<" sx:"<<sx<<" sb:"<<sb<<endl;
    
	    //cout<<(int)sid<<" "<<(int)sel<<" "<<r<<" "<<sb<<endl;
		glVertexAttrib4d(GLSLMgr::TexCoordsID,id+0.1, rows, pts, sel);
		glVertexAttrib4d(GLSLMgr::CommonID1, flip, cols, sx, sy);
	    glVertex3dv(t.values());
	}
	glEnd();
	return true;
}
//-------------------------------------------------------------
// SpriteMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *SpriteMgr::make(Point4DL &p, int n)
{
    return new SpritePoint(*this,p,n);
}
//PlaceData *SpriteMgr::make(Placement*s,Point bp,double d,double pts){
//	Point xp=bp-TheScene->xpoint; // TODO: Why only needed for sprites ?
//	return new SpriteData((SpritePoint*)s,xp,d,pts);
//}

PlaceData *SpriteMgr::make(Placement*s){
	s->setVertex();
	//Point xp=bp-TheScene->xpoint; // TODO: Why only needed for sprites ?
	return new SpriteData((SpritePoint*)s);
}

//************************************************************
// class SpritePoint
//************************************************************
SpritePoint::SpritePoint(SpriteMgr&mgr, Point4DL&p,int n) : Placement(mgr,p,n)
{
	sprites_rows=mgr.sprites_rows;
	sprites_cols=mgr.sprites_cols;
	variability=mgr.mult;
	rand_flip_prob=mgr.rand_flip_prob;
	select_bias=mgr.select_bias;
	instance=mgr.instance;
}

//-------------------------------------------------------------
// SpritePoint::set_terrain()	impact terrain
//-------------------------------------------------------------
//===================== SpriteData ==============================
SpriteData::SpriteData(SpritePoint *s): PlaceData(s){
	vertex=s->vertex-TheScene->xpoint; // TODO: Why only needed for sprites ?
  	sprites_cols=s->sprites_cols;
 	sprites_rows=s->sprites_rows;
	variability=s->variability;
	rand_flip_prob=s->rand_flip_prob;
	select_bias=s->select_bias;
}

//===================== Sprite ==============================
ValueList<PlaceData*> Sprite::data;
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
	rows=((TNsprite *)e)->getImageRows();
	cols=((TNsprite *)e)->getImageCols();
	valid=false;
}

void Sprite::reset()
{ 
	cout<<"Sprite::reset()"<<endl;
	data.free();
}

void Sprite::set_image(Image *i, int r, int c){
	image=i;
	rows=r;
	cols=c;
	valid=false;
	if(texture_id>0)
		glDeleteTextures(1,&texture_id);
}
//-------------------------------------------------------------
// Sprite::collect() collect valid sprite points
//-------------------------------------------------------------
void Sprite::collect()
{
	double d0=clock();

	data.free();

	for(int i=0;i<Td.sprites.size;i++){
		Sprite *sprite=Td.sprites[i];
		SpriteMgr *mgr=sprite->mgr();
		mgr->collect(data);
	} // next sprite
	if(data.size){
  		data.sort();
	}
	double d1=clock();
	cout<<"Sprites collected:"<<data.size<<" "<<1000*(d1-d0)/CLOCKS_PER_SEC<<" ms"<<endl;

}
//-------------------------------------------------------------
// Sprite::eval() evaluate TNtexture string
//-------------------------------------------------------------
void Sprite::eval()
{
	int mode=CurrentScope->passmode();
	CurrentScope->set_spass();
	expr->eval(); // TNsprite.eval()
	CurrentScope->set_passmode(mode);
}

bool Sprite::setProgram(){
	char str[MAXSTR];
	int texid=sprite_id;
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
        double aspect=((double)rows/cols);
		int w=image->width;
		int h=image->height;
		unsigned char* pixels=(unsigned char*)image->data;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		valid=true;
	}
	glBindTexture(GL_TEXTURE_2D, texture_id);
	GLhandleARB program=GLSLMgr::programHandle();
	sprintf(str,"samplers2d[%d]",texid);  
	glUniform1iARB(glGetUniformLocationARB(program,str),texid);
	
	GLSLVarMgr vars;
	
	vars.setProgram(program);
	vars.loadVars();

	return true;
}
bool Sprite::initProgram(){
	return false;
}

//===================== TNsprite ==============================
//************************************************************
// TNsprite class
//************************************************************
TNsprite::TNsprite(char *s, int opts,  TNode *l, TNode *r) : TNplacements(0,l,r,0)
{
    setImageMgr(&sprites_mgr);
	set_collapsed();
 	type=opts|SPRITES;
 	//cout<<"TNsprite ID="<<get_id()<<endl;
	setName(s);
	FREE(s);
	image=0;
	sprite=0;
    mgr=new SpriteMgr(SPRITES);
}

//-------------------------------------------------------------
// TNsprite::~TNsprite() destructor
//-------------------------------------------------------------
TNsprite::~TNsprite()
{
	DFREE(sprite);
}

//-------------------------------------------------------------
// TNsprite::applyExpr() apply expr value
//-------------------------------------------------------------
void TNsprite::applyExpr()
{
   if(expr){
	    TNsprite* sprt=(TNsprite*)expr;
	    type=sprt->type;
	    mgr->type=type;
		DFREE(left);
		left=expr->left;
		left->setParent(this);
		expr=0;
	}
	if(right)
		right->applyExpr();
 }
//-------------------------------------------------------------
// TNsprite::init() initialize the node
//-------------------------------------------------------------
void TNsprite::init()
{
	if(right)
    	right->init();

	SpriteMgr *smgr=(SpriteMgr*)mgr;
	char *file=getImageFile();
	if(!image){
		char path[512];
		if(getImageFilePath(file,path)){
			image=images.open(file,path);
			if(image){
				cout<<"Sprite image found:"<<path<<endl;
			}
		}
	}
	smgr->sprites_rows=getImageRows();
	smgr->sprites_cols=getImageCols();
	
	//cout<<smgr->sprites_cols<<"x"<<smgr->sprites_rows<<endl;
	if(!image){
		printf("TNsprites ERROR image %s not found\n",file);
		return;
	}
	if(sprite==0)
		sprite=new Sprite(image,type,this);
	smgr->set_first(1);
	smgr->init();
	TNplacements::init();
}

void TNsprite::set_id(int i){
	//instance=i;
	BIT_OFF(type,PID);
	type|=i&PID;
}
//-------------------------------------------------------------
// TNsprite::eval() evaluate the node
//-------------------------------------------------------------
void TNsprite::eval()
{	
	SINIT;
	if(!isEnabled()){
		if(right)
			right->eval();
		return;
	}
	if(CurrentScope->rpass()){
		int size=Td.sprites.size;
		instance=size;
		mgr->instance=instance;
		if(sprite)
			sprite->set_id(size);
		Td.add_sprite(sprite);
		mgr->setHashcode();
		if(right)
			right->eval();
		return;
	}	
	if(!CurrentScope->spass()){
		if(right)
			right->eval();
		return;
	}
	
	Color c =Color(1,1,1);

	SpriteMgr *smgr=(SpriteMgr*)mgr;

	PlacementMgr::htval=Height;
	ncalls++;
	
	double arg[10];
	INIT;
	TNarg &args=*((TNarg *)left);
	TNode *dexpr;
	
	int n=getargs(&args,arg,9);
	
	double density=1;

	if(n>0) mgr->levels=(int)arg[0]; 	// scale levels
	if(n>1) mgr->maxsize=arg[1];     	// size of largest craters
	if(n>2) mgr->mult=arg[2];			// random scale multiplier
	if(n>3) mgr->level_mult=arg[3];     // scale multiplier per level
	if(n>4) maxdensity=arg[4];
	if(n>5) smgr->slope_bias=arg[5];
	if(n>6) smgr->ht_bias=arg[6];
	if(n>7) smgr->lat_bias=arg[7];
	if(n>8) smgr->select_bias=arg[8];
	
	density=maxdensity;
	MaxSize=mgr->maxsize;
	radius=TheMap->radius;
		
	mgr->type=type;
	if(smgr->slope_bias){
		double slope=8*zslope();
		double f=2*lerp(fabs(smgr->slope_bias)*slope,0,1,-smgr->slope_bias,smgr->slope_bias);
#ifdef DEBUG_SLOPE_BIAS
		if(ncalls%100==0)
			cout<<"slope:"<<slope<<" f:"<<f<<endl;
#endif
		density+=f;
	}
	if(smgr->ht_bias){
		double f=2*lerp(8*fabs(smgr->ht_bias)*Height,-1,1,-smgr->ht_bias,smgr->ht_bias);
		density+=f;
	}
	if(smgr->lat_bias){
		double f=lerp(fabs(smgr->lat_bias)*fabs(2*Phi/180),0,1,-smgr->lat_bias,+smgr->lat_bias);
		density+=f;
	}
    density*=maxdensity;
	density=clamp(density,0,1);
	density=sqrt(density);

	mgr->density=density;	
    
	smgr->eval();  // calls SpritePoint.set_terrain	
	g_sm.setTests();
 }

//-------------------------------------------------------------
// TNsprite::optionString() get option string
//-------------------------------------------------------------
int TNsprite::optionString(char *c)
{
	return TNplacements::optionString(c);
}

//-------------------------------------------------------------
// TNtexture::valueString() node value substring
//-------------------------------------------------------------
void TNsprite::valueString(char *s)
{
	sprintf(s+strlen(s),"%s(\"%s\",",symbol(),getImageFile());
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
// TNfunc::save() archive the node
//-------------------------------------------------------------
void TNsprite::saveNode(FILE *f)
{
	char buff[1024];
	buff[0]=0;
	valueString(buff);
	//if(addtabs)
	    fprintf(f,"\n%s",tabs);
	fprintf(f,"%s",buff);
}

#define USE_IMGR
//-------------------------------------------------------------
// TNinode::setName() set name
//-------------------------------------------------------------
void TNsprite::setName(char *s)
{
	strcpy(image_file,s);
}

char *TNsprite::nodeName()  { 
	return image_file;
}

//-------------------------------------------------------------
// TNsprite::setSpritesTexture() set sprite image texture
//-------------------------------------------------------------
void TNsprite::setSpritesImage(char *name){
	setImage(name);
	if(image){
		sprite->set_image(image,image_rows,image_cols);
	}
}
