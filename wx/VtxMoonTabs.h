#ifndef MOON_TABS_H
#define MOON_TABS_H

#include "VtxTabsMgr.h"

class VtxMoonTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxMoonTabs)
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

	SliderCtrl *ShineSlider;
	ColorSlider *AmbientSlider;
	ColorSlider *SpecularSlider;
	ColorSlider *EmissionSlider;
	ColorSlider *DiffuseSlider;
	ColorSlider *ShadowSlider;

	void AddObjectTab(wxWindow *panel);
	void AddLightingTab(wxWindow *panel);
	Moon *object() 		{ return ((Moon*)(object_node->node));}
	double parentSize(){
		return ((Planet*)(object()->getParent()))->size;
	}

public:
	VtxMoonTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);

	~VtxMoonTabs(){
		delete CellSizeSlider;
		delete SizeSlider;
		delete TiltSlider;
		delete OrbitRadiusSlider;
		delete OrbitPhaseSlider;
		delete OrbitTiltSlider;
		delete DaySlider;
		delete RotPhaseSlider;
		delete YearSlider;
		delete ShineSlider;
		delete AmbientSlider;
		delete SpecularSlider;
		delete EmissionSlider;
		delete DiffuseSlider;
		delete ShadowSlider;
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
    	SizeSlider->setValueFromSlider();
    	Planetoid *obj=object();
        double val=SizeSlider->getValue()*MILES;
        obj->size=val;
        obj->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
    }
    void OnSizeSlider(wxScrollEvent& event){
    	SizeSlider->setValueFromSlider();
    }
    void OnSizeText(wxCommandEvent& event){
    	SizeSlider->setValueFromText();
    	double val=SizeSlider->getValue()*MILES;
        object()->size=val;
        object()->invalidate();
        TheView->set_changed_detail();
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

    void OnEndOrbitRadiusSlider(wxScrollEvent& event){
    	double val;
    	OnSliderValue(OrbitRadiusSlider, val);
    	object()->orbit_radius=val*parentSize();
    }
    void OnOrbitRadiusSlider(wxScrollEvent& event){
    	OrbitRadiusSlider->setValueFromSlider();
    }
    void OnOrbitRadiusText(wxCommandEvent& event){
       	double val;
    	OnSliderText(OrbitRadiusSlider, val);
       	object()->orbit_radius=val*parentSize();
    }

    DEFINE_SLIDER_VAR_EVENTS(OrbitPhase,object()->orbit_phase)
    DEFINE_SLIDER_VAR_EVENTS(OrbitTilt,object()->orbit_skew)
	DEFINE_SLIDER_VAR_EVENTS(Tilt,object()->tilt)
	DEFINE_SLIDER_VAR_EVENTS(Day,object()->day)
	DEFINE_SLIDER_VAR_EVENTS(Year,object()->year)
	DEFINE_SLIDER_VAR_EVENTS(RotPhase,object()->rot_phase)
	DEFINE_SLIDER_VAR_EVENTS(Shine,object()->shine)
	DEFINE_COLOR_VAR_EVENTS(Ambient,object()->ambient)
	DEFINE_COLOR_VAR_EVENTS(Specular,object()->specular)
	DEFINE_COLOR_VAR_EVENTS(Diffuse,object()->diffuse)
	DEFINE_COLOR_VAR_EVENTS(Emission,object()->emission)
	DEFINE_COLOR_VAR_EVENTS(Shadow,object()->shadow_color)

    void OnViewObj(wxCommandEvent& event);
    void OnUpdateViewObj(wxUpdateUIEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif /*MOON_TABS_H*/
