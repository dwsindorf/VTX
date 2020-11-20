#ifndef DFLTTABS_H_
#define DFTLTABS_H_


#include "VtxTabsMgr.h"
#include "SceneClass.h"

class VtxSurfaceTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxSurfaceTabs)
protected:
	void AddDisplayTab(wxWindow *panel);
	void getVarExpr();
	void getVariables();
	void deleteVar();
	void newVar();
	void changeVarName();
	void setVarExpr();
	void getObjAttributes();
	bool varExists(wxString s);

	TerrainMgr *getMgr();

	wxComboBox *m_list;
	ExprTextCtrl   *m_expr;
	wxButton	*m_newvar;
	wxButton	*m_rmvar;
	wxButton	*m_savevar;
	int last_selection;
	wxString m_varname;
	NodeIF *object() 	{ return object_node->node;}
	void invalidateObject(){
		object()->invalidate();
		TheScene->set_changed_detail();
		TheScene->rebuild();
	}

public:
	VtxSurfaceTabs(wxWindow* parent,
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
	~VtxSurfaceTabs(){
		delete m_expr;
	}
	int showMenu(bool);
	void updateControls();
	void OnVarSelect(wxCommandEvent& event){
		getVarExpr();
	}
    void OnChangeVarName(wxCommandEvent& event){
    	changeVarName();
    }
    void OnExprEdit(wxCommandEvent& event){
    	setVarExpr();
    }
    void OnNewVar(wxCommandEvent& event);
    void OnRmVar(wxCommandEvent& event);
    void OnSaveVar(wxCommandEvent& event);
    void OnUpdateBtns(wxUpdateUIEvent &event);

	DECLARE_EVENT_TABLE()
};


#endif /*SCENETABS_H_*/
