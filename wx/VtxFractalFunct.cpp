
#include "VtxFractalFunct.h"
#include "VtxFunctDialog.h"
#include "Fractal.h"

//########################### VtxFractalFunct Class ########################

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_WIDTH BOX_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

#define VALUE1 60
#define LABEL1 40

// define all resource ids here
enum {
	OBJ_DELETE,
	OBJ_SAVE,
	ID_SQR,
	ID_SS,

    ID_START_SLDR,
    ID_START_TEXT,
    ID_ORDERS_SLDR,
    ID_ORDERS_TEXT,
    ID_BUILD_SLDR,
    ID_BUILD_TEXT,
    ID_ERODE_SLDR,
    ID_ERODE_TEXT,
    ID_SLOPE_SLDR,
    ID_SLOPE_TEXT,
    ID_CLIP_SLDR,
    ID_CLIP_TEXT,
    ID_LIMIT_SLDR,
    ID_LIMIT_TEXT,
    ID_BIAS_SLDR,
    ID_BIAS_TEXT,
    ID_HMAX_SLDR,
    ID_HMAX_TEXT,
    ID_HVAL_SLDR,
    ID_HVAL_TEXT,
};

IMPLEMENT_CLASS(VtxFractalFunct, wxNotebook )

BEGIN_EVENT_TABLE(VtxFractalFunct, wxNotebook)

EVT_CHECKBOX(ID_SQR,VtxFractalFunct::OnChangeEvent)
EVT_CHECKBOX(ID_SS,VtxFractalFunct::OnChangeEvent)

SET_SLIDER_EVENTS(ORDERS,VtxFractalFunct,Orders)
SET_SLIDER_EVENTS(START,VtxFractalFunct,Start)
SET_SLIDER_EVENTS(BUILD,VtxFractalFunct,Build)
SET_SLIDER_EVENTS(ERODE,VtxFractalFunct,Erode)
SET_SLIDER_EVENTS(SLOPE,VtxFractalFunct,Slope)
SET_SLIDER_EVENTS(CLIP,VtxFractalFunct,Clip)
SET_SLIDER_EVENTS(LIMIT,VtxFractalFunct,Limit)
SET_SLIDER_EVENTS(BIAS,VtxFractalFunct,Bias)
SET_SLIDER_EVENTS(HMAX,VtxFractalFunct,HtMax)
SET_SLIDER_EVENTS(HVAL,VtxFractalFunct,HtVal)

END_EVENT_TABLE()

VtxFractalFunct::VtxFractalFunct(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxFunctMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxFractalFunct::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxFunctMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);

	AddControlsTab(page);
    AddPage(page,wxT("Fractal"),true);
    Show(false);

    return true;
}

