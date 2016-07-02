#ifndef COLORTABS_H_
#define COLORTABS_H_

#include "VtxTabsMgr.h"

#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <wx/clrpicker.h>
#include <TerrainClass.h>
#include <defs.h>


class VtxColorTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxColorTabs)

protected:
	TNcolor *object() 	{ return (TNcolor *)object_node->node;}
	void AddColorTab(wxWindow *panel);
    void invalidateObject(){
    	object()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();
	ExprTextCtrl *m_r_expr;
	ExprTextCtrl *m_g_expr;
	ExprTextCtrl *m_b_expr;
	ExprTextCtrl *m_a_expr;
	wxColourPickerCtrl *m_chooser;
	wxCheckBox *m_auto_check;
	wxButton *m_revert;
	bool m_auto;
	bool revert_needed;
	void setColorFromExpr();
	void setExprFromColor();
	wxString getExpr();
	void saveLastExpr();
	void restoreLastExpr();
	wxString m_last_expr;

public:
	VtxColorTabs(wxWindow* parent,
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

	~VtxColorTabs()
	{
		delete m_r_expr;
		delete m_g_expr;
		delete m_b_expr;
		delete m_a_expr;
	}
	void updateControls();
	int showMenu(bool);
    void OnChangedExpr(wxCommandEvent& event);
    void OnAuto(wxCommandEvent& event);
    void OnRevert(wxCommandEvent& event);
    void OnChangedColor(wxColourPickerEvent& event);
	DECLARE_EVENT_TABLE()
};
#endif /*COLORTABS_H_*/
