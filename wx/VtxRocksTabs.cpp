#include "VtxRocksTabs.h"
#include "VtxSceneDialog.h"

//########################### VtxRocksTabs Class ########################

#define LABEL1 60
#define VALUE1 60
#define SLIDER1 150
#undef SLIDER2
#define SLIDER2  70
// define all resource ids here
enum {
	OBJ_SHOW,
	OBJ_DELETE,
	OBJ_SAVE,

	ID_NAME_TEXT,
    ID_SCALE_SLDR,
    ID_SCALE_TEXT,
    ID_SCALE_MULT,
    ID_ORDERS,
    ID_DELTA_SIZE_SLDR,
    ID_DELTA_SIZE_TEXT,
    ID_DENSITY_SLDR,
    ID_DENSITY_TEXT,
    ID_SEED,
    ID_FLATNESS_SLDR,
    ID_FLATNESS_TEXT,
    ID_DROP_SLDR,
    ID_DROP_TEXT,
    ID_AMPL_SLDR,
    ID_AMPL_TEXT,
    ID_NOISE_EXPR,
};


IMPLEMENT_CLASS(VtxRocksTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxRocksTabs, wxNotebook)

SET_SLIDER_EVENTS(SCALE,VtxRocksTabs,Scale)
SET_SLIDER_EVENTS(DELTA_SIZE,VtxRocksTabs,DeltaSize)
SET_SLIDER_EVENTS(DENSITY,VtxRocksTabs,Density)
SET_SLIDER_EVENTS(FLATNESS,VtxRocksTabs,Flatness)
SET_SLIDER_EVENTS(DROP,VtxRocksTabs,Drop)
SET_SLIDER_EVENTS(AMPL,VtxRocksTabs,Ampl)

EVT_TEXT_ENTER(ID_NOISE_EXPR,VtxRocksTabs::OnChanged)
EVT_TEXT_ENTER(ID_NAME_TEXT,VtxRocksTabs::OnNameText)

EVT_CHOICE(ID_SCALE_MULT, VtxRocksTabs::OnChanged)
EVT_CHOICE(ID_SEED, VtxRocksTabs::OnChanged)
EVT_CHOICE(ID_ORDERS, VtxRocksTabs::OnChanged)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxRocksTabs::OnAddItem)
EVT_MENU(OBJ_DELETE,VtxRocksTabs::OnDelete)
EVT_MENU(OBJ_SHOW,VtxRocksTabs::OnEnable)

SET_FILE_EVENTS(VtxRocksTabs)

END_EVENT_TABLE()

VtxRocksTabs::VtxRocksTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxRocksTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);

    AddPropertiesTab(page);
    AddPage(page,wxT("Rocks"),true);

    return true;
}

int VtxRocksTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;

	wxMenu menu;

	menu.AppendCheckItem(OBJ_SHOW,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));
	menu.Append(OBJ_SAVE,wxT("Save.."));

	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}

	sceneDialog->AddFileMenu(menu,object_node->node);

	PopupMenu(&menu);
	return menu_action;
}

