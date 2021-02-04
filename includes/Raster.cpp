
#include "Effects.h"
#include "RenderOptions.h"
#include "AdaptOptions.h"
#include "matrix.h"
#include "MapNode.h"
#include "MapClass.h"
#include "KeyIF.h"
#include "matrix.h"
#include "GLSLMgr.h"

#define OPTIMIZE

extern void DisplayErrorMessage(const char *format,...);
extern void init_test_params();
extern double lsin(double t);
extern double lcos(double t);
extern double Rand();
extern int  RandSeed;
extern void Srand(int);
extern int hits,visits;
extern double Theta, Phi,Rscale;
extern Point MapPt;

const double maxdp=0.5;

const double minrd=0.1;
const double maxrd=0.45;
const double maxd=0.45;

static int zbuf_valid=0;
static int pixels_valid=0;
static int filter_valid=0;
static int shadow_lights=0;
static int wRefl, wCol, wUnmod;
static MinMax mm1,mm2,mm3,mm4;
static int lighting_mode;
static double vbias;
static bool raster_init = true;

// buffers
#define LUTSIZE 2000

static int			npixels=0;
static int			colsize=0;
class ShadowFlags;

static int zvalid=0;
static GLfloat		*gdata=0;		// geometry data
static GLfloat		*zbuf1=0;		// eye zbuffer
static GLfloat		*zbuf2=0;		// shadow zbuffer
static GLubyte		*pixels=0;		// eye pixel buffer
static GLubyte		*image=0;		// pointer to current image buffer
static GLubyte		*ibuffs[8];		// image buffer pointers
static double       *avals=0;		// column buffer for cos alpha values
static double       *tlut=0;		// tangent lut
static double       *alut=0;		// arc-tangent lut
static double        bmax=0;
static double        bmin=0;
static int           nibuffs=0;     // number of malloced image buffers
static int           lindex=0;      // light image buffer offset
static ShadowFlags *sflags=0;		// shadow flags buffer

static GLint    vport[4];

static double zn,zf;
static double s_zmin,s_zmax,s_rval,s_dpmin,s_dpmax;
static double ws1,ws2;
static double shad_scale;
static double r_factor=0.75;


typedef struct _SFlags {
	unsigned int svalue	: 8;
	unsigned int avalue	: 12;
	unsigned int sfact	: 4;
	unsigned int afact	: 4;
	unsigned int valid	: 1;
	unsigned int check	: 1;
	unsigned int bg	    : 1;
} _SFlags;

typedef union SFlags {
	struct _SFlags	s;
	int       		l;
} SFlags;


const double sf_size=255.0;
const double af_size=15.0;

class ShadowFlags
{
	SFlags  flags;

public:
	ShadowFlags()				{ flags.l=0;}
	void init()				    { flags.l=0;}

	void set_value(double c)    { flags.s.svalue=(int)(c*sf_size); }
	double value()				{ return flags.s.svalue*(1.0/sf_size);}

	void set_sfact(double c)    { flags.s.sfact=(int)(c*af_size); }
	double sfact()				{ return flags.s.sfact*(1.0/af_size);}

	void set_afact(double c)    { flags.s.afact=(int)(c*af_size); }
	double afact()				{ return flags.s.afact*(1.0/af_size);}

	void set(double c)	        { flags.s.avalue=(int)(c*sf_size);}
	void accumulate(double c)	{ flags.s.avalue+=(int)(c*sf_size);}
	double total()				{ return flags.s.avalue*(1.0/sf_size);}

	int checked()				{ return flags.s.check;}
	void set_checked(int c)		{ flags.s.check=c;}

	int valid()					{ return flags.s.valid;}
	void set_valid(int c)		{ flags.s.valid=c;}

	void clr_flags()		    { flags.s.check=0;}

};


void print_minmax(const char *c, MinMax &mx, double f)
{
	if(!mx.cnt)
		return;
	TheScene->draw_string(DATA_COLOR,"%s cnt %d min %2.1f max %2.1f ave %2.1f",
			c,mx.cnt,f*mx.min,f*mx.max,f*mx.sum/mx.cnt);
}

void init_raster_specs()
{
}
void show_raster_specs()
{
	if(!Render.display(RASTINFO))
		return;

	TheScene->draw_string(HDR1_COLOR,"------- pixel info ---------------");
	TheScene->draw_string(DATA_COLOR,"image: %d (%d X %d)",
			vport[2]*vport[3],vport[2],vport[3]);
	//if(Render.show_water() && Raster.reflections()){
	if(Raster.do_water){
	    TheScene->draw_string(DATA_COLOR,"water: R %2.1f C %2.1f U %2.1f ALL %2.1f",
		1e-3*wRefl, 1e-3*wCol,1e-3* wUnmod,1e-3*(wRefl+wCol+wUnmod));
	}
	else
		TheScene->draw_string(DATA_COLOR,"water: disabled");
	if(Raster.do_vfog)
		TheScene->draw_string(DATA_COLOR,"vfog:  enabled");
	else
		TheScene->draw_string(DATA_COLOR,"vfog:  disabled");
	TheScene->draw_string(HDR1_COLOR,"-----------------------------------");
}

//-------------------------------------------------------------
// prep_zbuf()   convert zbuf values to distance values
//-------------------------------------------------------------
static void prep_zbuf()
{
	int i,j,k;
	zn=TheScene->znear;
	zf=TheScene->zfar;
	ws1=1/zn;
	ws2=(zn-zf)/zf/zn;
	for(k=0,i=0;i<vport[3];i++){	// rows (y)
		for(j=0;j<vport[2];j++,k++){	// cols (x)
			if(zbuf1[k]<1)
				zbuf1[k]=(float)(1/(ws2*zbuf1[k]+ws1));
		}
	}
}

//-------------------------------------------------------------
// prep_zbuf()   initialize reflection vectors
//-------------------------------------------------------------
static void init_cols()
{
#ifdef OPTIMIZE
	int i;
	double amax=RPD*TheScene->fov;
	double astep=amax/colsize;
	double alpha=astep;
	for(i=0;i<colsize;i++){
		avals[i]=sin(alpha);
		alpha+=astep;
	}
	double c2=2*acos(maxd); //2*acos(0.45)
	double c1=2*acos(0.0); // PI

	bmax=PI-astep-c2;
	bmin=PI-amax-c1;
	double b=bmin;
	double c=0;
	double bstep=(bmax-bmin)/LUTSIZE;
	double cstep=(maxd)/LUTSIZE;
	for(i=0;i<LUTSIZE;i++){
		alut[i]=2*acos(c);
		tlut[i]=1-1/tan(b);
		b+=bstep;
		c+=cstep;
	}
#endif
}

//************************************************************
// RasterMgr class raster effects manager
// 1. Uses OpenGL fixed functionality only (no shaders)
// 2. Effects supported
//  - haze (horizontal)
//  - vfog (vertical)
//  - water (reflection, transparency)
//  - anti-aliasing
//  - shadows
// 3. Occlusion node visibility optimization
//************************************************************
double RasterMgr::BS=1.0/255;
double RasterMgr::BF=4.0/255;
GLdouble RasterMgr::vpmat[16] ={1,0,0,0, 0,1,0,0, 0,0,0.5,0, 0,0,0.5,1};
GLdouble RasterMgr::sproj[16];
GLdouble RasterMgr::smat[16];
GLdouble RasterMgr::vmat[16];
GLdouble RasterMgr::ismat[16];
GLdouble RasterMgr::vproj[16];
GLdouble RasterMgr::ivproj[16]={1,0,0,0, 0,1,0,0,  0,0,1,0, 0,0,0,1};
GLdouble RasterMgr::ivpmat[16]={1,0,0,0, 0,1,0,0, 0,0,2,0, -1,-1,-1,1};

