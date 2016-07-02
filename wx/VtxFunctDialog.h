#ifndef FUNCTION_DLG_H_
#define FUNCTION_DLG_H_

#include "VtxFunctMgr.h"

class VtxExprEdit;
class TNode;

WX_DECLARE_HASH_MAP( int, VtxFunctMgr*, wxIntegerHash, wxIntegerEqual, VtxFunctMgrHash );

#define DLG_HEIGHT (380)
#define DLG_WIDTH  (460)
#define TABS_BORDER 10
#ifndef TABS_WIDTH
#define TABS_WIDTH  (DLG_WIDTH-20)
#define TABS_HEIGHT (DLG_HEIGHT-50)
#endif
class VtxFunctDialog : public wxFrame
{
	DECLARE_CLASS(VtxFunctDialog)
	wxPoint position;

	VtxFunctMgrHash   functs;

	wxCheckBox *m_show_symbols;
//	wxCheckBox *m_auto_apply;
//	wxCheckBox *m_auto_edit;

	wxBoxSizer* controlsSizer;
	int currentFunct;
	int lastFunct;
	
public:
	static bool show_symbols;
	static bool auto_apply;
	static bool auto_edit;
	wxString symbol;
	wxString value;
	VtxExprEdit *editor;
	VtxFunctDialog(wxWindow *parent,
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
	bool Show(const bool show);
	bool setEditor(VtxExprEdit *);
	bool setFunct(wxString, wxString);
	void getFunct(wxString);

	void clear();

	void OnUpdateSymbols(wxUpdateUIEvent& event);
//	void OnUpdateAutoApply(wxUpdateUIEvent& event);
//	void OnUpdateAutoEdit(wxUpdateUIEvent& event);
	void OnSymbols(wxCommandEvent& event);
//	void OnAutoApply(wxCommandEvent& event);
//	void OnAutoEdit(wxCommandEvent& event);
    DECLARE_EVENT_TABLE()
};

class VtxNoEditFunct : public VtxFunctMgr
{
protected:
public:
	VtxNoEditFunct(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);

	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);
};

extern VtxFunctDialog *functDialog;
extern VtxFunctDialog *noiseDialog;

#endif /*FUNCTION_DLG_H_*/
