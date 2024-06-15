
#include "VtxBranchTabs.h"
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


//########################### VtxBranchTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
	ID_SAVE,
	ID_TEX_ENABLE,
	ID_COL_ENABLE,
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
	ID_FILELIST,
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

//########################### VtxBranchTabs Class ########################

IMPLEMENT_CLASS(VtxBranchTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxBranchTabs, VtxTabsMgr)
EVT_TEXT_ENTER(ID_NAME_TEXT,VtxBranchTabs::OnNameText)

EVT_MENU(ID_DELETE,VtxBranchTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxBranchTabs::OnEnable)
EVT_MENU(ID_SAVE,VtxBranchTabs::OnSave)
EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxBranchTabs::OnAddItem)

EVT_CHECKBOX(ID_FROM_END,VtxBranchTabs::OnChangedLevels)
EVT_CHECKBOX(ID_TEX_ENABLE,VtxBranchTabs::OnChanged)
EVT_CHECKBOX(ID_COL_ENABLE,VtxBranchTabs::OnChanged)

EVT_CHOICE(ID_MIN_LEVEL,VtxBranchTabs::OnChangedLevels)
EVT_CHOICE(ID_MAX_LEVEL,VtxBranchTabs::OnChangedLevels)
EVT_CHOICE(ID_FILELIST,VtxBranchTabs::OnChangedFile)

SET_SLIDER_EVENTS(SPLITS,VtxBranchTabs,Splits)
SET_SLIDER_EVENTS(LENGTH,VtxBranchTabs,Length)
SET_SLIDER_EVENTS(WIDTH,VtxBranchTabs,Width)
SET_SLIDER_EVENTS(RAND,VtxBranchTabs,Rand)
SET_SLIDER_EVENTS(DIVERGENCE,VtxBranchTabs,Divergence)
SET_SLIDER_EVENTS(FLATNESS,VtxBranchTabs,Flatness)
SET_SLIDER_EVENTS(WIDTH_TAPER,VtxBranchTabs,WidthTaper)
SET_SLIDER_EVENTS(LENGTH_TAPER,VtxBranchTabs,LengthTaper)
SET_SLIDER_EVENTS(FIRST_BIAS,VtxBranchTabs,FirstBias)

EVT_TEXT_ENTER(ID_RED,VtxBranchTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_GREEN,VtxBranchTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_BLUE,VtxBranchTabs::OnChangedExpr)
EVT_TEXT_ENTER(ID_ALPHA,VtxBranchTabs::OnChangedExpr)
EVT_COLOURPICKER_CHANGED(ID_COL,VtxBranchTabs::OnChangedColor)
EVT_BUTTON(ID_REVERT,VtxBranchTabs::OnRevert)

SET_FILE_EVENTS(VtxBranchTabs)

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
    page=new wxPanel(this,wxID_ANY);
	AddImageTab(page);
    AddPage(page,wxT("Texture"),false);
    page=new wxPanel(this,wxID_ANY);
	AddColorTab(page);
    AddPage(page,wxT("Color"),false);

    return true;
}

int VtxBranchTabs::showMenu(bool expanded){

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

void VtxBranchTabs::AddPropertiesTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);
 
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2+10,VALUE2+SLIDER2);

	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
    // levels
    
    wxBoxSizer *level = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Level"));
	level->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

 	wxString levels[]={"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"};
 	
    level->Add(new wxStaticText(panel,-1,"Max",wxDefaultPosition,wxSize(LABEL1,-1)), 0, wxALIGN_LEFT|wxALL, 4);

    m_max_level=new wxChoice(panel, ID_MAX_LEVEL, wxDefaultPosition,wxSize(50,-1),16, levels);
    m_max_level->SetSelection(1);
    
    level->Add(m_max_level, 0, wxALIGN_LEFT|wxALL, 3);

    level->Add(new wxStaticText(panel,-1,"Min",wxDefaultPosition,wxSize(LABEL1,-1)), 0, wxALIGN_LEFT|wxALL, 4);

    m_min_level=new wxChoice(panel, ID_MIN_LEVEL, wxDefaultPosition,wxSize(50,-1),16, levels);
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
	size->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	hline = new wxBoxSizer(wxHORIZONTAL);

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
	other->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

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

