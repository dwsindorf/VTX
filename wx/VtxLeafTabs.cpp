
#include "VtxLeafTabs.h"
#include "VtxSceneDialog.h"
#include "AdaptOptions.h"
#include "FileUtil.h"
#include <TerrainClass.h>


#include <wx/filefn.h>
#include <wx/dir.h>

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_WIDTH BOX_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30

#define VALUE1 50
#define LABEL1 30
#undef LABEL2
#define LABEL2 60

static wxString selections[]={"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"};

//########################### VtxLeafTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
	ID_SAVE,
	ID_TEX_ENABLE,
	ID_SHAPE_ENABLE,
	ID_SHADOW_ENABLE,
	ID_COL_ENABLE,
	ID_NAME_TEXT,
    ID_SEGS,
    ID_SECS,
    ID_CENTER,
    ID_DENSITY_SLDR,
    ID_DENSITY_TEXT,
    ID_LENGTH_SLDR,
    ID_LENGTH_TEXT,
    ID_WIDTH_SLDR,
    ID_WIDTH_TEXT,
    ID_WIDTH_TAPER_SLDR,
    ID_WIDTH_TAPER_TEXT,
    ID_LENGTH_TAPER_SLDR,
    ID_LENGTH_TAPER_TEXT,
    ID_DIVERGENCE_SLDR,
    ID_DIVERGENCE_TEXT,
    ID_FLATNESS_SLDR,
    ID_FLATNESS_TEXT,
    ID_RAND_SLDR,
    ID_RAND_TEXT,
	ID_BIAS_SLDR,
	ID_BIAS_TEXT,
	ID_FILELIST,
	ID_DIMLIST,

    ID_RED,
    ID_GREEN,
    ID_BLUE,
    ID_ALPHA,
    ID_COL,
    ID_REVERT

};

#define NAME_WIDTH  50
#define SLDR_WIDTH  180
#define EXPR_WIDTH  200

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 40

//########################### VtxLeafTabs Class ########################

IMPLEMENT_CLASS(VtxLeafTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxLeafTabs, VtxTabsMgr)
EVT_TEXT_ENTER(ID_NAME_TEXT,VtxLeafTabs::OnNameText)


EVT_MENU(ID_DELETE,VtxLeafTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxLeafTabs::OnEnable)
EVT_MENU(ID_SAVE,VtxLeafTabs::OnSave)
EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxLeafTabs::OnAddItem)

EVT_CHECKBOX(ID_TEX_ENABLE,VtxLeafTabs::OnChanged)
EVT_CHECKBOX(ID_COL_ENABLE,VtxLeafTabs::OnChanged)
EVT_CHECKBOX(ID_SHAPE_ENABLE,VtxLeafTabs::OnChanged)
EVT_CHECKBOX(ID_SHADOW_ENABLE,VtxLeafTabs::OnChanged)
EVT_CHOICE(ID_FILELIST,VtxLeafTabs::OnChangedFile)
EVT_CHOICE(ID_DIMLIST,VtxLeafTabs::OnDimSelect)

EVT_CHOICE(ID_SEGS,VtxLeafTabs::OnChanged)
EVT_CHOICE(ID_SECS,VtxLeafTabs::OnChanged)

SET_SLIDER_EVENTS(DENSITY,VtxLeafTabs,Density)
SET_SLIDER_EVENTS(LENGTH,VtxLeafTabs,Length)
SET_SLIDER_EVENTS(WIDTH,VtxLeafTabs,Width)
SET_SLIDER_EVENTS(RAND,VtxLeafTabs,Rand)
SET_SLIDER_EVENTS(BIAS,VtxLeafTabs,Bias)
SET_SLIDER_EVENTS(WIDTH_TAPER,VtxLeafTabs,WidthTaper)
SET_SLIDER_EVENTS(LENGTH_TAPER,VtxLeafTabs,LengthTaper)
SET_SLIDER_EVENTS(DIVERGENCE,VtxLeafTabs,Divergence)
SET_SLIDER_EVENTS(FLATNESS,VtxLeafTabs,Flatness)

EVT_TEXT_ENTER(ID_RED,VtxLeafTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_GREEN,VtxLeafTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_BLUE,VtxLeafTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_ALPHA,VtxLeafTabs::OnChangedExpr)
EVT_COLOURPICKER_CHANGED(ID_COL,VtxLeafTabs::OnChangedColor)
EVT_BUTTON(ID_REVERT,VtxLeafTabs::OnRevert)

