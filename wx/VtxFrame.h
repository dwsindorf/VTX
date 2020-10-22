#ifndef _VTX_FRAME_H_
#define _VTX_FRAME_H_

#include "wx/glcanvas.h"
#include "VtxMain.h"
#include "VtxScene.h"

class VtxFrame: public wxFrame
{
public:
    VtxFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
    void start_timer(int t);
    void set_status(char *c,int);
    void swap_buffers();

    void OnClose(wxCloseEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnShowTB1(wxCommandEvent& event);
    void OnClearViews(wxCommandEvent& event);
    void OnRebuild(wxCommandEvent& event);
    void OnSceneOpen(wxCommandEvent& event);
    void OnSceneSave(wxCommandEvent& event);
    void OnImageSave(wxCommandEvent& event);
    void OnImageOpen(wxCommandEvent& event);
    void OnGlobal(wxCommandEvent& event);
    void OnSurface(wxCommandEvent& event);
    void OnOrbit(wxCommandEvent& event);
    void OnSelect(wxCommandEvent& event);
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnAutotm(wxCommandEvent& event);
    void OnAutomv(wxCommandEvent& event);
    void OnHelpFkeys(wxCommandEvent& event);
    void OnSizeSmall(wxCommandEvent& event);
    void OnSizeMedium(wxCommandEvent& event);
    void OnSizeBig(wxCommandEvent& event);
    void OnSizeWide(wxCommandEvent& event);
    void OnSizePan(wxCommandEvent& event);
    void OnLodLow(wxCommandEvent& event);
    void OnLodMed(wxCommandEvent& event);
    void OnLodHigh(wxCommandEvent& event);
    void OnLodBest(wxCommandEvent& event);
    void OnLodMore(wxCommandEvent& event);
    void OnLodless(wxCommandEvent& event);

    void OnUpdateUndo(wxUpdateUIEvent& event);
    void OnUpdateRedo(wxUpdateUIEvent& event);
    void OnUpdateSelect(wxUpdateUIEvent& event);
    void OnUpdateAutotm(wxUpdateUIEvent& event);
    void OnUpdateAutomv(wxUpdateUIEvent& event);
    void OnUpdateFkeys(wxUpdateUIEvent& event);

    void OnObjsDlg(wxCommandEvent& event);
    void OnUpdateObjsDlg(wxUpdateUIEvent& event);
    void OnImageDlg(wxCommandEvent& event);
    void OnUpdateImageDlg(wxUpdateUIEvent& event);
    void OnMovieDlg(wxCommandEvent& event);
    void OnUpdateMovieDlg(wxUpdateUIEvent& event);

//#if wxUSE_GLCANVAS
    void SetCanvas( VtxScene *canvas ) { m_canvas = canvas; }
    VtxScene *GetCanvas() { return m_canvas; }
private:
    wxTimer    m_timer;
    VtxScene  *m_canvas;
    wxToolBar *m_tbar1;
    wxString   m_last_image;
    wxString   m_last_scene;
    int        m_last_itype;
    void make_tbar1();
    void make_menubar();
    void resize(wxSize);
//#endif
    DECLARE_EVENT_TABLE()
};

#endif