void VtxBranchTabs::AddImageTab(wxWindow *panel){
	
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* image_cntrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Image"));

	wxStaticText *lbl=new wxStaticText(panel,-1,"File",wxDefaultPosition,wxSize(25,-1));
	image_cntrls->Add(lbl, 0, wxALIGN_LEFT|wxALL, 1);
	//hline->AddSpacer(5);

    choices=new wxChoice(panel,ID_FILELIST,wxPoint(-1,4),wxSize(130,-1));
    choices->SetSelection(0);

    image_cntrls->Add(choices,0,wxALIGN_LEFT|wxALL,1);
    
    m_tex_enable=new wxCheckBox(panel, ID_TEX_ENABLE, "Enable");
    m_tex_enable->SetValue(true);
    image_cntrls->Add(m_tex_enable,0,wxALIGN_LEFT|wxALL,4);

    boxSizer->Add(image_cntrls,0,wxALIGN_LEFT|wxALL,0);
    
    image_sizer=new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Preview"));
	image_window = new VtxBitmapPanel(panel,wxID_ANY,wxDefaultPosition,wxSize(400,400));
	image_sizer->Add(image_window, 0, wxALIGN_LEFT|wxALL,2);

	boxSizer->Add(image_sizer,0,wxALIGN_LEFT|wxALL,0);

}
void VtxBranchTabs::AddColorTab(wxWindow *panel){
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
void VtxBranchTabs::makeFileList(wxString name){
	char sdir[512];
 	object()->getImageDir(0,sdir);

 	wxDir dir(sdir);
 	if ( !dir.IsOpened() )
 	{
 	    // deal with the error here - wxDir would already log an error message
 	    // explaining the exact reason of the failure
 	    return;
 	}
 	image_name=name;
	files.Clear();
	wxString filename;
	bool cont = dir.GetFirst(&filename);
	while ( cont ) {
		filename=filename.Before('.');
		files.Add(filename);
		cont = dir.GetNext(&filename);
		cout<<filename<<endl;
	}
	files.Sort();
	choices->Clear();
	choices->Append(files);
	if(image_name.IsEmpty())
		choices->SetSelection(0);

 	if(image_name.IsEmpty())
 		image_name=choices->GetStringSelection();
  	choices->SetStringSelection(image_name);

}

void VtxBranchTabs::OnChangedFile(wxCommandEvent& event){
	setObjAttributes();	
}
void VtxBranchTabs::setImagePanel(){
	char dir[512];
	char path[512];
	char sdir[512]={0};

	object()->getImageFilePath((char*)image_name.ToAscii(),0,dir);
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

	wxString str=choices->GetStringSelection();
	image_name=str;
	obj->setImage((char*)image_name.ToAscii());
	setImagePanel();

	wxString expr=getColorExpr();
	char *cstr=(char*)expr.ToAscii();
	obj->setTexEnabled(m_tex_enable->GetValue());
	obj->setColEnabled(m_col_enable->GetValue());
	
    obj->setColorExpr(cstr);
    obj->setExpr((char*)s.ToAscii());
	obj->applyExpr();
	if(strlen(obj->name_str))
		sceneDialog->setNodeName(obj->name_str);

	cout<<"set:"<<s.ToAscii()<<" image:"<<(char*)image_name.ToAscii()<<" color:"<<cstr<<endl;
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
	obj->initArgs();

	object_name->SetValue(obj->nodeName());
	
	m_max_level->SetSelection((int)obj->max_level);
	m_min_level->SetSelection(obj->min_level);
	SplitsSlider->setValue(obj->max_splits);
	LengthSlider->setValue(obj->length);
	WidthSlider->setValue(obj->width);
	RandSlider->setValue(obj->randomness);
	DivergenceSlider->setValue(obj->divergence);
	FlatnessSlider->setValue(obj->flatness);
	WidthTaperSlider->setValue(obj->width_taper);
	LengthTaperSlider->setValue(obj->length_taper);
	FirstBiasSlider->setValue(obj->first_bias);

	
	image_name=obj->getImageName();
	makeFileList(image_name);
	setImagePanel();
	
	TNcolor *tnode=obj->getColor();

	char red[128]="0";
	char green[128]="0";
	char blue[128]="0";
	char alpha[128]="0";
	if(tnode){
		TNarg &args=*((TNarg *)tnode->right);
		args[0]->valueString(red);
		args[1]->valueString(green);
		args[2]->valueString(blue);
		if(args[3])
			args[3]->valueString(alpha);
		else
			strcpy(alpha,"1.0");
	}
	else{
		m_col_enable->SetValue(false);
	}
	m_r_expr->SetValue(red);
	m_g_expr->SetValue(green);
	m_b_expr->SetValue(blue);
	m_a_expr->SetValue(alpha);

	setColorFromExpr();
	saveLastExpr();
   
	update_needed=false;
}

wxString VtxBranchTabs::getColorExpr(){
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

TNcolor* VtxBranchTabs::getColorFromExpr(){
	wxString expr=getColorExpr();
	TNcolor *tnode=(TNcolor*)TheScene->parse_node((char *)expr.ToAscii());
	return tnode;
}
void VtxBranchTabs::setColorFromExpr(){
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

void VtxBranchTabs::restoreLastExpr(){
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
void VtxBranchTabs::setExprFromColor(){
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
	m_a_expr->SetValue("1.0");
}

void VtxBranchTabs::OnRevert(wxCommandEvent& event){
	restoreLastExpr();
	setColorFromExpr();
	setObjAttributes();
	//m_last_expr=new_expr;
}
void VtxBranchTabs::saveLastExpr(){
    m_last_expr=getColorExpr();
}
void VtxBranchTabs::OnChangedExpr(wxCommandEvent& event){
	setColorFromExpr();
	setObjAttributes();
}
void VtxBranchTabs::OnChangedColor(wxColourPickerEvent& event){
	setExprFromColor();
	setObjAttributes();
}

