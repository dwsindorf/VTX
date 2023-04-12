
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

#include "VtxFrame.h"
#include "FileUtil.h"
#include "KeyIF.h"
#include "ViewClass.h"

#include <fileopen.xpm>
#include <filesave.xpm>
#include <undo.xpm>
#include <redo.xpm>

#include <vtx_image.xpm>
#include <vtx_view_image.xpm>
#include <vtx_move_orbit.xpm>
#include <vtx_move_select.xpm>
#include <vtx_move_surface.xpm>
#include <vtx_autotm.xpm>
#include <vtx_automv.xpm>
#include <vtx_question.xpm>
#include <vtx_small_icon.xpm>
#include <vtx_movie.xpm>
//#include <eyemove.xpm>
//#include <modelmove.xpm>
//#include <resetmove.xpm>
#include <objectdlg.xpm>
#include <image_dialog.xpm>

static VtxFrame *vtxframe=0;

//#define PRINT_STATUS
//#define DEBUG_DLGS

// called from GLglue.cpp
void set_indicator(int n,const char *c, va_list argptr)
{
    char buff[256];
    vsprintf( buff, c, argptr );
#ifdef PRINT_STATUS
    cout << buff<< endl;
#endif
    vtxframe->set_status(buff, n);
}

void set_status_field(int i,const char *c, ...)
{
	va_list argptr;
	va_start( argptr, c );
	set_indicator(i,c,argptr);
	va_end(argptr);
}

void swap_buffers()
{
	vtxframe->swap_buffers();
}
// define all resource ids here
enum {
    TIMER_ID            = 1,
    ID_TOOLBAR1         = 100,
    IDM_FILE_SCENE_OPEN = 110,
    IDM_FILE_SCENE_SAVE = 111,
    IDM_FILE_IMAGE_SAVE = 112,
    IDM_FILE_IMAGE_OPEN = 113,
    IDM_EDIT_UNDO       = 115,
    IDM_EDIT_REDO       = 116,
    IDM_VIEW_SHOW_TB1   = 120,
    IDM_VIEW_CLEAR_ALL  = 121,
    IDM_VIEW_REBUILD    = 122,
    IDM_VIEW_MOVETO     = 130,
      IDM_VIEW_GLOBAL   = 131,
      IDM_VIEW_ORBIT    = 132,
      IDM_VIEW_SURFACE  = 133,
      IDM_VIEW_SELECT   = 134,
    IDM_VIEW_ANIMATE    = 140,
      IDM_VIEW_AUTOTM   = 141,
      IDM_VIEW_AUTOMV   = 142,
    IDM_VIEW_WSIZE      = 150,
      IDM_VIEW_SIZE_SML = 151,
      IDM_VIEW_SIZE_MED = 152,
      IDM_VIEW_SIZE_BIG = 153,
      IDM_VIEW_SIZE_WID = 154,
      IDM_VIEW_SIZE_PAN = 155,
    IDM_VIEW_LOD        = 160,
      IDM_VIEW_LOD_LOW  = 161,
      IDM_VIEW_LOD_MED  = 162,
      IDM_VIEW_LOD_HIGH = 163,
      IDM_VIEW_LOD_BEST = 164,
      IDM_VIEW_LOD_MORE = 165,
      IDM_VIEW_LOD_LESS = 166,
      IDM_HELP_FKEYS    = 200,
    IDM_VIEW_OBJSDLG    = 350,
    IDM_VIEW_IMAGEDLG   = 351,
    IDM_VIEW_MOVIEDLG   = 352,
    IDM_HELP_ABOUT      = 400
};

