/////////////////////////////////////////////////////////////////////////////
// Name:    wxmain.cpp
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

#include "GLglue.h"
// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SceneClass.h"
#include "UniverseModel.h"
#include "TerrainClass.h"
#include "RenderOptions.h"
#include "AdaptOptions.h"
#include "Effects.h"
#include "KeyIF.h"
#include "GLSLMgr.h"
#include "Perlin.h"

#include "FileUtil.h"
#include "VtxScene.h"
#include "Bitmap.h"
#include "FrameClass.h"
#include <objectdlg.xpm>
#include <image_dialog.xpm>
#include <vtx_movie.xpm>


#include "VtxSceneDialog.h"
#include "VtxImageDialog.h"
#include "VtxFunctDialog.h"
#include "VtxMovieDialog.h"

#include "VtxImageMgr.h"
#include <wx/defs.h>

extern int scene_rendered;
//#define DEBUG_SCENE
//#define DEBUG_KEYS

static KeyIF kif;

#define Y      0x2
#define X      0x0
#define MIDDLE 0x2
#define RIGHT  0x1
#define LEFT   0x0
#define UP     0x1
#define DOWN   0x0

#define X_LEFT  0
#define X_RIGHT 1
#define Y_DOWN  2
#define Y_UP    3

static long mouse_x=0,mouse_y=0, mouse_dir=0,mouse_button=LEFT;
static bool dragging=false;
extern void set_status_field(int i,const char *c, ...);

VtxSceneDialog *sceneDialog=0;
VtxImageDialog *imageDialog=0;
VtxFunctDialog *functDialog=0;
VtxFunctDialog *noiseDialog=0;
VtxMovieDialog *movieDialog=0;

VtxScene       *vtxScene=0;


BEGIN_EVENT_TABLE(VtxScene, wxGLCanvas)
    EVT_CHAR(VtxScene::OnKey)
    EVT_SIZE(VtxScene::OnSize)
    EVT_PAINT(VtxScene::OnPaint)
    EVT_ERASE_BACKGROUND(VtxScene::OnEraseBackground)
    EVT_LEFT_DOWN(VtxScene::OnMouseDown)
    EVT_RIGHT_DOWN(VtxScene::OnMouseDown)
    EVT_MIDDLE_DOWN(VtxScene::OnMiddleDown)
    EVT_MIDDLE_UP(VtxScene::OnMiddleUp)
	EVT_MOTION(VtxScene::OnMouseMove)
	EVT_MOUSEWHEEL(VtxScene::OnMouseWheel)

END_EVENT_TABLE()

static int attributeList[] = {
	WX_GL_RGBA,
    WX_GL_DOUBLEBUFFER,
    WX_GL_DEPTH_SIZE, 24,
    WX_GL_LEVEL, 0,
    WX_GL_MIN_ACCUM_RED,8,
    WX_GL_MIN_ACCUM_GREEN,8,
    WX_GL_MIN_ACCUM_BLUE,8,
    WX_GL_MIN_ALPHA,8,
    0
};
wxStopWatch VtxScene::stopwatch;
static int last_cnt=0;

void rebuild_scene_tree(){
	sceneDialog->rebuildObjectTree();
}
void select_tree_node(NodeIF *n){
	sceneDialog->selectObject(n);
}