RasterMgr::RasterMgr()
{
	static int init_flag=1;
	set_options(DEFAULTS);
	set_idmode(0);
	set_accumulate(0);
	set_waterpass(0);
	set_bumptexs(0);
	set_fogpass(0);
	set_shadows_mode(0);

	bump_ampl=0.5;
	bump_shift=0.01;
	bump_bias=2;

	sky_color=Color(0,0,0,0);

	// HDR parameters

	hdr_min=0.35;
	hdr_max=1.8;

	// water parameters

	water_color1=Color(0,1,1,0.1);
	water_color2=Color(0,0.1,0.5,1);
	water_clarity=200*FEET;
	water_reflect=0.95;
	water_mix=0.8;
	water_specular=1;
	water_shine=10;
	water_level=0;
	water_dpr=0.2;
	water_dpm=0.5;

	// haze and fog parameters

	haze_color=Color(1,1,1);
	haze_value=0;
	haze_grad = 0.5;
	haze_min=100*FEET;
	haze_max=10*MILES;
	haze_zfar=MILES;

	fog_color=Color(1,1,1);
	fog_value=0;
	fog_vmin=0;
	fog_vmax=1000*FEET;
	fog_min=10*FEET;
	fog_max=100*FEET;

	// shadow parameters

	shadow_vcnt=0;
	shadow_count=0;

	shadow_darkness=1.0;
	shadow_randval=0.1;
	shadow_zmin=0.15;
	shadow_zmax=0.1;
	shadow_dscale=1.0;
	shadow_dpmin=0.0;
	shadow_dpmax=0.5;
	shadow_fov=1.0;
	shadow_dov=1.0;
	shadow_vbias=2;
	shadow_vshift=0.0;
	shadow_vsteps=4.0;
	shadow_fov_bg=1.3;
	shadow_dov_bg=1.0;
	shadow_vbias_bg=1.0;
	shadow_vshift_bg=0.4;
	shadow_vsteps_bg=1.1;
	shadow_edge_min=1;
	shadow_edge_width=1;
	shadow_color=Color(0,0,0);
	shadow_value=1;
	shadow_zfactor=0.05;
	shadow_blur=2.0;
	shadow_test=2;
	set_bgshadows(0);

	// filter parameters

	filter_color_ampl=0.2;
	filter_normal_ampl=1.0;
	set_filter(FILTER_DEFAULTS);
	set_use_shaders(1);

	filter_normal_min=0.1;
	filter_normal_max=0.3;

	filter_depth_max=0.3;
	filter_depth_min=0.5;

	set_lights(ALL_LIGHTS);

	reset();

	if(init_flag){
		add_initializer(&init_raster_specs);
		add_finisher(&show_raster_specs);
		init_flag=0;
	}
	idsize=idcnt=dcnt=0;
	idtbl=0;
}

RasterMgr::~RasterMgr()
{
	FREE(idtbl);
	FREE(pixels);
	FREE(gdata);

	for(int i=0;i<nibuffs;i++){
		FREE(ibuffs[i]);
	}
	nibuffs=0;
	FREE(zbuf1);
	FREE(zbuf2);
	FREE(sflags);
	FREE(alut);
	FREE(avals);
	FREE(tlut);
	idsize=0;
}

//-------------------------------------------------------------
// RasterMgr::set_defaults()   reset to default conditions
//-------------------------------------------------------------
void RasterMgr::set_defaults()
{
    set_options(DEFAULTS);
}

//-------------------------------------------------------------
// RasterMgr::reset()   clear raster flags
//-------------------------------------------------------------
void RasterMgr::reset()
{
	set_waterpass(0);
	set_fogpass(0);
	set_hazepass(0);
	set_bumps(0);
	if(TheScene && TheScene->changed_model())
		raster_init = true;
}

//-------------------------------------------------------------
// RasterMgr::modulate()   modulate render color
//-------------------------------------------------------------
void RasterMgr::modulate(Color &c)
{
	if(use_shaders())
		return;
	if(blend_factor)
		c=c.blend(blend_color,blend_factor);
	if(darken_factor)
		c=c.darken(darken_factor);
}

//================ Shadows functions ==========================

//-------------------------------------------------------------
// void RasterMgr::init_view()
// details:		called at start of view group
// want:		wiew width (wi) where:
//				w2=vbias*w1; and sum(wi)=vmax
// solution:    sum(wi)=vbias*w1+vbias^2*w1..=vbias^n*w1
//				w1=vmax/(n*sum(vbias^i))
//-------------------------------------------------------------
void RasterMgr::init_view()
{
	double z=0,sum=1,n,shift,vsteps;
	vsteps=shadow_vsteps;
	if(farview()){
		//vsteps*=shadow_vsteps_bg;
		vbias=shadow_vbias_bg;
		shift=shadow_vshift_bg;
	}
	else{
		vbias=shadow_vbias;
		shift=shadow_vshift;
	}
	n=(int)vsteps;
	n=n<1?1:n;
	shadow_vsteps=n;
	for(int i=0;i<n;i++){
		z+=sum;
		sum=sum*vbias;
	}
	shadow_zrange=shadow_vzf-shadow_vzn;
	//shadow_vstep=shadow_vmax/z;
	shadow_vstep=(shadow_vmax-0.5*shadow_vmin)/z;
	shadow_vleft=shadow_vmin+shift*(shadow_vmax-shadow_vmin);
    shadow_vright=shadow_vleft+shadow_vstep;
	//if(shadow_vright>shadow_vmax)
	//	shadow_vright=shadow_vmax;

	shadow_vcnt=0;
}

//-------------------------------------------------------------
// void RasterMgr::next_view()
//              called at start of each view
//	            shadow_vleft=shadow_vright
//-------------------------------------------------------------
int RasterMgr::next_view()
{
	shadow_vleft=shadow_vright;
	shadow_vstep*=vbias;
	shadow_vright=shadow_vleft+shadow_vstep;

	//if(shadow_vright>=shadow_vmax)
	//	shadow_vright=shadow_vmax;

	if(single_view())
		shadow_vleft=shadow_vmax;
	shadow_vcnt++;
	shadow_count++;
	return shadow_vcnt;
}

//-------------------------------------------------------------
// void RasterMgr::more_views()	 called at end of each view
//-------------------------------------------------------------
int RasterMgr::more_views()
{
	if(shadow_vcnt>=shadow_vsteps){
	//if(shadow_vleft>=shadow_vmax){
	//	shadow_vleft=shadow_vmin;
		return 0;
	}
	return 1;
}

//-------------------------------------------------------------
// void RasterMgr::clr_checked() called at start of each view
//-------------------------------------------------------------
void RasterMgr::clr_checked()
{
	for(int j=0;j<npixels;j++)
		sflags[j].set_checked(0);
}

//-------------------------------------------------------------
// void RasterMgr::init_shadows()	initialize shadows (all lights)
//-------------------------------------------------------------
void RasterMgr::init_shadows()
{
	s_dpmax=shadow_dpmax;
	s_dpmin=shadow_dpmin;
	s_rval=0.01*shadow_randval;

	setView();

	TheScene->getMatrix(GL_PROJECTION, vproj);
	TheScene->getViewport(vport);

	zn=TheScene->znear;
	zf=TheScene->zfar;
	ws1=1/zn;
	ws2=(zn-zf)/zf/zn;

	minv(vproj,ivproj,4);

	ivpmat[0]=(2.0/vport[2]);
	ivpmat[5]=(2.0/vport[3]);

	CMmmul(ivproj,ivpmat,ismat,4);

	glDisable(GL_LIGHTING);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_BLEND);
}
//------------------------------------------------------------
// void RasterMgr::set_light_view()	 called at start of each view
// TODO: need better light view algorithm
//-------------------------------------------------------------
void RasterMgr::set_light_view()
{
	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	GLSLMgr::clrBuffers();
	//cout<<"RasterMgr::set_light_view:"<<shadow_vcnt<<endl;

	double w,h,r,d,a,zn,zf,s,x,y,f,z;
    Point c,e,n,cv,l,cl;
	double ymax=2*shadow_vsize;

	double aspect=TheScene->aspect;
	double dov=1,fov=1,fov_step,dov_step;
		dov_step=shadow_dov;
		fov_step=shadow_fov;

	if(farview()){
		dov_step*=shadow_dov_bg;
		fov_step*=shadow_fov_bg;
	}
	dov=(shadow_vcnt+1)*dov_step;
	fov=(shadow_vcnt+1)*fov_step;
    l=light()->point;
	w=shadow_vright-shadow_vleft;
	h=2*shadow_vright*tan(RPD*0.5*TheScene->fov);
	cv=(TheScene->cpoint-TheScene->epoint).normalize();

	c=cv*(shadow_vleft+0.5*w);

	y=h>w?h:w;

	if(y>ymax)
	    y=ymax;
	cl=(l-c).normalize();

	r=shadow_vzf-shadow_vzn;
    r=y>r?y:r;

   	s=1000*r;

	e=c+cl*s;
    d=e.distance(c);
	fov=fov*2*DPR*atan(0.5*y/d);

	zn=d-4*dov*r;

	double s1,s2;
 	Point sp=TheScene->shadowobj->point;
 	double rr=shadow_vsize+TheScene->height;
 	if(e.intersect_sphere(c,sp,rr,s1,s2)>=0){
	    double z1=s1*d;
  	    //printf("intersect at d %g s1 %g s2 %g z1 %g zn %g\n",d,s1,s2,z1,zn);
 	    if(z1<zn)
 	         zn=z1;
	}
	if(zn<r)
		zn=r;
	zf=d+shadow_dov*r;
	//n=cl.cross(cv);
	//n=cl.cross(n).normalize();
	n=TheScene->npoint.normalize(); // NB
    double sf=farview()?2:1;
    //double sf=1;
    f=sf*shadow_zfactor/shadow_vzf;
	s_zmin=f*shadow_zmin;
	s_zmax=s_zmin+f*shadow_zmax;
//	char tmp[256];
//	sprintf(tmp,"# %d d %g h %g w %g y %g zn %g zf %g fov %g",shadow_vcnt,d,h,w,y,zn,zf,fov);
//	cout << tmp <<endl;

	TheScene->perspective(fov, aspect, zn, zf, e, c, n);
//	TheScene->perspective(1.8*fov, 1.2,0.999*zn, zf, e, c, n);

	TheScene->getMatrix(GL_PROJECTION,sproj);

	CMmmul(vpmat,sproj,smat,4);

	if(!maxview())
	    return;

	// optimize filling factor for light view

	Bounds vb=*TheScene->bounds;

    // get look matrix for bounds z values (l.s.)

	GLdouble look[16];
	TheScene->lookat(e,c,n);
	TheScene->getMatrix(GL_MODELVIEW, look);
	TheScene->loadIdentity();

	// transform bounds into light screen space (-1..1)

	int i;
    for(i=0;i<vb.size();i++){
        z=-vb.b[i].mz(look);
        vb.b[i]=vb.b[i].mm(sproj)/z;
    }
    Bounds lim;

    // get max and min x and y extent;

    for(i=0;i<8;i++)
        lim.eval(vb.b[i]);

    Point min=lim.bmin();
    Point max=lim.bmax();

    x=max.x-min.x;
    y=max.y-min.y;

    a=x/y;

    if(a<1)
        return;

	// scale original fov and aspect to fit screen
	// this is a hack. need to figure out how to combine bounds with
	// view fustrum and back-project for each view.

	if(a>1)
		TheScene->perspective(fov/a, a*aspect, zn, zf, e, c, n);
	else
		TheScene->perspective(fov, a*aspect, zn, zf, e, c, n);

	TheScene->getMatrix(GL_PROJECTION, sproj);
	CMmmul(vpmat,sproj,smat,4);
}

