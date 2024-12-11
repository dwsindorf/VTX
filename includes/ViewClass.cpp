// ViewClass.cpp

#include "ViewClass.h"
#include "matrix.h"
#include <math.h>
#include "MatrixClass.h"
#include "RenderOptions.h"
#include "NoiseClass.h"
//#include "AdaptOptions.h"

#define MATRIX_MGR          // enable MatrixMgr
#define TFACT 0.1

//extern double font_height,font_width,view_font_height,view_font_width;
extern double font_height,view_font_height;

int print_strings=0;
int draw_strings=1;

#define USE_MGR_MATRIX      // use MatrixMgr for transformations (d.p.)
//#define USE_OGL_MATRIX    // use openGL for transformations

// NOTE: to use testMatrix define USE_OGL_MATRIX

static LongSym vtypes[]={
	{"RECTANGULAR",	RECTANGULAR},
	{"GLOBAL",	    GLOBAL},
	{"LOCAL",	    LOCAL},
	{"SURFACE",	    SURFACE},
	{"ORBITAL",	    ORBITAL}
};
NameList<LongSym*> Vtypes(vtypes,sizeof(vtypes)/sizeof(LongSym));

static DoubleSym units[]={
	{"hours",		1},
	{"days",		24},
	{"feet",		FEET},
	{"ft",	    	FEET},
	{"miles",	    MILES},
	{"ml",	   		MILES},
	{"mm",			MM},
	{"au",			AU},
	{"bm",			BM},
	{"ly",			LY}
};
NameList<DoubleSym*> Units(units,sizeof(units)/sizeof(DoubleSym));

double View::dflt_fov=35;
double View::dflt_cellsize=3.5;
Color  View::syscolor[NUM_SYSCOLORS];
int    View::stgcolor[MAX_STRINGS]={0};
int    View::nstrings=0;

View::View()
{
	mode=MOVE;
	reset();
	viewtype=GLOBAL;
	cellsize=dflt_cellsize;

	suspend_cnt=0;
#ifdef MATRIX_MGR
	modelMgr=new MatrixMgr(32);
	projMgr=new MatrixMgr(8);
#else
	modelMgr=0;
	projMgr=0;
#endif
	reset_colors();
}

View::~View()
{
    if(modelMgr)
		delete modelMgr;
	modelMgr=0;
    if(projMgr)
		delete projMgr;
	projMgr=0;
}

//void View::set_changed_detail()	{
//	BIT_ON(status, CHANGED_DETAIL);
//	Adapt.set_maxcycles(50);
//}

//-------------------------------------------------------------
// View::reset_colors() reset screen colors to defaults
//-------------------------------------------------------------
void View::reset_colors(){
	syscolor[INFO_COLOR]=Color(0.0,0.9,1);
	syscolor[DATA_COLOR]=Color(1,1,1);
	syscolor[HDR1_COLOR]=Color(1,0.7,0);
	syscolor[MENU_COLOR]=Color(1,1,0.0);
	syscolor[RECD_COLOR]=Color(1,0.2,0.2);
	syscolor[PLAY_COLOR]=Color(0,1,0.5);
	syscolor[HIST_COLOR]=Color(0,1,1);
	syscolor[VIEW_COLOR]=Color(1,0,0);

	set_show_istring(1);
	auto_contrast=false;

}
//-------------------------------------------------------------
// View::reset() reset view parameters to defaults (on open)
//-------------------------------------------------------------
void View::reset()
{
	vflags=MOVED;
	radius=height=theta=phi=heading=pitch=gndlvl=0.0;
	view_angle=view_tilt=view_skew=0.0;
	eye=Point(0,0,1);
	normal=Point(0,1,0);
	center=Point(0,0,1);
	astride=1.0;
	fov=dflt_fov;
	view_step=2.0;
	zoom=vstride=hstride=1;
	zfar=1000;
	znear=0.001;
	gstride=0.1*LY;
	speed=1;
	vpoint=Point(0,0,0);
	xpoint=Point(0,0,0);
	set_down();
	bounds=0;
	bgpass=0;
	status=0;
	tfact=1;
	detail_direction=1;
	time_direction=1;
	reset_time();
	maxht=0;
	nstrings=0;
	istring[0]=vstring[0]=0;
	vobj=0;
	move_type=MODEL_MOVE;
	set_autoldm();
	set_autovh();
}

//-------------------------------------------------------------
// View::reset() reset view parameters to saved values
//-------------------------------------------------------------
void View::reset(View *v)
{
	radius=v->radius;
	theta=v->theta;
	phi=v->phi;
	heading=v->heading;
	pitch=v->pitch;
	view_tilt=v->view_tilt;
	view_angle=v->view_angle;
	eye=v->eye;
	normal=v->normal;
	center=v->center;
	fov=v->fov;
	zoom=v->zoom;
	hstride=v->hstride;
	vstride=v->vstride;
	astride=v->astride;

	gpoint=v->gpoint;
	spoint=v->spoint;
	vpoint=v->vpoint;
	speed=v->speed;
	gstride=v->gstride;
	gndlvl=v->gndlvl;
	height=v->height;
	znear=v->znear;
	zfar=v->zfar;
	set_autovh();
}

//-------------------------------------------------------------
// View::set_mode() set pass mode
//-------------------------------------------------------------
void View::set_mode(int m)
{
	mode=m;
}

//-------------------------------------------------------------
// View::scale() change depth range
//-------------------------------------------------------------
void View::scale(double n, double f)
{
	znear=n;
	zfar=f;
}

