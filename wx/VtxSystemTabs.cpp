
#include "VtxSystemTabs.h"
#include "RenderOptions.h"
#include "VtxSceneDialog.h"

//########################### VtxSystemTabs Class ########################

// define all resource ids here
enum {
	OBJ_VIEWOBJ,
	OBJ_DELETE,
	OBJ_SAVE,
	ID_NAME_TEXT,
    ID_RADIUS_SLDR,
    ID_RADIUS_TEXT,
};

IMPLEMENT_CLASS(VtxSystemTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxSystemTabs, wxNotebook)
EVT_MENU(TABS_ENABLE,VtxSystemTabs::OnEnable)
EVT_UPDATE_UI(TABS_ENABLE,VtxSystemTabs::OnUpdateEnable)

EVT_MENU(OBJ_VIEWOBJ,VtxSystemTabs::OnViewObj)
EVT_MENU(OBJ_DELETE,VtxSystemTabs::OnDelete)
EVT_MENU(OBJ_SAVE,VtxSystemTabs::OnSave)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxSystemTabs::OnAddItem)

END_EVENT_TABLE()

VtxSystemTabs::VtxSystemTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxSystemTabs::Create(wxWindow* parent,
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
    AddPage(page,wxT("Properties"),true);

    return true;
}

int VtxSystemTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(OBJ_VIEWOBJ,wxT("Set View"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));
	menu.Append(OBJ_SAVE,wxT("Save.."));

	wxMenu *addmenu=sceneDialog->getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	PopupMenu(&menu);
	return menu_action;
}

void VtxSystemTabs::AddDisplayTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2+10,VALUE2+SLIDER2);
	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->AddSpacer(10);

	SizeSlider=new SliderCtrl(panel,ID_RADIUS_SLDR,"Size(mm)",LABEL2S, VALUE2+10,SLIDER2);
	SizeSlider->setRange(100*MM,10000*MM);
	SizeSlider->setValue(1000*MM);

	hline->Add(SizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

   // boxSizer->Add(new wxStaticText(panel,-1,"Controls Not Yet Implemented"), 0, wxALIGN_LEFT|wxALL, 4);

    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

}

void VtxSystemTabs::OnViewObj(wxCommandEvent& event){
	if(!is_viewobj()){
		TheScene->set_viewobj(object());
		TheScene->selobj=object();
		TheScene->change_view(ORBITAL);
		menu_action=TABS_VIEWOBJ;
	}
}
void VtxSystemTabs::OnUpdateViewObj(wxUpdateUIEvent& event){
	event.Check(is_viewobj());
}

void VtxSystemTabs::updateControls(){
	Orbital *obj=object();
	updateSlider(SizeSlider,obj->size/MM);
	object_name->SetValue(object_node->node->nodeName());
}

