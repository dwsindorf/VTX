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

static double sval=0;
static double cval=0;
static double mind=0;
static double htval=0;
static int ncalls=0;
static int nhits=0;
static double thresh=1.0;    // move to argument ?
static double ht_offset=0.5; // move to argument ?

//static double roff_value=0;
//static double roff2_value=0.05*PI;

static double roff_value=1e-6;//0.5*PI;
static double roff2_value=0.5;

static double min_render_pts=2; // for render
static double min_adapt_pts=5; //  for adapt - increase resolution only around nearby sprites

static int cnt=0;
static int tests=0;
static int pts_fails=0;
static int dns_fails=0;

static TerrainData Td;
static SpriteMgr *s_mgr=0; // static finalizer
static int hits=0;
//#define DEBUG_PMEM

//#define COLOR_TEST
#define DENSITY_TEST

#define USE_AVEHT
//#define SHOW
#define MIN_VISITS 2
#define TEST_NEIGHBORS 1
#define TEST_PTS 
//#define SHOW_STATS
//#define DUMP
#ifdef DUMP
static void show_stats()
{
	if(s_mgr)
		s_mgr->dump();
	cout<<"calls="<<ncalls<< " hits="<<nhits<<endl;
}
#endif

class SData {
public:
    double v;
    double f;
    double value()   { return v;}
};
static SData   sdata[256];
static ValueList<SData*> slist(sdata,256);
static int          scnt;

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
	level_mult=0.2;
	slope_bias=0;
	ht_bias=0;
	lat_bias=0;
	rand_flip_prob=0.5;
	variability=1;
	dexpr=0;
	instance=0;
	select_bias=0;
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
	if(!first() || !scnt)
	    return;
	for(int i=0;i<scnt;i++){
	    slist.base[i]=sdata+i;
	}
	slist.size=scnt;
	slist.sort();
	
	for(int i=0;i<scnt;i++){
	   double f=slist.base[i]->f;
	   cval=f;
	}
}

