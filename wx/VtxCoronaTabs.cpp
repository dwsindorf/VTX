
#include "VtxCoronaTabs.h"
#include "UniverseModel.h"

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#undef CSLIDER
#define CSLIDER LINE_WIDTH-CBOX1-LABEL-VALUE
enum {
	ID_ENABLE,
	ID_DELETE,
	ID_NAME_TEXT,
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
	ID_NOISE_EXPR,
	ID_RATE_SCALE,
	ID_RATE_SLDR,
	ID_RATE_TEXT,
	ID_ANIMATE
};

IMPLEMENT_CLASS(VtxCoronaTabs, wxNotebook )


BEGIN_EVENT_TABLE(VtxCoronaTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxCoronaTabs::OnNameText)

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

EVT_TEXT_ENTER(ID_NOISE_EXPR,VtxCoronaTabs::OnChangedNoiseExpr)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_RATE_SLDR,VtxCoronaTabs::OnEndRateSlider)
EVT_COMMAND_SCROLL(ID_RATE_SLDR,VtxCoronaTabs::OnRateSlider)
EVT_TEXT_ENTER(ID_RATE_TEXT,VtxCoronaTabs::OnRateText)

EVT_CHOICE(ID_RATE_SCALE, VtxCoronaTabs::OnRateScale)

EVT_CHECKBOX(ID_ANIMATE, VtxCoronaTabs::OnAnimate)


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
    object_cntrls->SetMinSize(wxSize(LINE_WIDTH,-1));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2S,130);
	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->AddSpacer(10);

	SizeSlider=new SliderCtrl(panel,ID_SIZE_SLDR,"Radius",LABEL2S, VALUE2,110);
	SizeSlider->setRange(1,10);
	SizeSlider->setValue(2);

	hline->Add(SizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	boxsizer->Add(object_cntrls,0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* color_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Color"));
	color_cntrls->SetMinSize(wxSize(LINE_WIDTH,-1));

	InnerSlider=new ColorSlider(panel,ID_INNER_SLDR,"Inner",LABEL,VALUE,CSLIDER,CBOX1);
	color_cntrls->Add(InnerSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
	OuterSlider=new ColorSlider(panel,ID_OUTER_SLDR,"Outer",LABEL,VALUE,CSLIDER,CBOX1);
	color_cntrls->Add(OuterSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

	wxBoxSizer* grad_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Gradient"));
	grad_cntrls->SetMinSize(wxSize(LINE_WIDTH,-1));

	GradientSlider=new SliderCtrl(panel,ID_GRADIENT_SLDR,"Alpha",LABEL, VALUE,LINE_WIDTH-VALUE-LABEL);
	GradientSlider->setRange(0.0,1);
	GradientSlider->setValue(1.0);

	grad_cntrls->Add(GradientSlider->getSizer(),wxALIGN_LEFT|wxALL);

	wxBoxSizer* density_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Density"));
	NoiseExpr=new ExprTextCtrl(panel,ID_NOISE_EXPR,"",0,LINE_WIDTH);
	density_cntrls->Add(NoiseExpr->getSizer(), 0, wxALIGN_LEFT|wxALL,3);
	
	wxBoxSizer* time_ctrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Animation"));
	hline = new wxBoxSizer(wxHORIZONTAL);

	wxString exps[]={"1","0.1","0.01","0.001","1e-4","1e-5","1e-6","1e-7","1e-8"};
	
	animate=new wxCheckBox(panel, ID_ANIMATE, " On ");
	
	rate_scale=new wxChoice(panel, ID_RATE_SCALE, wxDefaultPosition,wxSize(80,-1),9, exps);

	rate_scale->SetSelection(4);
	RateSlider=new SliderCtrl(panel,ID_RATE_SLDR,"Rate",LABEL, VALUE,180);
	RateSlider->setRange(1,10);

	hline->Add(RateSlider->getSizer());
	hline->Add(rate_scale);
	hline->Add(animate);

    time_ctrls->Add(hline);

	boxsizer->Add(color_cntrls, 0, wxALIGN_LEFT|wxALL,0);
	boxsizer->Add(grad_cntrls, 0, wxALIGN_LEFT|wxALL,0);
	boxsizer->Add(time_ctrls, 0, wxALIGN_LEFT|wxALL,0);
	boxsizer->Add(density_cntrls, 0, wxALIGN_LEFT|wxALL,0);

}

void VtxCoronaTabs::updateControls(){
	Corona *obj=object();
	updateSlider(SizeSlider,obj->size/parentSize());

	updateSlider(GradientSlider,obj->gradient);
	updateColor(InnerSlider,obj->color1);
	updateColor(OuterSlider,obj->color2);
	object_name->SetValue(object_node->node->nodeName());
	getRate();
	animate->SetValue(obj->animation);

	char buff[256]={0};
	if(obj->getNoiseFunction(buff))
		NoiseExpr->SetValue(buff);
	else
		NoiseExpr->SetValue("");
}

void VtxCoronaTabs::OnChangedNoiseExpr(wxCommandEvent& event){
	Corona *obj=object();
	obj->setNoiseFunction((char*)NoiseExpr->GetValue().ToAscii());
	obj->applyNoiseFunction();
	obj->invalidate();
	TheScene->set_changed_detail();
	TheScene->rebuild();
}

void VtxCoronaTabs::getRate(){
	double rate=object()->rate;
	double exp=-floor(log10(rate));
	double rem=rate/pow(10,-exp);
	rate_scale->SetSelection((int)exp);
	RateSlider->setValue(rem);	
}
void VtxCoronaTabs::setRate(){
	double rem=RateSlider->getValue();
	double exp=rate_scale->GetSelection();
	double rate=rem*pow(10,-exp);
	object()->rate=rate;
	TheScene->set_changed_render();
}
