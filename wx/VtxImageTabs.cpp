
#include "VtxImageDialog.h"

#include "FileUtil.h"

#include <wx/filefn.h>
#include <wx/dir.h>
#include "RenderOptions.h"

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
};

#define LABEL1 50
#define VALUE1 50
#define SLIDER1 120
//########################### VtxImageTabs Class ########################

IMPLEMENT_CLASS(VtxImageTabs, wxPanel )

BEGIN_EVENT_TABLE(VtxImageTabs, wxPanel)

EVT_TEXT_ENTER(ID_IMAGE_EXPR,VtxImageTabs::OnExprEdit)
EVT_TEXT_ENTER(ID_FILELIST,VtxImageTabs::OnFileEdit)
EVT_CHOICE(ID_IMAGE_WIDTH, VtxImageTabs::OnImageSize)
EVT_CHOICE(ID_IMAGE_HEIGHT, VtxImageTabs::OnImageSize)
EVT_CHOICE(ID_IMAGE_MAP, VtxImageTabs::OnChanged)

EVT_CHECKBOX(ID_NORM,VtxImageTabs::OnChanged)
EVT_CHECKBOX(ID_INVERT,VtxImageTabs::OnChanged)
EVT_CHECKBOX(ID_GRAYS,VtxImageTabs::OnChanged)
EVT_COMBOBOX(ID_FILELIST,VtxImageTabs::OnFileSelect)

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

	m_file_menu=new wxComboBox(panel,ID_FILELIST,"",
			wxDefaultPosition,wxSize(130,-1),0, NULL, wxCB_SORT|wxTE_PROCESS_ENTER);

    hline->Add(m_file_menu,0,wxALIGN_LEFT|wxALL,0);

	wxString sizes[]={"1","2","4","8","16","32","64","128","256","512","1024"};
	m_image_width=new wxChoice(panel, ID_IMAGE_WIDTH, wxDefaultPosition,wxSize(60,-1),11, sizes);
	m_image_width->SetSelection(7);

	hline->Add(m_image_width,0,wxALIGN_LEFT|wxALL,0);

	m_image_height=new wxChoice(panel, ID_IMAGE_HEIGHT, wxDefaultPosition,wxSize(60,-1),11, sizes);
	m_image_height->SetSelection(7);

	hline->Add(m_image_height,0,wxALIGN_LEFT|wxALL,0);

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
    m_grays_check=new wxCheckBox(panel, ID_GRAYS, "Grays");
    options->Add(m_grays_check, 0, wxALIGN_LEFT|wxALL,0);

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
	char path[256];
	FileUtil::getBitmapsDir(path);

    wxDir dir(path);

    if (!dir.IsOpened() ){
    	cout << "Could not open Bitmap directory"<< endl;
    	return;
    }
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
// VtxImageTabs::updateControls() update controls
//-------------------------------------------------------------
void VtxImageTabs::updateControls(){
	if(update_needed)
		getObjAttributes();
}

//-------------------------------------------------------------
// VtxImageTabs::Invalidate() force rebuild of file list
//-------------------------------------------------------------
void VtxImageTabs::Invalidate(){
	update_needed=true;
	getObjAttributes();
	makeRevertList();
}

//-------------------------------------------------------------
// VtxImageTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxImageTabs::getObjAttributes(){
	if(!update_needed)
		return;
	makeImageList();
	displayImage(m_name.ToAscii());
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
	switch(opts&IMAP){
		default:
		case TILE:m_image_map->SetSelection(0);break;
		case PMAP:m_image_map->SetSelection(1);break;
		case SMAP:m_image_map->SetSelection(2);break;
		case CMAP:m_image_map->SetSelection(3);break;
	}
    m_norm_check->SetValue((opts&NORM)?true:false);
    m_invert_check->SetValue((opts&INVT)?true:false);
    m_grays_check->SetValue((opts&GRAY)?true:false);
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
	m_image_window->setImage(wxString(name),VtxImageWindow::TILE);
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

	char iexpr[512];
	wxString istr=m_image_expr->GetValue();
	sprintf(iexpr,"%s",(const char*)istr.ToAscii());

	int exp=m_image_width->GetSelection();
	int width=1<<exp;

	exp=m_image_height->GetSelection();
	int height=1<<exp;

	char buff[512];
	sprintf(buff,"image(\"%s\",%s,%d,%d,%s);\n",(const char*)name.ToAscii(),opts,width,height,iexpr);
	//cout << buff << endl;
	return wxString(buff);
}

