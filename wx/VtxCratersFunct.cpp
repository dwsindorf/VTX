
#include "VtxCratersFunct.h"
#include "VtxFunctDialog.h"
#include "Craters.h"

//########################### VtxCratersFunct Class ########################
//#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
//#define LINE_HEIGHT 30

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_WIDTH BOX_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

#define VALUE1 50
#define LABEL1 50
#undef LABEL2
#define LABEL2 55

// define all resource ids here
enum {
	OBJ_DELETE,
	OBJ_SAVE,

    ID_SCALE_SLDR,
    ID_SCALE_TEXT,
    ID_SCALE_MULT,
    ID_ORDERS,
    ID_DELTA_SIZE_SLDR,
    ID_DELTA_SIZE_TEXT,
    ID_DENSITY_SLDR,
    ID_DENSITY_TEXT,
    ID_SEED,
    ID_IMPACT_SLDR,
    ID_IMPACT_TEXT,
    ID_AMPL_SLDR,
    ID_AMPL_TEXT,
    ID_RISE_SLDR,
    ID_RISE_TEXT,
    ID_DROP_SLDR,
    ID_DROP_TEXT,
    ID_RIM_SLDR,
    ID_RIM_TEXT,
    ID_FLOOR_SLDR,
    ID_FLOOR_TEXT,
    ID_CNTR_SLDR,
    ID_CNTR_TEXT,
    ID_BIAS_SLDR,
    ID_BIAS_TEXT,
    ID_VNOISE_SLDR,
    ID_VNOISE_TEXT,
    ID_RNOISE_SLDR,
    ID_RNOISE_TEXT,
};

IMPLEMENT_CLASS(VtxCratersFunct, wxNotebook )

BEGIN_EVENT_TABLE(VtxCratersFunct, wxNotebook)

SET_SLIDER_EVENTS(SCALE,VtxCratersFunct,Scale)
SET_SLIDER_EVENTS(DELTA_SIZE,VtxCratersFunct,DeltaSize)
SET_SLIDER_EVENTS(DENSITY,VtxCratersFunct,Density)
SET_SLIDER_EVENTS(IMPACT,VtxCratersFunct,Impact)
SET_SLIDER_EVENTS(AMPL,VtxCratersFunct,Ampl)
SET_SLIDER_EVENTS(RISE,VtxCratersFunct,Rise)
SET_SLIDER_EVENTS(DROP,VtxCratersFunct,Drop)
SET_SLIDER_EVENTS(RIM,VtxCratersFunct,Rim)
SET_SLIDER_EVENTS(FLOOR,VtxCratersFunct,Floor)
SET_SLIDER_EVENTS(CNTR,VtxCratersFunct,Center)
SET_SLIDER_EVENTS(RNOISE,VtxCratersFunct,RNoise)
SET_SLIDER_EVENTS(VNOISE,VtxCratersFunct,VNoise)
SET_SLIDER_EVENTS(BIAS,VtxCratersFunct,Offset)

EVT_CHOICE(ID_SCALE_MULT, VtxCratersFunct::OnChangeEvent)
EVT_CHOICE(ID_SEED, VtxCratersFunct::OnChangeEvent)
EVT_CHOICE(ID_ORDERS, VtxCratersFunct::OnChangeEvent)

END_EVENT_TABLE()

VtxCratersFunct::VtxCratersFunct(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxFunctMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxCratersFunct::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxFunctMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);

	AddCratersTab(page);
    AddPage(page,wxT("Craters"),true);
    page=new wxPanel(this,wxID_ANY);
    AddShapeTab(page);
    AddPage(page,wxT("Shape"),false);
    Show(false);

    return true;
}