//-------------------------------------------------------------
// void RasterMgr::render_shadows()	 top level call
//-------------------------------------------------------------
void RasterMgr::render_shadows()
{
	double nscale;
	int cnt=0,i,start,end;
	set_shadows_mode(1);

	init_shadows();

	if(!accumulate()){
	    for(i=0;i<npixels;i++)
		    sflags[i].init();
	}

	nscale=all_lights()?1.0/Lights.size:1;

	end=Lights.size;
	start=0;

	if(last_light())
		start=end-1;
	else if(first_light())
		end=start+1;

	// muli-light loop

	shadow_lights=end-start;
	shad_scale=shadow_value*shadow_darkness;

	for(i=start;i<end;i++){
		set_light(i);
		double f=Lights.intensityFraction(light(),TheScene->shadowobj->point);

		//printf("light %d shadow %g\n",i,f);

		shadow_intensity=shad_scale*f;

		init_view();
		clr_checked();
		while(more_views()){
			set_light_view();
			shadow_view();
			next_view();
			cnt++;
		}
		shadow_light();

		if(!all_lights())
			break;
	}

	// end light loop

	set_shadows_mode(0);
}


//-------------------------------------------------------------
// void shadow_view()
//  render depth from eye POV (capture depth buffer)
//  for each stepped light view
//  - render from light POV
//  - read back depth buffer (light depth)
//  - for each pixel in EYE depth buffer
//    project into light space (screen(eye)->transform->object->screen(light))
//    compare depth with light space depth buffer
//    if light depth < eye depth pixel is in shadow
//  - accumulate result for each view
//-------------------------------------------------------------
void RasterMgr::shadow_view()
{

	GLdouble dz,zv;
	GLdouble v[4],w[4],s[4];
	int ix,iy;
	int i,j,k,l,n;
	int dpt=dptest();
	int rtst=0;      	// enable to randomize xy perterbation
	double dp=1,af,df,sf,gf;

    GLdouble wf,wz;
	//lindex=light_index()/3;
	lindex=light_index();
	image=ibuffs[lindex];
	//int li=light_index()-3*lindex;

	GLSLMgr::clrDepthBuffer();

	//set_top();
	//surface=1;
	Render.set_back();
    glPolygonOffset(2.0f, 1.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);

	TheScene->shadows_zvals();
	glDisable(GL_POLYGON_OFFSET_FILL);
	Render.set_front();
	//glDisable(GL_POLYGON_OFFSET_FILL);
	glReadPixels(0, 0, vport[2],vport[3],GL_DEPTH_COMPONENT, GL_FLOAT,zbuf2);

	for(k=0,i=0,l=0;i<vport[3];i++){
		for(j=0;j<vport[2];j++,k++){
			if(sflags[k].checked() || zbuf1[k]>=1.0)
				continue;

			// screen to object transformation
			// calculate 1/zv(eye) from zbuffer value, znear(eye) and zfar(eye)
			// wf=1/w[3]=1/zv=(zf-zbuf1[k]*(zf-zn))/zn/zf

			wz=zbuf1[k]; // eye depth value
			wf=ws2*wz+ws1;
			zv=1/wf;
			s[0]=j;
			s[1]=i;
			s[2]=wz;

			mvmul3(s,ismat,v);  // inverse project (screen->object)

			v[0]*=zv;			// remove perspective scaling
			v[1]*=zv;
			v[2]*=zv;
			v[3]=1.0;

			// object to screen transformation (light p.o.v.)

			mvmul4(v,smat,w);  	// forward project (object->screen)

			wf=1/w[3];			// add perspective scaling w[3]=1/z;
			if(rtst) // random ls. xy perturbation
				wf*=1-s_rval*Rand();
			w[0]=w[0]*wf;
			w[1]=w[1]*wf;
			w[2]*=wf;           // image zbuf depth -> light zbuf depth

			// equiv. openGL transformation routines (but ~5Xs slower)
			// gluUnProject(j,i,zbuf1[k],vmodel,vproj,vport,&v[0],&v[1],&v[2]);
			// gluProject(v[0],v[1],v[2],vmodel,sproj,vport,&w[0],&w[1],&w[2]);

			ix=(int)w[0]; // light's x
			iy=(int)w[1]; // light's y

			if(ix>0 && iy>0 && ix < vport[2] && iy<vport[3]){
				n=iy*vport[2]+ix;
				l=k*3;
				sflags[k].set_checked(1);
				dz=w[2]-zbuf2[n];
				double s1=zv*s_zmin;
				double s2=zv*s_zmax;
				df=rampstep(s1,s2,dz,1,0.5);
				sf=rampstep(s1,s2,dz,0,1);
				af=1;
				gf=1;
				if(dpt){
					//dp=image[l]*BS-0.5;
					dp=image[l]*BS;
					gf=image[l+1]*BS;
					if(dp<=s_dpmax){
						af=rampstep(s_dpmin,s_dpmax,dp,1,0);
				    //gf=rampstep(0,s_dpmin,dp,0,1);
					}
					//dp=rampstep(s_dpmin,s_dpmax,dp,1,0);
					sf=clamp(sf+dp,0,1);
				}
				sflags[k].set_sfact(df);
				sflags[k].set_afact(af);
				sflags[k].set_value(sf*shadow_intensity*gf);
			}
		}
	}
}

//-------------------------------------------------------------
// RasterMgr::shadow_light()	accumulate shadow for light
//-------------------------------------------------------------
void RasterMgr::shadow_light()
{
	int y,x,n;
	int i,j,k,x1,y1,x2,y2,step=0;
	double v=1,af;
	double f;
	int dpt=soft_edges();
	double istep=shadow_edge_width;

	for(n=0,y=0;y<vport[3];y++){		// rows (y)
		for(x=0;x<vport[2];x++,n++){	// cols (x)
			if(!sflags[n].checked())
				continue;
			f=sflags[n].value();

			if(dpt){
				step=(int)(istep*sflags[n].sfact());
				if(step<shadow_edge_min)
					step=shadow_edge_min;
				if(step){
					y1=y>step?(y-step):0;
					y2=y1+2*step;
					y2=y2>=vport[3]?vport[3]:y2;
					x1=x>step?x-step:0;
					x2=x1+2*step;
					x2=x2>=vport[2]?vport[2]:x2;
					v=1;
					for(i=y1;i<y2;i++){
						for(j=x1;j<x2;j++){
							//if(i==y && j==x)
							//	continue;
							k=i*vport[2]+j;
							if(!sflags[k].checked())
								continue;
							double af1=sflags[k].afact();
							double af2=sflags[n].afact();
							af=af1<af2?af1:af2;
							f+=sflags[k].value()*af;
							if(af)
							v+=af;
						}
					}
					f=f/v;
				}
			}
			if(light_index()==0)
				sflags[n].set(f);
			else
				sflags[n].accumulate(f);
			sflags[n].set_valid(1);
		}
	}
}

