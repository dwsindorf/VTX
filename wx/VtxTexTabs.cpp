
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

//########################### VtxTexTabs Class ########################
enum{
	OBJ_SHOW,
	OBJ_DELETE,
	OBJ_SAVE,
	ID_INTERP,
    ID_FILELIST,
    ID_CLAMP,
    ID_NORMALIZE,
    ID_TEXBUMP,
    ID_TEXCOLOR,
    ID_SHOW_BANDS,
    ID_SHOW_IMAGE,
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

EVT_RADIOBOX(ID_INTERP, VtxTexTabs::OnTexChanged)

EVT_CHECKBOX(ID_CLAMP,VtxTexTabs::OnTexChanged)
EVT_CHECKBOX(ID_NORMALIZE,VtxTexTabs::OnTexChanged)

EVT_CHECKBOX(ID_TEXCOLOR,VtxTexTabs::OnTexChanged)
EVT_CHECKBOX(ID_TEXBUMP,VtxTexTabs::OnTexChanged)

SET_SLIDER_EVENTS(START,VtxTexTabs,Start)
SET_SLIDER_EVENTS(ALPHA,VtxTexTabs,Alpha)
SET_SLIDER_EVENTS(BUMP,VtxTexTabs,BumpAmp)
SET_SLIDER_EVENTS(BIAS,VtxTexTabs,Bias)
SET_SLIDER_EVENTS(ORDERS,VtxTexTabs,Orders)
SET_SLIDER_EVENTS(ORDERS_DELTA,VtxTexTabs,OrdersDelta)
SET_SLIDER_EVENTS(ORDERS_ATTEN,VtxTexTabs,OrdersAtten)
SET_SLIDER_EVENTS(DAMP,VtxTexTabs,BumpDamp)

EVT_CHOICE(ID_FILELIST,VtxTexTabs::OnFileSelect)

EVT_RADIOBUTTON(ID_SHOW_BANDS, VtxTexTabs::OnShowBands)
EVT_RADIOBUTTON(ID_SHOW_IMAGE, VtxTexTabs::OnShowImage)
EVT_RADIOBUTTON(ID_SHOW_IMPORT, VtxTexTabs::OnShowImport)

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
    m_image_type=TYPE_BANDS;
	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	AddImageTab(page);
    AddPage(page,wxT("Image"),true);

    page=new wxPanel(this,wxID_ANY);
    AddFilterTab(page);
    AddPage(page,wxT("Properties"),false);

    //images.makeImagelist();
	m_name="";
	saveState(TYPE_BANDS);
	saveState(TYPE_IMAGE);
	saveState(TYPE_IMPORT);

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

	choices=new wxChoice(panel,ID_FILELIST,wxPoint(-1,4),wxSize(100,-1));
	choices->SetSelection(0);
	image_controls->Add(choices,0,wxALIGN_LEFT|wxALL,2);

	m_image_window = new VtxImageWindow(panel,wxID_ANY,wxDefaultPosition,wxSize(100,26));
	image_controls->Add(m_image_window, 0, wxALIGN_LEFT|wxALL,2);
	wxBitmap bmp(image_dialog_xpm);
	m_edit_button=new wxBitmapButton(panel,ID_SHOW_IMAGE_EDIT,bmp,wxDefaultPosition,wxSize(28,28));
	image_controls->Add(m_edit_button,0,wxALIGN_LEFT|wxALL,0);

	m_bands_button=new wxRadioButton(panel,ID_SHOW_BANDS,wxT("1D"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	m_image_button=new wxRadioButton(panel,ID_SHOW_IMAGE,wxT("2D"),wxDefaultPosition,wxDefaultSize);
	m_import_button=new wxRadioButton(panel,ID_SHOW_IMPORT,wxT("Bitmaps"),wxDefaultPosition,wxDefaultSize);

	image_controls->Add(m_bands_button,0,wxALIGN_LEFT|wxALL,0);
	image_controls->Add(m_image_button,0,wxALIGN_LEFT|wxALL,0);
	image_controls->Add(m_import_button,0,wxALIGN_LEFT|wxALL,0);

	image_controls->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT));


	boxSizer->Add(image_controls, 0, wxALIGN_LEFT|wxALL,0);

    // line 2 start-offset

	wxStaticBoxSizer* overlays = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Orders"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	
	StartSlider=new SliderCtrl(panel,ID_START_SLDR,"Start",LABEL2,VALUE2,SLIDER2);
	StartSlider->setRange(-1,24,0,25);

