
#include "VtxImageDialog.h"

#include "FileUtil.h"

#include <wx/filefn.h>
#include <wx/dir.h>
#include "RenderOptions.h"

extern wxString last_gradient;

#define STATE1D 0
#define STATE2D 1
//########################### VtxImageTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
    ID_NORM,
    ID_INVERT,
    ID_GRAYS,
	ID_FILELIST,
    ID_IMAGE_EXPR,
    ID_IMAGE_WIDTH,
    ID_IMAGE_HEIGHT,
    ID_IMAGE_MAP,
	ID_GRADIENT,
	ID_GRADIENT_LIST
};

#define LABEL1 50
#define VALUE1 50
#define SLIDER1 120
//########################### VtxImageTabs Class ########################

IMPLEMENT_CLASS(VtxImageTabs, wxPanel )

BEGIN_EVENT_TABLE(VtxImageTabs, wxPanel)

EVT_TEXT_ENTER(ID_IMAGE_EXPR,VtxImageTabs::OnExprEdit)
EVT_CHOICE(ID_IMAGE_WIDTH, VtxImageTabs::OnImageSize)
EVT_CHOICE(ID_IMAGE_HEIGHT, VtxImageTabs::OnImageSize)
EVT_CHOICE(ID_IMAGE_MAP, VtxImageTabs::OnChanged)

EVT_CHECKBOX(ID_NORM,VtxImageTabs::OnChanged)
EVT_CHECKBOX(ID_INVERT,VtxImageTabs::OnChanged)
EVT_CHECKBOX(ID_GRAYS,VtxImageTabs::OnGraysMode)
EVT_CHECKBOX(ID_GRADIENT,VtxImageTabs::OnGradientMode)

EVT_CHOICE(ID_FILELIST,VtxImageTabs::OnFileSelect)
EVT_CHOICE(ID_GRADIENT_LIST,VtxImageTabs::OnGradientSelect)

END_EVENT_TABLE()

VtxImageTabs::VtxImageTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: wxPanel(parent, id, pos, size,  style,name)
{
	AddImageTab(this);
    //images.makeImagelist();
	update_needed=true;
	type=IMAGE|SPX;
	image_list=0;
	revert_list=0;
	m_name="";
}

