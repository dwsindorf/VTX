
#include "VtxStarTabs.h"
#include "VtxSceneDialog.h"


//########################### ObjectAddMenu Class ########################
enum{
	OBJ_NONE,
	OBJ_SHOW,
	OBJ_VIEWOBJ,
	OBJ_DELETE,

	ID_NAME_TEXT,
    ID_CELLSIZE_SLDR,
    ID_CELLSIZE_TEXT,

    ID_RADIUS_SLDR,
    ID_RADIUS_TEXT,
    ID_ORBIT_RADIUS_SLDR,
    ID_ORBIT_RADIUS_TEXT,
    ID_ORBIT_PHASE_SLDR,
    ID_ORBIT_PHASE_TEXT,
    ID_ORBIT_TILT_SLDR,
    ID_ORBIT_TILT_TEXT,
	ID_TYPE_TEXT,
    ID_ROT_PHASE_SLDR,
    ID_ROT_PHASE_TEXT,
    ID_DAY_SLDR,
    ID_DAY_TEXT,
    ID_YEAR_SLDR,
    ID_YEAR_TEXT,
    ID_SPECULAR_SLDR,
    ID_SPECULAR_TEXT,
    ID_SPECULAR_COLOR,
    ID_DIFFUSE_SLDR,
    ID_DIFFUSE_TEXT,
    ID_DIFFUSE_COLOR,
    ID_EMISSION_SLDR,
    ID_EMISSION_TEXT,
    ID_EMISSION_COLOR,
};

//########################### VtxStarTabs Class ########################

IMPLEMENT_CLASS(VtxStarTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxStarTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxStarTabs::OnNameText)

SET_SLIDER_EVENTS(CELLSIZE,VtxStarTabs,CellSize)
SET_SLIDER_EVENTS(RADIUS,VtxStarTabs,Size)
SET_SLIDER_EVENTS(DAY,VtxStarTabs,Day)
SET_SLIDER_EVENTS(YEAR,VtxStarTabs,Year)
SET_SLIDER_EVENTS(ROT_PHASE,VtxStarTabs,RotPhase)
SET_SLIDER_EVENTS(ORBIT_RADIUS,VtxStarTabs,OrbitRadius)
SET_SLIDER_EVENTS(ORBIT_TILT,VtxStarTabs,OrbitTilt)
SET_SLIDER_EVENTS(ORBIT_PHASE,VtxStarTabs,OrbitPhase)
SET_COLOR_EVENTS(EMISSION,VtxStarTabs,Emission)
SET_COLOR_EVENTS(SPECULAR,VtxStarTabs,Specular)
SET_COLOR_EVENTS(DIFFUSE,VtxStarTabs,Diffuse)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxStarTabs::OnAddItem)
EVT_MENU(OBJ_DELETE,VtxStarTabs::OnDelete)

EVT_MENU(OBJ_SHOW,VtxStarTabs::OnEnable)
EVT_UPDATE_UI(OBJ_SHOW,VtxStarTabs::OnUpdateEnable)

EVT_MENU(OBJ_VIEWOBJ,VtxStarTabs::OnViewObj)
EVT_UPDATE_UI(OBJ_VIEWOBJ,VtxStarTabs::OnUpdateViewObj)

SET_FILE_EVENTS(VtxStarTabs)

END_EVENT_TABLE()

VtxStarTabs::VtxStarTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

int VtxStarTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;

	wxMenu menu;

	menu.AppendCheckItem(OBJ_VIEWOBJ,wxT("Set View"));
	menu.AppendCheckItem(OBJ_SHOW,wxT("Show"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));

	wxMenu *addmenu=getAddMenu((NodeIF*)object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	sceneDialog->AddFileMenu(menu,object_node->node);

	PopupMenu(&menu);
	return menu_action;
}

bool VtxStarTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

    radiance=Color(1,1,1);
	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
    AddObjectTab(page);
    AddPage(page,wxT("Object"),true);

    page=new wxPanel(this,wxID_ANY);
    AddLightingTab(page);
    AddPage(page,wxT("Lighting"),false);

    return true;
}

