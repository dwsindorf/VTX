#ifndef VTXIMAGETABS_H_
#define VTXIMAGETABS_H_
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

class VtxImageTabs : public wxPanel
{

	DECLARE_CLASS(VtxImageTabs)

protected:
    void setModified(bool);
    bool isModified();
	void AddImageTab(wxPanel *panel);
	void makeImageList();
	void freeGradientsList();
	void makeGradientsList();
	void freeImageList();
	void makeRevertList();
	void freeRevertList();
	void displayImage(char *name);
	void getObjAttributes();
	void setObjAttributes();
	void makeNewImage(char *, char *);
	bool Clone(wxString name,bool rename);

	wxChoice *m_file_menu;
	wxString m_name;
	VtxImageWindow *m_image_window;
	wxCheckBox *m_norm_check;
	wxCheckBox *m_invert_check;
	wxCheckBox *m_grays_check;
	wxCheckBox *m_show_tmps;

	ExprTextCtrl *m_image_expr;
	wxChoice *m_image_width;
	wxChoice *m_image_height;
	wxChoice *m_image_map;
	int type;
	bool update_needed;
	NameList<ImageSym*> *image_list;
	NameList<ImageSym*> *revert_list;
	
	VtxImageWindow *m_gradient_image;	
	wxCheckBox *m_gradient_check;
	NameList<ImageSym*> *gradient_list;
	wxChoice *m_gradient_file_menu;
	wxString m_gradient_name;

public:
	VtxImageTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxImageTabs();
	void updateControls();
	bool canDelete();
	bool canRevert();
	bool canSave();
    void Save();
    void Revert();
    void Delete();
    void setGradientName();
	bool New(wxString name);
	bool Rename(wxString name);

    void Invalidate();
    int rebuild();
    void setImageName(wxString s);
    void setImageName(char *n);
    char *getImageName();
   
    void setSelection(wxString name);
    wxString getSelection(){return m_file_menu->GetStringSelection();}

    void OnExprEdit(wxCommandEvent& event);
    void OnChanged(wxCommandEvent& event);
    void OnGradientMode(wxCommandEvent& event);
    void OnGraysMode(wxCommandEvent& event);
    void OnShowTmps(wxCommandEvent& event);

    void OnFileSelect(wxCommandEvent& event);
    void OnGradientSelect(wxCommandEvent& event);
    void OnImageSize(wxCommandEvent& event);

    wxString getImageString(wxString name);

	DECLARE_EVENT_TABLE()
};

#endif /*IMAGETABS_H_*/