void VtxFractalFunct::AddControlsTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* orders_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Orders"));

	StartSlider=new ExprSliderCtrl(panel,ID_START_SLDR,"Start",LABEL2, VALUE1,SLIDER2);
	StartSlider->setRange(0,25);

	orders_cntrls->Add(StartSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OrdersSlider=new ExprSliderCtrl(panel,ID_ORDERS_SLDR,"Levels",LABEL1, VALUE1,SLIDER2);
	OrdersSlider->setRange(1,25);

	orders_cntrls->Add(OrdersSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	orders_cntrls->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
	boxSizer->Add(orders_cntrls,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer* deposition = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Deposition"));

	BuildSlider=new ExprSliderCtrl(panel,ID_BUILD_SLDR,"Build",LABEL2,VALUE1,SLIDER2);
	BuildSlider->setRange(0.0,3.0);
	BuildSlider->setValue(0.5);
	deposition->Add(BuildSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	ErodeSlider=new ExprSliderCtrl(panel,ID_ERODE_SLDR,"Erode",LABEL1,VALUE1,SLIDER2);
	ErodeSlider->setRange(0.0,3.0);
	ErodeSlider->setValue(0.0);
	deposition->Add(ErodeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	deposition->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));

	boxSizer->Add(deposition,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer *slope = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Slope Bias"));

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	SlopeSlider=new ExprSliderCtrl(panel,ID_SLOPE_SLDR,"Magn",LABEL2, VALUE1,SLIDER2);
	SlopeSlider->setRange(0,3);
	SlopeSlider->setValue(0.0);

	hline->Add(SlopeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	ClipSlider=new ExprSliderCtrl(panel,ID_CLIP_SLDR,"Max",LABEL1, VALUE1,SLIDER2);
	ClipSlider->setRange(0.0,3.0);
	hline->Add(ClipSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT+TABS_BORDER));

	slope->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	LimitSlider=new ExprSliderCtrl(panel,ID_LIMIT_SLDR,"Thresh",LABEL2, VALUE1,SLIDER2);
	LimitSlider->setRange(0,1.0);
	LimitSlider->setValue(0.0);

	hline->Add(LimitSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	BiasSlider=new ExprSliderCtrl(panel,ID_BIAS_SLDR,"Mult",LABEL1, VALUE1,SLIDER2);
	BiasSlider->setRange(0.0,1.0);
	hline->Add(BiasSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT+TABS_BORDER));

	slope->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(slope,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer *htctrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Elevation Bias"));
    hline = new wxBoxSizer(wxHORIZONTAL);
    HtMaxSlider=new ExprSliderCtrl(panel,ID_HMAX_SLDR,"Max",LABEL2, VALUE1,SLIDER2);
    HtMaxSlider->setRange(0,1.0);
    HtMaxSlider->setValue(1.0);
    hline->Add(HtMaxSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    HtValSlider=new ExprSliderCtrl(panel,ID_HVAL_SLDR,"Val",LABEL1, VALUE1,SLIDER2);
    HtValSlider->setRange(0,1.0);
    HtValSlider->setValue(1.0);
    hline->Add(HtValSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    htctrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    boxSizer->Add(htctrls,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Options"));

	m_sqr=new wxCheckBox(panel, ID_SQR, "Square");
	options->Add(m_sqr, 0, wxALIGN_LEFT|wxALL,2);

	m_ss=new wxCheckBox(panel, ID_SS, "SmoothStep");
	options->Add(m_ss, 0, wxALIGN_LEFT|wxALL,2);

    options->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));

	boxSizer->Add(options,0,wxALIGN_LEFT|wxALL,0);

}

void VtxFractalFunct::setFunction(wxString f){
	TNfractal *tc=(TNfractal*)TheScene->parse_node((char*)f.ToAscii());
	if(!tc)
		return;

	int type=tc->options;

	m_sqr->SetValue((type & SQR)?true:false);
	m_ss->SetValue((type & SS)?true:false);
	nargs=numargs((TNarg *)tc->left);
	TNarg &args=*((TNarg *)tc->left);

	StartSlider->setValue(args[0]);
	OrdersSlider->setValue(args[1]);

	TNode *a=args[2];

	if(a)
		BuildSlider->setValue(a); // ampl
	else
		BuildSlider->setValue(0.1);
	a=args[3];
	if(a)
		SlopeSlider->setValue(a); // sbias
	else
		SlopeSlider->setValue(0.2);
	a=args[4];
	if(a)
		ClipSlider->setValue(a);  // smax
	else
		ClipSlider->setValue(2.0);
	a=args[5];
	if(a)
		ErodeSlider->setValue(a); // sdrop
	else
		ErodeSlider->setValue(0.1);

	a=args[6];
	if(a)
		LimitSlider->setValue(a); // thresh
	else
		LimitSlider->setValue(0.0);

	a=args[7];
	if(a)
		BiasSlider->setValue(a);  // thresh bias
	else
		BiasSlider->setValue(0.0);
	a=args[8];
	if(a)
		HtMaxSlider->setValue(a);  // htmax
	else
		HtMaxSlider->setValue(1.0);
	a=args[9];
	if(a)
		HtValSlider->setValue(a);  // htbias
	else
		HtValSlider->setValue(0.0);

	delete tc;

}

void VtxFractalFunct::getFunction(){
	char buff[256];

	wxString s="fractal(";
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
	s+=StartSlider->getText()+",";
	s+=OrdersSlider->getText()+",";
	s+=BuildSlider->getText()+",";
	s+=SlopeSlider->getText()+",";
	s+=ClipSlider->getText()+",";
	s+=ErodeSlider->getText()+",";
	s+=LimitSlider->getText()+",";
	s+=BiasSlider->getText()+",";;
	s+=HtMaxSlider->getText()+",";
	s+=HtValSlider->getText();
	s+=")";

	TNfractal *tn=(TNfractal*)TheScene->parse_node(s.ToAscii());
	if(!tn){
		return;
	}
	buff[0]=0;
	tn->valueString(buff);
	//cout << buff << endl;
	s = wxString(buff);

	functDialog->getFunct(s);
}
