
#include "VtxHardnessTabs.h"

#include <wx/filefn.h>
#include <wx/dir.h>

#define STATE1D 0
#define STATE2D 1
//########################### VtxHardnessTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
    ID_VALUE_SLDR,
    ID_VALUE_TEXT,
  };

#define LABEL1 40
#define VALUE1 50
#define SLIDER1 100

//########################### VtxHardnessTabs Class ########################

IMPLEMENT_CLASS(VtxHardnessTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxHardnessTabs, VtxTabsMgr)

SET_SLIDER_EVENTS(VALUE,VtxHardnessTabs,Value)

EVT_MENU(ID_DELETE,VtxHardnessTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxHardnessTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxHardnessTabs::OnUpdateEnable)

END_EVENT_TABLE()

VtxHardnessTabs::VtxHardnessTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxHardnessTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
 	AddPropsTab(page);
    AddPage(page,wxT("Properties"),true);

    return true;
}


void VtxHardnessTabs::AddPropsTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    ValueSlider=new SliderCtrl(panel,ID_VALUE_SLDR,"Density",LABEL, VALUE,SLIDER);
    ValueSlider->setRange(0.0,1);
    boxSizer->Add(ValueSlider->getSizer(),wxALIGN_LEFT|wxALL);
}

int VtxHardnessTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));

	PopupMenu(&menu);
	return menu_action;
}

void VtxHardnessTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}
//-------------------------------------------------------------
// VtxHardnessTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxHardnessTabs::setObjAttributes(){
	update_needed=true;
	char p[1024]="";
	TNhardness *tnode=object();

	double hardness=ValueSlider->getValue();

	sprintf(p,"hardness(%g)",hardness);
	tnode->setExpr(p);
	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
        TheView->set_changed_detail();
	}
}
//-------------------------------------------------------------
// VtxHardnessTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxHardnessTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNhardness *tnode=object();
	double hardness=0.0;

  	TNarg *arg=(TNarg*)tnode->left;
	if(arg){
	    arg->eval();
		hardness=S0.s;
	}
	updateSlider(ValueSlider,hardness);

	update_needed=false;
}
