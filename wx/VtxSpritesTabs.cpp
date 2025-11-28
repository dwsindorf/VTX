/*
 * VtxSpritesTabs.cpp
 *
 *  Created on: Jul 10, 2023
 *      Author: deans
 */

#include "VtxSpritesTabs.h"
#include "VtxSceneDialog.h"
#include "VtxImageDialog.h"

#include <wx/dir.h>

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_WIDTH BOX_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

#define VALUE1 50
#define LABEL1 50
#undef LABEL2
#define LABEL2 60

//extern VtxSceneDialog *sceneDialog;

static wxString selections[]={"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"};

//########################### VtxSpritesTabs Class ########################

enum{
	ID_ENABLE,
	ID_DELETE,
	ID_SAVE,
    ID_FILELIST,
    ID_SIZE_SLDR,
    ID_SIZE_TEXT,
    ID_DELTA_SIZE_SLDR,
    ID_DELTA_SIZE_TEXT,
    ID_LEVELS,
    ID_LEVELS_SIZE_SLDR,
    ID_LEVELS_SIZE_TEXT,
    ID_LEVELS_DELTA_SLDR,
    ID_LEVELS_DELTA_TEXT,
    ID_DENSITY_SLDR,
    ID_DENSITY_TEXT,

    ID_SLOPE_BIAS_SLDR,
    ID_SLOPE_BIAS_TEXT,
	ID_HARD_BIAS_SLDR,
	ID_HARD_BIAS_TEXT,
    ID_PHI_BIAS_SLDR,
    ID_PHI_BIAS_TEXT,
    ID_HT_BIAS_SLDR,
    ID_HT_BIAS_TEXT,
    ID_SEL_BIAS_SLDR,
    ID_SEL_BIAS_TEXT,
	ID_SPRITES_DIM,
	ID_SPRITES_VIEW,
};

IMPLEMENT_CLASS(VtxSpritesTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxSpritesTabs, VtxTabsMgr)

EVT_MENU(ID_DELETE,VtxSpritesTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxSpritesTabs::OnEnable)
EVT_MENU(ID_SAVE,VtxSpritesTabs::OnSave)
EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxSpritesTabs::OnAddItem)
EVT_CHOICE(ID_SPRITES_DIM,VtxSpritesTabs::OnDimSelect)
EVT_CHOICE(ID_SPRITES_VIEW,VtxSpritesTabs::OnViewSelect)
EVT_CHOICE(ID_FILELIST,VtxSpritesTabs::OnChangedFile)
EVT_CHOICE(ID_LEVELS,VtxSpritesTabs::OnChangedLevels)
SET_SLIDER_EVENTS(SIZE,VtxSpritesTabs,Size)
SET_SLIDER_EVENTS(DELTA_SIZE,VtxSpritesTabs,DeltaSize)
SET_SLIDER_EVENTS(LEVELS_DELTA,VtxSpritesTabs,LevelDelta)
SET_SLIDER_EVENTS(DENSITY,VtxSpritesTabs,Density)
SET_SLIDER_EVENTS(SLOPE_BIAS,VtxSpritesTabs,SlopeBias)
SET_SLIDER_EVENTS(PHI_BIAS,VtxSpritesTabs,PhiBias)
SET_SLIDER_EVENTS(HT_BIAS,VtxSpritesTabs,HtBias)
SET_SLIDER_EVENTS(HARD_BIAS,VtxSpritesTabs,HardBias)
SET_SLIDER_EVENTS(SEL_BIAS,VtxSpritesTabs,SelBias)

END_EVENT_TABLE()

