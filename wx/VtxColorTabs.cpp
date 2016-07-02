
#include "VtxColorTabs.h"
#include "FileUtil.h"

#include <wx/filefn.h>
#include <wx/dir.h>

#define STATE1D 0
#define STATE2D 1
//########################### VtxColorTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
    ID_RED,
    ID_GREEN,
    ID_BLUE,
    ID_ALPHA,
    ID_COL,
    ID_AUTO,
    ID_REVERT,
 };

#define LABEL1 40
#define VALUE1 50
#define SLIDER1 100

//########################### VtxColorTabs Class ########################

IMPLEMENT_CLASS(VtxColorTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxColorTabs, VtxTabsMgr)

EVT_TEXT_ENTER(ID_RED,VtxColorTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_GREEN,VtxColorTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_BLUE,VtxColorTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_ALPHA,VtxColorTabs::OnChangedExpr)
EVT_COLOURPICKER_CHANGED(ID_COL,VtxColorTabs::OnChangedColor)
EVT_CHECKBOX(ID_AUTO,VtxColorTabs::OnAuto)
EVT_BUTTON(ID_REVERT,VtxColorTabs::OnRevert)
EVT_MENU(ID_DELETE,VtxColorTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxColorTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxColorTabs::OnUpdateEnable)

END_EVENT_TABLE()

VtxColorTabs::VtxColorTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxColorTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	AddColorTab(page);
    AddPage(page,wxT("Color"),true);
    return true;
}

void VtxColorTabs::AddColorTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	m_r_expr = new ExprTextCtrl(panel,ID_RED,"",0,TABS_WIDTH-60-TABS_BORDER);
	m_g_expr = new ExprTextCtrl(panel,ID_GREEN,"",0,TABS_WIDTH-60-TABS_BORDER);
	m_b_expr = new ExprTextCtrl(panel,ID_BLUE,"",0,TABS_WIDTH-60-TABS_BORDER);
	m_a_expr = new ExprTextCtrl(panel,ID_ALPHA,"",0,TABS_WIDTH-60-TABS_BORDER);

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Red", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_r_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Green", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_g_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Blue", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_b_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Alpha", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_a_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	wxColor col("RGB(255,255,255)");
	wxStaticText *label=new wxStaticText(panel,-1,"Auto" ,wxDefaultPosition,wxSize(50,-1));
	hline->Add(label,0,wxALIGN_LEFT|wxALL,0);
	m_auto_check=new wxCheckBox(panel, ID_AUTO, "");
	hline->Add(m_auto_check,0,wxALIGN_LEFT|wxALL,4);
	m_chooser=new wxColourPickerCtrl(panel,ID_COL,col,wxDefaultPosition, wxSize(50,30));
	hline->Add(m_chooser,0,wxALIGN_LEFT|wxALL,4);
	m_revert=new wxButton(panel,ID_REVERT,"Revert",wxDefaultPosition, wxSize(50,32));
	hline->Add(m_revert,0,wxALIGN_LEFT|wxALL,4);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	revert_needed=false;
	m_revert->Enable(revert_needed);
}


int VtxColorTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));

	PopupMenu(&menu);
	return menu_action;
}

void VtxColorTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

void VtxColorTabs::OnChangedExpr(wxCommandEvent& event){
	setColorFromExpr();
	invalidateObject();
}
void VtxColorTabs::OnChangedColor(wxColourPickerEvent& event){
	if(m_auto){
		setExprFromColor();
		invalidateObject();
	}
}
void VtxColorTabs::OnAuto(wxCommandEvent& event){
	m_auto=m_auto_check->GetValue();
	if(m_auto){
		setExprFromColor();
		invalidateObject();
	}
}
void VtxColorTabs::OnRevert(wxCommandEvent& event){
	wxString new_expr=getExpr();
	restoreLastExpr();
	invalidateObject();
	setColorFromExpr();
	//revert_needed=false;
	//m_revert->Enable(revert_needed);
	m_last_expr=new_expr;
}
void VtxColorTabs::saveLastExpr(){
    m_last_expr=getExpr();
}
wxString VtxColorTabs::getExpr(){
	char red[128]="0.0";
	char green[128]="0.0";
	char blue[128]="0.0";
	char alpha[128]="1.0";
	if(strlen(m_r_expr->GetValue().ToAscii()))
		strcpy(red,m_r_expr->GetValue().ToAscii());
	if(strlen(m_g_expr->GetValue().ToAscii()))
		strcpy(green,m_g_expr->GetValue().ToAscii());
	if(strlen(m_b_expr->GetValue().ToAscii()))
		strcpy(blue,m_b_expr->GetValue().ToAscii());
	if(strlen(m_a_expr->GetValue().ToAscii()))
		strcpy(alpha,m_a_expr->GetValue().ToAscii());
	char cstr[256];
	sprintf(cstr,"Color(%s,%s,%s,%s)\n",red,green,blue,alpha);
    return wxString(cstr);
}

