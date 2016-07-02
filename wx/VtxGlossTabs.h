#ifndef GLOSSTABS_H_
#define GLOSSTABS_H_

#include "VtxTabsMgr.h"
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <TerrainClass.h>
#include <defs.h>


class VtxGlossTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxGlossTabs)

protected:
	TNgloss *object() 	{ return (TNgloss *)object_node->node;}
	void AddLightingTab(wxWindow *panel);
    void invalidateObject(){
    	object()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();

	SliderCtrl *ShineSlider;
	SliderCtrl *AlbedoSlider;
public:
	VtxGlossTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);

	bool Create(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);

	~VtxGlossTabs(){
		delete ShineSlider;
		delete AlbedoSlider;
	}
	void updateControls();
	int showMenu(bool);

	DEFINE_SLIDER_EVENTS(Shine)
	DEFINE_SLIDER_EVENTS(Albedo)

	DECLARE_EVENT_TABLE()
};
#endif /*GLOSSTABS_H_*/
