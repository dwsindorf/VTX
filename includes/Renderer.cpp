// Renderer.cpp

#include "Renderer.h"
#include "AdaptOptions.h"
#include "RenderOptions.h"
#include "GLSLMgr.h"

#include "GLglue.h"
#include <math.h>
#include <stdio.h>

// see what version is active at compile time

RenderOptions Render;

//************************************************************
// Renderer class
//************************************************************
Renderer::Renderer()
{
	last_view=new View();
	objects=new ObjectMgr();
	ambient_min=0.5;
	ambient_bias=0;
	tex_mip=color_mip=freq_mip=bump_mip=0;
	ambient=ambient_min;
	quality=NORMAL;
	current_size=-1;
#ifdef WINDOWS
	hborder=108;
	wborder=58;
#else
	hborder=6;
	wborder=6;
#endif

    small_ht=SMALL_H;
    small_width=SMALL_W;

    med_ht=MED_H;
    med_width=MED_W;

    big_ht=BIG_H;
    big_width=BIG_W;

	pan_ht=PAN_H;
	pan_width=PAN_W;

	wide_ht=WIDE_H;
	wide_width=WIDE_W;

	custom_ht=med_ht;
	custom_width=med_width;
	set_keep_children(1);
	set_keep_variables(1);
}

Renderer::~Renderer()
{
	free();
}

//-------------------------------------------------------------
// Renderer::free() free allocated memory
//-------------------------------------------------------------
void Renderer::free()
{
    if(objects){
	    objects->free();
		delete objects;
		objects=0;
	}
	if(last_view){
		delete last_view;
		last_view=0;
	}
}

//-------------------------------------------------------------
// Renderer::set_size() set viewport size
//-------------------------------------------------------------
void Renderer::set_size(int s,int w,int h)
{
    switch(s){
    case SMALL:
    	small_width=w;
    	small_ht=h;
    	break;
    case MEDIUM:
    	med_width=w;
    	med_ht=h;
    	break;
    case BIG:
    	big_width=w;
    	big_ht=h;
    	break;
    case CUSTOM:
    	custom_width=w;
    	custom_ht=h;
    	break;
	}
}

//-------------------------------------------------------------
// Renderer::set_size() get viewport size
//-------------------------------------------------------------
void Renderer::get_size(int s,int &w,int &h)
{
    switch(s){
    case SMALL:
    	w=small_width;
    	h=small_ht;
    	break;
    default:
    case MEDIUM:
    	w=med_width;
    	h=med_ht;
    	break;
    case BIG:
    	w=big_width;
    	h=big_ht;
    	break;
    case PAN:
    	w=pan_width;
    	h=pan_ht;
    	break;
    case WIDE:
    	w=wide_width;
    	h=wide_ht;
    	break;
    case CUSTOM:
    	w=custom_width;
    	h=custom_ht;
    	break;
	}
}

//-------------------------------------------------------------
// Renderer::set_size() change viewport size
//-------------------------------------------------------------
void Renderer::set_size(int s)
{
    current_size=s;
    int w,h;
    switch(s){
    case SMALL:
    	w=small_width;
    	h=small_ht;
    	break;
    default:
    case MEDIUM:
    	w=med_width;
    	h=med_ht;
    	break;
    case BIG:
    	w=big_width;
    	h=big_ht;
    	break;
    case PAN:
    	w=pan_width;
    	h=pan_ht;
    	break;
    case WIDE:
    	w=wide_width;
    	h=wide_ht;
    	break;
    case CUSTOM:
    	w=custom_width;
    	h=custom_ht;
    	break;
	}
    set_window_size(w+wborder,h+hborder);
    resize(w,h);
}

//-------------------------------------------------------------
// Renderer::get_size() get current size
//-------------------------------------------------------------
int Renderer::get_size()
{
    return current_size;
}

//-------------------------------------------------------------
// Renderer::get_size() change viewport size
//-------------------------------------------------------------
void Renderer::get_size(int &w, int &h)
{
    //glViewport(0, 0, w, h);
	w=current_width;
	h=current_ht;
}

//-------------------------------------------------------------
// Renderer::resize() change viewport size
//-------------------------------------------------------------
void Renderer::resize(int w, int h)
{
	if(h == 0||w==0)
		return;
	current_width=w;
	current_ht=h;
	setViewPort(w,h);
    printf("Renderer::resize(%d,%d)\n",w,h);
	GLSLMgr::initFrameBuffer();

}

//-------------------------------------------------------------
// Renderer::rebuild_all() build the startup scene
//-------------------------------------------------------------
void Renderer::rebuild_all()
{
	objects->visitAll(&Object3D::invalidate);
	rebuild();
}

//-------------------------------------------------------------
// Renderer::rebuild() build the startup scene
//-------------------------------------------------------------
void Renderer::rebuild()
{
	set_changed_detail();
	objects->visitAll(&Object3D::rebuild);
}