BEGIN_EVENT_TABLE(VtxFrame, wxFrame)
    EVT_TIMER(TIMER_ID,             VtxFrame::OnTimer)
    EVT_CLOSE(                      VtxFrame::OnClose)
    EVT_MENU(wxID_EXIT,             VtxFrame::OnExit)
    EVT_MENU(IDM_EDIT_UNDO,         VtxFrame::OnUndo)
    EVT_MENU(IDM_EDIT_REDO,         VtxFrame::OnRedo)
    EVT_MENU(IDM_FILE_SCENE_OPEN,   VtxFrame::OnSceneOpen)
    EVT_MENU(IDM_FILE_SCENE_SAVE,   VtxFrame::OnSceneSave)
    EVT_MENU(IDM_FILE_IMAGE_SAVE,   VtxFrame::OnImageSave)
    EVT_MENU(IDM_FILE_IMAGE_OPEN,   VtxFrame::OnImageOpen)
    EVT_MENU(IDM_VIEW_SHOW_TB1,     VtxFrame::OnShowTB1)
    EVT_MENU(IDM_VIEW_CLEAR_ALL,    VtxFrame::OnClearViews)
    EVT_MENU(IDM_VIEW_REBUILD,      VtxFrame::OnRebuild)
    EVT_MENU(IDM_VIEW_ORBIT,        VtxFrame::OnOrbit)
    EVT_MENU(IDM_VIEW_SURFACE,      VtxFrame::OnSurface)
    EVT_MENU(IDM_VIEW_SELECT,       VtxFrame::OnSelect)
    EVT_MENU(IDM_VIEW_AUTOTM,       VtxFrame::OnAutotm)
    EVT_MENU(IDM_VIEW_AUTOMV,       VtxFrame::OnAutomv)
    EVT_MENU(IDM_VIEW_SIZE_SML,     VtxFrame::OnSizeSmall)
    EVT_MENU(IDM_VIEW_SIZE_MED,     VtxFrame::OnSizeMedium)
    EVT_MENU(IDM_VIEW_SIZE_BIG,     VtxFrame::OnSizeBig)
    EVT_MENU(IDM_VIEW_SIZE_WID,     VtxFrame::OnSizeWide)
    EVT_MENU(IDM_VIEW_SIZE_PAN,     VtxFrame::OnSizePan)
    EVT_MENU(IDM_VIEW_LOD_LOW,      VtxFrame::OnLodLow)
    EVT_MENU(IDM_VIEW_LOD_MED,      VtxFrame::OnLodMed)
    EVT_MENU(IDM_VIEW_LOD_HIGH,     VtxFrame::OnLodHigh)
    EVT_MENU(IDM_VIEW_LOD_BEST,     VtxFrame::OnLodBest)
    EVT_MENU(IDM_VIEW_LOD_MORE,     VtxFrame::OnLodMore)
    EVT_MENU(IDM_VIEW_LOD_LESS,     VtxFrame::OnLodless)
    EVT_MENU(IDM_HELP_FKEYS,        VtxFrame::OnHelpFkeys)
    EVT_MENU(IDM_HELP_ABOUT,        VtxFrame::OnAbout)

    EVT_UPDATE_UI(IDM_HELP_FKEYS,   VtxFrame::OnUpdateFkeys)
    EVT_UPDATE_UI(IDM_EDIT_UNDO,    VtxFrame::OnUpdateUndo)
    EVT_UPDATE_UI(IDM_EDIT_REDO,    VtxFrame::OnUpdateRedo)
    EVT_UPDATE_UI(IDM_VIEW_SELECT,  VtxFrame::OnUpdateSelect)
    EVT_UPDATE_UI(IDM_VIEW_AUTOTM,  VtxFrame::OnUpdateAutotm)
    EVT_UPDATE_UI(IDM_VIEW_AUTOMV,  VtxFrame::OnUpdateAutomv)

	EVT_MENU(IDM_VIEW_OBJSDLG,VtxFrame::OnObjsDlg)
	EVT_UPDATE_UI(IDM_VIEW_OBJSDLG,VtxFrame::OnUpdateObjsDlg)
	EVT_MENU(IDM_VIEW_IMAGEDLG,VtxFrame::OnImageDlg)
	EVT_UPDATE_UI(IDM_VIEW_IMAGEDLG,VtxFrame::OnUpdateImageDlg)
	EVT_MENU(IDM_VIEW_MOVIEDLG,VtxFrame::OnMovieDlg)
	EVT_UPDATE_UI(IDM_VIEW_MOVIEDLG,VtxFrame::OnUpdateMovieDlg)


 END_EVENT_TABLE()

//-------------------------------------------------------------
// VtxFrame::VtxFrame() constructor
//-------------------------------------------------------------
VtxFrame::VtxFrame(wxFrame *frame, const wxString& filename, const wxPoint& pos,
    const wxSize& size, long style)
    : wxFrame(frame, wxID_ANY, filename, pos, size, style)
    , m_timer(this, TIMER_ID)
{
	::wxInitAllImageHandlers();

	char path[256];
    m_canvas = 0;
    static const int widths[] = { 80, 90,200,-1, 100 };
    wxIcon icon(vtx_small_icon_xpm);
    SetIcon(icon);
    CreateStatusBar(5);
    wxStatusBar *sb = GetStatusBar();
    sb->SetStatusWidths(5, widths);
    SetStatusBarPane(-1);
    File.makeFilePath(File.images,"snapshot",path);
    m_last_image=wxString(path);
    m_last_itype=FILE_JPG;
    wxString infile("untitled");
    if(filename!="")
    	infile=filename;
    SetTitle("VTX-"+infile);
    File.makeFilePath(File.saves,(char*)infile.ToAscii(),path);
    m_last_scene=wxString(path);
    make_tbar1();
    make_menubar();
    vtxframe=this;
}
static int repeat_rate=20;
//-------------------------------------------------------------
// VtxFrame::start_timer() start the refresh counter
//-------------------------------------------------------------
void VtxFrame::start_timer(int t)
{
    repeat_rate=t;
    m_timer.Start(t);
}
//-------------------------------------------------------------
// VtxFrame::set_status() update status bar
//-------------------------------------------------------------
void VtxFrame::set_status(char *c,int n)
{
    SetStatusText(c, n);
    wxStatusBar *sb = GetStatusBar();
    sb->Update();
}