//-------------------------------------------------------------
// View::draw_string() print a string
//-------------------------------------------------------------
void View::draw_string(int id, const char *msg,...)
{
	va_list             xp;
	int n=MAX_STRING_SIZE;

	va_start(xp, msg);
	char buff[512];
	vsprintf(buff,msg, xp);
	va_end(xp);
	strncpy(strings+nstrings*n,buff,n);
	stgcolor[nstrings]=id;
	if(nstrings<MAX_STRINGS)
		nstrings++;
}

//-------------------------------------------------------------
// View::set_istring() set view string
//-------------------------------------------------------------
void View::set_vstring(const char *msg,...)
{
    if(!msg){
    	vstring[0]=0;
    	return;
    }
	va_list             xp;
	va_start(xp, msg);

	int n=MAX_STRING_SIZE;
	char buff[512];
	vsprintf(buff,msg, xp);
	va_end(xp);
	strncpy(vstring,buff,n);
}

//-------------------------------------------------------------
// View::set_istring() set info string
//-------------------------------------------------------------
void View::set_istring(const char *msg,...)
{
    if(!msg){
    	istring[0]=0;
    	return;
    }
	va_list             xp;
	va_start(xp, msg);

	int n=MAX_STRING_SIZE;
	char buff[512];
	vsprintf(buff,msg, xp);
	va_end(xp);
	strncpy(istring,buff,n);
	//cout<<"View::set_istring "<<istring<<endl;
}

//-------------------------------------------------------------
// View::output_strings() draw cached strings
//-------------------------------------------------------------
void View::output_text()
{
	push_attributes();
	output_info_strings();
	output_select_string();
	output_view_string();
	pop_attributes();
}

//-------------------------------------------------------------
// View::push_attributes() push render attributes
//-------------------------------------------------------------
void View::push_attributes(){
	mm=getMatrixMode();
	pushMatrix();
    glLoadIdentity();
	setMatrixMode(GL_PROJECTION);
    loadIdentity();
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glRasterPos4d(-1,-1,0,1);
	glPushAttrib (GL_ALL_ATTRIB_BITS);
}
//-------------------------------------------------------------
// View::push_attributes() pop render attributes
//-------------------------------------------------------------
void View::pop_attributes(){
    glPopAttrib();
	glMatrixMode(mm);
	popMatrix();
	glRasterPos4d(-1,-1,0,1);
	glEnable(GL_DEPTH_TEST);
}

//-------------------------------------------------------------
// View::output_strings() draw cached strings
//-------------------------------------------------------------
void View::output_info_strings()
{
    if(nstrings==0)
    	return;

	glListBase(Font1);

	for (int i = 0; i < nstrings; i++) {
		char *str = strings + i * MAX_STRING_SIZE;
		FColor fg=syscolor[stgcolor[i]];
		int y = (int)(viewport[3] - i * font_height-1);
		glColor3d(fg.red(), fg.green(), fg.blue());
		draw_text(0, i, str);
		if (print_strings)
			printf("%s\n", str);
	}
}

//-------------------------------------------------------------
// View::output_strings() draw cached strings
//-------------------------------------------------------------
void View::output_select_string()
{
	//cout<<"View::output_select_string() "<<istring<<endl;

    if(istring[0]==0)
    	return;

	glListBase(Font2);

	if(print_strings)
		printf("%s\n",istring);
	double y=20.0/viewport[3];
	FColor fg=syscolor[INFO_COLOR];
	glColor3d(fg.red(),fg.green(),fg.blue());
	glRasterPos2d(-1,-1+y);
	glCallLists(strlen(istring),GL_UNSIGNED_BYTE,istring);
}

//-------------------------------------------------------------
// View::output_strings() draw cached strings
//-------------------------------------------------------------
void View::output_view_string()
{
    if(vstring[0]==0)
    	return;

	glListBase(Font3);

	if(print_strings)
		printf("%s\n",vstring);
	FColor fg=syscolor[VIEW_COLOR];
	glColor3d(fg.red(),fg.green(),fg.blue());
	glRasterPos2d(0.9,0.9);
	glCallLists(strlen(vstring),GL_UNSIGNED_BYTE,vstring);

}

//-------------------------------------------------------------
// View::set_view() set projection matrix
//-------------------------------------------------------------
void View::set_view()
{
	GLdouble M[16];
	getMatrix(GL_MODELVIEW, M);
	eye=Point(0,0,0).mm(M);
	double a=zfar;

	if(cartesion()){
		center=Point(a,0,0).mm(M);
		normal=Point(0,a,0).mm(M);
	}
	else{
		center=Point(0,0,a).mm(M);
		normal=Point(0,a,0).mm(M);
	}

	normal=normal-eye; // lookat fix ?

	setMatrixMode(GL_PROJECTION);
    loadIdentity();
	lookat(eye,center,normal);
	getMatrix(GL_PROJECTION,lookMatrix);
	setMatrixMode(GL_PROJECTION);
    loadIdentity();
	setMatrixMode(GL_MODELVIEW);
    loadIdentity();

    project();
}