SET_FILE_EVENTS(VtxLeafTabs)

END_EVENT_TABLE()


VtxLeafTabs::VtxLeafTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxLeafTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
    image_path="";
    image_dim=0;
    image_center=0;
    image_rows=0;
    image_cols=0;
    image_name="";

 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
 	AddPropertiesTab(page);
    AddPage(page,wxT("Properties"),true);
    page=new wxPanel(this,wxID_ANY);
	AddImageTab(page);
    AddPage(page,wxT("Texture"),false);
    page=new wxPanel(this,wxID_ANY);
	AddColorTab(page);
    AddPage(page,wxT("Color"),false);

    return true;
}

int VtxLeafTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));

	wxMenu *addmenu=sceneDialog->getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}

	sceneDialog->AddFileMenu(menu,object_node->node);

	PopupMenu(&menu);
	return menu_action;
}

void VtxLeafTabs::AddPropertiesTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);
 
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",40,80);

	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    hline->Add(new wxStaticText(panel,-1,"Cluster",wxDefaultPosition,wxSize(40,-1)), 0, wxALIGN_LEFT|wxALL, 5);

	wxString segs[]={"1","2","3","4","5","6","7","8","9","10"};
	m_segs=new wxChoice(panel, ID_SEGS, wxDefaultPosition,wxSize(40,-1),10, segs);
	m_segs->SetSelection(0);

	hline->Add(m_segs, 0, wxALIGN_LEFT|wxALL, 5);

	hline->Add(new wxStaticText(panel,-1,"X",wxDefaultPosition,wxSize(6,-1)), 0, wxALIGN_LEFT|wxALL, 4);

	m_secs=new wxChoice(panel, ID_SECS, wxDefaultPosition,wxSize(40,-1),10, segs);
	m_secs->SetSelection(0);

	hline->Add(m_secs, 0, wxALIGN_LEFT|wxALL, 5);
	
	m_shape_enable = new wxCheckBox(panel, ID_SHAPE_ENABLE, "Shape");
	m_shape_enable->SetValue(true);
	hline->Add(m_shape_enable, 0, wxALIGN_LEFT | wxALL, 7);

	m_shadow_enable = new wxCheckBox(panel, ID_SHADOW_ENABLE, "Shadow");
	m_shadow_enable->SetValue(true);
	hline->Add(m_shadow_enable, 0, wxALIGN_LEFT | wxALL, 7);
	
	hline->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

    // shape
    
    wxBoxSizer *shape = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Shape"));
    shape->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
    
	hline = new wxBoxSizer(wxHORIZONTAL);

	LengthSlider=new ExprSliderCtrl(panel,ID_LENGTH_SLDR,"Length",LABEL2, VALUE2,SLIDER2);
	LengthSlider->setRange(1,50);
	LengthSlider->setValue(1);
	hline->Add(LengthSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	LengthTaperSlider=new ExprSliderCtrl(panel,ID_LENGTH_TAPER_SLDR,"Taper",LABEL2,VALUE2,SLIDER2);
	LengthTaperSlider->setRange(0.0,1);
	LengthTaperSlider->setValue(0.5);
	hline->Add(LengthTaperSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	shape->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	
	hline = new wxBoxSizer(wxHORIZONTAL);
	
	WidthSlider=new ExprSliderCtrl(panel,ID_WIDTH_SLDR,"Width",LABEL2, VALUE2,SLIDER2);
	WidthSlider->setRange(0.2,5);
	WidthSlider->setValue(1);
	
	hline->Add(WidthSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
		
	WidthTaperSlider=new ExprSliderCtrl(panel,ID_WIDTH_TAPER_SLDR,"Taper",LABEL2,VALUE2,SLIDER2);
	WidthTaperSlider->setRange(0.1,1);
	WidthTaperSlider->setValue(0.8);

	hline->Add(WidthTaperSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	shape->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	
	boxSizer->Add(shape,0,wxALIGN_LEFT|wxALL,0);
	
	// randomness
	
	wxBoxSizer *variability = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Variability"));
	variability->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	    
    hline = new wxBoxSizer(wxHORIZONTAL);

	DensitySlider=new ExprSliderCtrl(panel,ID_DENSITY_SLDR,"Density",LABEL2, VALUE2,SLIDER2);
	DensitySlider->setRange(0,1);
	DensitySlider->setValue(1);
	
	hline->Add(DensitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	RandSlider=new ExprSliderCtrl(panel,ID_RAND_SLDR,"Random",LABEL2, VALUE2,SLIDER2);
	RandSlider->setRange(0,1);
	RandSlider->setValue(1);
	hline->Add(RandSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	variability->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	
	boxSizer->Add(variability,0,wxALIGN_LEFT|wxALL,0);
	
	// orientation
	
	wxBoxSizer *orientation = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Orientation"));
	orientation->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	
	hline = new wxBoxSizer(wxHORIZONTAL);

	DivergenceSlider=new ExprSliderCtrl(panel,ID_DIVERGENCE_SLDR,"Divergence",LABEL2,VALUE2,SLIDER2);
	DivergenceSlider->setRange(0.0,1);
	DivergenceSlider->setValue(1);
	hline->Add(DivergenceSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	FlatnessSlider=new ExprSliderCtrl(panel,ID_FLATNESS_SLDR,"Flatness",LABEL2,VALUE2,SLIDER2);
	FlatnessSlider->setRange(0.0,1);
	FlatnessSlider->setValue(1);
	hline->Add(FlatnessSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	orientation->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(orientation,0,wxALIGN_LEFT|wxALL,0);
}

void VtxLeafTabs::AddImageTab(wxWindow *panel){
	
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* image_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Image"));

	wxStaticText *lbl=new wxStaticText(panel,-1,"File",wxDefaultPosition,wxSize(25,-1));
	image_cntrls->Add(lbl, 0, wxALIGN_LEFT|wxALL,2);
	//hline->AddSpacer(5);

    m_file_choice=new wxChoice(panel,ID_FILELIST,wxPoint(-1,4),wxSize(110,-1));
    m_file_choice->SetSelection(0);
    image_cntrls->Add(m_file_choice,0,wxALIGN_LEFT|wxALL,1);
    
    int num_dirs=leaf_mgr.image_dirs.size;
    
	wxString offsets[num_dirs];
	for(int i=i;i<num_dirs;i++){
		offsets[i]=leaf_mgr.image_dirs[i]->name();
	}

    m_dim_choice=new wxChoice(panel, ID_DIMLIST, wxDefaultPosition,wxSize(55,-1),num_dirs, offsets);
    m_dim_choice->SetSelection(0);
    image_cntrls->Add(m_dim_choice,0,wxALIGN_LEFT|wxALL,1);
 
    m_tex_enable=new wxCheckBox(panel, ID_TEX_ENABLE, "Enable");
    m_tex_enable->SetValue(true);
    image_cntrls->Add(m_tex_enable,0,wxALIGN_LEFT|wxALL,4);
    
	BiasSlider=new ExprSliderCtrl(panel,ID_BIAS_SLDR,"Center",40, VALUE2,SLIDER2);
	BiasSlider->setRange(0,1);
	BiasSlider->setValue(0.5);
	image_cntrls->Add(BiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,1);
    boxSizer->Add(image_cntrls,0,wxALIGN_LEFT|wxALL,0);
    
    image_sizer=new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Preview"));
	image_window = new VtxBitmapPanel(panel,wxID_ANY,wxDefaultPosition,wxSize(400,400));
	image_sizer->Add(image_window, 0, wxALIGN_LEFT|wxALL,2);

	boxSizer->Add(image_sizer,0,wxALIGN_LEFT|wxALL,0);

}
void VtxLeafTabs::AddColorTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	m_r_expr = new ExprTextCtrl(panel,ID_RED,"",0,TABS_WIDTH-60-TABS_BORDER);
	m_g_expr = new ExprTextCtrl(panel,ID_GREEN,"",0,TABS_WIDTH-60-TABS_BORDER);
	m_b_expr = new ExprTextCtrl(panel,ID_BLUE,"",0,TABS_WIDTH-60-TABS_BORDER);
	m_a_expr = new ExprTextCtrl(panel,ID_ALPHA,"",0,TABS_WIDTH-60-TABS_BORDER);

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Red", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_r_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Green", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_g_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Blue", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_b_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Alpha", wxDefaultPosition, wxSize(50,-1)),0,wxALIGN_LEFT|wxALL,0);
	hline->Add(m_a_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	wxColor col("RGB(255,255,255)");
	//wxStaticText *label=new wxStaticText(panel,-1,"Auto" ,wxDefaultPosition,wxSize(50,-1));
	//hline->Add(label,0,wxALIGN_LEFT|wxALL,0);
	//m_auto_check=new wxCheckBox(panel, ID_AUTO, "");
	//hline->Add(m_auto_check,0,wxALIGN_LEFT|wxALL,4);
	m_color_chooser=new wxColourPickerCtrl(panel,ID_COL,col,wxDefaultPosition, wxSize(50,-1));
	hline->Add(m_color_chooser,0,wxALIGN_LEFT|wxALL,4);
	m_revert=new wxButton(panel,ID_REVERT,"Revert",wxDefaultPosition, wxSize(50,-1));
	hline->Add(m_revert,0,wxALIGN_LEFT|wxALL,4);
    m_col_enable=new wxCheckBox(panel, ID_COL_ENABLE, "Enable");
    m_col_enable->SetValue(true);
	hline->Add(m_col_enable,0,wxALIGN_LEFT|wxALL,4);

	boxSizer->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	//revert_needed=false;
	//m_revert->Enable(revert_needed);

}

void VtxLeafTabs::OnDimSelect(wxCommandEvent& event){
	int dim=m_dim_choice->GetSelection();
	wxString str=m_dim_choice->GetString(dim);
	object()->getImageDims((char*)str.ToAscii(),image_cols,image_rows);
	
	int n=image_rows*image_cols;
	makeFileList(str,"");
	update_needed=true;
	setObjAttributes();
}

void VtxLeafTabs::makeFileList(wxString wdir,wxString name){
	char sdir[512];
	char *wstr=(char*)wdir.ToAscii();
	object()->getImageDirPath(wstr,sdir);

 	wxDir dir(sdir);
 	if ( !dir.IsOpened() )
 	{
 	    // deal with the error here - wxDir would already log an error message
 	    // explaining the exact reason of the failure
 		cout<<"makeFileList error"<<sdir<<endl;
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
		if(image_name.IsEmpty())
			m_file_choice->SetSelection(0);
 	}
 	if(image_name.IsEmpty())
 		image_name=m_file_choice->GetStringSelection();
 	m_file_choice->SetStringSelection(image_name);

}

void VtxLeafTabs::OnChangedFile(wxCommandEvent& event){
	setObjAttributes();	
}
void VtxLeafTabs::setImagePanel(){
	char dir[512];
	char path[512];
	char sdir[512]={0};

	object()->getImageFilePath((char*)image_name.ToAscii(),dir);

	sprintf(path,"%s.jpg",dir);
	if(FileUtil::fileExists(path)){
		strcpy(sdir,path);
		image_window->setScaledImage(sdir,wxBITMAP_TYPE_JPEG);
	}
	else{
		sprintf(path,"%s.bmp",dir);
		if(FileUtil::fileExists(path)){
			strcpy(sdir,path);	
			image_window->setScaledImage(sdir,wxBITMAP_TYPE_BMP);
		}
		else{
			sprintf(path,"%s.png",dir);
			if(FileUtil::fileExists(path)){
				strcpy(sdir,path);	
				image_window->setScaledImage(sdir,wxBITMAP_TYPE_PNG);
			}
		}
	}
	if(strlen(sdir)==0)
		return;
	wxString ipath(sdir);
	if(ipath!=image_path){
		image_path=ipath;
	}
}

void VtxLeafTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

void VtxLeafTabs::OnChangedLevels(wxCommandEvent& event){
	setObjAttributes();
}

wxString VtxLeafTabs::exprString(){
	char p[1024]="";
	TNLeaf *obj=object();

	sprintf(p,"Leaf(");
	if(strlen(obj->name_str)){
		sprintf(p+strlen(p),"\"%s\",",obj->name_str);
	}

	wxString s(p);

 	obj->setTexEnabled((bool)m_tex_enable->GetValue());
	obj->setColEnabled((bool)m_col_enable->GetValue());
	obj->setShapeEnabled((bool)m_shape_enable->GetValue());
	obj->setShadowEnabled((bool)m_shadow_enable->GetValue());
	int enables=obj->enables;

	char tmp[32];
	int cnt = m_segs->GetSelection();
	sprintf(tmp,"%d",cnt+1);
	s+=wxString(tmp)+",";
	s+=DensitySlider->getText()+",";
	s+=LengthSlider->getText()+",";
	s+=WidthSlider->getText()+",";
	s+=RandSlider->getText()+",";
	s+=DivergenceSlider->getText()+","; // leaf angle from stem vector
	s+=FlatnessSlider->getText()+",";   // leaf orientation to eye
	s+=WidthTaperSlider->getText()+","; // width_taper
	s+=LengthTaperSlider->getText()+",";
	cnt = m_secs->GetSelection();  // first bias
		sprintf(tmp,"%d,",cnt);
	s+=wxString(tmp);
	s+="0,0,"; // minlvl, offset
	s+=BiasSlider->getText()+",";
	sprintf(tmp,"%d",enables);
	s+=wxString(tmp);
	s+=")";
 	return wxString(s);
}

//-------------------------------------------------------------
// VtxLeafTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxLeafTabs::setObjAttributes(){
	TNLeaf *obj=object();

	wxString s=exprString();

	wxString str=m_file_choice->GetStringSelection();
	image_name=str;
	obj->setPlantImage((char*)image_name.ToAscii());
	setImagePanel();

	wxString expr=getColorExpr();
	char *cstr=(char*)expr.ToAscii();
	
    obj->setColorExpr(cstr);
    obj->setExpr((char*)s.ToAscii());
	obj->applyExpr();
	if(strlen(obj->name_str))
		sceneDialog->setNodeName(obj->name_str);

	obj->getArgs();
	obj->initArgs();

	TheView->set_changed_detail();
	TheScene->rebuild();

	update_needed=false;
}
//-------------------------------------------------------------
// VtxLeafTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxLeafTabs::getObjAttributes(){
	if(!update_needed)
		return;
	update_needed=false;

	TNLeaf *obj=object();
	obj->initArgs();
	
	TNarg &args=*((TNarg *)obj->left);

	object_name->SetValue(obj->nodeName());
	image_name=obj->getImageFile();
	image_dir=obj->getImageDir();
	
	m_dim_choice->SetStringSelection(image_dir);

	
	m_segs->SetSelection(obj->max_level-1);

    if(args[1])
	   DensitySlider->setValue(args[1]);
	else
	   DensitySlider->setValue(obj->max_splits);
    if(args[2])
    	LengthSlider->setValue(args[2]);
	else
		LengthSlider->setValue(obj->length);
    if(args[3])
    	WidthSlider->setValue(args[3]);
 	else
 		WidthSlider->setValue(obj->width);
    if(args[4])
    	RandSlider->setValue(args[4]);
 	else
 		RandSlider->setValue(obj->randomness);
    if(args[5])
    	DivergenceSlider->setValue(args[5]);
 	else
 		DivergenceSlider->setValue(obj->divergence);
    if(args[6])
    	FlatnessSlider->setValue(args[6]);
  	else
  		FlatnessSlider->setValue(obj->flatness);
    if(args[7])
    	WidthTaperSlider->setValue(args[7]);
  	else
  		WidthTaperSlider->setValue(obj->width_taper);
    if(args[8])
    	LengthTaperSlider->setValue(args[8]);
  	else
  		LengthTaperSlider->setValue(obj->length_taper);
	if(args[12])
		BiasSlider->setValue(args[12]);
	else
		BiasSlider->setValue(obj->bias);

	//BiasSlider->setValue(obj->bias);
	m_secs->SetSelection(obj->first_bias);

	makeFileList(image_dir,image_name);
	setImagePanel();
	
	TNcolor *tnode=obj->getColor();

	char red[128]={0};
	char green[128]={0};
	char blue[128]={0};
	char alpha[128]={0};
	if(tnode){
		TNarg &args=*((TNarg *)tnode->right);
		args[0]->valueString(red);
		args[1]->valueString(green);
		args[2]->valueString(blue);
		if(args[3])
			args[3]->valueString(alpha);
		else
			strcpy(alpha,"0.0");
	}
	else{
		strcpy(red,"1.0");
		strcpy(green,"1.0");
		strcpy(blue,"1.0");
		strcpy(alpha,"0.0");
	}
	m_col_enable->SetValue(obj->isColEnabled());
	m_tex_enable->SetValue(obj->isTexEnabled());
	m_shape_enable->SetValue(obj->isShapeEnabled());
	m_shadow_enable->SetValue(obj->isShadowEnabled());

	m_r_expr->SetValue(red);
	m_g_expr->SetValue(green);
	m_b_expr->SetValue(blue);
	m_a_expr->SetValue(alpha);

	setColorFromExpr();
	saveLastExpr();
    
	update_needed=false;
}

wxString VtxLeafTabs::getColorExpr(){
	char cstr[MAXSTR]={0};
	char red[MAXSTR]="0.0";
	char green[MAXSTR]="0.0";
	char blue[MAXSTR]="0.0";
	char alpha[MAXSTR]="1.0";
	if(strlen(m_r_expr->GetValue().ToAscii()))
		strcpy(red,m_r_expr->GetValue().ToAscii());
	if(strlen(m_g_expr->GetValue().ToAscii()))
		strcpy(green,m_g_expr->GetValue().ToAscii());
	if(strlen(m_b_expr->GetValue().ToAscii()))
		strcpy(blue,m_b_expr->GetValue().ToAscii());
	if(strlen(m_a_expr->GetValue().ToAscii()))
		strcpy(alpha,m_a_expr->GetValue().ToAscii());
	sprintf(cstr,"Color(%s,%s,%s,%s)",red,green,blue,alpha);
    return wxString(cstr);
}

TNcolor* VtxLeafTabs::getColorFromExpr(){
	wxString expr=getColorExpr();
	TNcolor *tnode=(TNcolor*)TheScene->parse_node((char *)expr.ToAscii());
	return tnode;
}
void VtxLeafTabs::setColorFromExpr(){
	TNcolor *tnode=getColorFromExpr();
	if(tnode){
		TNarg &args=*((TNarg *)tnode->right);
		args[0]->eval();
		bool cflag=S0.constant()?true:false;
		unsigned char r=(unsigned char)(S0.s*255);
		args[1]->eval();
		cflag=S0.constant()?cflag:false;
		unsigned char g=(unsigned char)(S0.s*255);
		args[2]->eval();
		cflag=S0.constant()?cflag:false;
		unsigned char b=(unsigned char)(S0.s*255);
		if(args[3]){
			args[3]->eval();
			cflag=S0.constant()?cflag:false;
		}
		wxColor col(r,g,b);
		m_color_chooser->SetColour(col);
		delete tnode;
	}
}

void VtxLeafTabs::restoreLastExpr(){
	char cstr[MAXSTR];
	strcpy(cstr,m_last_expr.ToAscii());
	if(strlen(cstr)){
		TNcolor *tnode=(TNcolor*)TheScene->parse_node(cstr);
		if(tnode){
			char red[128]={0};
			char green[128]={0};
			char blue[128]={0};
			char alpha[128]={0};
			TNarg &args=*((TNarg *)tnode->right);
			args[0]->valueString(red);
			args[1]->valueString(green);
			args[2]->valueString(blue);
			if(args[3])
				args[3]->valueString(alpha);
			else
				strcpy(alpha,"1.0");
			m_r_expr->SetValue(red);
			m_g_expr->SetValue(green);
			m_b_expr->SetValue(blue);
			m_a_expr->SetValue(alpha);
			delete tnode;
		}
	}
	setObjAttributes();	
}
void VtxLeafTabs::setExprFromColor(){
	char red[128]={0};
	char green[128]={0};
	char blue[128]={0};
	wxColor col=m_color_chooser->GetColour();
	Color c;
	c.set_rb(col.Red());
	c.set_gb(col.Green());
	c.set_bb(col.Blue());
	sprintf(red,"%g",c.red());
	sprintf(green,"%g",c.green());
	sprintf(blue,"%g",c.blue());
	m_r_expr->SetValue(red);
	m_g_expr->SetValue(green);
	m_b_expr->SetValue(blue);
	//m_a_expr->SetValue("1.0");
}

void VtxLeafTabs::OnRevert(wxCommandEvent& event){
	restoreLastExpr();
	setColorFromExpr();
	setObjAttributes();
}
void VtxLeafTabs::saveLastExpr(){
    m_last_expr=getColorExpr();
}
void VtxLeafTabs::OnChangedExpr(wxCommandEvent& event){
	setColorFromExpr();
	setObjAttributes();
}
void VtxLeafTabs::OnChangedColor(wxColourPickerEvent& event){
	setExprFromColor();
	setObjAttributes();
}

