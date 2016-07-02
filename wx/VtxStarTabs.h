#ifndef STARTABS_H_
#define STARTABS_H_

#include "VtxTabsMgr.h"


class VtxStarTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxStarTabs)
protected:
	SliderCtrl *CellSizeSlider;
	SliderCtrl *SizeSlider;
	SliderCtrl *TiltSlider;
	SliderCtrl *OrbitRadiusSlider;
	SliderCtrl *OrbitPhaseSlider;
	SliderCtrl *OrbitTiltSlider;
	SliderCtrl *DaySlider;
	SliderCtrl *RotPhaseSlider;
	SliderCtrl *YearSlider;
	ColorSlider *SpecularSlider;
	ColorSlider *EmissionSlider;
	ColorSlider *DiffuseSlider;

	Star *object() 		{ return ((Star*)object_node->node);}

	void AddObjectTab(wxWindow *panel);
	void AddLightingTab(wxWindow *panel);
public:
	VtxStarTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxStarTabs(){
		delete CellSizeSlider;
		delete SizeSlider;
		delete TiltSlider;
		delete OrbitRadiusSlider;
		delete OrbitPhaseSlider;
		delete OrbitTiltSlider;
		delete DaySlider;
		delete RotPhaseSlider;
		delete YearSlider;
		delete SpecularSlider;
		delete EmissionSlider;
		delete DiffuseSlider;
	}
	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);

	int showMenu(bool);
	void updateControls();

    void OnEndSizeSlider(wxScrollEvent& event){
    	OnSliderValue(SizeSlider, object()->size);
        object()->invalidate();
        TheScene->rebuild();
    }
    void OnSizeSlider(wxScrollEvent& event){
    	SizeSlider->setValueFromSlider();
    }
    void OnSizeText(wxCommandEvent& event){
    	OnSliderText(SizeSlider, object()->size);
        object()->invalidate();
        TheScene->rebuild();
    }

    void OnEndCellSizeSlider(wxScrollEvent& event){
    	Spheroid *obj=object();
    	OnSliderValue(CellSizeSlider, obj->detail);
        obj->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
    }
    void OnCellSizeSlider(wxScrollEvent& event){
    	CellSizeSlider->setValueFromSlider();
    }
    void OnCellSizeText(wxCommandEvent& event){
    	Spheroid *obj=object();
    	OnSliderText(CellSizeSlider, obj->detail);
        obj->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
    }

    DEFINE_SLIDER_VAR_EVENTS(OrbitRadius,object()->orbit_radius)
    DEFINE_SLIDER_VAR_EVENTS(OrbitPhase,object()->orbit_phase)
    DEFINE_SLIDER_VAR_EVENTS(OrbitTilt,object()->orbit_skew)
    DEFINE_SLIDER_VAR_EVENTS(Tilt,object()->tilt)
    DEFINE_SLIDER_VAR_EVENTS(Day,object()->day)
    DEFINE_SLIDER_VAR_EVENTS(Year,object()->year)
    DEFINE_SLIDER_VAR_EVENTS(RotPhase,object()->rot_phase)
    DEFINE_COLOR_VAR_EVENTS(Specular,object()->specular)
    DEFINE_COLOR_VAR_EVENTS(Diffuse,object()->diffuse)
    DEFINE_COLOR_VAR_EVENTS(Emission,object()->emission)

    void OnViewObj(wxCommandEvent& event);
    void OnUpdateViewObj(wxUpdateUIEvent& event);

	DECLARE_EVENT_TABLE()
};
#endif /*STARTABS_H_*/
