
#include "VtxWaterTabs.h"
#include "VtxSceneDialog.h"
#include "RenderOptions.h"

#include <wx/filefn.h>
#include <wx/dir.h>

static wxString types[]={"Nitrogen","Methane","CO2", "SO2","Water"};
static double gas_temps[]={-196,-163,-78,-10,100};
static double solid_temps[]={-210,-182,-79,-72,0,};

//########################### VtxWaterTabs Class ########################
enum{
	ID_NAME_TEXT,
	ID_PLANET_TEMP_TEXT,
	ID_AUTO_STATE,
	ID_SHOW,
	ID_COMPOSITION,
	ID_SHOW_STATE,
    ID_LEVEL_SLDR,
    ID_LEVEL_TEXT,
    ID_OCEAN_FUNCTION_TEXT,
	ID_DEFAULT_MOD,
	ID_DEFAULT_LIQUID,
	ID_DEFAULT_SOLID,
    ID_LIQUID_SURFACE_TEXT,
    ID_LIQUID_TRANSMIT_SLDR,
    ID_LIQUID_TRANSMIT_TEXT,
    ID_LIQUID_TRANSMIT_COLOR,
    ID_LIQUID_REFLECT_SLDR,
    ID_LIQUID_REFLECT_TEXT,
    ID_LIQUID_REFLECT_COLOR,
    ID_LIQUID_SHINE_SLDR,
    ID_LIQUID_SHINE_TEXT,
    ID_LIQUID_ALBEDO_SLDR,
    ID_LIQUID_ALBEDO_TEXT,
    ID_LIQUID_TEMP_SLDR,
    ID_LIQUID_TEMP_TEXT,
    ID_LIQUID_TRANS_TEMP_SLDR,
    ID_LIQUID_TRANS_TEMP_TEXT,
    ID_LIQUID_MIX_SLDR,
    ID_LIQUID_MIX_TEXT,

    ID_SOLID_SURFACE_TEXT,
    ID_SOLID_TRANSMIT_SLDR,
    ID_SOLID_TRANSMIT_TEXT,
    ID_SOLID_TRANSMIT_COLOR,
    ID_SOLID_REFLECT_SLDR,
    ID_SOLID_REFLECT_TEXT,
    ID_SOLID_REFLECT_COLOR,
    ID_SOLID_SHINE_SLDR,
    ID_SOLID_SHINE_TEXT,
    ID_SOLID_ALBEDO_SLDR,
    ID_SOLID_ALBEDO_TEXT,
    ID_SOLID_TEMP_SLDR,
    ID_SOLID_TEMP_TEXT,
    ID_SOLID_TRANS_TEMP_SLDR,
    ID_SOLID_TRANS_TEMP_TEXT,
    ID_SOLID_MIX_SLDR,
    ID_SOLID_MIX_TEXT,


};

#define EXPR_WIDTH  140

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

//########################### VtxWaterTabs Class ########################

IMPLEMENT_CLASS(VtxWaterTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxWaterTabs, VtxTabsMgr)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxWaterTabs::OnNameText)

EVT_CHECKBOX(ID_AUTO_STATE,VtxWaterTabs::OnAutoState)


SET_SLIDER_EVENTS(LEVEL,VtxWaterTabs,Level)

EVT_RADIOBOX(ID_SHOW_STATE, VtxWaterTabs::OnSetState)
SET_COLOR_EVENTS(LIQUID_TRANSMIT,VtxWaterTabs,LiquidTransmit)
SET_COLOR_EVENTS(LIQUID_REFLECT,VtxWaterTabs,LiquidReflect)
SET_SLIDER_EVENTS(LIQUID_SHINE,VtxWaterTabs,LiquidShine)
SET_SLIDER_EVENTS(LIQUID_ALBEDO,VtxWaterTabs,LiquidAlbedo)
SET_SLIDER_EVENTS(LIQUID_TEMP,VtxWaterTabs,LiquidTemp)
SET_SLIDER_EVENTS(LIQUID_TRANS_TEMP,VtxWaterTabs,LiquidTransTemp)
SET_SLIDER_EVENTS(LIQUID_MIX,VtxWaterTabs,LiquidMix)

