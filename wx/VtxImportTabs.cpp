
#include "VtxImageDialog.h"

#include "FileUtil.h"

#include <wx/filefn.h>
#include <wx/dir.h>
#include "RenderOptions.h"

#define STATE1D 0
#define STATE2D 1
//########################### VtxImportTabs Class ########################
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
    ID_SHOW_MODE,
};

#define LABEL1 50
#define VALUE1 50
#define SLIDER1 120
//########################### VtxImportTabs Class ########################

IMPLEMENT_CLASS(VtxImportTabs, wxPanel )

BEGIN_EVENT_TABLE(VtxImportTabs, wxPanel)

EVT_COMBOBOX(ID_FILELIST,VtxImportTabs::OnFileSelect)
EVT_RADIOBOX(ID_SHOW_MODE, VtxImportTabs::OnShowMode)

END_EVENT_TABLE()

VtxImportTabs::VtxImportTabs(wxWindow* parent,
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
	image_list=0;
	m_name="";
}

void VtxImportTabs::AddImageTab(wxPanel *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* fileio = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("File"));

	m_file_menu=new wxComboBox(panel,ID_FILELIST,"",
			wxDefaultPosition,wxSize(130,-1),0, NULL, wxCB_SORT|wxTE_PROCESS_ENTER);

	fileio->Add(m_file_menu,0,wxALIGN_LEFT|wxALL,0);

	m_image_width=new wxTextCtrl(panel, ID_IMAGE_WIDTH, "",wxDefaultPosition,wxSize(60,-1),wxTE_READONLY);

	fileio->Add(m_image_width,0,wxALIGN_LEFT|wxALL,0);

	fileio->Add(new wxStaticText(panel, -1, " X ", wxDefaultPosition, wxSize(20,-1)),0,wxALIGN_LEFT|wxALL,5);

	m_image_height=new wxTextCtrl(panel, ID_IMAGE_HEIGHT,"", wxDefaultPosition,wxSize(60,-1),wxTE_READONLY);

	fileio->Add(m_image_height,0,wxALIGN_LEFT|wxALL,0);

	fileio->SetMinSize(wxSize(TABS_WIDTH,-1));
	boxSizer->Add(fileio, 0, wxALIGN_LEFT|wxALL,0);


	wxStaticBoxSizer* image_display = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Image"));
	m_image_window = new VtxImageWindow(this,wxID_ANY,wxDefaultPosition,wxSize(TABS_WIDTH-TABS_BORDER,256));
	image_display->Add(m_image_window, 0, wxALIGN_LEFT|wxALL,2);
	image_display->SetMinSize(wxSize(TABS_WIDTH,-1));

	boxSizer->Add(image_display, 0, wxALIGN_LEFT|wxALL,0);
}

VtxImportTabs::~VtxImportTabs()
{
	freeImageList();
}

//-------------------------------------------------------------
// VtxImportTabs::freeImageList() free image file list
//-------------------------------------------------------------
void VtxImportTabs::freeImageList() {
	if(image_list){
		image_list->free();
		delete image_list;
		image_list=0;
	}
}

//-------------------------------------------------------------
// VtxImportTabs::makeImageList() build image file list
//-------------------------------------------------------------
void VtxImportTabs::makeImageList(){
    images.makeImagelist();

	LinkedList<ImageSym *> list;
	images.getImageInfo(0, list);
	freeImageList();
	image_list=new NameList<ImageSym*>(list);
	image_list->ss();

    m_file_menu->Clear();
	ImageSym *is;
	while((is=(*image_list)++)>0){
		//int smode=m_showmode->GetSelection();
		//if(smode==0 || is->info&(INUSE|NEWIMG))
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
// VtxImportTabs::updateControls() update controls
//-------------------------------------------------------------
void VtxImportTabs::updateControls(){
	if(update_needed)
		getObjAttributes();
}

//-------------------------------------------------------------
// VtxImportTabs::Invalidate() force rebuild of file list
//-------------------------------------------------------------
void VtxImportTabs::Invalidate(){
	update_needed=true;
	getObjAttributes();
}

//-------------------------------------------------------------
// VtxImportTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxImportTabs::getObjAttributes(){
	if(!update_needed)
		return;
	makeImageList();
	displayImage(m_name.ToAscii());
}

//-------------------------------------------------------------
// VtxImportTabs::displayImage() when switched in
//-------------------------------------------------------------
void VtxImportTabs::displayImage(char *name){

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
	m_image_window->setImage(wxString(name));
	Image *img=images.load(name,BMP|JPG);
	char buff[64];
	sprintf(buff,"%d",img->width);
	m_image_width->SetValue(buff);
	sprintf(buff,"%d",img->height);
	m_image_height->SetValue(buff);

}

//-------------------------------------------------------------
// VtxImportTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxImportTabs::setObjAttributes(){
	m_image_window->setImage(m_name.ToAscii());
	update_needed=false;
	imageDialog->UpdateControls();
}

void VtxImportTabs::OnShowMode(wxCommandEvent& event){
	makeImageList();
	displayImage(m_name.ToAscii());
}


void VtxImportTabs::OnFileSelect(wxCommandEvent& event){
	m_name=m_file_menu->GetStringSelection();
    displayImage(m_name.ToAscii());
	imageDialog->UpdateControls();
}

void VtxImportTabs::setSelection(wxString name){
	m_file_menu->SetStringSelection(name);
	m_name=m_file_menu->GetStringSelection();
    displayImage(m_name.ToAscii());
	imageDialog->UpdateControls();
}

//-------------------------------------------------------------
// VtxImportTabs::canRevert() return true if image can be deleted
// - delete denied if image is being used by a Texture
//-------------------------------------------------------------
bool VtxImportTabs::canDelete()  {
	Image *img=images.find(m_name.ToAscii());
	if(!img || (img && !img->accessed()))
		return true;
	return false;
}

void VtxImportTabs::Delete(){
	if(canDelete()){
		images.removeAll(m_name.ToAscii());
		m_name="";
		makeImageList();
		displayImage(m_name.ToAscii());
		imageDialog->UpdateControls();
	}
}