//-------------------------------------------------------------
// Renderer::init() initialize all
//-------------------------------------------------------------
void Renderer::init()
{
	gl_init();
	reset();
}

//-------------------------------------------------------------
// Renderer::init() initialize open-GL
//-------------------------------------------------------------
void Renderer::gl_init()
{
	static GLfloat  specular[] = {1.0f, 1.0f, 1.0f, 0.0f};
	static GLfloat shininess [] = { 15.0f };
	glCullFace(GL_BACK);			// Cull the back
	glFrontFace(GL_CW);			// Counter Clock wise wound is front
	glEnable(GL_CULL_FACE);			// Enable the culling
	glEnable(GL_DEPTH_TEST);		// Hidden surface removal

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	//glDepthFunc(GL_LESS);

	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);

	glColor4d(1,1,1,1);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	set_ambient();

	glEnable(GL_COLOR_MATERIAL);

	//GLSupport::gl_init();
}

//-------------------------------------------------------------
// Renderer::set_ambient() set global ambient light
//-------------------------------------------------------------
void Renderer::set_ambient(double f)
{
    ambient=f;
    set_ambient();
}

//-------------------------------------------------------------
// Renderer::set_ambient() set global ambient light
//-------------------------------------------------------------
void Renderer::set_ambient()
{
	static GLfloat  a[] = { 0.2f, 0.2f, 0.2f, 1.0f};
	a[0]=a[1]=a[2]=(float)ambient_bias;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,a);
}

//-------------------------------------------------------------
// Renderer::reset() reset state
//-------------------------------------------------------------
void Renderer::reset()
{
	if(objects)
	    objects->free();
	View::reset();
	init_view();
	ambient_bias=0;
	ambient=ambient_min;
}

//-------------------------------------------------------------
// Renderer::reset_view() reset view
//-------------------------------------------------------------
void Renderer::reset_view()
{
	View::reset(last_view);
}

//-------------------------------------------------------------
// Renderer::erase() erase the frame buffer
//-------------------------------------------------------------
void Renderer::erase()
{
	GLSLMgr::clrFBODepthBuffers();
 	glClearColor((float)backcolor.red(),
 				 (float)backcolor.green(),
 				 (float)backcolor.blue(),
 				 1.0f);
	GLSLMgr::clrBuffers();

	glClearColor(0.0f, 0.0f,0.0f,1.0f);
}

//-------------------------------------------------------------
// Renderer::render_objects() render all objects
//-------------------------------------------------------------
void Renderer::render_objects()
{
	objects->visit(&Object3D::render);
}

//-------------------------------------------------------------
// Renderer::show_solid() show surface as solid
//-------------------------------------------------------------
void Renderer::show_solid()
{
	Render.show_solid();
	set_changed_render();
}

//-------------------------------------------------------------
// Renderer::show_lines() show surface as lines
//-------------------------------------------------------------
void Renderer::show_lines()
{
	Render.show_lines();
	set_changed_render();
}

//-------------------------------------------------------------
// Renderer::show_points() show surface as points
//-------------------------------------------------------------
void Renderer::show_points()
{
	Render.show_points();
	set_changed_render();
}

//-------------------------------------------------------------
// Renderer::init_view() initialize view
//-------------------------------------------------------------
void Renderer::init_view()
{
	View::reset();
	phi=0;
	heading=0;
	view_tilt=0;
	view_angle=0;
	view_skew=0.0;
	set_changed_view();
	set_moved();
}

//-------------------------------------------------------------
// Renderer::adapt() adapt the scene to current conditions
//-------------------------------------------------------------
void Renderer::adapt()
{
}

//-------------------------------------------------------------
// Scene::render() render the scene
//-------------------------------------------------------------
void Renderer::render()
{
}

//-------------------------------------------------------------
// Renderer::set_fog() set fog value
//-------------------------------------------------------------
void Renderer::set_glfog(double v, double min ,double max, Color c)
{
  	glDisable(GL_FOG);
	if(!v || !Render.haze())
		return;
	static float fc[4]={0.0f,0.0f,0.0f,1.0f};
	v*=Render.haze_value();
	fc[0]=(float)c.red();
	fc[1]=(float)c.green();
	fc[2]=(float)c.blue();
	  glFogfv(GL_FOG_COLOR,fc);
	switch(Render.fogmode()){
	case EXP:
		glFogf(GL_FOG_DENSITY,(float)(v/max));
		glFogi(GL_FOG_MODE,GL_EXP);
		break;
	case EXP2:
		glFogf(GL_FOG_DENSITY,(float)(v/max));
		glFogi(GL_FOG_MODE,GL_EXP2);
		break;
	case LINEAR:
	 	glFogf(GL_FOG_START,(float)(min));
		glFogf(GL_FOG_END,(float)(max/v));
		glFogi(GL_FOG_MODE,GL_LINEAR);
		break;
	}
	glEnable(GL_FOG);
}
