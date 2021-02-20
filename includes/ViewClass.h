
//  ViewClass.h

#ifndef _VIEWCLASS
#define _VIEWCLASS

#include "LightClass.h"
#include "ObjectClass.h"
#include "Util.h"
#include "MatrixClass.h"
#include "TrackBall.h"
//#include "AdaptOptions.h"

#define NO_MOVE     0
#define MODEL_MOVE  1
#define LIGHT_MOVE  2
#define EYE_MOVE    3

#define MAX_STRINGS 	64
#define MAX_STRING_SIZE 64

#define COS45	0.70711f

extern int use_pixel_mask;
enum{
	SURFACE  	= 0x01,
	ORBITAL  	= 0x02,
	LOCAL   	= 0x04,
	GLOBAL      = 0x08,
	SPHERICAL   = SURFACE|ORBITAL|LOCAL,
	RECTANGULAR = GLOBAL,
	ORBITVIEW   = SURFACE|ORBITAL
};
enum{
	MOVE		= 0,
	VIEW		= 1
};

class Model;
class Matrix;
class MatrixMgr;


enum{ // status bits
	AUTOTM			= 0x00000001,
	AUTOMV			= 0x00000002,
	AUTOH			= 0x00000004,
	AUTOV			= 0x00000008
};
enum{ // color bits
	INFO_COLOR	    = 0x00000000,
	DATA_COLOR      = 0x00000001,
	HDR1_COLOR		= 0x00000002,
	HDR2_COLOR		= 0x00000003,
	RECD_COLOR	    = 0x00000004,
	PLAY_COLOR	    = 0x00000005,
	HIST_COLOR	    = 0x00000006,
	MENU_COLOR	    = 0x00000007,
	NUM_SYSCOLORS   = 10,
};

class View  // private status bits
{
enum{ // status bits
	AUTOFLGS		= AUTOTM|AUTOMV,
	AUTOVH		    = AUTOV|AUTOH,
	SELECT			= 0x00000010,
	UNSELECT		= 0x00000020,
	CLRPASS			= 0x00000040,
	PASSOBJS	    = 0x00000080,

	CHANGED_DETAIL  = 0x00000100,
	CHANGED_VIEW    = 0x00000200,
	CHANGED_TIME    = 0x00000400,
	CHANGED_RENDER  = 0x00000800,
	CHANGED_FILE	= 0x00001000,
	CHANGED_OBJECT	= 0x00002000,
	CHANGED_MOVIE	= 0x00004000,
	CHANGED_MARKER	= 0x00008000,
	CHANGED_MODEL   = CHANGED_DETAIL|CHANGED_VIEW,
	CHANGED_FLAGS   = 0x00007f00,

	BACKSIDE		= 0x00010000,
	EYEREF			= 0x00020000,
	AUTOLDM			= 0x00040000,
	SIDESTEP		= 0x00080000,
	VSTEP		    = 0x00100000,

	TESTVIEW		= 0x00200000,
	LIGHTVIEW		= 0x00400000,
	VIEWTYPES       = TESTVIEW|LIGHTVIEW,

	SHOW_ISTRING	= 0x00800000,
	SHOW_VSTRING	= 0x01000000
};
enum{	// vflags bits
	MOVED			= 0x00000001,
	UPDIR			= 0x00000002,
	BKDIR			= 0x00000004,
	CHANGED_POS     = 0x00000008
};

protected:
	int		status;
	int         suspend_cnt;

public:
	TrackBall model_trackball;
	TrackBall light_trackball;
	TrackBall eye_trackball;

    MatrixMgr *modelMgr;
    MatrixMgr *projMgr;
	char       strings[MAX_STRINGS*MAX_STRING_SIZE];
	int        stgcolor[MAX_STRINGS];
	int        nstrings;
	char       istring[MAX_STRING_SIZE];
	Color      syscolor[NUM_SYSCOLORS];
	bool       auto_contrast; // enables color inversion to optimize contrast;
	int window_height;
	int window_width;

	int move_type;

	double radius;
	double theta;
	double phi;
	double heading;
	double pitch;
	double view_angle;
	double view_tilt;
	double view_skew;
	double hstride;
	double vstride;
	double gstride;
	double speed;
	double view_step;
	ObjectNode   *vobj;

