#include "VtxFunctDialog.h"
#include "VtxExprEdit.h"

#include "VtxControls.h"
#include "VtxNoiseFunct.h"
#include "VtxCratersFunct.h"
#include "VtxFractalFunct.h"
#include "VtxErodeFunct.h"

#include "SceneClass.h"
#include "UniverseModel.h"

#define PAGE_WIDTH TABS_WIDTH+5

//########################### VtxNoEditFunct Class Methods #########################

VtxNoEditFunct::VtxNoEditFunct(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxFunctMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxNoEditFunct::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxFunctMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *panel=new wxPanel(this,wxID_ANY);

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);

    boxSizer->Add(new wxStaticText(panel,-1,"Nothing is currently being edited"), 0, wxALIGN_LEFT|wxALL, 4);
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER|wxALL, 5);
    AddPage(panel,wxT("Function"),true);
    Show(false);

    return true;
}

// define all resource ids here
enum {
	ID_NOEDIT,
    ID_SAVE,
    ID_REVERT,
    ID_DELETE,
    ID_SYMBOLS,
    ID_AUTO_APPLY,
    ID_AUTO_EDIT,
 };


//########################### VtxFunctDialog Class Methods #############################

bool VtxFunctDialog::show_symbols=true;
bool VtxFunctDialog::auto_apply=true;
bool VtxFunctDialog::auto_edit=true;

IMPLEMENT_CLASS( VtxFunctDialog, wxFrame )

BEGIN_EVENT_TABLE( VtxFunctDialog, wxFrame )

EVT_CHECKBOX(ID_SYMBOLS,VtxFunctDialog::OnSymbols)
//EVT_CHECKBOX(ID_AUTO_APPLY,VtxFunctDialog::OnAutoApply)
//EVT_CHECKBOX(ID_AUTO_EDIT,VtxFunctDialog::OnAutoEdit)
EVT_UPDATE_UI(ID_SYMBOLS, VtxFunctDialog::OnUpdateSymbols)
//EVT_UPDATE_UI(ID_AUTO_APPLY, VtxFunctDialog::OnUpdateAutoApply)
//EVT_UPDATE_UI(ID_AUTO_EDIT, VtxFunctDialog::OnUpdateAutoEdit)
//EVT_CHECKBOX(ID_AUTO_EDIT,VtxFunctDialog::OnAutoEdit)

EVT_CLOSE(VtxFunctDialog::OnClose)

END_EVENT_TABLE()


VtxFunctDialog::VtxFunctDialog( wxWindow* parent,
  wxWindowID id, const wxString& caption,
  const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos,wxSize(DLG_WIDTH,DLG_HEIGHT), style);
}


bool VtxFunctDialog::Create( wxWindow* parent,
  wxWindowID id, const wxString& title,
  const wxPoint& pos, const wxSize& size, long style )
{

    if (!wxFrame::Create( parent, id, wxT("Function Editor"),
    		pos, size,  wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER  | wxMAXIMIZE_BOX)))
        return false;

    editor=0;

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

    controlsSizer = new wxBoxSizer(wxVERTICAL);

    topSizer->Add(controlsSizer,0,wxALIGN_LEFT|wxALL);

    functs[ID_NOEDIT]=new VtxNoEditFunct(this,wxID_ANY,wxPoint(0,0),wxSize(PAGE_WIDTH,TABS_HEIGHT));
    functs[ID_CRATERS]=new VtxCratersFunct(this,wxID_ANY,wxPoint(0,0),wxSize(PAGE_WIDTH,TABS_HEIGHT));
    functs[ID_FCHNL]=new VtxFractalFunct(this,wxID_ANY,wxPoint(0,0),wxSize(PAGE_WIDTH,TABS_HEIGHT));
    functs[ID_NOISE]=new VtxNoiseFunct(this,wxID_ANY,wxPoint(0,0),wxSize(PAGE_WIDTH,TABS_HEIGHT));
    functs[ID_ERODE]=new VtxErodeFunct(this,wxID_ANY,wxPoint(0,0),wxSize(PAGE_WIDTH,TABS_HEIGHT));

    controlsSizer->Add(functs[ID_NOEDIT],0,wxALIGN_LEFT|wxALL);

    currentFunct=lastFunct=ID_NOEDIT;

    functs[ID_NOEDIT]->Show(true);
    SetSizerAndFit(topSizer);
    Centre();
    int char_width=10;