//-------------------------------------------------------------
// VtxScene::VtxScene() constructor
//-------------------------------------------------------------
VtxScene::VtxScene(wxString file, wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
#if wxCHECK_VERSION(3, 0, 0)
: wxGLCanvas(parent, id, attributeList, pos, size, style|wxFULL_REPAINT_ON_RESIZE,
    name)
#else
: wxGLCanvas(parent, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE,
name,attributeList)
#endif
{
    TheScene=0;
    draw_cnt=0;
    height=width=200;
    aspect=1;
    state=0;
    vkey=0;
    frames=0;
    t0=0;
    vtxScene=this;
    m_glRC=NULL;
    infile=file;
    //SetContext();
}

//-------------------------------------------------------------
// VtxScene::~VtxScene() destructor
//-------------------------------------------------------------
VtxScene::~VtxScene()
{
    //SetCurrent();
	if(sceneDialog)
		delete sceneDialog;
	if(imageDialog)
		delete imageDialog;
	if(functDialog)
		delete functDialog;
	if(noiseDialog)
		delete noiseDialog;
	if(movieDialog)
		delete movieDialog;

    if(TheScene){
        delete TheScene;
        TheScene=0;
    }
    if(m_glRC)
        delete m_glRC;
}

//-------------------------------------------------------------
// VtxScene::show_scene_dialog()
//-------------------------------------------------------------
void VtxScene::show_scene_dialog()
{
	if(sceneDialog)
		sceneDialog->Show(sceneDialog->IsShown()?false:true);
}

//-------------------------------------------------------------
// VtxScene::scene_dialog_showing()
//-------------------------------------------------------------
bool VtxScene::scene_dialog_showing()
{
	if(sceneDialog)
		return sceneDialog->IsShown();
	return false;
}

//-------------------------------------------------------------
// VtxScene::show_image_dialog()
//-------------------------------------------------------------
void VtxScene::show_image_dialog()
{
	if(imageDialog)
		imageDialog->Show(imageDialog->IsShown()?false:true);
}

//-------------------------------------------------------------
// VtxScene::image_dialog_showing()
//-------------------------------------------------------------
bool VtxScene::image_dialog_showing()
{
	if(imageDialog)
		return imageDialog->IsShown();
	return false;
}

//-------------------------------------------------------------
// VtxScene::show_movie_dialog()
//-------------------------------------------------------------
void VtxScene::show_movie_dialog()
{
	if(movieDialog)
		movieDialog->Show(movieDialog->IsShown()?false:true);
}

//-------------------------------------------------------------
// VtxScene::movie_dialog_showing()
//-------------------------------------------------------------
bool VtxScene::movie_dialog_showing()
{
	if(movieDialog)
		return movieDialog->IsShown();
	return false;
}

//-------------------------------------------------------------
// VtxScene::quit() suspend rendering
//-------------------------------------------------------------
void VtxScene::quit()
{
	if(TheScene){
		char path[256];
		File.makeFilePath("Saves", "last.spx", path);
//	    int mode = Render.mode()&SHOW_FRONT;
//		TheScene->add_expr("drawmode",(double)mode);
		TheScene->save(path);
		set_key(KEY_CMND_QUIT);
	}
}

//-------------------------------------------------------------
// VtxScene::suspend() suspend rendering
//-------------------------------------------------------------
void VtxScene::suspend()
{
    if(TheView)
        TheView->suspend();
}

//-------------------------------------------------------------
// VtxScene::unsuspend() resume rendering
//-------------------------------------------------------------
void VtxScene::unsuspend()
{
    if(TheView)
        TheView->unsuspend();
}

//-------------------------------------------------------------
// VtxScene::clear_views() clear view history
//-------------------------------------------------------------
void VtxScene::clear_views()
{
    if(TheScene)
        TheScene->views_reset();
}

//-------------------------------------------------------------
// VtxScene::rebuild() rebuild scene
//-------------------------------------------------------------
void VtxScene::rebuild()
{
    if(TheScene)
        TheScene->rebuild_all();
}

//-------------------------------------------------------------
// VtxScene::autotm() return time animation state
//-------------------------------------------------------------
int VtxScene::autotm()
{
    return TheView? TheView->autotm():0;
}

//-------------------------------------------------------------
// VtxScene::automv() return motion animation state
//-------------------------------------------------------------
int VtxScene::automv()
{
    return TheView? TheView->automv():0;
}

//-------------------------------------------------------------
// VtxScene::set_autotm() set time animation mode
//-------------------------------------------------------------
void VtxScene::set_autotm(int on)
{
    if(on)
        TheView->set_autotm();
    else
        TheView->clr_autotm();
}

//-------------------------------------------------------------
// VtxScene::set_automv() set animation mode
//-------------------------------------------------------------
void VtxScene::set_automv(int on)
{
	TheView->set_forward();
    if(on){
        TheView->set_automv();
    }
    else{
        TheView->clr_automv();
    }
}

//-------------------------------------------------------------
// VtxScene::setMoveType() set move type
//-------------------------------------------------------------
void VtxScene::setMoveType(int move_type){
	TheView->setMoveType(move_type);
	if(move_type==EYE_MOVE)
		TheView->set_mode(VIEW);
	else
		TheView->set_mode(MOVE);

}

//-------------------------------------------------------------
// VtxScene::getMoveType() set move type
//-------------------------------------------------------------
int VtxScene::getMoveType(){
    if(!TheView)
        return NO_MOVE;

	return TheView->getMoveType();
}

//-------------------------------------------------------------
// VtxFrame::set_orbital_view() goto orbit
//-------------------------------------------------------------
void VtxScene::set_orbital_view()
{
    set_key(KEY_VIEW_ORBIT);
    TheScene->set_changed_detail();
    TheScene->rebuild_all();

	//Adapt.set_maxcycles(50);
    //TheView->set_autovh();
}
//-------------------------------------------------------------
// VtxFrame::set_surface_view() goto surface
//-------------------------------------------------------------
void VtxScene::set_surface_view()
{
    set_key(KEY_VIEW_SURFACE);
    TheScene->set_changed_detail();
    TheScene->rebuild_all();
	//Adapt.set_maxcycles(50);
    //TheView->clr_autovh();
}

//-------------------------------------------------------------
// VtxScene::reset_view() reset view
//-------------------------------------------------------------
void VtxScene::reset_view()
{
	switch(mouse_button){
	case LEFT:
		TheView->set_forward();
		if(TheView->viewtype==SURFACE){
			TheView->heading=TheView->pitch=0;
			TheView->set_sidestep(0);
		}
		else{
			TheView->set_sidestep(1);
			TheView->heading=TheView->view_tilt=0;
			TheView->pitch=-90;
			TheView->heading=90;
		}
		TheView->set_moved();
		break;
	case RIGHT:
		TheView->set_forward();
		TheView->view_angle=TheView->view_skew=0.0;
		if(TheView->viewtype==SURFACE){
			TheView->view_tilt=0;
			TheView->pitch=-15;
		}
		else{
			TheView->view_tilt=0;
			TheView->pitch=-90;
		}
		TheView->set_moved();
		break;
	}
}

//-------------------------------------------------------------
// VtxScene::can_undo_view()
//-------------------------------------------------------------
int VtxScene::can_undo_view()
{
    if(!TheScene || !TheScene->views)
        return false;
    if(TheScene->views->size()<2||TheScene->views->atstart())
        return false;
    else
        return true;
}

//-------------------------------------------------------------
// VtxScene::can_redo_view()
//-------------------------------------------------------------
int VtxScene::can_redo_view()
{
    if(!TheScene || !TheScene->views)
        return false;
    if(TheScene->views->size()<2||TheScene->views->atend())
        return false;
    else
        return true;
}

//-------------------------------------------------------------
// VtxScene::can_move_select()
//-------------------------------------------------------------
int VtxScene::can_move_select()
{
    if(!TheScene)
        return false;
    if(!TheScene->viewobj || !TheScene->selobj)
        return false;
    if (TheScene->focusobj && TheScene->focusobj->typeValue()==ID_GALAXY)
        return false;
    return true;
}
//-------------------------------------------------------------
// VtxScene::clear_canvas() clear and print a message in center of canvas
//-------------------------------------------------------------
void VtxScene::clear_canvas()
{
    wxPaintDC dc(this);
    wxString  msg("Building ");
    msg+=scene_name;
    msg+=" ...";
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();
    center_text(msg,"YELLOW",15,wxFONTSTYLE_SLANT);
}

//-------------------------------------------------------------
// VtxScene::showFPS() print a message in center of canvas
//-------------------------------------------------------------
void VtxScene::showFPS()
{
	double t=stopwatch.Time()*1e-3;
	// If one second has passed, or if this is the very first frame
	if ( (t-t0) >= 1.0 || frames==0) {
		double fps=0.0;
		char str[256];
		if (t-t0>0){
			fps = (double)frames / (t-t0);
			sprintf(str, "FPS: %.1f", fps);
			set_status_field(4,str);
		}
		t0 = t;
		frames = 0;
	}
	frames++;
}

//-------------------------------------------------------------
// VtxScene::center_text() print a message in center of canvas
//-------------------------------------------------------------
void VtxScene::center_text(wxString& msg, const wxString &cname, int fsize, int style)
{
    wxPaintDC dc(this);
    dc.SetFont(*wxNORMAL_FONT);
    wxFont font=dc.GetFont();
    font.SetPointSize(fsize);
    font.SetStyle(style);
    dc.SetFont(font);
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextForeground(wxColor(cname));

    // Get window and text dimensions
    wxSize sz = GetClientSize();
    wxCoord w, h;
    dc.GetTextExtent(msg, & w, & h);

    // Center the text on the window, but never
    // draw at a negative position.
    int x = wxMax(0, (sz.x - w)/2);
    int y = wxMax(0, (sz.y - h)/2);

    dc.DrawText(msg, x, y);
    dc.SetFont(*wxNORMAL_FONT);
}

//-------------------------------------------------------------
// VtxScene::update_status() timer event
//-------------------------------------------------------------
void VtxScene::update_status()
{
	TheScene->show_info(INFO_TIME);
	TheScene->show_status(INFO_POSITION);
    TheScene->show_status(INFO_TIME);
    showFPS();
 }

//-------------------------------------------------------------
// VtxScene::timer_tick() timer event
//-------------------------------------------------------------
void VtxScene::timer_tick()
{
#ifdef DEBUG_SCENE
   cout << "VtxScene::OnTimer()"<< endl;
#endif
    SetCurrent();
    if(!TheScene){
        set_fonts(); // set fonts for opengl text
        cout<<"make_scene start"<<endl;
        make_scene();
        cout<<"make_scene end"<<endl;
    }
    draw_cnt++;
    Refresh();
}

//-------------------------------------------------------------
// VtxScene::set_fkey_help() set fkey help state
//-------------------------------------------------------------
void VtxScene::set_fkey_help(int state)
{
     TheScene->set_help(state);
     TheScene->set_changed_render();
}

//-------------------------------------------------------------
// VtxScene::fkey_help() return fkey help state
//-------------------------------------------------------------
int  VtxScene::fkey_help()
{
    return TheScene?TheScene->help():0;
}

//-------------------------------------------------------------
// VtxScene::save_scene() save scene
//-------------------------------------------------------------
void VtxScene::save_scene(char *path)
{
#ifdef DEBUG_SCENE
    cout << "VtxScene::save_scene("<<path<<")" <<endl;
#endif
    TheScene->save(path);
}

//-------------------------------------------------------------
// VtxScene::save_image() save image
//-------------------------------------------------------------
void VtxScene::save_image(char *path,int type)
{
    char fullpath[256];
	char filename[64];
    char dir[256];

	File.getFilePath(path,dir);
	File.getFileName(path,filename);

    SetCurrent();
#ifdef DEBUG_SCENE
    cout << "VtxScene::save_image("<<path<<")" <<endl;
#endif
    switch(type){
    case FILE_JPG:
    	sprintf(fullpath,"%s/%s.jpg",dir,filename);
    	VtxImageMgr::writeJpegFile(fullpath,Render.image_quality());
        break;
    case FILE_BMP:
    	sprintf(fullpath,"%s/%s.bmp",dir,filename);
    	VtxImageMgr::writeBmpFile(fullpath);
        break;
    }
}

//-------------------------------------------------------------
// VtxScene::view_image() save image
//-------------------------------------------------------------
void VtxScene::view_image(char *path,int type)
{
    SetCurrent();
    switch(type){
    case FILE_JPG:
#ifdef DEBUG_SCENE
    cout << "VtxScene::view_image("<<path<<")" <<endl;
#endif
		VtxImageMgr::showJpegFile(path);
        break;
    case FILE_BMP:
    	VtxImageMgr::showBmpFile(path);
        break;
    }
}


//-------------------------------------------------------------
// VtxScene::rebuild_all() rebuild all UI elements
//-------------------------------------------------------------
void VtxScene::rebuild_all() {
    functDialog->clear();
    noiseDialog->clear();
    sceneDialog->rebuildObjectTree();
    imageDialog->Invalidate();
    movieDialog->Invalidate();
    rebuild();
}

//-------------------------------------------------------------
// VtxScene::open_scene() open & build scene object
//-------------------------------------------------------------
void VtxScene::open_scene(char *path)
{
#ifdef DEBUG_SCENE
    cout << "VtxScene::open_scene("<<path<<")" <<endl;
#endif
    //Adapt.set_maxcycles(50);

    TheScene->open(path);
    rebuild_all();
}

//-------------------------------------------------------------
// VtxScene::make_scene() build Startup scene object
//-------------------------------------------------------------
void VtxScene::make_scene()
{
    //wxString infile;
    char sargs[256];
    int  nargs=0;
    int i;
#ifdef DEBUG_SCENE
     cout << "VtxScene::make_scene()"<<endl;
#endif
#ifdef GLEW
	//glewExperimental = GL_TRUE;
	glewInit();
#endif
    SetCurrent();

    GetClientSize(&width, &height);
    bool make_default=false;

    for (i = 1; i < argc; i++) {
        if (argv[i][0]=='-'){
            switch(argv[i][1]){
            case KEY_DEFAULT:
            	make_default=true;
            	break;
            default:
                sargs[nargs++]=argv[i][1];
            }
        }
    }
    glViewport(0, 0, (GLint) width, (GLint) height);

    TheScene=new Scene(new UniverseModel());
    Perlin::seed=1;
	GLSLMgr::initGL(width,height);
    TheScene->resize(width,height);
    TheScene->init();
    if(!sceneDialog){
    	sceneDialog=new VtxSceneDialog(this);
    	sceneDialog->SetIcon(wxIcon(objectdlg_xpm));
    }
    if(!imageDialog){
    	imageDialog=new VtxImageDialog(this);
    	imageDialog->SetIcon(wxIcon(image_dialog_xpm));
    }
    if(!movieDialog){
    	movieDialog=new VtxMovieDialog(this);
    	movieDialog->SetIcon(wxIcon(vtx_movie_xpm));
    }

    if(!functDialog){
    	functDialog=new VtxFunctDialog(this);
    }
    if(!noiseDialog){
    	noiseDialog=new VtxFunctDialog(this);
    }
	//Render.invalidate_textures();
    if(make_default)
    	TheScene->make(); // create Default scene

    char path[256];
    if(infile.Length()>0){
        char name[256];
        File.getFileName((char*)infile.ToAscii(),name);
        strcat(name,FileUtil::ext);
        File.makeFilePath(File.saves,name,path);
    }
    else{
    	File.makeFilePath(File.saves,(char*)"Default.spx",path);
    }
    TheScene->open(path);
    setMoveType(MODEL_MOVE);

    glViewport(0, 0, (GLint) width, (GLint) height);

    TheScene->resize(width,height);

    state=0;
    cout<< "VtxScene: # key cmnds="<<nargs<<endl;

    for (i = 0; i < nargs; i++){
        kif.standard_key(state, sargs[i]);
    }
    kif.get_state(state);
    TheScene->set_changed_detail();
    TheScene->clr_autotm();

    sceneDialog->rebuildObjectTree();
    imageDialog->Invalidate();
    movieDialog->Invalidate();
	stopwatch.Start();
}


//-------------------------------------------------------------
// VtxScene::set_key() set key command
//-------------------------------------------------------------
void VtxScene::set_key(int key)
{
    vkey=key;
    switch(key){
    case WXK_LEFT:  vkey=KEY_LEFT;     break;
    case WXK_RIGHT: vkey=KEY_RIGHT;    break;
    case WXK_UP:    vkey=KEY_FORWARD;  break;
    case WXK_DOWN:  vkey=KEY_BACKWARD; break;
    case WXK_F1:    vkey=KEY_F1;       break;
    case WXK_F2:    vkey=KEY_F2;       break;
    case WXK_F3:    vkey=KEY_F3;       break;
    case WXK_F4:    vkey=KEY_F4;       break;
    case WXK_F5:    vkey=KEY_F5;       break;
    case WXK_F6:    vkey=KEY_F6;       break;
    case WXK_F7:    vkey=KEY_F7;       break;
    case WXK_F8:    vkey=KEY_F8;       break;
    case WXK_F9:    vkey=KEY_F9;       break;
    }
#ifdef DEBUG_KEYS
    cout << "VtxScene::set_key("<<key<<")" <<endl;
#endif
}
// ################# Event Handler functions ################
//-------------------------------------------------------------
// VtxScene::OnEraseBackground() Do nothing, to avoid flashing on MSW
//-------------------------------------------------------------
void VtxScene::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
}

