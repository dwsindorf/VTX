
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
    ID_HMIN_EXPR,
    ID_HMAX_SLDR,
    ID_HMAX_EXPR,
    ID_VMIN_SLDR,
    ID_VMIN_EXPR,
    ID_VMAX_SLDR,
    ID_VMAX_EXPR,
    ID_FOG_SLDR,
    ID_FOG_EXPR,
    ID_FOG_COLOR,
};

#define SLDR_WIDTH  160
#define EXPR_WIDTH  100

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

//########################### VtxFogTabs Class ########################

IMPLEMENT_CLASS(VtxFogTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxFogTabs, VtxTabsMgr)

EVT_TEXT_ENTER(ID_DENSITY_EXPR,VtxFogTabs::OnChangedExpr)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_HMIN_SLDR,VtxFogTabs::OnEndHminSlider)
EVT_COMMAND_SCROLL(ID_HMIN_SLDR,VtxFogTabs::OnHminSlider)
EVT_TEXT_ENTER(ID_HMIN_EXPR,VtxFogTabs::OnHminEnter)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_HMAX_SLDR,VtxFogTabs::OnEndHmaxSlider)
EVT_COMMAND_SCROLL(ID_HMAX_SLDR,VtxFogTabs::OnHmaxSlider)
EVT_TEXT_ENTER(ID_HMAX_EXPR,VtxFogTabs::OnHmaxEnter)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_VMIN_SLDR,VtxFogTabs::OnEndVminSlider)
EVT_COMMAND_SCROLL(ID_VMIN_SLDR,VtxFogTabs::OnVminSlider)
EVT_TEXT_ENTER(ID_VMIN_EXPR,VtxFogTabs::OnVminEnter)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_VMAX_SLDR,VtxFogTabs::OnEndVmaxSlider)
EVT_COMMAND_SCROLL(ID_VMAX_SLDR,VtxFogTabs::OnVmaxSlider)
EVT_TEXT_ENTER(ID_VMAX_EXPR,VtxFogTabs::OnVmaxEnter)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_FOG_SLDR,VtxFogTabs::OnEndFogSlider)
EVT_COMMAND_SCROLL(ID_FOG_SLDR,VtxFogTabs::OnFogSlider)
EVT_TEXT_ENTER(ID_FOG_EXPR,VtxFogTabs::OnFogEnter)

EVT_COLOURPICKER_CHANGED(ID_FOG_COLOR,VtxFogTabs::OnFogColor)

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
    m_hmin_sldr=new SliderCtrl(panel,ID_HMIN_SLDR,"Min",LABEL2,VALUE2,SLIDER2);
    m_hmin_sldr->setRange(0,1000);
    horizontal->Add(m_hmin_sldr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    m_hmax_sldr=new SliderCtrl(panel,ID_HMAX_SLDR,"Depth",LABEL2,VALUE2,SLIDER2);
    m_hmax_sldr->setRange(0,10000);
    horizontal->Add(m_hmax_sldr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    horizontal->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(horizontal, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* vertical = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Vertical Range (feet)"));
    m_vmin_sldr=new SliderCtrl(panel,ID_VMIN_SLDR,"Min",LABEL2,VALUE2,SLIDER2);
    m_vmin_sldr->setRange(-5000,5000);
    vertical->Add(m_vmin_sldr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    m_vmax_sldr=new SliderCtrl(panel,ID_VMAX_SLDR,"Depth",LABEL2,VALUE2,SLIDER2);
    m_vmax_sldr->setRange(0,10000);
    vertical->Add(m_vmax_sldr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    vertical->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(vertical, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* density = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Density"));

	m_density_expr=new ExprTextCtrl(panel,ID_DENSITY_EXPR,"Expr",LABEL2,EXPR_WIDTH);
	density->Add(m_density_expr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

    m_fog_color = new ColorSlider(panel,ID_FOG_SLDR,"Color",LABEL2,VALUE2,CSLIDER2,CBOX1);
    density->Add(m_fog_color->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
     density->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(density, 0, wxALIGN_LEFT|wxALL,0);
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
	orb->fog_vmin=m_vmin_sldr->getValue()*FEET;
	orb->fog_vmax=(m_vmax_sldr->getValue()+m_vmin_sldr->getValue())*FEET;
	orb->fog_min=m_hmin_sldr->getValue()*FEET;
	orb->fog_max=(m_hmax_sldr->getValue()+m_hmin_sldr->getValue())*FEET;

	Color color=m_fog_color->getColor();
	double fog_value=m_fog_color->getValue();
	color.set_alpha(fog_value);

	orb->fog_color=color;
	orb->fog_value=fog_value;

	wxString str="fog(";

	wxString expr=m_density_expr->getText();
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
		m_density_expr->setValue(arg);
	}
	Planetoid *orb=getOrbital();
	m_hmin_sldr->setValue(orb->fog_min/FEET);
	m_hmax_sldr->setValue((orb->fog_max-orb->fog_min)/FEET);
	m_vmin_sldr->setValue(orb->fog_vmin/FEET);
	m_vmax_sldr->setValue((orb->fog_vmax-orb->fog_vmin)/FEET);

	Color color=orb->fog_color;
	color.set_alpha(orb->fog_value);

	m_fog_color->setColor(color);
//	m_fog_color.SetColor(orb->fog_color);
//	m_fog_value=(int)(orb->fog_value*100);

	update_needed=false;

}