	int    bgpass;     // active bg pass

	Point eye;
	Point center;
	Point normal;
	Point vpoint;
	Point vcpoint;
	Point vnpoint;
	Point epoint;
	Point cpoint;
	Point npoint;
	Point gpoint;
	Point spoint;
	Point xpoint;
	Point xoffset;

 	int     viewport[4];

    GLdouble projMatrix[16];
    GLdouble lookMatrix[16];
    GLdouble viewMatrix[16];
    GLdouble invViewMatrix[16];

    Matrix  EyeMatrix;
    Matrix  InvEyeMatrix;
    Matrix  ViewMatrix;
    Matrix  InvViewMatrix;
    Matrix  InvModelMatrix;
    Matrix  ModelMatrix;
    Matrix  ModelViewProjMatrix;
    Matrix  InvModelViewProjMatrix;

	Bounds   *bounds;

	static double dflt_fov;
	static double dflt_cellsize;
	double	aspect;
	double  znear;
	double  zfar;
	double  fov;
	int	mode;
	int	vflags;
	int	viewtype;
	double	zoom;
	double  minr,maxr,minh;
	double  delh,delv;
	double  size;
	double	angle_change;
	double  wscale;
	double  height;   // view ht above surface
	double  elevation;// view ht above "sea level"
	double  gndlvl;   // ground ht above "sea level"
	int     color_bits;
	double  cellsize;	// cell size multiplier
	double  tesslevel;	// cell size multiplier
	double	resolution; // cell size multiplier
	int     detail_direction;
	double  time;		// current time
	double  ftime;		// frame time
	double  delt;		// time multiplier
	double  tfact;		// time multiplier
	double  maxht;		// local view max. height

	View();
	~View();

	virtual void spinTrackball(int type, int x, int y);

	virtual void reset_colors();
	void spinTrackball(int type, int x, int y, double ampl);
	void resetTrackball(int type);
	void setMouse(int x, int y);

	void setViewPort(int w, int h);

	void setMoveType(int m) { move_type=m;}
	int getMoveType()       { return move_type;}

	// view status functions

	void move_view();
	void calc_offset();
	void model_ref();

	void set_moved()			{ BIT_ON(vflags, MOVED);}
	void clr_moved()			{ BIT_OFF(vflags, MOVED);}
	void set_up()				{ BIT_ON(vflags, UPDIR);}
	void set_down()				{ BIT_OFF(vflags, UPDIR);}
	int	 moved()				{ return (vflags & MOVED);}
	int	 updir()				{ return (vflags & UPDIR);}
	int	 backdir()				{ return (vflags & BKDIR);}
	void set_forward()			{ BIT_OFF(vflags, BKDIR);}
	void set_backward()			{ BIT_ON(vflags, BKDIR);}

	void set_changed_position()	{ BIT_ON(vflags,CHANGED_POS);}
	void clr_changed_position()	{ BIT_OFF(vflags,CHANGED_POS);}
	int	 changed_position()		{ return vflags &CHANGED_POS;}


	void set_autov()	        { BIT_ON(status, AUTOV);}
	void clr_autov()	        { BIT_OFF(status, AUTOV);}
	void set_autoh()	        { BIT_ON(status, AUTOH);}
	void clr_autoh()	        { BIT_OFF(status, AUTOH);}
	int autov()                 { return status & AUTOV;}
	int autoh()                 { return status & AUTOH;}
	int autovh()                { return status & AUTOVH;}

	void set_autovh(int v)      { if(v) set_autovh(); else clr_autovh();}
	void set_autovh()           { BIT_ON(status, AUTOVH);}
	void clr_autovh()           { BIT_OFF(status, AUTOVH);}
	int  changed_model()	    { return moved() || (status & CHANGED_MODEL);}

	void set_changed_detail()	{ BIT_ON(status, CHANGED_DETAIL);}
	void clr_changed_detail()	{ BIT_OFF(status, CHANGED_DETAIL);}
	int	 changed_detail()		{ return (status & CHANGED_DETAIL);}

	void set_changed_time()		{ BIT_ON(status,CHANGED_TIME);}
	void clr_changed_time()		{ BIT_OFF(status,CHANGED_TIME);}
	int	 changed_time()			{ return (status & CHANGED_TIME);}