void VtxRocksTabs::AddPropertiesTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* props = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Properties"));

 	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Label",LABEL2+10,VALUE2+SLIDER2);
 	props->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
 	boxSizer->Add(props,0,wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* distro = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Distribution"));

	// levels

    distro->Add(new wxStaticText(panel,-1,"Levels",wxDefaultPosition,wxSize(LABEL2,-1)), 0, wxALIGN_LEFT|wxALL, 4);

	wxString orders[]={"1","2","3","4","5","6","7","8","9","10"};
	m_orders=new wxChoice(panel, ID_ORDERS, wxDefaultPosition,wxSize(50,-1),10, orders);
	m_orders->SetSelection(0);

	distro->Add(m_orders, 0, wxALIGN_LEFT|wxALL, 3);

	distro->AddSpacer(10);

	// density

	DensitySlider=new ExprSliderCtrl(panel,ID_DENSITY_SLDR,"Density",LABEL2S,VALUE1,SLIDER2);
	DensitySlider->setRange(0.0,1.0);
	DensitySlider->setValue(1.0);
	distro->Add(DensitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	distro->AddSpacer(10);

	//distro->Add(new wxStaticText(panel,-1,"L"), 0, wxALIGN_LEFT|wxALL, 4);
	wxString seed[]={"ID0","ID1","ID2","ID3","ID4","ID5","ID6","ID7","ID8","ID9"};
	m_seed=new wxChoice(panel, ID_SEED, wxDefaultPosition,wxSize(60,-1),10, seed);
	m_seed->SetSelection(0);

	distro->Add(m_seed, 0, wxALIGN_LEFT|wxALL, 3);
	distro->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(distro,0,wxALIGN_LEFT|wxALL,0);

    wxBoxSizer *size = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Max-Delta Size"));

    // size
	ScaleSlider=new SliderCtrl(panel,ID_SCALE_SLDR,"",0, VALUE1,SLIDER2);
	ScaleSlider->setRange(0.1,0.9999);
	ScaleSlider->setValue(1.0);

	size->Add(ScaleSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	wxString exps[]={"10","1","0.1","0.01","0.001","1e-4","1e-5","1e-6","1e-7"};
	m_scale_exp=new wxChoice(panel, ID_SCALE_MULT, wxDefaultPosition,wxSize(70,-1),9, exps);
	m_scale_exp->SetSelection(5);
	size->Add(new wxStaticText(panel,-1,"X"), 0, wxALIGN_LEFT|wxALL, 4);
	size->Add(m_scale_exp, 0, wxALIGN_LEFT|wxALL, 3);

	size->AddSpacer(20);

	DeltaSizeSlider=new ExprSliderCtrl(panel,ID_DELTA_SIZE_SLDR,"",0,VALUE1,SLIDER2);
	DeltaSizeSlider->setRange(0.0,1.0);
	DeltaSizeSlider->setValue(0.5);
	size->Add(DeltaSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	size->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(size,0,wxALIGN_LEFT|wxALL,0);


    wxStaticBoxSizer* ampl = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Surface"));

	AmplSlider=new ExprSliderCtrl(panel,ID_AMPL_SLDR,"Ampl",LABEL2, VALUE1,SLIDER2);
	AmplSlider->setRange(0.0,1.0);
	ampl->Add(AmplSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
	
	NoiseExpr=new ExprTextCtrl(panel,ID_NOISE_EXPR,"Noise",LABEL2S,120);
	ampl->Add(NoiseExpr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	ampl->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(ampl,0,wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* shape = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Shape"));

	FlatnessSlider=new ExprSliderCtrl(panel,ID_FLATNESS_SLDR,"Compress",LABEL2, VALUE1,SLIDER2);
	FlatnessSlider->setRange(0,1.0);
	FlatnessSlider->setValue(0.5);

	shape->Add(FlatnessSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	DropSlider=new ExprSliderCtrl(panel,ID_DROP_SLDR,"Drop",LABEL2S, VALUE1,SLIDER2);
	DropSlider->setRange(0,2);
	DropSlider->setValue(0.0);

	shape->Add(DropSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);


	shape->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(shape,0,wxALIGN_LEFT|wxALL,0);

}

void VtxRocksTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

//-------------------------------------------------------------
// VtxRocksTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxRocksTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNrocks *tc=object();
	RockMgr *mgr=(RockMgr*)tc->mgr;

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
	double exp=floor(log10(maxsize));
	uint indx=-exp;
	double rem=0.1*maxsize/pow(10,exp);

	m_scale_exp->SetSelection(indx);
	ScaleSlider->setValue(rem);

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
		FlatnessSlider->setValue(a);
	else
		FlatnessSlider->setValue(mgr->zcomp);
	a=args[5];
	if(a)
		DropSlider->setValue(a);
	else
		DropSlider->setValue(mgr->drop);
	a=args[6];
	if(a)
		AmplSlider->setValue(a);
	else
		AmplSlider->setValue(mgr->noise_ampl);
	a=args[7];
	if(a)
		NoiseExpr->setValue(a);
	else
		NoiseExpr->SetValue(wxString("noise(GRADIENT|NLOD,0,2)"));

	update_needed=false;

}

//-------------------------------------------------------------
// VtxRocksTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxRocksTabs::setObjAttributes(){
	update_needed=true;

	TNrocks *tc=object();
	char id[32];

	wxString s="rocks(";
	int pid=m_seed->GetSelection();
	if(pid>0){
		sprintf(id,"ID%d",pid);
		s+=wxString(id);
		s+=",";
	}
	int orders = m_orders->GetSelection();
	sprintf(id,"%d",orders+1);
	s+=wxString(id)+",";

	double f=ScaleSlider->getValue();
	int multiplier=-m_scale_exp->GetSelection()+1;
	double m=pow(10.0,multiplier);
	char buff[256];
	sprintf(buff,"%g",f*m);
	wxString str(buff);

	s+=str+",";
	s+=DeltaSizeSlider->getText()+",";
	s+=DensitySlider->getText()+",";
	s+=FlatnessSlider->getText()+",";
	s+=DropSlider->getText()+",";

	buff[0]=0;

	s+=AmplSlider->getText();
	if(!NoiseExpr->getText().IsEmpty()){
		s+=",";
		s+=NoiseExpr->getText();
	}
	s+=")";

	//cout << s << endl;
	s+="\n";

	char p[1024];
	strcpy(p,s.ToAscii());
	tc->setExpr(p);
	if(tc->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tc->applyExpr();
	}
	invalidateObject();
}


void VtxRocksTabs::OnEnable(wxCommandEvent& event){
	VtxTabsMgr::OnEnable(event);
	TheScene->rebuild_all();
}

void  VtxRocksTabs::OnDelete(wxCommandEvent& event){
 	menu_action=TABS_DELETE;
 	TheScene->rebuild_all();
}

