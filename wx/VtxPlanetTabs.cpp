#include "VtxPlanetTabs.h"
#include "UniverseModel.h"
#include "VtxSceneDialog.h"

//########################### ObjectAddMenu Class ########################
enum {
	OBJ_SHOW,
	OBJ_VIEWOBJ,
	OBJ_DELETE,

	ID_NAME_TEXT,
	ID_CELLSIZE_SLDR,
	ID_CELLSIZE_TEXT,

	ID_RADIUS_SLDR,
	ID_RADIUS_TEXT,
	ID_HSCALE_SLDR,
	ID_HSCALE_TEXT,
	ID_TYPE_TEXT,
	ID_TIDAL_LOCK,
	ID_SEASONAL,

	ID_ORBIT_RADIUS_SLDR,
	ID_ORBIT_RADIUS_TEXT,
	ID_ORBIT_PHASE_SLDR,
	ID_ORBIT_PHASE_TEXT,
	ID_ORBIT_TILT_SLDR,
	ID_ORBIT_TILT_TEXT,
	ID_TILT_SLDR,
	ID_TILT_TEXT,
	ID_ROT_PHASE_SLDR,
	ID_ROT_PHASE_TEXT,
	ID_DAY_SLDR,
	ID_DAY_TEXT,
	ID_YEAR_SLDR,
	ID_YEAR_TEXT,
	ID_SHINE_SLDR,
	ID_SHINE_TEXT,
	ID_ALBEDO_SLDR,
	ID_ALBEDO_TEXT,
	ID_SEASON_SLDR,
	ID_SEASON_TEXT,
	ID_TEMP_SLDR,
	ID_TEMP_TEXT,
	ID_AMBIENT_SLDR,
	ID_AMBIENT_TEXT,
	ID_AMBIENT_COLOR,
	ID_SPECULAR_SLDR,
	ID_SPECULAR_TEXT,
	ID_SPECULAR_COLOR,
	ID_DIFFUSE_SLDR,
	ID_DIFFUSE_TEXT,
	ID_DIFFUSE_COLOR,
	ID_EMISSION_SLDR,
	ID_EMISSION_TEXT,
	ID_EMISSION_COLOR,
	ID_SHADOW_SLDR,
	ID_SHADOW_TEXT,
	ID_SHADOW_COLOR,

};

//########################### VtxPlanetTabs Class ########################

IMPLEMENT_CLASS(VtxPlanetTabs, wxNotebook)

BEGIN_EVENT_TABLE(VtxPlanetTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxPlanetTabs::OnNameText)
//EVT_CHECKBOX(ID_CELSIUS,VtxPlanetTabs::OnCelsius)
EVT_CHECKBOX(ID_SEASONAL,VtxPlanetTabs::OnSeasonal)

SET_SLIDER_EVENTS(CELLSIZE,VtxPlanetTabs,CellSize)
SET_SLIDER_EVENTS(RADIUS,VtxPlanetTabs,Size)
SET_SLIDER_EVENTS(HSCALE,VtxPlanetTabs,Hscale)

SET_SLIDER_EVENTS(TILT,VtxPlanetTabs,Tilt)
SET_SLIDER_EVENTS(DAY,VtxPlanetTabs,Day)
SET_SLIDER_EVENTS(YEAR,VtxPlanetTabs,Year)
SET_SLIDER_EVENTS(ROT_PHASE,VtxPlanetTabs,RotPhase)
SET_SLIDER_EVENTS(ORBIT_RADIUS,VtxPlanetTabs,OrbitRadius)
SET_SLIDER_EVENTS(ORBIT_TILT,VtxPlanetTabs,OrbitTilt)
SET_SLIDER_EVENTS(ORBIT_PHASE,VtxPlanetTabs,OrbitPhase)
SET_SLIDER_EVENTS(SHINE,VtxPlanetTabs,Shine)
SET_SLIDER_EVENTS(ALBEDO,VtxPlanetTabs,Albedo)
SET_SLIDER_EVENTS(SEASON,VtxPlanetTabs,Season)
SET_SLIDER_EVENTS(TEMP,VtxPlanetTabs,Temp)

SET_COLOR_EVENTS(AMBIENT,VtxPlanetTabs,Ambient)
SET_COLOR_EVENTS(EMISSION,VtxPlanetTabs,Emission)
SET_COLOR_EVENTS(SPECULAR,VtxPlanetTabs,Specular)
SET_COLOR_EVENTS(DIFFUSE,VtxPlanetTabs,Diffuse)
SET_COLOR_EVENTS(SHADOW,VtxPlanetTabs,Shadow)
EVT_BUTTON(ID_TIDAL_LOCK,VtxPlanetTabs::OnTidalLock)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxPlanetTabs::OnAddItem)

