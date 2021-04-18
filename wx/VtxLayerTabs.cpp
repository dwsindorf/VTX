
#include "VtxLayerTabs.h"
#include "VtxSceneDialog.h"

//########################### VtxLayerTabs Class ########################
enum{
	OBJ_SHOW,
	OBJ_DELETE,
	OBJ_SAVE,
	ID_NAME_TEXT,

    ID_MORPH_SLDR,
    ID_MORPH_TEXT,
    ID_WIDTH_SLDR,
    ID_WIDTH_TEXT,
    ID_DROP_SLDR,
    ID_DROP_TEXT,
    ID_RAMP_SLDR,
    ID_RAMP_TEXT,
    ID_SQR,
};

#define SLDR_WIDTH  180
#define EXPR_WIDTH  200

#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 40

//########################### VtxLayerTabs Class ########################

IMPLEMENT_CLASS(VtxLayerTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxLayerTabs, VtxTabsMgr)

SET_SLIDER_EVENTS(WIDTH,VtxLayerTabs,Width)
SET_SLIDER_EVENTS(DROP,VtxLayerTabs,Drop)
SET_SLIDER_EVENTS(RAMP,VtxLayerTabs,Ramp)
SET_SLIDER_EVENTS(MORPH,VtxLayerTabs,Morph)

EVT_CHECKBOX(ID_SQR,VtxLayerTabs::OnChanged)
EVT_TEXT_ENTER(ID_NAME_TEXT,VtxLayerTabs::OnNameText)

EVT_MENU(OBJ_DELETE,VtxLayerTabs::OnDelete)
EVT_MENU(OBJ_SHOW,VtxLayerTabs::OnEnable)
//EVT_MENU(OBJ_SAVE,VtxLayerTabs::OnSave)
EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxLayerTabs::OnAddItem)

SET_FILE_EVENTS(VtxLayerTabs)

END_EVENT_TABLE()

VtxLayerTabs::VtxLayerTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxLayerTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	AddLayerTab(page);
    AddPage(page,wxT("Layer"),true);
    return true;
}

int VtxLayerTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(OBJ_SHOW,wxT("Show"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));

	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		//menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	sceneDialog->AddFileMenu(menu,object_node->node);

	PopupMenu(&menu);
	return menu_action;
}

void VtxLayerTabs::AddLayerTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBoxSizer* props = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Properties"));

	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Label",LABEL2+10,VALUE2+SLIDER2);
	props->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	props->AddSpacer(10);
    m_sqr_check=new wxCheckBox(panel, ID_SQR, "Square transition slope");
    props->Add(m_sqr_check,0,wxALIGN_LEFT|wxALL,0);
    props->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
    boxSizer->Add(props,0,wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* width = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Width"));

    WidthSlider=new ExprSliderCtrl(panel,ID_WIDTH_SLDR,"",0,EXPR_WIDTH,SLDR_WIDTH);
    WidthSlider->setRange(0,1.0);
    width->Add(WidthSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
    width->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(width, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* morph = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Morph"));
    MorphSlider=new ExprSliderCtrl(panel,ID_MORPH_SLDR,"",0,EXPR_WIDTH,SLDR_WIDTH);
    MorphSlider->setRange(0,1.0);
    morph->Add(MorphSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    morph->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(morph, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* drop = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Drop"));
    DropSlider=new ExprSliderCtrl(panel,ID_DROP_SLDR,"",0,EXPR_WIDTH,SLDR_WIDTH);
    DropSlider->setRange(0,1.0);
    drop->Add(DropSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    drop->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(drop, 0, wxALIGN_LEFT|wxALL,0);

    wxStaticBoxSizer* ramp = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Ramp"));
    RampSlider=new ExprSliderCtrl(panel,ID_RAMP_SLDR,"",0,EXPR_WIDTH,SLDR_WIDTH);
    RampSlider->setRange(0,1.0);
    ramp->Add(RampSlider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    ramp->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(ramp, 0, wxALIGN_LEFT|wxALL,0);


}

void VtxLayerTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}

//-------------------------------------------------------------
// VtxLayerTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxLayerTabs::setObjAttributes(){
	update_needed=true;
	TNlayer *tnode=object();

	wxString str="layer(";

	if(!object_name->GetValue().empty()){
		char name[128];
		sprintf(name,"\"%s\",",(const char*)object_name->GetValue().ToAscii());
		str+=name;
	}

	if(MorphSlider->getValue()>0)
		str+="MORPH";
	else
		str+="MESH";

	if(m_sqr_check->GetValue())
		str+="|FSQR";

	str+=","+MorphSlider->getText();
	str+=","+WidthSlider->getText();
	str+=","+DropSlider->getText();
	str+=","+RampSlider->getText();

	str+=")";
	str+="\n";

	char p[256];
	strcpy(p,str.ToAscii());
	cout << p << endl;
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
// VtxLayerTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxLayerTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNlayer *tnode=object();
	char *name=object_node->node->nodeName();
	if(name && strlen(name))
		object_name->SetValue(name);
	else
		object_name->SetValue("");
	m_sqr_check->SetValue(tnode->type & FSQR);
	TNarg *arg=(TNarg*)tnode->left;
	WidthSlider->setValue(DFLT_WIDTH);
	DropSlider->setValue(DFLT_DROP);
	MorphSlider->setValue(0.0);
	RampSlider->setValue(0.0);
	if(arg){	// morph expr
		MorphSlider->setValue(arg);
		arg=arg->next();
		if(arg){  // width expr
			WidthSlider->setValue(arg);
			arg=arg->next();
			if(arg){ // drop expr
				DropSlider->setValue(arg);
				arg=arg->next();
				if(arg){ // ramp expr
					RampSlider->setValue(arg);
					arg->eval();
				}
			}
		}
	}
	update_needed=false;

}
void VtxLayerTabs::OnEnable(wxCommandEvent& event){
	VtxTabsMgr::OnEnable(event);
//	setEnabled(!isEnabled());
//    TheView->set_changed_detail();
//	menu_action=TABS_ENABLE;
//	sceneDialog->updateObjectTree();
	TheScene->rebuild_all();
}

void  VtxLayerTabs::OnDelete(wxCommandEvent& event){
 	menu_action=TABS_DELETE;
 	TheScene->rebuild_all();
}