EVT_TEXT_ENTER(ID_LIQUID_SURFACE_TEXT,VtxWaterTabs::OnSurfaceFunctionEnter)

SET_COLOR_EVENTS(SOLID_TRANSMIT,VtxWaterTabs,SolidTransmit)
SET_COLOR_EVENTS(SOLID_REFLECT,VtxWaterTabs,SolidReflect)
SET_SLIDER_EVENTS(SOLID_SHINE,VtxWaterTabs,SolidShine)
SET_SLIDER_EVENTS(SOLID_ALBEDO,VtxWaterTabs,SolidAlbedo)
SET_SLIDER_EVENTS(SOLID_TEMP,VtxWaterTabs,SolidTemp)
SET_SLIDER_EVENTS(SOLID_TRANS_TEMP,VtxWaterTabs,SolidTransTemp)
SET_SLIDER_EVENTS(SOLID_MIX,VtxWaterTabs,SolidMix)

EVT_TEXT_ENTER(ID_SOLID_SURFACE_TEXT,VtxWaterTabs::OnSurfaceFunctionEnter)

EVT_TEXT_ENTER(ID_OCEAN_FUNCTION_TEXT,VtxWaterTabs::OnSurfaceFunctionEnter)

EVT_BUTTON(ID_DEFAULT_MOD,VtxWaterTabs::OnSetDefaultMod)
EVT_BUTTON(ID_DEFAULT_LIQUID,VtxWaterTabs::OnSetDefaultLiquid)
EVT_BUTTON(ID_DEFAULT_SOLID,VtxWaterTabs::OnSetDefaultSolid)
EVT_CHOICE(ID_COMPOSITION, VtxWaterTabs::OnChangeComposition)

EVT_MENU(ID_SHOW,VtxWaterTabs::OnEnable)
EVT_UPDATE_UI(ID_SHOW, VtxWaterTabs::OnUpdateEnable)
EVT_UPDATE_UI(ID_PLANET_TEMP_TEXT, VtxWaterTabs::OnUpdateState)
//EVT_UPDATE_UI(ID_AUTO_STATE, VtxWaterTabs::OnUpdateState)


EVT_MENU(TABS_DELETE,VtxWaterTabs::OnDelete)

SET_FILE_EVENTS(VtxWaterTabs)

END_EVENT_TABLE()


VtxWaterTabs::VtxWaterTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxWaterTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
 	AddPropertiesTab(page);
    AddPage(page,wxT("Properties"),true);
    
	page = new wxPanel(this, wxID_ANY);
	AddSolidTab(page);
	AddPage(page,wxT("Solid"),false);

	page = new wxPanel(this, wxID_ANY);
	AddLiquidTab(page);
	AddPage(page,wxT("Liquid"),false);


    return true;
}

