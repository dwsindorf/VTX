
#include "VtxCoronaTabs.h"
#include "UniverseModel.h"

enum {
	ID_ENABLE,
	ID_DELETE,
    ID_SIZE_SLDR,
    ID_SIZE_TEXT,
    ID_GRADIENT_TEXT,
    ID_GRADIENT_SLDR,
    ID_OUTER_SLDR,
    ID_OUTER_TEXT,
    ID_OUTER_COLOR,
    ID_INNER_SLDR,
    ID_INNER_TEXT,
    ID_INNER_COLOR,
};

IMPLEMENT_CLASS(VtxCoronaTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxCoronaTabs, wxNotebook)
EVT_MENU(ID_DELETE,VtxCoronaTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxCoronaTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxCoronaTabs::OnUpdateEnable)

EVT_COMMAND_SCROLL(ID_SIZE_SLDR,VtxCoronaTabs::OnSizeSlider)
EVT_TEXT_ENTER(ID_SIZE_TEXT,VtxCoronaTabs::OnSizeText)

EVT_COMMAND_SCROLL(ID_GRADIENT_SLDR,VtxCoronaTabs::OnGradientSlider)
EVT_TEXT_ENTER(ID_GRADIENT_TEXT,VtxCoronaTabs::OnGradientText)

EVT_COMMAND_SCROLL(ID_OUTER_SLDR,VtxCoronaTabs::OnOuterSlider)
EVT_TEXT_ENTER(ID_OUTER_TEXT,VtxCoronaTabs::OnOuterText)
EVT_COLOURPICKER_CHANGED(ID_OUTER_COLOR,VtxCoronaTabs::OnOuterColor)

EVT_COMMAND_SCROLL(ID_INNER_SLDR,VtxCoronaTabs::OnInnerSlider)
EVT_TEXT_ENTER(ID_INNER_TEXT,VtxCoronaTabs::OnInnerText)
EVT_COLOURPICKER_CHANGED(ID_INNER_COLOR,VtxCoronaTabs::OnInnerColor)


END_EVENT_TABLE()

VtxCoronaTabs::VtxCoronaTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

int VtxCoronaTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Show"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));
	PopupMenu(&menu);
	return menu_action;
}

bool VtxCoronaTabs::Create(wxWindow* parent,
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
    AddPage(page,wxT("Properties"),true);
    return true;
}

void VtxCoronaTabs::AddObjectTab(wxWindow *panel){
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topsizer);

    wxBoxSizer* boxsizer = new wxBoxSizer(wxVERTICAL);
    topsizer->Add(boxsizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* object_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Geometry"));
	SizeSlider=new SliderCtrl(panel,ID_SIZE_SLDR,"Radius",LABEL, VALUE,SLIDER);
	SizeSlider->setRange(1,10);
	SizeSlider->setValue(2);
	object_cntrls->Add(SizeSlider->getSizer(),wxALIGN_LEFT|wxALL);

	boxsizer->Add(object_cntrls,0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* color_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Color"));

	InnerSlider=new ColorSlider(panel,ID_INNER_SLDR,"Inner",LABEL,VALUE,CSLIDER,CBOX1);
	color_cntrls->Add(InnerSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
	OuterSlider=new ColorSlider(panel,ID_OUTER_SLDR,"Outer",LABEL,VALUE,CSLIDER,CBOX1);
	color_cntrls->Add(OuterSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

	wxBoxSizer* grad_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Gradient"));
	GradientSlider=new SliderCtrl(panel,ID_GRADIENT_SLDR,"Alpha",LABEL, VALUE,SLIDER);
	GradientSlider->setRange(0.0,1);
	GradientSlider->setValue(1.0);

	grad_cntrls->Add(GradientSlider->getSizer(),wxALIGN_LEFT|wxALL);

	boxsizer->Add(color_cntrls, 0, wxALIGN_LEFT|wxALL,0);
	boxsizer->Add(grad_cntrls, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxCoronaTabs::updateControls(){
	Corona *obj=object();
	updateSlider(SizeSlider,obj->size/parentSize());
	updateSlider(GradientSlider,obj->gradient);
	updateColor(InnerSlider,obj->color1);
	updateColor(OuterSlider,obj->color2);
}
