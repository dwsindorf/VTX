
#include "VtxNoiseFunct.h"
#include "VtxFunctDialog.h"
#include "TerrainClass.h"

//########################### VtxNoiseFunct Class ########################
#define BOX_WIDTH TABS_WIDTH
#define LINE_WIDTH BOX_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30
#undef SLIDER2
#undef LABEL2

#define SLIDER2  100
#define LABEL2   60


enum {
	ID_START_SLDR,
	ID_START_TEXT,
	ID_ORDERS_SLDR,
	ID_ORDERS_TEXT,
	ID_H_SLDR,
	ID_H_TEXT,
	ID_L_SLDR,
	ID_L_TEXT,
	ID_HOMOG_SLDR,
	ID_HOMOG_TEXT,
	ID_AMPL_SLDR,
	ID_AMPL_TEXT,
	ID_CLAMP_SLDR,
	ID_CLAMP_TEXT,
	ID_OFFSET_SLDR,
	ID_OFFSET_TEXT,
	ID_ROUND_SLDR,
	ID_ROUND_TEXT,
	ID_DOMAIN,
	ID_MAPPING,
	ID_MODE,
	ID_SQR,
	ID_UNS,
	ID_NEG,
	ID_ABS,
	ID_NORM,
	ID_SCALE,
	ID_LOD,
	ID_TYPE
};

IMPLEMENT_CLASS(VtxNoiseFunct, wxNotebook )

BEGIN_EVENT_TABLE(VtxNoiseFunct, wxNotebook)

EVT_CHECKBOX(ID_SQR,VtxNoiseFunct::OnChangeEvent)
EVT_CHECKBOX(ID_UNS,VtxNoiseFunct::OnChangeEvent)
EVT_CHECKBOX(ID_NEG,VtxNoiseFunct::OnChangeEvent)
EVT_CHECKBOX(ID_NORM,VtxNoiseFunct::OnChangeEvent)
EVT_CHECKBOX(ID_SCALE,VtxNoiseFunct::OnChangeEvent)
EVT_CHECKBOX(ID_LOD,VtxNoiseFunct::OnChangeEvent)
EVT_CHECKBOX(ID_ABS,VtxNoiseFunct::OnChangeEvent)
EVT_CHOICE(ID_DOMAIN, VtxNoiseFunct::OnChangeEvent)
EVT_CHOICE(ID_MAPPING, VtxNoiseFunct::OnChangeEvent)
EVT_CHOICE(ID_MODE, VtxNoiseFunct::OnChangeEvent)
EVT_RADIOBOX(ID_TYPE, VtxNoiseFunct::OnChangeEvent)

SET_SLIDER_EVENTS(ROUND,VtxNoiseFunct,Round)
SET_SLIDER_EVENTS(OFFSET,VtxNoiseFunct,Offset)
SET_SLIDER_EVENTS(CLAMP,VtxNoiseFunct,Clamp)
SET_SLIDER_EVENTS(AMPL,VtxNoiseFunct,Ampl)
SET_SLIDER_EVENTS(START,VtxNoiseFunct,Start)
SET_SLIDER_EVENTS(ORDERS,VtxNoiseFunct,Orders)
SET_SLIDER_EVENTS(H,VtxNoiseFunct,H)
SET_SLIDER_EVENTS(L,VtxNoiseFunct,L)
SET_SLIDER_EVENTS(HOMOG,VtxNoiseFunct,Homog)

END_EVENT_TABLE()

