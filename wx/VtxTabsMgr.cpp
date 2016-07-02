#include "VtxTabsMgr.h"
#include "VtxSceneDialog.h"

IMPLEMENT_CLASS( VtxTabsMgr, wxNotebook )

VtxTabsMgr::VtxTabsMgr(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
{
	menu_action=TABS_NONE;

	object_node=0;
	object_name=0;
	object_type=0;
  	update_needed=true;
	Show(false);
}

void VtxTabsMgr::OnNameText(wxCommandEvent& event){
	char *s=object_name->GetValue().ToAscii();
	sceneDialog->setNodeName(s);
}
void VtxTabsMgr::OnEnable(wxCommandEvent& event){
    TheView->set_changed_detail();
	setEnabled(!isEnabled());
	menu_action=TABS_ENABLE;
	sceneDialog->updateObjectTree();
	TheScene->rebuild_all();
}


wxMenu *VtxTabsMgr::getAddMenu(NodeIF *n){
	return sceneDialog->getAddMenu(n);
}

wxMenu *VtxTabsMgr::getRemoveMenu(NodeIF *n){
	return sceneDialog->getRemoveMenu(n);
}

void VtxTabsMgr::getObjAttributes(){}
void VtxTabsMgr::setObjAttributes(){}
void VtxTabsMgr::invalidateObject(){
	setObjAttributes();
	TheView->set_changed_detail();
	TheScene->rebuild();
}

void VtxTabsMgr::invalidateRender(){
	setObjAttributes();
	TheView->set_changed_render();
}

void VtxTabsMgr::invalidate(){
	invalidateRender();
}
