
#include "VtxSnowTabs.h"

#define LABEL_WIDTH 40
#define VALUE_WIDTH 60
#define SLIDER_WIDTH 90
#define LINE_WIDTH TABS_WIDTH-2*TABS_BORDER

//########################### VtxSnowTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
    ID_HEIGHT_SLDR,
    ID_HEIGHT_TEXT,
    ID_BUMP_SLDR,
    ID_BUMP_TEXT,
    ID_SLOPE_SLDR,
    ID_SLOPE_TEXT,
    ID_THRESH_SLDR,
    ID_THRESH_TEXT,
    ID_DEPTH_SLDR,
    ID_DEPTH_TEXT,
    ID_LAT_SLDR,
    ID_LAT_TEXT,
};


IMPLEMENT_CLASS(VtxSnowTabs, VtxTabsMgr )

BEGIN_EVENT_TABLE(VtxSnowTabs, VtxTabsMgr)

SET_SLIDER_EVENTS(HEIGHT,VtxSnowTabs,Height)
SET_SLIDER_EVENTS(BUMP,VtxSnowTabs,Bump)
SET_SLIDER_EVENTS(DEPTH,VtxSnowTabs,Depth)
SET_SLIDER_EVENTS(SLOPE,VtxSnowTabs,Slope)
SET_SLIDER_EVENTS(THRESH,VtxSnowTabs,Thresh)
SET_SLIDER_EVENTS(LAT,VtxSnowTabs,Lat)

EVT_MENU(ID_DELETE,VtxSnowTabs::OnDelete)
EVT_MENU(ID_ENABLE,VtxSnowTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxSnowTabs::OnUpdateEnable)

END_EVENT_TABLE()

VtxSnowTabs::VtxSnowTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{

	Create(parent, id, pos,size, style, name);
}

bool VtxSnowTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;
 	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
 	AddControlsTab(page);
    AddPage(page,wxT("Properties"),true);

    return true;
}

void VtxSnowTabs::AddControlsTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* box = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Snow"));

    wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

    ThreshSlider=new SliderCtrl(panel,ID_THRESH_SLDR,"Level",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH);
    ThreshSlider->setRange(-4,4.0);
    hline->Add(ThreshSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    DepthSlider=new SliderCtrl(panel,ID_DEPTH_SLDR,"Depth",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH);
    DepthSlider->setRange(0.0,1.0);
    hline->Add(DepthSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
    hline->SetMinSize(wxSize(LINE_WIDTH,-1));

    box->Add(hline,0,wxALIGN_LEFT|wxALL,0);
    boxSizer->Add(box,0,wxALIGN_LEFT|wxALL,0);

    box = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Bias"));

    hline = new wxBoxSizer(wxHORIZONTAL);

    LatSlider=new SliderCtrl(panel,ID_LAT_SLDR,"Lat",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH);
    LatSlider->setRange(0.0,1.0);
    hline->Add(LatSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    SlopeSlider=new SliderCtrl(panel,ID_SLOPE_SLDR,"Slope",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH);
    SlopeSlider->setRange(-1,1);
    hline->Add(SlopeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    hline->SetMinSize(wxSize(LINE_WIDTH,-1));
    box->Add(hline,0,wxALIGN_LEFT|wxALL,0);

    hline = new wxBoxSizer(wxHORIZONTAL);

    HeightSlider=new SliderCtrl(panel,ID_HEIGHT_SLDR,"Ht",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH);
    HeightSlider->setRange(-4,4.0);
    hline->Add(HeightSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    BumpSlider=new SliderCtrl(panel,ID_BUMP_SLDR,"Bump",LABEL_WIDTH, VALUE_WIDTH,SLIDER_WIDTH);
    BumpSlider->setRange(-1,1.0);
    hline->Add(BumpSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
    
    hline->SetMinSize(wxSize(LINE_WIDTH,-1));
    box->Add(hline,0,wxALIGN_LEFT|wxALL,0);
    boxSizer->Add(box,0,wxALIGN_LEFT|wxALL,0);
}

int VtxSnowTabs::showMenu(bool expanded){

	menu_action=TABS_NONE;
	wxMenu menu;

	menu.AppendCheckItem(ID_ENABLE,wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE,wxT("Delete"));

	PopupMenu(&menu);
	return menu_action;
}

void VtxSnowTabs::updateControls(){
	if(update_needed){
		getObjAttributes();
	}
}
//-------------------------------------------------------------
// VtxSnowTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxSnowTabs::setObjAttributes(){
	update_needed=true;
	TNsnow *tnode=object();

	object()->invalidate();

	wxString str="snow(";
	str+=ThreshSlider->getText();
	str+=",";
	str+=HeightSlider->getText();
	str+=",";
	str+=LatSlider->getText();
	str+=",";
	str+=SlopeSlider->getText();
	str+=",";
	str+=DepthSlider->getText();
	str+=",";
	str+=BumpSlider->getText();
	str+=")\n";

	tnode->setExpr((char*)str.ToAscii());
	if(tnode->getExprNode()==0)
		update_needed=true;
	else{
		update_needed=false;
		tnode->applyExpr();
        //TheView->set_changed_detail();
        //TheScene->rebuild();
	}
}

//-------------------------------------------------------------
// VtxSnowTabs::getObjAttributes() when switched in
//-------------------------------------------------------------
void VtxSnowTabs::getObjAttributes(){
	if(!update_needed)
		return;
	TNsnow *tnode=object();

	double args[6];

	getargs((TNarg*)tnode->left,args,6);
	ThreshSlider->setValue(args[0]);
	HeightSlider->setValue(args[1]);
	LatSlider->setValue(args[2]);
	SlopeSlider->setValue(args[3]);
	DepthSlider->setValue(args[4]);
	BumpSlider->setValue(args[5]);

	update_needed=false;
}