void VtxCratersFunct::AddCratersTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* distro = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Distribution"));

	// levels

    distro->Add(new wxStaticText(panel,-1,"Levels",wxDefaultPosition,wxSize(LABEL2,-1)), 0, wxALIGN_LEFT|wxALL, 4);

	wxString orders[]={"1","2","3","4","5","6","7","8","9","10"};
	m_orders=new wxChoice(panel, ID_ORDERS, wxDefaultPosition,wxSize(50,-1),10, orders);
	m_orders->SetSelection(0);

	distro->Add(m_orders, 0, wxALIGN_LEFT|wxALL, 3);

	distro->AddSpacer(10);

	// density

	DensitySlider=new ExprSliderCtrl(panel,ID_DENSITY_SLDR,"Density",LABEL2,VALUE1,SLIDER2);
	DensitySlider->setRange(0.0,1.0);
	DensitySlider->setValue(1.0);
	distro->Add(DensitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);


	wxString seed[]={"ID0","ID1","ID2","ID3","ID4","ID5","ID6","ID7","ID8","ID9"};
	m_seed=new wxChoice(panel, ID_SEED, wxDefaultPosition,wxSize(60,-1),10, seed);
	m_seed->SetSelection(0);

	distro->Add(m_seed, 0, wxALIGN_LEFT|wxALL, 3);
	distro->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));

	boxSizer->Add(distro,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer *size = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Max-Delta Size"));

    // size
	ScaleSlider=new SliderCtrl(panel,ID_SCALE_SLDR,"",0, VALUE1,SLIDER2);
	ScaleSlider->setRange(0.1,0.9999);
	ScaleSlider->setValue(1.0);

	size->Add(ScaleSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	wxString exps[]={"100","10","1","0.1","0.01","10-3","10-4","10-5","10-6"};
	m_scale_exp=new wxChoice(panel, ID_SCALE_MULT, wxDefaultPosition,wxSize(60,-1),9, exps);
	m_scale_exp->SetSelection(4);
	size->Add(new wxStaticText(panel,-1,"X"), 0, wxALIGN_LEFT|wxALL, 4);
	size->Add(m_scale_exp, 0, wxALIGN_LEFT|wxALL, 3);

	size->AddSpacer(5);

	DeltaSizeSlider=new ExprSliderCtrl(panel,ID_DELTA_SIZE_SLDR,"",0,VALUE1,SLIDER2);
	DeltaSizeSlider->setRange(0.0,1.0);
	DeltaSizeSlider->setValue(0.5);
	size->Add(DeltaSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	size->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
	boxSizer->Add(size,0,wxALIGN_LEFT|wxALL,0);


    wxStaticBoxSizer* ampl = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Amplitude"));


	AmplSlider=new ExprSliderCtrl(panel,ID_AMPL_SLDR,"Depth",LABEL2, VALUE1,SLIDER2);
	AmplSlider->setRange(0.0,1.0);
	ampl->Add(AmplSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	ImpactSlider=new ExprSliderCtrl(panel,ID_IMPACT_SLDR,"Impact",LABEL1, VALUE1,SLIDER2);
	ImpactSlider->setRange(0,1.0);
	ImpactSlider->setValue(0.0);

	ampl->Add(ImpactSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	ampl->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));

	boxSizer->Add(ampl,0,wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* shape = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Noise"));

	VNoiseSlider=new ExprSliderCtrl(panel,ID_VNOISE_SLDR,"Vertical",LABEL2, VALUE1,SLIDER2);
	VNoiseSlider->setRange(0,1.0);
	shape->Add(VNoiseSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	RNoiseSlider=new ExprSliderCtrl(panel,ID_RNOISE_SLDR,"Radial",LABEL1, VALUE1,SLIDER2);
	RNoiseSlider->setRange(0,1.0);
	shape->Add(RNoiseSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	shape->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
	boxSizer->Add(shape,0,wxALIGN_LEFT|wxALL,0);
}

void VtxCratersFunct::AddShapeTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* geometry = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Geometry"));

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	RiseSlider=new ExprSliderCtrl(panel,ID_RISE_SLDR,"Rise",LABEL1, VALUE1,SLIDER2);
	RiseSlider->setRange(0.0,10.0);
	hline->Add(RiseSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	DropSlider=new ExprSliderCtrl(panel,ID_DROP_SLDR,"Drop",LABEL1, VALUE1,SLIDER2);
	DropSlider->setRange(0,10.0);
	DropSlider->setValue(0.0);

	hline->Add(DropSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT+TABS_BORDER));

	geometry->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	RimSlider=new ExprSliderCtrl(panel,ID_RIM_SLDR,"Rim",LABEL1, VALUE1,SLIDER2);
	RimSlider->setRange(0.0,1.0);
	hline->Add(RimSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	FloorSlider=new ExprSliderCtrl(panel,ID_FLOOR_SLDR,"Floor",LABEL1, VALUE1,SLIDER2);
	FloorSlider->setRange(0,1.0);
	FloorSlider->setValue(0.0);

	hline->Add(FloorSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT+TABS_BORDER));

	geometry->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	CenterSlider=new ExprSliderCtrl(panel,ID_CNTR_SLDR,"Center",LABEL1, VALUE1,SLIDER2);
	CenterSlider->setRange(0.0,1.0);
	hline->Add(CenterSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	OffsetSlider=new ExprSliderCtrl(panel,ID_BIAS_SLDR,"Offset",LABEL1, VALUE1,SLIDER2);
	OffsetSlider->setRange(-1,1);
	hline->Add(OffsetSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	geometry->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(geometry,0,wxALIGN_LEFT|wxALL,0);

}
void VtxCratersFunct::setFunction(wxString f){
	TNcraters *tc=(TNcraters*)TheScene->parse_node((char*)f.ToAscii());
	if(!tc)
		return;

	CraterMgr *mgr=(CraterMgr*)tc->mgr;

	int id=tc->get_id();
	m_seed->SetSelection(id);
	TNarg &args=*((TNarg *)tc->left);
	TNode *a=args[0];

	int levels=mgr->levels;

	if(a){
		a->eval();
		levels=(int)S0.s;
	}
	m_orders->SetSelection(levels-1);
	a=args[1];
	double maxsize=mgr->maxsize;
	if(a){
		a->eval();
		maxsize=S0.s;
	}
	double v=maxsize;
	int i=0;
	double mul=10.0;
	while(v<mul){
		mul/=10;
		i++;
	}
	mul*=10;
	double fm=fmod(v,mul)/mul;
	m_scale_exp->SetSelection(i);
	ScaleSlider->setValue(fm);

	a=args[2];
	if(a)
		DeltaSizeSlider->setValue(a);
	else
		DeltaSizeSlider->setValue(mgr->mult);
	a=args[3];
	if(a)
		DensitySlider->setValue(a);
	else
		DensitySlider->setValue(mgr->density);
	a=args[4];
	if(a)
		AmplSlider->setValue(a);
	else
		AmplSlider->setValue(mgr->ampl);
	a=args[5];
	if(a)
		ImpactSlider->setValue(a);
	else
		ImpactSlider->setValue(mgr->impact);

	a=args[6];
	if(a)
		RNoiseSlider->setValue(a);
	else
		RNoiseSlider->setValue(1.0);

	a=args[7];
	if(a)
		VNoiseSlider->setValue(a);
	else
		VNoiseSlider->setValue(1.0);

	a=args[8];
	if(a)
		RiseSlider->setValue(a);
	else
		RiseSlider->setValue(mgr->rise);
	a=args[9];
	if(a)
		DropSlider->setValue(a);
	else
		DropSlider->setValue(mgr->drop);
	a=args[10];
	if(a)
		RimSlider->setValue(a);
	else
		RimSlider->setValue(mgr->rim);
	a=args[11];
	if(a)
		FloorSlider->setValue(a);
	else
		FloorSlider->setValue(mgr->flr);
	a=args[12];
	if(a)
		CenterSlider->setValue(a);
	else
		CenterSlider->setValue(mgr->ctr);
	a=args[13];
	if(a)
		OffsetSlider->setValue(a);
	else
		OffsetSlider->setValue(mgr->offset);

	delete tc;


}

void VtxCratersFunct::getFunction(){
	char buff[256];

	wxString s="craters(";
	int pid=m_seed->GetSelection();
	if(pid>0){
		sprintf(buff,"ID%d",pid);
		s+=wxString(buff);
		s+=",";
	}
	int orders = m_orders->GetSelection();
	sprintf(buff,"%d",orders+1);
	s+=wxString(buff)+",";

	double f=ScaleSlider->getValue();
	int multiplier=-m_scale_exp->GetSelection()+2;
	double m=pow(10.0,multiplier);

	sprintf(buff,"%g",f*m);
	wxString str(buff);

	s+=str+",";

	s+=DeltaSizeSlider->getText()+",";
	s+=DensitySlider->getText()+",";
	s+=AmplSlider->getText()+",";
	s+=ImpactSlider->getText()+",";
	str=RNoiseSlider->getText();
	if(!str.IsEmpty() && !str.IsSameAs("1.0"))
		s+=str;
	else
		s+="1.0";
	s+=",";
	str=VNoiseSlider->getText();
	if(!str.IsEmpty() && !str.IsSameAs("1.0"))
		s+=str;
	else
		s+="1.0";

	s+=",";
	s+=RiseSlider->getText()+",";
	s+=DropSlider->getText()+",";
	s+=RimSlider->getText()+",";
	s+=FloorSlider->getText()+",";
	s+=CenterSlider->getText()+",";
	s+=OffsetSlider->getText();


	s+=")";
	//cout << s << endl;

	TNcraters *tn=(TNcraters*)TheScene->parse_node((char*)s.ToAscii());
	if(!tn){
		return;
	}
	buff[0]=0;
	tn->valueString(buff);
	s = wxString(buff);
	//cout << buff << endl;
	functDialog->getFunct(s);
}
