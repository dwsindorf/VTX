
#include "VtxWaterTabs.h"
#include "VtxSceneDialog.h"
#include "RenderOptions.h"

#include <wx/filefn.h>
#include <wx/dir.h>

//########################### VtxWaterTabs Class ########################
enum{
	ID_SHOW,
    ID_LEVEL_SLDR,
    ID_LEVEL_TEXT,
    ID_WAVES_TEXT,
    ID_TRANSMIT_SLDR,
    ID_TRANSMIT_TEXT,
    ID_TRANSMIT_COLOR,
    ID_REFLECT_SLDR,
    ID_REFLECT_TEXT,
    ID_REFLECT_COLOR,
    ID_SHINE_SLDR,
    ID_SHINE_TEXT,
    ID_ALBEDO_SLDR,
    ID_ALBEDO_TEXT,
};

#define EXPR_WIDTH  140

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

//########################### VtxWaterTabs Class ########################

IMPLEMENT_CLASS(VtxWaterTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxWaterTabs, VtxTabsMgr)

SET_SLIDER_EVENTS(LEVEL,VtxWaterTabs,Level)

SET_COLOR_EVENTS(TRANSMIT,VtxWaterTabs,Transmit)
SET_COLOR_EVENTS(REFLECT,VtxWaterTabs,Reflect)
SET_SLIDER_EVENTS(SHINE,VtxWaterTabs,Shine)
SET_SLIDER_EVENTS(ALBEDO,VtxWaterTabs,Albedo)

EVT_TEXT_ENTER(ID_WAVES_TEXT,VtxWaterTabs::OnWavesEnter)

EVT_MENU(ID_SHOW,VtxWaterTabs::OnEnable)
EVT_UPDATE_UI(ID_SHOW, VtxWaterTabs::OnUpdateEnable)

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
 	AddWaterTab(page);
    AddPage(page,wxT("Properties"),true);

    return true;
}

void VtxWaterTabs::AddWaterTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* surface = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Surface"));

	LevelSlider=new SliderCtrl(panel,ID_LEVEL_SLDR,"level",LABEL2,VALUE,SLIDER2);
	LevelSlider->setRange(-10000,10000);
	surface->Add(LevelSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
	m_waves=new ExprTextCtrl(panel,ID_WAVES_TEXT,"waves",LABEL2S,EXPR_WIDTH);
	surface->Add(m_waves->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
	surface->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(surface, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* transmission = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Transparency"));

    TransmitSlider = new ColorSlider(panel,ID_TRANSMIT_SLDR,"feet",LABEL,VALUE,CSLIDER,CBOX1);
    TransmitSlider->setRange(1,1000);
    transmission->Add(TransmitSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    transmission->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(transmission, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* reflection = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Reflection"));

    ReflectSlider = new ColorSlider(panel,ID_REFLECT_SLDR,"%",LABEL,VALUE,CSLIDER,CBOX1);
    ReflectSlider->setRange(0,1);
    reflection->Add(ReflectSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    reflection->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(reflection, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* specular = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Specular"));

    ShineSlider = new SliderCtrl(panel,ID_SHINE_SLDR,"shine",LABEL2,VALUE,SLIDER2);
    ShineSlider->setRange(0,100.0);
    specular->Add(ShineSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);


    AlbedoSlider = new SliderCtrl(panel,ID_ALBEDO_SLDR,"albedo",LABEL2S,VALUE,SLIDER2);
    AlbedoSlider->setRange(0,1.0);
    specular->Add(AlbedoSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

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


void VtxWaterTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

void VtxWaterTabs::OnEnable(wxCommandEvent& event){
	setEnabled(!isEnabled());
	Render.set_water_show(!Render.show_water());
	TheScene->set_changed_render();
	menu_action=TABS_ENABLE;
	sceneDialog->updateObjectTree();
}
void VtxWaterTabs::OnUpdateEnable(wxUpdateUIEvent& event) {
 	event.Check(Render.show_water());
}

//-------------------------------------------------------------
// VtxWaterTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxWaterTabs::setObjAttributes(){
	update_needed=true;
	TNwater *tnode=water();

	Planetoid *orb=getOrbital();

	orb->water_color1=ReflectSlider->getColor();
	orb->water_color1.set_alpha(ReflectSlider->getValue());
	orb->water_color2=TransmitSlider->getColor();
	orb->water_clarity=TransmitSlider->getValue()*FEET;
	orb->water_shine=ShineSlider->getValue();
	orb->water_specular=AlbedoSlider->getValue();
	orb->water_level=LevelSlider->getValue()*FEET;

	wxString str="water(";
	str+=m_waves->GetValue();
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
	//if(TheScene->inside_sky())
		invalidateRender();
	//else
	//	invalidateObject();

}
//-------------------------------------------------------------
// VtxWaterTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxWaterTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNwater *tnode=water();

	TNarg *arg=(TNarg*)tnode->left;

	if(arg)
		m_waves->SetValue(arg);
	else
		m_waves->SetValue("");
	Planetoid *orb=getOrbital();
	LevelSlider->setValue(orb->water_level/FEET);
	TransmitSlider->setColor(orb->water_color2);
	TransmitSlider->setValue(orb->water_clarity/FEET);
	ReflectSlider->setColor(orb->water_color1);
	ReflectSlider->setValue(orb->water_color1.alpha());

	ShineSlider->setValue(orb->water_shine);
	AlbedoSlider->setValue(orb->water_specular);

	update_needed=false;

}