EVT_MENU(OBJ_DELETE,VtxPlanetTabs::OnDelete)

EVT_MENU(OBJ_SHOW,VtxPlanetTabs::OnEnable)
EVT_UPDATE_UI(OBJ_SHOW,VtxPlanetTabs::OnUpdateEnable)

EVT_UPDATE_UI(ID_TYPE_TEXT, VtxPlanetTabs::OnUpdateTemp)

EVT_MENU(OBJ_VIEWOBJ,VtxPlanetTabs::OnViewObj)
EVT_UPDATE_UI(OBJ_VIEWOBJ,VtxPlanetTabs::OnUpdateViewObj)

SET_FILE_EVENTS(VtxPlanetTabs)

END_EVENT_TABLE()

VtxPlanetTabs::VtxPlanetTabs(wxWindow *parent, wxWindowID id,
		const wxPoint &pos, const wxSize &size, long style,
		const wxString &name) :
		VtxTabsMgr(parent, id, pos, size, style, name) {
	Create(parent, id, pos, size, style, name);
}

int VtxPlanetTabs::showMenu(bool expanded) {
	menu_action = TABS_NONE;

	wxMenu menu;

	menu.AppendCheckItem(OBJ_VIEWOBJ, wxT("Set View"));
	menu.AppendCheckItem(OBJ_SHOW, wxT("Show"));
	menu.AppendSeparator();

	menu.Append(OBJ_DELETE, wxT("Delete"));

	wxMenu *addmenu = getAddMenu(object());

	if (addmenu) {
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu, "Add");
	}

	sceneDialog->AddFileMenu(menu,object_node->node);
	PopupMenu(&menu);
	return menu_action;
}

