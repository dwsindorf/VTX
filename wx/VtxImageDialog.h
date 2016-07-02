#ifndef IMAGEDIALOG_H_
#define IMAGEDIALOG_H_

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/colour.h>
#include <wx/clrpicker.h>
#include <wx/radiobox.h>
#include <wx/button.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <wx/gdicmn.h>
#include <wx/notebook.h>
#include "VtxControls.h"
#include "VtxImageTabs.h"
#include "VtxImportTabs.h"
#include "VtxBandsTabs.h"
#include "SceneClass.h"

#define TYPE_BANDS 0
#define TYPE_IMAGE 1
#define TYPE_IMPORT 2

#define DLG_HEIGHT (380)
#define DLG_WIDTH  (440)
#define TABS_BORDER 10
#ifndef TABS_WIDTH
#define TABS_WIDTH  (DLG_WIDTH-20)
#define TABS_HEIGHT (DLG_HEIGHT-50)
#endif
class VtxImageDialog : public wxFrame
{
	DECLARE_CLASS(VtxImageDialog)

	wxPoint position;

	wxNotebook   *m_tabs;
	VtxImageTabs *m_image_tabs;
	VtxBandsTabs *m_bands_tabs;
	VtxImportTabs *m_import_tabs;

	wxButton *m_save;
	wxButton *m_revert;
	wxButton *m_delete;

	//int m_tab_selected;
	int m_type;

	void setTitle();
	void updateControls();

public:
	VtxImageDialog(wxWindow *parent,
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

	//--------- event handlers ------------------------------

    // implemented in cpp file

    bool Show(const bool show);
    bool Show(wxString name, int d2);
    void OnOk(wxCommandEvent &event);
    void OnTabSwitch(wxNotebookEvent &event);
    void OnSave(wxCommandEvent &event);
    void OnRevert(wxCommandEvent &event);
    void OnDelete(wxCommandEvent &event);

    void Invalidate();

    void UpdateControls();

    DECLARE_EVENT_TABLE()
};

extern VtxImageDialog *imageDialog;

#endif /*IMAGEDIALOG_H_*/
