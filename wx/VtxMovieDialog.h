/*
 * VtxMovieDialog.h
 *
 *  Created on: Oct 7, 2020
 *      Author: dean
 */

#ifndef WX_VTXMOVIEDIALOG_H_
#define WX_VTXMOVIEDIALOG_H_

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/notebook.h>
#include <wx/tglbtn.h>
#ifndef WIN32
	#include <wx/gtk/frame.h>
#endif

#define DLG_HEIGHT (380)
#define DLG_WIDTH  (440)
#define TABS_BORDER 10
#ifndef TABS_WIDTH
#define TABS_WIDTH  (DLG_WIDTH-20)
#define TABS_HEIGHT (DLG_HEIGHT-50)
#endif

class VtxMovieDialog: public wxFrame {

	DECLARE_CLASS(VtxMovieDialog)
	wxPoint position;

	wxNotebook   *m_tabs;
	wxButton *m_save_path;
	wxButton *m_load_path;

	wxButton *m_clear;
	wxButton *m_rewind;

	wxButton *m_record_path;
	wxColour m_button_color;
	wxColour m_pressed_color;

	wxButton *m_play_path;

	wxButton *m_record_movie;
	wxButton *m_test_movie;
	wxButton *m_save_movie;
	wxButton *m_play_movie;

    wxString   m_last_path;
    wxString   m_last_movie;
    int        m_last_ext;


	void setTitle();

    void Invalidate();
    void UpdateControls();
    static wxWindow *test(wxWindow *);
    void Stop();
public:
	VtxMovieDialog(wxWindow *parent,
			wxWindowID id= wxID_ANY,
			const wxString &caption= wxT(""),
			const wxPoint &pos=wxDefaultPosition,
			const wxSize & size= wxDefaultSize,
			long style = wxCAPTION|wxSYSTEM_MENU
			);

	bool Create( wxWindow* parent,
			wxWindowID id = wxID_ANY,
			const wxString& caption = wxT(""),
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxCAPTION|wxSYSTEM_MENU );

	void OnClose(wxCloseEvent& event);

    void OnOk(wxCommandEvent &event);
    void OnClear(wxCommandEvent &event);
    void OnLoadPath(wxCommandEvent &event);

    void OnRecordPath(wxCommandEvent &event);
    void OnStop(wxCommandEvent &event);
    void OnRewind(wxCommandEvent &event);
    void OnPlayPath(wxCommandEvent &event);
    void OnSavePath(wxCommandEvent &event);

    void OnRecordMovie(wxCommandEvent &event);
    void OnSaveMovie(wxCommandEvent &event);
    void OnTestMovie(wxCommandEvent &event);
    void OnPlayMovie(wxCommandEvent &event);
    void OnTestCheck(wxCommandEvent &event);

    void OnUpdateRecordMovie(wxUpdateUIEvent& event);
    void OnUpdateRecordPath(wxUpdateUIEvent& event);
    void OnUpdateTestMovie(wxUpdateUIEvent& event);
    void OnUpdateSaveMovie(wxUpdateUIEvent& event);
    void OnUpdatePlayPath(wxUpdateUIEvent& event);
    void OnUpdateSavePath(wxUpdateUIEvent& event);
    void OnUpdateRewind(wxUpdateUIEvent& event);

    bool Show(const bool show);

	DECLARE_EVENT_TABLE()
};

extern VtxMovieDialog *movieDialog;

#endif /* WX_VTXMOVIEDIALOG_H_ */
