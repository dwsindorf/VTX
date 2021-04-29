
#include "VtxFogTabs.h"
#include "VtxSceneDialog.h"
#include "RenderOptions.h"

#include <wx/filefn.h>
#include <wx/dir.h>

//########################### VtxFogTabs Class ########################
enum{
	ID_ENABLE,
    ID_DENSITY_EXPR,
    ID_HMIN_SLDR,
    ID_HMIN_TEXT,
    ID_HMAX_SLDR,
    ID_HMAX_TEXT,
    ID_VMIN_SLDR,
    ID_VMIN_TEXT,
    ID_VMAX_SLDR,
    ID_VMAX_TEXT,
    ID_FOG_COLOR_SLDR,
    ID_FOG_COLOR_TEXT,
    ID_FOG_COLOR_COLOR,
    ID_FOG_GLOW_SLDR,
    ID_FOG_GLOW_TEXT,
};

#define SLDR_WIDTH  160

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30
#define EXPR_WIDTH  LINE_WIDTH-LABEL2


//########################### VtxFogTabs Class ########################

IMPLEMENT_CLASS(VtxFogTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxFogTabs, VtxTabsMgr)


SET_SLIDER_EVENTS(FOG_GLOW,VtxFogTabs,FogGlow)
SET_SLIDER_EVENTS(HMIN,VtxFogTabs,HMin)
SET_SLIDER_EVENTS(HMAX,VtxFogTabs,HMax)
SET_SLIDER_EVENTS(VMIN,VtxFogTabs,VMin)
SET_SLIDER_EVENTS(VMAX,VtxFogTabs,VMax)


SET_COLOR_EVENTS(FOG_COLOR,VtxFogTabs,FogColor)

EVT_TEXT_ENTER(ID_DENSITY_EXPR,VtxFogTabs::OnChangedExpr)

EVT_MENU(ID_ENABLE,VtxFogTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxFogTabs::OnUpdateEnable)

EVT_MENU(TABS_DELETE,VtxFogTabs::OnDelete)
SET_FILE_EVENTS(VtxFogTabs)

END_EVENT_TABLE()

VtxFogTabs::VtxFogTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxFogTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
 	AddFogTab(page);
    AddPage(page,wxT("Properties"),true);
    return true;
}

void VtxFogTabs::AddFogTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* horizontal = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Horizontal Range (feet)"));
    HMinSlider=new SliderCtrl(panel,ID_HMIN_SLDR,"Min",LABEL2,VALUE2,SLIDER2);
    HMinSlider->setRange(0,1000);
    horizontal->Add(HMinSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    HMaxSlider=new SliderCtrl(panel,ID_HMAX_SLDR,"Depth",LABEL2,VALUE2,SLIDER2);
    HMaxSlider->setRange(0,10000);
    horizontal->Add(HMaxSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    horizontal->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(horizontal, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* vertical = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Vertical Range (feet)"));
    VMinSlider=new SliderCtrl(panel,ID_VMIN_SLDR,"Min",LABEL2,VALUE2,SLIDER2);
    VMinSlider->setRange(-5000,5000);
    vertical->Add(VMinSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    VMaxSlider=new SliderCtrl(panel,ID_VMAX_SLDR,"Depth",LABEL2,VALUE2,SLIDER2);
    VMaxSlider->setRange(0,10000);
    vertical->Add(VMaxSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    vertical->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(vertical, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* properties = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Properties"));

	DensityExpr=new ExprTextCtrl(panel,ID_DENSITY_EXPR,"Density",LABEL2,EXPR_WIDTH);
	properties->Add(DensityExpr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

    FogColorSlider = new ColorSlider(panel,ID_FOG_COLOR_SLDR,"Color",LABEL2,VALUE2,90,CBOX3);
    hline->Add(FogColorSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,3);

    FogGlowSlider=new SliderCtrl(panel,ID_FOG_GLOW_SLDR,"Glow",50,VALUE2,SLIDER2);
    FogGlowSlider->setRange(0,1);

    hline->Add(FogGlowSlider->getSizer(), 5, wxALIGN_LEFT|wxALL,0);

    properties->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

    properties->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(properties, 0, wxALIGN_LEFT|wxALL,0);
}

int VtxFogTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.Append(TABS_DELETE,wxT("Delete"));
	sceneDialog->AddFileMenu(menu,object_node->node);
	PopupMenu(&menu);
	return menu_action;
}


void VtxFogTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}
void VtxFogTabs::OnEnable(wxCommandEvent& event){
	setEnabled(!isEnabled());
	Render.set_fog(!Render.fog());
	TheScene->set_changed_render();
	menu_action=TABS_ENABLE;
	sceneDialog->updateObjectTree();
}
void VtxFogTabs::OnUpdateEnable(wxUpdateUIEvent& event) {
 	event.Check(Render.fog());
}

//-------------------------------------------------------------
// VtxFogTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxFogTabs::setObjAttributes(){
	update_needed=true;
	TNfog *tnode=fog();

	Planetoid *orb=getOrbital();
	orb->fog_vmin=VMinSlider->getValue()*FEET;
	orb->fog_vmax=(VMaxSlider->getValue()+VMinSlider->getValue())*FEET;
	orb->fog_min=HMinSlider->getValue()*FEET;
	orb->fog_max=(HMaxSlider->getValue()+HMinSlider->getValue())*FEET;

	Color color=FogColorSlider->getColor();
	double fog_value=FogColorSlider->getValue();
	color.set_alpha(fog_value);

	orb->fog_color=color;
	orb->fog_value=fog_value;
	orb->fog_glow=FogGlowSlider->getValue();

	wxString str="fog(";

	wxString expr=DensityExpr->getText();
	if(expr.length()==0)
		expr="1";
	str+=expr;

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
}
//-------------------------------------------------------------
// VtxFogTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxFogTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNfog *tnode=fog();

	TNarg *arg=(TNarg*)tnode->left;

	//m_density_expr->setValue("1.0");

	if(arg){	// height expr
		DensityExpr->setValue(arg);
	}
	Planetoid *orb=getOrbital();
	HMinSlider->setValue(orb->fog_min/FEET);
	HMaxSlider->setValue((orb->fog_max-orb->fog_min)/FEET);
	VMinSlider->setValue(orb->fog_vmin/FEET);
	VMaxSlider->setValue((orb->fog_vmax-orb->fog_vmin)/FEET);
	FogGlowSlider->setValue(orb->fog_glow);

	Color color=orb->fog_color;
	color.set_alpha(orb->fog_value);

	FogColorSlider->setColor(color);
//	m_fog_color.SetColor(orb->fog_color);
//	m_fog_value=(int)(orb->fog_value*100);

	update_needed=false;

}
