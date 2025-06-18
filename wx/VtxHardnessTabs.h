#ifndef HARDNESSTABS_H_
#define HARDNESSTABS_H_

#include "VtxTabsMgr.h"
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <TerrainClass.h>
#include <Erode.h>
#include <defs.h>


class VtxHardnessTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxHardnessTabs)

protected:
	TNhardness *object() 	{ return (TNhardness *)object_node->node;}
	void AddPropsTab(wxWindow *panel);
    void invalidateObject(){
    	object()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_detail();
    	//TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();

	SliderCtrl *ValueSlider;
public:
	VtxHardnessTabs(wxWindow* parent,
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

	~VtxHardnessTabs(){
		delete ValueSlider;
	}
	void updateControls();
	int showMenu(bool);

	DEFINE_SLIDER_EVENTS(Value)

	DECLARE_EVENT_TABLE()
};
#endif /*HARDNESSTABS_H_*/
