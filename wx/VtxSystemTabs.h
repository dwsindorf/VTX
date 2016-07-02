#ifndef SYSTEM_TABS_H_
#define SYSTEM_TABS_H_


#include "VtxTabsMgr.h"
#include "SceneClass.h"

class VtxSystemTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxSystemTabs)
protected:
	void AddDisplayTab(wxWindow *panel);
	SliderCtrl *SizeSlider;
	System *object() 		{ return ((System*)(object_node->node));}

public:
	VtxSystemTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxSystemTabs(){
		delete SizeSlider;
	}
	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);

	int showMenu(bool);
	void updateControls();

    void OnViewObj(wxCommandEvent& event);
    void OnUpdateViewObj(wxUpdateUIEvent& event);

    DEFINE_SLIDER_VAR_EVENTS(Size,object()->size)

	DECLARE_EVENT_TABLE()
};

#endif /*SYSTEM_TABS_H_*/
