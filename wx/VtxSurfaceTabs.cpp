
#include "VtxSurfaceTabs.h"
#include "RenderOptions.h"
#include "VtxSceneDialog.h"
#include "VtxTabsMgr.h"

//########################### VtxSurfaceTabs Class ########################

// define all resource ids here
enum {
	ID_VARLIST,
	ID_VAREXPR,
	ID_NEWVAR,
	ID_RMVAR,
	ID_SAVEVAR,

};

#define LIST_WIDTH 80
#define BTNWIDTH 75
#define BTNSIZE wxSize(BTNWIDTH,30)


IMPLEMENT_CLASS(VtxSurfaceTabs, wxNotebook )


BEGIN_EVENT_TABLE(VtxSurfaceTabs, wxNotebook)
EVT_MENU(TABS_ENABLE,VtxSurfaceTabs::OnEnable)

EVT_COMBOBOX(ID_VARLIST,VtxSurfaceTabs::OnVarSelect)
//EVT_TEXT_ENTER(ID_VARLIST,VtxSurfaceTabs::OnChangeVarName)
EVT_TEXT_ENTER(ID_VAREXPR,VtxSurfaceTabs::OnExprEdit)
EVT_BUTTON(ID_NEWVAR,VtxSurfaceTabs::OnNewVar)
EVT_BUTTON(ID_RMVAR,VtxSurfaceTabs::OnRmVar)
EVT_BUTTON(ID_SAVEVAR,VtxSurfaceTabs::OnSaveVar)
EVT_UPDATE_UI(ID_RMVAR,  VtxSurfaceTabs::OnUpdateBtns)
EVT_UPDATE_UI(ID_SAVEVAR,  VtxSurfaceTabs::OnUpdateBtns)

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

    hline = new wxBoxSizer(wxHORIZONTAL);

 	m_newvar=new wxButton(panel,ID_NEWVAR,"New",wxDefaultPosition,BTNSIZE);
 	m_rmvar=new wxButton(panel,ID_RMVAR,"Delete",wxDefaultPosition,BTNSIZE);
 	m_savevar=new wxButton(panel,ID_SAVEVAR,"Save",wxDefaultPosition,BTNSIZE);
 	hline->Add(m_newvar,0,wxALIGN_LEFT|wxALL,0);
 	hline->Add(m_rmvar,0,wxALIGN_LEFT|wxALL,0);
 	hline->Add(m_savevar,0,wxALIGN_LEFT|wxALL,0);

 	variables->Add(hline, 0, wxALIGN_LEFT|wxALL, 0);

    variables->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
    boxSizer->Add(variables, 0, wxALIGN_LEFT|wxALL, 0);

}
void VtxSurfaceTabs::OnUpdateBtns(wxUpdateUIEvent &event) {
	m_rmvar->Enable(!m_list->IsListEmpty());
	m_savevar->Enable(!m_list->IsListEmpty());
}

void VtxSurfaceTabs::OnNewVar(wxCommandEvent &event) {
	wxString name = m_list->GetValue();
	if(varExists(name)){
		cout << name << "is already a variable"<<endl;
		return;
	}

	m_varname=name;
	TerrainMgr *mgr=getMgr();
	if(!mgr)
		 return;

	cout << "new var:" << m_varname<< endl;
    if(m_expr->GetValue().IsEmpty())
    	m_expr->SetValue("0");
	TNvar *obj=mgr->setVar((char*)m_varname.ToAscii(),(char*)m_expr->GetValue().ToAscii());
	if(!obj){ // expr bad
		cout<<"bad expression"<<endl;
		return;
	}

	int i=m_list->Append(m_varname);
	//update_needed=true;
	mgr->setVarIndex(i);
	m_list->SetSelection(i);
	m_varname=m_list->GetStringSelection();
	update_needed=true;
	invalidateObject();
	getObjAttributes();

}

void VtxSurfaceTabs::OnRmVar(wxCommandEvent &event) {
	deleteVar();
}

void VtxSurfaceTabs::OnSaveVar(wxCommandEvent &event) {
	cout<<"save var"<<endl;
	setVarExpr();
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
	while((obj=(TNvar*)mgr->exprs++)){
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
    if(mgr->getVarExpr((char*)name.ToAscii(),buff)){
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

    if(!mgr)
    	return;
    if(m_list->IsListEmpty())
    	return;
    wxString name = m_list->GetStringSelection();
	wxString expr = m_expr->GetValue();
    if(!expr.IsEmpty()){
    //	deleteVar();
   //else{
		mgr->setVarExpr((char*)name.ToAscii(),(char*)expr.ToAscii());
		mgr->applyVarExprs();
    }
	update_needed=true;
	invalidateObject();
	getObjAttributes();
}

//-------------------------------------------------------------
// VtxSurfaceTabs::changeVarName() change current variable name
//-------------------------------------------------------------
void VtxSurfaceTabs::changeVarName()
{
	cout <<"changeVarName"<<endl;
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
	TerrainMgr *mgr=getMgr();

	if(!mgr)
	    return;
	cout << "delete var:" << m_varname<< endl;

	TNvar *vnode=mgr->removeVar((char*)m_varname.ToAscii());
	if(vnode)
		delete vnode;

	int i = m_list->GetSelection();
	if(i==wxNOT_FOUND)
		return;

	m_list->Delete(i);
	if(m_list->IsListEmpty()){
		m_expr->SetValue("");
		m_varname="";
		m_list->Clear();
	}
	else{
		m_list->SetSelection(0);
		getVarExpr();
	}

	update_needed=true;
	invalidateObject();
	getObjAttributes();
}

//-------------------------------------------------------------
// varExists::varExists() returb true if var exists
//-------------------------------------------------------------
bool VtxSurfaceTabs::varExists(wxString s){
	TerrainMgr *mgr=getMgr();
	if(!mgr)
	    return false;
	TNvar *obj;
	obj=mgr->getVar((char*)s.ToAscii());
	if(obj)
		return true; // var exists

	return false;
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
	obj=mgr->getVar((char*)name.ToAscii());
	if(obj){
		return; // var exists
	}

	wxString expr = m_expr->GetValue();

	obj=mgr->setVar((char*)name.ToAscii(),(char*)expr.ToAscii());
	if(!obj) // expr bad
		return;

	m_list->Append(name);
	int i=m_list->GetCount()-1;
	mgr->setVarIndex(i);
	m_list->SetSelection(i);
	m_varname=m_list->GetStringSelection();
}

