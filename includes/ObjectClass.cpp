
// ObjectClass.cpp

#include "ViewClass.h"
#include "ObjectClass.h"

//#define DEBUG_OBJS
extern void screen_rect(RECT *wrect);

// Object

int Object3D::included()
{
	if(offscreen())
		return 0;
	if(selected())
		return 1;
	return 0;
}

// virtual base class functions

Object3D::~Object3D() {}
void Object3D::set_geometry() {}
void Object3D::rebuild() {}
void Object3D::init() {}
void Object3D::locate() {}
void Object3D::select(){}
void Object3D::render() {}
void Object3D::render_object() {}
void Object3D::set_ref(){}
void Object3D::animate(){}
void Object3D::set_surface(TerrainData &d){}
void Object3D::set_view(){}
void Object3D::set_scene(){}
void Object3D::adapt(){}
void Object3D::adapt_object() {}
void Object3D::init_adapt(){}
void Object3D::init_render(){}
void Object3D::init_view(){}
Bounds *Object3D::bounds(){return 0;}
void Object3D::set_lights(){}
void Object3D::set_lighting(){}
void Object3D::save(FILE *fp){}
void Object3D::set_color(Color c){}
void Object3D::map_color(MapData *d,Color &c){}
double Object3D::resolution(){ return 1.0;}
int Object3D::select_pass(){ clr_selected();return 0;}
int Object3D::adapt_pass(){ return select_pass();}
int Object3D::render_pass(){ return select_pass();}
int Object3D::selection_pass(){ clr_selected();return 0;}
int Object3D::shadow_pass(){ clr_selected();return 0;}
void Object3D::free() {}
int Object3D::cells(){ return 0;}
int Object3D::cycles(){ return 0;}
Object3D * Object3D::getObjParent(){ return 0;}
Map *Object3D::getMap() { return 0;}
void Object3D::select_pass(int t)
{
	if(TheView->bgpass==t){
		set_selected();
    }
}
void Object3D::clear_pass(int t)
{
	if(TheView->bgpass==t){
		set_selected();
		TheView->set_clrpass(1);
    }
}
int  Object3D::type()
{
	return 0;
}
const char *Object3D::name()
{
	return "Object3D";
}
Color Object3D::color()
{
	return WHITE;
}

void Object3D::set_point()
{
	point=Point(0,0,0).mm(TheView->viewMatrix)+TheView->xoffset;
}

//-------------------------------------------------------------
// Object3D::radius() 	return radius
//-------------------------------------------------------------
double  Object3D::radius()
{
    return size;
}
//-------------------------------------------------------------
// Object3D::height() 	return surface height at theta, phi
//-------------------------------------------------------------
double  Object3D::height(double t, double p)
{
    return 0;
}
//-------------------------------------------------------------
// Object3D::far_height() 	return ht for ORBITAL view
//-------------------------------------------------------------
double  Object3D::far_height()
{
    return 2.0*size;
}

//-------------------------------------------------------------
// Object3D::max_height() 	return ht range as fraction of size
//-------------------------------------------------------------
double  Object3D::max_height()
{
    return 1e-6;
}

//-------------------------------------------------------------
// Object3D::scale() 	set znear, zfar
//-------------------------------------------------------------
int  Object3D::scale(double &zn, double &zf)
{
	const double MINZN=100*FEET;

	double x,y,z,w,y1,y2,x1,x2;
	double aspect=TheView->aspect;
	double d=TheView->epoint.distance(point);

	setvis(OFFSCREEN);

	if(d<=size){
		zn=size*(1-max_height())-d;
		if(zn<MINZN){
		    zn=MINZN;
		}
		zf=size*(1+max_height())-d;

		setvis(INSIDE);
		//cout << name()<< " zn:"<<zn/FEET <<" maxht:"<< max_height()<< endl;

		return INSIDE;
	}
	else{
		zn=d-size*(1+max_height());
		if(zn<MINZN)
			zn=MINZN;
		zf=zn+size*(1+max_height());
		//cout << name()<< " zn:"<<zn/FEET <<" maxht:"<< max_height()<< endl;

	}

	z=point.mz(TheView->lookMatrix);	// screen z (before perspective)

	if(z>=0)
		return OFFSCREEN;

	w=fabs(size/tan(RPD*TheView->fov/2)/z);
	x=point.mx(TheView->projMatrix)/z;	// screen x (-1..1)
	x2=x+w;
	x1=x-w;
	if(x1>aspect || x2<-aspect)
		return OFFSCREEN;
	y=point.my(TheView->projMatrix)/z;	// screen y (-1..1)
	y2=y+w; y1=y-w;
	if(y1>1 || y2<-1)
		return OFFSCREEN;

	zn=-z-(size+max_height());		// front end
	zf=-z+size;    	// back end
	if(zn<MINZN)
		zn=MINZN;
	setvis(OUTSIDE);

	return OUTSIDE;
}

