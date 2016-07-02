
#include "VtxRingTabs.h"
#include "UniverseModel.h"

enum {
	OBJ_SHOW,
	OBJ_DELETE,
	OBJ_SAVE,
	ID_NAME_TEXT,
    ID_CELLSIZE_SLDR,
    ID_CELLSIZE_TEXT,
    ID_RADIUS_SLDR,
    ID_RADIUS_TEXT,
    ID_WIDTH_SLDR,
    ID_WIDTH_TEXT,
    ID_EMISSION_SLDR,
    ID_EMISSION_TEXT,
    ID_EMISSION_COLOR,
    ID_SHADOW_SLDR,
    ID_SHADOW_TEXT,
    ID_SHADOW_COLOR,
};

IMPLEMENT_CLASS(VtxRingTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxRingTabs, wxNotebook)
EVT_MENU(OBJ_DELETE,VtxRingTabs::OnDelete)
EVT_MENU(OBJ_SHOW,VtxRingTabs::OnEnable)
EVT_UPDATE_UI(OBJ_SHOW, VtxRingTabs::OnUpdateEnable)

SET_SLIDER_EVENTS(CELLSIZE,VtxRingTabs,CellSize)
EVT_TEXT_ENTER(ID_NAME_TEXT,VtxRingTabs::OnNameText)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_RADIUS_SLDR,VtxRingTabs::OnEndRadiusSlider)
EVT_COMMAND_SCROLL(ID_RADIUS_SLDR,VtxRingTabs::OnRadiusSlider)
EVT_TEXT_ENTER(ID_RADIUS_TEXT,VtxRingTabs::OnRadiusText)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_WIDTH_SLDR,VtxRingTabs::OnEndWidthSlider)
EVT_COMMAND_SCROLL(ID_WIDTH_SLDR,VtxRingTabs::OnWidthSlider)
EVT_TEXT_ENTER(ID_WIDTH_TEXT,VtxRingTabs::OnWidthText)

SET_COLOR_EVENTS(EMISSION,VtxRingTabs,Emission)
SET_COLOR_EVENTS(SHADOW,VtxRingTabs,Shadow)

EVT_MENU(OBJ_DELETE,VtxRingTabs::OnDelete)
EVT_MENU(OBJ_SAVE,VtxRingTabs::OnSave)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxRingTabs::OnAddItem)

END_EVENT_TABLE()

VtxRingTabs::VtxRingTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

int VtxRingTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(OBJ_SHOW,wxT("Show"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));
	menu.Append(OBJ_SAVE,wxT("Save.."));

	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	PopupMenu(&menu);
	return menu_action;
}

bool VtxRingTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

	changing=true;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
    AddObjectTab(page);
    AddPage(page,wxT("Object"),true);

    page=new wxPanel(this,wxID_ANY);
    AddLightingTab(page);
    AddPage(page,wxT("Lighting"),false);
	changing=false;
    return true;
}

void VtxRingTabs::AddObjectTab(wxWindow *panel){
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topsizer);

    wxBoxSizer* boxsizer = new wxBoxSizer(wxVERTICAL);
    topsizer->Add(boxsizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* object_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Object"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2+10,VALUE2+SLIDER2);
	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->AddSpacer(10);

	CellSizeSlider=new SliderCtrl(panel,ID_CELLSIZE_SLDR,"Grid",LABEL2S, VALUE2,SLIDER2);
	CellSizeSlider->setRange(1,4);
	CellSizeSlider->setValue(1);
	hline->Add(CellSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
    object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	RadiusSlider=new SliderCtrl(panel,ID_RADIUS_SLDR,"Radius",LABEL, VALUE,SLIDER);
	RadiusSlider->setRange(0.1,2);
	RadiusSlider->setValue(0.5);

	object_cntrls->Add(RadiusSlider->getSizer(),wxALIGN_LEFT|wxALL);

	WidthSlider=new SliderCtrl(panel,ID_WIDTH_SLDR,"Width",LABEL, VALUE,SLIDER);
	WidthSlider->setRange(0.1,2);
	WidthSlider->setValue(0.1);

	object_cntrls->Add(WidthSlider->getSizer(),wxALIGN_LEFT|wxALL);

	object_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxsizer->Add(object_cntrls,0, wxALIGN_LEFT|wxALL,0);
}

void VtxRingTabs::AddLightingTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);
	wxBoxSizer* color_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Lighting"));

	EmissionSlider=new ColorSlider(panel,ID_EMISSION_SLDR,"Emission",LABEL,VALUE,CSLIDER,CBOX1);
	color_cntrls->Add(EmissionSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
    ShadowSlider=new ColorSlider(panel,ID_SHADOW_SLDR,"Shadow",LABEL,VALUE,CSLIDER,CBOX1);
    color_cntrls->Add(ShadowSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

	color_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(color_cntrls, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxRingTabs::setObjAttributes()
{
	Ring *obj=object();
	double r=parentSize();
	obj->inner_radius=r*RadiusSlider->getValue();
	obj->width=r*WidthSlider->getValue();
	obj->set_geometry();

}
void VtxRingTabs::getObjAttributes()
{
	if(!update_needed)
		return;

	Ring *obj=object();
	double r=parentSize();
	RadiusSlider->setValue(obj->inner_radius/r);
	WidthSlider->setValue(obj->width/r);
	updateColor(EmissionSlider,obj->emission);
	updateColor(ShadowSlider,obj->shadow_color);
	updateSlider(CellSizeSlider,obj->detail);
	object_name->SetValue(object_node->node->nodeName());
	update_needed=false;
}

void VtxRingTabs::updateControls(){
	if(update_needed)
		getObjAttributes();
}
