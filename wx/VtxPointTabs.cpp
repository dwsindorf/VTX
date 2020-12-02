
#include "VtxPointTabs.h"
#include "FileUtil.h"

#include <wx/filefn.h>
#include <wx/dir.h>

#define STATE1D 0
#define STATE2D 1
//########################### VtxPointTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
    ID_X,
    ID_Y,
    ID_Z,
 };

#define LABEL1 40
#define VALUE1 50
#define SLIDER1 100

//########################### VtxPointTabs Class ########################

IMPLEMENT_CLASS(VtxPointTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxPointTabs, VtxTabsMgr)

EVT_TEXT_ENTER(ID_X,VtxPointTabs::OnXEnter)
EVT_TEXT_ENTER(ID_Y,VtxPointTabs::OnYEnter)
EVT_TEXT_ENTER(ID_Z,VtxPointTabs::OnZEnter)

EVT_MENU(ID_DELETE,VtxPointTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxPointTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxPointTabs::OnUpdateEnable)

END_EVENT_TABLE()

VtxPointTabs::VtxPointTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxPointTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	AddPointTab(page);
    AddPage(page,wxT("Point"),true);
    return true;
}

void VtxPointTabs::AddPointTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	m_x_expr = new ExprTextCtrl(panel,ID_X,"",0,TABS_WIDTH-60-TABS_BORDER);
	m_y_expr = new ExprTextCtrl(panel,ID_Y,"",0,TABS_WIDTH-60-TABS_BORDER);
	m_z_expr = new ExprTextCtrl(panel,ID_Z,"",0,TABS_WIDTH-60-TABS_BORDER);

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "X", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_x_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Y", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_y_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Z", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_z_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);
}

int VtxPointTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));

	PopupMenu(&menu);
	return menu_action;
}

void VtxPointTabs::updateControls(){
	//if(update_needed){
		getObjAttributes();
	//}
}
//-------------------------------------------------------------
// VtxPointTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxPointTabs::setObjAttributes(){
	update_needed=true;
	char p[1024];
	TNpoint *tnode=object();
	char x[1024];
	char y[1024];
	char z[1024];
	strcpy(x,m_x_expr->GetValue().ToAscii());
	strcpy(y,m_y_expr->GetValue().ToAscii());
	strcpy(z,m_z_expr->GetValue().ToAscii());
	if(strlen(x)>0||strlen(y)>0)
		sprintf(p,"Point(%s,%s,%s)\n",x,y,z);
	else
		sprintf(p,"Z(%s)\n",z);
	cout<<p<<endl;
	tnode->setExpr(p);
	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
	}
}
//-------------------------------------------------------------
// VtxPointTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxPointTabs::getObjAttributes(){
	//if(!update_needed)
	//	return;
	TNpoint *tnode=object();

	char x[1024]={0};
	char y[1024]={0};
	char z[1024]={0};

	TNarg &args=*((TNarg *)tnode->right);
	INIT;
	if(!args[1]&& !args[2])
		args[0]->valueString(z);
	else{
		args[0]->valueString(x);
		args[1]->valueString(y);
		args[2]->valueString(z);

	}
	m_x_expr->SetValue(x);
	m_y_expr->SetValue(y);
	m_z_expr->SetValue(z);
	update_needed=false;

}
