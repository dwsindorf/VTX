#include "wx/wxprec.h"
#include "wx/imagpng.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/clrpicker.h>
#include <wx/radiobox.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <wx/gdicmn.h>
#include <malloc.h>

#include "VtxImageDialog.h"
#include "OrbitalClass.h"
#include "UniverseModel.h"

// define all resource ids here
enum {
	ID_OK,
    ID_SAVE,
    ID_REVERT,
    ID_DELETE,
    ID_NEW,
    ID_RENAME,
    ID_SWITCH,
    ID_SIZE,
 };

#define BTNSIZE wxSize(70,30)

wxString last_gradient("");

class FileIODialog: public wxDialog {
private:
	wxTextCtrl *name;
public:
	FileIODialog(const wxString &title,wxString label):
		wxDialog(NULL, -1, title, wxDefaultPosition, wxSize(200, 100)) 
	{
		wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
		
		SetSizer(topSizer);
		
		wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
		topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);   
		wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
		name = new wxTextCtrl(this, -1, label, wxDefaultPosition,wxSize(150, 30), wxTE_PROCESS_ENTER);
		hline->Add(name, 1);
		wxButton *okButton = new wxButton(this, wxID_OK, wxT("Ok"), wxDefaultPosition,
				wxSize(70, 30));
		hline->Add(okButton, 1);
		boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
		Centre();
	}
	wxString getName(){
		return name->GetValue();
	}
};


//########################### VtxImageDialog Class Methods #############################
IMPLEMENT_CLASS( VtxImageDialog, wxFrame )

BEGIN_EVENT_TABLE( VtxImageDialog, wxFrame )

EVT_CLOSE(VtxImageDialog::OnClose)

EVT_BUTTON(wxID_OK, VtxImageDialog::OnOk)
EVT_BUTTON(ID_SAVE, VtxImageDialog::OnSave)
EVT_BUTTON(ID_REVERT, VtxImageDialog::OnRevert)
EVT_BUTTON(ID_DELETE, VtxImageDialog::OnDelete)
EVT_BUTTON(ID_NEW, VtxImageDialog::OnNew)
EVT_BUTTON(ID_RENAME, VtxImageDialog::OnRename)

EVT_NOTEBOOK_PAGE_CHANGED(ID_SWITCH,VtxImageDialog::OnTabSwitch)

END_EVENT_TABLE()


VtxImageDialog::VtxImageDialog( wxWindow* parent,
  wxWindowID id, const wxString& caption,
  const wxPoint& pos, const wxSize& size, long style )
{	
    Create(parent, id, caption, pos,wxSize(DLG_WIDTH,DLG_HEIGHT), style);
}

wxString VtxImageDialog::type_names[]={"1D","2D","Tex","Map","HMap"};

wxString  VtxImageDialog::getSelection(){
	wxString name;
	switch(m_type){
	case TYPE_1D:
		name=m_1D_tabs->getSelection();
		break;
	case TYPE_2D:
		name=m_2D_tabs->getSelection();
		break;
	case TYPE_IMPORT:
		name=m_img_tabs->getSelection();
		break;
	case TYPE_MAP:
		name=m_map_tabs->getSelection();
		break;
	case TYPE_HTMAP:
		name=m_hmap_tabs->getSelection();
		break;
	}
	return name;
}
void VtxImageDialog::OnNew(wxCommandEvent &event){
	wxString name=getSelection();
	FileIODialog *dlg = new FileIODialog("New",name);
    if ( dlg->ShowModal() == wxID_OK ){
    	bool ok=false;
    	wxString new_name=dlg->getName();
    	switch(m_type){
    	case TYPE_1D:
    		ok=m_1D_tabs->New(new_name);
    		m_2D_tabs->Invalidate();
    		break;
    	case TYPE_2D:
    		ok=m_2D_tabs->New(new_name);
    		break;
    	}
    	if(ok)
    		cout<<new_name+" created"<<endl;
    	else
    		cout<<new_name+" exists"<<endl;
    }
    dlg->Destroy();
}

void VtxImageDialog::OnRename(wxCommandEvent &event){
	wxString name=getSelection();

	FileIODialog *dlg = new FileIODialog("Rename",name);
    if ( dlg->ShowModal() == wxID_OK ){
    	bool ok=false;
    	wxString new_name=dlg->getName();
    	switch(m_type){
    	case TYPE_1D:
    		ok=m_1D_tabs->Rename(new_name);
    		break;
    	case TYPE_2D:
    		ok=m_2D_tabs->Rename(new_name);
    		break;
    	}
    	if(ok)
    		cout<<new_name+" created"<<endl;
    	else
    		cout<<new_name+" exists"<<endl;
    }
    dlg->Destroy();
}

