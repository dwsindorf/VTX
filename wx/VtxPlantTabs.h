#ifndef PLANTTABS_H_
#define PLANTTABS_H_

#include "VtxTabsMgr.h"
#include <Plants.h>
class VtxPlantTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxPlantTabs)

protected:
	TNplant *object() 	{ return ((TNplant *)object_node->node);}
	void AddDistribTab(wxWindow *panel);
	void getObjAttributes();
	void setObjAttributes();
	void invalidateObject(){
	    object()->invalidate();
	    setObjAttributes();
	    TheView->set_changed_render();
	}

	wxChoice *m_orders;
	SliderCtrl *SizeSlider;

	ExprSliderCtrl *DeltaSizeSlider;
	ExprSliderCtrl *LevelDeltaSlider;
	ExprSliderCtrl *DensitySlider;
	ExprSliderCtrl *SlopeBiasSlider;
	ExprSliderCtrl *PhiBiasSlider;
	ExprSliderCtrl *HtBiasSlider;

	SliderCtrl *DropSlider;
	
    wxString exprString();

    void OnChangedLevels(wxCommandEvent& event);


public:
	VtxPlantTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxPlantTabs(){}
	bool Create(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	void updateControls();
	int showMenu(bool);

	DEFINE_SLIDER_EVENTS(Size)
	DEFINE_SLIDER_EVENTS(DeltaSize)
	DEFINE_SLIDER_EVENTS(LevelDelta)
	DEFINE_SLIDER_EVENTS(Density)
	DEFINE_SLIDER_EVENTS(SlopeBias)
	DEFINE_SLIDER_EVENTS(PhiBias)
	DEFINE_SLIDER_EVENTS(HtBias)
	DEFINE_SLIDER_EVENTS(Drop)
	DECLARE_EVENT_TABLE()
};
#endif /*PLANTTABS_H_*/