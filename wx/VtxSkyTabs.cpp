
//#include "UniverseModel.h"
#include "VtxSkyTabs.h"
#include "VtxSceneDialog.h"

#define LABEL_WIDTH 40
#define VALUE_WIDTH 40
#define SLIDER_WIDTH 100
//########################### ObjectAddMenu Class ########################
enum{
	OBJ_NONE,
	OBJ_SHOW,
	OBJ_DELETE,
	//OBJ_SAVE,

	ID_NAME_TEXT,
    ID_CELLSIZE_SLDR,
    ID_CELLSIZE_TEXT,

    ID_HEIGHT_SLDR,
    ID_HEIGHT_TEXT,
    ID_DENSITY_SLDR,
    ID_DENSITY_TEXT,
    ID_HAZE_GRAD_SLDR,
    ID_HAZE_GRAD_TEXT,
    ID_HAZE_SLIDER,
    ID_HAZE_TEXT,
    ID_HAZE_COLOR,
    ID_SKY_SLDR,
    ID_SKY_TEXT,
    ID_SKY_COLOR,
    ID_SKY_GRAD_SLDR,
    ID_SKY_GRAD_TEXT,
    ID_TWILITE_SLDR,
    ID_TWILITE_TEXT,
    ID_TWILITE_COLOR,
    ID_TWILITE_GRAD_SLDR,
    ID_TWILITE_GRAD_TEXT,
    ID_NIGHT_SLDR,
    ID_NIGHT_TEXT,
    ID_NIGHT_COLOR,
    ID_NIGHT_GRAD_SLDR,
    ID_NIGHT_GRAD_TEXT,
    ID_HALO_SLDR,
    ID_HALO_TEXT,
    ID_HALO_COLOR,
    ID_HALO_GRAD_SLDR,
    ID_HALO_GRAD_TEXT,
 };

//########################### VtxSkyTabs Class ########################

IMPLEMENT_CLASS(VtxSkyTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxSkyTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxSkyTabs::OnNameText)

SET_SLIDER_EVENTS(CELLSIZE,VtxSkyTabs,CellSize)

EVT_MENU(OBJ_SHOW,VtxSkyTabs::OnEnable)
EVT_UPDATE_UI(OBJ_SHOW,VtxSkyTabs::OnUpdateEnable)

EVT_COMMAND_SCROLL(ID_HEIGHT_SLDR,VtxSkyTabs::OnHeightSlider)
EVT_TEXT_ENTER(ID_HEIGHT_TEXT,VtxSkyTabs::OnHeightText)

EVT_COMMAND_SCROLL(ID_DENSITY_SLDR,VtxSkyTabs::OnDensitySlider)
EVT_TEXT_ENTER(ID_DENSITY_TEXT,VtxSkyTabs::OnDensityText)

EVT_COMMAND_SCROLL(ID_HAZE_SLIDER,VtxSkyTabs::OnHazeSlider)
EVT_TEXT_ENTER(ID_HAZE_TEXT,VtxSkyTabs::OnHazeText)
EVT_COLOURPICKER_CHANGED(ID_HAZE_COLOR,VtxSkyTabs::OnHazeColor)

EVT_COMMAND_SCROLL(ID_TWILITE_SLDR,VtxSkyTabs::OnTwiliteSlider)
EVT_TEXT_ENTER(ID_TWILITE_TEXT,VtxSkyTabs::OnTwiliteText)
EVT_COLOURPICKER_CHANGED(ID_TWILITE_COLOR,VtxSkyTabs::OnTwiliteColor)

EVT_COMMAND_SCROLL(ID_SKY_SLDR,VtxSkyTabs::OnSkySlider)
EVT_TEXT_ENTER(ID_SKY_TEXT,VtxSkyTabs::OnSkyText)
EVT_COLOURPICKER_CHANGED(ID_SKY_COLOR,VtxSkyTabs::OnSkyColor)
EVT_COMMAND_SCROLL(ID_SKY_GRAD_SLDR,VtxSkyTabs::OnSkyGradSlider)
EVT_TEXT_ENTER(ID_SKY_GRAD_TEXT,VtxSkyTabs::OnSkyGradText)

EVT_COMMAND_SCROLL(ID_HALO_SLDR,VtxSkyTabs::OnHaloSlider)
EVT_TEXT_ENTER(ID_HALO_TEXT,VtxSkyTabs::OnHaloText)
EVT_COLOURPICKER_CHANGED(ID_HALO_COLOR,VtxSkyTabs::OnHaloColor)
EVT_COMMAND_SCROLL(ID_HALO_GRAD_SLDR,VtxSkyTabs::OnHaloGradSlider)
EVT_TEXT_ENTER(ID_HALO_GRAD_TEXT,VtxSkyTabs::OnHaloGradText)