void VtxStarTabs::AddObjectTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* object_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Object"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2+10,VALUE2+SLIDER2);
	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->AddSpacer(10);

	object_type=new StaticTextCtrl(panel,ID_TYPE_TEXT,"Type",LABEL2S,VALUE2+SLIDER2);
	hline->Add(object_type->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    hline = new wxBoxSizer(wxHORIZONTAL);

	SizeSlider=new SliderCtrl(panel,ID_RADIUS_SLDR,"Size(mm)",LABEL2B, VALUE2,SLIDER2);
	SizeSlider->setRange(0.1,10);
	SizeSlider->setValue(1);

	hline->Add(SizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	CellSizeSlider=new SliderCtrl(panel,ID_CELLSIZE_SLDR,"Grid",LABEL2S, VALUE2,SLIDER2);
	CellSizeSlider->setRange(1,4);
	CellSizeSlider->setValue(1);
	hline->Add(CellSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	//TiltSlider=new SliderCtrl(panel,ID_TILT_SLDR,"Tilt",LABEL2S, VALUE2,SLIDER2);
	//TiltSlider->setRange(0,360);
	//TiltSlider->setValue(0.0);

	//hline->Add(TiltSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	DaySlider=new SliderCtrl(panel,ID_DAY_SLDR,"Day(hrs)",LABEL2B, VALUE2,SLIDER2);
	DaySlider->setRange(0,200);
	DaySlider->setValue(24);

	hline->Add(DaySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	RotPhaseSlider=new SliderCtrl(panel,ID_ROT_PHASE_SLDR,"Phase",LABEL2S, VALUE2,SLIDER2);
	RotPhaseSlider->setRange(0,360);
	RotPhaseSlider->setValue(0.0);

	hline->Add(RotPhaseSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	object_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(object_cntrls, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* orbit_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Orbit"));

	hline = new wxBoxSizer(wxHORIZONTAL);

	OrbitRadiusSlider=new SliderCtrl(panel,ID_ORBIT_RADIUS_SLDR,"Radius(mm)",LABEL2B, VALUE2,SLIDER2);
	OrbitRadiusSlider->setRange(0,100);
	OrbitRadiusSlider->setValue(10);

	hline->Add(OrbitRadiusSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OrbitTiltSlider=new SliderCtrl(panel,ID_ORBIT_TILT_SLDR,"Oblique",LABEL2S, VALUE2,SLIDER2);
	OrbitTiltSlider->setRange(0,0.95);
	OrbitTiltSlider->setValue(0.0);
	hline->Add(OrbitTiltSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	orbit_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	YearSlider=new SliderCtrl(panel,ID_YEAR_SLDR,"Year(days)",LABEL2B, VALUE2,SLIDER2);
	YearSlider->setRange(0,1000);
	YearSlider->setValue(300);

	hline->Add(YearSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OrbitPhaseSlider=new SliderCtrl(panel,ID_ORBIT_PHASE_SLDR,"Phase",LABEL2S, VALUE2,SLIDER2);
	OrbitPhaseSlider->setRange(0,360);
	OrbitPhaseSlider->setValue(0.0);

	hline->Add(OrbitPhaseSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	orbit_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	orbit_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(orbit_cntrls, 0, wxALIGN_LEFT|wxALL,0);

}

void VtxStarTabs::AddLightingTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* color_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Color"));

    EmissionSlider=new ColorSlider(panel,ID_EMISSION_SLDR,"Radiance",LABEL,VALUE,CSLIDER,CBOX1);
    color_cntrls->Add(EmissionSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
    SpecularSlider=new ColorSlider(panel,ID_SPECULAR_SLDR,"Specular",LABEL,VALUE,CSLIDER,CBOX1);
    color_cntrls->Add(SpecularSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
	DiffuseSlider=new ColorSlider(panel,ID_DIFFUSE_SLDR,"Diffuse",LABEL,VALUE,CSLIDER,CBOX1);
	color_cntrls->Add(DiffuseSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
    color_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(color_cntrls, 0, wxALIGN_LEFT|wxALL,0);

	boxSizer = new wxBoxSizer(wxVERTICAL);

}

void VtxStarTabs::OnViewObj(wxCommandEvent& event){
	if(!is_viewobj()){
		TheScene->set_viewobj(object());
		TheScene->selobj=object();
		TheScene->change_view(ORBITAL);
		menu_action=TABS_VIEWOBJ;
	}
}

void VtxStarTabs::OnUpdateViewObj(wxUpdateUIEvent& event){
	event.Check(is_viewobj());
}

void VtxStarTabs::updateControls(){
	Star *obj=object();
	char type[256];
	type[0]=0;
	double temp=0;
	object()->getStarData(&temp, type);
	char type_str[256];
	sprintf(type_str,"%s     %d K",type,(int)temp);
	object_type->SetValue(type_str);
	//object_type->SetValue((char*)tree->typeName());
	//obj->emission.print();
	updateSlider(CellSizeSlider,obj->detail);
	updateSlider(SizeSlider,obj->size);
	updateSlider(OrbitRadiusSlider,obj->orbit_radius);
	updateSlider(OrbitPhaseSlider,obj->orbit_phase);
	updateSlider(OrbitTiltSlider,obj->orbit_eccentricity);
	//updateSlider(TiltSlider,obj->tilt);
	updateSlider(RotPhaseSlider,obj->rot_phase);
	updateSlider(DaySlider,obj->day);
	updateSlider(YearSlider,obj->year);
	updateColor(EmissionSlider,obj->emission);
	updateColor(SpecularSlider, object()->specular);
	updateColor(DiffuseSlider,object()->diffuse);
	object_name->SetValue(object_node->node->nodeName());
}


