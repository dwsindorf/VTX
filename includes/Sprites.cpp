#include "SceneClass.h"
#include "RenderOptions.h"
#include "Sprites.h"
#include "TerrainClass.h"
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
// 11) with new PlaceObjMgr code "layer" sprites (rendered first)trash depth buffer for "global" sprites
//     - need to collect and sort all sprites before rendering (fixed - removed global SpriteObjMgr)
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

extern double Hscale, Drop, MaxSize,Height,Theta,Phi;
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

//static int hits=0;
//#define DEBUG_PMEM

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
	//level_mult=0.2;
	mult=0.5;
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
	set_testDensity(true);
#ifdef TEST_SPRITES
	set_testColor(true);
#endif
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
//-------------------------------------------------------------
// SpriteMgr::init()	initialize global objects
//-------------------------------------------------------------
void SpriteMgr::init()
{
#ifdef DEBUG_PMEM
  	printf("SpriteMgr::init()\n");
#endif
	PlacementMgr::init();
	nhits=0;
	cnt=0;
  	reset();
 }

void SpriteMgr::eval(){	
	PlacementMgr::eval(); 
}

SpriteMgr *Sprite::mgr() { 
	return ((TNsprite*)expr)->mgr;
}

void SpriteVBO::build() {
    if (!dirty || vertices.empty()) return;

    if (!vao) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SpriteVertex),
                 vertices.data(), GL_DYNAMIC_DRAW);

    size_t stride = sizeof(SpriteVertex);

    // Position -> gl_Vertex
    glVertexPointer(3, GL_FLOAT, stride, (void*)offsetof(SpriteVertex, pos));
    glEnableClientState(GL_VERTEX_ARRAY);

    // TexCoordsID (id, rows, pts, sel)
    glVertexAttribPointer(GLSLMgr::TexCoordsID, 4, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(SpriteVertex, texcoord));
    glEnableVertexAttribArray(GLSLMgr::TexCoordsID);

    // CommonID1 (flip, cols, sx, sy)
    glVertexAttribPointer(GLSLMgr::CommonID1, 4, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(SpriteVertex, attrib1));
    glEnableVertexAttribArray(GLSLMgr::CommonID1);

    glBindVertexArray(0);

    vertCount = vertices.size();
    dirty = false;
}
void SpriteVBO::render() {
    if (vertices.empty()) return;
    build();
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, vertCount);
    glBindVertexArray(0);
}

void SpriteVBO::free() {
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    vertices.clear();
    vertCount = 0;
    dirty = true;
}
ValueList<PlaceData*> SpriteObjMgr::data(50000,10000);
bool SpriteObjMgr::vbo_valid=false;  // Add this
SpriteVBO SpriteObjMgr::spriteVBO;

void SpriteObjMgr::collect(){
	data.free();
	for(int i=0;i<objs.size;i++){
		PlaceObj *obj=objs[i];
		obj->mgr()->collect(data);
	}
	if(data.size)
		data.sort();
	vbo_valid = false;  // Mark VBO as needing rebuild
}

bool SpriteObjMgr::setProgram(){
	if(!data.size || !objs.size)
		return false;
	char defs[1024]="";
	sprintf(defs+strlen(defs),"#define NSPRITES %d\n",objs.size);
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
	for(int i=0;i<objs.size;i++){
		objs[i]->setProgram();
	}

	return true;
}
// VBO Sprites TID:1 Objects:3 Placements:86061 MapData processed:33109 rejected:0 times reset:48 eval:578 collect:104 render:7 total:737 ms
// t   Sprites TID:1 Objects:3 Placements:58776 MapData processed:28471 rejected:0 times reset:77 eval:1085 collect:94 render:13 total:1269 ms
// t   Sprites TID:4 Objects:3 Placements:62943 MapData processed:29237 rejected:0 times reset:95 eval:1103 collect:95 render:68 total:1361 ms