//-------------------------------------------------------------
// View::setMatrix() capture current Ogl model matrix
//-------------------------------------------------------------
void View::setMatrix()
{
	getMatrix(viewMatrix);

	Matrix A(viewMatrix);
	Matrix B=A*InvEyeMatrix;

	if(!far_pass()){
		setMatrix(B.values());
		getMatrix(viewMatrix);
	}
	minv(viewMatrix,invViewMatrix,4);		// global-to-local
	xpoint=eye.mm(invViewMatrix)-xoffset.mm(invViewMatrix);
	vpoint=(eye-xoffset).mm(invViewMatrix);
	vcpoint=center.mm(invViewMatrix);
	vnpoint=normal.mm(invViewMatrix);
	if(far_pass() && spherical())
	    vpoint=gpoint.mm(invViewMatrix);

	// lost track of what all the above matrixs are forModelViewProjMatrix
	// the following code captures the active ogl model-proj matrix
	// (point->screen) and it's inverse (screen->point) just prior
	// to adapt and render calls for all objects

	GLdouble p[16];
	GLdouble m[16];
	GLdouble pm[16];
	glGetIntegerv(GL_VIEWPORT,viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX,m);
	glGetDoublev(GL_PROJECTION_MATRIX,p);

	CMmmul(p,m,pm,4);
	ModelViewProjMatrix.setValues(pm);
	minv(pm,pm,4);
	InvModelViewProjMatrix.setValues(pm);

}

//-------------------------------------------------------------
// View::model_ref() set view origin to model or global 0,0,0
//-------------------------------------------------------------
void View::model_ref()
{
	if(cartesion()){
		translate(spoint.x,spoint.y,spoint.z);
		rotate(heading,0,1,0);		// rotate about x
		rotate(pitch,0,0,1);		// rotate about z
		rotate(view_angle,0,1,0);	// orient left-right view direction
		rotate(view_tilt,1,0,0);	// orient up-down view direction
		/*
		rotate(view_angle,0,1,0);	// orient left-right view direction
		rotate(pitch+view_tilt,1,0,0);	// orient up-down view direction
		*/
		rotate(view_skew,0,0,1);	// orient skew direction
	}
	else{
		rotate(90,0,0,1);			// set equator view
		rotate(theta,1,0,0);		// rotate along lat.
		rotate(phi,0,0,-1);			// rotate along long.
		if(far_pass())
			translate(0,radius,0);		// move to viewpoint
		rotate(P360(view_angle+heading),0,1,0);// orient left-right view direction
		rotate(pitch+view_tilt,-1,0,0);	// orient up-down view direction
		rotate(view_skew,0,0,1);	// orient skew direction
	}
	getMatrix(EyeMatrix.values());
	InvEyeMatrix=EyeMatrix.invert();
	if(!far_pass()){
		setMatrixMode(GL_MODELVIEW);	// load identity
		loadIdentity();
	}
	getMatrix(GL_MODELVIEW, viewMatrix);
	epoint=Point(0,0,0).mm(viewMatrix);
	cpoint=Point(0,0,1).mm(viewMatrix);
	npoint=Point(0,1,0).mm(viewMatrix);
	//testMatrix("model_ref");

	if(far_pass() /*&& spherical()*/)
	    gpoint=epoint;
}

//-------------------------------------------------------------
// View::calc_offset() calculate view origin to eye position
//-------------------------------------------------------------
void View::calc_offset()
{
	setMatrixMode(GL_MODELVIEW);	// load identity
    loadIdentity();
	clr_eyeref();

	xoffset=Point(0,0,0);
	xpoint=Point(0,0,0);

	if(cartesion() || far_pass())
	    return;

	Point p1,p2,p3;

	// execute eye transformations with translation to radius

	rotate(90,0,0,1);			// set equator view
	rotate(theta,1,0,0);		// rotate along lat.
	rotate(phi,0,0,-1);			// rotate along long.
	translate(0,radius,0);		// move to viewpoint
	rotate(P360(view_angle+heading),0,1,0);// orient left-right view direction
	rotate(pitch+view_tilt,-1,0,0);	// orient up-down view direction
	rotate(view_skew,0,0,1);	// orient skew direction

	getMatrix(GL_MODELVIEW, viewMatrix);
	p2=Point(0,0,0).mm(viewMatrix);
	spoint=p2;

    loadIdentity();

    // execute eye transformations without translation to radius

	rotate(90,0,0,1);			// set equator view
	rotate(theta,1,0,0);		// rotate along lat.
	rotate(phi,0,0,-1);			// rotate along long.
	rotate(P360(view_angle+heading),0,1,0);// orient left-right view direction
	rotate(pitch+view_tilt,-1,0,0);	// orient up-down view direction
	rotate(view_skew,0,0,1);	// orient skew view direction

	getMatrix(GL_MODELVIEW, viewMatrix);
	p1=Point(0,0,0).mm(viewMatrix);

	Point plocal=p1-p2;  // local

    Matrix A;
	getMatrix(A.values());
	Matrix B=A.invert();
    xoffset=plocal.mm(B.values());
	set_eyeref();
    loadIdentity();
}

//-------------------------------------------------------------
// View::fustrum() make a fustrum using bounds box
//-------------------------------------------------------------
void View::fustrum(double zn, double zf, Point *B)
{
	Point p0,p1,p2;
	Point vz,vy,vx;
	double f,x,y;

	vy=npoint.normalize();
	vz=(cpoint-epoint).normalize();
	vx=vz.cross(vy).normalize();

	f=tan(RPD*fov/2);
	y=0.5*zn*f;
	x=y*aspect;
	p0=vpoint+vz*zn;

	B[0]=p0+(vy*y)-(vx*x);
	B[3]=p0+(vy*y)+(vx*x);
	B[4]=p0-(vy*y)-(vx*x);
	B[7]=p0-(vy*y)+(vx*x);

	p1=vpoint+vz*zf;
	y=0.5*zf*f;
	x=y*aspect;

	B[1]=p1+(vy*y)-(vx*x);
	B[2]=p1+(vy*y)+(vx*x);
	B[5]=p1-(vy*y)-(vx*x);
	B[6]=p1-(vy*y)+(vx*x);
}