//-------------------------------------------------------------
// VtxFrame::swap_buffers() swap gl buffers
//-------------------------------------------------------------
void VtxFrame::swap_buffers()
{
	m_canvas->SetCurrent();
	m_canvas->SwapBuffers();
}
//-------------------------------------------------------------
// VtxFrame::make_menubar() build menubar
//-------------------------------------------------------------
void VtxFrame::make_menubar()
{
    wxMenuBar *menubar = new wxMenuBar;
    wxMenu *menu  = 0;
    wxMenu *submenu = 0;

    // file menu
    menu = new wxMenu;
    menu->Append(IDM_FILE_SCENE_OPEN, wxT("Open .."));
    menu->Append(IDM_FILE_SCENE_SAVE, wxT("Save .."));
    menu->Append(IDM_FILE_IMAGE_SAVE, wxT("Save Image .."));
    menu->Append(IDM_FILE_IMAGE_OPEN, wxT("View Image .."));
    menu->Append(wxID_EXIT, wxT("E&xit"));
    menubar->Append(menu, wxT("&File"));

    menu = new wxMenu;
    menu->Append(IDM_EDIT_UNDO, wxT("Undo &Z"));
    menu->Append(IDM_EDIT_REDO, wxT("Redo &Y"));
    menu->Append(IDM_VIEW_REBUILD, wxT("Rebuild\tSHIFT-R"));
    menubar->Append(menu, wxT("&Edit"));

    // view menu
    menu = new wxMenu;
    menu->AppendCheckItem(IDM_VIEW_SHOW_TB1,
                              _T("Show Toolbar"),
                              _T("Show/Hide the toolbar"));
    menu->Check(IDM_VIEW_SHOW_TB1,true);
    menu->Append(IDM_VIEW_CLEAR_ALL, wxT("Clear History"));
    menu->AppendSeparator();
    submenu = new wxMenu;
    menu->Append(IDM_VIEW_MOVETO, wxT("Move to"), submenu);
       submenu->Append(IDM_VIEW_ORBIT,   wxT("    Orbit\tSHIFT-U"));
       submenu->Append(IDM_VIEW_SURFACE, wxT("  Surface\tSHIFT-G"));
       submenu->Append(IDM_VIEW_SELECT,  wxT("Selection\tSHIFT-M"));
    submenu = new wxMenu;
    menu->Append(IDM_VIEW_ANIMATE, wxT("Animate"), submenu);
       submenu->Append(IDM_VIEW_AUTOTM, wxT("Time"));
       submenu->Append(IDM_VIEW_AUTOMV, wxT("Motion"));
    submenu = new wxMenu;
    menu->Append(IDM_VIEW_WSIZE, wxT("Window Size"), submenu);
       submenu->Append(IDM_VIEW_SIZE_SML, wxT("Small"));
       submenu->Append(IDM_VIEW_SIZE_MED, wxT("Medium"));
       submenu->Append(IDM_VIEW_SIZE_BIG, wxT("Big"));
       submenu->Append(IDM_VIEW_SIZE_WID, wxT("Wide"));
       submenu->Append(IDM_VIEW_SIZE_PAN, wxT("Panoramic"));
    submenu = new wxMenu;
    menu->Append(IDM_VIEW_WSIZE, wxT("Scene Quality"), submenu);
       submenu->Append(IDM_VIEW_LOD_LOW,  wxT("      Low\tSHIFT-D"));
       submenu->Append(IDM_VIEW_LOD_MED,  wxT("   Medium\tSHIFT-C"));
       submenu->Append(IDM_VIEW_LOD_HIGH, wxT("     High\tSHIFT-B"));
       submenu->Append(IDM_VIEW_LOD_BEST, wxT("Very High\tSHIFT-A"));
       submenu->AppendSeparator();
       submenu->Append(IDM_VIEW_LOD_MORE,  wxT("More Detail\t]"));
       submenu->Append(IDM_VIEW_LOD_LESS,  wxT("Less Detail\t["));

    menubar->Append(menu, wxT("&View"));

    // help menu
    menu = new wxMenu;
    menu->AppendCheckItem(IDM_HELP_FKEYS,
                              _T("key help &?"),
                              _T("Show/Hide Onscreen key help"));
    menu->Append(IDM_HELP_ABOUT, _T("&About..."), _T("Show about dialog"));
    menubar->Append(menu, wxT("&Help"));
    SetMenuBar(menubar);
 }