// NO  Sprites TID:1 Objects:3 Placements:86220 MapData processed:32979 rejected:0 times reset:53 eval:572 collect:109 render:53 total:787 ms
// t   Sprites TID:1 Objects:3 Placements:58776 MapData processed:28472 rejected:0 times reset:78 eval:1076 collect:93 render:14 total:1261 ms
void SpriteObjMgr::render() {

	int n=placements();

	if(n==0)
		return;
	bool flip = false;

	bool moved = TheScene->moved() || TheScene->changed_detail();
	
#ifdef USE_SPRITES_VBO
    bool update_needed = moved || !vbo_valid;
#else
    bool update_needed = true;
#endif
	setProgram(); // set sprites shader

	SpriteData *s = (SpriteData*) data[0];
	glNormal3dv(s->normal.values());

	if (update_needed) {

#ifdef USE_SPRITES_VBO
		spriteVBO.clear();
#else
		glBegin(GL_POINTS);
#endif

		for (int i = n - 1; i >= 0; i--) {
			SpriteData *s = (SpriteData*) data[i];
			int id = s->instance; //s->get_id();
			Point t = s->vertex-TheScene->xpoint;
			double pts = s->pts;
			
			// random reflection - based on sprite hash table center position

			int rval = s->rval;

			if (s->flip())
				flip = RANDVAL(rval) + 0.5 > s->rand_flip_prob ? false : true;

			double x = RANDVAL(rval);
			double rv = s->variability * x;
			pts *= 1 + rv;

			double sel = 0.1;
			double r = 0;
			double sid = 0;
			int rows = s->sprites_rows;
			int cols = s->sprites_cols;
			double nn = (rows * cols);
			double sb = 0;

			if (nn > 1) { // random selection in multi-row sprites image
				r = 2 * RANDVAL(rval);
				r = clamp(r, -1, 1);
				sid = s->get_id();
				sb = (1 - s->select_bias) * r * (nn);
				sel = sid + sb + 0.5;
				sel = sel > nn ? sel - nn : sel;
				sel = sel < 0 ? nn + sel : sel;
				sel = clamp(sel, 0, nn - 1);
			}
			int sy = sel / cols;
			int sx = sel - sy * rows;
			sy = rows - sy - 1; // invert y
#ifdef USE_SPRITES_VBO
			spriteVBO.addSprite(t, id, rows, cols, pts, sel, sx, sy, flip ? -1.0f : 1.0f);
#else			
			glVertexAttrib4d(GLSLMgr::TexCoordsID, id + 0.1, rows, pts, sel);
			glVertexAttrib4d(GLSLMgr::CommonID1, flip, cols, sx, sy);
			glVertex3dv(t.values());
#endif			
		}
#ifndef USE_SPRITES_VBO
		glEnd();
#else
		vbo_valid = true;  // Mark VBO as valid
#endif
	} 
#ifdef USE_SPRITES_VBO
	spriteVBO.render();
#endif
}
//-------------------------------------------------------------
// SpriteMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *SpriteMgr::make(Point4DL &p, int n)
{
    return new SpritePoint(*this,p,n);
}

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
// SpritePoint::SpriteData()
//-------------------------------------------------------------
//===================== SpriteData ==============================
SpriteData::SpriteData(SpritePoint *s): PlaceData(s){
	vertex=s->vertex;//-TheScene->xpoint; // TODO: Why only needed for sprites ?
  	sprites_cols=s->sprites_cols;
 	sprites_rows=s->sprites_rows;
	variability=s->variability;
	rand_flip_prob=s->rand_flip_prob;
	select_bias=s->select_bias;
}

//===================== Sprite ==============================
//ValueList<PlaceData*> Sprite::data;
//-------------------------------------------------------------
// Sprite::Sprite() Constructor
//-------------------------------------------------------------
Sprite::Sprite(Image *i, int t, TNode *e) :PlaceObj(t,e)
{
	texture_id=0;
    image=i;
	rows=((TNsprite *)e)->getImageRows();
	cols=((TNsprite *)e)->getImageCols();
}

void Sprite::set_image(Image *i, int r, int c){
	image=i;
	rows=r;
	cols=c;
	valid=false;
	if(texture_id>0)
		glDeleteTextures(1,&texture_id);
}
bool Sprite::setProgram(){
	char str[MAXSTR];
	int texid=id;
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
	BIT_OFF(type,PID);
	type|=i&PID;
}

//-------------------------------------------------------------
// TNsprite::eval() evaluate the node
//-------------------------------------------------------------
void TNsprite::eval()
{
	TerrainData ground;

	SpriteMgr *smgr=(SpriteMgr*)mgr;

	SINIT;
	if(!isEnabled() || TheScene->viewtype !=SURFACE || Raster.surface==2){
		if(right)
			right->eval();
		return;
	}
	if(CurrentScope->rpass()){
		int layer=inLayer()?Td.tp->type():0; // layer id
		int instance=Td.tp->Sprites.objects();		
		mgr->instance=instance;
		mgr->layer=layer;
		if(sprite){
			sprite->set_id(instance);
			sprite->layer=layer;
		}
		Td.tp->Sprites.addObject(sprite);
		Td.pids++;
		mgr->setHashcode();
		if(right)
			right->eval();
		return;
	}
	if(right)
		right->eval();
	if(!CurrentScope->spass()){
		ground.copy(S0); // save S0.p.z etc.
		Height=S0.p.z;
		MapPt=TheMap->point(Theta,Phi,Height)-TheScene->xpoint;
	}

	INIT;

	smgr->getArgs((TNarg *)left);
	MaxSize=mgr->maxsize;
	double density=smgr->density;	
	radius=TheMap->radius;
	mgr->type=type;
	TNarg &args=*((TNarg *)left);

	TNarg *a = args.index(8);
	double arg[3];
	if (a) {  
		int n = getargs(a, arg, 3);
		if(n>0)
			smgr->select_bias=arg[0];
	}
	if(density>0)
		mgr->eval();  // calls SpritePoint.set_terrain	
		
	if(!CurrentScope->spass()){
		S0.copy(ground); // restore ground parameters (S0.p.z)
		mgr->setTests(); // set S0.s, S0.c ,S0.set_flag(DVALUE)
	}
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
