
#include "VtxWaterTabs.h"
#include "VtxSceneDialog.h"
#include "RenderOptions.h"

#include <wx/filefn.h>
#include <wx/dir.h>

enum {GAS=0,LIQUID=1,SOLID,AUTO=2};

static char*    def_water_func="noise(GRADIENT|SCALE|RO1,16,3,-0.02,0.5,2,0.05,1,0,0)";
static char*    def_ice_func="noise(GRADIENT|NABS|SCALE|SQR|RO1,15.2,8.6,0.1,0.4,1.84,0.73,-0.34,0,0)";
static char*    def_transition_func="noise(GRADIENT|NNORM|SCALE|RO1,5,9.5,1,0.5,2,0.4,1,0,0)";

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

EVT_TEXT_ENTER(ID_LIQUID_SURFACE_TEXT,VtxWaterTabs::OnSurfaceFunctionEnter)

SET_COLOR_EVENTS(SOLID_TRANSMIT,VtxWaterTabs,SolidTransmit)
SET_COLOR_EVENTS(SOLID_REFLECT,VtxWaterTabs,SolidReflect)
SET_SLIDER_EVENTS(SOLID_SHINE,VtxWaterTabs,SolidShine)
SET_SLIDER_EVENTS(SOLID_ALBEDO,VtxWaterTabs,SolidAlbedo)
SET_SLIDER_EVENTS(SOLID_TEMP,VtxWaterTabs,SolidTemp)

EVT_TEXT_ENTER(ID_SOLID_SURFACE_TEXT,VtxWaterTabs::OnSurfaceFunctionEnter)

EVT_TEXT_ENTER(ID_OCEAN_FUNCTION_TEXT,VtxWaterTabs::OnSurfaceFunctionEnter)

EVT_BUTTON(ID_DEFAULT_MOD,VtxWaterTabs::OnSetDefaultMod)

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
	AddLiquidTab(page);
	AddPage(page,wxT("Liquid"),false);

	page = new wxPanel(this, wxID_ANY);
	AddSolidTab(page);
	AddPage(page,wxT("Solid"),false);

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

	wxBoxSizer *temp_cntrls = new wxStaticBoxSizer(wxVERTICAL, panel,
			wxT("Temperature (C)"));


	hline = new wxBoxSizer(wxHORIZONTAL);

	Composition=new wxChoice(panel, ID_COMPOSITION, wxDefaultPosition,wxSize(95,-1),5, types);
	Composition->SetSelection(4);

	hline->Add(Composition, 0, wxALIGN_LEFT | wxALL, 0);

	LiquidTempSlider = new SliderCtrl(panel, ID_LIQUID_TEMP_SLDR, "Gas", 30,
			VALUE2, 80);
	LiquidTempSlider->setRange(-1000, 1000);
	LiquidTempSlider->setValue(100.0);

	hline->Add(LiquidTempSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	SolidTempSlider = new SliderCtrl(panel, ID_SOLID_TEMP_SLDR, "Liquid", 50,
			VALUE2, 80);
	SolidTempSlider->setRange(-1000, 1000);
	SolidTempSlider->setValue(0.0);

	hline->Add(SolidTempSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);
	temp_cntrls->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);
	boxSizer->Add(temp_cntrls, 0, wxALIGN_LEFT | wxALL, 0);

	hline = new wxBoxSizer(wxHORIZONTAL);

    wxString modes[]={"Gas", "Liquid","Solid"};

    State=new wxRadioBox(panel,ID_SHOW_STATE,wxT("State"),wxPoint(-1,-1),wxSize(-1,-1),3,
       		modes,3,wxRA_SPECIFY_COLS);
    hline->Add(State, 0, wxALIGN_LEFT | wxALL, 0);


	wxBoxSizer *default_state = new wxStaticBoxSizer(wxHORIZONTAL, panel,
			wxT(""));

	auto_state=new wxCheckBox(panel, ID_AUTO_STATE, "Auto");

	default_state->Add(auto_state, 0, wxALIGN_LEFT | wxALL, 0);

	planet_temp=new StaticTextCtrl(panel,ID_PLANET_TEMP_TEXT,"",0,200);

	default_state->Add(planet_temp->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);
	hline->Add(default_state, 0, wxALIGN_LEFT | wxALL, 0);

    boxSizer->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	OceanFunction=new ExprTextCtrl(panel,ID_OCEAN_FUNCTION_TEXT,"Modulation",100,280);

	hline->Add(OceanFunction->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	default_mod=new wxButton(panel,ID_DEFAULT_MOD,"Default",wxDefaultPosition,wxSize(60,25));
	hline->Add(default_mod,0,wxALIGN_LEFT|wxALL,2);


	//default_state->Add(OceanFunction->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);


    boxSizer->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);


}
void VtxWaterTabs::AddLiquidTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* surface = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Surface"));
	LiquidFunction=new ExprTextCtrl(panel,ID_LIQUID_SURFACE_TEXT,"waves",LABEL2S,EXPR_WIDTH);
	surface->Add(LiquidFunction->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
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
}

