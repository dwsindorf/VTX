/*
 * VtxSpritesTabs.cpp
 *
 *  Created on: Jul 10, 2023
 *      Author: deans
 */

#include "VtxSpritesTabs.h"
#include "VtxSceneDialog.h"
#include "VtxImageDialog.h"

#include <wx/dir.h>

wxString selections[]={"0","1","2","3","4","5","6","7","8"};

//########################### VtxSpritesTabs Class ########################

enum{
	OBJ_SHOW,
	OBJ_DELETE,
	OBJ_SAVE,
    ID_FILELIST,
    ID_SIZE_SLDR,
    ID_SIZE_TEXT,
    ID_DELTA_SIZE_SLDR,
    ID_DELTA_SIZE_TEXT,
    ID_LEVELS,
    ID_LEVELS_SIZE_SLDR,
    ID_LEVELS_SIZE_TEXT,
    ID_DENSITY_SLDR,
    ID_DENSITY_TEXT,

    ID_SLOPE_BIAS_SLDR,
    ID_SLOPE_BIAS_TEXT,
    ID_PHI_BIAS_SLDR,
    ID_PHI_BIAS_TEXT,
    ID_HT_BIAS_SLDR,
    ID_HT_BIAS_TEXT,
	ID_SPRITES_DIM,
	ID_SPRITES_VIEW,
};

IMPLEMENT_CLASS(VtxSpritesTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxSpritesTabs, VtxTabsMgr)

EVT_MENU(OBJ_DELETE,VtxSpritesTabs::OnDelete)
EVT_MENU(OBJ_SAVE,VtxSpritesTabs::OnSave)
EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxSpritesTabs::OnAddItem)
EVT_CHOICE(ID_SPRITES_DIM,VtxSpritesTabs::OnDimSelect)
EVT_CHOICE(ID_SPRITES_VIEW,VtxSpritesTabs::OnViewSelect)
EVT_CHOICE(ID_FILELIST,VtxSpritesTabs::OnChangedFile)

END_EVENT_TABLE()

VtxSpritesTabs::VtxSpritesTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxSpritesTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 
    image_path="";
    changed_cell_expr=true;
    image_dim=0;
    image_name="";
    sprites_file="";
        
	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	AddImageTab(page);
    AddPage(page,wxT("Image"),true);

    page=new wxPanel(this,wxID_ANY);
    AddMappingTab(page);
    AddPage(page,wxT("Mapping"),false);

    return true;
}
void VtxSpritesTabs::AddMappingTab(wxWindow *panel){
}
void VtxSpritesTabs::AddImageTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxStaticBoxSizer* image_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Sprites"));
    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *lbl=new wxStaticText(panel,-1,"File",wxDefaultPosition,wxSize(25,-1));
	hline->Add(lbl, 0, wxALIGN_LEFT|wxALL, 8);
	//hline->AddSpacer(5);

    choices=new wxChoice(panel,ID_FILELIST,wxPoint(-1,4),wxSize(130,-1));
    choices->SetSelection(0);

	hline->Add(choices,0,wxALIGN_LEFT|wxALL,2);

	wxString offsets[]={"1x","4x","9x"};

	sprites_dim=new wxChoice(panel, ID_SPRITES_DIM, wxDefaultPosition,wxSize(55,-1),2, offsets);
	sprites_dim->SetSelection(1);
	hline->Add(sprites_dim,0,wxALIGN_LEFT|wxALL,0);

	lbl=new wxStaticText(panel,-1,"View",wxDefaultPosition,wxSize(25,-1));
	hline->Add(lbl, 0, wxALIGN_LEFT|wxALL, 8);

	select=new wxChoice(panel, ID_SPRITES_VIEW, wxDefaultPosition,wxSize(55,-1),2, selections);
	select->SetSelection(0);

	//select=new wxChoice(panel, ID_SPRITES_VIEW, wxDefaultPosition,wxSize(55,-1));
	//select->SetSelection(0);

	//type_expr = new ExprTextCtrl(panel,ID_TYPE_EXPR,"",0,TABS_WIDTH-60-TABS_BORDER);
	hline->Add(select,0,wxALIGN_LEFT|wxALL,2);

	image_cntrls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(image_cntrls,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	image_window = new VtxBitmapPanel(panel,wxID_ANY,wxDefaultPosition,wxSize(200,200));
	hline->Add(image_window, 0, wxALIGN_LEFT|wxALL, 0);
	cell_window = new VtxBitmapPanel(panel,wxID_ANY,wxDefaultPosition,wxSize(200,200));
	hline->AddSpacer(10);

	hline->Add(cell_window, 0, wxALIGN_LEFT|wxALL, 0);
	image_sizer=new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Images"));
	image_sizer->Add(hline, 0, wxALIGN_LEFT|wxALL,2);
	boxSizer->Add(image_sizer,0,wxALIGN_LEFT|wxALL,0);
}