//================ IdImage functions ==========================

//-------------------------------------------------------------
// RasterMgr::resize_idtbl()	reallloc id table
//-------------------------------------------------------------
void RasterMgr::resize_idtbl(int size)
{
	if(size > idsize){
		printf("realloc ids %d\n",size);
		REALLOC(size,MapNode*,idtbl);
		idsize=size;
	}
}

//-------------------------------------------------------------
// RasterMgr::alloc_idtbl()	alloc & init id table
//-------------------------------------------------------------
void RasterMgr::alloc_idtbl(int size)
{
	FREE(idtbl)
	MALLOC(size,MapNode*,idtbl);
	printf("alloc ids %d\n",size);
	idsize=size;
	reset_idtbl();
}

//-------------------------------------------------------------
// RasterMgr::reset_idtbl()	init id table
//-------------------------------------------------------------
void RasterMgr::reset_idtbl()
{
	idtbl[0]=0;
	idcnt=1;
	dcnt=1;
}

//-------------------------------------------------------------
// RasterMgr::setVisibleIDs() use ids from pixels to assign visibility bit
//-------------------------------------------------------------
void RasterMgr::setVisibleIDs()
{

	int i=0;
    int errs=0;
	static idu      idb;

	int n = TheScene->viewport[2] * TheScene->viewport[3];

	idb.l=0;
	int cnt=0;
	while(i<4*(n-1)){
		idb.c.red=roundf(gdata[i]*255);
		idb.c.green=roundf(gdata[i+1]*255);
		idb.c.blue=roundf(gdata[i+2]*255);
		float gval=gdata[i+3];

		idb.c.alpha=0;
		if(invalid_id(idb.l))
            errs++;
		else if(idb.l>0){
			if(!idtbl[idb.l]->visible()){
				TheMap->vnodes++;
				idtbl[idb.l]->clr_masked();
			}
		}
		i=i+4;
	}
    if(errs && raster_init){
        DisplayErrorMessage("%d illegal map id errors in viewport w:%d h:%d",
                errs,TheScene->viewport[2],TheScene->viewport[3]);
        raster_init = false;
    }
}

//-------------------------------------------------------------
// RasterMgr::pixelID()	map ids from pixels
//-------------------------------------------------------------
MapNode *RasterMgr::pixelID(int x,int y)
{
	static idu idb;
	int i = 4*(y*(TheScene->viewport[3]-1)+x);
	idb.l=0;
	idb.c.red=roundf(gdata[i]*255);
	idb.c.green=roundf(gdata[i+1]*255);
	idb.c.blue=roundf(gdata[i+2]*255);
	idb.c.alpha=0;
    //printf("d %d i %d  t %d\n",dcnt,i,idb.l);
	if(idb.l==0 || idb.l>dcnt)
		return 0;
	return idtbl[idb.l];
}

//-------------------------------------------------------------
// RasterMgr::read_ids()	get color bits from frame buffer
//-------------------------------------------------------------
void RasterMgr::read_ids()
{
	glFinish();
	glFlush();
	GLSLMgr::setFBOReadPass();
	glActiveTextureARB(GL_TEXTURE0);  // read frame buffer
	//glActiveTextureARB(GL_TEXTURE1);  // enable to read from fbo texture 1
	glReadPixels(0, 0, TheScene->viewport[2],TheScene->viewport[3],
			GL_RGBA, GL_FLOAT,gdata);
	glActiveTextureARB(GL_TEXTURE0);
	glUseProgramObjectARB(0);
}

//#define DEBUG_LIMITS
//-------------------------------------------------------------
// RasterMgr::getLimits()	get zn,zf from depth buffer
//-------------------------------------------------------------
void RasterMgr::getLimits(double &zn, double &zf)
{
	zn=1;
	zf=-1;
	if(!zvalid)
		getZbuf(0);
	double z1=TheScene->znear;
	double z2=TheScene->zfar;
	double ws1=1/z1;
	double ws2=(z1-z2)/z2/z1;

	int n = TheScene->viewport[2] * TheScene->viewport[3];
	for(int i=0;i<n;i++){
		double z=zbuf1[i];
		if(z==1.0)
			continue;
		zn=z<zn?z:zn;
		zf=z>zf?z:zf;
	}
	zn=1.0/(ws2*zn+ws1);
	zf=1.0/(ws2*zf+ws1);
#ifdef DEBUG_LIMITS
	cout <<"RasterMgr::getLimits zn:"<<zn/FEET<<" zf:"<<zf/FEET<<endl;
#endif

}
//=============================================================
//-------------------------------------------------------------
// RasterMgr::getZbuf()	capture z buffer
//-------------------------------------------------------------
void RasterMgr::getZbuf(int i)
{
    if(i==0){
		//if(!zvalid)
		glReadPixels(0, 0, vport[2],vport[3],GL_DEPTH_COMPONENT, GL_FLOAT,zbuf1);
		zvalid=1;
	}
	else
		glReadPixels(0, 0, vport[2],vport[3],GL_DEPTH_COMPONENT, GL_FLOAT,zbuf2);
}

//-------------------------------------------------------------
// RasterMgr::getAuxImage()	capture pixel buffer
//-------------------------------------------------------------
void RasterMgr::getAuxImage(int i)
{
	glReadPixels(0, 0, vport[2],vport[3],GL_RGB, GL_UNSIGNED_BYTE,ibuffs[i]);
}

//-------------------------------------------------------------
// RasterMgr::getPixels()	capture pixel buffer
//-------------------------------------------------------------
void RasterMgr::getPixels()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	//if(!pixels_valid)
		glReadPixels(0, 0, vport[2],vport[3],GL_RGBA, GL_UNSIGNED_BYTE,pixels);
	//glEnable(GL_DEPTH_TEST);
	pixels_valid=1;
}

//-------------------------------------------------------------
// RasterMgr::drawPixels()	draw pixel buffer
//-------------------------------------------------------------
void RasterMgr::drawPixels()
{
	glDrawPixels(vport[2],vport[3],GL_RGBA, GL_UNSIGNED_BYTE,pixels);
	glEnable(GL_DEPTH_TEST);
}

//-------------------------------------------------------------
// RasterMgr::render_shading()	render shading (raster shadows)
//-------------------------------------------------------------
void RasterMgr::render_shading()
{
	extern void invalidate_normals();
    set_draw_nvis(1);
    int nl=Lights.size;
	//invalidate_normals();
	glColor4d(1,1,1,1);
    for(int i=0;i<nl;i++){
    	lindex=i;
		surface=1;
		set_render_type(SHADING);
		TheScene->render_raster();
		surface=2;
		TheScene->render_raster();
		glFlush();
		getAuxImage(i);
	}
	surface=1;
	set_render_type(0);
    set_draw_nvis(0);
	//invalidate_normals();
}

//-------------------------------------------------------------
// RasterMgr::render_image() render auximage
//-------------------------------------------------------------
void RasterMgr::render_auximage()
{
    set_draw_nvis(1);
	TheScene->render_raster();
	glFlush();
    set_draw_nvis(0);
}