void VtxImageDialog::OnClose(wxCloseEvent& event){
	event.Veto(true);
	Show(false);
}

bool VtxImageDialog::Create( wxWindow* parent,
  wxWindowID id, const wxString& title,
  const wxPoint& pos, const wxSize& size, long style )
{

    if (!wxFrame::Create( parent, id, wxT("Image Editor"),
    		pos, size,  wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)))
        return false;
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    m_type=TYPE_1D;
	wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    m_save = new wxButton(this,ID_SAVE,"Save",wxDefaultPosition,BTNSIZE);
    m_revert = new wxButton(this,ID_REVERT,"Revert",wxDefaultPosition,BTNSIZE);
    m_delete = new wxButton(this,ID_DELETE,"Delete",wxDefaultPosition,BTNSIZE);
    m_new = new wxButton(this,ID_NEW,"New",wxDefaultPosition,BTNSIZE);
    m_rename = new wxButton(this,ID_RENAME,"Rename",wxDefaultPosition,BTNSIZE);
    
    m_rename->Enable(true);

    
    buttons->Add(m_save, 0, wxALIGN_LEFT|wxALL,0);
    buttons->Add(m_revert, 0, wxALIGN_LEFT|wxALL,0);
    buttons->Add(m_delete, 0, wxALIGN_LEFT|wxALL,0);
    buttons->Add(m_new, 0, wxALIGN_LEFT|wxALL,0);
    buttons->Add(m_rename, 0, wxALIGN_LEFT|wxALL,0);


    m_tabs=new wxNotebook(this,ID_SWITCH,wxPoint(0,0),wxSize(DLG_WIDTH,DLG_HEIGHT));
    m_1D_tabs=new VtxBandsTabs(m_tabs,wxID_ANY);
	m_2D_tabs=new VtxImageTabs(m_tabs,wxID_ANY);
    m_img_tabs=new VtxImportTabs(m_tabs,IMPORT,wxID_ANY);
    m_map_tabs=new VtxImportTabs(m_tabs,MAP,wxID_ANY);
    m_hmap_tabs=new VtxImportTabs(m_tabs,HTMAP,wxID_ANY);

 	m_tabs->AddPage(m_1D_tabs,wxT("1D"),true);
 	m_tabs->AddPage(m_2D_tabs,wxT("2D"),false);
 	m_tabs->AddPage(m_img_tabs,wxT("Img"),false);
 	m_tabs->AddPage(m_map_tabs,wxT("Map"),false);
 	m_tabs->AddPage(m_hmap_tabs,wxT("HMap"),false);

	topSizer->Add(m_tabs,0,wxALIGN_LEFT|wxALL);

    topSizer->Add(buttons,0,wxALIGN_LEFT|wxALL);
    SetSizerAndFit(topSizer);

    Centre();

    SetBackgroundColour(m_tabs->GetBackgroundColour());

    position=GetScreenPosition();
    Move(position.x+size.GetWidth()+150,position.y);
    position=GetScreenPosition();
    return true;
}

void VtxImageDialog::setTitle(){
	wxString title("Image Editor");
	SetTitle(title);

}
bool VtxImageDialog::Show(const bool b){
	if(b){
		m_1D_tabs->updateControls();
		m_2D_tabs->updateControls();
		m_img_tabs->updateControls();
		m_map_tabs->updateControls();
		m_hmap_tabs->updateControls();
		UpdateControls();
	}
	return wxFrame::Show(b);
}

bool VtxImageDialog::Show(wxString name, int type){
	m_type=type;
	image_name=name;
	m_tabs->SetSelection(m_type);
	switch(m_type){
	case TYPE_2D:
		m_2D_tabs->setSelection(name);
		break;
	case TYPE_1D:
		last_gradient=name;
		m_1D_tabs->setSelection(name);
		break;
	case TYPE_IMPORT:
		m_img_tabs->setSelection(name);
		break;
	case TYPE_MAP:
		m_map_tabs->setSelection(name);
		break;
	case TYPE_HTMAP:
		m_hmap_tabs->setSelection(name);
		break;
	}
	return Show(true);
}

void VtxImageDialog::Invalidate(){
	if(IsShown()){
		m_2D_tabs->Invalidate();
		m_1D_tabs->Invalidate();
		m_img_tabs->Invalidate();
		m_map_tabs->Invalidate();
		m_hmap_tabs->Invalidate();
	}
}

