#ifndef ASTEROID_TABS_H
#define ASTEROID_TABS_H

#include "VtxTabsMgr.h"

class VtxAsteroidTabs: public VtxTabsMgr {
DECLARE_CLASS(VtxAsteroidTabs)
protected:
	SliderCtrl *CellSizeSlider;
	SliderCtrl *SizeSlider;
	SliderCtrl *OrbitRadiusSlider;
	SliderCtrl *OrbitPhaseSlider;
	SliderCtrl *OrbitTiltSlider;
	SliderCtrl *DaySlider;
	SliderCtrl *RotPhaseSlider;
	SliderCtrl *YearSlider;
	SliderCtrl *HscaleSlider;
	SliderCtrl *ShineSlider;
	SliderCtrl *AlbedoSlider;
	ExprTextCtrl   *NoiseExpr;

	ColorSlider *AmbientSlider;
	ColorSlider *SpecularSlider;
	ColorSlider *EmissionSlider;
	ColorSlider *DiffuseSlider;
	ColorSlider *ShadowSlider;
	
	void AddObjectTab(wxWindow *panel);
	void AddLightingTab(wxWindow *panel);
	void OnTidalLock(wxCommandEvent& event);
	void rebuild(){
		object()->invalidate();
		TheView->set_changed_detail();
		TheScene->rebuild();
	}
public:
	VtxAsteroidTabs(wxWindow *parent, wxWindowID id, const wxPoint &pos =
			wxDefaultPosition, const wxSize &size = wxDefaultSize, long style =
			0, const wxString &name = wxNotebookNameStr);
	~VtxAsteroidTabs() {
		delete CellSizeSlider;
		delete SizeSlider;
		delete OrbitRadiusSlider;
		delete OrbitPhaseSlider;
		delete OrbitTiltSlider;
		delete DaySlider;
		delete RotPhaseSlider;
		delete YearSlider;
		delete ShineSlider;
		delete AlbedoSlider;
		delete AmbientSlider;
		delete SpecularSlider;
		delete EmissionSlider;
		delete DiffuseSlider;
		delete ShadowSlider;
		delete NoiseExpr;

	}
	bool Create(wxWindow *parent, wxWindowID id, const wxPoint &pos =
			wxDefaultPosition, const wxSize &size = wxDefaultSize, long style =
			0, const wxString &name = wxNotebookNameStr);

	int showMenu(bool);

	Asteroid* object() {
		return ((Asteroid*) (object_node->node));
	}
	void updateControls();
	void OnEndSizeSlider(wxScrollEvent &event) {
		SizeSlider->setValueFromSlider();
		Asteroid *obj = object();
		double val = SizeSlider->getValue() * MILES*1000;
		obj->size = val;
		rebuild();
	}
	void OnSizeSlider(wxScrollEvent &event) {
		SizeSlider->setValueFromSlider();
	}
	void OnSizeText(wxCommandEvent &event) {
		SizeSlider->setValueFromText();
		double val = SizeSlider->getValue() * MILES*1000;
		object()->size = val;
		rebuild();
	}
	void OnEndHscaleSlider(wxScrollEvent &event) {
		HscaleSlider->setValueFromSlider();
		double val = HscaleSlider->getValue();
		object()->hscale = val;// obj->size;
		rebuild();
	}
	void OnHscaleSlider(wxScrollEvent &event) {
		HscaleSlider->setValueFromSlider();
	}
	void OnHscaleText(wxCommandEvent &event) {
		HscaleSlider->setValueFromText();
		double val = HscaleSlider->getValue();
		object()->hscale = val;
		rebuild();
	}

	void OnEndCellSizeSlider(wxScrollEvent &event) {
		OnSliderValue(CellSizeSlider, object()->detail);
		rebuild();
	}
	void OnCellSizeSlider(wxScrollEvent &event) {
		CellSizeSlider->setValueFromSlider();
	}
	void OnCellSizeText(wxCommandEvent &event) {
		OnSliderText(CellSizeSlider, object()->detail);
		rebuild();
	}
	void OnEndOrbitRadiusSlider(wxScrollEvent &event) {}

    void OnOrbitRadiusSlider(wxScrollEvent& event){
    	OrbitRadiusSlider->setValueFromSlider();
    	OnSliderValue(OrbitRadiusSlider, object()->orbit_radius);
		invalidateRender();
	}
    void OnOrbitRadiusText (wxCommandEvent& event){
    	OnSliderText( OrbitRadiusSlider, object()->orbit_radius);
		invalidateRender();
	}

	void OnEndAlbedoSlider(wxScrollEvent &event) {}

    void OnAlbedoSlider(wxScrollEvent& event){
    	AlbedoSlider->setValueFromSlider();
    	OnSliderValue(AlbedoSlider, object()->albedo);
		invalidateRender();
	}
    void OnAlbedoText (wxCommandEvent& event){
    	OnSliderText( AlbedoSlider, object()->albedo);
		invalidateRender();
	}
    void invalidate(){
        object()->invalidate();
        TheView->set_changed_detail();
    }
    void OnSliderText(SliderCtrl *s, double &var){
       s->setValueFromText();
       var=s->getValue();
       invalidate();
    }
    void OnSliderValue(SliderCtrl *s, double &var){
        s->setValueFromSlider();
        var=s->getValue();
        invalidate();
    }

    void OnChangedNoiseExpr(wxCommandEvent& event){
    	cout<<"changed noise"<<endl;
    	object()->setNoiseFunction((char*)NoiseExpr->GetValue().ToAscii());
    	object()->applyNoiseFunction();
    	rebuild();
    }
    
	DEFINE_SLIDER_VAR_EVENTS(OrbitPhase,object()->orbit_phase)
	DEFINE_SLIDER_VAR_EVENTS(OrbitTilt,object()->orbit_skew)
	DEFINE_SLIDER_VAR_EVENTS(Day,object()->day)
	DEFINE_SLIDER_VAR_EVENTS(Year,object()->year)
	DEFINE_SLIDER_VAR_EVENTS(RotPhase,object()->rot_phase)
	DEFINE_SLIDER_VAR_EVENTS(Shine,object()->shine)
	
	DEFINE_COLOR_VAR_EVENTS(Ambient,object()->ambient)
	DEFINE_COLOR_VAR_EVENTS(Specular,object()->specular)
	DEFINE_COLOR_VAR_EVENTS(Diffuse,object()->diffuse)
	DEFINE_COLOR_VAR_EVENTS(Emission,object()->emission)
	DEFINE_COLOR_VAR_EVENTS(Shadow,object()->shadow_color)

	void OnViewObj(wxCommandEvent &event);
	void OnUpdateViewObj(wxUpdateUIEvent &event);


DECLARE_EVENT_TABLE()
};

#endif /*ASTEROID_TABS_H*/