//-------------------------------------------------------------
// VtxScene::OnSize() resize event handler
//-------------------------------------------------------------
void VtxScene::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
   // wxGLCanvas::OnSize(event);
    if(!IsShown())
    	return;

    // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)

#ifndef __WXMOTIF__
    if ( GetContext() )
#endif
    {
    	int w, h;
    	GetClientSize(&w, &h);
        width=w;
        height=h;
#ifdef DEBUG_SCENE
    cout << "VtxScene::resizeGL("<<w<<","<<h<<")" <<endl;
#endif
        SetCurrent();
        aspect=(double)w/(double)h;
        glViewport(0, 0, (GLint) w, (GLint) h);
        if(TheScene){
            TheScene->resize(w,h);
            TheScene->set_changed_detail();
        }
     }
}

bool  VtxScene::SwapBuffers(){
	wxGLCanvas::SwapBuffers();
}
//-------------------------------------------------------------
// VtxScene::OnKey() key-press event handler
//-------------------------------------------------------------
void VtxScene::OnKey( wxKeyEvent& event )
{
	int key=event.GetKeyCode();
	if(key==' '){
		SetCurrent();
		SwapBuffers();
		Refresh();
		Update();
	}
	else
		set_key(key);
}

//-------------------------------------------------------------
// VtxScene::OnMiddleDown() middle-mouse event handler
//-------------------------------------------------------------
void VtxScene::OnMiddleDown( wxMouseEvent& event )
{
	TheView->set_forward();
    TheView->set_automv();
}
//-------------------------------------------------------------
// VtxScene::OnMiddleUp() middle-mouse event handler
//-------------------------------------------------------------
void VtxScene::OnMiddleUp( wxMouseEvent& event )
{
    TheView->clr_automv();
}
//-------------------------------------------------------------
// VtxScene::OnMouseDown() left-right-mouse up event handler
//-------------------------------------------------------------
void VtxScene::OnMouseDown( wxMouseEvent& event )
{
    extern void mouse_down(int x, int y); // in SceneClass.cpp
    long x,y;
	event.GetPosition(&x, &y);
    //if(TheView){
    	//int mtype=TheView->getMoveType();
    	//if(mtype==NO_MOVE){
	      //  mouse_down(x, y);
    	//}
    //}
    mouse_x=x;
    mouse_y=y;
    dragging = false;
    last_cnt=draw_cnt;
    if(event.LeftIsDown()){
    	mouse_button=LEFT;
   		if(TheView)
    		 mouse_down(x, y);
    }
    else if (event.RightIsDown()){
    	mouse_button=RIGHT;
    }
   // cout << "mouse down:"<< draw_cnt<<endl;
}