int VtxSpritesTabs::showMenu(bool expanded){

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

void VtxSpritesTabs::setImagePanel(){
	char sdir[256];
	object()->getSpritesFilePath((char*)image_name.ToAscii(),image_dim,sdir);
	strcat(sdir,".png");
	wxString path(sdir);
	if(sdir!=image_path){
		image_window->setScaledImage(sdir,wxBITMAP_TYPE_PNG);
		cell_window->setName((char*)path.ToAscii());
		cout<<"setting image panel:"<<sdir<<endl;
		changed_cell_expr=true;
		image_path=sdir;
	}
}

void VtxSpritesTabs::setViewPanel(){
	if(image_window->imageOk()&&changed_cell_expr){
		GLuint dim=sprites_dim->GetSelection()+1;
		int cell=select->GetCurrentSelection();
		int y=cell/dim;
		int x=cell-y*dim;
		//cout<<"cell:"<<cell<<" x:"<<x<<" y:"<<y<<endl;

		wxRect r(x,y,dim,dim);
		cell_window->setSubImage(r,wxBITMAP_TYPE_PNG);
		cell_window->scaleImage();
		cout<<"setting type panel:"<<cell_window->getName()<<endl;

		changed_cell_expr=false;
	}
}
void VtxSpritesTabs::OnDimSelect(wxCommandEvent& event){
	int dim=sprites_dim->GetSelection();
	dim+=1;
	//cout<<"VtxSpritesTabs::OnDimSelect "<<dim<<endl;
	makeFileList(dim,"");
	select->Set(dim*dim,selections);
	select->SetSelection(0);
	getObjAttributes();
}

void VtxSpritesTabs::makeFileList(int dim,wxString name){
	char sdir[512];
   // cout<<"VtxSpritesTabs::makeFileList "<<name<<" "<<dim<<endl;
	object()->getSpritesDir(dim,sdir);

 	wxDir dir(sdir);
 	if ( !dir.IsOpened() )
 	{
 	    // deal with the error here - wxDir would already log an error message
 	    // explaining the exact reason of the failure
 	    return;
 	}
 	image_name=name;
 	if(dim != image_dim){
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
		image_dim=dim;
		select->Set(dim*dim,selections);
		if(image_name.IsEmpty())
			choices->SetSelection(0);
		update_needed=true;
 	}
 	if(image_name.IsEmpty())
 		image_name=choices->GetStringSelection();

 	choices->SetStringSelection(image_name);
 	select->SetSelection(0);
 	setObjAttributes();
}

void VtxSpritesTabs::getObjAttributes(){
	TNsprite *obj=object();
	uint dim=0;
	sprites_file=obj->getSpritesFile(dim);
	cout<<"VtxSpritesTabs::getObjAttributes file:"<<sprites_file<<" dim:"<<dim<<endl;

	sprites_dim->SetSelection(dim-1);
	makeFileList(dim,sprites_file);
	choices->SetStringSelection(sprites_file);
	setImagePanel();
	setViewPanel();
	update_needed=false;

}
void VtxSpritesTabs::setObjAttributes(){
	cout<<"VtxSpritesTabs::setObjAttributes file:"<<sprites_file<<" dim:"<<image_dim<<endl;

	if(!update_needed)
		return;
	wxString str=choices->GetStringSelection();
	if(str!=wxEmptyString){
		object()->setSpritesImage((char*)str.ToAscii());
		setImagePanel();
		setViewPanel();
	}
	update_needed=false;
}

void VtxSpritesTabs::updateControls(){
	cout<<"VtxSpritesTabs::updateControls "<<update_needed<<endl;
	if(update_needed){
		getObjAttributes();
	}
}

void VtxSpritesTabs::OnChangedFile(wxCommandEvent& event){
	update_needed=true;
	changed_cell_expr=true;
	image_path="";
	image_name=choices->GetStringSelection();
	invalidateObject();
	
}
void VtxSpritesTabs::OnViewSelect(wxCommandEvent& event){
	changed_cell_expr=true;
	setViewPanel();
}