//-------------------------------------------------------------
// Object3D::depth() 	return z depth
//-------------------------------------------------------------
double Object3D::depth()
{
	double z=-point.mz(TheView->lookMatrix);
	return z>= 0 ? z :1e-9;
}

//-------------------------------------------------------------
// Object3D::set_view_info()
//-------------------------------------------------------------
void Object3D::set_view_info()
{
}
//-------------------------------------------------------------
// Object3D::set_focus() set focus rect
//-------------------------------------------------------------
void Object3D::set_focus(Point &p)
{
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
	draw_rect();
	TheView->set_istring("%s dist %g miles",name(),point.length()/MILES);
}

//-------------------------------------------------------------
// Object3D::move_focus() move scene focu
//-------------------------------------------------------------
void Object3D::move_focus(Point &selm)
{
}

//-------------------------------------------------------------
// Object3D::get_focus() get focus rect
//-------------------------------------------------------------
Point Object3D::get_focus(void *v)
{
    return point;
}

//-------------------------------------------------------------
// Object3D::draw_rect() draw focus rect
//-------------------------------------------------------------
void Object3D::draw_rect()
{
	RECT rect;
	project(rect);
	GLboolean lflag;
	glGetBooleanv(GL_LIGHTING,&lflag);
	glDisable(GL_LIGHTING);

    static Matrix M(0,1);
    static Matrix P(0,1);
    static Point R[4];

    static GLint    vport[4];
	TheView->getViewport(vport);

	double l,r,t,b,z;
	l=rect.left;
	r=rect.right;
	t=rect.top;
	b=rect.bottom;

	z=0;
	R[0]=TheView->unProject(l,t,z);
	R[1]=TheView->unProject(r,t,z);
	R[2]=TheView->unProject(r,b,z);
	R[3]=TheView->unProject(l,b,z);

	glLineWidth(2.0f);
	glColor3d(1,1,0);

	glBegin(GL_LINE_LOOP);

	glVertex3dv((double*)(R+0));
	glVertex3dv((double*)(R+1));
	glVertex3dv((double*)(R+2));
	glVertex3dv((double*)(R+3));
	glEnd();
	glFlush();
	if(lflag==GL_TRUE)
		glEnable(GL_LIGHTING);

	glLineWidth(1.0f);

}

//-------------------------------------------------------------
// Object3D::project() 	create a perspective scaled box
//-------------------------------------------------------------
void Object3D::project(RECT &wrect)
{
	double s;
	Point p;
	p=screen();
    s=extent();

    int w=TheView->viewport[2]-1;
	int h=TheView->viewport[3]-1;

	s=s<1?1:s;
	wrect.left=(int)(p.x-s);
	wrect.right=(int)(p.x+s);
	wrect.bottom=(int)(p.y+s);
	wrect.top=(int)(p.y-s);

	wrect.left=wrect.left<0?0:wrect.left;
	wrect.top=wrect.top<0?0:wrect.top;
	wrect.right=wrect.right>w?w:wrect.right;
	wrect.bottom=wrect.bottom>h?h:wrect.bottom;
}

//-------------------------------------------------------------
// Object3D::extent() 	return max screen coverage
//-------------------------------------------------------------
double Object3D::extent()
{
	double f,maxf;

	maxf=0.5* TheView->viewport[3];
	f=TheView->wscale*size/depth();
	//return f < 0 ? 0 : f> maxf ? maxf : f;
	return f < 0 ? 0 : f;
}