void VtxImageTabs::AddImageTab(wxPanel *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer *topline = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBoxSizer* fileio = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("File"));

	m_file_menu=new wxChoice(panel,ID_FILELIST,wxDefaultPosition,wxSize(160,-1));
	m_file_menu->SetColumns(5);

	//m_file_menu=new wxComboBox(panel,ID_FILELIST,"",
	//		wxDefaultPosition,wxSize(160,-1),0, NULL, wxCB_SORT|wxTE_PROCESS_ENTER);

    hline->Add(m_file_menu,0,wxALIGN_LEFT|wxALL,0);

	wxString sizes[]={"1","2","4","8","16","32","64","128","256","512","1024"};
	m_image_height=new wxChoice(panel, ID_IMAGE_HEIGHT, wxDefaultPosition,wxSize(60,-1),11, sizes);
	m_image_height->SetSelection(7);
	hline->Add(m_image_height,0,wxALIGN_LEFT|wxALL,0);

	m_image_width=new wxChoice(panel, ID_IMAGE_WIDTH, wxDefaultPosition,wxSize(60,-1),11, sizes);
	m_image_width->SetSelection(7);
	hline->Add(m_image_width,0,wxALIGN_LEFT|wxALL,0);

	hline->AddSpacer(2);

	wxString maps[]={"Tile","Decal","Sphere","Cylinder"};
	m_image_map=new wxChoice(panel, ID_IMAGE_MAP, wxDefaultPosition,wxSize(100,-1),4, maps);
	m_image_map->SetSelection(0);
	hline->Add(m_image_map,0,wxALIGN_LEFT|wxALL,0);
	fileio->Add(hline,0,wxALIGN_LEFT|wxALL);

	fileio->SetMinSize(wxSize(TABS_WIDTH,-1));

	topline->Add(fileio, 0, wxALIGN_LEFT|wxALL,0);

    wxSize size=fileio->GetMinSize();

	boxSizer->Add(topline, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Options"));
    m_norm_check=new wxCheckBox(panel, ID_NORM, "Norm");
    options->Add(m_norm_check, 0, wxALIGN_LEFT|wxALL,0);
    m_invert_check=new wxCheckBox(panel, ID_INVERT, "Invert");
    options->Add(m_invert_check, 0, wxALIGN_LEFT|wxALL,0);
    m_grays_check=new wxCheckBox(panel, ID_GRAYS, "Gray");
    options->Add(m_grays_check, 0, wxALIGN_LEFT|wxALL,0);
    
    m_gradient_check=new wxCheckBox(panel, ID_GRADIENT, "Color");
    options->Add(m_gradient_check, 0, wxALIGN_LEFT|wxALL,0);
    
    m_gradient_file_menu= new wxChoice(panel,ID_GRADIENT_LIST,wxPoint(-1,4),wxSize(120,-1));
    m_gradient_file_menu->SetColumns(5);
    //m_gradient_file_menu=new wxComboBox(panel,ID_GRADIENT_LIST,"",
    //			wxDefaultPosition,wxSize(120,-1),0, NULL, wxCB_SORT|wxTE_PROCESS_ENTER);
    options->Add(m_gradient_file_menu,0,wxALIGN_LEFT|wxALL,0);
	m_gradient_image = new VtxImageWindow(panel,wxID_ANY,wxDefaultPosition,wxSize(100,26));
	options->Add(m_gradient_image, 0, wxALIGN_LEFT|wxALL,2);

    options->SetMinSize(wxSize(TABS_WIDTH,-1));
    boxSizer->Add(options, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* image_expr_line = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Function"));

	m_image_expr = new ExprTextCtrl(panel,ID_IMAGE_EXPR,"",0,TABS_WIDTH-TABS_BORDER);

	image_expr_line->Add(m_image_expr->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	image_expr_line->SetMinSize(wxSize(TABS_WIDTH,-1));
	boxSizer->Add(image_expr_line, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* image_display = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Image"));
	m_image_window = new VtxImageWindow(this,wxID_ANY,wxDefaultPosition,wxSize(TABS_WIDTH-TABS_BORDER,160));
	image_display->Add(m_image_window, 0, wxALIGN_LEFT|wxALL,0);
	image_display->SetMinSize(wxSize(TABS_WIDTH,-1));

	boxSizer->Add(image_display, 0, wxALIGN_LEFT|wxALL,0);
}

VtxImageTabs::~VtxImageTabs()
{
	freeImageList();
	freeRevertList();
	delete m_image_expr;
}

//-------------------------------------------------------------
// VtxImageTabs::freeImageList() free image file list
//-------------------------------------------------------------
void VtxImageTabs::freeImageList() {
	if(image_list){
		image_list->free();
		delete image_list;
		image_list=0;
	}
}

//-------------------------------------------------------------
// VtxImageTabs::freeRevertList() free image file list
//-------------------------------------------------------------
void VtxImageTabs::freeRevertList() {
	if(revert_list){
		revert_list->free();
		delete revert_list;
		revert_list=0;
	}
}

//-------------------------------------------------------------
// VtxImageTabs::makeRevertList() build image revert list
//-------------------------------------------------------------
void VtxImageTabs::makeRevertList(){
	char path[256];
	FileUtil::getBitmapsDir(path);

    wxDir dir(path);
    if (!dir.IsOpened() ){
     	return;
    }
	LinkedList<ImageSym *> list;
	images.getImageInfo(type, list);
	freeRevertList();
	revert_list=new NameList<ImageSym*>(list);
}

//-------------------------------------------------------------
// VtxImageTabs::makeImageList() build image file list
//-------------------------------------------------------------
void VtxImageTabs::makeImageList(){
	cout<<"VtxImageTabs::makeImageList()"<<endl;
    images.makeImagelist();
	LinkedList<ImageSym *> list;
	images.getImageInfo(type, list);
	freeImageList();
	image_list=new NameList<ImageSym*>(list);
	image_list->ss();

    m_file_menu->Clear();
	ImageSym *is;
	while((is=(*image_list)++)>0){
//		int smode=m_showmode->GetSelection();
//		if(smode==0 || is->info&(INUSE|NEWIMG))
			m_file_menu->Append(is->name());
	}
	int index=m_file_menu->FindString(m_name);
	if(index== wxNOT_FOUND){
		index=0;
	}
	if(m_file_menu->GetCount()>0){
		m_file_menu->SetSelection(index);
		m_name=m_file_menu->GetStringSelection();
	}
}

//-------------------------------------------------------------
// VtxImageTabs::freeImageList() free image file list
//-------------------------------------------------------------
void VtxImageTabs::freeGradientsList() {
	if(gradient_list){
		gradient_list->free();
		delete gradient_list;
		gradient_list=0;
	}
}

//-------------------------------------------------------------
// VtxImageTabs::makeImageList() build image file list
//-------------------------------------------------------------
void VtxImageTabs::setGradientName(){
	int index=m_gradient_file_menu->FindString(m_gradient_name);
	if(index== wxNOT_FOUND){
		index=0;
	}
	if(m_gradient_file_menu->GetCount()>0){
		m_gradient_file_menu->SetSelection(index);
		m_gradient_name=m_gradient_file_menu->GetStringSelection();
	}
	m_gradient_image->setImage(wxString(m_gradient_name),m_gradient_image->TILE);
}
//-------------------------------------------------------------
// VtxImageTabs::makeImageList() build image file list
//-------------------------------------------------------------
void VtxImageTabs::makeGradientsList(){
	LinkedList<ImageSym *> list;
	images.getImageInfo(BANDS|T1D|SPX, list);
	freeGradientsList();
	gradient_list=new NameList<ImageSym*>(list);
	gradient_list->ss();

    m_gradient_file_menu->Clear();
	ImageSym *is;
	while((is=(*gradient_list)++)>0){
		m_gradient_file_menu->Append(is->name());
	}
	m_gradient_file_menu->SetSelection(0);

}

void VtxImageTabs::OnGradientMode(wxCommandEvent& event){
	update_needed=true;
	if(m_gradient_check->IsChecked()){
		setModified(true);	
		m_grays_check->SetValue(false);
		setObjAttributes();
	}
}
void VtxImageTabs::OnGraysMode(wxCommandEvent& event){
	if(m_grays_check->IsChecked()){
		update_needed=true;
		m_gradient_check->SetValue(false);
		setObjAttributes();
	}
}

void VtxImageTabs::OnGradientSelect(wxCommandEvent& event){
	m_gradient_name=m_gradient_file_menu->GetStringSelection();
	m_gradient_image->setImage(wxString(m_gradient_name),m_gradient_image->TILE);
	last_gradient=m_gradient_name;
	if(m_gradient_check->IsChecked()){
		setModified(true);
		update_needed=true;
		imageDialog->UpdateControls();
		setObjAttributes();
	}
}

//-------------------------------------------------------------
// VtxImageTabs::updateControls() update controls
//-------------------------------------------------------------
void VtxImageTabs::updateControls(){
	//if(update_needed)
		getObjAttributes();
}

//-------------------------------------------------------------
// VtxImageTabs::Invalidate() force rebuild of file list
//-------------------------------------------------------------
void VtxImageTabs::Invalidate(){
	update_needed=true;
    setModified(true);
	rebuild();
	getObjAttributes();
	update_needed=true;

	//rebuild();
	makeGradientsList();
	setObjAttributes();
	//makeRevertList();
}

//-------------------------------------------------------------
// VtxImageTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxImageTabs::getObjAttributes(){
	//if(!update_needed)
	//	return;
	makeImageList();
	makeGradientsList();
	displayImage((char*)m_name.ToAscii());
}

//-------------------------------------------------------------
// VtxImageTabs::makeNewImage() create a new image
//-------------------------------------------------------------
void VtxImageTabs::makeNewImage(char *name, char *iexpr){
	TNinode *n=(TNinode*)TheScene->parse_node(iexpr);
	if(!n)
		return;
	n->init();
	delete n;
	displayImage(name);
}

//-------------------------------------------------------------
// VtxImageTabs::displayImage() when switched in
//-------------------------------------------------------------
void VtxImageTabs::displayImage(char *name){

	ImageSym *is=0;
	if(name)
		is=image_list->inlist(name);
	if(!is && image_list->size){
		is=image_list->base[0];
		m_file_menu->SetSelection(0);
		m_name=m_file_menu->GetStringSelection();
	}
	if(!is)
		return;

	is=images.images.inlist(is->name());
	if(!is)
		return;

	NodeIF *n=TheScene->parse_node(is->istring);
    if(!n)
    	return;
	TNimage *inode=(TNimage*)n;
	int opts=inode->opts;
	int map_type=opts&IMAP;
	int display_mode=VtxImageWindow::TILE;
	switch(map_type){
		default:
		case TILE:
			m_image_map->SetSelection(0);
		break;
		case PMAP:
			m_image_map->SetSelection(1);
			break;
		case SMAP:
			m_image_map->SetSelection(2);
			display_mode=VtxImageWindow::SCALE;
			break;
		case CMAP:
			m_image_map->SetSelection(3);
			display_mode=VtxImageWindow::SCALE;
			break;
	}
    m_norm_check->SetValue((opts&NORM)?true:false);
    m_invert_check->SetValue((opts&INVT)?true:false);
    m_grays_check->SetValue((opts&GRAY)?true:false);
    m_gradient_check->SetValue((opts&ACHNL)?true:false);

 	TNarg *arg=(TNarg*)inode->right;

	arg->eval();
	m_image_width->SetSelection((int)log2((int)S0.s));
	arg=arg->next();
	arg->eval();
	m_image_height->SetSelection((int)log2((int)S0.s));
	arg=arg->next();
	TNode *vnode=arg->left;
	vnode->init();
	vnode->eval();
	char vals[512]="";
	vnode->valueString(vals);
	m_image_expr->SetValue(vals);
	arg=arg->next();
	if(arg){
		arg->eval();
		if(S0.strvalid()){
			m_gradient_name=S0.string;
			last_gradient=m_gradient_name;
		}
	}
	m_image_window->setImage(wxString(name),display_mode);
	setGradientName();

	delete inode;
}

//-------------------------------------------------------------
// VtxImageTabs::getImageString() return expr string
//-------------------------------------------------------------
wxString VtxImageTabs::getImageString(wxString name){
	int itype=m_image_map->GetSelection();
	char opts[256];
	switch(itype){
		default:
		case 0:strcpy(opts,"TILE");break;
		case 1:strcpy(opts,"PMAP");break;
		case 2:strcpy(opts,"SMAP");break;
		case 3:strcpy(opts,"CMAP");break;
	}
	if(m_norm_check->GetValue())
		strcat(opts,"|NORM");

	if(m_invert_check->GetValue())
		strcat(opts,"|INVT");

	if(m_grays_check->GetValue())
		strcat(opts,"|GRAY");
	if(m_gradient_check->GetValue())
		strcat(opts,"|ACHNL");

	char iexpr[512];
	wxString istr=m_image_expr->GetValue();
	sprintf(iexpr,"%s",(const char*)istr.ToAscii());

	int exp=m_image_width->GetSelection();
	int width=1<<exp;

	exp=m_image_height->GetSelection();
	int height=1<<exp;

	char buff[512];
	
	if(!m_gradient_name.IsEmpty())
		sprintf(buff,"image(\"%s\",%s,%d,%d,%s,\"%s\");\n",(const char*)name.ToAscii(),opts,width,height,iexpr,m_gradient_name.ToAscii());
	else
		sprintf(buff,"image(\"%s\",%s,%d,%d,%s);\n",(const char*)name.ToAscii(),opts,width,height,iexpr);
	//cout << buff << endl;
	return wxString(buff);
}

int VtxImageTabs::rebuild(){
	if(m_name.IsEmpty())
		return 0;
	wxString istr=getImageString(m_name);
	char buff[512];
	strcpy(buff,istr.ToAscii());
	TNinode *n=(TNinode*)TheScene->parse_node(buff);
	if(!n){
		cout <<"error parsing expr:"<<buff<<endl;
		return 0;
	}
	//cout << buff << endl;
	if(isModified()){
		BIT_ON(n->opts,CHANGED);
		cout<<"rebuilding image"<<endl;
	}
	else{
		BIT_OFF(n->opts,CHANGED);
		cout<<"loading previous image"<<endl;
	}
	n->init();
	int opts=n->opts;
	delete n;
	return opts;	
}
//-------------------------------------------------------------
// VtxImageTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxImageTabs::setObjAttributes(){
	if(!update_needed)
		return;
	int opts=rebuild();
	int map_type=opts&IMAP;
	int display_mode=map_type==SMAP?VtxImageWindow::SCALE:VtxImageWindow::TILE;
    
	m_image_window->setImage(m_name.ToAscii(),display_mode);
	Render.invalidate_textures();
	TheScene->set_changed_detail();
	TheScene->rebuild_all();
	imageDialog->UpdateControls();
	update_needed=false;

}

//-------------------------------------------------------------
// VtxImageTabs::setModified() set modified flag
//-------------------------------------------------------------
void VtxImageTabs::setModified(bool b){
	if(m_name.IsEmpty() || !image_list)
		return;
	char *name=m_name.ToAscii();
	ImageSym *is=image_list->inlist(name);
	if(is){
		if(b)
			BIT_ON(is->info,CHANGED);
		else
			BIT_OFF(is->info,CHANGED);
	}
	Image *img=images.find(name);
	if(img)
		img->set_changed(b?1:0);
}

//-------------------------------------------------------------
// VtxImageTabs::isModified() return modified flag
//-------------------------------------------------------------
bool VtxImageTabs::isModified()  {
	if(!image_list)
		return false;
	char *name=m_name.ToAscii();
	ImageSym *is=image_list->inlist(name);
	if(is && (is->info&CHANGED))
		return true;
	return false;
}

void VtxImageTabs::OnExprEdit(wxCommandEvent& event){
	update_needed=true;
	setObjAttributes();
}


void VtxImageTabs::OnChanged(wxCommandEvent& event){
	update_needed=true;
	setModified(true);
	setObjAttributes();
}

void VtxImageTabs::OnFileSelect(wxCommandEvent& event){
	m_name=m_file_menu->GetStringSelection();
    displayImage((char*)m_name.ToAscii());
	imageDialog->UpdateControls();
}

void VtxImageTabs::setSelection(wxString name){
	m_file_menu->SetStringSelection(name);
	m_name=m_file_menu->GetStringSelection();
    displayImage((char*)m_name.ToAscii());
	imageDialog->UpdateControls();
}

void VtxImageTabs::OnImageSize(wxCommandEvent& event){
	update_needed=true;
	setModified(true);
	setObjAttributes();
	TheView->set_changed_render();
}

void VtxImageTabs::Save(){
	setModified(false);
	rebuild();
	makeImageList();
	makeRevertList();
//	char *name=m_name.ToAscii();
//	ImageSym *isc=image_list->inlist(name);
//	if(!isc)
//		return;
//	ImageSym *isr=revert_list->inlist(name);
//	ImageSym *isn=new ImageSym(isc);
//	if(isr){
//		revert_list->remove(isr);
//	}
//	revert_list->add(isn);
//	revert_list->sort();
	setModified(false);
	imageDialog->UpdateControls();
}

void VtxImageTabs::Revert(){
	if(!revert_list)
		return;
	char *name=(char*)m_name.ToAscii();
	ImageSym *is=revert_list->inlist(name);
	if(is && is->istring){
		makeNewImage(name, is->istring);
	}
	Render.invalidate_textures();
	TheScene->set_changed_detail();
	TheScene->rebuild_all();
	setModified(false);
	imageDialog->UpdateControls();
}

bool VtxImageTabs::Clone(wxString new_name,bool rename){
	char *name=(char*)new_name.ToAscii();
	ImageSym *is=image_list->inlist((char*)new_name.ToAscii());
	if(is)
		return false; // name exists
	is=image_list->inlist((char*)m_name.ToAscii());
	if(is && is->istring){
		wxString iexpr=getImageString(new_name);
		TNinode *n=(TNinode*)TheScene->parse_node((char*)iexpr.ToAscii());
		if(!n)
			return false;
		if(rename)
			images.removeAll((char*)m_name.ToAscii());

		n->init();
		m_name=new_name;
		makeImageList();
		makeRevertList();
		displayImage((char*)m_name.ToAscii());
	    setSelection(name);
		return true;	
	}
	return false;	
}
bool VtxImageTabs::New(wxString name){
	return Clone(name,false);	
}
bool VtxImageTabs::Rename(wxString name){
	return Clone(name,true);
}


bool VtxImageTabs::canDelete()  {
	Image *img=images.find((char*)m_name.ToAscii());
	if(!img || (img && !img->accessed())){
		return true;
	}
	return false;
}

bool VtxImageTabs::canSave()  {
	return true;
}

bool VtxImageTabs::canRevert(){
	if(!revert_list)
		return false;
	if(!isModified())
		return false;
	char *name=m_name.ToAscii();
	ImageSym *is=revert_list->inlist(name);
	if(is)
		return true;
	return false;
}
void VtxImageTabs::Delete(){
	if(canDelete()){
		images.removeAll((char*)m_name.ToAscii());
		m_name="";
		makeImageList();
		displayImage((char*)m_name.ToAscii());
		imageDialog->UpdateControls();
	}
}
