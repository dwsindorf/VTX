#ifndef MAPTABS_H_
#define MAPTABS_H_

#include "VtxTabsMgr.h"
#include <Layers.h>
class VtxMapTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxMapTabs)

protected:
	TNmap *object() 	{ return (TNmap *)object_node->node;}
	void AddMapTab(wxWindow *panel);
    void invalidateObject(){
    	object()->invalidate();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();

	ExprSliderCtrl   *ScaleSlider;
	ExprSliderCtrl   *HeightSlider;
	ExprSliderCtrl   *BaseSlider;
	ExprSliderCtrl   *MarginSlider;

public:
	VtxMapTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxMapTabs(){
		delete ScaleSlider;
		delete HeightSlider;
		delete BaseSlider;
		delete MarginSlider;
	}
	bool Create(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	void updateControls();
	int showMenu(bool);
	void OnDelete(wxCommandEvent& event);	
	DEFINE_SLIDER_EVENTS(Height)
	DEFINE_SLIDER_EVENTS(Scale)
	DEFINE_SLIDER_EVENTS(Base)
	DEFINE_SLIDER_EVENTS(Margin)

	DECLARE_EVENT_TABLE()
};
#endif /*MAPTABS_H_*/