//-------------------------------------------------------------
// VtxFrame::make_tbar1() build toolbar1
//-------------------------------------------------------------
void VtxFrame::make_tbar1()
{
    #define INIT_TB1_BMP(bmp) toolBarBitmaps[TB1_##bmp] = wxBitmap(bmp##_xpm)

    enum
    {
        TB1_fileopen,
        TB1_filesave,
        TB1_vtx_image,
        TB1_vtx_view_image,
        TB1_vtx_movie,
        TB1_undo,
        TB1_redo,
        TB1_vtx_move_orbit,
        TB1_vtx_move_surface,
        TB1_vtx_move_select,
        TB1_vtx_autotm,
        TB1_vtx_automv,
        TB1_vtx_question,
	    TB1_objectdlg,
	    TB1_image_dialog,
        TB1_Max
    };

    wxBitmap toolBarBitmaps[TB1_Max];

    long style = wxTB_FLAT|wxTB_HORIZONTAL;

    m_tbar1 =CreateToolBar(style, ID_TOOLBAR1);

    wxSize size(16,15);

    //m_tbar1->SetMargins(4, 4);
    m_tbar1->SetToolBitmapSize(size);
    //m_tbar1->SetToolPacking(10);
    size=m_tbar1->GetToolSize();

    INIT_TB1_BMP(fileopen);
    m_tbar1->AddTool(IDM_FILE_SCENE_OPEN, _T("Open"),
        toolBarBitmaps[TB1_fileopen], _T("Open"), wxITEM_CHECK);

    INIT_TB1_BMP(filesave);
    m_tbar1->AddTool(IDM_FILE_SCENE_SAVE, _T("Save"),
        toolBarBitmaps[TB1_filesave], _T("Save"), wxITEM_CHECK);

    INIT_TB1_BMP(vtx_view_image);
    m_tbar1->AddTool(IDM_FILE_IMAGE_OPEN, _T("ViewImage"),
        toolBarBitmaps[TB1_vtx_view_image], _T("View Image"), wxITEM_CHECK);

    INIT_TB1_BMP(vtx_image);
    m_tbar1->AddTool(IDM_FILE_IMAGE_SAVE, _T("Snapshot"),
        toolBarBitmaps[TB1_vtx_image], _T("Save Image"), wxITEM_CHECK);

    INIT_TB1_BMP(undo);
    m_tbar1->AddTool(IDM_EDIT_UNDO, _T("Undo"),
        toolBarBitmaps[TB1_undo], _T("Undo"), wxITEM_CHECK);

    INIT_TB1_BMP(redo);
    m_tbar1->AddTool(IDM_EDIT_REDO, _T("Redo"),
        toolBarBitmaps[TB1_redo], _T("Redo"), wxITEM_CHECK);

    m_tbar1->AddSeparator();

    INIT_TB1_BMP(vtx_move_orbit);
    m_tbar1->AddTool(IDM_VIEW_ORBIT, _T("Orbit"),
        toolBarBitmaps[TB1_vtx_move_orbit], _T("Move to Orbit"), wxITEM_CHECK);

    INIT_TB1_BMP(vtx_move_surface);
    m_tbar1->AddTool(IDM_VIEW_SURFACE, _T("Surface"),
        toolBarBitmaps[TB1_vtx_move_surface], _T("Move to Surface"), wxITEM_CHECK);

    INIT_TB1_BMP(vtx_move_select);
    m_tbar1->AddTool(IDM_VIEW_SELECT, _T("Selection"),
        toolBarBitmaps[TB1_vtx_move_select], _T("Move to Selection"), wxITEM_CHECK);

    m_tbar1->AddSeparator();

    INIT_TB1_BMP(vtx_autotm);
    m_tbar1->AddTool(IDM_VIEW_AUTOTM, _T("Animate time"),
        toolBarBitmaps[TB1_vtx_autotm], _T("Time animation"), wxITEM_CHECK);

    INIT_TB1_BMP(vtx_automv);
    m_tbar1->AddTool(IDM_VIEW_AUTOMV, _T("Animate motion"),
        toolBarBitmaps[TB1_vtx_automv], _T("Motion animation"), wxITEM_CHECK);

    m_tbar1->AddSeparator();

    INIT_TB1_BMP(vtx_question);
    m_tbar1->AddTool(IDM_HELP_FKEYS, _T("Key help"),
        toolBarBitmaps[TB1_vtx_question], _T("Toggle On-Screen Key help"), wxITEM_CHECK);

    INIT_TB1_BMP(objectdlg);
    m_tbar1->AddTool(IDM_VIEW_OBJSDLG, _T("Scene Dialog"),
        toolBarBitmaps[TB1_objectdlg], _T("Open Scene Dialog"), wxITEM_CHECK);

    INIT_TB1_BMP(image_dialog);
    m_tbar1->AddTool(IDM_VIEW_IMAGEDLG, _T("Image Editor"),
        toolBarBitmaps[TB1_image_dialog], _T("Open Image Editor"), wxITEM_CHECK);

    INIT_TB1_BMP(vtx_movie);
    m_tbar1->AddTool(IDM_VIEW_MOVIEDLG, _T("Movie"),
        toolBarBitmaps[TB1_vtx_movie], _T("Edit Movie"), wxITEM_CHECK);


    m_tbar1->Realize();
}