//-------------------------------------------------------------
// VtxScene::OnMouseMove() mouse-drag event handler
//-------------------------------------------------------------
void VtxScene::OnMouseMove(wxMouseEvent& event )
{
	int x = event.GetX();
	int y = event.GetY();
	if (event.Dragging()) {
		if(draw_cnt-last_cnt<10)
			return;
		if (event.LeftIsDown())
			mouse_button = LEFT;
		else if (event.RightIsDown())
			mouse_button = RIGHT;
		else {
			return;
		}

		dragging = true;
		//cout << "dragging:"<< draw_cnt<<endl;
		long dx = x - mouse_x;
		long dy = y - mouse_y;
		mouse_dir = 0;
		if (abs(dy) > abs(dx)) {
			mouse_dir |= Y;
			if (dy > 0)
				mouse_dir |= UP;
		} else {
			if (dx > 0)
				mouse_dir = LEFT;
			else
				mouse_dir = RIGHT;
		}
		mouse_x = x;
		mouse_y = y;
	}
}

//-------------------------------------------------------------
// VtxScene::OnMouseWheel() mouse-wheel event handler
//-------------------------------------------------------------
void VtxScene::OnMouseWheel(wxMouseEvent& event )
{
	int dir=event.GetWheelRotation();
	//Adapt.set_maxcycles(3);
    if(TheView){
    	if(scene_rendered && !TheView->motion())
    		return;
    	if(TheView->viewtype==ORBITAL){
	    	if(dir>0)
	    		TheView->move_forward();
	    	else
	    		TheView->move_backward();
    	}
    	else {
	    	if(dir>0)
	    		TheView->move_down();
	    	else
	    		TheView->move_up();
        }

    }
	//->changeElevation(sign*delta_move);
}