EVT_COMMAND_SCROLL(ID_NIGHT_SLDR,VtxSkyTabs::OnNightSlider)
EVT_TEXT_ENTER(ID_NIGHT_TEXT,VtxSkyTabs::OnNightText)
EVT_COLOURPICKER_CHANGED(ID_NIGHT_COLOR,VtxSkyTabs::OnNightColor)
EVT_COMMAND_SCROLL(ID_NIGHT_GRAD_SLDR,VtxSkyTabs::OnNightGradSlider)
EVT_TEXT_ENTER(ID_NIGHT_GRAD_TEXT,VtxSkyTabs::OnNightGradText)

EVT_COMMAND_SCROLL(ID_HAZE_GRAD_SLDR,VtxSkyTabs::OnHazeGradSlider)
EVT_TEXT_ENTER(ID_HAZE_GRAD_TEXT,VtxSkyTabs::OnHazeGradText)

EVT_COMMAND_SCROLL(ID_TWILITE_GRAD_SLDR,VtxSkyTabs::OnTwiliteGradSlider)
EVT_TEXT_ENTER(ID_TWILITE_GRAD_TEXT,VtxSkyTabs::OnTwiliteGradText)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxSkyTabs::OnAddItem)

EVT_MENU(OBJ_DELETE,VtxSkyTabs::OnDelete)

SET_FILE_EVENTS(VtxSkyTabs)

//EVT_MENU(OBJ_SAVE,VtxSkyTabs::OnSave)

END_EVENT_TABLE()

VtxSkyTabs::VtxSkyTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

int VtxSkyTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;

	wxMenu menu;
	menu.AppendCheckItem(OBJ_SHOW,wxT("Show"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));
	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}

	sceneDialog->AddFileMenu(menu,object_node->node);

	PopupMenu(&menu);

	return menu_action;
}

bool VtxSkyTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
    AddObjectTab(page);
    AddPage(page,wxT("Object"),true);

    page=new wxPanel(this,wxID_ANY);
    AddLightingTab(page);
    AddPage(page,wxT("Lighting"),false);

    return true;
}