VtxSpritesTabs::VtxSpritesTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxSpritesTabs::Create(wxWindow* parent,
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
    image_center=0;
    image_rows=0;
    image_cols=0;
    image_name="";
    sprites_file="";
    
      
	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	
    AddDistribTab(page);
    AddPage(page,wxT("Distribution"),true);
    
    page=new wxPanel(this,wxID_ANY);

	AddImageTab(page);
    AddPage(page,wxT("Image"),false);
    return true;
}
void VtxSpritesTabs::AddDistribTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);
    
    wxStaticBoxSizer* distro = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Multilevel"));

    distro->Add(new wxStaticText(panel,-1,"Levels",wxDefaultPosition,wxSize(LABEL2,-1)), 0, wxALIGN_LEFT|wxALL, 4);

	wxString orders[]={"1","2","3","4","5","6","7","8","9","10"};
	m_orders=new wxChoice(panel, ID_LEVELS, wxDefaultPosition,wxSize(50,-1),10, orders);
	m_orders->SetSelection(0);

	distro->Add(m_orders, 0, wxALIGN_LEFT|wxALL, 3);

	  // size
	SizeSlider=new SliderCtrl(panel,ID_SIZE_SLDR,"Size",40, VALUE2,60);
	SizeSlider->setRange(1,200);
	SizeSlider->setValue(10.0);

	distro->Add(SizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	DensitySlider=new ExprSliderCtrl(panel,ID_DENSITY_SLDR,"Density",40,VALUE2,60);
	DensitySlider->setRange(0.0,1.0);
	DensitySlider->setValue(1.0);
	distro->Add(DensitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	boxSizer->Add(distro,0,wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* size = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Dispersion"));

	DeltaSizeSlider=new ExprSliderCtrl(panel,ID_DELTA_SIZE_SLDR,"Size",LABEL2,VALUE2,SLIDER2);
	DeltaSizeSlider->setRange(0.0,2);
	DeltaSizeSlider->setValue(0.0);
	size->Add(DeltaSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	LevelDeltaSlider=new ExprSliderCtrl(panel,ID_LEVELS_DELTA_SLDR,"Level",LABEL2,VALUE2,SLIDER2);
	LevelDeltaSlider->setRange(0.0,2);
	LevelDeltaSlider->setValue(0.5);
	size->Add(LevelDeltaSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	size->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT+TABS_BORDER));
	boxSizer->Add(size,0,wxALIGN_LEFT|wxALL,0);
	
	wxStaticBoxSizer* bias = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Density Bias"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);	
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
}
void VtxSpritesTabs::AddImageTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* image_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Sprites"));
    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *lbl=new wxStaticText(panel,-1,"File",wxDefaultPosition,wxSize(25,-1));
	hline->Add(lbl, 0, wxALIGN_LEFT|wxALL, 1);
	//hline->AddSpacer(5);

    m_file_choice=new wxChoice(panel,ID_FILELIST,wxPoint(-1,4),wxSize(130,-1));
    m_file_choice->SetSelection(0);

	hline->Add(m_file_choice,0,wxALIGN_LEFT|wxALL,1);
    //int num_dirs=SpriteMgr::sprite_dirs.size;
	int num_dirs=sprites_mgr.image_dirs.size;
	//cout<<"VtxSpritesTabs num sprites="<<num_dirs<<endl;
 
	wxString offsets[num_dirs];
	for(int i=i;i<num_dirs;i++){
		offsets[i]=sprites_mgr.image_dirs[i]->name();
	}

	m_dim_choice=new wxChoice(panel, ID_SPRITES_DIM, wxDefaultPosition,wxSize(55,-1),num_dirs, offsets);
	m_dim_choice->SetSelection(1);
	hline->Add(m_dim_choice,0,wxALIGN_LEFT|wxALL,0);

	lbl=new wxStaticText(panel,-1,"Center",wxDefaultPosition,wxSize(40,-1));
	hline->Add(lbl, 0, wxALIGN_LEFT|wxALL, 1);

	m_select=new wxChoice(panel, ID_SPRITES_VIEW, wxDefaultPosition,wxSize(40,-1),2, selections);
	m_select->SetSelection(0);

	hline->Add(m_select,0,wxALIGN_LEFT|wxALL,1);
	
	SelBiasSlider=new ExprSliderCtrl(panel,ID_SEL_BIAS_SLDR,"Bias",30,40,SLIDER2);
	SelBiasSlider->setRange(0,1);
	SelBiasSlider->setValue(0.0);

	hline->Add(SelBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,2);
	
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

int VtxSpritesTabs::showMenu(bool expanded){

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

void VtxSpritesTabs::setImagePanel(){
	char sdir[512];
	char *name=(char*)image_name.ToAscii();
	TNsprite *obj=object();
	obj->getImageFilePath(name,sdir);

	strcat(sdir,".png");
	wxString path(sdir);
	if(sdir!=image_path || obj->get_id() != image_center){
		image_window->setScaledImage(sdir,wxBITMAP_TYPE_PNG);
		cell_window->setName((char*)path.ToAscii());
		//cout<<"--setting image panel:"<<sdir<<endl;
		changed_cell_expr=true;
		image_path=sdir;
		image_center=obj->get_id();
	}
}

void VtxSpritesTabs::setViewPanel(){
	if(image_window->imageOk()&&changed_cell_expr){
		GLuint sel=m_dim_choice->GetSelection();
		wxString str=m_dim_choice->GetString(sel);
		uint rows;
		uint cols;
		object()->getImageDims((char*)str.ToAscii(),cols,rows);
		int cell=m_select->GetCurrentSelection();
		int y=cell/cols;
		int x=cell-y*rows;
		cout<<str<<" cols:"<<cols<<" rows:"<<rows<<" cell:"<<cell<<" x:"<<x<<" y:"<<y<<endl;

		wxRect r(x,y,cols,rows);
		cell_window->setSubImage(r,wxBITMAP_TYPE_PNG);
		cell_window->scaleImage();

		changed_cell_expr=false;
	}
}
void VtxSpritesTabs::OnDimSelect(wxCommandEvent& event){
	int dim=m_dim_choice->GetSelection();
	wxString str=m_dim_choice->GetString(dim);
	object()->getImageDims((char*)str.ToAscii(),image_cols,image_rows);
	
	int n=image_rows*image_cols;
	makeFileList(str,"");
	m_select->Set(n,selections);
	update_needed=true;
	m_select->SetSelection(0);
	setObjAttributes();
}

void VtxSpritesTabs::makeFileList(wxString wdir,wxString name){
	char sdir[512];
	char *wstr=(char*)wdir.ToAscii();
	object()->getImageDirPath(wstr,sdir);

 	wxDir dir(sdir);
 	if ( !dir.IsOpened() )
 	{
 	    // deal with the error here - wxDir would already log an error message
 	    // explaining the exact reason of the failure
 	    return;
 	}
 	image_name=name;
 	uint rows=0;
 	uint cols=0;
 	int dim=m_dim_choice->GetSelection();
 	wxString str=m_dim_choice->GetString(dim);
 	object()->getImageDims((char*)str.ToAscii(),cols,rows);
 	
 	if(dim!=image_dim ||rows != image_rows || cols!=image_cols){
		files.Clear();
		wxString filename;
		bool cont = dir.GetFirst(&filename);
		while ( cont ) {
			filename=filename.Before('.');
			files.Add(filename);
			cont = dir.GetNext(&filename);
		}
		files.Sort();
		m_file_choice->Clear();
		m_file_choice->Append(files);
		image_dim=dim;
		image_rows=rows;
		image_cols=cols;
		int n=image_rows*image_cols;
		m_select->Set(n,selections);
		if(image_name.IsEmpty())
			m_file_choice->SetSelection(0);
 	}
 	if(image_name.IsEmpty())
 		image_name=m_file_choice->GetStringSelection();
 	sprites_file=image_name;
 	m_file_choice->SetStringSelection(sprites_file);
}

void VtxSpritesTabs::OnChangedLevels(wxCommandEvent& event){
	setObjAttributes();
}

wxString VtxSpritesTabs::exprString(){
	char p[1024]="";
	TNsprite *obj=object();
	int sel=m_select->GetSelection();

	sprintf(p,"Sprite(\"%s\",ID%d|FLIP|NOLOD,",(const char*)sprites_file.ToAscii(),sel);
	sprintf(p+strlen(p),"%d,",m_orders->GetSelection()+1);
	sprintf(p+strlen(p),"%g,",FEET*SizeSlider->getValue()/obj->radius);
	wxString s(p);

	s+=DeltaSizeSlider->getText()+",";
	s+=LevelDeltaSlider->getText()+",";
	s+=DensitySlider->getText()+",";
	s+=SlopeBiasSlider->getText()+",";
	s+=HtBiasSlider->getText()+",";
	s+=PhiBiasSlider->getText()+",";
	s+=HardBiasSlider->getText()+",";
	s+=SelBiasSlider->getText();
	s+=")";
 	return wxString(s);
}

void VtxSpritesTabs::getObjAttributes(){
	if(!update_needed)
		return;
	update_needed=false;

	TNsprite *obj=object();
	SpriteMgr *mgr=(SpriteMgr*)obj->mgr;

	obj->getImageDims(image_cols,image_rows);
	sprites_file=obj->getImageFile();
	sprites_dir=obj->getImageDir();
	
	int ns=m_dim_choice->FindString(sprites_dir, false);
	cout<<ns<<" "<<sprites_dir<<"/"<<sprites_file<<" "<<image_cols<<"x"<<image_rows<<endl;
	m_dim_choice->SetSelection(ns);
	
	makeFileList(sprites_dir,sprites_file);
	
	int id=obj->get_id();

	m_select->SetSelection(id);
	
	m_file_choice->SetStringSelection(sprites_file);
	setImagePanel();
	setViewPanel();
	
	m_orders->SetSelection(mgr->levels-1);
	double maxsize=mgr->maxsize;
	SizeSlider->setValue(mgr->maxsize*obj->radius/FEET);

	TNarg &args=*((TNarg *)obj->left);
	TNode *a=args[2];

	if(a)
		DeltaSizeSlider->setValue(a);
	else
		DeltaSizeSlider->setValue(mgr->mult);

	a=args[3];
	if(a)
		LevelDeltaSlider->setValue(a);
	else
		LevelDeltaSlider->setValue(mgr->level_mult);
	
	a=args[4];
	if(a)
		DensitySlider->setValue(a);
	else
		DensitySlider->setValue(obj->maxdensity);

	a=args[5];
	if(a)
		SlopeBiasSlider->setValue(a);
	else
		SlopeBiasSlider->setValue(mgr->slope_bias);

	a=args[6];
	if(a)
		HtBiasSlider->setValue(a);
	else
		HtBiasSlider->setValue(mgr->ht_bias);

	a=args[7];
	if(a)
		PhiBiasSlider->setValue(a);
	else
		PhiBiasSlider->setValue(mgr->lat_bias);

	a=args[8];
	if(a)
		HardBiasSlider->setValue(a);
	else
		HardBiasSlider->setValue(mgr->hardness_bias);

	a=args[9];
	if(a)
		SelBiasSlider->setValue(a);
	else
		SelBiasSlider->setValue(mgr->select_bias);

	wxString s=exprString();
}

void VtxSpritesTabs::setObjAttributes(){
	wxString str=m_file_choice->GetStringSelection();
	if(str!=wxEmptyString){
		object()->setSpritesImage((char*)str.ToAscii());
		setImagePanel();
		setViewPanel();
	}
	TNsprite *obj=object();
	SpriteMgr *smgr=(SpriteMgr*)obj->mgr;

	wxString s=exprString();

	obj->setExpr((char*)s.ToAscii());
	obj->applyExpr();
	TheView->set_changed_detail();
	TheScene->rebuild();
	sceneDialog->setNodeName((char*)image_name.ToAscii());

	update_needed=false;
}

void VtxSpritesTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

void VtxSpritesTabs::OnChangedFile(wxCommandEvent& event){
	changed_cell_expr=true;
	image_path="";
	image_name=m_file_choice->GetStringSelection();
	invalidateObject();
		
}
void VtxSpritesTabs::OnViewSelect(wxCommandEvent& event){
	changed_cell_expr=true;
	setViewPanel();
	invalidateRender();
}