//=================== OpenGL encapsolation ====================

void View::getViewport(GLint *v)
{
	glGetIntegerv(GL_VIEWPORT,v);
}

//-------------------------------------------------------------
// View::getMatrixMode() return ogl matrix mode
//-------------------------------------------------------------
int View::getMatrixMode()
{
	GLint  mmode;
	glGetIntegerv(GL_MATRIX_MODE,&mmode);
	return mmode;
}

//-------------------------------------------------------------
// View::getMatrixMgr() return ogl matrix mode
//-------------------------------------------------------------
MatrixMgr &View::getMatrixMgr()
{
	if(getMatrixMode()==GL_MODELVIEW)
	    return *modelMgr;
	return *projMgr;
}

//-------------------------------------------------------------
// View::setMatrixMode() set ogl matrix mode
//-------------------------------------------------------------
void View::setMatrixMode(GLint m)
{
	glMatrixMode(m);
}

//-------------------------------------------------------------
// View::loadIdentity() set current matrix to identity
//-------------------------------------------------------------
void View::loadIdentity()
{
#ifdef MATRIX_MGR
    getMatrixMgr().getMatrix().loadIdentity();
#endif
	if(autoldm())
		glLoadIdentity();
}

//-------------------------------------------------------------
// View::pushMatrix() push matrix
//-------------------------------------------------------------
void View::pushMatrix()
{
#ifdef MATRIX_MGR
	getMatrixMgr().pushMatrix();
#endif
#ifdef USE_OGL_MATRIX
	glPushMatrix();
#endif
}

//-------------------------------------------------------------
// View::popMatrix() pop  matrix
//-------------------------------------------------------------
void View::popMatrix()
{
#ifdef MATRIX_MGR
	getMatrixMgr().popMatrix();
#ifdef USE_MATRIX_MGR
	glLoadMatrixd(getMatrixMgr().getMatrix().values());
#endif
#endif
#ifdef USE_OGL_MATRIX
	glPopMatrix();
#endif
}

//-------------------------------------------------------------
// View::getMatrix() get ogl matrix
//-------------------------------------------------------------
void View::getMatrix(GLdouble *m)
{
	GLint  mmode;
	glGetIntegerv(GL_MATRIX_MODE,&mmode);
	getMatrix(mmode,m);
}

//-------------------------------------------------------------
// View::getMatrix() get ogl matrix
//-------------------------------------------------------------
void View::getMatrix(GLint n, GLdouble *m)
{
#ifdef USE_MGR_MATRIX
	if(n==GL_MODELVIEW)
	    modelMgr->getMatrix(m);
	else
	    projMgr->getMatrix(m);
#else
	if(n==GL_MODELVIEW)
	    glGetDoublev(GL_MODELVIEW_MATRIX,m);
	else
	    glGetDoublev(GL_PROJECTION_MATRIX,m);
#endif
}

//-------------------------------------------------------------
// View::testMatrix() test matrix mode
//-------------------------------------------------------------
void View::testMatrix(const char *ttl)
{
#ifdef MATRIX_MGR
#ifdef USE_OGL_MATRIX
	Matrix A;
	Matrix B;
	const char *s;

	if(getMatrixMode()==GL_MODELVIEW){
	    glGetDoublev(GL_MODELVIEW_MATRIX,A.values());
	    B=modelMgr->getMatrix();
	    s="model";
	}
	else{
	    glGetDoublev(GL_PROJECTION_MATRIX,A.values());
	    B=projMgr->getMatrix();
	    s="proj";
	}
	//draw_string("%s %s %g",ttl,s,A.difference(B));
	//printf("%s %s %g\n",ttl,s,A.difference(B));
	//A.print();
	//B.print();
#endif
#endif
}

//-------------------------------------------------------------
// View::setMatrix() set ogl matrix
//-------------------------------------------------------------
void View::setMatrix(GLdouble *m)
{
#ifdef MATRIX_MGR
    Matrix A(m);
    getMatrixMgr().setMatrix(A);
#ifdef USE_MGR_MATRIX
    m=A.values();
#endif
#endif
	glLoadMatrixd(m);
}


//-------------------------------------------------------------
// View::setMatrix() set ogl matrix
//-------------------------------------------------------------
void View::setMatrix(Matrix &d)
{
#ifdef MATRIX_MGR
    getMatrixMgr().setMatrix(d);
#ifdef USE_MGR_MATRIX
	loadMatrix(autoldm());
#endif
#endif
}

//-------------------------------------------------------------
// View::pickMatrix() load selection matrix
//-------------------------------------------------------------
void View::pickMatrix(int x,int y, int delta)
{
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(x, viewport[3]-y, delta,delta, viewport);
    Matrix M;
    if(getMatrixMode()==GL_MODELVIEW)
	    glGetDoublev(GL_MODELVIEW_MATRIX,M.values());
	else
	    glGetDoublev(GL_PROJECTION_MATRIX,M.values());

    glPopMatrix();
	Matrix R;
	R=M*getMatrixMgr().getMatrix();
	setMatrix(R);
}

//-------------------------------------------------------------
// View::unProject() convert screen point to scene point
//-------------------------------------------------------------
Point View::unProject(double x,double y, double z)
{
    Point v;
    v.x=2*x/viewport[2]-1;
    v.y=2*y/viewport[3]-1;
    v.z=1-z;
    v.x*=v.z;
    v.y*=v.z;
    v.z=-v.z;
    return v;
}

