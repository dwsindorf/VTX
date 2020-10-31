
#include "VtxCloudsTabs.h"
#include "UniverseModel.h"
#include "TerrainClass.h"

#undef SLIDER2
#undef LABEL
#define LABEL   70
#define SLIDER2 90

//########################### ObjectAddMenu Class ########################
enum{
	OBJ_NONE,
	OBJ_SHOW,
	OBJ_VIEWOBJ,
	OBJ_DELETE,
	OBJ_SAVE,

	ID_NAME_TEXT,
	ID_TYPE_TEXT,
    ID_CELLSIZE_SLDR,
    ID_CELLSIZE_TEXT,

    ID_HEIGHT_SLDR,
    ID_HEIGHT_TEXT,
    ID_TILT_SLDR,
    ID_TILT_TEXT,
    ID_ROT_PHASE_SLDR,
    ID_ROT_PHASE_TEXT,
    ID_DAY_SLDR,
    ID_DAY_TEXT,
    ID_SHINE_SLDR,
    ID_SHINE_TEXT,
    ID_AMBIENT_SLDR,
    ID_AMBIENT_TEXT,
    ID_AMBIENT_COLOR,
    ID_SPECULAR_SLDR,
    ID_SPECULAR_TEXT,
    ID_SPECULAR_COLOR,
    ID_DIFFUSE_SLDR,
    ID_DIFFUSE_TEXT,
    ID_DIFFUSE_COLOR,
    ID_EMISSION_SLDR,
    ID_EMISSION_TEXT,
    ID_EMISSION_COLOR,

    ID_TOP_EXPR,
    ID_BOTTOM_EXPR,
    ID_TYPE_EXPR,
    ID_CMIN_SLDR,
    ID_CMIN_TEXT,
    ID_CMAX_SLDR,
    ID_CMAX_TEXT,
    ID_CROT_SLDR,
    ID_CROT_TEXT,
    ID_SMAX_SLDR,
    ID_SMAX_TEXT,

    ID_NUM_SPRITES_SLDR,
    ID_NUM_SPRITES_TEXT,

    ID_DIFFUSION_SLDR,
    ID_DIFFUSION_TEXT,

};

//########################### VtxCloudsTabs Class ########################

IMPLEMENT_CLASS(VtxCloudsTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxCloudsTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxCloudsTabs::OnNameText)

SET_SLIDER_EVENTS(CELLSIZE,VtxCloudsTabs,CellSize)
EVT_COMMAND_SCROLL(ID_HEIGHT_SLDR,VtxCloudsTabs::OnHeightSlider)
EVT_TEXT_ENTER(ID_HEIGHT_TEXT,VtxCloudsTabs::OnHeightText)

SET_SLIDER_EVENTS(TILT,VtxCloudsTabs,Tilt)
SET_SLIDER_EVENTS(DAY,VtxCloudsTabs,Day)
SET_SLIDER_EVENTS(ROT_PHASE,VtxCloudsTabs,RotPhase)
SET_SLIDER_EVENTS(SHINE,VtxCloudsTabs,Shine)
SET_SLIDER_EVENTS(NUM_SPRITES,VtxCloudsTabs,NumSprites)
SET_SLIDER_EVENTS(DIFFUSION,VtxCloudsTabs,Diffusion)
SET_COLOR_EVENTS(AMBIENT,VtxCloudsTabs,Ambient)
SET_COLOR_EVENTS(EMISSION,VtxCloudsTabs,Emission)
SET_COLOR_EVENTS(SPECULAR,VtxCloudsTabs,Specular)
SET_COLOR_EVENTS(DIFFUSE,VtxCloudsTabs,Diffuse)

EVT_TEXT_ENTER(ID_TOP_EXPR,VtxCloudsTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_BOTTOM_EXPR,VtxCloudsTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_TYPE_EXPR,VtxCloudsTabs::OnChangedExpr)

