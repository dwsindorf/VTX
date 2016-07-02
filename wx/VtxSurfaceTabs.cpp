
#include "VtxSurfaceTabs.h"
#include "RenderOptions.h"
#include "VtxSceneDialog.h"

//########################### VtxSurfaceTabs Class ########################

// define all resource ids here
enum {
	ID_SAVE,
	ID_VARLIST,
	ID_VAREXPR,
};

#define LIST_WIDTH 80

IMPLEMENT_CLASS(VtxSurfaceTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxSurfaceTabs, wxNotebook)
EVT_MENU(TABS_ENABLE,VtxSurfaceTabs::OnEnable)

EVT_COMBOBOX(ID_VARLIST,VtxSurfaceTabs::OnVarSelect)
EVT_TEXT_ENTER(ID_VARLIST,VtxSurfaceTabs::OnChangeVarName)
EVT_TEXT_ENTER(ID_VAREXPR,VtxSurfaceTabs::OnExprEdit)
//EVT_MENU(ID_SAVE,VtxSurfaceTabs::OnSave)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxSurfaceTabs::OnAddItem)

SET_FILE_EVENTS(VtxSurfaceTabs)

END_EVENT_TABLE()

VtxSurfaceTabs::VtxSurfaceTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxSurfaceTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);

    AddDisplayTab(page);
    AddPage(page,wxT("Controls"),true);

    return true;
}

int VtxSurfaceTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;
	wxMenu menu;


	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		menu.AppendSubMenu(addmenu,"Add");
	}
	sceneDialog->AddFileMenu(menu,object_node->node);

	//menu.Append(ID_SAVE,wxT("Save.."));
	PopupMenu(&menu);
	return menu_action;
}

void VtxSurfaceTabs::AddDisplayTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls
    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxBoxSizer* variables = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Variables"));

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
    m_list=new wxComboBox(panel,ID_VARLIST,"",
    			wxDefaultPosition,wxSize(LIST_WIDTH,-1),0, NULL, wxCB_SORT|wxTE_PROCESS_ENTER);
    hline->Add(m_list,0,wxALIGN_LEFT|wxALL,0);

    m_expr=new ExprTextCtrl(panel,ID_VAREXPR,"",0,TABS_WIDTH-TABS_BORDER-LIST_WIDTH);

    hline->Add(m_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
    variables->Add(hline,0,wxALIGN_LEFT|wxALL,0);
    variables->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
    boxSizer->Add(variables, 0, wxALIGN_LEFT|wxALL, 0);
}

//-------------------------------------------------------------
// VtxSurfaceTabs::updateControls() update widgets
//-------------------------------------------------------------
void VtxSurfaceTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

//-------------------------------------------------------------
// VtxSurfaceTabs::getMgr() return terrain manager
//-------------------------------------------------------------
TerrainMgr *VtxSurfaceTabs::getMgr()
{
	NodeIF *obj=object();
	NodeIF *mgr=obj;
	while(mgr && !(mgr->typeValue() & ID_TMGR))
		mgr=mgr->getParent();

	return (TerrainMgr*)mgr;
}

//-------------------------------------------------------------
// VtxSurfaceTabs::getObjAttributes() set controls from object
//-------------------------------------------------------------
void VtxSurfaceTabs::getObjAttributes(){
	if(!update_needed)
		return;
	getVariables();
	update_needed=false;
}

//-------------------------------------------------------------
// VtxSurfaceTabs::getVariables() get Terrain variables
//-------------------------------------------------------------
void VtxSurfaceTabs::getVariables()
{
	TerrainMgr *mgr=getMgr();
	if(!mgr)
		return;
	CurrentScope=mgr;
	m_list->Clear();
	int index=mgr->getVarIndex();

	Node<TNode*> *ptr=mgr->exprs.ptr;
	mgr->exprs.ss();
	TNvar *obj;
	while((obj=(TNvar*)mgr->exprs++)>0){
		m_list->Append(obj->name());
	}
	if(mgr->exprs.size>0)
		m_list->SetSelection(index);
	m_varname=m_list->GetStringSelection();
	getVarExpr();
	mgr->exprs.ptr=ptr;
}
//-------------------------------------------------------------
// VtxSurfaceTabs::getVarExpr() get variable expression from name
//-------------------------------------------------------------
void VtxSurfaceTabs::getVarExpr()
{
	TerrainMgr *mgr=getMgr();
	if(!mgr)
	    return;
	wxString name = m_list->GetStringSelection();
	char buff[1024];
	buff[0]=0;
    if(mgr->getVarExpr(name.ToAscii(),buff)){
    	mgr->setVarIndex(m_list->GetSelection());
     	m_expr->SetValue(buff);
    }
}

//-------------------------------------------------------------
// VtxSurfaceTabs::setVarExpr() change var expression
//-------------------------------------------------------------
void VtxSurfaceTabs::setVarExpr()
{
	TerrainMgr *mgr=getMgr();

    if(!mgr|| m_list->IsEmpty())
    	return;
    wxString name = m_list->GetStringSelection();
	wxString expr = m_expr->GetValue();
    if(expr.IsEmpty())
    	deleteVar();
    else{
    	//cout << "VtxSurfaceTabs::setVarExpr:" << expr << endl;
		mgr->setVarExpr(name.ToAscii(),expr.ToAscii());
		mgr->applyVarExprs();
		invalidateObject();
    }
}

//-------------------------------------------------------------
// VtxSurfaceTabs::changeVarName() change current variable name
//-------------------------------------------------------------
void VtxSurfaceTabs::changeVarName()
{
	wxString name = m_list->GetValue();
	if(name.IsEmpty())
		deleteVar();
	else{
		newVar();
	}
}
//-------------------------------------------------------------
// VtxSurfaceTabs::deleteVar() delete current variable
//-------------------------------------------------------------
void VtxSurfaceTabs::deleteVar()
{
	cout << "delete var:" << m_varname<< endl;
	TerrainMgr *mgr=getMgr();

	if(!mgr)
	    return;

	TNvar *vnode=mgr->removeVar(m_varname.ToAscii());
	if(vnode)
		delete vnode;

	int i = m_list->GetSelection();
	if(i != wxNOT_FOUND){
		m_list->Delete(i);
		if(!m_list->IsEmpty())
			m_list->SetSelection(0);
	}
	invalidateObject();
}
//-------------------------------------------------------------
// VtxSurfaceTabs::newVar() create new variable
//-------------------------------------------------------------
void VtxSurfaceTabs::newVar()
{
	wxString name = m_list->GetValue();

	TerrainMgr *mgr=getMgr();
	if(!mgr)
	    return;

	TNvar *obj;
	obj=mgr->getVar(name.ToAscii());
	if(obj){
		return; // var exists
	}

	wxString expr = m_expr->GetValue();

	obj=mgr->setVar(name.ToAscii(),expr.ToAscii());
	if(!obj) // expr bad
		return;

	m_list->Append(name);
	int i=m_list->GetCount()-1;
	mgr->setVarIndex(i);
	m_list->SetSelection(i);
	m_varname=m_list->GetStringSelection();
}

