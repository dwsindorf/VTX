
#include "VtxMapTabs.h"
#include "VtxSceneDialog.h"
#include "AdaptOptions.h"
#include "RenderOptions.h"


#include <wx/filefn.h>
#include <wx/dir.h>

//########################### VtxMapTabs Class ########################
enum{
	OBJ_SHOW,
	OBJ_DELETE,
	OBJ_SAVE,

    ID_MERGE_MODE,
//    ID_TEXMERGE,

	ID_HEIGHT_SLDR,
    ID_HEIGHT_TEXT,
    ID_SCALE_SLDR,
    ID_SCALE_TEXT,
    ID_BASE_SLDR,
    ID_BASE_TEXT,
    ID_MARGIN_SLDR,
    ID_MARGIN_TEXT,

};

#define NAME_WIDTH  50
#define SLDR_WIDTH  180
#define EXPR_WIDTH  200

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 40

//########################### VtxMapTabs Class ########################

IMPLEMENT_CLASS(VtxMapTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxMapTabs, VtxTabsMgr)

SET_SLIDER_EVENTS(HEIGHT,VtxMapTabs,Height)
SET_SLIDER_EVENTS(SCALE,VtxMapTabs,Scale)
SET_SLIDER_EVENTS(BASE,VtxMapTabs,Base)
SET_SLIDER_EVENTS(MARGIN,VtxMapTabs,Margin)

EVT_MENU(OBJ_DELETE,VtxMapTabs::OnDelete)
EVT_MENU(OBJ_SHOW,VtxMapTabs::OnEnable)

EVT_RADIOBOX(ID_MERGE_MODE, VtxMapTabs::OnMergeMode)
EVT_UPDATE_UI(ID_MERGE_MODE, VtxMapTabs::OnUpdateMergeMode)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxMapTabs::OnAddItem)

SET_FILE_EVENTS(VtxMapTabs)

END_EVENT_TABLE()

VtxMapTabs::VtxMapTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxMapTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	AddMapTab(page);
    AddPage(page,wxT("Properties"),true);
    return true;
}

int VtxMapTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.Append(OBJ_DELETE,wxT("Delete"));

	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	sceneDialog->AddFileMenu(menu,object_node->node);

	PopupMenu(&menu);
	return menu_action;
}

void VtxMapTabs::AddMapTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* map_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Transfer Functions"));
    map_cntrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

    boxSizer->Add(map_cntrls, 0, wxALIGN_LEFT|wxALL,0);

	HeightSlider=new ExprSliderCtrl(panel,ID_HEIGHT_SLDR,"Height",NAME_WIDTH,EXPR_WIDTH,SLDR_WIDTH);
	HeightSlider->setRange(0,1.0);
	map_cntrls->Add(HeightSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	BaseSlider=new ExprSliderCtrl(panel,ID_BASE_SLDR,"Base",NAME_WIDTH,EXPR_WIDTH,SLDR_WIDTH);
	BaseSlider->setRange(-1.0,1.0);
	map_cntrls->Add(BaseSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	ScaleSlider=new ExprSliderCtrl(panel,ID_SCALE_SLDR,"Scale",NAME_WIDTH,EXPR_WIDTH,SLDR_WIDTH);
	ScaleSlider->setRange(0.0,1.0);
	map_cntrls->Add(ScaleSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

	MarginSlider=new ExprSliderCtrl(panel,ID_MARGIN_SLDR,"Margin",NAME_WIDTH,EXPR_WIDTH,SLDR_WIDTH);
	MarginSlider->setRange(0.0,4.0);

	map_cntrls->Add(MarginSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);

   // wxStaticBoxSizer* merge_cntrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Layer Intersection Mode"));

    wxString lmodes[]={"Overlap","Merge"};
    mergemode=new wxRadioBox(panel,ID_MERGE_MODE,wxT("Layer Intersection Mode"),wxPoint(-1,-1),wxSize(-1,-1),2,
    		lmodes,2,wxRA_SPECIFY_COLS);

    mergemode->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

    //merge_cntrls->Add(mergemode, 0, wxALIGN_LEFT|wxALL,0);

	topSizer->Add(mergemode, 0, wxALIGN_LEFT|wxALL, 5);
}

void VtxMapTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}


//-------------------------------------------------------------
// VtxMapTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxMapTabs::setObjAttributes(){
	update_needed=true;
	TNmap *tnode=object();

	wxString str="map(";

	str+=HeightSlider->getText();
	str+=","+BaseSlider->getText();
	str+=","+ScaleSlider->getText();
	str+=","+MarginSlider->getText();

	str+=")";
	str+="\n";

	char p[256];
	strcpy(p,str.ToAscii());
	tnode->setExpr(p);
	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
	}
	invalidateObject();
}
//-------------------------------------------------------------
// VtxMapTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxMapTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNmap *tnode=object();

	TNarg *arg=(TNarg*)tnode->left;

	HeightSlider->setValue(0.0);
	BaseSlider->setValue(0.0);
	ScaleSlider->setValue(0.0);
	MarginSlider->setValue(1.0);

	if(arg){	// height expr
		HeightSlider->setValue(arg);
		arg=arg->next();
		if(arg){  // base expr
			BaseSlider->setValue(arg);
			arg=arg->next();
			if(arg){ // scale expr
				ScaleSlider->setValue(arg);
				arg=arg->next();
				if(arg){ // scale expr
					MarginSlider->setValue(arg);
				}
			}
		}
	}
	update_needed=false;

}
void  VtxMapTabs::OnDelete(wxCommandEvent& event){
 	menu_action=TABS_DELETE;
 	TheScene->rebuild_all();
}

void VtxMapTabs::OnMergeMode(wxCommandEvent& event){
	int mode=event.GetSelection();
	Adapt.set_mindcnt(mode);
	TheScene->rebuild_all();
}
void VtxMapTabs::OnUpdateMergeMode(wxUpdateUIEvent& event){
	int mode=Adapt.mindcnt();
	mergemode->SetSelection(mode);
}