//-------------------------------------------------------------
// Object3D::screen() 	return projection of model center on screen
//-------------------------------------------------------------
Point Object3D::screen()
{
	double x,y,z,h,w;
	Point p;

	w=TheView->viewport[2];
	h=TheView->viewport[3];

	z=point.mz(TheView->lookMatrix);		// screen z (before perspective)
	x=1-point.mx(TheView->projMatrix)/z;	// screen x (0..2)
	y=1-point.my(TheView->projMatrix)/z;	// screen y (0..2)
	y*=h*0.5;								// y in pixels
	p.x=(x*w*0.5);							// x in pixels
	//p.y=(h-y);						// screen window has 0 at the top
	p.y=y;								// screen window has 0 at the top
	return p;
}

//-------------------------------------------------------------
// Object3D::visit() visit the object & apply function
//-------------------------------------------------------------
void Object3D::visit(void (Object3D::*func)())
{
	(this->*func)();
}

//-------------------------------------------------------------
// ObjectNode::visit() visit the object & apply function
//-------------------------------------------------------------
void Object3D::visit(void (*func)(Object3D*))
{
	(*func)(this);
}

//-------------------------------------------------------------
// Object3D::visit() visit the object & apply function
//-------------------------------------------------------------
void Object3D::visitAll(void (Object3D::*func)())
{
	(this->*func)();
}

//-------------------------------------------------------------
// ObjectNode::visit() visit the object & apply function
//-------------------------------------------------------------
void Object3D::visitAll(void (*func)(Object3D*))
{
	(*func)(this);
}
//************************************************************
// ObjectNode class
//************************************************************
ObjectNode::ObjectNode(ObjectNode *m, double s) : Object3D(s)
{
#ifdef DEBUG_OBJS
	printf("ObjectNode\n");
#endif
	parent=m;
	if(parent){
		set_groups(parent->groups());
		parent->children.add(this);
	}
}
ObjectNode::ObjectNode(ObjectNode *m) : Object3D()
{
#ifdef DEBUG_OBJS
	printf("ObjectNode\n");
#endif
	parent=m;
	size=0;
	if(parent){
	    size=parent->size;
		set_groups(parent->groups());
		parent->children.add(this);
	}
}
ObjectNode::~ObjectNode()
{
#ifdef DEBUG_OBJS
	printf("~ObjectNode\n");
#endif
	free();
}

//-------------------------------------------------------------
// ObjectNode::NodeIF methods
//-------------------------------------------------------------
int ObjectNode::getChildren(LinkedList<NodeIF*>&l){
	children.ss();
	NodeIF *obj;
	while(obj=children++)
		l.add(obj);
	return children.size;
}

//-------------------------------------------------------------
NodeIF *ObjectNode::addChild(NodeIF *c){
   if(c->typeClass()&ID_OBJECT && !children.inlist((Object3D*)c)){
		c->setParent(this);
		Object3D *obj=(Object3D*)c;
		obj->set_groups(groups());
		children.add(obj);
   }
   return c;
}
//-------------------------------------------------------------
NodeIF *ObjectNode::addAfter(NodeIF *b,NodeIF *c){
	if(b==0){  // add as first child
		c->setParent(this);
		children.ss();
		children.push((Object3D*)c);
	}
	else { //if(b->typeClass()&ID_OBJECT && children.inlist((Object3D*)b)){
	   if(b->expanded() || b->isEmpty())
		   b->addAfter(0,c);
	   else{
			c->setParent(this);
			Object3D *obj=(Object3D*)c;
			obj->set_groups(groups());
			if(b)
				children.add((Object3D*)b,obj);
			else
				children.add(obj);
	   }
    }
	return c;
}

//-------------------------------------------------------------
NodeIF *ObjectNode::removeChild(NodeIF *c){
   if(c->typeClass()&ID_OBJECT)
        children.remove((Object3D*)c);
   return c;
}

//-------------------------------------------------------------
NodeIF *ObjectNode::replaceChild(NodeIF *c,NodeIF *n){
	if(c->typeClass()&ID_OBJECT){
		n->setParent(this);
		children.replace((Object3D*)c,(Object3D*)n);
		return n;
	}
	return 0;
}

void ObjectNode::rebuild(){validate();}

