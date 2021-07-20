#ifndef LAYERTABS_H_
#define LAYERTABS_H_

#include "VtxTabsMgr.h"
#include <Layers.h>

class VtxLayerTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxLayerTabs)

protected:
	TNlayer *object() 	{ return (TNlayer *)object_node->node;}
	void AddLayerTab(wxWindow *panel);
    void invalidateObject(){
    	object()->invalidate();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();
	ExprSliderCtrl   *MorphSlider;
	ExprSliderCtrl   *DropSlider;
	ExprSliderCtrl   *RampSlider;
	ExprSliderCtrl   *WidthSlider;
	wxCheckBox *m_sqr_check;
public:
	VtxLayerTabs(wxWindow* parent,
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

	~VtxLayerTabs(){
		delete MorphSlider;
		delete DropSlider;
		delete RampSlider;
		delete WidthSlider;
	}
	void updateControls();
	int showMenu(bool);
	
	DEFINE_SLIDER_EVENTS(Morph)
	DEFINE_SLIDER_EVENTS(Width)
	DEFINE_SLIDER_EVENTS(Drop)
	DEFINE_SLIDER_EVENTS(Ramp)

	void OnDelete(wxCommandEvent& event);
	void OnEnable(wxCommandEvent& event);
    void OnChanged(wxCommandEvent& event){
		setObjAttributes();
    }

	DECLARE_EVENT_TABLE()
};
#endif /*LAYERTABS_H_*/