	hline->Add(StartSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	OrdersSlider=new SliderCtrl(panel,ID_ORDERS_SLDR,"Levels",LABEL2, VALUE2,SLIDER2);
	OrdersSlider->setRange(1,25);
	OrdersSlider->setValue(1.0);

	hline->Add(OrdersSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	
	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	overlays->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);
	
	OrdersAttenSlider=new SliderCtrl(panel,ID_ORDERS_ATTEN_SLDR,"DelA",LABEL2,VALUE2,SLIDER2);
	OrdersAttenSlider->setRange(0.0,1.0);
	OrdersAttenSlider->setValue(0.5);
	hline->Add(OrdersAttenSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OrdersDeltaSlider=new SliderCtrl(panel,ID_ORDERS_DELTA_SLDR,"DelF",LABEL2,VALUE2,SLIDER2);
	OrdersDeltaSlider->setRange(1.0,5.0);
	OrdersDeltaSlider->setValue(2.0);
	hline->Add(OrdersDeltaSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	overlays->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	
	boxSizer->Add(overlays, 0, wxALIGN_LEFT|wxALL,0);

    // line 4 Color

	wxStaticBoxSizer* color_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Color"));
	m_tex_check=new wxCheckBox(panel, ID_TEXCOLOR, "");
	m_tex_check->SetValue(true);
	color_controls->Add(m_tex_check,0,wxALIGN_LEFT|wxALL,4);

	AlphaSlider=new SliderCtrl(panel,ID_ALPHA_SLDR,"Ampl",LABEL1,VALUE2,SLIDER2);
	AlphaSlider->setRange(0,2.0);
	AlphaSlider->setValue(1.0);

	color_controls->Add(AlphaSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	//color_controls->SetMinSize(wxSize(205,BOX_HEIGHT));

	BiasSlider=new SliderCtrl(panel,ID_BIAS_SLDR,"Bias",LABEL1,VALUE2,SLIDER2);
	BiasSlider->setRange(-4.0,4.0);
	BiasSlider->setValue(0.0);
	color_controls->Add(BiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(color_controls, 0, wxALIGN_LEFT|wxALL,0);
	//bias_controls->SetMinSize(wxSize(LINE_WIDTH-EXPR_WIDTH,BOX_HEIGHT));

	//hline->Add(color_controls,0,wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* bump_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Bump"));
	m_bump_check=new wxCheckBox(panel, ID_TEXBUMP, "");
	bump_controls->Add(m_bump_check,0,wxALIGN_LEFT|wxALL,4);

	BumpAmpSlider=new SliderCtrl(panel,ID_BUMP_SLDR,"Ampl",LABEL1,VALUE2,SLIDER2);
	BumpAmpSlider->setRange(-2,2);
	BumpAmpSlider->setValue(1.0);
	bump_controls->Add(BumpAmpSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	BumpDampSlider=new SliderCtrl(panel,ID_DAMP_SLDR,"In",LABEL1,VALUE2,SLIDER2);
	BumpDampSlider->setRange(1,0,0,1);
	BumpDampSlider->setValue(1.0);
	bump_controls->Add(BumpDampSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(bump_controls, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxTexTabs::AddFilterTab(wxWindow *panel) {
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT | wxALL, 5);

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	//wxStaticBoxSizer* texmode = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("Interpolation"));

    wxString lmodes[]={"Nearest","Linear","Mip"};
    interp_mode=new wxRadioBox(panel,ID_INTERP,wxT("Interpolation"),wxPoint(-1,-1),wxSize(200, 44),3,
    		lmodes,3,wxRA_SPECIFY_COLS);
    interp_mode->SetSelection(2);
    hline->Add(interp_mode, 0, wxALIGN_LEFT | wxALL, 0);

	wxStaticBoxSizer* texmap = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("Texture"));
	m_clamp_check=new wxCheckBox(panel, ID_CLAMP, "Clamp");
	texmap->Add(m_clamp_check, 0, wxALIGN_LEFT|wxALL,0);

	m_norm_check=new wxCheckBox(panel, ID_NORMALIZE, "Normalize");
	texmap->Add(m_norm_check, 0, wxALIGN_LEFT|wxALL,0);
    wxSize size=interp_mode->GetMinSize();
	texmap->SetMinSize(wxSize(BOX_WIDTH-200,size.GetHeight()));
	hline->Add(texmap, 0, wxALIGN_LEFT | wxALL, 0);

	boxSizer->Add(hline, 0, wxALIGN_LEFT | wxALL, 0);

	wxStaticBoxSizer* ampl_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Distortion"));
	m_amp_expr = new ExprTextCtrl(panel,ID_AMP_EXPR,"",0,LINE_WIDTH);

	ampl_controls->Add(m_amp_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(ampl_controls, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* alpha_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Opacity"));
	m_alpha_expr = new ExprTextCtrl(panel,ID_ALPHA_EXPR,"",0,LINE_WIDTH);

	alpha_controls->Add(m_alpha_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(alpha_controls, 0, wxALIGN_LEFT|wxALL,0);

}

void VtxTexTabs::makeFileList(){
	int info=0;
	switch(m_image_type){
	case TYPE_BANDS:
		info=BANDS|SPX;
		break;
	case TYPE_IMAGE:
		info=IMAGE|SPX;
		break;
	case TYPE_IMPORT:
		info=0;
		break;
	}
    images.makeImagelist();

	LinkedList<ImageSym *> list;
	images.getImageInfo(info, list);
    files.Clear();
	for(int i=0;i<list.size;i++){
		int m;
		char name[64];
		images.unhashName(list[i]->name(),m,name);
		files.Add(name);
	}
    files.Sort();
	choices->Clear();
	choices->Append(files);

	int index=files.Index(m_name);
	if(index== wxNOT_FOUND)
		index=0;
	choices->SetSelection(index);
	//set_image();
}
void VtxTexTabs::OnFileSelect(wxCommandEvent& event){
	int i=choices->GetCurrentSelection();
    m_name=files[i];
    set_image();
    sceneDialog->setNodeName((const char*)m_name.ToAscii());
    setObjAttributes();
    TheView->set_changed_render();
}

void VtxTexTabs::saveState(int which){
	state[which].file=m_name;
	state[which].amp_expr=m_amp_expr->GetValue();
	state[which].alpha_expr=m_alpha_expr->GetValue();
	state[which].start=StartSlider->getValue();
	state[which].alpha=AlphaSlider->getValue();
	state[which].bump=BumpAmpSlider->getValue();
	state[which].bias=BiasSlider->getValue();
	state[which].interp_state=interp_mode->GetSelection();
	state[which].clamp=m_clamp_check->GetValue();
	state[which].norm=m_norm_check->GetValue();
	state[which].tex_enable=m_tex_check->GetValue();
	state[which].bump_enable=m_bump_check->GetValue();
	state[which].orders=OrdersSlider->getValue();
	state[which].orders_delta=OrdersDeltaSlider->getValue();
	state[which].orders_atten=OrdersAttenSlider->getValue();
	state[which].bump_damp=BumpDampSlider->getValue();
}
void VtxTexTabs::restoreState(int which){
	m_name=state[which].file;
	m_amp_expr->SetValue(state[which].amp_expr);
	m_alpha_expr->SetValue(state[which].alpha_expr);
	StartSlider->setValue(state[which].start);
	AlphaSlider->setValue(state[which].alpha);
	BumpAmpSlider->setValue(state[which].bump);
	BiasSlider->setValue(state[which].bias);
	interp_mode->SetSelection(state[which].interp_state);
	m_tex_check->SetValue(state[which].tex_enable);
	m_bump_check->SetValue(state[which].bump_enable);
	m_clamp_check->SetValue(state[which].clamp);
	m_norm_check->SetValue(state[which].norm);
	OrdersSlider->setValue(state[which].orders);
	OrdersDeltaSlider->setValue(state[which].orders_delta);
	OrdersAttenSlider->setValue(state[which].orders_atten);
	BumpDampSlider->setValue(state[which].bump_damp);
	set_image();
}

void VtxTexTabs::OnShowBands(wxCommandEvent& event){
	wxString file=state[TYPE_IMAGE].file;
	saveState(TYPE_IMAGE);
	state[TYPE_IMAGE].file=file;
	file=state[TYPE_IMPORT].file;
	state[TYPE_IMPORT].file=file;
	m_image_type=TYPE_BANDS;
	restoreState(m_image_type);
	makeFileList();
	int id=choices->FindString(m_name);
	if(id==wxNOT_FOUND)
		id=0;
	choices->SetSelection(id);
    m_name=files[id];
    set_image();
    sceneDialog->setNodeName((const char*)m_name.ToAscii());
    object()->invalidate();
    texture()->invalidate();
    //getObjAttributes();
    setObjAttributes();
    TheView->set_changed_detail();
}
void VtxTexTabs::OnShowImage(wxCommandEvent& event){
	saveState(m_image_type); // save last image type state
	if(m_image_type==TYPE_BANDS)
		restoreState(TYPE_IMAGE);	   // restore 2d-properties
	else
		m_name=state[TYPE_IMAGE].file; // just get last selection

	m_image_type=TYPE_IMAGE;
	makeFileList();
	int id=choices->FindString(m_name);
	if(id==wxNOT_FOUND)
		id=0;
	choices->SetSelection(id);
    m_name=files[id];
    set_image();
    sceneDialog->setNodeName((const char*)m_name.ToAscii());
    object()->invalidate();
    texture()->invalidate();
    //getObjAttributes();
    setObjAttributes();
    TheView->set_changed_detail();
}

void VtxTexTabs::OnShowImport(wxCommandEvent& event){
	saveState(m_image_type);
	if(m_image_type==TYPE_BANDS)
		restoreState(TYPE_IMPORT);	   // restore 2d-properties
	else
		m_name=state[TYPE_IMPORT].file; // just get last selection
	m_image_type=TYPE_IMPORT;
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
    sceneDialog->setNodeName((const char*)m_name.ToAscii());
    object()->invalidate();
    texture()->invalidate();
    //getObjAttributes();
    setObjAttributes();
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
		m_image_window->setImage(m_name);
		if(imageDialog->IsShown())
			imageDialog->Show(m_name,m_image_type);
	}
}

//-------------------------------------------------------------
// VtxTexTabs::updateControls() set controls
//-------------------------------------------------------------
void VtxTexTabs::updateControls(){
	if(update_needed){
		wxString selected=wxString(object()->name);
		int index=files.Index(selected);
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
	if(!m_tex_check->GetValue())
		BIT_ON(opts,NTEX);
	else
		BIT_OFF(opts,NTEX);
	if(!m_bump_check->GetValue())
		BIT_OFF(opts,BUMP);
	else
		BIT_ON(opts,BUMP);

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
    double bias=BiasSlider->getValue();
    double orders=OrdersSlider->getValue();
    double orders_delta=OrdersDeltaSlider->getValue();
    double orders_atten=OrdersAttenSlider->getValue();
    double damp=BumpDampSlider->getValue();

    if(opts & BUMP)
    	sprintf(p+strlen(p),"%g,%g,%g,%g",scale,bump,ampl,bias);
    else
    	sprintf(p+strlen(p),"%g,%g,%g,%g",scale,ampl,bump,bias);

     sprintf(p+strlen(p),",%g,%g,%g,%g)\n",orders,orders_delta,orders_atten,damp);

	tnode->setExpr(p);

	if(tnode->getExprNode()==0){
		cout << "parse error: " << p << endl;
		restoreState(m_image_type);
		update_needed=true;
	}
	else{
		// this call will replace texture so need to reset properties
		tnode->applyExpr();
		Texture *tex=tnode->texture;
		tnode->opts=opts;
		tex->scale=scale;
		tex->bumpamp=bump;
		tex->texamp=ampl;
		tex->bias=bias;
		tex->orders=orders;
		tex->orders_delta=orders_delta;
		tex->orders_atten=orders_atten;
		tex->bump_damp=damp;
		tex->invalidate();
	}
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

	if(tex->t1d())
		BIT_ON(tnode->opts,SEXPR);
	int opts=tnode->opts;

  	m_name=tnode->name;
	char tname[256];
    images.hashName(tnode->name,opts,tname);

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

	TNarg &args=*((TNarg *)tnode->right);

	int i=0;
	m_amp_expr->SetValue("");
	m_alpha_expr->SetValue("");
	if(opts & SEXPR){
		char p[128]={0};
		args[i++]->valueString(p);
		m_amp_expr->SetValue(wxString(p));
	}
	if(opts & AEXPR){
		char p[128]={0};
		args[i++]->valueString(p);
		m_alpha_expr->SetValue(wxString(p));
	}
	if(m_type & SPX){
		if(m_type&BANDS){
			m_image_type=TYPE_BANDS;
			m_import_button->SetValue(false);
			m_bands_button->SetValue(true);
			m_image_button->SetValue(false);
		}
		else if(m_type&IMAGE){
			m_import_button->SetValue(false);
			m_bands_button->SetValue(false);
			m_image_button->SetValue(true);
			m_image_type=TYPE_IMAGE;
		}
	}
	else{
		m_bands_button->SetValue(false);
		m_image_button->SetValue(false);
		m_import_button->SetValue(true);
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

	m_tex_check->SetValue(tnode->texActive());
	m_bump_check->SetValue(tnode->bumpActive());
	makeFileList();
	m_image_window->setImage(m_name);
	saveState(m_image_type);
	update_needed=false;

}