//-------------------------------------------------------------
// View::unProject() convert screen point to scene point
//-------------------------------------------------------------
Point View::unProject(Point p)
{
	GLdouble v[4]={2*p.x/viewport[2]-1,2*p.y/viewport[3]-1,2*p.z-1,1};
	GLdouble w[4];
	mvmul4(v,InvModelViewProjMatrix.values(),w);
	double wz=1/w[3];
	return Point(w[0]*wz,w[1]*wz,w[2]*wz);
}

//-------------------------------------------------------------
// View::project() convert scene point to screen point
//-------------------------------------------------------------
Point View::project(Point q)
{
    GLdouble qv[4]={q.x,q.y,q.z,1};
	GLdouble w[4];
	mvmul4(qv,ModelViewProjMatrix.values(),w);
	double wz=1/w[3];
    return Point(0.5*viewport[2]*(w[0]*wz+1),0.5*viewport[3]*(w[1]*wz+1),0.5*(w[2]*wz+1));
}

//-------------------------------------------------------------
// View::unProjectN() convert screen point to normalized scene point
//-------------------------------------------------------------
Point View::unProjectN(Point p)
{
	GLdouble v[4]={p.x,p.y,p.z,1};
	GLdouble w[4];
	mvmul4(v,InvModelViewProjMatrix.values(),w);
	double wz=1/w[3];
	return Point(w[0]*wz,w[1]*wz,w[2]*wz);
}

//-------------------------------------------------------------
// View::projectN() convert scene point to normalized screen point
//-------------------------------------------------------------
Point View::projectN(Point q)
{
    GLdouble qv[4]={q.x,q.y,q.z,1};
	GLdouble w[4];
	mvmul4(qv,ModelViewProjMatrix.values(),w);
	double wz=1/w[3];
    return Point(w[0]*wz,w[1]*wz,w[2]*wz);
}

//-------------------------------------------------------------
// View::project() convert scene point to screen point
//-------------------------------------------------------------
Point View::project(double x,double y, double z)
{
    Point v;
    Point p(x,y,z);
    v.z=-p.mz(lookMatrix);
    v.x=1+p.mx(projMatrix)/v.z;
    v.y=1+p.my(projMatrix)/v.z;
    v.x=0.5*v.x*viewport[2];
    v.y=0.5*v.y*viewport[3];
    return v;
}

//-------------------------------------------------------------
// View::loadMatrix() load ogl matrix
//-------------------------------------------------------------
void View::loadMatrix(int flag)
{
#ifdef MATRIX_MGR
#ifdef USE_MGR_MATRIX
    if(flag)
		glLoadMatrixd(getMatrixMgr().getMatrix().values());
#endif
#endif
}

//-------------------------------------------------------------
// View::rotate() rotate view
//-------------------------------------------------------------
void View::rotate(GLdouble angle,GLdouble x, GLdouble y, GLdouble z)
{
#ifdef MATRIX_MGR
    Matrix R;
    Matrix M;
	double c=cos(RPD*angle);
	double cf=1-c;
	double mag=sqrt(x*x+y*y+z*z);
	x=x/mag;
	y=y/mag;
	z=z/mag;
	double s=sin(RPD*angle);

	M[0]=x*x*cf+c;    M[4]=x*y*cf-z*s; M[8]=x*z*cf+y*s;   M[12]=0;
	M[1]=x*y*cf+z*s;  M[5]=y*y*cf+c;   M[9]=y*z*cf-x*s;   M[13]=0;
	M[2]=x*z*cf-y*s;  M[6]=y*z*cf+x*s; M[10]=z*z*cf+c;    M[14]=0;
	M[3]=0;           M[7]=0;          M[11]=0;           M[15]=1;

	R=M*getMatrixMgr().getMatrix();
	setMatrix(R);
#endif
#ifdef USE_OGL_MATRIX
	glRotated(angle,x,y,z);
#endif
}

//-------------------------------------------------------------
// View::translate() translate view
//-------------------------------------------------------------
void View::translate(GLdouble x, GLdouble y, GLdouble z)
{
#ifdef MATRIX_MGR
    Matrix R;
    Matrix M;
	M[0]=1;  M[4]=0;  M[8]=0;   M[12]=x;
	M[1]=0;  M[5]=1;  M[9]=0;   M[13]=y;
	M[2]=0;  M[6]=0;  M[10]=1;  M[14]=z;
	M[3]=0;  M[7]=0;  M[11]=0;  M[15]=1;

	R=M*getMatrixMgr().getMatrix();
	setMatrix(R);
#endif
#ifdef USE_OGL_MATRIX
	glTranslated(x,y,z);
#endif
}

//-------------------------------------------------------------
// View::multMatrix() multiply ogl matrix
//-------------------------------------------------------------
void View::multMatrix(GLdouble *m)
{
#ifdef MATRIX_MGR
    Matrix A(m);
    Matrix R=A*getMatrixMgr().getMatrix();
#endif
#ifdef USE_MGR_MATRIX
    setMatrix(R);
#else
    glMultMatrixd(m);
#endif
}

//-------------------------------------------------------------
// View::translate() translate view
//-------------------------------------------------------------
void View::translate(Point &p)
{
	translate(p.x,p.y,p.z);
}

//-------------------------------------------------------------
// View::project() set projection (always perspective for now)
//-------------------------------------------------------------
void View::project()
{
	perspective(fov, aspect, znear, zfar, eye, center, normal);
}

//-------------------------------------------------------------
// View::perspective() set perspective projection matrix
//-------------------------------------------------------------
void View::perspective(GLdouble f, GLdouble a, GLdouble zn, GLdouble zf,
                       Point &e,Point &c, Point &n)
{
	setMatrixMode(GL_PROJECTION);
    loadIdentity();
	perspective(f, a, zn, zf);
	lookat(e,c,n);
	getMatrix(GL_PROJECTION, projMatrix);
	loadMatrix(1);
	setMatrixMode(GL_MODELVIEW);
    loadIdentity();
}

