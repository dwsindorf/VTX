
#include "VtxGlossTabs.h"

#include <wx/filefn.h>
#include <wx/dir.h>

#define STATE1D 0
#define STATE2D 1
//########################### VtxGlossTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
    ID_SHINE_SLDR,
    ID_SHINE_TEXT,
    ID_ALBEDO_SLDR,
    ID_ALBEDO_TEXT,
 };

#define LABEL1 40
#define VALUE1 50
#define SLIDER1 100

//########################### VtxGlossTabs Class ########################

IMPLEMENT_CLASS(VtxGlossTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxGlossTabs, VtxTabsMgr)

SET_SLIDER_EVENTS(SHINE,VtxGlossTabs,Shine)
SET_SLIDER_EVENTS(ALBEDO,VtxGlossTabs,Albedo)

EVT_MENU(ID_DELETE,VtxGlossTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxGlossTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxGlossTabs::OnUpdateEnable)

END_EVENT_TABLE()

VtxGlossTabs::VtxGlossTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxGlossTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
 	AddLightingTab(page);
    AddPage(page,wxT("Lighting"),true);

    return true;
}

void VtxGlossTabs::AddLightingTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    ShineSlider=new SliderCtrl(panel,ID_SHINE_SLDR,"Shine",LABEL, VALUE,SLIDER);
    ShineSlider->setRange(0.5,20);
    boxSizer->Add(ShineSlider->getSizer(),wxALIGN_LEFT|wxALL);

    AlbedoSlider=new SliderCtrl(panel,ID_ALBEDO_SLDR,"Albedo",LABEL, VALUE,SLIDER);
    AlbedoSlider->setRange(0,1);
    boxSizer->Add(AlbedoSlider->getSizer(),wxALIGN_LEFT|wxALL);
}

int VtxGlossTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));

	PopupMenu(&menu);
	return menu_action;
}

void VtxGlossTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}
//-------------------------------------------------------------
// VtxGlossTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxGlossTabs::setObjAttributes(){
	update_needed=true;
	char p[1024]="";
	TNgloss *tnode=object();

	double shine=ShineSlider->getValue();
	double albedo=AlbedoSlider->getValue();

	if(shine)
		sprintf(p,"gloss(%g,%g)\n",albedo,shine);
	else
		sprintf(p,"gloss(%g)\n",albedo);
	tnode->setExpr(p);
	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
        TheView->set_changed_detail();
        TheScene->rebuild();
	}
}
//-------------------------------------------------------------
// VtxGlossTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxGlossTabs::getObjAttributes(){
	//if(!update_needed)
	//	return;
	TNgloss *tnode=object();
	double shine=0.0;
	double albedo=0.0;

  	TNarg *arg=(TNarg*)tnode->right;
	if(arg){
	    arg->eval();
		albedo=S0.s;
		arg=arg->next();
		if(arg){
			arg->eval();
			shine=S0.s;
		}
	}
	updateSlider(AlbedoSlider,albedo);
	updateSlider(ShineSlider,shine);

	update_needed=false;
}