//-------------------------------------------------------------
// RasterMgr::vertex() render image vertex
// 			  called from MapNode::Rvertex
//-------------------------------------------------------------
void RasterMgr::vertex(MapNode *node)
{
	double dp,r=0,g=0,b=0;
	static double lastdp=0;
	Point *norm;

	MapData *data=node->ndata();
	MapData *d=node->surface_data(data);
	if(!d)
		return;

	Point pt=d->point();
	Texture *t;
    Color c=WHITE;

	switch(render_type()){
	case SHADERS:
		{
			double type=0,vfog=0;
			if(TheMap->object==TheScene->viewobj)
				type = (d->type()+1.5); // note: for water floor(g)=1;
			Point *norm=node->normal(d);
			if(norm)
				glNormal3dv(norm->values());

			double phi = d->phi() / 180;
			double ht=d->Z()*Rscale;

			if (GLSLMgr::CommonID >= 0) // b:reflect a:diffuse calculated in shader
				glVertexAttrib4d(GLSLMgr::CommonID, ht, type, d->density(), phi);
		}
		break;
	case NORMALS:
		{
		Point *p=node->normal(d);
		if(p)
			glNormal3dv((double*)p);
		}
		break;
	case SHADING:
		TheMap->object->map_color(d,c);
		//g=shadow_darkness*c.alpha();
		g=shadow_darkness;

		// image map for raster shadows (norm-light dp)

		if(!lighting_mode)
		    r=0;
		else{
			Point *nptr=node->normal(data);
			static Point norm;
			if(nptr)
				norm=*nptr;
			glNormal3dv((double*)&norm);
			Point p=lpts[lindex]-pt;
			r=rampstep(shadow_dpmin,shadow_dpmax,norm.dot(p),1,0);
		}
		glColor3d(r,g,b);
		break;
	case BUMPS:
		t=TheMap->texture;
		if(!t)
			break;
		if(d->type()!=TheMap->tid)
			break;

		d->apply_texture(); // this call will set tx->s & tx->t

		switch(bmppass){
		case 1:
			t->texCoords(GL_TEXTURE0);
			break;
	    case 2:
	        {
				Point *nptr=node->normal(data);
				static Point N;
				if(nptr){
					N=*nptr;
					N.normalize();
				}
				Point L=lpts[0]-pt;

				L=L.normalize();
				MapData *v[4]={0};

				bool oktorun=node->TanGroup(v);
				if(!oktorun)
					break;

				Point V1=v[GUP]->point()-v[GDN]->point();
				Point V2=v[GRT]->point()-v[GLT]->point();

				double s1,s2;

				s1=v[GUP]->phi()-v[GDN]->phi();  // why does this seem to work for 1D texs
				s2=v[GRT]->phi()-v[GLT]->phi();  // instead of texS ?

				Point T=(V1*s1)+(V2*s2);

				double ds=T.length()*t->scale*t->width();
				double mf=TheScene->wscale/ds; // pixels per image point

                double mag=rampstep(0,bump_bias,mf,0,bump_shift);

				T=T.normalize();
				T.z=0;
				Point B=T.cross(N);

				T=N.cross(B);
				T=T.normalize();
				T.z=0;

				Point p;
				p.x=L.x*T.x+L.y*T.y+L.z*T.z;
				p.y=L.x*B.x+L.y*B.y+L.z*B.z;

			    t->bumpCoords(GL_TEXTURE0,p.x*mag,p.y*mag);
	    	}
			break;
	    }
	    break;

	case AUXIMAGE:
		// blue byte: encode vfog amplitude for fog
		if(do_vfog)
			b=d->density()*rampstep(fog_vmin,fog_vmax,d->height(),1,0);
		// green byte: encode layer type+water flag for filter and water
        if(Render.dealias()||filter_show())
			g = (d->type()+1)*BS;
		else if(surface==2 && do_water && node->has_water())
			g=FWATER*BS; // water flag
		// red byte: encode dp normal-eye for water reflections and filter
		norm=node->normal(data);
		if(norm){
			Point v=TheScene->eye-pt;
			v=v.normalize();
			dp=norm->dot(v);
			lastdp=dp;
		}
		r=rampstep(0,1,lastdp,0,1);
		glColor3d(r,g,b);
		break;
	}
	glVertex3dv((double*)(&pt));
}

//-------------------------------------------------------------
// RasterMgr::init_shading()	render shading (raster shadows)
//-------------------------------------------------------------
void RasterMgr::init_lights(int enbl)
{
	lighting_mode=enbl;
	for(int i=0;i<Lights.size;i++)
	    lpts[i]=Lights[i]->point.mm(TheScene->invViewMatrix);
}

//-------------------------------------------------------------
// RasterMgr::applyShadows()	apply shadow to image
//-------------------------------------------------------------
void RasterMgr::applyShadows()
{
	int y,x,n;
	double f1,f2;
	GLubyte *c;
	double r,g,b;
	const double bf=1.0/255;

	//Color scol=shadow_color.blend(sky_color,0.5*sky_color.alpha());
	Color scol=shadow_color.blend(sky_color,1-shadow_color.alpha());
	//double smax=shadow_color.alpha()*rampstep(1,0,sky_color.alpha(),0.3,1);
	r=scol.rb()*bf;
	g=scol.gb()*bf;
	b=scol.bb()*bf;

	for(n=0,y=0;y<vport[3];y++){		// rows (y)
		for(x=0;x<vport[2];x++,n++){	// cols (x)
			if(!sflags[n].valid())
				continue;
			f1=clamp(sflags[n].total(),0,1);
			if(f1){
				c=pixels+n*4;
				f2=1.0-f1;
//				c[0]=(GLubyte)(255);
//				c[1]=(GLubyte)(0);
//				c[2]=(GLubyte)(0);
				c[0]=(GLubyte)(f2*c[0]+f1*r*c[0]);
				c[1]=(GLubyte)(f2*c[1]+f1*g*c[1]);
				c[2]=(GLubyte)(f2*c[2]+f1*b*c[2]);
			}
		}
	}
}

//-------------------------------------------------------------
// RasterMgr::applyFilter()	apply filter to image
//-------------------------------------------------------------
void RasterMgr::applyFilter()
{
	TheScene->getViewport(vport);
	make_filter_mask();
	getPixels();

	if(Render.dealias()){

		switch(filter_type()){
		case FILTER_TYPE2:
			boxFilter();
			break;
		case FILTER_TYPE1:
			pointFilter();
			break;
		case FILTER_TYPE3:
			circleFilter();
			break;
		}
	}
	if(filter_show())
		show_filter_mask();
	glDisable(GL_DEPTH_TEST);
	glDrawPixels(vport[2],vport[3],GL_RGBA, GL_UNSIGNED_BYTE,pixels);
	//glClear(GL_DEPTH_BUFFER_BIT);
}

//-------------------------------------------------------------
// RasterMgr::show_filter_mask() show filter mask
//-------------------------------------------------------------
void RasterMgr::show_filter_mask()
{
	int i,k;
	GLubyte *image;
	image=ibuffs[0];
	int n=vport[3]*vport[2];
	for(i=0;i<n-1;i++){
		k=i*4;
		GLubyte b=image[k+1]&FMAX;
		if(image[k+1]&FZDIFF){
			int r=pixels[k+2]+b*2;
			pixels[k]/=2;
			pixels[k+1]=(r>255?255:r);
			pixels[k+2]=(r>255?255:r);
		}
		else{
			int r=pixels[k]/2+b*2;
			pixels[k]=(r>255?255:r);
			pixels[k+1]/=2;
			pixels[k+2]/=2;
		}
	}
}

