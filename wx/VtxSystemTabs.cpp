
#include "VtxSystemTabs.h"
#include "RenderOptions.h"
#include "VtxSceneDialog.h"

//########################### VtxSystemTabs Class ########################

#define PLABEL   0
#define PVALUE   50
#define PSLIDER  70
#define LINE_HEIGHT 30
#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define BOX_WIDTH TABS_WIDTH

#define BTNWIDTH 75
#define BTNSIZE wxSize(BTNWIDTH,30)

// define all resource ids here
enum {
	OBJ_VIEWOBJ,
	OBJ_DELETE,
	OBJ_SAVE,
	ID_NAME_TEXT,
	ID_OVERHEAD,
	ID_OBLIQUE,
	ID_EDGEON,
    ID_ORIGIN_X_SLDR,
    ID_ORIGIN_X_TEXT,
    ID_ORIGIN_Y_SLDR,
    ID_ORIGIN_Y_TEXT,
    ID_ORIGIN_Z_SLDR,
    ID_ORIGIN_Z_TEXT
};

IMPLEMENT_CLASS(VtxSystemTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxSystemTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxSystemTabs::OnNameText)

SET_SLIDER_EVENTS(ORIGIN_X,VtxSystemTabs,OriginX)
SET_SLIDER_EVENTS(ORIGIN_Y,VtxSystemTabs,OriginY)
SET_SLIDER_EVENTS(ORIGIN_Z,VtxSystemTabs,OriginZ)


EVT_BUTTON(ID_OVERHEAD,VtxSystemTabs::OnOverheadView)
EVT_BUTTON(ID_OBLIQUE,VtxSystemTabs::OnObliqueView)
EVT_BUTTON(ID_EDGEON,VtxSystemTabs::OnEdgeView)


EVT_MENU(TABS_ENABLE,VtxSystemTabs::OnEnable)
EVT_UPDATE_UI(TABS_ENABLE,VtxSystemTabs::OnUpdateEnable)

EVT_MENU(OBJ_VIEWOBJ,VtxSystemTabs::OnViewObj)
EVT_MENU(OBJ_DELETE,VtxSystemTabs::OnDelete)
//EVT_MENU(OBJ_SAVE,VtxSystemTabs::OnSave)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxSystemTabs::OnAddItem)

SET_FILE_EVENTS(VtxSystemTabs)


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

    page=new wxPanel(this,wxID_ANY);
    AddViewTab(page);
    AddPage(page,wxT("View"),false);

    return true;
}

void VtxSystemTabs::setObjAttributes()
{
	System *system=(System*)object();
	system->origin.x=OriginXSlider->getValue()*LY;
	system->origin.y=OriginYSlider->getValue()*LY;
	system->origin.z=OriginZSlider->getValue()*LY;
	invalidate_object();
}
void VtxSystemTabs::getObjAttributes()
{
	if(!update_needed)
		return;
	System *system=(System*)object();
	size=system->size;
	updateSlider(OriginXSlider,system->origin.x/LY);
	updateSlider(OriginYSlider,system->origin.y/LY);
	updateSlider(OriginZSlider,system->origin.z/LY);
}

int VtxSystemTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(OBJ_VIEWOBJ,wxT("Set View"));

	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));

	wxMenu *addmenu=sceneDialog->getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	sceneDialog->AddFileMenu(menu,object_node->node);
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

	hline->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* origin = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Origin"));
	OriginXSlider = new SliderCtrl(panel,ID_ORIGIN_X_SLDR,"X",PLABEL,PVALUE,PSLIDER);
	OriginXSlider->setRange(-1e4,1e4);
	OriginYSlider = new SliderCtrl(panel,ID_ORIGIN_Y_SLDR,"Y",PLABEL,PVALUE,PSLIDER);
	OriginYSlider->setRange(-1e2,1e2);
	OriginZSlider = new SliderCtrl(panel,ID_ORIGIN_Z_SLDR,"Z",PLABEL,PVALUE,PSLIDER);
	OriginZSlider->setRange(-1e3,1e3);
	origin->Add(OriginXSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	origin->Add(OriginYSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	origin->Add(OriginZSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(origin, 0, wxALIGN_LEFT|wxALL,0);

	origin->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));

    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

}

//-------------------------------------------------------------
// VtxSystemTabs::AddViewTab() Add view controls tab
//-------------------------------------------------------------
void VtxSystemTabs::AddViewTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* set_view = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Set View"));

	m_overhead=new wxButton(panel,ID_OVERHEAD,"Top",wxDefaultPosition,BTNSIZE);
	m_edgeon=new wxButton(panel,ID_EDGEON,"Edge",wxDefaultPosition,BTNSIZE);
	m_oblique=new wxButton(panel,ID_OBLIQUE,"Oblique",wxDefaultPosition,BTNSIZE);

	set_view->Add(m_overhead,0,wxALIGN_LEFT|wxALL,0);
	set_view->Add(m_edgeon,0,wxALIGN_LEFT|wxALL,0);
	set_view->Add(m_oblique,0,wxALIGN_LEFT|wxALL,0);
	set_view->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(set_view, 0, wxALIGN_LEFT|wxALL,0);
	topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

}

void VtxSystemTabs::OnViewObj(wxCommandEvent& event){
	//if(!is_viewobj()){
		TheScene->set_viewobj(object());
		TheScene->selobj=object();
		TheScene->focusobj=object();
		TheScene->set_changed_view();
		TheScene->change_view(ORBITAL);
		menu_action=TABS_VIEWOBJ;
	//}
}
void VtxSystemTabs::OnUpdateViewObj(wxUpdateUIEvent& event){
	event.Check(is_viewobj());
}

void VtxSystemTabs::updateControls(){
	Orbital *obj=object();
	//updateSlider(SizeSlider,obj->size/MM);
	object_name->SetValue(object_node->node->nodeName());
	getObjAttributes();
}

//-------------------------------------------------------------
// VtxSystemTabs::set_view() set view point
//-------------------------------------------------------------
void VtxSystemTabs::set_view(double v, double t, double p, double r)
{
	System *system=(System*)object();

	TheScene->viewtype=ORBITAL;
	TheScene->set_changed_view();
	system->init_view();
	TheScene->theta=t;
	TheScene->phi=p;
	TheScene->height=TheScene->radius=r;
	TheScene->selobj=TheScene->focusobj=0;
	TheScene->set_viewobj(system);
	TheScene->views_mark();
	TheScene->clr_automv();
	TheScene->view_angle=v;

	TheScene->gstride=TheScene->vstride=0.1001*system->size;

	TheScene->minr=0.0;
	TheScene->view->speed=TheScene->gstride;
	TheScene->rebuild_all();
	update_needed=true;
	getObjAttributes();
}

void VtxSystemTabs::OnOverheadView(wxCommandEvent& event){
	set_view(-90,0,90,4*size);
}
void VtxSystemTabs::OnObliqueView(wxCommandEvent& event){
	set_view(0,0,45,3*size);
}
void VtxSystemTabs::OnEdgeView(wxCommandEvent& event){
	set_view(0,0,0,4*size);
}
