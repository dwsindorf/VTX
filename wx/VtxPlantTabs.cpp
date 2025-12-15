
#include "VtxPlantTabs.h"
#include "VtxSceneDialog.h"
#include "AdaptOptions.h"


#include <wx/filefn.h>
#include <wx/dir.h>

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_WIDTH BOX_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

#define VALUE1 50
#define LABEL1 50
#undef LABEL2
#define LABEL2 60


//########################### VtxPlantTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
	ID_SAVE,

	ID_NAME_TEXT,
    ID_SIZE_SLDR,
    ID_SIZE_TEXT,
    ID_DELTA_SIZE_SLDR,
    ID_DELTA_SIZE_TEXT,
    ID_LEVELS,
    ID_LEVELS_SIZE_SLDR,
    ID_LEVELS_SIZE_TEXT,
    ID_DENSITY_SLDR,
    ID_DENSITY_TEXT,

    ID_SLOPE_BIAS_SLDR,
    ID_SLOPE_BIAS_TEXT,
    ID_PHI_BIAS_SLDR,
    ID_PHI_BIAS_TEXT,
    ID_HT_BIAS_SLDR,
    ID_HT_BIAS_TEXT,
    ID_HARD_BIAS_SLDR,
    ID_HARD_BIAS_TEXT,
	ID_DROP_SLDR,
	ID_DROP_TEXT,
	ID_3D,
	ID_SPLINES,
	ID_LINES,
	ID_SHOW_ONE,
};

#define NAME_WIDTH  50
#define SLDR_WIDTH  180
#define EXPR_WIDTH  200

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 40

//########################### VtxPlantTabs Class ########################

IMPLEMENT_CLASS(VtxPlantTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxPlantTabs, VtxTabsMgr)
EVT_TEXT_ENTER(ID_NAME_TEXT,VtxPlantTabs::OnNameText)


EVT_MENU(ID_DELETE,VtxPlantTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxPlantTabs::OnEnable)
EVT_MENU(ID_SAVE,VtxPlantTabs::OnSave)
EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxPlantTabs::OnAddItem)

EVT_CHOICE(ID_LEVELS,VtxPlantTabs::OnChangedLevels)
SET_SLIDER_EVENTS(SIZE,VtxPlantTabs,Size)
SET_SLIDER_EVENTS(DELTA_SIZE,VtxPlantTabs,DeltaSize)
SET_SLIDER_EVENTS(DENSITY,VtxPlantTabs,Density)
SET_SLIDER_EVENTS(SLOPE_BIAS,VtxPlantTabs,SlopeBias)
SET_SLIDER_EVENTS(PHI_BIAS,VtxPlantTabs,PhiBias)
SET_SLIDER_EVENTS(HT_BIAS,VtxPlantTabs,HtBias)
SET_SLIDER_EVENTS(HARD_BIAS,VtxPlantTabs,HardBias)
SET_SLIDER_EVENTS(DROP,VtxPlantTabs,Drop)

EVT_CHECKBOX(ID_3D,VtxPlantTabs::OnChangedDisplay)
EVT_CHECKBOX(ID_LINES,VtxPlantTabs::OnChangedDisplay)
EVT_CHECKBOX(ID_SPLINES,VtxPlantTabs::OnChangedDisplay)
EVT_CHECKBOX(ID_SHOW_ONE,VtxPlantTabs::OnChangedDisplay)

SET_FILE_EVENTS(VtxPlantTabs)

END_EVENT_TABLE()

VtxPlantTabs::VtxPlantTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxPlantTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
 	AddDistribTab(page);
    AddPage(page,wxT("Properties"),true);
    return true;
}

int VtxPlantTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));
	menu.Append(ID_SAVE,wxT("Save.."));

	wxMenu *addmenu=sceneDialog->getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	sceneDialog->AddFileMenu(menu,object_node->node);

	PopupMenu(&menu);
	return menu_action;
}

