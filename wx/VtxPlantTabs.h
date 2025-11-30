#ifndef PLANTTABS_H_
#define PLANTTABS_H_

#include "VtxTabsMgr.h"
#include <Plants.h>
#include "TerrainClass.h"
class VtxPlantTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxPlantTabs)

protected:
	TNplant *object() 	{ return ((TNplant *)object_node->node);}
	void getDisplayState();
	void AddDistribTab(wxWindow *panel);
	void getObjAttributes();
	void setObjAttributes();
	void invalidateObject(){
	    //object()->invalidate();
	    setObjAttributes();
	    TheView->set_changed_render();
	}

	wxChoice *m_orders;
	SliderCtrl *SizeSlider;

	ExprSliderCtrl *DeltaSizeSlider;
	ExprSliderCtrl *DensitySlider;
	ExprSliderCtrl *SlopeBiasSlider;
	ExprSliderCtrl *PhiBiasSlider;
	ExprSliderCtrl *HtBiasSlider;
	ExprSliderCtrl *HardBiasSlider;
	ExprSliderCtrl *DropSlider;
	
	wxCheckBox *m_skeleton;
	wxCheckBox *m_splines;
	wxCheckBox *m_lines;
	wxCheckBox *m_show_one;
	
    wxString exprString();

    void OnChangedLevels(wxCommandEvent& event);
    void OnChangedDim(wxCommandEvent& event);
    void OnChangedSpline(wxCommandEvent& event);
    void OnChangedDisplay(wxCommandEvent& event);
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
	DEFINE_SLIDER_EVENTS(Density)
	DEFINE_SLIDER_EVENTS(SlopeBias)
	DEFINE_SLIDER_EVENTS(PhiBias)
	DEFINE_SLIDER_EVENTS(HardBias)
	DEFINE_SLIDER_EVENTS(HtBias)
	DEFINE_SLIDER_EVENTS(Drop)
	DECLARE_EVENT_TABLE()
};
#endif /*PLANTTABS_H_*/