void SpriteMgr::reset(){
	PlacementMgr::reset();
	tests=pts_fails=dns_fails=0;
	cval=0;
	scnt=0;
}
//-------------------------------------------------------------
// SpritePoint::set_terrain()	impact terrain
//-------------------------------------------------------------
bool SpriteMgr::valid()
{ 
    tests++;

#ifdef TEST_PTS
	double mps=min_render_pts;
	if(TheScene->adapt_mode())
		mps=min_adapt_pts;
	Point pv=MapPt;
	double d=pv.length();
	
	double r=TheMap->radius*size;
	double f=TheScene->wscale*r/d;
    double pts=f;
    if(pts<mps){
    	pts_fails++;
    	return false;
    }
#endif  
    // TODO set density biases here ?

    if(density<=0){
    	dns_fails++;
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
	for(int i=0;i<tp->sprites.size;i++){
		TerrainProperties::sid=i;
		tp->sprites[i]->setProgram();
	}
	
	glBegin(GL_POINTS);
	
	// render the sprites
	int n=Sprite::sprites.size;
	int flip=0;
	for(int i=n-1;i>=0;i--){
		SpriteData *s=Sprite::sprites[i];
		int id=s->instance;//s->get_id();
		Point t=s->center;
		double pts=s->pntsize;
				
		// random reflection - based on sprite hash table center position
		Point4DL pp=s->point;
		if(s->flip())
			flip=Random(pp.x+2,pp.y+6,pp.z+10)+0.5>s->rand_flip_prob?0:1;
		
		double x=Random(pp.x,pp.y,pp.z);//+0.5;
		double rv=s->variability*x;
		pts*=1+rv;
		
		double sel=0.1;	
		double r=0;
		double sid=0;
		double nn=(s->sprites_dim*s->sprites_dim);
		double sb=0;

		if(s->sprites_dim>1){ // random selection in multirow sprites image
			r=2*Random(pp.x+1,pp.y+1,pp.z+1);//)+0.5;
			r=clamp(r,-1,1);
			sid=s->get_id();
			sb=(1-s->select_bias)*r*(nn);
			sel=sid+sb+0.5;
			sel=sel>nn?sel-nn:sel;
			
			sel=sel<0?nn+sel:sel;
			sel=clamp(sel,0,nn-1);
		}
		int rows=s->sprites_dim;
	    int y1=sel/rows;
	    int sy=rows-y1-1.0;
	    int sx=sel-rows*y1;//+0.1;
	    
	    //cout<<(int)sid<<" "<<(int)sel<<" "<<r<<" "<<sb<<endl;

		glVertexAttrib4d(GLSLMgr::TexCoordsID,id+0.1, rows, pts, sel);
		glVertexAttrib4d(GLSLMgr::CommonID1, flip, rows, sx, sy);
		
		Point pn=Point(pp.x,pp.y,pp.z);
		Point ppn=pn.normalize();
	    glVertex3dv(t.values());
	    glNormal3dv(ppn.values());
	    //if(i<10)
		//printf("%d x:%-1.5g y:%-1.5g z:%-1.5g d:%-4.1f r:%-4.1f s:%-1.5f\n ",n-i,t.x,t.y,t.z,s->distance/FEET,s->radius/FEET);	
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
SpritePoint::SpritePoint(SpriteMgr&mgr, Point4DL&p,int n) : Placement(mgr,p,n)
{
	ht=0;
	aveht=0;
	wtsum=0;
	dist=1e16;
	visits=0;
	hits=0;
	mind=1e16;
	sprites_dim=mgr.sprites_dim;
	variability=mgr.mult;
	rand_flip_prob=mgr.rand_flip_prob;
	select_bias=mgr.select_bias;
	instance=mgr.instance;
	//cout<<instance<<" ";

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
	sval=0;
	
	if(d>thresh)
		return false;
	visits++;
    flags.s.active=true;
	sval=lerp(d,0,thresh,0,1);

    double wt=1/(0.01+sval);
    aveht+=Height*wt;
	
    wtsum+=wt;

	if(d<dist){
		ht=Height;
		dist=d;
		mind=d;
		hits++;
	}
	::hits++;

 	sdata[scnt].v=hid;
   	sdata[scnt].f=sval;
  	if(scnt<255)
  	    scnt++;
	return true;
}

void SpritePoint::reset(){
	flags.s.active=0;
	visits=0;
	hits=0;
	dist=1e6;
	aveht=0;
	wtsum=0;
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
SpriteData::SpriteData(SpritePoint *pnt,Point vp, double d, double ps){
	type=pnt->type;
	ht=pnt->ht;
	
	point=pnt->point;
	
	aveht=pnt->aveht/pnt->wtsum;
	center=vp;
	radius=pnt->radius;
    pntsize=ps;
 	distance=d;//TheScene->vpoint.distance(t);
 	sprites_dim=pnt->sprites_dim;
	visits=pnt->visits;
	variability=pnt->variability;
	rand_flip_prob=pnt->rand_flip_prob;
	select_bias=pnt->select_bias;
	instance=pnt->instance;
}

void SpriteData::print(){
	char msg[256];
	Point pp=Point(point.x,point.y,point.z);
	//pp=pp.normalize();
	//Point ps=pp.spherical();
	double h=TheMap->radius*TheMap->hscale;
	
	//sprintf(msg,"visits:%-1d t:%-1.6g p:%-1.3g ht:%-1.4f aveht:%-1.4f dist:%g",visits,ps.x+180,ps.y,h*ht/FEET,h*aveht/FEET,distance/FEET);
	sprintf(msg,"visits:%-1d x:%d y:%d z:%d ht:%-1.4f aveht:%-1.4f dist:%g",visits,point.x,point.y,point.z,h*ht/FEET,h*aveht/FEET,distance/FEET);
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
	sprites_dim=((TNsprite *)e)->sprites_dim;
	valid=false;
}

void Sprite::reset()
{
	sprites.free();
	TerrainProperties *tp=Td.tp;
	for(int i=0;i<tp->sprites.size;i++){
		Sprite *sprite=tp->sprites[i];
		sprite->mgr()->free_htable();
	}
}

void Sprite::set_image(Image *i, int d){
	image=i;
	sprites_dim=d;
	valid=false;
	if(texture_id>0)
		glDeleteTextures(1,&texture_id);
}
//-------------------------------------------------------------
// Sprite::collect() collect valid sprite points
//-------------------------------------------------------------
void Sprite::collect()
{
#ifdef TEST_PTS
	if(tests>0)
		cout<<"tests:"<<tests<<" fails  pts:"<<100.0*pts_fails/tests<<" %"<<" dns:"<<100.0*dns_fails/tests<<endl;
#endif
	//cout<<"znear:"<< TheScene->znear<<" zfar:"<<TheScene->zfar<<" "<<TheScene->zfar/TheScene->znear<<endl;
	int new_sprites=0;
	int bad_pts=0;
#ifdef SHOW_STATS	
	int trys=0;
	int visits=0;
	int bad_visits=0;
	int bad_valid=0;
	int bad_active=0;
#endif	

	TerrainProperties *tp=Td.tp;
	for(int i=0;i<tp->sprites.size;i++){
#ifdef SHOW_STATS	
		trys=visits=bad_visits=bad_valid=bad_active=bad_pts=new_sprites=0;
#endif
		Sprite *sprite=tp->sprites[i];
		sprite->mgr()->ss();
		SpritePoint *s=(SpritePoint*)sprite->mgr()->next();
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
		if(s->visits>=1 && s->flags.s.valid && s->flags.s.active){
			Point4D	p(s->center);
			Point pp=Point(p.x,p.y,p.z);
			Point ps=pp.spherical();
#ifdef USE_AVEHT
			double ht=s->aveht/s->wtsum;
#else
			double ht=s->ht;
#endif			
			Point center=TheMap->point(ps.x, ps.y,ht+s->radius*ht_offset/TheMap->radius);
			Point vp=Point(-center.x,center.y,-center.z)-TheScene->xpoint; // why the 180 rotation around y axis ????
			double d=vp.length(); // distance	
			double r=TheMap->radius*s->radius;
			double f=TheScene->wscale*r/d;
		    double pts=f;
		    double minv=lerp(pts,min_render_pts,10*min_render_pts,1,2*MIN_VISITS); 
		    bool pts_test=true;
#ifdef TEST_PTS
		    if(pts<min_render_pts){
		    	pts_test=false;
		    	bad_pts++;
		    }
#endif
		    if(pts_test && s->visits>=minv){
		    	new_sprites++;
		    	sprites.add(new SpriteData((SpritePoint*)s,vp,d,pts));
		    }
		}

		s=sprite->mgr()->next();
	  }	
#ifdef SHOW_STATS
	double usage=100.0*trys/PlacementMgr::hashsize;
	double badvis=100.0*bad_visits/trys;
	double badactive=100.0*bad_active/trys;
	double badpts=100.0*bad_pts/trys;
	cout<<sprite->name()<<" sprites "<<new_sprites<<" tests:"<<trys<<" %hash:"<<usage<<" %inactive:"<<badactive<<" %small:"<<badpts<<endl;
#endif

	} // next sprite
	//}
	if(sprites.size){
    	cout<<"total sprites collected:"<<sprites.size<<endl;
 		sprites.sort();
	}
#ifdef SHOW
	//int pnrt_num=sprites.size-1;
	int pnrt_num=min(2,sprites.size-1);

	for(int i=pnrt_num;i>=0;i--){
		cout<<i<<" ";
		sprites[i]->print();	
	}
#endif
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
	set_collapsed();
 	type=opts|SPRITES|NOLOD;
 	//cout<<"TNsprite ID="<<get_id()<<endl;
	setName(s);
	FREE(s);
	image=0;
	sprite=0;
    mgr=new SpriteMgr(SPRITES|NOLOD);
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

	//cout<<"TNsprite::init"<<endl;
	SpriteMgr *smgr=(SpriteMgr*)mgr;
	if(!image){
		char path[512];
		sprites_dim=getFilePath(sprites_file,path);
		if(sprites_dim){
			image=images.open(sprites_file,path);
			if(image)
				cout<<"Sprite image found:"<<path<<endl;
		}
	}
	smgr->sprites_dim=sprites_dim;
	if(!image){
		printf("TNsprites ERROR image %s not found\n",sprites_file);
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
	//TNplacements::set_id(i);
	//if(sprite)
	//	sprite->set_id(i);
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
		int size=Td.tp->sprites.size;
		instance=size;
		//cout<<instance<<" ";

		mgr->instance=instance;

		if(sprite)
			sprite->set_id(size);
		//set_id(size);
		Td.add_sprite(sprite);
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

	htval=Height;
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
	TerrainProperties *tp=TerrainData::tp;
		
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
	double hashcode=(mgr->levels+
		            1/mgr->maxsize
					+11*tp->id
					+7*instance
					);
	mgr->id=(int)hashcode+mgr->type+SPRITES+hashcode*TheNoise.rseed;
	
	sval=0;
	hits=0;
	cval=0;
	scnt=0;
	//cout<<instance<<" ";

	smgr->eval();  // calls SpritePoint.set_terrain
   
	if(hits>0){ // inside target radius
		nhits++;
		double x=1-cval;
#ifdef COLOR_TEST
		if(instance==0)
			c=Color(x,0,1);
		else
			c=Color(x,1,0);
		Td.diffuse=Td.diffuse.mix(c,0.5);
#endif
#ifdef DENSITY_TEST
		x=1/(cval+1e-6);
		x=x*x;//*x*x;
		Td.density+=lerp(cval,0,0.2,0,.05*x);
#endif
	}
 }
//-------------------------------------------------------------
// TNinode::setName() set name
//-------------------------------------------------------------
void TNsprite::setName(char *s)
{
	strcpy(sprites_file,s);
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

//-------------------------------------------------------------
// TNsprite::setSpritesTexture() set sprite image texture
//-------------------------------------------------------------
void TNsprite::setSpritesImage(char *name){
	if(strcmp(name,sprites_file)){
		setName(name);
		char path[512];
		sprites_dim=getFilePath(sprites_file,path);
		if(sprites_dim){
			delete image;
			image=images.open(sprites_file,path);
			if(image){
				cout<<"Sprite image found:"<<path<<endl;
				sprite->set_image(image,sprites_dim);
			}
		}
	}
}

int TNsprite::getFilePath(char*name,char *dir){
	sprites_dim=0;
	char path[512];
	path[0]=0;

	for(int i=0;i<4;i++){
		getSpritesFilePath(name,i+1,dir);
		sprintf(path,"%s.png",dir);
		if(FileUtil::fileExists(path)){
			sprites_dim=i+1;
			break;
		}
	}
	return sprites_dim;

}

void TNsprite::getSpritesDir(int dim, char*dir){
	char base[256];
	char dimdir[32];
  	File.getBaseDirectory(base);
  	switch(dim){
  	default:
  	case 1: strcpy(dimdir,"1x"); break;
  	case 2: strcpy(dimdir,"4x"); break;
  	case 3: strcpy(dimdir,"9x"); break;
  	case 4: strcpy(dimdir,"16x"); break;

 	}
 	sprintf(dir,"%s/Textures/Sprites/%s",base,dimdir);
}

void TNsprite::getSpritesFilePath(char*name,int dim,char *dir){
	char dimdir[512];
	getSpritesDir(dim,dimdir);
  	sprintf(dir,"%s/%s",dimdir,name);
}

char *TNsprite::getSpritesFile(GLuint &dim){
	dim=sprites_dim;
	return sprites_file;
}