//-------------------------------------------------------------
// RasterMgr::make_filter_mask() generate a mask for anti-aliasing
//-------------------------------------------------------------
//  I. Image format
//   - All image data is reflected vertically so that row zero is the bottom row
//   - Input ibuffs[0]
//         r  dot-product between eye and normal (0..255)
//         g  FWATER or zero
//         b  fog and haze (0..255)
//   - Output ibuffs[0]
//         r  unchanged
//         g  filter amplitude(0..64) [|FWATER] [|FZDIFF]
//         b  unchanged
//  II. Depth discontinuity detection (ridgelines)
//   - Algorithm detects discontinuities in depth between neighboring pixels
//   - The depth(d) of a pixel at a given zbuffer value(z) is:
//
//     d = n x f/((n-f)z+f)
//
//    - The rate of depth change at a given z value is:
//
//      Dd/Dz = n x f * (n-f)/((n-f)z+f)^2  (using the quotient rule)
//
//    - Differences in unadjusted depth values are used since the default encoding
//      actually has a convenient non-linearity, where a greater change is required
//      for pixels at larger depths to exceed the discontinuity test threshold.
//      This reduces the frequency of false detection of ridgelines due to normal
//      perspective projection.
//
//  III. Normal discontinuity detection
//   - Algorithm detects discontinuities in dot product between eye to
//     pixel vector and normal at pixel
//-------------------------------------------------------------
void RasterMgr::make_filter_mask()
{
	if(filter_valid)
	    return;
	if(!zvalid)
		getZbuf(0);

	filter_valid=1;
	int i,j,k,n,w,wc;
	GLubyte *image;
	GLubyte b,t;
	image=ibuffs[0];
	double z,c,dc,dz,cmax,zmax,delta;

	w=vport[2];
	wc=3*w;

	double mindp=filter_normal_min;
	double maxdp=1.01*filter_normal_min+filter_normal_max;
	double mindz=filter_depth_min*1e-2;
	double maxdz=1.01*mindz+2*filter_depth_max*1e-2;
	double fampl=filter_color_ampl*FMAX;

	if(filter_edges() && !filter_bg()){	// mask=edges only
		for(i=0;i<vport[3]-1;i++){	    // rows (y)
			for(j=0;j<w-1;j++){		   	// cols (x)
				n=i*w+j;
				z=zbuf1[n];   			// depth
				k=3*n;
				if(z==1.0) { 			// background
					//image[k+1]=0;
					continue;
				}
				t = image[k+1];
				if(t==WATER+1)
					image[k+1]=FWATER;
				else
					image[k+1]=0;
				// test for background-foreground boundary
				if(zbuf1[n+w]==1.0){  	// up-depth
					b=(GLubyte)fampl;
					image[k+1]|=b|FZDIFF;
					image[k+1+wc]|=b;
					image[k+1-wc]|=b;
					continue;
				}
				// test for layer edge boundary
				if(t!=image[k+4] || t!=image[k+wc+1] || t!=image[k+4+wc]){
					b=(GLubyte)fampl;
					image[k+1]|=b;
					continue;
				}
				zmax=cmax=0;
				if(filter_depth()){					// depth edge test
					delta=fabs(z-zbuf1[n+1]); 		// right
					dz=fabs(z-zbuf1[n+w]);    		// down
	                delta=dz>delta?dz:delta;
	                dz=fabs(z-zbuf1[n+w+1]);  		// down-right
	                delta=dz>delta?dz:delta;
					if(delta>mindz)
		                zmax=rampstep(mindz,maxdz,delta,0,fampl);
				}
				if(filter_normal()){			  	// normal edge test
					c=image[k]*BS;                	// dot-product eye-normal
					delta=fabs(image[k+wc]*BS-c); 	// down
					dc=fabs(image[k+3]*BS-c);     	// right
					delta=dc>delta?dc:delta;
					dc=fabs(image[k+3+wc]*BS-c);  	// down-right
					delta=dc>delta?dc:delta;
					if(delta>mindp)
						cmax=rampstep(mindp,maxdp,delta,0,fampl);
				}
				if(!zmax && !cmax)
					continue;
				if(cmax>zmax)
					image[k+1]|=(GLubyte)cmax;
				else
					image[k+1]|=FZDIFF|(GLubyte)zmax;
			}
		}
		return;
	}
	if(filter_bg()&&!filter_edges()){		// mask=all pixels
		for(i=0;i<vport[3]*vport[2];i++)
			image[i*3+1]|=FMAX;
	}
	else if(!filter_bg()&&!filter_edges()){ // mask=all fg
		for(i=0;i<vport[3]*vport[2];i++){
			if(zbuf1[i]<1.0)
				image[i*3+1]|=FMAX;
		}
	}
	else{ // mask=bg+edges
		for(i=0,n=0;i<vport[3];i++){		// rows (y)
			for(j=0;j<vport[2]-2;j++){		// cols (x)
				n=i*vport[2]+j;
				k=3*n;
				if(i>0 && zbuf1[n+vport[2]]==1.0)
					image[k+1]|=FMAX;
				else if(zbuf1[n]==1.0||image[k+1]||image[k+4])
					image[k+1]|=FMAX;
			}
		}
	}
}

//-------------------------------------------------------------
// RasterMgr::pointFilter() average pixel & pixel to the right
//-------------------------------------------------------------
void RasterMgr::pointFilter()
{
	int y,x,n,k;
	GLubyte *image,*p0,*p1;
	image=ibuffs[0];
	double f1,f2;

	for(y=0,n=0;y<vport[3];y++){			// rows (y)
		for(x=0;x<vport[2]-1;x++,n++){		// cols (x)
			k=4*n;
			GLubyte b=image[k+1]&FMAX;
			if(b==0)
				continue;
			if(filter_stretch()){
				p1=pixels+k;
				p0=p1+3;
			}
			else{
    			p0=pixels+k;
    			p1=p0+3;
            }

			f2=0.5*BF*b;
			f1=1-f2;
			p0[0]=(GLubyte)(f1*p0[0]+f2*p1[0]);
			p0[1]=(GLubyte)(f1*p0[1]+f2*p1[1]);
			p0[2]=(GLubyte)(f1*p0[2]+f2*p1[2]);
		}
		n++;
	}
}
//-------------------------------------------------------------
// RasterMgr::boxFilter() average pixel, 4 neighbors
//-------------------------------------------------------------
void RasterMgr::boxFilter()
{
	int y,x,n,i,k;
	GLubyte *image,*p0,*p1,*p2,*p3;
	image=ibuffs[0];
	double f1,f2,f3,fb,fc;

	for(y=0,n=0;y<vport[3]-1;y++){			// rows (y)
		for(x=0;x<vport[2]-1;x++,n++){		// cols (x)
			k=3*n;
			GLubyte b=image[k+1]&FMAX;
			if(b==0)
				continue;
			fb = BF*b;
			fc = fb/1.41;

			f1 = 1.0/(1+2*fb+fc);
			f2 = f1*fb;
			f3 = f2/1.41;

			p0=pixels+4*n;
			p1=p0+3; // right
			p2=p0+3*vport[2]; // up
			p3=p2+3; // up-right
			for(i=0;i<3;i++)
				p0[i]=(GLubyte)(f2*p1[i]+f2*p2[i]+f3*p3[i]+f1*p0[i]);
		}
	}
	for(x=0;x<vport[2]-1;x++,n++){  // last row
		k=3*n;
		GLubyte b=image[k+1]&FMAX;
		if(b==0)
			continue;
		f2=BF*b;
		f1=1-f2;

		p0=pixels+4*n;
		p1=p0+3;
		for(i=0;i<3;i++)
			p0[i]=(GLubyte)(f2*p1[i]+f1*p0[i]);
	}
}

//-------------------------------------------------------------
// RasterMgr::circleFilter() average pixel, 8 neighbors
//-------------------------------------------------------------
void RasterMgr::circleFilter()
{
	int y,x,n,i;
	GLubyte *image,*p0,*p1,*p2,*p3,*p4,*p5,*p6,*p7,*p8;
	image=ibuffs[0];
	double f1=1,f2=0.6,f3=0.3,fb,fc;

	for(y=1,n=0;y<vport[3]-1;y++){			// rows (y)
		for(x=1;x<vport[2]-1;x++){		// cols (x)
			n=y*vport[2]+x;
			GLubyte b=image[3*n+1]&FMAX;
			if(!b)
				continue;
			p0=pixels+4*n;
			p1=p0-3;
			p3=p0+3;
			p4=p0+3*vport[2];
			p2=p0-3*vport[2];
			p5=p2-3;
			p6=p2+3;
			p7=p4-3;
			p8=p4+3;
			fb = BF*b;
			fc = fb/1.41;
			f1 = 1.0/(1+4*fb+4*fc);
			f2 = f1*fb;
			f3 = f2/1.41;

			for(i=0;i<3;i++)
				p0[i]=(GLubyte)(
				f1*p0[i]
				+f2*p1[i]+f2*p2[i]+f2*p3[i]+f2*p3[i]
				+f3*p5[i]+f3*p6[i]+f3*p7[i]+f3*p8[i]
				);
		}
	}
}

//-------------------------------------------------------------
// RasterMgr::applyFog()	apply fog to image
//-------------------------------------------------------------
void RasterMgr::applyFog()
{
	int i,j,k;
	GLubyte *c;
	GLdouble z,z1h=0,z2h=0,z1v=0,z2v=0,x1,x2;
	GLdouble f1=0,f2;
	GLdouble hc[3]={0},vc[3]={0};
	double f=TheScene->height/TheScene->maxht;
	double attn=rampstep(0,r_factor,f,1,0);
	double vf=fog_value*Render.fog_value()*attn;
	double hf=haze_value*Render.haze_value()*attn;

	vf=clamp(vf,0,1)/255.0;

	zn=TheScene->znear;
	zf=TheScene->zfar;
	ws1=1/zn;
	ws2=(zn-zf)/zf/zn;

	image=ibuffs[0];

	if(do_haze){
		z1h=haze_min;
		z2h=haze_max/haze_value;
		hc[0]=haze_color.rb();
		hc[1]=haze_color.gb();
   		hc[2]=haze_color.bb();
	}
	if(do_vfog){
		z1v=fog_min;
		z2v=fog_max/fog_value;
		vc[0]=fog_color.rb();
		vc[1]=fog_color.gb();
   		vc[2]=fog_color.bb();
	}

	int type=Render.fogmode();
	if(Render.draw_shaded())
		type = POW;
    double haze_amp=rampstep(0.0,0.4,haze_grad,2.0,1.0);
	for(k=0,i=0;i<vport[3];i++){		// rows (y)
		for(j=0;j<vport[2];j++,k++){	// cols (x)
			z=zbuf1[k];
			//if(z>=1)
			if(z==0 || z==1)
				continue;
			c=pixels+k*4;
			if(do_vfog){
				f1=image[k*3+2]*rampstep(z1v,z2v,z,0,vf);
				f2=(1-f1);
				c[0]=(GLubyte)(f2*c[0]+f1*vc[0]);
				c[1]=(GLubyte)(f2*c[1]+f1*vc[1]);
				c[2]=(GLubyte)(f2*c[2]+f1*vc[2]);
			}
			if(do_haze){
				switch(type){
				case LINEAR:
					f1=rampstep(z1h,z2h,z,0,1);
					break;
				case EXP:
					x1=z/z2h;
					x2=x1*x1;
					f1=1+x1+0.5*x2+0.16667*x1*x2;
					f1=1-1/f1;
					break;
				case EXP2:
					x1=z/z2h;
					x2=x1*x1;
					f1=1+x1+0.5*x2+0.16667*x1*x2;
					f1=1-1/f1;
					f1*=f1;
					break;
				case POW:
					x1=1.5*z/haze_zfar;
					//x1=z/z2h;
					//x1=haze_amp*pow(x1,2*haze_grad);
					x1=haze_value*pow(x1,2*haze_grad);
					x1=x1>1.0?1.0:x1;
					f1=x1;
					break;
				}
				f1*=hf;
				f2=(1-f1);
				c[0]=(GLubyte)(f2*c[0]+f1*hc[0]);
				c[1]=(GLubyte)(f2*c[1]+f1*hc[1]);
				c[2]=(GLubyte)(f2*c[2]+f1*hc[2]);
			}
		}
	}
}

