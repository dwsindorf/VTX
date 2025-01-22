
#include "VtxTexTabs.h"
#include "VtxSceneDialog.h"
#include "VtxImageDialog.h"
#include "FileUtil.h"

#include <wx/filefn.h>
#include <wx/dir.h>
#include <wx/bmpbuttn.h>
#include <image_dialog.xpm>

#define LINE_HEIGHT 30
#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define BOX_HEIGHT 53
#define LINE_WIDTH TABS_WIDTH-2*TABS_BORDER
#define EXPR_WIDTH 250

#define LABEL1 40
#define VALUE1 50
#define SLIDER1 100
#undef LABEL2
#define LABEL2 60

//########################### VtxTexTabs Class ########################
enum{
	OBJ_SHOW,
	OBJ_DELETE,
	OBJ_SAVE,
	ID_INTERP,
    ID_FILELIST,
    ID_MODE,
    ID_CLAMP,
    ID_NORMALIZE,
    ID_SEASONAL,
    ID_RANDOMIZE,
    ID_TEXBUMP,
    ID_TEXCOLOR,
    ID_SHOW_BANDS,
    ID_SHOW_IMAGE,
    ID_SHOW_MAP,
    ID_SHOW_IMPORT,
    ID_AMP_EXPR,
    ID_ALPHA_EXPR,
    ID_ORDERS,
    ID_START_SLDR,
    ID_START_TEXT,
    ID_BIAS_SLDR,
    ID_BIAS_TEXT,
    ID_ALPHA_SLDR,
    ID_ALPHA_TEXT,
    ID_BUMP_SLDR,
    ID_BUMP_TEXT,
    ID_ORDERS_SLDR,
    ID_ORDERS_TEXT,
    ID_ORDERS_DELTA_SLDR,
    ID_ORDERS_DELTA_TEXT,
    ID_ORDERS_ATTEN_SLDR,
    ID_ORDERS_ATTEN_TEXT,
    ID_DAMP_SLDR,
    ID_DAMP_TEXT,
    ID_HMAP,
    ID_HMAP_AMP_SLDR,
    ID_HMAP_AMP_TEXT,
    ID_HMAP_BIAS_SLDR,
    ID_HMAP_BIAS_TEXT,
    ID_PHI_BIAS_SLDR,
    ID_PHI_BIAS_TEXT,
    ID_HT_BIAS_SLDR,
    ID_HT_BIAS_TEXT,
    ID_BUMP_BIAS_SLDR,
    ID_BUMP_BIAS_TEXT,
    ID_SLOPE_BIAS_SLDR,
    ID_SLOPE_BIAS_TEXT,
   ID_SHOW_IMAGE_EDIT,
};


//########################### VtxTexTabs Class ########################

IMPLEMENT_CLASS(VtxTexTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxTexTabs, VtxTabsMgr)

EVT_TEXT_ENTER(ID_AMP_EXPR,VtxTexTabs::OnTextEnter)
EVT_TEXT_ENTER(ID_ALPHA_EXPR,VtxTexTabs::OnTextEnter)

EVT_MENU(OBJ_DELETE,VtxTexTabs::OnDelete)
EVT_MENU(OBJ_SAVE,VtxTexTabs::OnSave)
EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxTexTabs::OnAddItem)

EVT_MENU(OBJ_SHOW,VtxTexTabs::OnEnable)
EVT_UPDATE_UI(OBJ_SHOW, VtxTexTabs::OnUpdateEnable)

EVT_RADIOBOX(ID_INTERP, VtxTexTabs::OnModeChanged)

EVT_CHECKBOX(ID_CLAMP,VtxTexTabs::OnTexChanged)
EVT_CHECKBOX(ID_NORMALIZE,VtxTexTabs::OnTexChanged)
EVT_CHECKBOX(ID_RANDOMIZE,VtxTexTabs::OnTexChanged)
EVT_CHECKBOX(ID_SEASONAL,VtxTexTabs::OnTexChanged)

EVT_CHECKBOX(ID_TEXCOLOR,VtxTexTabs::OnTexChanged)
EVT_CHECKBOX(ID_TEXBUMP,VtxTexTabs::OnTexChanged)
EVT_CHECKBOX(ID_HMAP,VtxTexTabs::OnHmapChanged)

