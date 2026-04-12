#include "VtxAsteroidTabs.h"
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

	ID_ORBIT_RADIUS_SLDR,
	ID_ORBIT_RADIUS_TEXT,
	ID_ORBIT_PHASE_SLDR,
	ID_ORBIT_PHASE_TEXT,
	ID_ORBIT_TILT_SLDR,
	ID_ORBIT_TILT_TEXT,
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
	ID_NOISE_EXPR,
	ID_MAXDEPTH,
	ID_CLIPTEST,
	ID_BACKTEST

};

//########################### VtxAsteroidTabs Class ########################

IMPLEMENT_CLASS(VtxAsteroidTabs, wxNotebook)

BEGIN_EVENT_TABLE(VtxAsteroidTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxAsteroidTabs::OnNameText)
EVT_TEXT_ENTER(ID_NOISE_EXPR,OnChangedNoiseExpr)
SET_SLIDER_EVENTS(HSCALE,VtxAsteroidTabs,Hscale)

SET_SLIDER_EVENTS(CELLSIZE,VtxAsteroidTabs,CellSize)
SET_SLIDER_EVENTS(RADIUS,VtxAsteroidTabs,Size)

SET_SLIDER_EVENTS(DAY,VtxAsteroidTabs,Day)
SET_SLIDER_EVENTS(YEAR,VtxAsteroidTabs,Year)
SET_SLIDER_EVENTS(ROT_PHASE,VtxAsteroidTabs,RotPhase)
SET_SLIDER_EVENTS(ORBIT_RADIUS,VtxAsteroidTabs,OrbitRadius)
SET_SLIDER_EVENTS(ORBIT_TILT,VtxAsteroidTabs,OrbitTilt)
SET_SLIDER_EVENTS(ORBIT_PHASE,VtxAsteroidTabs,OrbitPhase)
SET_SLIDER_EVENTS(SHINE,VtxAsteroidTabs,Shine)
SET_SLIDER_EVENTS(ALBEDO,VtxAsteroidTabs,Albedo)

SET_COLOR_EVENTS(AMBIENT,VtxAsteroidTabs,Ambient)
SET_COLOR_EVENTS(EMISSION,VtxAsteroidTabs,Emission)
SET_COLOR_EVENTS(SPECULAR,VtxAsteroidTabs,Specular)
SET_COLOR_EVENTS(DIFFUSE,VtxAsteroidTabs,Diffuse)
SET_COLOR_EVENTS(SHADOW,VtxAsteroidTabs,Shadow)

EVT_CHOICE(ID_MAXDEPTH, VtxAsteroidTabs::OnMaxDepth)
EVT_CHECKBOX(ID_CLIPTEST,VtxAsteroidTabs::OnChangedFlags)
EVT_CHECKBOX(ID_BACKTEST,VtxAsteroidTabs::OnChangedFlags)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxAsteroidTabs::OnAddItem)
EVT_MENU(OBJ_DELETE,VtxAsteroidTabs::OnDelete)

EVT_MENU(OBJ_SHOW,VtxAsteroidTabs::OnEnable)
EVT_UPDATE_UI(OBJ_SHOW,VtxAsteroidTabs::OnUpdateEnable)

EVT_MENU(OBJ_VIEWOBJ,VtxAsteroidTabs::OnViewObj)
EVT_UPDATE_UI(OBJ_VIEWOBJ,VtxAsteroidTabs::OnUpdateViewObj)

SET_FILE_EVENTS(VtxAsteroidTabs)

END_EVENT_TABLE()

VtxAsteroidTabs::VtxAsteroidTabs(wxWindow *parent, wxWindowID id,
		const wxPoint &pos, const wxSize &size, long style,
		const wxString &name) :
		VtxTabsMgr(parent, id, pos, size, style, name) {
	Create(parent, id, pos, size, style, name);
}