//-------------------------------------------------------------
// void applyWaterdepth()	apply water depth cue to image
//-------------------------------------------------------------
void RasterMgr::applyWaterdepth()
{
	int i,j,k;
	double dz;
	GLubyte *c;
	GLdouble z1,z2,f;
	GLdouble f1,f2;
	double m[3];
	if(!reflections() && !water_modulation() &&!do_depth)
	    return;

	zn=TheScene->znear;
	zf=TheScene->zfar;
	ws1=1/zn;
	ws2=(zn-zf)/zf/zn;

	m[0]=water_color2.rb();
	m[1]=water_color2.gb();
	m[2]=water_color2.bb();

	image=ibuffs[0];
	GLubyte  *lighting=ibuffs[1];

	double af=rampstep(0,TheScene->maxht,TheScene->height,1,0);
	for(k=0,i=0;i<vport[3];i++){	// rows (y)
		for(j=0;j<vport[2];j++,k++){	// cols (x)
			if(!(image[k*4+1] & FWATER))
				continue;
			z2=zbuf1[k];
			if(z2==0||z2==1||zbuf2[k]==1)
				continue;

			double dp=image[k*3]*BS;
			if(dp==0)
				continue;
			c=pixels+k*4;
			if(do_depth){
				z1=1/(ws2*zbuf2[k]+ws1);
				dz=(z1-z2);
				dz=dz<0?0:dz;
				f=rampstep(0,water_clarity,dz,0,af);
				f1=1-f;
				f2=f;
				c[0]=(GLubyte)(f1*c[0]+f2*m[0]);
				c[1]=(GLubyte)(f1*c[1]+f2*m[1]);
				c[2]=(GLubyte)(f1*c[2]+f2*m[2]);
			}
			GLubyte *b=lighting+k*4;
			int s=c[0]+b[0];
			c[0]=s>255?255:s;
			s=c[1]+b[1];
			c[1]=s>255?255:s;
			s=c[2]+b[2];
			c[2]=s>255?255:s;
		}
	}
}

//-------------------------------------------------------------
// RasterMgr::applyReflections() apply reflections and sky tint
//
// ray-traced reflections:
// - uses current color buffer as a 2d environment map or LUT
//   algorithm
//
//    1. for each water pixel (p)
//    2. Calculate a reflected ray into scene (yz direction)
//    3. determine max projection of ray on screen (ymax)
//    4. trace ray in y projection (0..ymax)
//    5. for each y ray pixel if z[yi]<z[zbuffer] then ray intersects
//    6. if ray intersects use pixel[yi] for reflected color else next yi
//
//    ray math                                 e -----r
//    1. form a triangle e-p-r                  \a  b/      a=alpha
//       e=eye point(0,0)                      l \c /       b=beta
//       p=water pixel p-e                    ____\/________c=theta____
//       r=ray intersection vector r-e            p
//       alpha=angle between p and r
//       theta=2*angle between normal and p-e vector
//       beta=angle between r-p and r-e
//    2. alpha
//       step  : TheScene->fov/screen-ht(pixels)
//       start : 0 (step)
//       end   : fov*(1-sy)
//            max_angle = fov init_angle = sy*fov max_alpha=fov-init_angle
//    3. theta=2*acos(dp) dp=dot product (normal and p)
//    4. given 2 angles (alpha,theta) and one side (l=length(p)) can solve for r
//    5. beta=PI-alpha-theta (sum of angles in a triangle=PI)
//    6. r=l*(cos(alpha)+sin(alpha)*cos(beta)/sin(beta))
//-------------------------------------------------------------
void RasterMgr::applyReflections()
{
	int i,j,k,n,aflg=0;
	double f1,f2,f3,mix,cmod,rmod=0,z1,z2;
	GLubyte *c;
	GLubyte *p;
	int y=0;
	int rflag=reflections();
	int mflag=water_modulation();
	if(!rflag && !mflag)
	    return;
	wRefl=wCol=wUnmod=0;
	double m[3];
	Color sky=sky_color;
	Color water=water_color1;
	double saturation = water_color1.alpha();
	//Color sc=sky*(1-f)+water*f;
	Color sc=water;

	modulate(sc);
	m[0]=sc.rb();
	m[1]=sc.gb();
	m[2]=sc.bb();

	image=ibuffs[0];

	GLubyte s[3];
	s[0]=sky.rb();
	s[1]=sky.bb();
	s[2]=sky.gb();
	//double f=TheScene->height/TheScene->maxht;
	//double attn=rampstep(0,r_factor,f,1,0);
	double astep=RPD*TheScene->fov/vport[3];

	double attn=rampstep(0,TheScene->maxht,TheScene->height,1,0);
	if(attn==1)
	    return;
	double bscale=LUTSIZE/(bmax-bmin);
	double ascale=2*LUTSIZE*maxd;
	for(i=0;i<vport[3];i++){	// rows (y)
		for(j=0;j<vport[2];j++){	// cols (x)
			k=i*vport[2]+j;
			aflg=0;
			if(!(image[k*3+1] & FWATER)){
				wUnmod++;
				continue;
			}
			double dp=image[k*3]*BS;
			if(dp>=1||dp==0){
				wUnmod++;
				continue;
			}
			if(j>0)
				aflg=1;
			c=pixels+k*4;
			if(rflag){  // color modulation + reflection
				double dpr=dp*2.0;
				rmod=rampstep(0,1.5*water_dpr,dpr,water_reflect,0);
				n=0;
				if(rmod){
					double dy=2*vport[3]*dpr;
					y=(int)(i+dy);
					if(i==y)
						n=k;
					else if(y>=vport[3]){
						y=vport[3]-1;
						n=y*vport[2]+j;
					}
					else
						n=y*vport[2]+j;
					if(Render.raytrace()){
						z1=zbuf1[k];
						//double rm=rmod;
						//rmod=0;
						double alpha=astep;
#ifdef OPTIMIZE
						double theta=alut[(int)(ascale*dpr)];
#else
						float theta=2*acos(dp);
#endif
						int dn=k;
						for(int l=0;l<y-i;l++){
							float beta=PI-theta-alpha;
#ifdef OPTIMIZE
							double t=tlut[(int)(bscale*(beta-bmin))%LUTSIZE];
							double zr=1-avals[l]*t;
							double zt=z1*zr;
#else
							float zt=z1*(cos(alpha)+sin(alpha)*cos(beta)/sin(beta));
#endif
							dn+=vport[2];
							z2=zbuf1[dn];
							if(z2>0 && z2<zt){
								//rmod=rm;
								break;
							}
							alpha+=astep;
						}
						n=dn;
					}
				}
				p=pixels+4*n;
				mix=rampstep(0.25*water_dpr,water_dpm,dp,water_mix*attn,0);
				cmod=(1-rmod)*saturation*mflag;
				mix*=mix;
				f3=rmod;
				f2=mix*cmod;
			    f1=1-mix;
				int s=(int)(f1*c[0]+f2*m[0]+f3*p[0]);
				c[0]=s>255?255:s;
				s=(int)(f1*c[1]+f2*m[1]+f3*p[1]);
				c[1]=s>255?255:s;
				s=(int)(f1*c[2]+f2*m[2]+f3*p[2]);
				c[2]=s>255?255:s;
				wCol++;
				wRefl++;
			}
			else if(mflag){   // pure color modulation
			    //f2=af*(1-dp)*water_mix*water_saturation;
			    f2=(1-dp)*water_mix*saturation;
			    f1=1-f2;
				c[0]=(GLubyte)(f1*c[0]+f2*m[0]);
				c[1]=(GLubyte)(f1*c[1]+f2*m[1]);
				c[2]=(GLubyte)(f1*c[2]+f2*m[2]);
				wCol++;
			}
			if(aflg){
				p=pixels+4*(k-1);
				c[0]=(GLubyte)(0.5*c[0]+0.5*p[0]);
				c[1]=(GLubyte)(0.5*c[1]+0.5*p[1]);
				c[2]=(GLubyte)(0.5*c[2]+0.5*p[2]);
			}
		}
	}
}