//-------------------------------------------------------------
// ObjectNode::save() archiving function
//-------------------------------------------------------------
void ObjectNode::save(FILE *fp)
{
	Object3D *obj;
	children.ss();
	while(obj=children++)
		obj->save(fp);
}

//-------------------------------------------------------------
// ObjectNode::visit() visit the object & apply function recursively
//-------------------------------------------------------------
void ObjectNode::visit(void (Object3D::*func)())
{
	Object3D *obj;

	(this->*func)();

	children.ss();
	while(obj=children++){
		(obj->*func)();
	}
}

//-------------------------------------------------------------
// ObjectNode::visit() visit the objects children & apply function
//-------------------------------------------------------------
void ObjectNode::visitChildren(void  (Object3D::*func)()){
	Object3D *obj;
	children.ss();
	while(obj=children++){
		(obj->*func)();
	}
}

//-------------------------------------------------------------
// ObjectNode::visit() visit the objects children & apply function
//-------------------------------------------------------------
void ObjectNode::visitChildren(void  (*func)(Object3D*)){
	Object3D *obj;
	children.ss();
	while(obj=children++){
		obj->visit(func);
	}
}

//-------------------------------------------------------------
// ObjectNode::visit() visit the objects children & apply function
//-------------------------------------------------------------
void ObjectNode::visitAll(void (Object3D::*func)())
{
	Object3D *obj;

	(this->*func)();

#ifdef BUGGY
	// for some reason this code fails when "init" function is called
	// (maybe others?) but seems to work with some functions (e.g. clr_groups)
	// when it fails the function for the last object in the list is not
	// called (the loop is only executed n-1 times instead of n)
	children.ss();
	while(obj=children++){
		obj->visitAll(func);
	}
#else
	for(int i=0;i<children.size;i++){
		obj=children[i];
		obj->visitAll(func);
	}
#endif
}

//-------------------------------------------------------------
// ObjectNode::visit() visit the objects children & apply function
//-------------------------------------------------------------
void ObjectNode::visit(void (*func)(Object3D*))
{
	Object3D *obj;

	(*func)(this);

	children.ss();
	while(obj=children++){
		obj->visit(func);
	}
}

//-------------------------------------------------------------
// ObjectNode::visit() visit the objects children & apply function
//-------------------------------------------------------------
void ObjectNode::visitAll(void (*func)(Object3D*))
{
	visit(func);
}

//************************************************************
// ObjectMgr class
//************************************************************

void ObjectMgr::save(FILE *fp)
{
	Object3D *obj;
	list.ss();
	while((obj=list++))
		obj->save(fp);
}

Object3D*  ObjectMgr::set(Object3D *v){
	int found=list.setptr(v);
	if(!found)
		return 0;
	return list.at();
}
//-------------------------------------------------------------
// ObjectMgr::visit() visit all objects & apply Object3D::function
//-------------------------------------------------------------
void ObjectMgr::visit(void (Object3D::*func)())
{
	Object3D *obj;
	list.ss();
	while((obj=list++))
		(obj->*func)();
}

//-------------------------------------------------------------
// ObjectMgr::visit() visit all objects & apply Object3D::function
//-------------------------------------------------------------
void ObjectMgr::visitAll(void (Object3D::*func)())
{
	Object3D *obj;
	list.ss();
	while((obj=list++))
		obj->visitAll(func);
}

//-------------------------------------------------------------
// ObjectMgr::visit() visit all objects & apply void function
//-------------------------------------------------------------
void ObjectMgr::visit(void (*func)(Object3D*))
{
	Object3D *obj;
	list.ss();
	while((obj=list++))
		obj->visit(func);
}
//-------------------------------------------------------------
// ObjectMgr::visit() visit all objects & apply void function
//-------------------------------------------------------------
void ObjectMgr::visitAll(void (*func)(Object3D*))
{
	Object3D *obj;
	list.ss();
	while((obj=list++))
		obj->visitAll(func);
}
//-------------------------------------------------------------
// ObjectMgr::visit() visit all objects & apply void function
//-------------------------------------------------------------
void ObjectMgr::visitNodes(void (*func)(NodeIF*))
{
	Object3D *obj;
	list.ss();
	while((obj=list++))
		obj->visitNode(func);
}