void VtxColorTabs::restoreLastExpr(){
	char cstr[256];
	strcpy(cstr,m_last_expr.ToAscii());
	if(strlen(cstr)){
		TNcolor *tnode=(TNcolor*)TheScene->parse_node(cstr);
		if(tnode){
			char red[128]={0};
			char green[128]={0};
			char blue[128]={0};
			char alpha[128]={0};
			TNarg &args=*((TNarg *)tnode->right);
			args[0]->valueString(red);
			args[1]->valueString(green);
			args[2]->valueString(blue);
			if(args[3])
				args[3]->valueString(alpha);
			else
				strcpy(alpha,"1.0");
			m_r_expr->SetValue(red);
			m_g_expr->SetValue(green);
			m_b_expr->SetValue(blue);
			m_a_expr->SetValue(alpha);
			delete tnode;
		}
	}
}

void VtxColorTabs::setColorFromExpr(){
	wxString expr=getExpr();
	TNcolor *tnode=(TNcolor*)TheScene->parse_node(expr.ToAscii());
	if(tnode){
		TNarg &args=*((TNarg *)tnode->right);
		args[0]->eval();
		bool cflag=S0.constant()?true:false;
		unsigned char r=(unsigned char)(S0.s*255);
		args[1]->eval();
		cflag=S0.constant()?cflag:false;
		unsigned char g=(unsigned char)(S0.s*255);
		args[2]->eval();
		cflag=S0.constant()?cflag:false;
		unsigned char b=(unsigned char)(S0.s*255);
		if(args[3]){
			args[3]->eval();
			cflag=S0.constant()?cflag:false;
		}

		m_auto=cflag;
		wxColor col(r,g,b);
		m_chooser->SetColour(col);
		delete tnode;
	}
	m_auto_check->SetValue(m_auto);
}

void VtxColorTabs::setExprFromColor(){
	char red[128]={0};
	char green[128]={0};
	char blue[128]={0};
	wxColor col=m_chooser->GetColour();
	Color c;
	c.set_rb(col.Red());
	c.set_gb(col.Green());
	c.set_bb(col.Blue());
	sprintf(red,"%g",c.red());
	sprintf(green,"%g",c.green());
	sprintf(blue,"%g",c.blue());
	m_r_expr->SetValue(red);
	m_g_expr->SetValue(green);
	m_b_expr->SetValue(blue);
	m_a_expr->SetValue("1.0");
}

//-------------------------------------------------------------
// VtxColorTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxColorTabs::setObjAttributes(){
	update_needed=true;
	TNcolor *tnode=object();
	tnode->setExpr(getExpr().ToAscii());
	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
		revert_needed=true;
		m_revert->Enable(revert_needed);
	}
}
//-------------------------------------------------------------
// VtxColorTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxColorTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNcolor *tnode=object();

	char red[128]={0};
	char green[128]={0};
	char blue[128]={0};
	char alpha[128]={0};

	TNarg &args=*((TNarg *)tnode->right);

	args[0]->valueString(red);
	args[1]->valueString(green);
	args[2]->valueString(blue);
	if(args[3])
		args[3]->valueString(alpha);
	else
		strcpy(alpha,"1.0");

	m_r_expr->SetValue(red);
	m_g_expr->SetValue(green);
	m_b_expr->SetValue(blue);
	m_a_expr->SetValue(alpha);

	setColorFromExpr();
	update_needed=false;
	saveLastExpr();

}
