
#include "VtxDefaultTabs.h"
#include "RenderOptions.h"
#include <GLSLMgr.h>

//########################### VtxDefaultTabs Class ########################

// define all resource ids here
enum {
};

IMPLEMENT_CLASS(VtxDefaultTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxDefaultTabs, wxNotebook)
EVT_MENU(TABS_ENABLE,VtxDefaultTabs::OnEnable)
EVT_UPDATE_UI(TABS_ENABLE,VtxDefaultTabs::OnUpdateEnable)

END_EVENT_TABLE()

VtxDefaultTabs::VtxDefaultTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxDefaultTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);

    AddDisplayTab(page);
    AddPage(page,wxT("Controls"),true);

    return true;
}

int VtxDefaultTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;

	wxMenu menu;
	menu.AppendCheckItem(TABS_ENABLE,wxT("Enable"));
	PopupMenu(&menu);
	return menu_action;
}

void VtxDefaultTabs::AddDisplayTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);

    boxSizer->Add(new wxStaticText(panel,-1,"Controls Not Yet Implemented"), 0, wxALIGN_LEFT|wxALL, 4);
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

}