SET_SLIDER_EVENTS(START,VtxTexTabs,Start)
SET_SLIDER_EVENTS(ALPHA,VtxTexTabs,Alpha)
SET_SLIDER_EVENTS(BUMP,VtxTexTabs,BumpAmp)
SET_SLIDER_EVENTS(HMAP_AMP,VtxTexTabs,HmapAmp)
SET_SLIDER_EVENTS(HMAP_BIAS,VtxTexTabs,HmapBias)

SET_SLIDER_EVENTS(PHI_BIAS,VtxTexTabs,PhiBias)
SET_SLIDER_EVENTS(HT_BIAS,VtxTexTabs,HtBias)
SET_SLIDER_EVENTS(BUMP_BIAS,VtxTexTabs,BumpBias)
SET_SLIDER_EVENTS(SLOPE_BIAS,VtxTexTabs,SlopeBias)

SET_SLIDER_EVENTS(BIAS,VtxTexTabs,Bias)
SET_SLIDER_EVENTS(ORDERS,VtxTexTabs,Orders)
SET_SLIDER_EVENTS(ORDERS_DELTA,VtxTexTabs,OrdersDelta)
SET_SLIDER_EVENTS(ORDERS_ATTEN,VtxTexTabs,OrdersAtten)
SET_SLIDER_EVENTS(DAMP,VtxTexTabs,BumpDamp)

EVT_CHOICE(ID_FILELIST,VtxTexTabs::OnFileSelect)
EVT_CHOICE(ID_MODE,VtxTexTabs::OnModeSelect)

EVT_BUTTON(ID_SHOW_IMAGE_EDIT, VtxTexTabs::OnImageEdit)

END_EVENT_TABLE()

VtxTexTabs::VtxTexTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxTexTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
    choices=0;
    m_type=0;
    m_image_type=TYPE_1D;
	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	AddImageTab(page);
    AddPage(page,wxT("Image"),true);

    page=new wxPanel(this,wxID_ANY);
    AddFilterTab(page);
    AddPage(page,wxT("Mapping"),false);

    //images.makeImagelist();
	m_name="";
	saveState(TYPE_1D);
	saveState(TYPE_2D);
	saveState(TYPE_IMPORT);
	saveState(TYPE_MAP);
	saveState(TYPE_HTMAP);

    return true;
}

int VtxTexTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(OBJ_SHOW,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));
	menu.Append(OBJ_SAVE,wxT("Save.."));

	wxMenu *addmenu=sceneDialog->getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}

	PopupMenu(&menu);
	return menu_action;
}