void VtxSkyTabs::AddObjectTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* object_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Object"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2,VALUE2+SLIDER2+5);
	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	//hline->AddSpacer(10);

	CellSizeSlider=new SliderCtrl(panel,ID_CELLSIZE_SLDR,"Grid",LABEL2, VALUE2,SLIDER2);
	CellSizeSlider->setRange(1,4);
	CellSizeSlider->setValue(1);
	hline->Add(CellSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
    object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    hline = new wxBoxSizer(wxHORIZONTAL);

	HeightSlider=new SliderCtrl(panel,ID_HEIGHT_SLDR,"Height",LABEL2, VALUE2,SLIDER2);
	HeightSlider->setRange(10,200);
	HeightSlider->setValue(20);

	hline->Add(HeightSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	DensitySlider=new SliderCtrl(panel,ID_DENSITY_SLDR,"Density",LABEL2, VALUE2,SLIDER2);
	DensitySlider->setRange(0,1);

	hline->Add(DensitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->SetMinSize(wxSize(TABS_WIDTH-2*TABS_BORDER,-1));
	object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	object_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(object_cntrls, 0, wxALIGN_LEFT|wxALL,0);

}


void VtxSkyTabs::AddLightingTab(wxWindow *panel){
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topsizer);

    wxBoxSizer* boxsizer = new wxBoxSizer(wxVERTICAL);
    topsizer->Add(boxsizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxBoxSizer* color_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Color"));
	wxBoxSizer* grad_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Sharpen"));
	wxBoxSizer* cg_cntrls = new wxBoxSizer(wxHORIZONTAL);

	HaloColor=new ColorSlider(panel,ID_HALO_SLDR,"Halo",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH,CBOX3);
	color_cntrls->Add(HaloColor->getSizer(),wxALIGN_LEFT|wxALL);

	HaloGrad=new SliderCtrl(panel,ID_HALO_GRAD_SLDR,"",0, VALUE_WIDTH,SLIDER_WIDTH);
	grad_cntrls->Add(HaloGrad->getSizer(),wxALIGN_LEFT|wxALL,0);
	HaloGrad->setRange(0.1,5);

	SkyColor=new ColorSlider(panel,ID_SKY_SLDR,"Sky",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH,CBOX3);
	color_cntrls->Add(SkyColor->getSizer(),wxALIGN_LEFT|wxALL);
	SkyGrad=new SliderCtrl(panel,ID_SKY_GRAD_SLDR,"",0, VALUE_WIDTH,SLIDER_WIDTH);
	SkyGrad->setRange(0.01,1);
	grad_cntrls->Add(SkyGrad->getSizer(),wxALIGN_LEFT|wxALL,0);

	TwiliteColor=new ColorSlider(panel,ID_TWILITE_SLDR,"Twilite",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH,CBOX3);
	color_cntrls->Add(TwiliteColor->getSizer(),wxALIGN_LEFT|wxALL);
	TwiliteGrad=new SliderCtrl(panel,ID_TWILITE_GRAD_SLDR,"",0, VALUE_WIDTH,SLIDER_WIDTH);
	grad_cntrls->Add(TwiliteGrad->getSizer(),wxALIGN_LEFT|wxALL,0);
	TwiliteGrad->setRange(0.01,1);

	NightColor=new ColorSlider(panel,ID_NIGHT_SLDR,"Night",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH,CBOX3);
	color_cntrls->Add(NightColor->getSizer(),wxALIGN_LEFT|wxALL);
	NightGrad=new SliderCtrl(panel,ID_NIGHT_GRAD_SLDR,"",0, VALUE_WIDTH,SLIDER_WIDTH);
	grad_cntrls->Add(NightGrad->getSizer(),wxALIGN_LEFT|wxALL,0);
	NightGrad->setRange(-0.01,-1,0.01,1.0);

	wxSize size=color_cntrls->GetMinSize();

	grad_cntrls->SetMinSize(wxSize(TABS_WIDTH-size.GetWidth()-TABS_BORDER,size.GetHeight()));
	cg_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,size.GetHeight()));

	cg_cntrls->Add(color_cntrls,0,wxALIGN_LEFT|wxALL,0);
	cg_cntrls->Add(grad_cntrls,0,wxALIGN_LEFT|wxALL,0);
	boxsizer->Add(cg_cntrls, 0, wxALIGN_LEFT|wxALL,0);

	// Haze controls

	wxBoxSizer* haze_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Haze"));

	HazeColor=new ColorSlider(panel,ID_HAZE_SLIDER,"Color",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH,CBOX3);
	haze_cntrls->Add(HazeColor->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	haze_cntrls->AddSpacer(10);
	HazeGrad=new SliderCtrl(panel,ID_HAZE_GRAD_SLDR,"",0, VALUE_WIDTH,SLIDER_WIDTH);
	HazeGrad->setRange(0,1,0.0,1);

	haze_cntrls->Add(HazeGrad->getSizer(),wxALIGN_LEFT|wxALL);
	haze_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxsizer->Add(haze_cntrls, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxSkyTabs::updateControls(){
	Sky *sky=(Sky*)object();
	double val=(sky->size-parentSize())/MILES;
	updateSlider(HeightSlider,val);

	Color c=sky->color();
	updateColor(SkyColor,c);
	updateColor(TwiliteColor,sky->twilite_color);
	updateColor(HazeColor, sky->haze_color);
	updateColor(HaloColor,sky->halo_color);
	updateSlider(HaloGrad,sky->shine);
	updateColor(NightColor,sky->night_color);
	updateSlider(NightGrad,sky->twilite_min);
	updateSlider(HazeGrad,sky->haze_grad);
	updateSlider(DensitySlider,sky->density);
	updateSlider(TwiliteGrad,sky->twilite_max);
	updateSlider(SkyGrad,sky->sky_grad);
	updateSlider(CellSizeSlider,sky->detail);
	object_name->SetValue(object_node->node->nodeName());
}

void VtxSkyTabs::OnHazeGradSlider(wxScrollEvent& event){
    OnSliderValue(HazeGrad, object()->haze_grad);
    if(!Render.draw_shaded()){
    	if(object()->haze_grad>0.6)
			Render.set_fogmode(POW);
    	else if(object()->haze_grad>0.4)
			Render.set_fogmode(EXP2);
		else if (object()->haze_grad>0.2)
			Render.set_fogmode(EXP);
		else
			Render.set_fogmode(LINEAR);
    }
 }
void VtxSkyTabs::OnHazeGradText(wxCommandEvent& event){
	OnSliderText(HazeGrad, object()->haze_grad);
	if(!Render.draw_shaded()){
    	if(object()->haze_grad>0.6)
			Render.set_fogmode(POW);
    	else if(object()->haze_grad>0.4)
			Render.set_fogmode(EXP2);
		else if (object()->haze_grad>0.2)
			Render.set_fogmode(EXP);
		else
			Render.set_fogmode(LINEAR);
	}
}
