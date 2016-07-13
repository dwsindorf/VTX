/////////////////////////////////////////////////////////////////////////////
// Name:        wxtest.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SCENE_H_
#define _WX_SCENE_H_


#include "wx/defs.h"
#include "wx/app.h"
#include "wx/menu.h"
#include "wx/dcclient.h"

#include "wx/glcanvas.h"
class Scene;

class VtxScene: public wxGLCanvas
{
	static  wxStopWatch stopwatch;
    int frames;
    double t0;

    int argc;
    char** argv;
    int height;
    int width;
    double aspect;
    int draw_cnt;
    int vkey;
    unsigned int state;
    void make_scene();
    void update_status();
    void dragAction();
    void showFPS();
    wxGLContext* m_glRC;
    wxString  scene_name;
    void clear_canvas();

public:
    VtxScene(wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxString& name = wxT("VtxScene"));

    ~VtxScene();

    void SetCurrent();
    void SetContext();
    void SetSceneName(wxString name) { scene_name=name;}
    wxGLContext *GetContext();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnMouseDown( wxMouseEvent& event );
    void OnMiddleDown( wxMouseEvent& event );
    void OnMiddleUp( wxMouseEvent& event );
    void OnMouseMove( wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnKey(wxKeyEvent& event);

    void center_text(wxString& msg, const wxString &cname,int size,int style);
    void timer_tick();
    void open_scene(char *);
    void save_scene(char *);
    void save_image(char *,int);
    void view_image(char *,int);
    void suspend();
    void unsuspend();
    int  can_undo_view();
    int  can_redo_view();
    int  can_move_select();
    void set_autotm(int on);
    void set_automv(int on);
    int  autotm();
    int  automv();
    void clear_views();
    void rebuild();
    void reset_view();
    void set_surface_view();
    void set_orbital_view();
    void quit();
    void show_scene_dialog();
    bool scene_dialog_showing();
    void show_image_dialog();
    bool image_dialog_showing();
    void show_funct_dialog();
    bool funct_dialog_showing();
    void set_fkey_help(int);
    int  fkey_help();
    void set_key(int key);
    void setargs(int n,char **v){argc=n;argv=v;}
    void setMoveType(int move_type);
    int getMoveType();
private:
    DECLARE_EVENT_TABLE()
};

extern VtxScene *vtxScene;

#endif