SET_SLIDER_EVENTS(CMIN,VtxCloudsTabs,Cmin)
SET_SLIDER_EVENTS(CMAX,VtxCloudsTabs,Cmax)
SET_SLIDER_EVENTS(SMAX,VtxCloudsTabs,Smax)
SET_SLIDER_EVENTS(CROT,VtxCloudsTabs,Crot)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxCloudsTabs::OnAddItem)
EVT_MENU(OBJ_SAVE,VtxCloudsTabs::OnSave)

EVT_MENU(OBJ_SHOW,VtxCloudsTabs::OnEnable)
EVT_MENU(OBJ_DELETE,VtxCloudsTabs::OnDelete)

EVT_UPDATE_UI(OBJ_SHOW,VtxCloudsTabs::OnUpdateEnable)

END_EVENT_TABLE()

VtxCloudsTabs::VtxCloudsTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

VtxCloudsTabs::~VtxCloudsTabs(){

	delete CellSizeSlider;
	delete HeightSlider;
	delete TiltSlider;
	delete DaySlider;
	delete RotPhaseSlider;

	delete ShineSlider;
	delete AmbientSlider;
	delete SpecularSlider;
	delete EmissionSlider;
	delete DiffuseSlider;

	delete top_expr;
	delete bottom_expr;
	delete type_expr;

	delete CminSlider;
	delete CmaxSlider;
	delete SmaxSlider;
	delete CrotSlider;

	delete NumSpritesSlider;
	delete DiffusionSlider;

}
int VtxCloudsTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;
	wxMenu menu;
	menu.AppendCheckItem(OBJ_SHOW,wxT("Show"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));
	menu.Append(OBJ_SAVE,wxT("Save.."));

	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	PopupMenu(&menu);
	return menu_action;
}