//-------------------------------------------------------------
// RasterMgr::setView()	capture view for WindowCoords
//-------------------------------------------------------------
void RasterMgr::setView()
{
	TheScene->getViewport(vport);
	vpmat[0]=vpmat[12]=(vport[2]/2.0);
	vpmat[5]=vpmat[13]=(vport[3]/2.0);

	TheScene->getMatrix(GL_PROJECTION, vproj);
	CMmmul(vpmat,vproj,vmat,4);
}

//-------------------------------------------------------------
// RasterMgr::manageBuffers()	allocate and free buffers
//-------------------------------------------------------------
void RasterMgr::manageBuffers()
{
	int n = TheScene->pixels();
	int i;

	TheScene->getViewport(vport);

	//printf("BGN RasterMgr::manageBuffers(%d)\n",n);

	set_idmode(0);

	int need_pixels=Adapt.overlap_test();
	if(!Render.draw_shaded() && !need_pixels)
		need_pixels=shadows()
	   || Render.show_water()
	   || (Render.fog() && fog())
	   || Render.dealias()
	   || filter_show()
	   ;

	if(!need_pixels || n>npixels){
		FREE(pixels);
		FREE(gdata);
	}

	int need_image=0;
	if(!Render.draw_shaded()){
		need_image=Render.show_water()
		||(shadows()&&Lights.size)
		||(Render.fog() && fog())
		||(Render.dealias() && filter_edges())
		|| filter_show()
		;
	}

    int ni=need_image?1:0;
    if(!Render.draw_shaded()){
		if(shadows()&&Lights.size){
			//ni=1+(Lights.size-1)/3;
			ni=Lights.size;
			if(Render.show_water()&&Lights.size==1)
				ni++;
		}
		else if(Render.show_water())
			ni++;
    }

 	if(n>npixels || ni<nibuffs){
 	    int nstart=n>npixels?0:ni;
		for(i=nstart;i<nibuffs;i++){
 	    	printf("free buffer %d 0x%X\n",i,ibuffs[i]);
			FREE(ibuffs[i]);
		}
	}
	int need_ids=Adapt.overlap_test();

	if(!Render.show_water()||!Render.raytrace()||colsize!=TheScene->current_ht){
		FREE(avals);
		FREE(tlut);
		FREE(alut);
	}
	colsize=TheScene->current_ht;
	if(!Render.draw_shaded() && Render.show_water()&&Render.raytrace()&&!avals){
		MALLOC(colsize,double,avals);
		MALLOC(LUTSIZE,double,tlut);
		MALLOC(LUTSIZE,double,alut);
		init_cols();
	}
	if(!need_ids){
		FREE(idtbl);
	}

	if(!idtbl && need_ids){
		int n=TheScene->cells;
		if(n<30000)
			n=30000;
		if(!idtbl||n>idsize)
			alloc_idtbl(n);
	}

	if(!pixels && need_pixels){
		MALLOC((4*n),GLubyte,pixels);
		MALLOC((4*n),GLfloat,gdata);
	}

	for(i=0;i<ni;i++){
	    if(!ibuffs[i]){
		    MALLOC((3*n),GLubyte,ibuffs[i]);
 	    	printf("malloc buffer %d 0x%X size=%d\n",i,ibuffs[i],n);
		}
	}
	nibuffs=ni;
	zvalid=0;
	int need_zbuf1=1;
	int need_zbuf2=0;
	int need_sflags=0;
	if(!Render.draw_shaded()){
		need_zbuf2=shadows()|| Render.show_water();
		need_sflags=shadows();
	}

	if(!need_zbuf1 || n>npixels)
		FREE(zbuf1);
	if(!need_zbuf2 || n>npixels)
		FREE(zbuf2);
	if(!need_sflags || n>npixels)
		FREE(sflags);

	if(!zbuf1 && need_zbuf1)
		MALLOC(n,GLfloat ,zbuf1);
	if(!zbuf2 && need_zbuf2)
		MALLOC(n,GLfloat ,zbuf2);
	if(!sflags && need_sflags)
		MALLOC(n,ShadowFlags ,sflags);
	npixels=n;
	zbuf_valid=0;
	pixels_valid=0;
	filter_valid=0;
 	//printf("END RasterMgr::manageBuffers(%d)\n",npixels);

}
//-------------------------------------------------------------
// RasterMgr::init_render()	collect enables for raster scans
//-------------------------------------------------------------
void RasterMgr::init_render()
{
	do_hdr=hdr();
	do_water=Render.show_water() && waterpass() && (TheScene->viewtype==SURFACE);
	do_depth=do_water && water_depth();
	do_reflect=do_water && (reflections() || water_modulation());
	do_shadows=Lights.size && shadows() && Render.lighting();
	do_bumps=Render.bumps() && bumptexs() && Render.lighting();
	do_vfog=Render.fog() && fogpass()&&(TheScene->viewtype==SURFACE);
	do_haze=Render.haze()&& hazepass()&& TheScene->inside_sky();
	do_edges=Render.dealias()||filter_show();
	do_fog=do_haze|do_vfog;
	do_image=do_water||do_fog||do_edges;
}
//-------------------------------------------------------------
// RasterMgr::render()	pixel image rasterization
//-------------------------------------------------------------
void RasterMgr::render()
{

	setView();

	if(do_water){
		bool tmp=Map::use_call_lists;
		Map::use_call_lists=false;
		set_all();
	    surface=1;
		set_render_type(AUXIMAGE);
		render_auximage();
		getAuxImage(0);
		getZbuf(1);  // terrain surface
		surface=2;
		render_auximage();
		getAuxImage(0);
		getZbuf(0); // water surface
		Lights.setSpecular(water_specular);
		Lights.setShininess(water_shine);

		// water specular highlights
		//   - color=black,alpha=1,lighting=enabled
		//   - add resulting image to water depth image
		//   - this will lighten water pixels with specular highlights
		glColor4d(0,0,0,1);
		set_render_type(0);
		TheScene->render_raster();
		getAuxImage(1); // specular color
		Map::use_call_lists=tmp;
		GLSLMgr::clrBuffers();
	}
	else if(do_image){
	    surface=1;
		set_render_type(AUXIMAGE);
		render_auximage();
		getAuxImage(0);
		getZbuf(0);
		GLSLMgr::clrBuffers();
	}
	set_render_type(0);
	surface=1;

}

//-------------------------------------------------------------
// RasterMgr::apply()	pixel image rasterization
//-------------------------------------------------------------
void RasterMgr::apply()
{
	if(!do_water && !do_shadows && !do_fog)
		return;
	getPixels();

	if(do_water || do_fog)
		prep_zbuf();
	if(do_water)
		applyWaterdepth();
	if(do_shadows)
		applyShadows();
	if(do_water&&do_fog){
		bool hz=do_haze;
//		if(do_vfog){
//			do_haze=0;
//			applyFog();
//		}
		applyReflections();
		do_haze=hz;
		applyFog();
	}
	else if(do_water)
		applyReflections();
	else if(do_fog)
		applyFog();

	drawPixels();

}

void RasterMgr::renderBgShadows(){
	if(Render.draw_shaded())
		return;
	render_shading();    // create dot-product image
	getZbuf(0);
	render_shadows();    // create shadow image
	GLSLMgr::clrBuffers();
}
void RasterMgr::applyBgShadows(){
	setView();
	getPixels();
	applyShadows();
	drawPixels();
}
void RasterMgr::renderFgShadows(){
	render_shading();
	getZbuf(0);
	glClear(GL_COLOR_BUFFER_BIT);
	if(farview())
	    set_accumulate(1);
	set_farview(0);
	render_shadows();
    set_accumulate(0);
	GLSLMgr::clrBuffers();
}
