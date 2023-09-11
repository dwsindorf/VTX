
#include "VtxErodeTabs.h"
#include "VtxFunctDialog.h"
#include "Erode.h"

//########################### VtxErodeTabs Class ########################

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_WIDTH BOX_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

#define VALUE1 60
#define LABEL1 40

// define all resource ids here
enum {
	ID_DELETE,
	ID_ENABLE,

	ID_SQR,
	ID_SS,

    ID_START_SLDR,
    ID_START_TEXT,
    ID_FILL_AMPL_SLDR,
    ID_FILL_AMPL_TEXT,
    ID_FILL_TRANS_SLDR,
    ID_FILL_TRANS_TEXT,
    ID_FILL_LEVEL_SLDR,
    ID_FILL_LEVEL_TEXT,
    ID_FILL_MARGIN_SLDR,
    ID_FILL_MARGIN_TEXT,
    ID_SLOPE_AMPL_SLDR,
    ID_SLOPE_AMPL_TEXT,
    ID_SLOPE_MAX_SLDR,
    ID_SLOPE_MAX_TEXT,
    ID_SLOPE_MIN_SLDR,
    ID_SLOPE_MIN_TEXT,
    ID_SLOPE_DROP_SLDR,
    ID_SLOPE_DROP_TEXT,
};

IMPLEMENT_CLASS(VtxErodeTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxErodeTabs, wxNotebook)

EVT_CHECKBOX(ID_SQR,VtxErodeTabs::OnChangeEvent)
EVT_CHECKBOX(ID_SS,VtxErodeTabs::OnChangeEvent)

//SET_SLIDER_EVENTS(START,VtxErodeTabs,Start)
SET_SLIDER_EVENTS(FILL_AMPL,VtxErodeTabs,FillAmpl)
SET_SLIDER_EVENTS(FILL_TRANS,VtxErodeTabs,FillTransport)

SET_SLIDER_EVENTS(FILL_LEVEL,VtxErodeTabs,FillLevel)
SET_SLIDER_EVENTS(FILL_MARGIN,VtxErodeTabs,FillMargin)
SET_SLIDER_EVENTS(SLOPE_AMPL,VtxErodeTabs,SlopeAmpl)
SET_SLIDER_EVENTS(SLOPE_MAX,VtxErodeTabs,SlopeMax)
SET_SLIDER_EVENTS(SLOPE_MIN,VtxErodeTabs,SlopeMin)

SET_SLIDER_EVENTS(SLOPE_DROP,VtxErodeTabs,SlopeDrop)

EVT_MENU(ID_DELETE,VtxErodeTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxErodeTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxErodeTabs::OnUpdateEnable)

END_EVENT_TABLE()

VtxErodeTabs::VtxErodeTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxErodeTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);

	AddControlsTab(page);
    AddPage(page,wxT("Erode"),true);
    Show(false);

    return true;
}

int VtxErodeTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));

	PopupMenu(&menu);
	return menu_action;
}

void VtxErodeTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

void VtxErodeTabs::AddControlsTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

//    wxBoxSizer* orders_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Orders"));

//	StartSlider=new SliderCtrl(panel,ID_START_SLDR,"Start",LABEL2, VALUE1,SLIDER2);
//	StartSlider->setRange(0,25);