bool VtxCloudsTabs::Create(wxWindow* parent,
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
    AddPage(page,wxT("Object"),true);

    page=new wxPanel(this,wxID_ANY);
    AddLightingTab(page);
    AddPage(page,wxT("Lighting"),false);

    page=new wxPanel(this,wxID_ANY);
    AddGeometryTab(page);
    AddPage(page,wxT("Volume"),false);
    geometry_page=page;


    return true;
}
void VtxCloudsTabs::AddObjectTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* object_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Orbital"));
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2,VALUE2+SLIDER2);
	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->AddSpacer(5);

	CellSizeSlider=new SliderCtrl(panel,ID_CELLSIZE_SLDR,"Grid",LABEL2S, VALUE2,SLIDER2);
	CellSizeSlider->setRange(1,4);
	CellSizeSlider->setValue(1);
	CellSizeSlider->slider->SetToolTip("Grid Size");
	hline->Add(CellSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
    object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
    hline= new wxBoxSizer(wxHORIZONTAL);

    HeightSlider=new SliderCtrl(panel,ID_HEIGHT_SLDR,"Height",LABEL2, VALUE2,SLIDER2);
	HeightSlider->setRange(10,100);
	HeightSlider->setValue(20);
	HeightSlider->slider->SetToolTip("Layer Height (Miles)");
	hline->Add(HeightSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	TiltSlider=new SliderCtrl(panel,ID_TILT_SLDR,"Tilt",LABEL2S, VALUE2,SLIDER2);
	TiltSlider->setRange(0,360);
	TiltSlider->setValue(0.0);
	TiltSlider->slider->SetToolTip("Pole Tilt (Degrees)");

	hline->Add(TiltSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	DaySlider=new SliderCtrl(panel,ID_DAY_SLDR,"Period",LABEL2, VALUE2,SLIDER2);
	DaySlider->setRange(-20,20);
	DaySlider->slider->SetToolTip("Rotation Period Relative To Parent (Hours)");

	DaySlider->setValue(0.0);

	hline->Add(DaySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	RotPhaseSlider=new SliderCtrl(panel,ID_ROT_PHASE_SLDR,"Phase",LABEL2S, VALUE2,SLIDER2);
	RotPhaseSlider->setRange(0,360);
	RotPhaseSlider->slider->SetToolTip("Rotation Phase (Degrees)");
	RotPhaseSlider->setValue(0.0);

	hline->Add(RotPhaseSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	object_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	object_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(object_cntrls, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxCloudsTabs::AddLightingTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* color_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Color"));

    DiffusionSlider=new SliderCtrl(panel,ID_DIFFUSION_SLDR,"Saturation",LABEL, VALUE,SLIDER);
    DiffusionSlider->setRange(0,1);
    color_cntrls->Add(DiffusionSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

	ShineSlider=new SliderCtrl(panel,ID_SHINE_SLDR,"Shine",LABEL, VALUE,SLIDER);
    ShineSlider->setRange(0.25,100);
    color_cntrls->Add(ShineSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    AmbientSlider=new ColorSlider(panel,ID_AMBIENT_SLDR,"Ambient",LABEL,VALUE,CSLIDER,CBOX1);
    color_cntrls->Add(AmbientSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
    SpecularSlider=new ColorSlider(panel,ID_SPECULAR_SLDR,"Specular",LABEL,VALUE,CSLIDER,CBOX1);
    color_cntrls->Add(SpecularSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
	DiffuseSlider=new ColorSlider(panel,ID_DIFFUSE_SLDR,"Diffuse",LABEL,VALUE,CSLIDER,CBOX1);
	color_cntrls->Add(DiffuseSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
    EmissionSlider=new ColorSlider(panel,ID_EMISSION_SLDR,"Radiance",LABEL,VALUE,CSLIDER,CBOX1);
    color_cntrls->Add(EmissionSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
    color_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(color_cntrls, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxCloudsTabs::AddGeometryTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* shape_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Shape"));

	top_expr = new ExprTextCtrl(panel,ID_TOP_EXPR,"",0,TABS_WIDTH-60-TABS_BORDER);
	bottom_expr = new ExprTextCtrl(panel,ID_BOTTOM_EXPR,"",0,TABS_WIDTH-60-TABS_BORDER);
	type_expr = new ExprTextCtrl(panel,ID_TYPE_EXPR,"",0,TABS_WIDTH-60-TABS_BORDER);

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Top", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(top_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	shape_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Bottom", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(bottom_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	shape_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Type", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(type_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	shape_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(shape_cntrls,0,wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* size_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Sprites"));

	hline = new wxBoxSizer(wxHORIZONTAL);

	NumSpritesSlider=new SliderCtrl(panel,ID_NUM_SPRITES_SLDR,"Num",40, 50,SLIDER2);
	NumSpritesSlider->setRange(1,8);
	NumSpritesSlider->slider->SetToolTip("Number of Sprites per vertex");
    hline->Add(NumSpritesSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    SmaxSlider=new SliderCtrl(panel,ID_SMAX_SLDR,"Ext",40, 50,SLIDER2);
    SmaxSlider->setRange(0,8);
    SmaxSlider->slider->SetToolTip("Minimum Extent");
    hline->Add(SmaxSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    size_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

    CminSlider=new SliderCtrl(panel,ID_CMIN_SLDR,"Size",40, 50,SLIDER2);
    CminSlider->setRange(1,128);
    CminSlider->slider->SetToolTip("Minimum Point Size");
    hline->Add(CminSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    CmaxSlider=new SliderCtrl(panel,ID_CMAX_SLDR,"Range",40, 50,SLIDER2);
    CmaxSlider->setRange(1,4);
    CmaxSlider->slider->SetToolTip("Point Size Range");
    hline->Add(CmaxSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    size_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    boxSizer->Add(size_cntrls,0,wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* rot_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Sprite Rotation"));

	CrotSlider=new SliderCtrl(panel,ID_CROT_SLDR,"Angle",40, 50,TABS_WIDTH-130);
	CrotSlider->setRange(0,1,0,360);
	CrotSlider->slider->SetToolTip("Random Rotation Angle");
	rot_cntrls->Add(CrotSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

	boxSizer->Add(rot_cntrls,0,wxALIGN_LEFT|wxALL,0);

}

void VtxCloudsTabs::OnChangedExpr(wxCommandEvent& event){
	invalidateObject();
}

//-------------------------------------------------------------
// VtxCloudsTabs::getCloudsExpr() return clouds value string
//-------------------------------------------------------------
wxString VtxCloudsTabs::getCloudsExpr(){
	char top[128]="HT";
	char bottom[128]="-0.25*HT";
	char type[128]="1.0+10.0*HT";

	if(strlen(top_expr->GetValue().ToAscii()))
		strcpy(top,top_expr->GetValue().ToAscii());
	if(strlen(bottom_expr->GetValue().ToAscii()))
		strcpy(bottom,bottom_expr->GetValue().ToAscii());
	if(strlen(type_expr->GetValue().ToAscii()))
		strcpy(type,type_expr->GetValue().ToAscii());
	char cstr[256];
	sprintf(cstr,"clouds(%s,%s,%s)",top,bottom,type);
    return wxString(cstr);
}

//-------------------------------------------------------------
// VtxCloudsTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxCloudsTabs::getObjAttributes(){
	if(!update_needed)
		return;
	CloudLayer *obj=object();

	double val=(obj->size-parentSize())/MILES;
	updateSlider(CellSizeSlider,obj->detail);
	updateSlider(HeightSlider,val);
	updateSlider(TiltSlider,obj->tilt);
	updateSlider(RotPhaseSlider,obj->rot_phase);
	updateSlider(DaySlider,obj->day);
	updateSlider(ShineSlider,obj->shine);
	//updateSlider(DensitySlider,obj->density);
	updateSlider(DiffusionSlider,obj->diffusion);

	updateSlider(CminSlider,obj->cmin);
	updateSlider(CmaxSlider,obj->cmax);
	updateSlider(SmaxSlider,obj->smax);
	updateSlider(CrotSlider,obj->crot);
	updateSlider(NumSpritesSlider,obj->num_sprites);

	updateColor(AmbientSlider,obj->ambient);
	updateColor(EmissionSlider,obj->emission);
	updateColor(SpecularSlider, obj->specular);
	updateColor(DiffuseSlider,obj->diffuse);
	object_name->SetValue(object_node->node->nodeName());

	update_needed=false;

	TNclouds *tnode=obj->getClouds();
	if(!tnode)
		return;

	char top[128]="";
	char bottom[128]="";
	char type[128]="";
	char offset[128]="";

	TNarg &args=*((TNarg *)tnode->left);

	if(args[0])
		args[0]->valueString(top);
	if(args[1])
		args[1]->valueString(bottom);
	if(args[2])
		args[2]->valueString(type);
	if(args[3])
		args[3]->valueString(offset);

	top_expr->SetValue(top);
	bottom_expr->SetValue(bottom);
	type_expr->SetValue(type);
}

//-------------------------------------------------------------
// VtxCloudsTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxCloudsTabs::setObjAttributes(){
	update_needed=true;
	TNclouds *tnode=object()->getClouds();
	if(!tnode)
		return;

	object()->invalidate();

	wxString str=getCloudsExpr();

	//cout << str << endl;

	tnode->setExpr((char*)(str.mb_str()));
	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
        TheView->set_changed_detail();
        TheScene->rebuild();
	}
}

//-------------------------------------------------------------
// VtxCloudsTabs::updateControls() update UI
//-------------------------------------------------------------
void VtxCloudsTabs::updateControls(){
	CloudLayer *obj=object();
	int pages=GetPageCount();
	if(obj->threeD()){
		if(pages==2)
			InsertPage(2,geometry_page,"Volume",false,-1);
	}
	else if(pages==3){
		RemovePage(2);
	}
	if(update_needed){
		getObjAttributes();
	}
}
