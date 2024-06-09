
#include "VtxBranchTabs.h"
#include "VtxSceneDialog.h"
#include "AdaptOptions.h"


#include <wx/filefn.h>
#include <wx/dir.h>

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_WIDTH BOX_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

#define VALUE1 50
#define LABEL1 30
#undef LABEL2
#define LABEL2 60


//########################### VtxBranchTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
	ID_SAVE,

	ID_NAME_TEXT,
    ID_MAX_LEVEL,
	ID_MIN_LEVEL,
    ID_SPLITS_SLDR,
    ID_SPLITS_TEXT,
    ID_LENGTH_SLDR,
    ID_LENGTH_TEXT,
    ID_WIDTH_SLDR,
    ID_WIDTH_TEXT,
    ID_RAND_SLDR,
    ID_RAND_TEXT,
    ID_DIVERGENCE_SLDR,
    ID_DIVERGENCE_TEXT,

    ID_FLATNESS_SLDR,
    ID_FLATNESS_TEXT,
    ID_WIDTH_TAPER_SLDR,
    ID_WIDTH_TAPER_TEXT,
    ID_LENGTH_TAPER_SLDR,
    ID_LENGTH_TAPER_TEXT,
	ID_FIRST_BIAS_SLDR,
	ID_FIRST_BIAS_TEXT,
	ID_FROM_END,
};

#define NAME_WIDTH  50
#define SLDR_WIDTH  180
#define EXPR_WIDTH  200

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 40

//########################### VtxBranchTabs Class ########################

IMPLEMENT_CLASS(VtxBranchTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxBranchTabs, VtxTabsMgr)
EVT_TEXT_ENTER(ID_NAME_TEXT,VtxBranchTabs::OnNameText)


EVT_MENU(ID_DELETE,VtxBranchTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxBranchTabs::OnEnable)
EVT_MENU(ID_SAVE,VtxBranchTabs::OnSave)
EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxBranchTabs::OnAddItem)

EVT_CHOICE(ID_MIN_LEVEL,VtxBranchTabs::OnChangedLevels)
EVT_CHOICE(ID_MAX_LEVEL,VtxBranchTabs::OnChangedLevels)

SET_SLIDER_EVENTS(SPLITS,VtxBranchTabs,Splits)
SET_SLIDER_EVENTS(LENGTH,VtxBranchTabs,Length)
SET_SLIDER_EVENTS(WIDTH,VtxBranchTabs,Width)
SET_SLIDER_EVENTS(RAND,VtxBranchTabs,Rand)
SET_SLIDER_EVENTS(DIVERGENCE,VtxBranchTabs,Divergence)
SET_SLIDER_EVENTS(FLATNESS,VtxBranchTabs,Flatness)
SET_SLIDER_EVENTS(WIDTH_TAPER,VtxBranchTabs,WidthTaper)
SET_SLIDER_EVENTS(LENGTH_TAPER,VtxBranchTabs,LengthTaper)
SET_SLIDER_EVENTS(FIRST_BIAS,VtxBranchTabs,FirstBias)

END_EVENT_TABLE()


VtxBranchTabs::VtxBranchTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxBranchTabs::Create(wxWindow* parent,
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
    AddPage(page,wxT("Properties"),true);
    return true;
}

int VtxBranchTabs::showMenu(bool expanded){

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

	PopupMenu(&menu);
	return menu_action;
}