void VtxWaterTabs::AddSolidTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* surface = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Surface"));
	SolidFunction=new ExprTextCtrl(panel,ID_SOLID_SURFACE_TEXT,"function",LABEL2S,EXPR_WIDTH);
	surface->Add(SolidFunction->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
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

	if(auto_state_enabled)
		State->SetSelection(orb->ocean_state);
}

void VtxWaterTabs::getDefaultState() {

	Planetoid *obj=getOrbital();
   // obj->calcTemperature();
    char state_str[256];
	char type_str[256];
    double solid_temp=SolidTempSlider->getValue();
    double liquid_temp=LiquidTempSlider->getValue();
	double tc=obj->temperature-273;

	if(tc<=solid_temp)
		strcpy(state_str,"Solid");
	else if(tc<=liquid_temp)
		strcpy(state_str,"Liquid");
	else
		strcpy(state_str,"Gas");
	sprintf(type_str,"%d C (%s)",(int)tc,state_str);
	planet_temp->SetValue(type_str);
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
	orb->ocean_liquid_temp=LiquidTempSlider->getValue();
	orb->ocean_solid_temp=SolidTempSlider->getValue();
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
	OceanFunction->SetValue(def_transition_func);
	orb->setOceanFunction(def_transition_func);
	orb->invalidate();
	TheScene->rebuild();
}

//-------------------------------------------------------------
// VtxWaterTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxWaterTabs::setObjAttributes(){
	update_needed=true;
	TNwater *tnode=water();

	Planetoid *orb=getOrbital();

	char *s=object_name->GetValue().ToAscii();
	if(s)
		strncpy(orb->ocean_name,s,64);

	int state=State->GetSelection();
    orb->ocean_state=state;

	orb->ocean_liquid_temp=LiquidTempSlider->getValue();
	orb->ocean_solid_temp=SolidTempSlider->getValue();

	orb->water_color1=LiquidReflectSlider->getColor();
	orb->water_color1.set_alpha(LiquidReflectSlider->getValue());
	orb->water_color2=LiquidTransmitSlider->getColor();
	orb->water_clarity=LiquidTransmitSlider->getValue()*FEET;
	orb->water_shine=LiquidShineSlider->getValue();
	orb->water_specular=LiquidAlbedoSlider->getValue();
	orb->ocean_level=LevelSlider->getValue()*FEET;

	orb->ice_color1=SolidReflectSlider->getColor();
	orb->ice_color1.set_alpha(SolidReflectSlider->getValue());
	orb->ice_color2=SolidTransmitSlider->getColor();
	orb->ice_clarity=SolidTransmitSlider->getValue()*FEET;
	orb->ice_shine=SolidShineSlider->getValue();
	orb->ice_specular=SolidAlbedoSlider->getValue();

	orb->setOceanFunction((char*)OceanFunction->GetValue().ToAscii());

	wxString str="ocean(";
	str+=LiquidFunction->GetValue();
	str+=",";
	str+=SolidFunction->GetValue();
	str+=")";
	str+="\n";

	char p[256];
	strcpy(p,str.ToAscii());
	tnode->setExpr(p);
	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
	}
	invalidateRender();
}
//-------------------------------------------------------------
// VtxWaterTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxWaterTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNwater *tnode=water();

	TNarg *arg=(TNarg*)tnode->left;

	LiquidFunction->SetValue(def_water_func);
	SolidFunction->SetValue(def_ice_func);

	if(arg){
		LiquidFunction->SetValue(arg);
		arg=arg->next();
		if(arg)
			SolidFunction->SetValue(arg);
	}
	Planetoid *orb=getOrbital();

	char buff[256];
	orb->getOceanFunction(buff);

	OceanFunction->SetValue(buff);

	State->SetSelection(orb->ocean_state);

	object_name->SetValue(orb->ocean_name);
	LiquidTempSlider->setValue(orb->ocean_liquid_temp);
	SolidTempSlider->setValue(orb->ocean_solid_temp);

	LevelSlider->setValue(orb->ocean_level/FEET);

	LiquidTransmitSlider->setColor(orb->water_color2);
	LiquidTransmitSlider->setValue(orb->water_clarity/FEET);
	LiquidReflectSlider->setColor(orb->water_color1);
	LiquidReflectSlider->setValue(orb->water_color1.alpha());

	LiquidShineSlider->setValue(orb->water_shine);
	LiquidAlbedoSlider->setValue(orb->water_specular);

	SolidTransmitSlider->setColor(orb->ice_color2);
	SolidTransmitSlider->setValue(orb->ice_clarity/FEET);
	SolidReflectSlider->setColor(orb->ice_color1);
	SolidReflectSlider->setValue(orb->ice_color1.alpha());

	SolidShineSlider->setValue(orb->ice_shine);
	SolidAlbedoSlider->setValue(orb->ice_specular);

	auto_state->SetValue(orb->ocean_auto);

	update_needed=false;

}
