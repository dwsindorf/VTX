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

//#define TYPE_IMAGE 0
//#define TYPE_BANDS 1

// define all resource ids here
enum {
	ID_OK,
    ID_SAVE,
    ID_REVERT,
    ID_DELETE,
    ID_SWITCH,
    ID_SIZE,
 };

#define BTNSIZE wxSize(60,30)

//########################### VtxImageDialog Class Methods #############################
IMPLEMENT_CLASS( VtxImageDialog, wxFrame )

BEGIN_EVENT_TABLE( VtxImageDialog, wxFrame )

EVT_CLOSE(VtxImageDialog::OnClose)

EVT_BUTTON(wxID_OK, VtxImageDialog::OnOk)
EVT_BUTTON(ID_SAVE, VtxImageDialog::OnSave)
EVT_BUTTON(ID_REVERT, VtxImageDialog::OnRevert)
EVT_BUTTON(ID_DELETE, VtxImageDialog::OnDelete)
EVT_NOTEBOOK_PAGE_CHANGED(ID_SWITCH,VtxImageDialog::OnTabSwitch)

END_EVENT_TABLE()


VtxImageDialog::VtxImageDialog( wxWindow* parent,
  wxWindowID id, const wxString& caption,
  const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos,wxSize(DLG_WIDTH,DLG_HEIGHT), style);
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
    m_type=TYPE_BANDS;
	wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    m_save = new wxButton(this,ID_SAVE,"Save",wxDefaultPosition,BTNSIZE);
    m_revert = new wxButton(this,ID_REVERT,"Revert",wxDefaultPosition,BTNSIZE);
    m_delete = new wxButton(this,ID_DELETE,"Delete",wxDefaultPosition,BTNSIZE);
    buttons->Add(m_save, 0, wxALIGN_LEFT|wxALL,0);
    buttons->Add(m_revert, 0, wxALIGN_LEFT|wxALL,0);
    buttons->Add(m_delete, 0, wxALIGN_LEFT|wxALL,0);

    m_tabs=new wxNotebook(this,ID_SWITCH,wxPoint(0,0),wxSize(DLG_WIDTH,DLG_HEIGHT));
	m_image_tabs=new VtxImageTabs(m_tabs,wxID_ANY);
    m_import_tabs=new VtxImportTabs(m_tabs,wxID_ANY);
    m_bands_tabs=new VtxBandsTabs(m_tabs,wxID_ANY);
 	m_tabs->AddPage(m_bands_tabs,wxT("1D"),true);
 	m_tabs->AddPage(m_image_tabs,wxT("2D"),false);
 	m_tabs->AddPage(m_import_tabs,wxT("Bitmaps"),false);
	topSizer->Add(m_tabs,0,wxALIGN_LEFT|wxALL);


    topSizer->Add(buttons,0,wxALIGN_LEFT|wxALL);
    SetSizerAndFit(topSizer);

    Centre();
	//m_tabs->Show(true);

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
		m_image_tabs->updateControls();
		m_bands_tabs->updateControls();
		m_import_tabs->updateControls();
		UpdateControls();
	}
	return wxFrame::Show(b);
}

bool VtxImageDialog::Show(wxString name, int type){
	m_type=type;
	m_tabs->SetSelection(m_type);
	switch(m_type){
	case TYPE_IMAGE:
		m_image_tabs->setSelection(name);
		break;
	case TYPE_BANDS:
		m_bands_tabs->setSelection(name);
		break;
	case TYPE_IMPORT:
		m_import_tabs->setSelection(name);
		break;
	}
	return Show(true);
}

void VtxImageDialog::Invalidate(){
	m_image_tabs->Invalidate();
	m_bands_tabs->Invalidate();
	m_import_tabs->Invalidate();
}

void VtxImageDialog::OnOk(wxCommandEvent& event){
	Show(false);
}
void VtxImageDialog::OnTabSwitch(wxNotebookEvent &event){
	m_type=event.GetSelection();
	switch(m_type){
	case TYPE_IMAGE:
		m_revert->Enable(m_image_tabs->canRevert());
		m_delete->Enable(m_image_tabs->canDelete());
		m_save->Enable(m_image_tabs->canSave());
		break;
	case TYPE_BANDS:
		m_revert->Enable(m_bands_tabs->canRevert());
		m_delete->Enable(m_bands_tabs->canDelete());
		m_save->Enable(m_bands_tabs->canSave());
		break;
	case TYPE_IMPORT:
		m_revert->Enable(m_import_tabs->canRevert());
		m_delete->Enable(m_import_tabs->canDelete());
		m_save->Enable(m_import_tabs->canSave());
		break;
	}

}
void VtxImageDialog::OnSave(wxCommandEvent &event){
	switch(m_type){
	case TYPE_IMAGE:
		m_image_tabs->Save();
		break;
	case TYPE_BANDS:
		m_bands_tabs->Save();
		break;
	case TYPE_IMPORT:
		m_import_tabs->Save();
		break;
	}
}
void VtxImageDialog::OnRevert(wxCommandEvent &event){
	switch(m_type){
	case TYPE_IMAGE:
		m_image_tabs->Revert();
		break;
	case TYPE_BANDS:
		m_bands_tabs->Revert();
		break;
	case TYPE_IMPORT:
		m_import_tabs->Revert();
		break;
	}
}
void VtxImageDialog::OnDelete(wxCommandEvent &event){
	switch(m_type){
	case TYPE_IMAGE:
		m_image_tabs->Delete();
		break;
	case TYPE_BANDS:
		m_bands_tabs->Delete();
		break;
	case TYPE_IMPORT:
		m_import_tabs->Delete();
		break;
	}
}
void VtxImageDialog::UpdateControls(){
	switch(m_type){
	case TYPE_IMAGE:
		m_revert->Enable(m_image_tabs->canRevert());
		m_delete->Enable(m_image_tabs->canDelete());
		m_save->Enable(m_image_tabs->canSave());
		break;
	case TYPE_BANDS:
		m_revert->Enable(m_bands_tabs->canRevert());
		m_delete->Enable(m_bands_tabs->canDelete());
		m_save->Enable(m_bands_tabs->canSave());
		break;
	case TYPE_IMPORT:
		m_revert->Enable(m_import_tabs->canRevert());
		m_delete->Enable(m_import_tabs->canDelete());
		m_save->Enable(m_import_tabs->canSave());
		break;
	}
	Refresh();
}