bool VtxPlanetTabs::Create(wxWindow *parent, wxWindowID id, const wxPoint &pos,
		const wxSize &size, long style, const wxString &name) {
	if (!VtxTabsMgr::Create(parent, id, pos, size, style, name))
		return false;

	changing = true;

	wxNotebookPage *page = new wxPanel(this, wxID_ANY);
	AddObjectTab(page);
	AddPage(page, wxT("Object"), true);

	page = new wxPanel(this, wxID_ANY);
	AddLightingTab(page);
	AddPage(page, wxT("Lighting"), false);
	changing = false;

	return true;
}
void VtxPlanetTabs::AddObjectTab(wxWindow *panel) {
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);

	wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT | wxALL, 6);

	wxBoxSizer *object_cntrls = new wxStaticBoxSizer(wxVERTICAL, panel,
			wxT("Object"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name = new TextCtrl(panel, ID_NAME_TEXT, "Name", 40,100);
	hline->Add(object_name->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);
	//hline->AddSpacer(10);

	tidal_lock=new wxButton(panel,ID_TIDAL_LOCK,"TidalLock",wxDefaultPosition,wxSize(60,25));
	hline->Add(tidal_lock,0,wxALIGN_LEFT|wxALL,0);
	
	temp_state=new StaticTextCtrl(panel,ID_TYPE_TEXT,"Temp",30,80);
	hline->Add(temp_state->getSizer(),0,wxALIGN_LEFT|wxALL,0);

//	celsius=new wxCheckBox(panel, ID_CELSIUS, "Celsius");
//	hline->Add(celsius,0, wxALIGN_LEFT|wxALL,5);

	seasonal=new wxCheckBox(panel, ID_SEASONAL, "Seasonal");
	hline->Add(seasonal,0, wxALIGN_LEFT|wxALL,5);
	object_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	SizeSlider = new SliderCtrl(panel, ID_RADIUS_SLDR, "Size(1e3 ml)", LABEL2B,
			VALUE2, SLIDER2);
	SizeSlider->setRange(1, 1000);
	SizeSlider->setValue(1);

	hline->Add(SizeSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	CellSizeSlider = new SliderCtrl(panel, ID_CELLSIZE_SLDR, "Grid", LABEL2S,
			VALUE2, SLIDER2);
	CellSizeSlider->setRange(1, 4);
	CellSizeSlider->setValue(1);
	hline->Add(CellSizeSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	object_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	HscaleSlider = new SliderCtrl(panel, ID_HSCALE_SLDR, "Ht(mls)", LABEL2B,
			VALUE2, SLIDER2);
	HscaleSlider->setRange(0.1, 20);
	HscaleSlider->setValue(2.5);

	hline->Add(HscaleSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	TiltSlider = new SliderCtrl(panel, ID_TILT_SLDR, "Tilt", LABEL2S, VALUE2,
			SLIDER2);
	TiltSlider->setRange(-90, 90);
	TiltSlider->setValue(0.0);

	hline->Add(TiltSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	object_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	DaySlider = new SliderCtrl(panel, ID_DAY_SLDR, "Day(hrs)", LABEL2B, VALUE2,
			SLIDER2);
	DaySlider->setRange(0, 200);
	DaySlider->setValue(24);

	hline->Add(DaySlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	RotPhaseSlider = new SliderCtrl(panel, ID_ROT_PHASE_SLDR, "Phase", LABEL2S,
			VALUE2, SLIDER2);
	RotPhaseSlider->setRange(0, 360);
	RotPhaseSlider->setValue(0.0);

	hline->Add(RotPhaseSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	object_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);
	object_cntrls->SetMinSize(wxSize(TABS_WIDTH - TABS_BORDER, -1));
	boxSizer->Add(object_cntrls, 0, wxALIGN_LEFT | wxALL, 0);

	wxBoxSizer *orbit_cntrls = new wxStaticBoxSizer(wxVERTICAL, panel,
			wxT("Orbit"));

	hline = new wxBoxSizer(wxHORIZONTAL);

	OrbitRadiusSlider = new SliderCtrl(panel, ID_ORBIT_RADIUS_SLDR,
			"Radius(x1e6)", LABEL2B, VALUE2, SLIDER2);
	OrbitRadiusSlider->setRange(1, 1000);
	OrbitRadiusSlider->setValue(1);

	hline->Add(OrbitRadiusSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	OrbitTiltSlider = new SliderCtrl(panel, ID_ORBIT_TILT_SLDR, "Tilt", LABEL2S,
			VALUE2, SLIDER2);
	OrbitTiltSlider->setRange(0, 90);
	OrbitTiltSlider->setValue(0.0);
	hline->Add(OrbitTiltSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	orbit_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	YearSlider = new SliderCtrl(panel, ID_YEAR_SLDR, "Year(days)", LABEL2B,
			VALUE2, SLIDER2);
	YearSlider->setRange(0, 1000);
	YearSlider->setValue(300);

	hline->Add(YearSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	OrbitPhaseSlider = new SliderCtrl(panel, ID_ORBIT_PHASE_SLDR, "Phase",
			LABEL2S, VALUE2, SLIDER2);
	OrbitPhaseSlider->setRange(0, 360);
	OrbitPhaseSlider->setValue(0.0);

	hline->Add(OrbitPhaseSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	orbit_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);
	orbit_cntrls->SetMinSize(wxSize(TABS_WIDTH - TABS_BORDER, -1));

	boxSizer->Add(orbit_cntrls, 0, wxALIGN_LEFT | wxALL, 0);
	
	wxBoxSizer *season_cntrls = new wxStaticBoxSizer(wxVERTICAL, panel,
			wxT("Variability"));
	season_cntrls->SetMinSize(wxSize(TABS_WIDTH - TABS_BORDER, -1));
	hline = new wxBoxSizer(wxHORIZONTAL);
	
	SeasonSlider = new SliderCtrl(panel, ID_SEASON_SLDR, "Season", LABEL2B,
			VALUE2, SLIDER2);
	SeasonSlider->setRange(0.0, 1);
	SeasonSlider->setValue(0.1);
	hline->Add(SeasonSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	TempSlider = new SliderCtrl(panel, ID_TEMP_SLDR, "Temp", LABEL2S,
			VALUE2, SLIDER2);
	TempSlider->setRange(0.0, 1);
	TempSlider->setValue(0.01);
	hline->Add(TempSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	season_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	boxSizer->Add(season_cntrls, 0, wxALIGN_LEFT | wxALL, 0);


}
void VtxPlanetTabs::AddLightingTab(wxWindow *panel) {
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);

	wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT | wxALL, 5);

	wxStaticBoxSizer *color_cntrls = new wxStaticBoxSizer(wxVERTICAL, panel,
			wxT("Color"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	ShineSlider = new SliderCtrl(panel, ID_SHINE_SLDR, "Shine", LABEL, VALUE, SLIDER2);
	ShineSlider->setRange(0.25, 100);
	hline->Add(ShineSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);
	AlbedoSlider = new SliderCtrl(panel, ID_ALBEDO_SLDR, "Albedo", LABEL2, VALUE, SLIDER2);
	AlbedoSlider->setRange(0.01, 1);
	hline->Add(AlbedoSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);
	color_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	AmbientSlider = new ColorSlider(panel, ID_AMBIENT_SLDR, "Ambient", LABEL,
			VALUE, CSLIDER, CBOX1);
	color_cntrls->Add(AmbientSlider->getSizer(), 1, wxALIGN_LEFT | wxALL);
	SpecularSlider = new ColorSlider(panel, ID_SPECULAR_SLDR, "Specular", LABEL,
			VALUE, CSLIDER, CBOX1);
	color_cntrls->Add(SpecularSlider->getSizer(), 1, wxALIGN_LEFT | wxALL);
	DiffuseSlider = new ColorSlider(panel, ID_DIFFUSE_SLDR, "Diffuse", LABEL,
			VALUE, CSLIDER, CBOX1);
	color_cntrls->Add(DiffuseSlider->getSizer(), 1, wxALIGN_LEFT | wxALL);
	EmissionSlider = new ColorSlider(panel, ID_EMISSION_SLDR, "Radiance", LABEL,
			VALUE, CSLIDER, CBOX1);
	color_cntrls->Add(EmissionSlider->getSizer(), 1, wxALIGN_LEFT | wxALL);
	ShadowSlider = new ColorSlider(panel, ID_SHADOW_SLDR, "Shadow", LABEL,
			VALUE, CSLIDER, CBOX1);
	color_cntrls->Add(ShadowSlider->getSizer(), 1, wxALIGN_LEFT | wxALL);
	color_cntrls->SetMinSize(wxSize(TABS_WIDTH - TABS_BORDER, -1));
	boxSizer->Add(color_cntrls, 0, wxALIGN_LEFT | wxALL, 0);
}

void VtxPlanetTabs::OnTidalLock(wxCommandEvent& event){
	Planetoid *obj = (Planetoid*) object();
	obj->setTidalLocked();
	obj->invalidate();
	TheScene->rebuild();
}
void VtxPlanetTabs::OnViewObj(wxCommandEvent &event) {
	if (!is_viewobj()) {
		double tm = (1 - object()->calc_delt());
		tm = 24 * fmod(tm, 1);
		TheScene->set_viewobj(object());
		TheScene->selobj = object();
		TheScene->change_view(ORBITAL);

		//object()->set_time(0.5); // morning
		menu_action = TABS_VIEWOBJ;
	}
}

void VtxPlanetTabs::OnUpdateTemp(wxUpdateUIEvent &event) {
	//setTemp();
}

void VtxPlanetTabs::OnUpdateViewObj(wxUpdateUIEvent &event) {
	event.Check(is_viewobj());
}

void VtxPlanetTabs::setTemp() {
	Planetoid *obj = (Planetoid*) object();
	static wxString oldtstr;
 	char temp_str[256]={0};
	obj->getTempString(temp_str);
	wxString tstr(temp_str);
     
    if(oldtstr!=tstr){
    	//cout<<temp_str<<endl;
		temp_state->SetValue(temp_str);
		oldtstr=tstr;
    }
 
}
void VtxPlanetTabs::updateControls() {
	if (changing)
		return;
	Planetoid *obj = (Planetoid*) object();

	setTemp();

	updateSlider(CellSizeSlider, obj->detail);
	updateSlider(SizeSlider, obj->size / MILES/1000);
	updateSlider(OrbitRadiusSlider, obj->orbit_radius);
	updateSlider(OrbitPhaseSlider, obj->orbit_phase);
	updateSlider(OrbitTiltSlider, obj->orbit_skew);
	updateSlider(TiltSlider, obj->tilt);
	updateSlider(RotPhaseSlider, obj->rot_phase);
	updateSlider(DaySlider, obj->day);
	updateSlider(YearSlider, obj->year);
	updateSlider(ShineSlider, obj->shine);
	updateSlider(AlbedoSlider, obj->albedo);
	updateSlider(SeasonSlider, obj->season_factor);
	updateSlider(TempSlider, obj->temp_factor);
	updateSlider(HscaleSlider, obj->hscale * 1e-3/MILES);

	updateColor(AmbientSlider, obj->ambient);
	updateColor(EmissionSlider, obj->emission);
	updateColor(SpecularSlider, obj->specular);
	updateColor(DiffuseSlider, obj->diffuse);
	updateColor(ShadowSlider, obj->shadow_color);
	seasonal->SetValue(obj->seasonal);

	object_name->SetValue(object_node->node->nodeName());
}