//-------------------------------------------------------------
// VtxScene::dragAction() mouse drag event handler
//-------------------------------------------------------------
void VtxScene::dragAction(){
	//Adapt.set_maxcycles(3);

	switch(TheView->viewtype){
	case SURFACE:
		switch(mouse_dir){
		case X_LEFT:
			if(mouse_button==RIGHT)
				TheView->look_left();
			else
				TheView->left();
			break;
		case X_RIGHT:
			if(mouse_button==RIGHT)
				TheView->look_right();
			else
				TheView->right();
			break;
		case Y_UP:
//			if(TheView->mode==VIEW)
//				TheView->look_down();
//			else if(TheView->automv()||mouse_button==RIGHT)
//				TheView->head_down();
//			else
//				TheView->move_forward();
			if(TheView->automv())
				TheView->head_down();
			else if(mouse_button==RIGHT)
				TheView->look_down();
			else
				TheView->move_forward();

			break;
		case Y_DOWN:
//			if(TheView->mode==VIEW)
//				TheView->look_up();
//			else if(TheView->automv()||mouse_button==RIGHT)
//				TheView->head_up();
//			else
//				TheView->move_backward();
			if(TheView->automv())
				TheView->head_up();
			else if(mouse_button==RIGHT)
				TheView->look_up();
			else
				TheView->move_backward();
			break;
		}
		break;
	case ORBITAL:
		switch(mouse_dir){
		case X_LEFT:
			TheView->left();
			break;
		case X_RIGHT:
			TheView->right();
			break;
		case Y_DOWN:
			if(mouse_button==RIGHT)
				TheView->head_up();
			else
				TheView->move_up();
			break;
		case Y_UP:
			if(mouse_button==RIGHT)
				TheView->head_down();
			else
				TheView->move_down();
			break;
		}
		break;
	}
    dragging=false;

}