//-------------------------------------------------------------
// VtxFrame::resize() resize window
//-------------------------------------------------------------
void VtxFrame::resize(wxSize size)
{
    SetClientSize(size.GetWidth()+FRAME_BORDER,size.GetHeight()+FRAME_BORDER);
}

//################# Event Handler functions ################

//-------------------------------------------------------------
// VtxFrame::OnExit() Exit from menu or close box
//-------------------------------------------------------------
void VtxFrame::OnClose( wxCloseEvent& WXUNUSED(event) )
{
	m_timer.Stop();
    m_canvas->quit();
    Destroy();
}

//-------------------------------------------------------------
// VtxFrame::OnExit() Exit from menu
//-------------------------------------------------------------
void VtxFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    //cout << "OnExit" << endl;
    //m_canvas->quit();
    Close(true);
}
//-------------------------------------------------------------
// VtxFrame::TimerTick() timer event
//-------------------------------------------------------------
void VtxFrame::OnTimer(wxTimerEvent& event)
{
    m_canvas->timer_tick();
}
//-------------------------------------------------------------
// VtxFrame::OnShowTB1() Show/Hide toolbar1
//-------------------------------------------------------------
void VtxFrame::OnShowTB1(wxCommandEvent& WXUNUSED(event))
{
    if (m_tbar1){
        delete m_tbar1;
        m_tbar1 = NULL;
        return;
    }
    make_tbar1();
}
//-------------------------------------------------------------
// VtxFrame::OnRebuild() rebuild scene
//-------------------------------------------------------------
void VtxFrame::OnRebuild(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->rebuild();
}
//-------------------------------------------------------------
// VtxFrame::OnClearViews() Clear view history
//-------------------------------------------------------------
void VtxFrame::OnClearViews(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->clear_views();
}
//-------------------------------------------------------------
// VtxFrame::OnOrbit() goto orbit
//-------------------------------------------------------------
void VtxFrame::OnOrbit(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_orbital_view();
    m_tbar1->ToggleTool(IDM_VIEW_ORBIT, false);
}
//-------------------------------------------------------------
// VtxFrame::OnSurface() goto surface
//-------------------------------------------------------------
void VtxFrame::OnSurface(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_surface_view();
    m_tbar1->ToggleTool(IDM_VIEW_SURFACE, false);
}
//-------------------------------------------------------------
// VtxFrame::OnAutotm() animate time
//-------------------------------------------------------------
void VtxFrame::OnAutotm(wxCommandEvent& WXUNUSED(event))
{
   m_canvas->set_autotm(!m_canvas->autotm());
}
//-------------------------------------------------------------
// VtxFrame::OnAutomv() animate motion
//-------------------------------------------------------------
void VtxFrame::OnAutomv(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_automv(!m_canvas->automv());
}
//-------------------------------------------------------------
// VtxFrame::OnSelect() move to selection
//-------------------------------------------------------------
void VtxFrame::OnSelect(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_MOVE_SELECT);
}
//-------------------------------------------------------------
// VtxFrame::OnUndo() undo
//-------------------------------------------------------------
void VtxFrame::OnUndo(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_DECR_VIEW);
}
//-------------------------------------------------------------
// VtxFrame::OnRedo() redo
//-------------------------------------------------------------
void VtxFrame::OnRedo(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_INCR_VIEW);
}
//-------------------------------------------------------------
// VtxFrame::OnSizeSmall() set small window size
//-------------------------------------------------------------
void VtxFrame::OnSizeSmall(wxCommandEvent& WXUNUSED(event))
{
    resize(wxSize(SMALL_W,SMALL_H));
}
//-------------------------------------------------------------
// VtxFrame::OnSizeMedium() set medium window size
//-------------------------------------------------------------
void VtxFrame::OnSizeMedium(wxCommandEvent& WXUNUSED(event))
{
    resize(wxSize(MED_W,MED_H));
}
//-------------------------------------------------------------
// VtxFrame::OnSizeBig() set large window size
//-------------------------------------------------------------
void VtxFrame::OnSizeBig(wxCommandEvent& WXUNUSED(event))
{
    resize(wxSize(BIG_W,BIG_H));
}
//-------------------------------------------------------------
// VtxFrame::OnSizeWide() set wide window size
//-------------------------------------------------------------
void VtxFrame::OnSizeWide(wxCommandEvent& WXUNUSED(event))
{
    resize(wxSize(WIDE_W,WIDE_H));
}
//-------------------------------------------------------------
// VtxFrame::OnSizePan() set panoramic window size
//-------------------------------------------------------------
void VtxFrame::OnSizePan(wxCommandEvent& WXUNUSED(event))
{
    resize(wxSize(PAN_W,PAN_H));
}
//-------------------------------------------------------------
// VtxFrame::OnLodLow() set low lod
//-------------------------------------------------------------
void VtxFrame::OnLodLow(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_ADAPT_LOW);
}
//-------------------------------------------------------------
// VtxFrame::OnLodMed() set medium lod
//-------------------------------------------------------------
void VtxFrame::OnLodMed(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_ADAPT_MED);
}
//-------------------------------------------------------------
// VtxFrame::OnLodHigh() set high lod
//-------------------------------------------------------------
void VtxFrame::OnLodHigh(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_ADAPT_HIGH);
}
//-------------------------------------------------------------
// VtxFrame::OnLodBest() set highest lod
//-------------------------------------------------------------
void VtxFrame::OnLodBest(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_ADAPT_BEST);
}
//-------------------------------------------------------------
// VtxFrame::OnLodMore() increase lod
//-------------------------------------------------------------
void VtxFrame::OnLodMore(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_MORE_DETAIL);
}
//-------------------------------------------------------------
// VtxFrame::OnLodless() decrease lod
//-------------------------------------------------------------
void VtxFrame::OnLodless(wxCommandEvent& WXUNUSED(event))
{
    m_canvas->set_key(KEY_LESS_DETAIL);
}
//-------------------------------------------------------------
// VtxFrame::OnHelpFkeys() toggle on-screen function key help
//-------------------------------------------------------------
void VtxFrame::OnHelpFkeys(wxCommandEvent& WXUNUSED(event))
{
    int helpon=m_canvas->fkey_help();
    m_canvas->set_fkey_help(!helpon);
}