//-------------------------------------------------------------
// View::perspective() set perspective projection matrix
//-------------------------------------------------------------
void View::perspective(GLdouble fv, GLdouble a, GLdouble zn, GLdouble zf)
{
#ifdef MATRIX_MGR
	Matrix M;
    Matrix R;
	double f=1/tan(RPD*fv/2);
	double zm=zn-zf;
	double zp=zn+zf;
	double zt=2*zn*zf;

	M[0]=f/a;  M[4]=0;  M[8]=0;      M[12]=0;
	M[1]=0;    M[5]=f;  M[9]=0;      M[13]=0;
	M[2]=0;    M[6]=0;  M[10]=zp/zm; M[14]=zt/zm;
	M[3]=0;    M[7]=0;  M[11]=-1;    M[15]=0;

	R=M*getMatrixMgr().getMatrix();
	setMatrix(R);
	//glDepthRange(zn, zf);
#endif
#ifdef USE_OGL_MATRIX
	gluPerspective(fv, a, zn, zf);
#endif
}

//-------------------------------------------------------------
// View::fustrum() set perspective projection matrix (untested)
//-------------------------------------------------------------
void View::fustrum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                   GLdouble zn, GLdouble zf)
{
#ifdef MATRIX_MGR
	Matrix M;
    Matrix R;
    double A=(right+left)/(top+bottom);
    double B=zn+zf;
    double C=2*zn*zf;
    double D=-zf-zn;
    double E=2*zn/(right-left);
    double F=2*zn/(top-bottom);

	M[0]=E;  M[4]=0;  M[8]=A;    M[12]=0;
	M[1]=0;  M[5]=F;  M[9]=B;    M[13]=0;
	M[2]=0;  M[6]=0;  M[10]=C;   M[14]=D;
	M[3]=0;  M[7]=0;  M[11]=-1;  M[15]=0;

	R=M*getMatrixMgr().getMatrix();
	setMatrix(R);

#endif
#ifdef USE_OGL_MATRIX
	glFrustum(left, right, bottom, top, zn, zf);
#endif
}

//-------------------------------------------------------------
// View::ortho() set orthoganal projection matrix	(untested)
//-------------------------------------------------------------
void View::ortho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                   GLdouble zn, GLdouble zf)
{
#ifdef MATRIX_MGR
	Matrix M;
    Matrix R;
    double tx=(right+left)/(-top+bottom);
    double ty=-zf+zf;
    double tz=-zf-zn;
    double A=2/(right-left);
    double B=2/(top-bottom);
    double C=-2/(zf-zn);

	M[0]=A;  M[4]=0;  M[8]=0;    M[12]=tx;
	M[1]=0;  M[5]=B;  M[9]=0;    M[13]=ty;
	M[2]=0;  M[6]=0;  M[10]=C;   M[14]=tz;
	M[3]=0;  M[7]=0;  M[11]=0;   M[15]=1;

	R=M*getMatrixMgr().getMatrix();
	setMatrix(R);

#endif
#ifdef USE_OGL_MATRIX
	glOrtho(left, right, bottom, top, zn, zf);
#endif
}

//-------------------------------------------------------------
// View::lookat() set viewpoint based on eye,center and normal
//-------------------------------------------------------------
void View::lookat(Point &e,Point &c, Point &n)
{
#ifdef MATRIX_MGR
	Matrix M;
	Matrix E;
	Matrix A;
	Matrix B;

    Point f=(c-e).normalize();
    Point up=n.normalize();

	Point s=f.cross(up);
	s=s.normalize();
	Point u=s.cross(f);
	u=u.normalize();

	M[0]=s.x;  M[4]=s.y;  M[8]=s.z;   M[12]=0;
	M[1]=u.x;  M[5]=u.y;  M[9]=u.z;   M[13]=0;
	M[2]=-f.x; M[6]=-f.y; M[10]=-f.z; M[14]=0;
	M[3]=0;    M[7]=0;    M[11]=0;    M[15]=1;

	E[0]=1;  E[4]=0;  E[8]=0;   E[12]=-e.x;
	E[1]=0;  E[5]=1;  E[9]=0;   E[13]=-e.y;
	E[2]=0;  E[6]=0;  E[10]=1;  E[14]=-e.z;
	E[3]=0;  E[7]=0;  E[11]=0;  E[15]=1;

	getMatrixMgr().getMatrix(B.values());

	A=M*B;
	B=E*A;
	setMatrix(B);
#endif
#ifdef USE_OGL_MATRIX
	gluLookAt(e.x,e.y,e.z,c.x,c.y,c.z,n.x,n.y,n.z);
#endif
}
// =======  movement and eye position functions =================

void View::forward()
{
	switch(mode){
	case MOVE:	move_forward();		break;
	case VIEW:	look_forward();		break;
	}
}

void View::backward()
{
	switch(mode){
	case MOVE:	move_backward();	break;
	case VIEW:	look_backward();	break;
	}
}

void View::up()
{
	set_up();
	auto_stride();
	switch(mode){
	case MOVE:
		//if(vstep()||!automv())
		if(vstep())
			move_up();
	    else
	        head_up();
		break;
	case VIEW:
		look_up();
		break;
	}
	auto_stride();
}

void View::down()
{
	set_down();
	auto_stride();
	switch(mode){
	case MOVE:
		//if(sidestep()||!automv())
		if(vstep())
		//if(sidestep())
			move_down();
	    else
			head_down();
		break;
	case VIEW:
		look_down();
		break;
	}
}