void VtxImageDialog::OnOk(wxCommandEvent& event){
	Show(false);
}
void VtxImageDialog::OnTabSwitch(wxNotebookEvent &event){
	m_type=event.GetSelection();
	switch(m_type){
	case TYPE_2D:
		m_revert->Enable(m_2D_tabs->canRevert());
		m_delete->Enable(m_2D_tabs->canDelete());
		m_save->Enable(m_2D_tabs->canSave());
		m_new->Enable(m_2D_tabs->canSave());
		break;
	case TYPE_1D:
		m_revert->Enable(m_1D_tabs->canRevert());
		m_delete->Enable(m_1D_tabs->canDelete());
		m_save->Enable(m_1D_tabs->canSave());
		m_new->Enable(m_1D_tabs->canSave());
		if(!last_gradient.IsEmpty())
			m_1D_tabs->setSelection(last_gradient);
		break;
	case TYPE_IMPORT:
		m_revert->Enable(m_img_tabs->canRevert());
		m_delete->Enable(m_img_tabs->canDelete());
		m_save->Enable(m_img_tabs->canSave());
		m_new->Enable(m_img_tabs->canSave());
		break;
	case TYPE_MAP:
		m_revert->Enable(m_map_tabs->canRevert());
		m_delete->Enable(m_map_tabs->canDelete());
		m_save->Enable(m_map_tabs->canSave());
		m_new->Enable(m_map_tabs->canSave());
		break;
	case TYPE_HTMAP:
		m_revert->Enable(m_hmap_tabs->canRevert());
		m_delete->Enable(m_hmap_tabs->canDelete());
		m_save->Enable(m_hmap_tabs->canSave());
		m_new->Enable(m_hmap_tabs->canSave());
		break;
	}

}
void VtxImageDialog::OnSave(wxCommandEvent &event){
	//Invalidate();
	switch(m_type){
	case TYPE_2D:
		//m_2D_tabs->Save();
		m_2D_tabs->Invalidate();
		break;
	case TYPE_1D:
		m_1D_tabs->Invalidate();
		break;
	}
}


void VtxImageDialog::OnRevert(wxCommandEvent &event){
	switch(m_type){
	case TYPE_2D:
		m_2D_tabs->Revert();
		break;
	case TYPE_1D:
		m_1D_tabs->Revert();
		break;
	case TYPE_IMPORT:
		m_img_tabs->Revert();
		break;
	case TYPE_MAP:
		m_map_tabs->Revert();
		break;
	case TYPE_HTMAP:
		m_hmap_tabs->Revert();
		break;
	}
}
void VtxImageDialog::OnDelete(wxCommandEvent &event){
	switch(m_type){
	case TYPE_2D:
		m_2D_tabs->Delete();
		break;
	case TYPE_1D:
		m_1D_tabs->Delete();
		break;
	case TYPE_IMPORT:
		m_img_tabs->Delete();
		break;
	case TYPE_MAP:
		m_map_tabs->Delete();
		break;
	case TYPE_HTMAP:
		m_hmap_tabs->Delete();
		break;
	}
}
void VtxImageDialog::UpdateControls(){
	switch(m_type){
	case TYPE_2D:
		m_revert->Enable(m_2D_tabs->canRevert());
		m_delete->Enable(m_2D_tabs->canDelete());
		m_save->Enable(m_2D_tabs->canSave());
		m_new->Enable(m_2D_tabs->canSave());
		break;
	case TYPE_1D:
		m_revert->Enable(m_1D_tabs->canRevert());
		m_delete->Enable(m_1D_tabs->canDelete());
		m_save->Enable(m_1D_tabs->canSave());
		m_new->Enable(m_1D_tabs->canSave());
		break;
	case TYPE_IMPORT:
		m_revert->Enable(m_img_tabs->canRevert());
		m_delete->Enable(m_img_tabs->canDelete());
		m_save->Enable(m_img_tabs->canSave());
		m_new->Enable(m_img_tabs->canSave());
		break;
	case TYPE_MAP:
		m_revert->Enable(m_map_tabs->canRevert());
		m_delete->Enable(m_map_tabs->canDelete());
		m_save->Enable(m_map_tabs->canSave());
		m_new->Enable(m_map_tabs->canSave());
		break;
	case TYPE_HTMAP:
		m_revert->Enable(m_hmap_tabs->canRevert());
		m_delete->Enable(m_hmap_tabs->canDelete());
		m_save->Enable(m_hmap_tabs->canSave());
		m_new->Enable(m_hmap_tabs->canSave());
		break;
	}
	Refresh();
}