void VtxBranchTabs::AddPropertiesTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);
    
    // levels
    
    wxBoxSizer *level = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Level"));
 	wxString levels[]={"0","1","2","3","4","5","6","7","8","9","10"};
 	
    level->Add(new wxStaticText(panel,-1,"Max",wxDefaultPosition,wxSize(LABEL1,-1)), 0, wxALIGN_LEFT|wxALL, 4);

    m_max_level=new wxChoice(panel, ID_MAX_LEVEL, wxDefaultPosition,wxSize(50,-1),10, levels);
    m_max_level->SetSelection(1);
    
    level->Add(m_max_level, 0, wxALIGN_LEFT|wxALL, 3);

    level->Add(new wxStaticText(panel,-1,"Min",wxDefaultPosition,wxSize(LABEL1,-1)), 0, wxALIGN_LEFT|wxALL, 4);

    m_min_level=new wxChoice(panel, ID_MIN_LEVEL, wxDefaultPosition,wxSize(50,-1),10, levels);
    m_min_level->SetSelection(0);
    
    level->Add(m_min_level, 0, wxALIGN_LEFT|wxALL, 3);
    
    m_from_end=new wxCheckBox(panel, ID_FROM_END, "From End");
    m_from_end->SetValue(true);
	level->Add(m_from_end, 0, wxALIGN_LEFT|wxALL,8);

    level->AddSpacer(10);
    
	SplitsSlider=new SliderCtrl(panel,ID_SPLITS_SLDR,"Splits",LABEL1, VALUE2,SLIDER2);
	SplitsSlider->setRange(1,10);
	SplitsSlider->setValue(1);

	level->Add(SplitsSlider->getSizer(),0,wxALIGN_LEFT|wxALL,5);
	
	boxSizer->Add(level,0,wxALIGN_LEFT|wxALL,0);

    // size
    
    wxBoxSizer *size = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Size"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	LengthSlider=new SliderCtrl(panel,ID_LENGTH_SLDR,"Length",LABEL2, VALUE2,SLIDER2);
	LengthSlider->setRange(0.1,2);
	LengthSlider->setValue(1);

	hline->Add(LengthSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	LengthTaperSlider=new SliderCtrl(panel,ID_LENGTH_TAPER_SLDR,"Taper",LABEL2,VALUE2,SLIDER2);
	LengthTaperSlider->setRange(0.1,1);
	LengthTaperSlider->setValue(0.8);
	hline->Add(LengthTaperSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	size->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	
	hline = new wxBoxSizer(wxHORIZONTAL);

	WidthSlider=new SliderCtrl(panel,ID_WIDTH_SLDR,"Width",LABEL2, VALUE2,SLIDER2);
	WidthSlider->setRange(0.1,1);
	WidthSlider->setValue(1);

	hline->Add(WidthSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	WidthTaperSlider=new SliderCtrl(panel,ID_WIDTH_TAPER_SLDR,"Taper",LABEL2,VALUE2,SLIDER2);
	WidthTaperSlider->setRange(0.1,1);
	WidthTaperSlider->setValue(0.8);
	hline->Add(WidthTaperSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	size->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(size,0,wxALIGN_LEFT|wxALL,0);

	// other

	wxStaticBoxSizer* other = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Other"));

	hline = new wxBoxSizer(wxHORIZONTAL);
	
	RandSlider=new SliderCtrl(panel,ID_RAND_SLDR,"Randomness",LABEL2, VALUE2,SLIDER2);
	RandSlider->setRange(0,2);
	RandSlider->setValue(1);

	hline->Add(RandSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	FirstBiasSlider=new SliderCtrl(panel,ID_FIRST_BIAS_SLDR,"FirstBias",LABEL2,VALUE2,SLIDER2);
	FirstBiasSlider->setRange(1,100);
	FirstBiasSlider->setValue(1);
	hline->Add(FirstBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	other->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	
	hline = new wxBoxSizer(wxHORIZONTAL);

	DivergenceSlider=new SliderCtrl(panel,ID_DIVERGENCE_SLDR,"Divergece",LABEL2, VALUE2,SLIDER2);
	DivergenceSlider->setRange(0,1);
	DivergenceSlider->setValue(1.0);

	hline->Add(DivergenceSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	FlatnessSlider=new SliderCtrl(panel,ID_FLATNESS_SLDR,"Flatness",LABEL2,VALUE2,SLIDER2);
	FlatnessSlider->setRange(0,1);
	FlatnessSlider->setValue(0.0);
	hline->Add(FlatnessSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	other->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(other,0,wxALIGN_LEFT|wxALL,0);

}

void VtxBranchTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

void VtxBranchTabs::OnChangedLevels(wxCommandEvent& event){
	setObjAttributes();
}

wxString VtxBranchTabs::exprString(){
	char p[1024]="";
	TNBranch *obj=object();

	sprintf(p,"Branch(");
	if(strlen(obj->name_str)){
		sprintf(p+strlen(p),"\"%s\",",obj->name_str);
	}

	sprintf(p+strlen(p),"%d,",m_max_level->GetSelection());
	wxString s(p);
	s+=SplitsSlider->getText()+",";
	s+=LengthSlider->getText()+",";
	s+=WidthSlider->getText()+",";
	s+=RandSlider->getText()+",";
	s+=DivergenceSlider->getText()+",";
	s+=FlatnessSlider->getText()+",";
	s+=WidthTaperSlider->getText()+",";
	s+=LengthTaperSlider->getText()+",";
	s+=FirstBiasSlider->getText()+",";
	int minlvl=m_min_level->GetSelection();
	if(m_from_end->GetValue())
		sprintf(p,"-%d",minlvl);
	else
		sprintf(p,"%d",minlvl);
	s+=p;
	s+=")";
 	return wxString(s);
}

//-------------------------------------------------------------
// VtxBranchTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxBranchTabs::setObjAttributes(){
	TNBranch *obj=object();

	wxString s=exprString();
	
	if(strlen(obj->name_str))
		sceneDialog->setNodeName(obj->name_str);

	cout<<"set:"<<s.ToAscii()<<endl;

	obj->setExpr((char*)s.ToAscii());
	obj->applyExpr();
	TheView->set_changed_detail();
	TheScene->rebuild();

	update_needed=false;
}
//-------------------------------------------------------------
// VtxBranchTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxBranchTabs::getObjAttributes(){
	if(!update_needed)
		return;
	update_needed=false;

	TNBranch *obj=object();

	if(strlen(obj->name_str))
		sceneDialog->setNodeName(obj->name_str);

	m_max_level->SetSelection(obj->max_level);
	m_min_level->SetSelection(obj->min_level);

	TNarg &args=*((TNarg *)obj->left);
	TNode *a=args[1];

	if(a)
		SplitsSlider->setValue(a);
	else
		SplitsSlider->setValue(1);

	a=args[2];
	if(a)
		LengthSlider->setValue(a);
	else
		LengthSlider->setValue(1);
	
	a=args[3];
	if(a)
		WidthSlider->setValue(a);
	else
		WidthSlider->setValue(1);

	a=args[4];
	if(a)
		RandSlider->setValue(a);
	else
		RandSlider->setValue(1);

	a=args[5];
	if(a)
		DivergenceSlider->setValue(a);
	else
		DivergenceSlider->setValue(0.5);

	a=args[6];
	if(a)
		FlatnessSlider->setValue(a);
	else
		FlatnessSlider->setValue(0.0);

	a=args[7];
	if(a)
		WidthTaperSlider->setValue(a);
	else
		WidthTaperSlider->setValue(0.75);

	a=args[8];
	if(a)
		LengthTaperSlider->setValue(a);
	else
		LengthTaperSlider->setValue(0.75);

	a=args[9];
	if(a)
		FirstBiasSlider->setValue(a);
	else
		FirstBiasSlider->setValue(0.0);

}



