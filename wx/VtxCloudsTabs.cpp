
#include "VtxCloudsTabs.h"
#include "UniverseModel.h"
#include "TerrainClass.h"
#include "FileUtil.h"
#include "VtxSceneDialog.h"

#include <wx/dir.h>


#undef SLIDER2
#undef LABEL
#undef LABEL1
#undef LABEL2
#undef LABEL2S

#define LABEL   70
#define LABEL1   55
#define LABEL2   50
#define LABEL2S   45

#define SLIDER2 120
#define BTNWIDTH 75
#define BTNSIZE wxSize(BTNWIDTH,30)


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

	ID_SPRITES_FILE,
	ID_FILELIST,

	ID_SPRITES_DIM,
	ID_CLOUDS_DIM,

};

//########################### VtxCloudsTabs Class ########################

IMPLEMENT_CLASS(VtxCloudsTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxCloudsTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxCloudsTabs::OnNameText)

EVT_COMMAND_SCROLL(ID_HEIGHT_SLDR,VtxCloudsTabs::OnHeightSlider)
EVT_TEXT_ENTER(ID_HEIGHT_TEXT,VtxCloudsTabs::OnHeightText)

SET_SLIDER_EVENTS(CELLSIZE,VtxCloudsTabs,CellSize)
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
EVT_TEXT_ENTER(ID_TYPE_EXPR,VtxCloudsTabs::OnChangedTypeExpr)

EVT_TEXT_ENTER(ID_SPRITES_FILE,VtxCloudsTabs::OnChangedFile)

SET_SLIDER_EVENTS(CMIN,VtxCloudsTabs,Cmin)
SET_SLIDER_EVENTS(CMAX,VtxCloudsTabs,Cmax)
SET_SLIDER_EVENTS(SMAX,VtxCloudsTabs,Smax)
SET_SLIDER_EVENTS(CROT,VtxCloudsTabs,Crot)

EVT_CHOICE(ID_SPRITES_DIM,VtxCloudsTabs::OnDimSelect)
EVT_CHOICE(ID_CLOUDS_DIM,VtxCloudsTabs::OnCloudsDimSelect)

EVT_CHOICE(ID_FILELIST,VtxCloudsTabs::OnChangedFile)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxCloudsTabs::OnAddItem)
EVT_MENU(OBJ_SAVE,VtxCloudsTabs::OnSave)

EVT_MENU(OBJ_SHOW,VtxCloudsTabs::OnEnable)
EVT_MENU(OBJ_DELETE,VtxCloudsTabs::OnDelete)

EVT_UPDATE_UI(OBJ_SHOW,VtxCloudsTabs::OnUpdateEnable)

SET_FILE_EVENTS(VtxCloudsTabs)

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
	sceneDialog->AddFileMenu(menu,object_node->node);

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
    image_path="";
    changed_cell_expr=true;
    image_dim=0;
    image_name="";
	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
    AddObjectTab(page);
    AddPage(page,wxT("Object"),true);

    page=new wxPanel(this,wxID_ANY);
    AddLightingTab(page);
    AddPage(page,wxT("Lighting"),false);

    page=new wxPanel(this,wxID_ANY);
    AddGeometryTab(page);
    geometry_page=page;

    page=new wxPanel(this,wxID_ANY);
    AddImagesTab(page);
    sprites_page=page;

    add3DTabs();
    return true;
}

void VtxCloudsTabs::add3DTabs(){
    AddPage(geometry_page,wxT("Volume"),false);
    AddPage(sprites_page,wxT("Images"),false);
}

void VtxCloudsTabs::AddObjectTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* object_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Orbital"));
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2,VALUE2+SLIDER2-40);
	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	wxString dims[]={"2D","3D"};

	clouds_dim=new wxChoice(panel, ID_CLOUDS_DIM, wxDefaultPosition,wxSize(50,-1),2, dims);
	clouds_dim->SetSelection(0);
	hline->Add(clouds_dim,0,wxALIGN_LEFT|wxALL,0);

	hline->AddSpacer(5);

	CellSizeSlider=new SliderCtrl(panel,ID_CELLSIZE_SLDR,"Grid",35, VALUE2,SLIDER2);
	CellSizeSlider->setRange(1,8);
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