	void set_changed_render()	{ BIT_ON(status,CHANGED_RENDER);}
	void clr_changed_render()	{ BIT_OFF(status,CHANGED_RENDER);}
	int  changed_render()	    { return status & CHANGED_RENDER;}

	void set_backside()			{ BIT_ON(status,BACKSIDE);}
	void set_frontside()		{ BIT_OFF(status,BACKSIDE);}
	int frontside()				{ return status & BACKSIDE?0:1;}
	int backside()				{ return status & BACKSIDE?1:0;}

	void set_clrpass(int i)		{ BIT_SET(status,CLRPASS,i);}
	int clrpass()				{ return status & CLRPASS;}

	int  far_pass()			    { return bgpass==BGFAR ? 1:0;}
	int  raster_pass()			{ return bgpass==FG0 ? 1:0;}
	int  shadows_pass()			{ return bgpass==FGS ? 1:0;}
	int  bgshadows_pass()		{ return bgpass==BGS ? 1:0;}

	void set_passobjs()			{ BIT_ON(status,PASSOBJS);}
	void clr_passobjs()			{ BIT_OFF(status,PASSOBJS);}
	int  passobjs()				{ return (status&PASSOBJS)?1:0;}

	void set_eye_view()			{ BIT_OFF(status,TESTVIEW);
								  BIT_OFF(status,LIGHTVIEW);
					            }
	void set_test_view()		{ BIT_ON(status,TESTVIEW);
								  BIT_OFF(status,LIGHTVIEW);
					            }
	void set_light_view()		{ BIT_ON(status,LIGHTVIEW);
								  BIT_OFF(status,TESTVIEW);
								}
	int test_view()				{ return status &TESTVIEW ? 1:0;}
	int light_view()			{ return status &LIGHTVIEW ? 1:0;}
	int eye_view()			    { return status &VIEWTYPES ? 0:1;}

	int eyeref()				{ return status &EYEREF;}
	void set_eyeref()			{ BIT_ON(status,EYEREF);}
	void clr_eyeref()			{ BIT_OFF(status,EYEREF);}

	int sidestep()				{ return (status &SIDESTEP)?1:0;}
	void set_sidestep(int i)	{ BIT_SET(status,SIDESTEP,i);}
	int vstep()					{ return (status & VSTEP)?1:0;}
	void set_vstep(int i)		{ BIT_SET(status,VSTEP,i);}

	int autoldm()				{ return status &AUTOLDM;}
	void set_autoldm()			{ BIT_ON(status,AUTOLDM);}
	void clr_autoldm()			{ BIT_OFF(status,AUTOLDM);}

	void set_changed_marker()	{ BIT_ON(status,CHANGED_MARKER);}
	void clr_changed_marker()	{ BIT_OFF(status,CHANGED_MARKER);}
	int	 changed_marker()		{ return status &CHANGED_MARKER;}

	void set_changed_file()		{ BIT_ON(status,CHANGED_FILE);}
 	void clr_changed_file()		{ BIT_OFF(status,CHANGED_FILE);}
 	int  changed_file()			{ return status &CHANGED_FILE;}

	void set_changed_object()	{ BIT_ON(status,CHANGED_OBJECT);}
 	void clr_changed_object()	{ BIT_OFF(status,CHANGED_OBJECT);}
 	int  changed_object()		{ return status &CHANGED_OBJECT;}

	void set_changed_movie()	{ BIT_ON(status,CHANGED_MOVIE);}
 	void clr_changed_movie()	{ BIT_OFF(status,CHANGED_MOVIE);}
 	int  changed_movie()		{ return status &CHANGED_MOVIE;}

    void clr_changed_flags()    { BIT_OFF(status,CHANGED_FLAGS);}

	void set_changed_view()		{ BIT_ON(status,CHANGED_VIEW);}
 	void clr_changed_view()		{ BIT_OFF(status,CHANGED_VIEW);}
 	int  changed_view()			{ return status &CHANGED_VIEW;}
 	void set_autotm()			{ BIT_ON(status,AUTOTM);}
 	void clr_autotm()			{ BIT_OFF(status,AUTOTM);}
 	int  autotm()				{ return status & AUTOTM;}
 	void set_automv()			{ BIT_ON(status,AUTOMV);}
 	void clr_automv()			{ BIT_OFF(status,AUTOMV);}
	int automv()                { return status & AUTOMV;}
	int	 motion()				{ return status & AUTOFLGS;}

