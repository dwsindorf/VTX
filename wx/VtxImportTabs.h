#ifndef VTXIMPORTTABS_H_
#define VTXIMPORTTABS_H_
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <ImageClass.h>
#include <TextureClass.h>
#include <defs.h>

#include "VtxImageWindow.h"
#include "VtxControls.h"

class VtxImportTabs : public wxPanel
{

	DECLARE_CLASS(VtxImportTabs)

protected:
	void AddImageTab(wxPanel *panel);
	void makeImageList();
	void freeImageList();
	void displayImage(char *name);
	void getObjAttributes();
	void setObjAttributes();

	int type;
	wxComboBox *m_file_menu;
	wxString m_name;
	VtxImageWindow *m_image_window;
	wxTextCtrl *m_image_width;
	wxTextCtrl *m_image_height;
	bool update_needed;
	NameList<ImageSym*> *image_list;
	wxCheckBox *m_tile;


public:
	VtxImportTabs(wxWindow* parent,
			const int type,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxImportTabs();
	void updateControls();
	bool canDelete();
	bool canRevert()  { return false;}
	bool canSave()    { return false;}
    void Save() {}
    void Revert() {}
    void Delete();
    void Invalidate();

    void setSelection(wxString name);

    void OnFileSelect(wxCommandEvent& event);
    void OnShowMode(wxCommandEvent& event);
    void OnTile(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif /*IMAGETABS_H_*/