//    DiffusionSlider=new SliderCtrl(panel,ID_DIFFUSION_SLDR,"Saturation",LABEL, VALUE,SLIDER);
//    DiffusionSlider->setRange(0,1);
//    color_cntrls->Add(DiffusionSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

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

	wxStaticBoxSizer* shape_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("3D Shape"));

	top_expr = new ExprTextCtrl(panel,ID_TOP_EXPR,"",0,TABS_WIDTH-60-TABS_BORDER);
	bottom_expr = new ExprTextCtrl(panel,ID_BOTTOM_EXPR,"",0,TABS_WIDTH-60-TABS_BORDER);
	//type_expr = new ExprTextCtrl(panel,ID_TYPE_EXPR,"",0,TABS_WIDTH-60-TABS_BORDER);

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Top", wxDefaultPosition, wxSize(60,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(top_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	shape_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Bottom", wxDefaultPosition, wxSize(60,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(bottom_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	shape_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(shape_cntrls,0,wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* size_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Sprite Properties"));

	hline = new wxBoxSizer(wxHORIZONTAL);

	NumSpritesSlider=new SliderCtrl(panel,ID_NUM_SPRITES_SLDR,"Number",LABEL1, VALUE,SLIDER2);
	NumSpritesSlider->setRange(1,8);
	NumSpritesSlider->slider->SetToolTip("Number of Sprites per vertex");
    hline->Add(NumSpritesSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    SmaxSlider=new SliderCtrl(panel,ID_SMAX_SLDR,"Scatter",LABEL1, VALUE,SLIDER2);
    SmaxSlider->setRange(0,32);
    SmaxSlider->slider->SetToolTip("Random Radius");
    hline->Add(SmaxSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    size_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

    CminSlider=new SliderCtrl(panel,ID_CMIN_SLDR,"Size",LABEL1, VALUE,SLIDER2);
    CminSlider->setRange(1,128);
    CminSlider->slider->SetToolTip("Minimum Point Size");
    hline->Add(CminSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    CmaxSlider=new SliderCtrl(panel,ID_CMAX_SLDR,"Range",LABEL1, VALUE,SLIDER2);
    CmaxSlider->setRange(1,4);
    CmaxSlider->slider->SetToolTip("Point Size Range");
    hline->Add(CmaxSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

    size_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    hline = new wxBoxSizer(wxHORIZONTAL);

	CrotSlider=new SliderCtrl(panel,ID_CROT_SLDR,"Angle",LABEL1,VALUE,SLIDER2);
	CrotSlider->setRange(0,1,0,360);
	CrotSlider->slider->SetToolTip("Random Rotation Angle");
	//rot_cntrls->Add(CrotSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
	hline->Add(CrotSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

	DiffusionSlider=new SliderCtrl(panel,ID_DIFFUSION_SLDR,"Darken",LABEL1, VALUE,SLIDER2);
	DiffusionSlider->setRange(0,2);
	DiffusionSlider->slider->SetToolTip("Darken with thickness");

	hline->Add(DiffusionSlider->getSizer(),1,wxALIGN_LEFT|wxALL);
	size_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	//color_cntrls->Add(DiffusionSlider->getSizer(),1,wxALIGN_LEFT|wxALL);

	boxSizer->Add(size_cntrls,0,wxALIGN_LEFT|wxALL,0);
	//boxSizer->Add(rot_cntrls,0,wxALIGN_LEFT|wxALL,0);

}

void VtxCloudsTabs::AddImagesTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* image_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Sprites"));
    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *lbl=new wxStaticText(panel,-1,"File",wxDefaultPosition,wxSize(25,-1));
	hline->Add(lbl, 0, wxALIGN_LEFT|wxALL, 8);
	//hline->AddSpacer(5);

    choices=new wxChoice(panel,ID_FILELIST,wxPoint(-1,4),wxSize(130,-1));
    choices->SetSelection(0);

	hline->Add(choices,0,wxALIGN_LEFT|wxALL,2);


	wxString offsets[]={"1x","4x","9x","16x","25x"};

	sprites_dim=new wxChoice(panel, ID_SPRITES_DIM, wxDefaultPosition,wxSize(55,-1),4, offsets);
	sprites_dim->SetSelection(3);
	hline->Add(sprites_dim,0,wxALIGN_LEFT|wxALL,0);

	lbl=new wxStaticText(panel,-1,"Cell",wxDefaultPosition,wxSize(25,-1));
	hline->Add(lbl, 0, wxALIGN_LEFT|wxALL, 8);

	type_expr = new ExprTextCtrl(panel,ID_TYPE_EXPR,"",0,TABS_WIDTH-60-TABS_BORDER);
	hline->Add(type_expr->getSizer(),0,wxALIGN_LEFT|wxALL,2);

	image_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(image_cntrls,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	image_window = new VtxBitmapPanel(panel,wxID_ANY,wxDefaultPosition,wxSize(200,200));
	hline->Add(image_window, 0, wxALIGN_LEFT|wxALL, 0);
	cell_window = new VtxBitmapPanel(panel,wxID_ANY,wxDefaultPosition,wxSize(200,200));
	hline->AddSpacer(10);

	hline->Add(cell_window, 0, wxALIGN_LEFT|wxALL, 0);
	image_sizer=new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Images"));
	image_sizer->Add(hline, 0, wxALIGN_LEFT|wxALL,2);
	boxSizer->Add(image_sizer,0,wxALIGN_LEFT|wxALL,0);
}

void  VtxCloudsTabs::invalidateObject() {
	setObjAttributes();
	object()->invalidate();
	TheView->set_changed_detail();
	TheScene->rebuild();
}
void VtxCloudsTabs::OnChangedTypeExpr(wxCommandEvent& event){
    changed_cell_expr=true;
	invalidateObject();
}

void VtxCloudsTabs::OnChangedExpr(wxCommandEvent& event){
	invalidateObject();
}

void VtxCloudsTabs::OnChangedFile(wxCommandEvent& event){
	invalidateObject();
}

void VtxCloudsTabs::OnDimSelect(wxCommandEvent& event){
	int dim=sprites_dim->GetSelection();
	dim+=1;
	makeFileList(dim,(char*)image_name.ToAscii());
	invalidateObject();
}

void VtxCloudsTabs::OnCloudsDimSelect(wxCommandEvent& event){
	GLuint dim=clouds_dim->GetSelection();
	TNclouds *tnode=object()->getClouds();
	switch(dim){
	case 0: // 2D selected
		if(tnode){
			cout<< "3D->2D"<<endl;
			object()->deleteClouds();
			object()->invalidate();
			TheView->set_changed_detail();

			//TheScene->rebuild_all();
		}
		break;
	case 1: // 3d selected
		if(!tnode){
			cout<< "2D->3D"<<endl;
			wxString str=getCloudsExpr();
			TNclouds *clouds=(TNclouds*)TheScene->parse_node((char*)str.ToAscii());
			if(clouds !=NULL){
				object()->setClouds(clouds);
				TheView->set_changed_detail();
				TheScene->rebuild_all();
				update_needed=true;
				getObjAttributes();
				sceneDialog->replaceSelected(object());
			}
		}
		break;
	}
	updateControls();
}

void VtxCloudsTabs::setTypePanel(){
	if(image_window->imageOk()&&changed_cell_expr){
		GLuint dim=sprites_dim->GetSelection()+1;
		TNclouds *tnode=object()->getClouds();
		double args[6];
		getargs((TNarg*)tnode->left,args,3);
		int cell = (int)args[2];
		int max_cell=dim*dim-1;
		cell=cell>max_cell?max_cell:cell;
		cell=cell<0?0:cell;
		int y=cell/dim;
		int x=cell-y*dim;
		cout<<"cell:"<<cell<<" x:"<<x<<" y:"<<y<<endl;

		wxRect r(x,y,dim,dim);
		//cell_window->setName((char*)image_path.ToAscii());
		cell_window->setSubImage(r);
		cell_window->scaleImage();
		cout<<"setting type panel:"<<cell_window->getName()<<endl;

		changed_cell_expr=false;
	}
}

void VtxCloudsTabs::setImagePanel(){
	char sdir[256];
	object()->getSpritesFilePath(sdir);
	strcat(sdir,".bmp");
	wxString path(sdir);
	if(sdir!=image_path){
		image_window->setScaledImage(sdir);
		cell_window->setName((char*)path.ToAscii());
		cout<<"setting image panel:"<<sdir<<endl;
		changed_cell_expr=true;
		image_path=sdir;
	}
}

void VtxCloudsTabs::makeFileList(int dim,char * name){
	char sdir[256];

	object()->getSpritesDir(dim,sdir);

 	wxDir dir(sdir);
 	if ( !dir.IsOpened() )
 	{
 	    // deal with the error here - wxDir would already log an error message
 	    // explaining the exact reason of the failure
 	    return;
 	}
 	if(dim != image_dim){
		files.Clear();
		wxString filename;
		bool cont = dir.GetFirst(&filename);
		while ( cont ) {
			filename=filename.Before('.');
			files.Add(filename);
			cont = dir.GetNext(&filename);
		}
		files.Sort();
		choices->Clear();
		choices->Append(files);
		choices->SetSelection(0);
		image_dim=dim;
 	}
 	image_name=name;
 	setObjAttributes();
}

//-------------------------------------------------------------
// VtxCloudsTabs::getCloudsExpr() return clouds value string
//-------------------------------------------------------------
wxString VtxCloudsTabs::getCloudsExpr(){
	char top[128]="0.1";
	char bottom[128]="-0.1";
	char type[128]="0";

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

	int is3D=obj->threeD()?1:0;

	clouds_dim->SetSelection(is3D);

	TNclouds *tnode=obj->getClouds();
	if(!is3D || !tnode)
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
	uint dim=0;
	char* sprites_file=obj->getSpritesFile(dim);


	sprites_dim->SetSelection(dim-1);
	makeFileList(dim,sprites_file);
	cout<<"file:"<<sprites_file<<" dim:"<<dim<<endl;
	choices->SetStringSelection(sprites_file);
	setImagePanel();
	setTypePanel();
}

//-------------------------------------------------------------
// VtxCloudsTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxCloudsTabs::setObjAttributes(){
	update_needed=true;
	TNclouds *tnode=object()->getClouds();
	if(!tnode){
		cout << "NO CLOUDS" << endl;
		return; // 2d
	}

	object()->invalidate();


	wxString str=getCloudsExpr();

	cout << str << endl;

	tnode->setExpr((char*)(str.ToAscii()));

	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
		tnode->eval();
        TheView->set_changed_detail();
        TheScene->rebuild();
	}
	str=choices->GetStringSelection();
	GLuint dim=sprites_dim->GetSelection();
	dim+=1;
	if(str!=wxEmptyString){
		object()->setSpritesFile((char*)str.ToAscii(), dim);
		setImagePanel();
		setTypePanel();
	}

}

//-------------------------------------------------------------
// VtxCloudsTabs::updateControls() update UI
//-------------------------------------------------------------
void VtxCloudsTabs::updateControls(){
	CloudLayer *obj=object();
	int pages=GetPageCount();

	if(obj->threeD()){
		if(pages==2){
			InsertPage(2,geometry_page,"Volume",false,-1);
			InsertPage(3,sprites_page,"Images",false,-1);
		}
	}
	else if(pages==4){
		RemovePage(2);
		RemovePage(2);
	}
	if(update_needed){
		getObjAttributes();
	}
}