void VtxWaterTabs::AddPropertiesTab(wxWindow *panel){

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);

	wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT | wxALL, 6);

	wxBoxSizer *object_cntrls = new wxStaticBoxSizer(wxVERTICAL, panel,
			wxT("Object"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name = new TextCtrl(panel, ID_NAME_TEXT, "Name", 70,
			VALUE2 + SLIDER2);
	hline->Add(object_name->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);
	//hline->AddSpacer(10);

	LevelSlider=new SliderCtrl(panel,ID_LEVEL_SLDR,"Level",LABEL2,60,150);
	LevelSlider->setRange(-10000,10000);

	hline->Add(LevelSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	object_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	boxSizer->Add(object_cntrls, 0, wxALIGN_LEFT | wxALL, 0);

	hline = new wxBoxSizer(wxHORIZONTAL);

    wxString modes[]={"Gas", "Liquid","Solid"};

    State=new wxRadioBox(panel,ID_SHOW_STATE,wxT(""),wxPoint(-1,-1),wxSize(-1,40),3,
       		modes,3,wxRA_SPECIFY_COLS);
    hline->Add(State, 0, wxALIGN_LEFT | wxALL, 0);


	wxBoxSizer *default_state = new wxStaticBoxSizer(wxHORIZONTAL, panel,
			wxT(""));

	auto_state=new wxCheckBox(panel, ID_AUTO_STATE, "Auto");

	default_state->Add(auto_state, 0, wxALIGN_LEFT | wxALL, 5);

	planet_temp=new StaticTextCtrl(panel,ID_PLANET_TEMP_TEXT,"",0,200);

	default_state->Add(planet_temp->getSizer(), 0, wxALIGN_LEFT | wxALL, 2);
	hline->Add(default_state, 0, wxALIGN_LEFT|wxTOP, 0);

    boxSizer->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	wxBoxSizer *presets = new wxStaticBoxSizer(wxHORIZONTAL, panel,
			wxT("Presets"));

	//hline = new wxBoxSizer(wxHORIZONTAL);

	Composition=new wxChoice(panel, ID_COMPOSITION, wxDefaultPosition,wxSize(95,-1),5, types);
	Composition->SetSelection(4);
	presets->Add(Composition, 0, wxALIGN_LEFT | wxALL, 2);

	OceanFunction=new ExprTextCtrl(panel,ID_OCEAN_FUNCTION_TEXT,"Modulation",80,150);

	presets->Add(OceanFunction->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	default_mod=new wxButton(panel,ID_DEFAULT_MOD,"Default",wxDefaultPosition,wxSize(50,25));
	presets->Add(default_mod,0,wxALIGN_LEFT|wxALL,0);
	presets->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));

    boxSizer->Add(presets, 0, wxALIGN_LEFT | wxALL, 0);

}
void VtxWaterTabs::AddLiquidTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* surface = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Surface"));
	LiquidFunction=new ExprTextCtrl(panel,ID_LIQUID_SURFACE_TEXT,"Modulation",LABEL2,EXPR_WIDTH);
	surface->Add(LiquidFunction->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
	
	default_liquid=new wxButton(panel,ID_DEFAULT_LIQUID,"Default",wxDefaultPosition,wxSize(60,25));
	surface->Add(default_liquid,0,wxALIGN_LEFT|wxALL,0);

	LiquidMixSlider = new SliderCtrl(panel,ID_LIQUID_MIX_SLDR,"Mix",30,VALUE,SLIDER2);
	LiquidMixSlider->setRange(0,1.0);
	surface->Add(LiquidMixSlider->getSizer(), 0, wxALIGN_LEFT|wxLEFT,5);

	surface->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(surface, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* transmission = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Transparency"));

    LiquidTransmitSlider = new ColorSlider(panel,ID_LIQUID_TRANSMIT_SLDR,"feet",LABEL,VALUE,CSLIDER,CBOX1);
    LiquidTransmitSlider->setRange(1,1000);
    transmission->Add(LiquidTransmitSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    transmission->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(transmission, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* reflection = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Reflection"));

    LiquidReflectSlider = new ColorSlider(panel,ID_LIQUID_REFLECT_SLDR,"%",LABEL,VALUE,CSLIDER,CBOX1);
    LiquidReflectSlider->setRange(0,1);
    reflection->Add(LiquidReflectSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    reflection->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(reflection, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* specular = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Specular"));

    LiquidShineSlider = new SliderCtrl(panel,ID_LIQUID_SHINE_SLDR,"shine",LABEL2,VALUE,SLIDER2);
    LiquidShineSlider->setRange(0,100.0);
    specular->Add(LiquidShineSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

    LiquidAlbedoSlider = new SliderCtrl(panel,ID_LIQUID_ALBEDO_SLDR,"albedo",LABEL2S,VALUE,SLIDER2);
    LiquidAlbedoSlider->setRange(0,1.0);
    specular->Add(LiquidAlbedoSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

    specular->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(specular, 0, wxALIGN_LEFT|wxALL,0);
    
    wxStaticBoxSizer* temperature = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Temperature"));

	LiquidTempSlider = new SliderCtrl(panel, ID_LIQUID_TEMP_SLDR, "Gas", LABEL2,VALUE,SLIDER2);
	LiquidTempSlider->setRange(-1000, 1000);
	LiquidTempSlider->setValue(100.0);
	temperature->Add(LiquidTempSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);


	LiquidTransTempSlider = new SliderCtrl(panel,ID_LIQUID_TRANS_TEMP_SLDR,"Trans",LABEL2S,VALUE,SLIDER2);
	LiquidTransTempSlider->setRange(0,10.0);
	temperature->Add(LiquidTransTempSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	temperature->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(temperature, 0, wxALIGN_LEFT|wxALL,0);

}

void VtxWaterTabs::AddSolidTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* surface = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Surface"));
	SolidFunction=new ExprTextCtrl(panel,ID_SOLID_SURFACE_TEXT,"Modulation",LABEL2,EXPR_WIDTH);
	surface->Add(SolidFunction->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
	
	default_solid=new wxButton(panel,ID_DEFAULT_SOLID,"Default",wxDefaultPosition,wxSize(60,25));
	surface->Add(default_solid,0,wxALIGN_LEFT|wxALL,0);

	SolidMixSlider = new SliderCtrl(panel,ID_SOLID_MIX_SLDR,"Mix",30,VALUE,SLIDER2);
	SolidMixSlider->setRange(0,1.0);
	surface->Add(SolidMixSlider->getSizer(), 0, wxALIGN_LEFT|wxLEFT,5);

	surface->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(surface, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* transmission = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Transparency"));

    SolidTransmitSlider = new ColorSlider(panel,ID_SOLID_TRANSMIT_SLDR,"feet",LABEL,VALUE,CSLIDER,CBOX1);
    SolidTransmitSlider->setRange(1,1000);
    transmission->Add(SolidTransmitSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    transmission->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(transmission, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* reflection = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Reflection"));

    SolidReflectSlider = new ColorSlider(panel,ID_SOLID_REFLECT_SLDR,"%",LABEL,VALUE,CSLIDER,CBOX1);
    SolidReflectSlider->setRange(0,1);
    reflection->Add(SolidReflectSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    reflection->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(reflection, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* specular = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Specular"));

    SolidShineSlider = new SliderCtrl(panel,ID_SOLID_SHINE_SLDR,"shine",LABEL2,VALUE,SLIDER2);
    SolidShineSlider->setRange(0,100.0);
    specular->Add(SolidShineSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

    SolidAlbedoSlider = new SliderCtrl(panel,ID_SOLID_ALBEDO_SLDR,"albedo",LABEL2S,VALUE,SLIDER2);
    SolidAlbedoSlider->setRange(0,1.0);
    specular->Add(SolidAlbedoSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

    specular->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(specular, 0, wxALIGN_LEFT|wxALL,0);
 
    wxStaticBoxSizer* temperature = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Temperature"));

	SolidTempSlider = new SliderCtrl(panel, ID_SOLID_TEMP_SLDR, "Liquid", LABEL2,VALUE,SLIDER2);
	SolidTempSlider->setRange(-1000, 1000);
	SolidTempSlider->setValue(0.0);
	temperature->Add(SolidTempSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);


	SolidTransTempSlider = new SliderCtrl(panel,ID_SOLID_TRANS_TEMP_SLDR,"Trans",LABEL2S,VALUE,SLIDER2);
	SolidTransTempSlider->setRange(0,10.0);
	temperature->Add(SolidTransTempSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	temperature->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(temperature, 0, wxALIGN_LEFT|wxALL,0);

    
}

int VtxWaterTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_SHOW,wxT("Enable"));
	menu.Append(TABS_DELETE,wxT("Delete"));
	sceneDialog->AddFileMenu(menu,object_node->node);
	PopupMenu(&menu);
	return menu_action;
}

void VtxWaterTabs::OnUpdateState(wxUpdateUIEvent &event) {
	getDefaultState();
	bool auto_state_enabled=auto_state->IsChecked();
	Planetoid *orb=getOrbital();
    int state=orb->ocean_state;
    static int oldstate=-1;
	if(auto_state_enabled && oldstate!=state)
		State->SetSelection(state);
	oldstate=state;
}

void VtxWaterTabs::getDefaultState() {
    static wxString oldtstr;
	Planetoid *obj=getOrbital();
   // obj->calcTemperature();
    char state_str[256];
	char type_str[256];
    double solid_temp=SolidTempSlider->getValue();
    double liquid_temp=LiquidTempSlider->getValue();
	double tc=obj->surface_temp-273;

	if(tc<=solid_temp)
		strcpy(state_str,"Solid");
	else if(tc<=liquid_temp)
		strcpy(state_str,"Liquid");
	else
		strcpy(state_str,"Gas");
	sprintf(type_str,"%d C (%s)",(int)tc,state_str);
	wxString tstr(type_str);
	if(oldtstr!=tstr)
		planet_temp->SetValue(type_str);
	oldtstr=tstr;
}

void VtxWaterTabs::OnAutoState(wxCommandEvent& event){
	Planetoid *orb=getOrbital();
	bool autoset=event.IsChecked();
	orb->ocean_auto=autoset;
	orb->calcAveTemperature();
}

void VtxWaterTabs::OnChangeComposition(wxCommandEvent& event){
    int n = Composition->GetSelection();
    double solid=solid_temps[n];
    double liquid=gas_temps[n];
    SolidTempSlider->setValue(solid);
    LiquidTempSlider->setValue(liquid);
	Planetoid *orb=getOrbital();
	OceanState *ocean=orb->getOcean();

	ocean->setOceanGasTemp(LiquidTempSlider->getValue()+273);
	ocean->setOceanSolidTemp(SolidTempSlider->getValue()+273);
	orb->calcAveTemperature();
	orb->invalidate();
	TheScene->rebuild();
}

void VtxWaterTabs::OnSetState(wxCommandEvent& event){
	int state=event.GetSelection();
	Planetoid *orb=getOrbital();
	bool autoset=auto_state->IsChecked();
	orb->ocean_auto=autoset;
	int oldstate=orb->ocean_state;
	if(!autoset){
		orb->ocean_state=state;
		if(oldstate!=state){
			orb->invalidate();
			TheScene->rebuild();
		}
    }
	orb->calcAveTemperature();
}

void VtxWaterTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

void VtxWaterTabs::OnEnable(wxCommandEvent& event){
	setEnabled(!isEnabled());
	Render.set_water_show(!Render.show_water());
	TheScene->set_changed_detail();
	TheScene->rebuild_all();
	menu_action=TABS_ENABLE;
	sceneDialog->updateObjectTree();
}
void VtxWaterTabs::OnUpdateEnable(wxUpdateUIEvent& event) {
 	event.Check(Render.show_water());
}

void VtxWaterTabs::OnSetDefaultMod(wxCommandEvent& event){
	Planetoid *orb=getOrbital();
	OceanFunction->SetValue(OceanState::getDfltOceanExpr());
	setObjAttributes();
	orb->invalidate();
	TheScene->rebuild();
}

void VtxWaterTabs::OnSetDefaultLiquid(wxCommandEvent& event){
	Planetoid *orb=getOrbital();
	LiquidFunction->SetValue(OceanState::getDfltOceanLiquidExpr());
	setObjAttributes();
	orb->invalidate();
	TheScene->rebuild();
}
void VtxWaterTabs::OnSetDefaultSolid(wxCommandEvent& event){
	Planetoid *orb=getOrbital();
	SolidFunction->SetValue(OceanState::getDfltOceanSolidExpr());
	setObjAttributes();
	orb->invalidate();
	TheScene->rebuild();
}
//-------------------------------------------------------------
// VtxWaterTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxWaterTabs::setObjAttributes(){
	update_needed=true;
	TNwater *tnode=water();
	char tstr[256]={0};

	Planetoid *orb=getOrbital();
	OceanState *ocean=orb->getOcean();
	
	char *s=object_name->GetValue().ToAscii();
	if(s)
		ocean->setOceanName(s);

	int state=State->GetSelection();

    orb->ocean_state=state;
	orb->ocean_level=LevelSlider->getValue()*FEET;

    ocean->setOceanGasTemp(LiquidTempSlider->getValue()+273);
    ocean->setOceanSolidTemp(SolidTempSlider->getValue()+273);
    ocean->setOceanGasTransTemp(LiquidTransTempSlider->getValue());
    ocean->setOceanSolidTransTemp(SolidTransTempSlider->getValue());
    Color wc=LiquidReflectSlider->getColor();
    wc.set_alpha(LiquidReflectSlider->getValue());
    wc.toString(tstr);

    ocean->setWaterColor1(wc);
 	wc=LiquidTransmitSlider->getColor();
	ocean->setWaterColor2(wc);
	ocean->setWaterClarity(LiquidTransmitSlider->getValue());
	ocean->setWaterShine(LiquidShineSlider->getValue());
	ocean->setWaterSpecular(LiquidAlbedoSlider->getValue());
	ocean->setWaterMix(LiquidMixSlider->getValue());
	
    wc=SolidReflectSlider->getColor();
    ocean->setIceColor1(wc);
    ocean->setIceColor2(SolidTransmitSlider->getColor());
    ocean->setIceClarity(SolidTransmitSlider->getValue());
    ocean->setIceShine(SolidShineSlider->getValue());
    ocean->setIceSpecular(SolidAlbedoSlider->getValue());
    ocean->setIceMix(SolidMixSlider->getValue());

    orb->setOceanFunction((char*)OceanFunction->GetValue().ToAscii());
    
    ocean->setOceanLiquidExpr((char*)LiquidFunction->GetValue().ToAscii());   
    ocean->setOceanSolidExpr((char*)SolidFunction->GetValue().ToAscii());

    tnode->setNoiseExprs(ocean);
 
	invalidateObject();

}
//-------------------------------------------------------------
// VtxWaterTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxWaterTabs::getObjAttributes(){
	if(!update_needed)
		return;
	Planetoid *orb=getOrbital();
	TNwater *tnode=water();
	
	OceanState *ocean=orb->getOcean();

	TNarg *arg=(TNarg*)tnode->left;

	LiquidFunction->SetValue(ocean->getOceanLiquidExpr());
	SolidFunction->SetValue(ocean->getOceanSolidExpr());

	if(arg){
		LiquidFunction->SetValue(arg);
		ocean->setOceanLiquidExpr((char *)LiquidFunction->GetValue().ToAscii());
		arg=arg->next();
		if(arg){
 			SolidFunction->SetValue(arg);
 			ocean->setOceanSolidExpr((char *)SolidFunction->GetValue().ToAscii());
		}
	}
 	char buff[256];
 	orb->getOceanFunction(buff);

	OceanFunction->SetValue(buff);

	State->SetSelection(orb->ocean_state);

	object_name->SetValue(ocean->getOceanName());
	LiquidTempSlider->setValue(ocean->oceanGasTemp()-273);
	SolidTempSlider->setValue(ocean->oceanSolidTemp()-273);
	
	LiquidTransTempSlider->setValue(ocean->oceanGasTransTemp());
	SolidTransTempSlider->setValue(ocean->oceanSolidTransTemp());

	LevelSlider->setValue(orb->ocean_level/FEET);

	LiquidTransmitSlider->setColor(ocean->waterColor2());
	LiquidTransmitSlider->setValue(ocean->waterClarity()/FEET);
	LiquidReflectSlider->setColor(ocean->waterColor1());
	LiquidReflectSlider->setValue(ocean->waterColor1().alpha());

	LiquidShineSlider->setValue(ocean->waterShine());
	LiquidAlbedoSlider->setValue(ocean->waterSpecular());
	LiquidMixSlider->setValue(ocean->waterMix());

	SolidTransmitSlider->setColor(ocean->iceColor2());
	SolidTransmitSlider->setValue(ocean->iceClarity()/FEET);
	SolidReflectSlider->setColor(ocean->iceColor1());
	SolidReflectSlider->setValue(ocean->iceColor1().alpha());

	SolidShineSlider->setValue(ocean->iceShine());
	SolidAlbedoSlider->setValue(ocean->iceSpecular());
	SolidMixSlider->setValue(ocean->iceMix());
	auto_state->SetValue(orb->ocean_auto);

	update_needed=false;

}