void VtxPlantTabs::AddDistribTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticBoxSizer* props = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Properties"));
    
	//wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",50,100);
	props->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	props->Add(new wxStaticText(panel,-1,"Levels",wxDefaultPosition,wxSize(40,-1)), 0, wxALIGN_LEFT|wxALL,5);

	wxString orders[]={"1","2","3","4","5","6","7","8","9","10"};
	m_orders=new wxChoice(panel, ID_LEVELS, wxDefaultPosition,wxSize(50,-1),10, orders);
	m_orders->SetSelection(0);
	
	props->Add(m_orders, 0, wxALIGN_LEFT|wxALL, 0);
	
	DensitySlider=new ExprSliderCtrl(panel,ID_DENSITY_SLDR,"Density",LABEL1,VALUE2,SLIDER2);
	DensitySlider->setRange(0.0,1.0);
	DensitySlider->setValue(1.0);
	props->Add(DensitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	props->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
	
	boxSizer->Add(props,0,wxALIGN_LEFT|wxALL,0);
		
    wxStaticBoxSizer* size = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Size"));
    
	SizeSlider=new SliderCtrl(panel,ID_SIZE_SLDR,"Max",LABEL2,VALUE2,SLIDER2);
	SizeSlider->setRange(1,200);
	SizeSlider->setValue(10.0);

	size->Add(SizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	DeltaSizeSlider=new ExprSliderCtrl(panel,ID_DELTA_SIZE_SLDR,"Delta",LABEL2,VALUE2,SLIDER2);
	DeltaSizeSlider->setRange(0.0,2);
	DeltaSizeSlider->setValue(0.0);
	
	size->Add(DeltaSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	size->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
	
	boxSizer->Add(size,0,wxALIGN_LEFT|wxALL,0);
	
	// biases
	
	wxStaticBoxSizer* bias = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Density Bias"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	//hline = new wxBoxSizer(wxHORIZONTAL);
	
	SlopeBiasSlider=new ExprSliderCtrl(panel,ID_SLOPE_BIAS_SLDR,"Slope",LABEL2,VALUE2,SLIDER2);
	SlopeBiasSlider->setRange(-1,1);
	SlopeBiasSlider->setValue(0.0);

	hline->Add(SlopeBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	HardBiasSlider=new ExprSliderCtrl(panel,ID_HARD_BIAS_SLDR,"Hardness",LABEL2,VALUE2,SLIDER2);
	HardBiasSlider->setRange(-1,1);
	HardBiasSlider->setValue(0.0);

	hline->Add(HardBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	bias->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	
	hline = new wxBoxSizer(wxHORIZONTAL);
	
	PhiBiasSlider=new ExprSliderCtrl(panel,ID_PHI_BIAS_SLDR,"Latitude",LABEL2,VALUE2,SLIDER2);
	PhiBiasSlider->setRange(-1,1);
	PhiBiasSlider->setValue(0.0);

	hline->Add(PhiBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	HtBiasSlider=new ExprSliderCtrl(panel,ID_HT_BIAS_SLDR,"Height",LABEL2, VALUE2,SLIDER2);
	HtBiasSlider->setRange(-1,1);
	HtBiasSlider->setValue(0.0);

	hline->Add(HtBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	bias->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	bias->SetMinSize(wxSize(BOX_WIDTH,2*LINE_HEIGHT+TABS_BORDER));

	boxSizer->Add(bias,0,wxALIGN_LEFT|wxALL,0);
	
    wxStaticBoxSizer* misc = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Other"));
	
	DropSlider=new ExprSliderCtrl(panel,ID_DROP_SLDR,"Drop",LABEL2, VALUE2,SLIDER2);
	DropSlider->setRange(0,2);
	DropSlider->setValue(0.0);
	
	misc->Add(DropSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	m_skeleton=new wxCheckBox(panel, ID_3D, "Skeleton");
	m_skeleton->SetValue(true);
	misc->Add(m_skeleton, 0, wxALIGN_LEFT|wxTOP,5);
	
	m_lines=new wxCheckBox(panel, ID_LINES, "Lines");
	m_lines->SetValue(false);
	misc->Add(m_lines, 0, wxALIGN_LEFT|wxTOP,5);

	m_splines=new wxCheckBox(panel, ID_SPLINES, "Splines");
	m_splines->SetValue(true);
	misc->Add(m_splines, 0, wxALIGN_LEFT|wxTOP,5);

	m_show_one=new wxCheckBox(panel, ID_SPLINES, "One");
	m_show_one->SetValue(false);
	misc->Add(m_show_one, 0, wxALIGN_LEFT|wxTOP,5);
	
	misc->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));

	//misc->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(misc, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxPlantTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

void VtxPlantTabs::OnChangedLevels(wxCommandEvent& event){
	setObjAttributes();
}
void VtxPlantTabs::OnChangedDisplay(wxCommandEvent& event){
	PlantMgr::shader_lines=m_skeleton->GetValue();
	PlantMgr::spline=m_splines->GetValue();
	PlantMgr::poly_lines=m_lines->GetValue();
	PlantMgr::show_one=m_show_one->GetValue();
	TheScene->set_changed_detail();
}
void VtxPlantTabs::getDisplayState(){
	m_skeleton->SetValue(PlantMgr::shader_lines);
	m_splines->SetValue(PlantMgr::spline);
	m_lines->SetValue(PlantMgr::poly_lines);
	m_show_one->SetValue(PlantMgr::show_one);
}
wxString VtxPlantTabs::exprString(){
	char p[1024]="";
	TNplant *obj=object();

	sprintf(p,"Plant(");
	if(strlen(obj->name_str)){
		sprintf(p+strlen(p),"\"%s\",",obj->name_str);
	}

	sprintf(p+strlen(p),"%d,",m_orders->GetSelection()+1);
	sprintf(p+strlen(p),"%g,",FEET*SizeSlider->getValue()/obj->radius);
	wxString s(p);

	s+=DeltaSizeSlider->getText()+",";
	s+=DensitySlider->getText()+",";
	s+="0,"; // compr (not used)
	s+=DropSlider->getText();
	s+=",0,0,"; // noise_amp noise_expr (not used)
	s+=SlopeBiasSlider->getText()+",";
	s+=HtBiasSlider->getText()+",";
	s+=PhiBiasSlider->getText()+",";
	s+=HardBiasSlider->getText();
	s+=")";
 	return wxString(s);
}

//-------------------------------------------------------------
// VtxPlantTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxPlantTabs::setObjAttributes(){
	TNplant *obj=object();
	PlantMgr *smgr=(PlantMgr*)obj->mgr;

	wxString s=exprString();
	
	if(strlen(obj->name_str))
		sceneDialog->setNodeName(obj->name_str);

	//cout<<"set:"<<s.ToAscii()<<endl;

	obj->setExpr((char*)s.ToAscii());
	obj->applyExpr();
	TheView->set_changed_detail();
	//TheScene->rebuild_all();

	update_needed=false;
}
//-------------------------------------------------------------
// VtxPlantTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxPlantTabs::getObjAttributes(){
	if(!update_needed)
		return;
	update_needed=false;

	TNplant *obj=object();
	PlantMgr *mgr=(PlantMgr*)obj->mgr;
	//cout<<"get:";

	object_name->SetValue(obj->nodeName());

	m_orders->SetSelection(mgr->levels-1);
	double maxsize=mgr->maxsize;
	SizeSlider->setValue(mgr->maxsize*obj->radius/FEET);

	char tmp[1024];
	tmp[0]=0;
	
	TNarg &args=*((TNarg *)obj->left);

	TNode *a=args[2];
	a->valueString(tmp);
	if(a)
		DeltaSizeSlider->setValue(a);
	else
		DeltaSizeSlider->setValue(mgr->mult);
	a=args[3];

	if(a)
		DensitySlider->setValue(a);
	else
	a=args[5];
	if(a)
		DropSlider->setValue(a);
	else
		DropSlider->setValue(mgr->drop);
	// args[6]=noise_ampl
	// args[7]=noise_expr
	a=args[8];
	if(a)
		SlopeBiasSlider->setValue(a);
	else
		SlopeBiasSlider->setValue(mgr->slope_bias);
	a=args[9];
	if(a)
		HtBiasSlider->setValue(a);
	else
		HtBiasSlider->setValue(mgr->ht_bias);
	a=args[10];
	if(a)
		PhiBiasSlider->setValue(a);
	else
		PhiBiasSlider->setValue(mgr->lat_bias);
	cout<<PhiBiasSlider->getText()+",";

	a=args[11];
	if(a)
		HardBiasSlider->setValue(a);
	else
		HardBiasSlider->setValue(mgr->hardness_bias);
    getDisplayState();


}