VtxNoiseFunct::VtxNoiseFunct(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxFunctMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxNoiseFunct::Create(wxWindow* parent,
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
    AddPage(page,wxT("Multi-Noise"),true);
    page=new wxPanel(this,wxID_ANY);
    AddTypeTab(page);
    AddPage(page,wxT("Advanced"),false);


    Show(false);

    return true;
}

void VtxNoiseFunct::AddControlsTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* orders_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Orders"));
    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

    // Orders: line 1

	StartSlider=new SliderCtrl(panel,ID_START_SLDR,"Start",LABEL2, VALUE2,SLIDER2);
	StartSlider->setRange(0,25);

	hline->Add(StartSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OrdersSlider=new SliderCtrl(panel,ID_ORDERS_SLDR,"Levels",LABEL2, VALUE2,SLIDER2);
	OrdersSlider->setRange(1,25);

	hline->Add(OrdersSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	orders_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    // Orders: line 2

	hline = new wxBoxSizer(wxHORIZONTAL);

	HSlider=new SliderCtrl(panel,ID_H_SLDR,"Atten",LABEL2, VALUE2,SLIDER2);
	HSlider->setRange(0,1);

	hline->Add(HSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	LSlider=new SliderCtrl(panel,ID_L_SLDR,"DelF",LABEL2, VALUE2,SLIDER2);
	LSlider->setRange(1,4);

	hline->Add(LSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	orders_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    // Orders: line 3

	hline = new wxBoxSizer(wxHORIZONTAL);

	HomogSlider=new SliderCtrl(panel,ID_HOMOG_SLDR,"Homog",LABEL2, VALUE2,SLIDER2);
	HomogSlider->setRange(-1,1);
	hline->Add(HomogSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);


	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	orders_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(orders_cntrls, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* shape_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Shape"));

	// Shape: line 1

	m_sqr=new wxCheckBox(panel, ID_SQR, "Sqr");
	shape_cntrls->Add(m_sqr, 0, wxALIGN_LEFT|wxALL,2);

	m_neg=new wxCheckBox(panel, ID_NEG, "Invt");
	shape_cntrls->Add(m_neg, 0, wxALIGN_LEFT|wxALL,2);

	m_uns=new wxCheckBox(panel, ID_UNS, "Uns");
	shape_cntrls->Add(m_uns, 0, wxALIGN_LEFT|wxALL,2);

	m_abs=new wxCheckBox(panel, ID_ABS, "Abs");
	shape_cntrls->Add(m_abs, 0, wxALIGN_LEFT|wxALL,2);

	m_norm=new wxCheckBox(panel, ID_NORM, "Norm");
	shape_cntrls->Add(m_norm, 0, wxALIGN_LEFT|wxALL,2);

	m_scale=new wxCheckBox(panel, ID_SCALE, "Scale");
	shape_cntrls->Add(m_scale, 0, wxALIGN_LEFT|wxALL,2);

	m_lod=new wxCheckBox(panel, ID_LOD, "LOD");
	shape_cntrls->Add(m_lod, 0, wxALIGN_LEFT|wxALL,2);


	shape_cntrls->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
	boxSizer->Add(shape_cntrls, 0, wxALIGN_LEFT|wxALL,0);

	// Amplitude

	wxBoxSizer* ampl_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Amplitude"));

	AmplSlider=new SliderCtrl(panel,ID_AMPL_SLDR,"Gain",LABEL2, VALUE2,SLIDER2);
	AmplSlider->setRange(0,1);

	ampl_cntrls->Add(AmplSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OffsetSlider=new SliderCtrl(panel,ID_OFFSET_SLDR,"Offset",LABEL2, VALUE2,SLIDER2);
	OffsetSlider->setRange(-2.0,2.0);

	ampl_cntrls->Add(OffsetSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);


	ampl_cntrls->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
	boxSizer->Add(ampl_cntrls, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* mod_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Modulation"));

	RoundSlider=new SliderCtrl(panel,ID_ROUND_SLDR,"Smooth",LABEL2, VALUE2,SLIDER2);
	RoundSlider->setRange(0,1);

	mod_cntrls->Add(RoundSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	ClampSlider=new SliderCtrl(panel,ID_CLAMP_SLDR,"Clamp",LABEL2, VALUE2,SLIDER2);
	ClampSlider->setRange(-1,1);

	mod_cntrls->Add(ClampSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);


	mod_cntrls->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));

	boxSizer->Add(mod_cntrls, 0, wxALIGN_LEFT|wxALL,0);

    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

}

void VtxNoiseFunct::AddTypeTab(wxWindow *panel){

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(boxSizer);

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

    wxString lmodes[]={"Perlin","Voronoi"};
    m_noisetype=new wxRadioBox(panel,ID_TYPE,wxT("Type"),wxPoint(-1,-1),wxSize(-1, 50),2,
   		lmodes,2,wxRA_SPECIFY_COLS);
    m_noisetype->SetSelection(0);

    hline->Add(m_noisetype,0,wxALIGN_LEFT|wxALL,5);

	wxBoxSizer* noise_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Noise"));

	wxStaticText *lbl=new wxStaticText(panel,-1,"RSeed",wxDefaultPosition,wxSize(-1,-1));
	noise_cntrls->Add(lbl, 0, wxALIGN_LEFT|wxTop|wxLEFT,5);

	wxString offsets[]={"0","1","2","3","4","5"};

	m_domain=new wxChoice(panel, ID_DOMAIN, wxDefaultPosition,wxSize(45,-1),6, offsets);
	m_domain->SetSelection(0);
	noise_cntrls->Add(m_domain,0,wxALIGN_LEFT|wxLEFT,5);

	lbl=new wxStaticText(panel,-1,"RMode",wxDefaultPosition,wxSize(-1,-1));
	noise_cntrls->Add(lbl, 0, wxALIGN_LEFT|wxTop|wxLEFT,5);

	wxString mode[]={"Vertex","Pixel"};
	m_mode=new wxChoice(panel, ID_MODE, wxDefaultPosition,wxSize(90,-1),2, mode);
	m_mode->SetSelection(0);
	noise_cntrls->Add(m_mode,0,wxALIGN_LEFT|wxLEFT,5);
	hline->Add(noise_cntrls,0,wxALIGN_LEFT|wxALL,5);

    boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
}
//-------------------------------------------------------------
// VtxNoiseFunct::setFunction() set controls from string
//-------------------------------------------------------------
void VtxNoiseFunct::setFunction(wxString f){
	if(f.IsEmpty())
		return;

	TNnoise *tn=(TNnoise*)TheScene->parse_node((char*)f.ToAscii());
	if(!tn)
		return;

	TNarg &args=*((TNarg *)tn->right);

	StartSlider->setValue(args[0]);
	OrdersSlider->setValue(args[1]);

	int type=tn->type;

	TNode *a=args[2];

	if(a)
		HomogSlider->setValue(a);
	else
		HomogSlider->setValue(1.0);
	a=args[3];
	if(a)
		HSlider->setValue(a);
	else
		HSlider->setValue(TheNoise.dflt_H);
	a=args[4];
	if(a)
		LSlider->setValue(a);
	else
		LSlider->setValue(TheNoise.dflt_L);

	a=args[5];
	if(a)
		AmplSlider->setValue(a);
	else
		AmplSlider->setValue(1.0);

	a=args[6];
	if(a)
		ClampSlider->setValue(a);
	else
		ClampSlider->setValue(1.0);

	a=args[7];
	if(a)
		RoundSlider->setValue(a);
	else
		RoundSlider->setValue(0.0);

	a=args[8];
	if(a)
		OffsetSlider->setValue(a);
	else
		OffsetSlider->setValue(0.0);

	setTypeControls(type);

	delete tn;
}

//-------------------------------------------------------------
// VtxNoiseFunct::setTypeControls() set controls from type
//-------------------------------------------------------------
void VtxNoiseFunct::setTypeControls(int type)
{
	m_neg->SetValue((type & NEG)?true:false);
	m_uns->SetValue((type & UNS)?true:false);
	m_sqr->SetValue((type & SQR)?true:false);
	m_abs->SetValue((type & NABS)?true:false);
	m_scale->SetValue((type & SCALE)?true:false);
	m_lod->SetValue((type & NLOD)?false:true);
	m_norm->SetValue((type & NNORM)?false:true);
	m_domain->SetSelection(type & ROFF);

	if(type & FS){
//		if(type & BP)
//			m_mode->SetSelection(2);
//		else
			m_mode->SetSelection(1);
	}
	else
		m_mode->SetSelection(0);
	int ntype=type & NTYPES;
	switch(ntype){
	case VORONOI:
		m_noisetype->SetSelection(1);
		break;
	default:
		m_noisetype->SetSelection(0);
	}
}

//-------------------------------------------------------------
// VtxNoiseFunct::getTypeStr() return type string from controls
//-------------------------------------------------------------
wxString VtxNoiseFunct::getTypeStr()
{
	wxString n="";
	int type=m_noisetype->GetSelection();
	switch(type){
	default:
		n="GRADIENT";
		break;
	case 1:
		n="VORONOI";
		break;
	}
	if(m_uns->GetValue())
		n+="|UNS";
	if(m_sqr->GetValue())
		n+="|SQR";
	if(m_scale->GetValue())
		n+="|SCALE";
	if(m_neg->GetValue())
		n+="|NEG";
	if(!m_lod->GetValue())
		n+="|NLOD";
	if(!m_norm->GetValue())
		n+="|NNORM";
	if(m_abs->GetValue())
		n+="|NABS";

    int roff = m_domain->GetSelection();
	switch(roff){
	case 1: n+="|RO1"; break;
	case 2: n+="|RO2"; break;
	case 3: n+="|RO3"; break;
	case 4: n+="|RO4"; break;
	case 5: n+="|RO5"; break;
	}

	int mode=m_mode->GetSelection();
	switch(mode){
	case 1: n+="|FS"; break;
	}
	return n;
}

//-------------------------------------------------------------
// VtxNoiseFunct::getFunction() set return string from controls
//-------------------------------------------------------------
void VtxNoiseFunct::getFunction(){
	wxString s="noise(";
	s+=getTypeStr()+",";
	s+=StartSlider->getText()+",";
	s+=OrdersSlider->getText()+",";
	s+=HomogSlider->getText()+",";
	s+=HSlider->getText()+",";
	s+=LSlider->getText()+",";
	s+=AmplSlider->getText()+",";
	s+=ClampSlider->getText()+",";
	s+=RoundSlider->getText()+",";
	s+=OffsetSlider->getText();
	s+=")";

	TNnoise *tn=(TNnoise*)TheScene->parse_node((char*)s.ToAscii());
	if(!tn){
		cout<<"parser error:"<<s.ToAscii()<<endl;
		return;
	}
	char buff[256];
	buff[0]=0;
	tn->valueString(buff);
	s = wxString(buff);

	noiseDialog->getFunct(s);
}
