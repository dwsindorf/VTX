#ifndef POINTTABS_H_
#define POINTTABS_H_

#include "VtxTabsMgr.h"

#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <TerrainClass.h>
#include <defs.h>


class VtxPointTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxPointTabs)

protected:
	TNpoint *object() 	{ return (TNpoint *)object_node->node;}
	void AddPointTab(wxWindow *panel);
    void invalidateObject(){
    	object()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();
	ExprTextCtrl *m_x_expr;
	ExprTextCtrl *m_y_expr;
	ExprTextCtrl *m_z_expr;
public:
	VtxPointTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxPointTabs(){
		delete m_x_expr;
		delete m_y_expr;
		delete m_z_expr;
	}
	bool Create(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	void updateControls();
	int showMenu(bool);
    void OnXEnter(wxCommandEvent& event){
    	invalidateObject();
    }
    void OnYEnter(wxCommandEvent& event){
    	invalidateObject();
    }
    void OnZEnter(wxCommandEvent& event){
    	invalidateObject();
    }
    void OnEnable(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};
#endif /*COLORTABS_H_*/