//-------------------------------------------------------------
// VtxFrame::OnSceneOpen() Open a Scene File
//-------------------------------------------------------------
void VtxFrame::OnSceneOpen(wxCommandEvent& WXUNUSED(event))
{
	char filename[64];
    char dir[256];
    filename[0]=0;
 	m_canvas->suspend();
	File.getFilePath((char*)m_last_scene.ToAscii(),dir);
	File.getFileName((char*)m_last_scene.ToAscii(),filename);

    wxString ext("*");
    ext+=FileUtil::ext;
    wxFileDialog dialog
                 (
                    this,
                    _T("Open Scene File"),
                    wxString(dir),
                    wxString(filename),
                    ext,
                    wxFD_OPEN
                  );

    if (dialog.ShowModal() == wxID_OK) {
    	strcpy(filename, dialog.GetFilename().ToAscii());
        File.getFileName(filename,filename);

        m_last_scene=dialog.GetPath();//_T(dialog.GetPath().ToAscii());

#ifdef DEBUG_DLGS
        cout << "VtxFrame::OnSceneOpen()"<<endl;
        cout << "  Path:      " << dialog.GetPath().ToAscii() << endl;
        cout << "  Dir:       " << dialog.GetDirectory().ToAscii() << endl;
        cout << "  Filename:  " << filename << endl;
        cout << "  Ext:       " << ext << endl;
#endif
        m_canvas->open_scene((char*)m_last_scene.ToAscii());
    }
    m_tbar1->ToggleTool(IDM_FILE_SCENE_OPEN, false);
    m_canvas->unsuspend();
    wxString vtx("VTX-");
    vtx=vtx+filename;
    SetTitle(vtx);
}