void View::left()
{
	switch(mode){
	case MOVE:
	    if(!automv()&&sidestep() && astride!=90)
			move_left();
		else
		    head_left();
		break;
	case VIEW:
		look_left();
		break;
	}
}

void View::right()
{
	switch(mode){
	case MOVE:
	    if(!automv()&&sidestep() && astride!=90)
		    move_right();
		else
		    head_right();
		break;
	case VIEW:
		look_right();
		break;
	}
}

void View::scan_left()
{
	view_skew=P360(view_skew-astride);
	set_moved();
}

void View::scan_right()
{
	view_skew=P360(view_skew+astride);
	set_moved();
}

void View::look_left()
{
	view_angle=P360(view_angle+astride);
	set_moved();
}

void View::look_right()
{
	view_angle=P360(view_angle-astride);
	set_moved();
}

void View::look_forward()
{
	view_tilt=view_angle=0;
	set_moved();
}

void View::look_backward()
{
	view_tilt=0;
	view_angle=180;
	set_moved();
}

void View::look_up()
{
	view_tilt=P360(view_tilt+astride);
	set_moved();
}

void View::look_down()
{
	view_tilt=P360(view_tilt-astride);
	set_moved();
}
void View::adjust_view()
{
	if(radius==0)
		radius=1e-8;
	else if (radius <0){
		radius=-radius;
		pitch=-pitch;
		theta=theta+180;
		phi=-phi;
		heading=-heading;
	}
	heading=P360(heading);
	theta=P360(theta);
	phi=P180(phi);
	pitch=P180(pitch);
}

void View::move_up()
{
	if(cartesion()){
	    head_up();
    } else {
		auto_stride();
		if(radius==0)
			radius=1e-6;
		double d=-sin(RPD*pitch)*delh;
		theta+=d*cos(RPD*heading);
		phi+=d*sin(RPD*heading);
		radius+=delv*cos(RPD*pitch);
		//cout<<radius<<" "<<delv<<endl;
		adjust_view();
	}
	set_moved();
	set_changed_marker();
}

void View::move_down()
{
	if(cartesion()){
	    head_down();
    } else {
		auto_stride();
		if(radius==0)
			radius=1e-6;
		double d=-sin(RPD*pitch)*delh;
		theta-=d*cos(RPD*heading);
		phi-=d*sin(RPD*heading);
		radius-=0.5*delv*cos(RPD*pitch);

		adjust_view();
	}
	set_moved();
	set_changed_marker();
}
void View::move_backward()
{
	if(cartesion()){
		spoint=spoint-Point(P360(heading+180),pitch,gstride).rectangular();
    } else {
		if(radius==0)
			radius=1e-6;
		double d=cos(RPD*pitch)*delh;
		theta-=d*cos(RPD*heading);
		phi-=d*sin(RPD*heading);
		auto_stride();
		radius-=delv*sin(RPD*pitch);
		adjust_view();
		auto_stride();
	}
	set_backward();
	set_changed_marker();
	set_moved();
	set_changed_position();
}
void View::move_forward()
{
	if(cartesion()){
		spoint=spoint+Point(P360(heading+180),pitch,gstride).rectangular();
    } else {
		if(radius==0)
			radius=1e-6;
		auto_stride();
		double d=cos(RPD*pitch)*delh;
		theta+=d*cos(RPD*heading);
		phi+=d*sin(RPD*heading);
		radius+=delv*sin(RPD*pitch);
		adjust_view();
		if(view_skew>=astride)
		    view_skew-=astride;
		else if(view_skew<=-astride)
		    view_skew+=astride;
		auto_stride();
	}
	set_forward();
	set_changed_position();
	set_changed_marker();
	set_moved();
}

void View::move_right()
{
	if(cartesion()){
		head_right();
    } else {
		auto_stride();
		double h=P360(heading-90);
		double d=delh;
		theta+=d*cos(RPD*h);
		//theta=P360(theta);
		phi+=d*sin(RPD*h);
		//phi=P180(phi);
		adjust_view();

	}
	set_forward();
	set_changed_position();
	set_changed_marker();
	set_moved();
}

void View::move_left()
{
	if(cartesion()){
		head_left();
    } else {
		auto_stride();
		double h=P360(heading+90);
		double d=delh;
		theta+=d*cos(RPD*h);
		phi+=d*sin(RPD*h);
		adjust_view();
	}
	set_forward();
	set_changed_position();
	set_changed_marker();
	set_moved();
}

void View::head_left()
{
	heading+=astride;
	heading=P360(heading);
	if(automv() && !cartesion())
		view_skew+=2*astride;
	view_skew=P360(view_skew);
	auto_stride();
	set_changed_position();
	set_changed_marker();
	set_moved();
}

void View::head_right()
{
	heading-=astride;
	heading=P360(heading);
	if(automv() && !cartesion())
	    view_skew-=2*astride;
	view_skew=P360(view_skew);
	auto_stride();
	set_changed_position();
	set_changed_marker();
	set_moved();
}

void View::head_up()
{
	pitch+=astride;
	set_moved();
	set_changed_marker();
	set_up();
}

void View::head_down()
{
	pitch-=astride;
	set_moved();
	set_changed_marker();
	set_down();
}

void View::head_forward()
{
	if(backdir())
		move_backward();
	else
	    move_forward();
}