//-------------------------------------------------------------
// VtxImageTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxImageTabs::setObjAttributes(){
	update_needed=true;
	wxString istr=getImageString(m_name);
	char buff[512];
	strcpy(buff,istr.ToAscii());
	//cout << buff << endl;
	TNinode *n=(TNinode*)TheScene->parse_node(buff);
	if(!n){
		cout <<"error parsing expr:"<<buff<<endl;
		return;
	}
	n->init();
	delete n;
	m_image_window->setImage(m_name.ToAscii(),VtxImageWindow::TILE);
	update_needed=false;
	Render.invalidate_textures();
	TheScene->set_changed_detail();
	TheScene->rebuild_all();
	setModified(true);
	imageDialog->UpdateControls();
}

//-------------------------------------------------------------
// VtxImageTabs::setModified() set modified flag
//-------------------------------------------------------------
void VtxImageTabs::setModified(bool b){
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
	setObjAttributes();
}

void VtxImageTabs::OnFileEdit(wxCommandEvent& event){
	wxString name=m_file_menu->GetValue();
	int index=m_file_menu->FindString(name);
	if(index != wxNOT_FOUND){
		m_file_menu->SetSelection(index);
		m_name=m_file_menu->GetStringSelection();
		displayImage(m_name.ToAscii());
	}
	else{
		wxString istr=getImageString(name); // new name
		char buff[512];
		strcpy(buff,istr.ToAscii());
		TNinode *n=(TNinode*)TheScene->parse_node(buff);
		if(!n)
			return;
		n->init();
		delete n;
		Image *img=images.find(name.ToAscii());
		if(img){
			img->set_newimg(1);
			m_name=name;
			makeImageList();
		}
	}
	imageDialog->UpdateControls();
}

void VtxImageTabs::OnChanged(wxCommandEvent& event){
	setObjAttributes();
}

void VtxImageTabs::OnFileSelect(wxCommandEvent& event){
	m_name=m_file_menu->GetStringSelection();
    displayImage(m_name.ToAscii());
	imageDialog->UpdateControls();
}

void VtxImageTabs::setSelection(wxString name){
	m_file_menu->SetStringSelection(name);
	m_name=m_file_menu->GetStringSelection();
    displayImage(m_name.ToAscii());
	imageDialog->UpdateControls();
}

void VtxImageTabs::OnImageSize(wxCommandEvent& event){
	setObjAttributes();
	TheView->set_changed_render();
}

void VtxImageTabs::Save(){
	char *name=m_name.ToAscii();
	ImageSym *isc=image_list->inlist(name);
	if(!isc)
		return;
	ImageSym *isr=revert_list->inlist(name);
	ImageSym *isn=new ImageSym(isc);
	if(isr){
		revert_list->remove(isr);
	}
	revert_list->add(isn);
	revert_list->sort();

	setModified(false);
	imageDialog->UpdateControls();
}

void VtxImageTabs::Revert(){
	if(!revert_list)
		return;
	char *name=m_name.ToAscii();
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

bool VtxImageTabs::canDelete()  {
	Image *img=images.find(m_name.ToAscii());
	if(!img || (img && !img->accessed())){
		return true;
	}
	return false;
}

bool VtxImageTabs::canSave()  {
	return isModified();
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
		images.removeAll(m_name.ToAscii());
		m_name="";
		makeImageList();
		displayImage(m_name.ToAscii());
		imageDialog->UpdateControls();
	}
}