//	orders_cntrls->Add(StartSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
//
//	OrdersSlider=new SliderCtrl(panel,ID_ORDERS_SLDR,"Levels",LABEL1, VALUE1,SLIDER2);
//	OrdersSlider->setRange(1,25);
//
//	orders_cntrls->Add(OrdersSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
//
//	orders_cntrls->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
//	boxSizer->Add(orders_cntrls,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer* deposition = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Deposition"));

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	FillAmplSlider=new ExprSliderCtrl(panel,ID_FILL_AMPL_SLDR,"Ampl",LABEL2,VALUE1,SLIDER2);
	FillAmplSlider->setRange(0.0,1.0);
	FillAmplSlider->setValue(0.5);
	hline->Add(FillAmplSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);


    FillTransportSlider=new ExprSliderCtrl(panel,ID_FILL_TRANS_SLDR,"Trans",LABEL2,VALUE1,SLIDER2);
    FillTransportSlider->setRange(0.0,1.0);
    FillTransportSlider->setValue(0.0);
    hline->Add(FillTransportSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    deposition->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    hline = new wxBoxSizer(wxHORIZONTAL);

    FillLevelSlider=new ExprSliderCtrl(panel,ID_FILL_LEVEL_SLDR,"Level",LABEL2,VALUE1,SLIDER2);
    FillLevelSlider->setRange(0.0,3.0);
    FillLevelSlider->setValue(0.0);

    hline->Add(FillLevelSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    FillMarginSlider=new ExprSliderCtrl(panel,ID_FILL_MARGIN_SLDR,"Width",LABEL2,VALUE1,SLIDER2);
    FillMarginSlider->setRange(0.0,3.0);
    FillMarginSlider->setValue(0.0);
    hline->Add(FillMarginSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	deposition->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    boxSizer->Add(deposition,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer *slope = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Slope"));

    hline = new wxBoxSizer(wxHORIZONTAL);

    SlopeAmplSlider=new ExprSliderCtrl(panel,ID_SLOPE_AMPL_SLDR,"Ampl",LABEL2, VALUE1,SLIDER2);
    SlopeAmplSlider->setRange(0,1);
    SlopeAmplSlider->setValue(0.0);

    hline->Add(SlopeAmplSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    SlopeDropSlider=new ExprSliderCtrl(panel,ID_SLOPE_DROP_SLDR,"Drop",LABEL2,VALUE1,SLIDER2);
    SlopeDropSlider->setRange(0.0,1.0);
    SlopeDropSlider->setValue(0.0);

    hline->Add(SlopeDropSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    slope->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    hline = new wxBoxSizer(wxHORIZONTAL);

    SlopeMinSlider=new ExprSliderCtrl(panel,ID_SLOPE_MIN_SLDR,"Min",LABEL2,VALUE1,SLIDER2);
    SlopeMinSlider->setRange(0.0,1.0);
    SlopeMinSlider->setValue(0.25);

    hline->Add(SlopeMinSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    SlopeMaxSlider=new ExprSliderCtrl(panel,ID_SLOPE_MAX_SLDR,"Max",LABEL2,VALUE1,SLIDER2);
    SlopeMaxSlider->setRange(0.0,5.0);
    SlopeMaxSlider->setValue(2.0);

    hline->Add(SlopeMaxSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    slope->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(slope,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Options"));

	m_sqr=new wxCheckBox(panel, ID_SQR, "Square");
	options->Add(m_sqr, 0, wxALIGN_LEFT|wxALL,2);

	m_ss=new wxCheckBox(panel, ID_SS, "SmoothStep");
	options->Add(m_ss, 0, wxALIGN_LEFT|wxALL,2);

    options->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));

	boxSizer->Add(options,0,wxALIGN_LEFT|wxALL,0);
}

void VtxErodeTabs::getObjAttributes(){
	if(!update_needed)
		return;
	
	TNerode *tc=object();

	int type=tc->options;

	m_sqr->SetValue((type & SQR)?true:false);
	m_ss->SetValue((type & SS)?true:false);

	TNarg &args=*((TNarg *)tc->left);

	//StartSlider->setValue(args[0]);
	//OrdersSlider->setValue(args[1]);

	int idx=0;
	TNode *a=args[idx++];

	if(a)
		FillAmplSlider->setValue(a);
	else
		FillAmplSlider->setValue(0.0);
    a=args[idx++];
    if(a)
        FillTransportSlider->setValue(a);
    else
        FillTransportSlider->setValue(0.0);
    a=args[idx++];
    if(a)
        FillLevelSlider->setValue(a);
    else
        FillLevelSlider->setValue(0.0);
    a=args[idx++];
    if(a)
        FillMarginSlider->setValue(a);
    else
        FillMarginSlider->setValue(0.2);

	a=args[idx++];
	if(a)
		SlopeAmplSlider->setValue(a);
	else
	    SlopeAmplSlider->setValue(0.0);
    a=args[idx++];
    if(a)
        SlopeDropSlider->setValue(a);
    else
        SlopeDropSlider->setValue(0.0);

    a=args[idx++];
    if(a)
        SlopeMinSlider->setValue(a);
    else
        SlopeMinSlider->setValue(0.0);

	a=args[idx++];
	if(a)
	    SlopeMaxSlider->setValue(a);
	else
	    SlopeMaxSlider->setValue(3.0);

	update_needed=false;
}

void VtxErodeTabs::setObjAttributes(){

	TNerode *tnode=object();

	wxString s="erode(";
	wxString n="";

	if(m_sqr->GetValue()){
		if(n.IsEmpty())
			n+="SQR";
		else
			n+="|SQR";
	}
	if(m_ss->GetValue()){
		if(n.IsEmpty())
			n+="SS";
		else
			n+="|SS";
	}
	if(!n.IsEmpty()){
		s+=n+",";
	}
	//s+=StartSlider->getText()+",";
	s+=FillAmplSlider->getText()+",";
    s+=FillTransportSlider->getText()+",";
    s+=FillLevelSlider->getText()+",";
    s+=FillMarginSlider->getText()+",";

	s+=SlopeAmplSlider->getText()+",";
    s+=SlopeDropSlider->getText()+",";
    s+=SlopeMinSlider->getText()+",";
	s+=SlopeMaxSlider->getText();
	s+=")";

	tnode->setExpr((char*)s.ToAscii());
	update_needed=false;
	tnode->applyExpr();
	TheView->set_changed_detail();
	TheScene->rebuild_all();
}