void View::auto_delv() {
	if (minr) {
		//cout << "r/m:"<<radius/minr<< " exp:"<< (1-exp(-(radius-minr)/minr))/FEET << endl;
		delv = zoom * vstride * ((1.0 - exp(-(radius - minr) / minr))) + minh;
		//cout<<" vstride:"<<vstride<<" radius:"<<radius<<" minr:"<<minr<<" minh:"<<minh<<" zoom:"<<zoom<<endl;
		if (radius < minr) {
			radius = minr;
			delv = 0;
		}
	} else
		delv = zoom * vstride;
}

void View::auto_delh()
{
	if(minr && autoh())
		delh=zoom*hstride*(1.0-exp(-(radius-minr)/minr)+1000*minh)/minr;
	//cout<<" hstride:"<<hstride<<" radius:"<<radius<<" minr:"<<minr<<" minh:"<<minh<<" zoom:"<<zoom<<endl;
}

void View::reset_stride()
{
    hstride=2.0;
    vstride=0.02;
    gstride=0.1*LY;
    auto_delv();
    auto_delh();
}

void View::auto_stride()
{
	auto_delv();
	auto_delh();
}

void View::increase_hstride()
{
	gstride*=2;
	hstride*=2;
	auto_delh();
}
void View::decrease_hstride()
{
	gstride*=0.5;
	hstride*=0.5;
	auto_delh();
}

void View::increase_vstride()
{
	gstride*=2;
	vstride*=2;
	auto_delv();
}
void View::decrease_vstride()
{
	gstride*=0.5;
	vstride*=0.5;
	auto_delv();
}

//-------------------------------------------------------------
// View::increase_detail() increase detail level
//-------------------------------------------------------------
void View::increase_detail()
{
	cellsize*=0.75;
	if(resolution*cellsize<=1.0){
		cellsize=1.0/resolution;
		return;
	}
	set_changed_detail();
	detail_direction=1;
}

//-------------------------------------------------------------
// View::decrease_detail() decrease detail level
//-------------------------------------------------------------
void View::decrease_detail()
{
	cellsize*=1.0/0.8;
	if(cellsize>=128){
		cellsize=128;
		return;
	}
	set_changed_detail();
	detail_direction=0;
}

//-------------------------------------------------------------
// View::reset_detail() reset detail level
//-------------------------------------------------------------
void View::reset_detail()
{
	cellsize=dflt_cellsize;
	set_changed_detail();
}
//-------------------------------------------------------------
// View::set_delt() set time change increment
//-------------------------------------------------------------
void View::set_delt(double t)
{
	delt=pow(2.0,fabs(t));
	if(t<0)
		delt=-delt;
	set_changed_time();
}

//-------------------------------------------------------------
// View::set_time() set scene time
//-------------------------------------------------------------
void View::set_time(double t)
{
	time=t;
	set_changed_time();
}

//-------------------------------------------------------------
// View::set_time() increment scene time
//-------------------------------------------------------------
void View::set_time()
{
	time+=TFACT*delt;
	set_changed_time();
}

//-------------------------------------------------------------
// View::increase_time() increment scene time
//-------------------------------------------------------------
void View::increase_time()
{
	if(autotm())
		set_delt(++tfact);
	else
		time+=TFACT*delt;
	set_changed_time();
}

//-------------------------------------------------------------
// View::decrease_time() decrease scene time
//-------------------------------------------------------------
void View::decrease_time()
{
	if(autotm())
		set_delt(--tfact);
	else
		time-=TFACT*delt;
	set_changed_time();
}

//-------------------------------------------------------------
// View::reset_time() reset scene time
//-------------------------------------------------------------
void View::reset_time()
{
	time=ftime=0;
	//tfact=10;
	set_changed_time();
	set_delt(tfact);
}

//-------------------------------------------------------------
// View::animate() do all animation tasks
//-------------------------------------------------------------
void View::animate()
{
	ftime=time;		//capture current time
	if(automv())
	    head_forward();

	if(autotm()){
		time+=time_direction*TFACT*delt;
		set_changed_time();
	}
}

void View::change_view(int v)
{
}
void View::change_view(int v,double ht)
{
}
void View::change_view(int v,Point p)
{
}

void View::setMouse(int x, int y){
	eye_trackball.setMouse(x,y);
	model_trackball.setMouse(x,y);
	light_trackball.setMouse(x,y);
}

//-------------------------------------------------------------
// View::setViewPort() set screen dimensions
//-------------------------------------------------------------
void View::setViewPort(int w, int h){
    glViewport(0, 0, (GLint) w, (GLint) h);
    window_width=w;
    window_height=h;
    if (window_height<=0)
    	window_height=1;
    aspect=(double)w/(double)h;
	getViewport(viewport);
	wscale=0.5*viewport[3]/tan(RPD*fov/2);
    set_changed_detail();
    //cout << wscale << endl;
}

void View::resetTrackball(int type){
	set_moved();
	switch(type){
	case MODEL_MOVE:
		model_trackball.reset();
		break;
	case LIGHT_MOVE:
		light_trackball.reset();
		break;
	case EYE_MOVE:
		eye_trackball.reset();
		break;
	}
}

void View::spinTrackball(int type, int x, int y){
	spinTrackball(type, x, y, 1.0);
}

void View::spinTrackball(int type, int x, int y, double ampl){
	set_moved();
    TrackBall *trackball=&eye_trackball;
    switch(type){
    default:
    case MODEL_MOVE:
     	trackball=&model_trackball;
    	break;
    case EYE_MOVE:
    	trackball=&eye_trackball;
    	break;
    case LIGHT_MOVE:
    	trackball=&light_trackball;
    	break;
    }
    trackball->spin(x,y,window_width,window_height,ampl);
 }