//-------------------------------------------------------------
// VtxFrame::OnSceneSave() Save a scene
//-------------------------------------------------------------
void VtxFrame::OnSceneSave(wxCommandEvent& WXUNUSED(event))
{
	char filename[64];
    char dir[256];
    filename[0]=0;
 	m_canvas->suspend();
	File.getFilePath((char*)m_last_scene.ToAscii(),dir);
	File.getFileName((char*)m_last_scene.ToAscii(), filename);

    wxString ext("*");
    ext+=FileUtil::ext;
    wxFileDialog dialog
                 (
                    this,
                    _T("Save scene file"),
                    wxString(dir),
                    wxString(filename),
                    ext,
                    wxFD_SAVE
                  );
    if (dialog.ShowModal() == wxID_OK) {
        strcpy(filename, dialog.GetFilename().ToAscii());
        File.getFileName(filename,filename);

#ifdef DEBUG_DLGS
        cout << "VtxFrame::OnSceneSave()"<<endl;
        cout << "  Path:      " << dialog.GetPath().ToAscii() << endl;
        cout << "  Dir:       " << dialog.GetDirectory().ToAscii() << endl;
        cout << "  Filename:  " << filename << endl;
        cout << "  Ext:       " << ext << endl;
#endif
        m_last_scene=dialog.GetDirectory()+FileUtil::separator+filename+".spx";
        m_canvas->save_scene((char*)m_last_scene.ToAscii());
    }
    m_tbar1->ToggleTool(IDM_FILE_SCENE_SAVE, false);
    wxString vtx("VTX-");
    vtx=vtx+filename;
    SetTitle(vtx);
    m_canvas->unsuspend();
}

//-------------------------------------------------------------
// VtxFrame::OnImageOpen() view a screenshot
//-------------------------------------------------------------
void VtxFrame::OnImageOpen(wxCommandEvent& WXUNUSED(event) )
{
	char filename[64];
    char dir[256];
    filename[0]=0;
    m_timer.Stop();

 	m_canvas->suspend();
	File.getFilePath((char*)m_last_image.ToAscii(),dir);
	File.getFileName((char*)m_last_image.ToAscii(),filename);

    wxFileDialog dialog
                 (
                    this,
                    _T("Open Image File"),
                    wxString(dir),
                    wxString(filename),
                    _T("JPEG *.jpg|*.jpg|BMP *.bmp|*.bmp"),
                    wxFD_OPEN
                  );

    dialog.SetFilterIndex(m_last_itype==FILE_JPG?0:1);
    if (dialog.ShowModal() == wxID_OK) {
        strcpy(filename, dialog.GetFilename().ToAscii());
        File.getFileName(filename,filename);
        m_last_itype=dialog.GetFilterIndex()==0?FILE_JPG:FILE_BMP;
        //m_last_image=_T(dialog.GetPath().ToAscii());
        m_last_image=dialog.GetPath();
#ifdef DEBUG_DLGS
        const char *ext=(dialog.GetFilterIndex()==0)?".jpg":".bmp";
        cout << "VtxFrame::OnImageOpen()"<<endl;
        cout << "  Path:      " << dialog.GetPath().ToAscii() << endl;
        cout << "  Dir:       " << dialog.GetDirectory().ToAscii() << endl;
        cout << "  Filename:  " << filename << endl;
        cout << "  Ext:       " << ext << endl;
#endif
        m_canvas->view_image((char*)m_last_image.ToAscii(),m_last_itype);
    }
    ::wxMilliSleep(300);
    m_canvas->SwapBuffers();
    m_canvas->Refresh();
    m_canvas->Update();
    m_timer.Start(repeat_rate);
    m_tbar1->ToggleTool(IDM_FILE_IMAGE_OPEN, false);
    m_canvas->unsuspend();
}

//-------------------------------------------------------------
// VtxFrame::OnImageSave() Save a screenshot
//-------------------------------------------------------------
void VtxFrame::OnImageSave(wxCommandEvent& WXUNUSED(event) )
{
	m_timer.Stop();

	char filename[256];
    char dir[256];
    filename[0]=0;
 	m_canvas->suspend();
 	char path[256];
 	strcpy(path,m_last_image.ToAscii());
	File.getFilePath(path,dir);
	File.getFileName(path,filename);
    wxFileDialog dialog
                 (
                    this,
                    _T("Save image file"),
                    wxString(dir),
                    wxString(filename),
                    _T("JPEG *.jpg|*.jpg|BMP *.bmp|*.bmp"),
                    wxFD_SAVE
                  );
    dialog.SetFilterIndex(m_last_itype==FILE_JPG?0:1);
    if (dialog.ShowModal() == wxID_OK) {
        strcpy(filename, dialog.GetFilename().ToAscii());
        File.getFileName(filename,filename);
        m_last_itype=dialog.GetFilterIndex()==0?FILE_JPG:FILE_BMP;
        const char *ext=(dialog.GetFilterIndex()==0)?".jpg":".bmp";
#ifdef DEBUG_DLGS
        cout << "VtxFrame::OnImageSave()"<<endl;
        cout << "  Path:      " << dialog.GetPath().ToAscii() << endl;
        cout << "  Dir:       " << dialog.GetDirectory().ToAscii() << endl;
        cout << "  Filename:  " << filename << endl;
        cout << "  Ext:       " << ext << endl;
#endif
        strcpy(dir,dialog.GetDirectory().ToAscii());
        sprintf(path,"%s%s%s%s",dir,FileUtil::separator,filename,ext);
        m_last_image=wxString(path);
        m_canvas->save_image(path,m_last_itype);
    }
    m_tbar1->ToggleTool(IDM_FILE_IMAGE_SAVE, false);
    m_canvas->unsuspend();
    m_timer.Start(repeat_rate);
}

void VtxFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf( _T("VTX       A Virtual Terrain Exploration Tool\n")
                _T("Author    Dean Sindorf\n")
                _T("Version   2.2 1997-2023\n")
                _T("GUI      %s"), wxVERSION_STRING);
    wxMessageBox(msg, _T("About VTX"), wxOK | wxICON_INFORMATION, this);
}

//################# Update Handler functions ################

//-------------------------------------------------------------
// VtxFrame::OnUpdateUndo() Update undo gui
//-------------------------------------------------------------
void VtxFrame::OnUpdateUndo(wxUpdateUIEvent& event)
{
    event.Enable(m_canvas->can_undo_view());
    event.Check(false);    // unpress push-button
}
//-------------------------------------------------------------
// VtxFrame::OnUpdateUndo() Update redo gui
//-------------------------------------------------------------
void VtxFrame::OnUpdateRedo(wxUpdateUIEvent& event)
{
    event.Enable(m_canvas->can_redo_view());
    event.Check(false);   // unpress push-button
}
//-------------------------------------------------------------
// VtxFrame::OnUpdateSelect() update selection gui
//-------------------------------------------------------------
void VtxFrame::OnUpdateSelect(wxUpdateUIEvent& event)
{
    event.Enable(m_canvas->can_move_select());
    event.Check(false);  // unpress push-button
}
//-------------------------------------------------------------
// VtxFrame::OnUpdateAutotm() update autotm gui
//-------------------------------------------------------------
void VtxFrame::OnUpdateAutotm(wxUpdateUIEvent& event)
{
   // causes "can't check item" error in wxWidgets 3.1
   //   line 710 src/gtk/menu.cpp getKind() returns invalid type
   //   instead of wxITEM_CHECK
    event.Check(m_canvas->autotm());
}
//-------------------------------------------------------------
// VtxFrame::OnUpdateAutomv() update automv gui
//-------------------------------------------------------------
void VtxFrame::OnUpdateAutomv(wxUpdateUIEvent& event)
{
	// also causes "can't check item" error in wxWidgets 3.1
    event.Check(m_canvas->automv());
}

//-------------------------------------------------------------
// VtxFrame::OnObjsDlg()
//-------------------------------------------------------------
void VtxFrame::OnObjsDlg(wxCommandEvent& WXUNUSED(event))
{
	m_canvas->show_scene_dialog();
}

//-------------------------------------------------------------
// VtxFrame::OnUpdateObjsDlg()
//-------------------------------------------------------------
void VtxFrame::OnUpdateObjsDlg(wxUpdateUIEvent& event)
{
	event.Check(m_canvas->scene_dialog_showing());
}

//-------------------------------------------------------------
// VtxFrame::OnImageDlg()
//-------------------------------------------------------------
void VtxFrame::OnImageDlg(wxCommandEvent& WXUNUSED(event))
{
	m_canvas->show_image_dialog();
}

//-------------------------------------------------------------
// VtxFrame::OnUpdateImageDlg()
//-------------------------------------------------------------
void VtxFrame::OnUpdateImageDlg(wxUpdateUIEvent& event)
{
	event.Check(m_canvas->image_dialog_showing());
}

//-------------------------------------------------------------
// VtxFrame::OnMovieDlg()
//-------------------------------------------------------------
void VtxFrame::OnMovieDlg(wxCommandEvent& WXUNUSED(event))
{
	m_canvas->show_movie_dialog();
}

//-------------------------------------------------------------
// VtxFrame::OnUpdateMovieDlg()
//-------------------------------------------------------------
void VtxFrame::OnUpdateMovieDlg(wxUpdateUIEvent& event)
{
	event.Check(m_canvas->movie_dialog_showing());
}

//-------------------------------------------------------------
// VtxFrame::OnUpdateFkeys() update help gui
//-------------------------------------------------------------
void VtxFrame::OnUpdateFkeys(wxUpdateUIEvent& event)
{
    //cout << "Update()" <<endl;
    event.Check(m_canvas->fkey_help());
}