void VtxTexTabs::AddImageTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    // line 1

	wxStaticBoxSizer* image_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Image"));

	choices=new wxChoice(panel,ID_FILELIST,wxPoint(-1,4),wxSize(120,-1));
	choices->SetSelection(0);
	image_controls->Add(choices,0,wxALIGN_LEFT|wxALL,2);

	//m_image_height=new wxChoice(panel, ID_IMAGE_HEIGHT, wxDefaultPosition,wxSize(60,-1),11, sizes);


	m_image_window = new VtxImageWindow(panel,wxID_ANY,wxDefaultPosition,wxSize(100,26));
	image_controls->Add(m_image_window, 0, wxALIGN_LEFT|wxALL,2);
	wxBitmap bmp(image_dialog_xpm);
	m_edit_button=new wxBitmapButton(panel,ID_SHOW_IMAGE_EDIT,bmp,wxDefaultPosition,wxSize(28,28));
	image_controls->Add(m_edit_button,0,wxALIGN_LEFT|wxALL,0);

	mode=new wxChoice(panel,ID_MODE,wxDefaultPosition,wxSize(60,-1),5,VtxImageDialog::type_names);
	mode->SetSelection(0);
	image_controls->Add(mode,0,wxALIGN_LEFT|wxALL,2);

	image_controls->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT));


	boxSizer->Add(image_controls, 0, wxALIGN_LEFT|wxALL,0);

    // line 4 Color

	wxStaticBoxSizer* color_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Color"));

	AlphaSlider=new SliderCtrl(panel,ID_ALPHA_SLDR,"Ampl",LABEL1,VALUE2,SLIDER2);
	AlphaSlider->setRange(0,2.0);
	AlphaSlider->setValue(1.0);

	color_controls->Add(AlphaSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	//color_controls->SetMinSize(wxSize(205,BOX_HEIGHT));

	BiasSlider=new SliderCtrl(panel,ID_BIAS_SLDR,"Offset",LABEL2,VALUE2,SLIDER2);
	BiasSlider->setRange(-4.0,4.0);
	BiasSlider->setValue(0.0);
	color_controls->Add(BiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	m_tex_check=new wxCheckBox(panel, ID_TEXCOLOR, "");
	m_tex_check->SetValue(true);
	color_controls->Add(m_tex_check,0,wxALIGN_LEFT|wxALL,4);

	boxSizer->Add(color_controls, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* bump_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Bump"));

	BumpAmpSlider=new SliderCtrl(panel,ID_BUMP_SLDR,"Ampl",LABEL1,VALUE2,SLIDER2);
	BumpAmpSlider->setRange(-8,8);
	BumpAmpSlider->setValue(1.0);
	bump_controls->Add(BumpAmpSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	BumpDampSlider=new SliderCtrl(panel,ID_DAMP_SLDR,"Offset",LABEL2,VALUE2,SLIDER2);
	BumpDampSlider->setRange(0,1,0,1);
	BumpDampSlider->setValue(0.0);
	bump_controls->Add(BumpDampSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	m_bump_check=new wxCheckBox(panel, ID_TEXBUMP, "");
	bump_controls->Add(m_bump_check,0,wxALIGN_LEFT|wxALL,4);

	boxSizer->Add(bump_controls, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer*hmap_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("HtMap"));

	HmapAmpSlider=new SliderCtrl(panel,ID_HMAP_AMP_SLDR,"Ampl",LABEL1,VALUE2,SLIDER2);
	HmapAmpSlider->setRange(-50,50);
	HmapAmpSlider->setValue(1.0);
	hmap_controls->Add(HmapAmpSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	HmapBiasSlider=new SliderCtrl(panel,ID_HMAP_BIAS_SLDR,"Offset",LABEL2,VALUE2,SLIDER2);
	HmapBiasSlider->setRange(-1,1);
	HmapBiasSlider->setValue(0.0);
	hmap_controls->Add(HmapBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	m_hmap_check=new wxCheckBox(panel, ID_HMAP, "");
	hmap_controls->Add(m_hmap_check,0,wxALIGN_LEFT|wxALL,4);

	boxSizer->Add(hmap_controls, 0, wxALIGN_LEFT|wxALL,0);
	wxStaticBoxSizer* overlays = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Overlays"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	StartSlider=new SliderCtrl(panel,ID_START_SLDR,"Start",LABEL1,VALUE2,SLIDER2);
	StartSlider->setRange(-1,24,0,25);

	hline->Add(StartSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OrdersSlider=new SliderCtrl(panel,ID_ORDERS_SLDR,"Levels",LABEL2, VALUE2,SLIDER2);
	OrdersSlider->setRange(1,25);
	OrdersSlider->setValue(1.0);

	hline->Add(OrdersSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	overlays->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	OrdersAttenSlider=new SliderCtrl(panel,ID_ORDERS_ATTEN_SLDR,"Ampl",LABEL1,VALUE2,SLIDER2);
	OrdersAttenSlider->setRange(0.0,1.0);
	OrdersAttenSlider->setValue(0.5);
	hline->Add(OrdersAttenSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OrdersDeltaSlider=new SliderCtrl(panel,ID_ORDERS_DELTA_SLDR,"Freq",LABEL2,VALUE2,SLIDER2);
	OrdersDeltaSlider->setRange(1.0,5.0);
	OrdersDeltaSlider->setValue(2.0);
	hline->Add(OrdersDeltaSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	overlays->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(overlays, 0, wxALIGN_LEFT|wxALL,0);

}

void VtxTexTabs::AddFilterTab(wxWindow *panel) {
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT | wxALL, 5);

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

    wxString lmodes[]={"Nearest","Linear","Mip"};
    interp_mode=new wxRadioBox(panel,ID_INTERP,wxT("Interpolation"),wxPoint(-1,-1),wxSize(190, 44),3,
    		lmodes,3,wxRA_SPECIFY_COLS);
    interp_mode->SetSelection(2);
    hline->Add(interp_mode, 0, wxALIGN_LEFT | wxALL, 0);

	wxStaticBoxSizer* texmap = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("Options"));
	m_clamp_check=new wxCheckBox(panel, ID_CLAMP, "Clamp");
	texmap->Add(m_clamp_check, 0, wxALIGN_LEFT|wxALL,0);

	m_norm_check=new wxCheckBox(panel, ID_NORMALIZE, "Normalize");
	texmap->Add(m_norm_check, 0, wxALIGN_LEFT|wxALL,0);

	m_rand_check=new wxCheckBox(panel, ID_NORMALIZE, "Dealias");
	texmap->Add(m_rand_check, 0, wxALIGN_LEFT|wxALL,0);
	
	m_tilt_check=new wxCheckBox(panel, ID_SEASONAL, "Seasonal");
	texmap->Add(m_tilt_check, 0, wxALIGN_LEFT|wxALL,0);

    wxSize size=interp_mode->GetMinSize();
	texmap->SetMinSize(wxSize(BOX_WIDTH-200,size.GetHeight()));
	hline->Add(texmap, 0, wxALIGN_LEFT | wxALL, 0);

	boxSizer->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	wxStaticBoxSizer* modulation = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Modulation"));

	hline = new wxBoxSizer(wxHORIZONTAL);
	hline->Add(new wxStaticText(panel, -1, "Color", wxDefaultPosition, wxSize(40,-1)),5,wxALIGN_LEFT|wxTOP,5);

	//wxStaticBoxSizer* ampl_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Color"));
	m_amp_expr = new ExprTextCtrl(panel,ID_AMP_EXPR,"",10,LINE_WIDTH-60);

	hline->Add(m_amp_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	modulation->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	hline->Add(new wxStaticText(panel, -1, "Alpha", wxDefaultPosition, wxSize(40,-1)),5,wxALIGN_LEFT|wxTOP,5);

	//wxStaticBoxSizer* alpha_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Alpha"));
	m_alpha_expr = new ExprTextCtrl(panel,ID_ALPHA_EXPR,"",10,LINE_WIDTH-60);

	hline->Add(m_alpha_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	modulation->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(modulation, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* biases = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Env. Bias"));
	hline = new wxBoxSizer(wxHORIZONTAL);

	PhiBiasSlider=new SliderCtrl(panel,ID_PHI_BIAS_SLDR,"Lat",LABEL1,VALUE2,SLIDER2);
	PhiBiasSlider->setRange(-1,1);

	hline->Add(PhiBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	HtBiasSlider=new SliderCtrl(panel,ID_HT_BIAS_SLDR,"Height",LABEL2, VALUE2,SLIDER2);
	HtBiasSlider->setRange(-1,1);
	HtBiasSlider->setValue(0.0);

	hline->Add(HtBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	biases->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	BumpBiasSlider=new SliderCtrl(panel,ID_BUMP_BIAS_SLDR,"Bump",LABEL1,VALUE2,SLIDER2);
	BumpBiasSlider->setRange(-1,1);

	hline->Add(BumpBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	SlopeBiasSlider=new SliderCtrl(panel,ID_SLOPE_BIAS_SLDR,"Slope",LABEL2,VALUE2,SLIDER2);
	SlopeBiasSlider->setRange(-1,1);

	hline->Add(SlopeBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	biases->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(biases, 0, wxALIGN_LEFT|wxALL,0);

}

void VtxTexTabs::makeFileList(){
	int info=0;
	switch(m_image_type){
	case TYPE_1D:
		info=BANDS|SPX;
		break;
	case TYPE_2D:
		info=IMAGE|SPX;
		break;
	case TYPE_IMPORT:
		info=IMPORT;
		break;
	case TYPE_MAP:
		info=MAP;
		break;
	case TYPE_HTMAP:
		info=HTMAP;
		break;
	}
    images.makeImagelist();

	LinkedList<ImageSym *> list;
	images.getImageInfo(info, list);
    files.Clear();
	for(int i=0;i<list.size;i++){
		files.Add(list[i]->name());
	}
    files.Sort();
	choices->Clear();

	choices->Append(files);
	choices->SetColumns(5);

	int index=files.Index(m_name);
	if(index== wxNOT_FOUND)
		index=0;
	choices->SetSelection(index);
}

void VtxTexTabs::OnFileSelect(wxCommandEvent& event){
	int i=choices->GetCurrentSelection();
    m_name=files[i];
    set_image();
 
    if(imageDialog->IsShown())
    	imageDialog->Show(m_name,m_image_type);
    sceneDialog->setNodeName((char*)m_name.ToAscii());
    if(m_hmap_check->GetValue()){
		texture()->invalidate();
		invalidateObject();
    }
    else{
        setObjAttributes();
    	TheView->set_changed_render();
    }
}

void VtxTexTabs::saveState(int which){
	state[which].file=m_name;
	state[which].amp_expr=m_amp_expr->GetValue();
	state[which].alpha_expr=m_alpha_expr->GetValue();
	state[which].start=StartSlider->getValue();
	state[which].alpha=AlphaSlider->getValue();
	state[which].bump=BumpAmpSlider->getValue();
	state[which].bias=BiasSlider->getValue();
	state[which].phi_bias=PhiBiasSlider->getValue();
	state[which].ht_bias=HtBiasSlider->getValue();
	state[which].bump_bias=BumpBiasSlider->getValue();
	state[which].slope_bias=SlopeBiasSlider->getValue();

	state[which].interp_state=interp_mode->GetSelection();
	state[which].clamp=m_clamp_check->GetValue();
	state[which].randomize=m_rand_check->GetValue();
	state[which].tilt_enable=m_tilt_check->GetValue();
	state[which].norm=m_norm_check->GetValue();
	state[which].tex_enable=m_tex_check->GetValue();
	state[which].bump_enable=m_bump_check->GetValue();
	state[which].orders=OrdersSlider->getValue();
	state[which].orders_delta=OrdersDeltaSlider->getValue();
	state[which].orders_atten=OrdersAttenSlider->getValue();
	state[which].bump_damp=BumpDampSlider->getValue();
	state[which].hmap=HmapAmpSlider->getValue();
	state[which].hmap_bias=HmapBiasSlider->getValue();
	state[which].hmap_enable=m_hmap_check->GetValue();
}
void VtxTexTabs::restoreState(int which){
	m_name=state[which].file;
	m_amp_expr->SetValue(state[which].amp_expr);
	m_alpha_expr->SetValue(state[which].alpha_expr);
	StartSlider->setValue(state[which].start);
	AlphaSlider->setValue(state[which].alpha);
	BumpAmpSlider->setValue(state[which].bump);
	BiasSlider->setValue(state[which].bias);
	PhiBiasSlider->setValue(state[which].phi_bias);
	HtBiasSlider->setValue(state[which].ht_bias);
	BumpBiasSlider->setValue(state[which].bump_bias);
	SlopeBiasSlider->setValue(state[which].slope_bias);
	interp_mode->SetSelection(state[which].interp_state);
	m_tex_check->SetValue(state[which].tex_enable);
	m_bump_check->SetValue(state[which].bump_enable);
	m_clamp_check->SetValue(state[which].clamp);
	m_norm_check->SetValue(state[which].norm);
	m_rand_check->SetValue(state[which].randomize);
	m_tilt_check->SetValue(state[which].tilt_enable);
	OrdersSlider->setValue(state[which].orders);
	OrdersDeltaSlider->setValue(state[which].orders_delta);
	OrdersAttenSlider->setValue(state[which].orders_atten);
	BumpDampSlider->setValue(state[which].bump_damp);
	HmapAmpSlider->setValue(state[which].hmap);
	HmapBiasSlider->setValue(state[which].hmap_bias);
	m_hmap_check->SetValue(state[which].hmap_enable);
	set_image();
}

void VtxTexTabs::OnModeSelect(wxCommandEvent& event){
	int m=mode->GetSelection();
	get_files(m);

}

void VtxTexTabs::get_files(int type){
	saveState(m_image_type);
	bool oldhmap=m_hmap_check->GetValue();
	restoreState(type);
	bool newhmap=m_hmap_check->GetValue();
	m_image_type=type;
	makeFileList();
	int id=choices->FindString(m_name);
	if(id==wxNOT_FOUND)
		id=0;
	choices->SetSelection(id);
	if(choices->IsEmpty())
		m_name="None";
	else
		m_name=files[id];
    set_image();
    sceneDialog->setNodeName((char*)m_name.ToAscii());
    object()->invalidate();
    if(texture())
    	texture()->invalidate();
    setObjAttributes();
    if(oldhmap!=newhmap){
      	object()->invalidate();
    }
    TheView->set_changed_detail();
    TheScene->rebuild();
}

void VtxTexTabs::OnTextEnter(wxCommandEvent& event){
	invalidateObject();
}

void VtxTexTabs::OnImageEdit(wxCommandEvent& event){
	imageDialog->Show(m_name,m_image_type);
}

//-------------------------------------------------------------
// VtxTexTabs::set_image() show selected image
//-------------------------------------------------------------
void VtxTexTabs::set_image(){
	if(!choices->IsEmpty()){
		if(m_image_type==TYPE_MAP)
			m_image_window->setImage(m_name,VtxImageWindow::SCALE);
		else
			m_image_window->setImage(m_name,VtxImageWindow::TILE);
		if(imageDialog->IsShown())
			imageDialog->Show(m_name,m_image_type);
	}
}

void VtxTexTabs::setSelected(TreeNode *s){
//	cout<<"VtxTexTabs::getObjAttributes() object:"<<TheScene->changed_object()
//			<<" render:"<<TheScene->changed_object()
//			<<" detail:"<<TheScene->changed_detail()
//			<<" moved:"<<TheScene->moved()
//			<<endl;
    update_needed=true;
	object_node=s;
}

//-------------------------------------------------------------
// VtxTexTabs::updateControls() set controls
//-------------------------------------------------------------
void VtxTexTabs::updateControls(){
	if(update_needed){
		wxString selected=wxString(object()->name);
		int index=files.Index(selected);
		if(index>=0)
		choices->SetSelection(index);
		getObjAttributes();
	}
}

//-------------------------------------------------------------
// VtxTexTabs::set_ampl() set texamp expr
//-------------------------------------------------------------
void VtxTexTabs::set_ampl(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(AlphaSlider,S0.s);
	}
	else
		updateSlider(AlphaSlider,1.0);
}

//-------------------------------------------------------------
// VtxTexTabs::set_bump() set bump expr
//-------------------------------------------------------------
void VtxTexTabs::set_bump(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(BumpAmpSlider,S0.s);
	}
	else
		updateSlider(BumpAmpSlider,1.0);
}

//-------------------------------------------------------------
// VtxTexTabs::set_bias() set bias expr
//-------------------------------------------------------------
void VtxTexTabs::set_bias(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(BiasSlider,S0.s);
	}
	else
		updateSlider(BiasSlider,0.0);
}

//-------------------------------------------------------------
// VtxTexTabs::set_hmap() set bump expr
//-------------------------------------------------------------
void VtxTexTabs::set_hmap(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(HmapAmpSlider,S0.s);
	}
	else
		updateSlider(HmapAmpSlider,1.0);
}
//-------------------------------------------------------------
// VtxTexTabs::set_hmap() set bump expr
//-------------------------------------------------------------
void VtxTexTabs::set_hmap_bias(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(HmapBiasSlider,S0.s);
	}
	else
		updateSlider(HmapBiasSlider,1.0);
}

//-------------------------------------------------------------
// VtxTexTabs::set_pbias() set bias expr
//-------------------------------------------------------------
void VtxTexTabs::set_pbias(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(PhiBiasSlider,S0.s);
	}
	else
		updateSlider(PhiBiasSlider,0.0);
}
//-------------------------------------------------------------
// VtxTexTabs::set_hbias() set bias expr
//-------------------------------------------------------------
void VtxTexTabs::set_hbias(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(HtBiasSlider,S0.s);
	}
	else
		updateSlider(HtBiasSlider,0.0);
}
//-------------------------------------------------------------
// VtxTexTabs::set_bbias() set bias expr
//-------------------------------------------------------------
void VtxTexTabs::set_bbias(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(BumpBiasSlider,S0.s);
	}
	else
		updateSlider(BumpBiasSlider,0.0);
}
//-------------------------------------------------------------
// VtxTexTabs::set_bbias() set bias expr
//-------------------------------------------------------------
void VtxTexTabs::set_sbias(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(SlopeBiasSlider,S0.s);
	}
	else
		updateSlider(SlopeBiasSlider,0.0);
}

//-------------------------------------------------------------
// VtxTexTabs::set_start() set start
//-------------------------------------------------------------
void VtxTexTabs::set_start(TNode *node)
{
	double f=0;
	if(node){
		node->eval();
		f=log2(S0.s);
	}
	updateSlider(StartSlider,f);
}

//-------------------------------------------------------------
// VtxTexTabs::set_orders() set texture orders
//-------------------------------------------------------------
void VtxTexTabs::set_orders(TNode *node)
{
	double f=1;
	if(node){
		node->eval();
		f=S0.s;
	}
	updateSlider(OrdersSlider,f);
}

//-------------------------------------------------------------
// VtxTexTabs::set_orders_delta() set texture orders delta
//-------------------------------------------------------------
void VtxTexTabs::set_orders_delta(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(OrdersDeltaSlider,S0.s);
	}
	else
		updateSlider(OrdersDeltaSlider,2.0);
}

//-------------------------------------------------------------
// VtxTexTabs::set_orders_atten() set texture orders attenuation
//-------------------------------------------------------------
void VtxTexTabs::set_orders_atten(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(OrdersAttenSlider,S0.s);
	}
	else
		updateSlider(OrdersAttenSlider,1.0);
}

//-------------------------------------------------------------
// VtxTexTabs::set_damp() set texture damp
//-------------------------------------------------------------
void VtxTexTabs::set_damp(TNode *node)
{
	if(node){
		node->eval();
		updateSlider(BumpDampSlider,S0.s);
	}
	else
		updateSlider(BumpDampSlider,0.0);
}

//-------------------------------------------------------------
// VtxTexTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxTexTabs::setObjAttributes(){
	update_needed=true;
	TNtexture *tnode=object();
	char p[1024]="";
	char opts_str[256]="";

	int opts=tnode->opts;

	BIT_OFF(opts,BUMP);
	BIT_OFF(opts,MMAP|INTERP|NEAREST);

	int imode=interp_mode->GetSelection();
	if(imode==0)
		BIT_ON(opts,NEAREST);
	else if(imode==1)
		BIT_ON(opts,INTERP);

	if(m_clamp_check->GetValue())
		BIT_ON(opts,BORDER);
	else
		BIT_OFF(opts,BORDER);
	if(m_norm_check->GetValue())
		BIT_ON(opts,NORM);
	else
		BIT_OFF(opts,NORM);
	if(m_tex_check->GetValue())
		BIT_ON(opts,TEX);
	else
		BIT_OFF(opts,TEX);
	if(m_rand_check->GetValue())
		BIT_ON(opts,RANDOMIZE);
	else
		BIT_OFF(opts,RANDOMIZE);
	if(m_tilt_check->GetValue())
		BIT_ON(opts,TBIAS);
	else
		BIT_OFF(opts,TBIAS);

	if(!m_bump_check->GetValue())
		BIT_OFF(opts,BUMP);
	else
		BIT_ON(opts,BUMP);
	if(!m_hmap_check->GetValue())
		BIT_OFF(opts,HMAP);
	else
		BIT_ON(opts,HMAP);

	BIT_OFF(opts,SEXPR);
	BIT_OFF(opts,AEXPR);
	wxString s_expr=m_amp_expr->GetValue();
	if(s_expr.Length()>0)
		BIT_ON(opts,SEXPR);
	wxString a_expr=m_alpha_expr->GetValue();
	if(a_expr.Length()>0)
		BIT_ON(opts,AEXPR);

	TNinode::optionString(opts_str,opts);
	sprintf(p,"Texture(\"%s\",%s",(const char*)m_name.ToAscii(),opts_str);

	if(opts & SEXPR)
		sprintf(p+strlen(p),"%s,",(const char*)s_expr.ToAscii());
	if((opts & AEXPR))
		sprintf(p+strlen(p),"%s,",(const char*)a_expr.ToAscii());
	double f=StartSlider->getValue();
	double scale=exp2(f);
	
    double ampl=AlphaSlider->getValue();
    double bump=BumpAmpSlider->getValue();
    double hmap=HmapAmpSlider->getValue();
    double hmap_bias=HmapBiasSlider->getValue();
    double bias=BiasSlider->getValue();
    double orders=OrdersSlider->getValue();
    double orders_delta=OrdersDeltaSlider->getValue();
    double orders_atten=OrdersAttenSlider->getValue();
    double damp=BumpDampSlider->getValue();
    double pbias=PhiBiasSlider->getValue();
    double hbias=HtBiasSlider->getValue();
    double bbias=BumpBiasSlider->getValue();
    double sbias=SlopeBiasSlider->getValue();

    if(opts & BUMP)
    	sprintf(p+strlen(p),"%g,%g,%g,%g",scale,bump,ampl,bias);
    else
    	sprintf(p+strlen(p),"%g,%g,%g,%g",scale,ampl,bump,bias);

    sprintf(p+strlen(p),",%g,%g,%g,%g",orders,orders_delta,orders_atten,damp);

    if(opts & HMAP)
    	sprintf(p+strlen(p),",%g,%g",hmap,hmap_bias);
    sprintf(p+strlen(p),",%g,%g,%g,%g",pbias,hbias,bbias,sbias);

    sprintf(p+strlen(p),")\n");

	tnode->setExpr(p);

	if(tnode->getExprNode()==0){
		cout << "parse error: " << p << endl;
		restoreState(m_image_type);
		update_needed=true;
	}
	else if(tnode->texture){
		// this call will replace texture so need to reset properties
		tnode->applyExpr();
		Texture *tex=tnode->texture;
		if(tex){
			tnode->opts=opts;
			tex->scale=scale;
			tex->bumpamp=bump;
			tex->texamp=ampl;
			tex->bias=bias;
			tex->orders=orders;
			tex->orders_delta=orders_delta;
			tex->orders_atten=orders_atten;
			tex->bump_damp=damp;
			tex->hmap_amp=hmap;
			tex->hmap_bias=hmap_bias;
			tex->phi_bias=pbias;
			tex->height_bias=hbias;
			tex->bump_bias=bbias;
			tex->slope_bias=sbias;
			tex->invalidate();
		}
	}

	if(opts & HMAP){
		object()->invalidate();
		TheView->set_changed_detail();
		TheScene->rebuild();
	}
	else
		invalidateTexture();
	

}
//-------------------------------------------------------------
// VtxTexTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxTexTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNtexture *tnode=(TNtexture*)object();

	TNtexture *expr=(TNtexture*)tnode->getExprNode();
	Texture *tex=texture();
	if(expr)
		tnode=expr;

	if(tnode->texture)
		d_data=tnode->texture->d_data;


	//if(tex && tex->t1d())
	//	BIT_ON(tnode->opts,SEXPR);
	int opts=tnode->opts;

  	m_name=tnode->name;
	char tname[256];
    //images.copyName(tnode->name,tname);
	strcpy(tname,tnode->name);

	ImageSym *is=images.getImageInfo(tname);
	m_type=is->info;
	delete is;

	int interp=opts & INTRP_MASK;
	switch(interp){
	case NEAREST:
		interp_mode->SetSelection(0);
		break;
	case INTERP:
		interp_mode->SetSelection(1);
		break;
	case MMAP:
		interp_mode->SetSelection(2);
		break;
	}

	m_clamp_check->SetValue((opts&BORDER)?true:false);
	m_norm_check->SetValue((opts&NORM)?true:false);
	m_rand_check->SetValue((opts&RANDOMIZE)?true:false);
	m_tilt_check->SetValue((opts&TBIAS)?true:false);

	TNarg &args=*((TNarg *)tnode->right);

	int i=0;
	m_amp_expr->SetValue("");
	m_alpha_expr->SetValue("");
	if(opts & SEXPR){
		char p[256]={0};
		args[i++]->valueString(p);
		m_amp_expr->SetValue(wxString(p));
	}
	if(opts & AEXPR){
		char p[2048]={0};
		args[i++]->valueString(p);
		m_alpha_expr->SetValue(wxString(p));
	}
	if(m_type & SPX){
		if(m_type&BANDS){
			m_image_type=TYPE_1D;
			mode->SetSelection(TYPE_1D);
		}
		else if(m_type&IMAGE){
			m_image_type=TYPE_2D;
			mode->SetSelection(TYPE_2D);
		}
	}
	else if((m_type & IMTYPE) == MAP){
		mode->SetSelection(TYPE_MAP);
		m_image_type=TYPE_MAP;
	}
	else if((m_type & IMTYPE) == HTMAP){
		mode->SetSelection(TYPE_HTMAP);
		m_image_type=TYPE_HTMAP;
	}
	else{
		mode->SetSelection(TYPE_IMPORT);
		m_image_type=TYPE_IMPORT;
	}
	set_start(args[i++]);

	if(opts & BUMP){
		set_bump(args[i++]);
		set_ampl(args[i++]);
	}
	else{
		set_ampl(args[i++]);
		set_bump(args[i++]);
	}
	set_bias(args[i++]);
	set_orders(args[i++]);
	set_orders_delta(args[i++]);
	set_orders_atten(args[i++]);
	set_damp(args[i++]);
	if(opts & HMAP){
		set_hmap(args[i++]);
		set_hmap_bias(args[i++]);
	}
	set_pbias(args[i++]);
	set_hbias(args[i++]);
	set_bbias(args[i++]);
	set_sbias(args[i++]);

	m_tex_check->SetValue(tnode->texActive());
	m_bump_check->SetValue(tnode->bumpActive());
	m_hmap_check->SetValue(tnode->hmapActive());

	makeFileList();
	m_image_window->setImage(m_name,VtxImageWindow::TILE);
	saveState(m_image_type);
	update_needed=false;
}

void VtxTexTabs::setHtmap(){
	HmapAmpSlider->setValueFromSlider();
	double val = HmapAmpSlider->getValue();
	bool hm=m_hmap_check->GetValue();
	texture()->hmap_amp=val;
	texture()->invalidate();
	if(hm)
		invalidateObject();
}