//-------------------------------------------------------------
// VtxScene::OnPaint() repaint needed event handler
//-------------------------------------------------------------
void VtxScene::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    if(!GetContext()){
    	SetContext();
        SetCurrent();
        clear_canvas();
     }
     if(!IsShown())
    	return;
     if(!TheScene)
    	return;
#ifdef DEBUG_SCENE
    cout << "VtxScene::OnPaint()" <<endl;
#endif
    kif.get_state(state);
    if(state & CMD_QUIT)
         return;

    SetCurrent();
  	if(dragging)
  		dragAction();

	if (vkey) {
		if (vkey == WXK_TAB)
			show_scene_dialog();
		else
			kif.set_key(state, (unsigned) vkey);
		if(!motionKeypressed((wxKeyCode)vkey))
			vkey = 0;
	}
    wxString cwd=::wxGetCwd();

    ::wxSetWorkingDirectory("../Shaders");

   //cout << wxGetCwd() << endl;
   // if(!TheScene->suspended())
    int moved=TheScene->moved();
    TheScene->render();
    if(TheScene->automv()||TheScene->autotm()||moved)
    	sceneDialog->updateTabs();
    if(scene_rendered)
         update_status();
    ::wxSetWorkingDirectory(cwd);
}

void VtxScene::SetCurrent() {
    if(!IsShown())
    	return;
    if(!GetContext())
    	 cout << "VtxScene::Error SetCurrent called before context initialized"<< endl;
#if wxCHECK_VERSION(3, 0, 0)
	wxGLCanvas::SetCurrent(*m_glRC);
#else
	wxGLCanvas::SetCurrent();
#endif
}

bool VtxScene::motionKeypressed(wxKeyCode key) {
	bool keystate=wxGetKeyState(key);
	if(wxGetKeyState(WXK_LEFT) ||wxGetKeyState(WXK_RIGHT)){
		return true;
	}
	if(!keystate)
		return false;
	if(key==KEY_UP ||  key==KEY_DOWN)
		return true;
	return false;
}

wxGLContext *VtxScene::GetContext(){
	return m_glRC;
}
void VtxScene::SetContext(){
	if(m_glRC)
		delete m_glRC;
    m_glRC = new wxGLContext(this);
}