//    wxString edit_str("AutoEdit");
//    wxString apply_str("AutoSave");
    wxString symbols_str("ShowTokens");
//    int edit_pos=edit_str.Length()*char_width;
//    int apply_pos=edit_pos+apply_str.Length()*char_width;
    int symbols_pos=symbols_str.Length()*char_width+10;
//    m_auto_edit=new wxCheckBox(this, ID_AUTO_EDIT, edit_str,wxPoint(PAGE_WIDTH-edit_pos,5),wxSize(-1,-1));
//    m_auto_edit->SetValue(true);
//    m_auto_apply=new wxCheckBox(this, ID_AUTO_APPLY, apply_str,wxPoint(PAGE_WIDTH-apply_pos,5),wxSize(-1,-1));
//    m_auto_apply->SetValue(true);
    m_show_symbols=new wxCheckBox(this, ID_SYMBOLS, symbols_str,wxPoint(PAGE_WIDTH-symbols_pos,5),wxSize(-1,-1));
    position=GetScreenPosition();
    Move(position.x+size.GetWidth()+100,position.y+100);
    position=GetScreenPosition();
    return true;
}

bool VtxFunctDialog::setEditor(VtxExprEdit *e){
	editor = e;
	return true;
}
void VtxFunctDialog::getFunct(wxString val){
	//cout << "VtxFunctDialog::getFunct:"<<val<<endl;
	if(editor){
		editor->setToken(symbol,val);
	}

}
bool VtxFunctDialog::setFunct(wxString token, wxString val){
	TNode *tn=(TNode*)TheScene->parse_node(val.ToAscii());
	if(!tn){
		cout << "function expression parse failed : "<< val.ToAscii() << endl;
		return false;
	}
	int t=tn->typeValue();
	if(functs.find(t) == functs.end()){
		cout << "dialog for id : "<< t << " not yet supported" <<endl;
		return false;
	}
	symbol=token;
	value=val;
	VtxFunctMgr* mgr=functs[t];
	mgr->setFunction(value);
	if(t != currentFunct){
		if(lastFunct>=0){
			controlsSizer->Detach(0);
			if(functs.find(lastFunct) != functs.end())
				functs[lastFunct]->Show(false);
		}
		mgr->Show(true);
		controlsSizer->Add(mgr,0,wxALIGN_LEFT|wxALL);
		lastFunct=t;
		currentFunct=t;
		controlsSizer->Layout();
	}
	Raise();
	delete tn;
	return true;
}

void VtxFunctDialog::clear(){
	editor=0;
	int t=ID_NOEDIT;
	VtxFunctMgr* mgr=functs[t];
	mgr->setFunction(value);
	if(t != currentFunct){
		if(lastFunct>=0){
			controlsSizer->Detach(0);
			if(functs.find(lastFunct) != functs.end())
				functs[lastFunct]->Show(false);
		}
		mgr->Show(true);
		controlsSizer->Add(mgr,0,wxALIGN_LEFT|wxALL);
		lastFunct=t;
		currentFunct=t;
		controlsSizer->Layout();
	}
}

//void VtxFunctDialog::OnAutoApply(wxCommandEvent& event){
//	auto_apply=auto_apply?false:true;
//}
//void VtxFunctDialog::OnUpdateAutoApply(wxUpdateUIEvent& event){
//	event.Check(auto_apply);
//}
//
//void VtxFunctDialog::OnAutoEdit(wxCommandEvent& event){
//	auto_edit=auto_edit?false:true;
//}
//void VtxFunctDialog::OnUpdateAutoEdit(wxUpdateUIEvent& event){
//	event.Check(auto_edit);
//}

void VtxFunctDialog::OnUpdateSymbols(wxUpdateUIEvent& event){
	event.Check(show_symbols);
}
void VtxFunctDialog::OnSymbols(wxCommandEvent& event){
	show_symbols=show_symbols?false:true;
	if(editor)
		editor->update();
}
void VtxFunctDialog::OnClose(wxCloseEvent& event){
	event.Veto(true);
	Show(false);
}
bool VtxFunctDialog::Show(const bool b){
	if(b)
		Move(position.x,position.y);
	else{
		position=GetScreenPosition();
		// for some reason can't restore position after a move unless
		// we change some position attribute manually
		position.x-=1;
	}
	if(b){
	}
	return wxFrame::Show(b);
}