int VtxAsteroidTabs::showMenu(bool expanded) {
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

bool VtxAsteroidTabs::Create(wxWindow *parent, wxWindowID id, const wxPoint &pos,
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
void VtxAsteroidTabs::AddObjectTab(wxWindow *panel) {
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);

	wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT | wxALL, 6);

	wxBoxSizer *object_cntrls = new wxStaticBoxSizer(wxVERTICAL, panel,
			wxT("Object"));

	
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name = new TextCtrl(panel, ID_NAME_TEXT, "Name", 40,100);
	hline->Add(object_name->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);
	
	wxStaticText *lbl=new wxStaticText(panel,-1,"MaxDepth",wxDefaultPosition,wxSize(-1,-1));
	hline->Add(lbl, 5, wxALIGN_LEFT|wxTop|wxLEFT,0);
	
	wxString depths[]={"1","2","3","4","5","6","7","8","9","10","11","12"};	

    m_maxDepth=new wxChoice(panel, ID_MAXDEPTH, wxDefaultPosition,wxSize(40,-1),12, depths);
	m_maxDepth->SetSelection(8);

	hline->Add(m_maxDepth, 5, wxALIGN_LEFT | wxALL, 5);
	
	wxStaticText *tests=new wxStaticText(panel,-1,"Tests",wxDefaultPosition,wxSize(-1,-1));
	hline->Add(tests, 5, wxALIGN_LEFT|wxTop|wxLEFT,0);
	
	m_clip=new wxCheckBox(panel, ID_CLIPTEST, "Fustrum");
	m_clip->SetToolTip("Decrease cells in off-screen areas");
	hline->Add(m_clip,5,wxALIGN_LEFT|wxALL,5);

	m_backface=new wxCheckBox(panel, ID_BACKTEST, "Interior");
	m_backface->SetToolTip("Decrease cells in interior");
	hline->Add(m_backface,5,wxALIGN_LEFT|wxALL,5);

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

	wxStaticBoxSizer* shape = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Shape"));

	hline = new wxBoxSizer(wxHORIZONTAL);
	HscaleSlider = new SliderCtrl(panel, ID_HSCALE_SLDR, "Scale", LABEL2B,
			VALUE2, SLIDER2);
	HscaleSlider->setRange(0.0, 1);
	HscaleSlider->setValue(0.5);

	hline->Add(HscaleSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);
	
	NoiseExpr=new ExprTextCtrl(panel,ID_NOISE_EXPR,"Noise",LABEL2S,140);
	hline->Add(NoiseExpr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
	
	shape->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(shape, 0, wxALIGN_LEFT | wxALL, 0);

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
	
}
void VtxAsteroidTabs::AddLightingTab(wxWindow *panel) {
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

void VtxAsteroidTabs::OnViewObj(wxCommandEvent &event) {
	if (!is_viewobj()) {
		TheScene->set_viewobj(object());
		TheScene->selobj = object();
		TheScene->change_view(ORBITAL);
		menu_action = TABS_VIEWOBJ;
	}
}


void VtxAsteroidTabs::OnUpdateViewObj(wxUpdateUIEvent &event) {
	event.Check(is_viewobj());
}


void VtxAsteroidTabs::updateControls() {
	if (changing)
		return;
	Asteroid *obj = (Asteroid*) object();
	
	updateSlider(CellSizeSlider, obj->detail);
	updateSlider(SizeSlider, obj->size / MILES/1000);
	updateSlider(OrbitRadiusSlider, obj->orbit_radius);
	updateSlider(OrbitPhaseSlider, obj->orbit_phase);
	updateSlider(OrbitTiltSlider, obj->orbit_skew);
	updateSlider(RotPhaseSlider, obj->rot_phase);
	updateSlider(DaySlider, obj->day);
	updateSlider(YearSlider, obj->year);
	updateSlider(ShineSlider, obj->shine);
	updateSlider(AlbedoSlider, obj->albedo);
	updateSlider(HscaleSlider, obj->hscale);

	updateColor(AmbientSlider, obj->ambient);
	updateColor(EmissionSlider, obj->emission);
	updateColor(SpecularSlider, obj->specular);
	updateColor(DiffuseSlider, obj->diffuse);
	updateColor(ShadowSlider, obj->shadow_color);
	
	
	m_maxDepth->SetSelection(obj->maxDepth-1);
	
	m_clip->SetValue(obj->cliptest);
	m_backface->SetValue(obj->backtest);

		
	char buff[256];
	obj->getNoiseFunction(buff);
	NoiseExpr->SetValue(buff);

	object_name->SetValue(object_node->node->nodeName());
}