	void set_none()				{ BIT_OFF(status,SELECT|UNSELECT);}
	void set_all()				{ BIT_ON(status,SELECT|UNSELECT);}

	int  selected()				{ return status & SELECT;}
	int  unselected()			{ return status & UNSELECT;}
	void set_selected()			{ set_none();BIT_ON(status,SELECT);}
	void set_unselected()		{ set_none();BIT_ON(status,UNSELECT);}

	int  suspended()			{ return suspend_cnt>0?1:0;}
	void suspend()				{ suspend_cnt++;}
	void unsuspend()			{ suspend_cnt--;}

	int orbitview()			 	{ return viewtype & ORBITVIEW;}
	int cartesion()				{ return viewtype & RECTANGULAR;}
	int spherical()				{ return viewtype & SPHERICAL;}

	void scale(double,double);
	void reset();
	void reset(View*);

	void draw_string(int c, const char *msg,...);
	void output_strings();
	void set_istring(const char *msg,...);
	void set_show_istring(int c) { BIT_SET(status,SHOW_ISTRING,c);}
	int show_istring()           { return status & SHOW_ISTRING;}

	// viewpoint motion functions

	void auto_delv();
	void auto_delh();
	virtual void auto_stride();
	virtual void increase_hstride();
	virtual void decrease_hstride();
	virtual void increase_vstride();
	virtual void decrease_vstride();
	virtual void reset_stride();
	virtual void forward();
	virtual void backward();
	virtual void left();
	virtual void right();
	virtual void up();
	virtual void down();
	virtual void move_forward();
	virtual void move_backward();
	virtual void move_left();
	virtual void move_right();
	virtual void move_up();
	virtual void move_down();
	virtual void head_left();
	virtual void head_right();
	virtual void head_up();
	virtual void head_down();
	virtual void head_forward();
	virtual void look_forward();
	virtual void look_backward();
	virtual void look_left();
	virtual void look_right();
	virtual void look_up();
	virtual void look_down();
	virtual void scan_left();
	virtual void scan_right();
	void project();
	void set_view();
	void adjust_view();
	void setMatrix();
	void set_mode(int);
	void set_small_angle()	{	angle_change=10.0;}
	void set_rt_angle()		{	angle_change=90.0;}
	int  rt_angle()			{	return angle_change==90.0 ? 1:0;}
	int pixels()			{	return viewport[2]*viewport[3];}
	int swidth()			{	return viewport[3];}
	int sheight()			{	return viewport[2];}
	double extent()			{	return cellsize*resolution;}

	void fustrum(double, double, Point*);
	void perspective(double,double,double,double,Point&,Point&,Point&);

	virtual void change_view(int);
	virtual void change_view(int,double);
	virtual void change_view(int,Point);
	virtual void animate();

	void increase_detail();
	void decrease_detail();
	void reset_detail();
	void set_time();
	void set_time(double);
	void set_delt(double);
	void increase_time();
	void decrease_time();
	void reset_time();

	// gl encapsolation

	void perspective(GLdouble,GLdouble,GLdouble,GLdouble);
	void lookat(Point&,Point&,Point&);
    void fustrum(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
    void ortho(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
	void translate(Point &);
	void translate(GLdouble x,GLdouble y,GLdouble z);
	void rotate(GLdouble,GLdouble,GLdouble,GLdouble);
	void pushMatrix();
	void popMatrix();
	void getMatrix(GLint i, GLdouble *d);
	void getMatrix(GLdouble *d);
	void setMatrix(GLdouble *d);
	void multMatrix(GLdouble *d);
	void setMatrix(Matrix &d);
	void loadMatrix(int);
	void pickMatrix(int x,int y, int delta);
	void getViewport(GLint *);
	void loadIdentity();
	int getMatrixMode();
	void setMatrixMode(GLint);
	MatrixMgr &getMatrixMgr();
	void testMatrix(const char *s);
	Point unProject(double x,double y, double z);
	Point project(double x,double y, double z);
	Point unProject(Point p);
	Point project(Point q);
	Point unProjectN(Point p);
	Point projectN(Point q);
};

extern View *TheView;
#endif
