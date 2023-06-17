// BUGS/problems
// 1) need to disable neighbor test in PlacementMgr
//    - else no color or density gradient causes random sprite 
//      visibility as viewpoint moves
// 2) can only have 1 level (FIXED)
// 3) at maximum probability minimum density very low
//    - compensated somewhat bt adding additional levels
// 4) Occasionally some sprites disappear reappear as viewpoint moves
//   - but still see colored placement circles in debug mode
// 5) draw order sometimes fails (back to front)
//   - defined by distance from viewpoint
// 6) need to add small level offset (roff) otherwise sprites will overlap
//   - but this causes more jitter in sprite ht and "floating" sprites
//   - possibly because spread in xy uses same ht as first level
//   - need to generate new true placements for each level ?
// TODO
// 1) change sprite pointsize with distance (DONE)
// 2) set ht offset based on sprite actual size and distance (DONE)
// 3) implement method for multiple sprite types and multiple terrain types
// 4) create sprite wxWidgets gui
// 5) allow sprite images to be in different directory than 2d textures
// 6) implement multiple sprite lookup from single image
//    - Could be same sprite different views depending on camera angle
//    - or different sprites of similar types (trees etc)
// 7) implement random horizontal flip of sprite image for better aliasing
// 8) implement environmental density (ht,slope) as in Texture class
// 9) implement lighting
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
#include "GLSLMgr.h"

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
static double thresh=0.5;
static double ht_offset=0.9;
//static double lvl_offset=5e-5;
static double lvl_offset=0;
static double min_pts=2;

static int cnt=0;
static int tests=0;
static int fails=0;

static TerrainData Td;
static SpriteMgr *s_mgr=0; // static finalizer
static int hits=0;
#define DEBUG_PMEM
#define TEST
//#define SHOW
#define MIN_VISITS 1
#define TEST_NEIGHBORS 0
//#define TEST_PTS 
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
	roff=lvl_offset;
	//roff2=lvl_offset;
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
	char defs[128]="";
	sprintf(defs,"#define NSPRITES %d\n",tp->sprites.size);

	GLSLMgr::setDefString(defs);
	
	GLSLMgr::loadProgram("sprites.ps.vert","sprites.ps.frag");
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;
	GLSLMgr::setProgram();
	glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SPRITE);
	glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	GLSLVarMgr vars;
	for(int i=0;i<tp->sprites.size;i++){
		TerrainProperties::sid=i;
		tp->sprites[i]->setProgram();
	}
	
	glBegin(GL_POINTS);
	
	// render the sprites
	int n=Sprite::sprites.size;
	for(int i=n-1;i>=0;i--){
		SpriteData *s=Sprite::sprites[i];
		int id=s->get_id();
		Point t=s->center;
		double f=s->pntsize;
		//glVertexAttrib4d(GLSLMgr::TexCoordsID,tp->sprites.size-id-1, f, f, 1);
		glVertexAttrib4d(GLSLMgr::TexCoordsID,id, f, f, 1);

	    glVertex3dv(t.values());
		//printf("%d x:%-1.5g y:%-1.5g z:%-1.5g d:%-4.1f r:%-4.1f s:%-1.5f\n ",n-i,t.x,t.y,t.z,s->distance/FEET,s->radius/FEET,f);	
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);
	cout<<"rendered "<< n<<" sprites"<<endl;
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
	int trys=0;
	int visits=0;
	int bad_visits=0;
	int bad_valid=0;
	int bad_active=0;
	int bad_pts=0;
	
	PlacementMgr::ss();
	SpritePoint *s=(SpritePoint*)PlacementMgr::next();
	cout<<"tests:"<<tests<<" fails:"<<fails<<" "<<100.0*fails/tests<<" %"<<endl;

	while(s){
		trys++;
		if(s->visits<MIN_VISITS)
			bad_visits++;
		if(!s->flags.s.valid)
			bad_valid++;
		if(!s->flags.s.active)
			bad_active++;
		
		if(s->visits>=MIN_VISITS && /*(s->get_class()==SPRITES) && */s->flags.s.valid && s->flags.s.active){
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
		    
		    if(pts>=min_pts)
				sprites.add(new SpriteData((SpritePoint*)s,t,d,pts));
		    else
		    	bad_pts++;
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
	double usage=100.0*trys/PlacementMgr::hashsize;
	double badvis=100.0*bad_visits/trys;
	double badactive=100.0*bad_active/trys;
	double badpts=100.0*bad_pts/trys;

	cout<<" collected "<<sprites.size<<" trys:"<<trys<<" usage:"<<usage<<" bad - active:"<<badactive<<" pts:"<<badpts<<" %"<<endl;

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
TNsprite::TNsprite(char *s, TNode *l, TNode *r) : TNplacements(SPRITES|NNBRS|NOLOD,l,r,0)
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
	Color c =Color(1,1,1);
	double density=0;
	if(!CurrentScope->spass())
		return;	

	SpriteMgr *smgr=(SpriteMgr*)mgr;

	htval=Height;
	ncalls++;

	TNplacements::eval();  // evaluate common arguments
	
	cval=0;
	hits=0;
	smgr->eval();  // calls SpritePoint.set_terrain
   
	if(hits>0 && cval>0 && cval<=1/**/){ // inside target radius
		nhits++;
		double x=1-cval;
#ifdef COLOR_TEST
		if(get_id()==1)
			c=Color(x,1,0);
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
