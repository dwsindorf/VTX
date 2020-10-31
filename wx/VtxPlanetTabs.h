#ifndef PLANET_TABS_H
#define PLANET_TABS_H

#include "VtxTabsMgr.h"

class VtxPlanetTabs: public VtxTabsMgr {
DECLARE_CLASS(VtxPlanetTabs)
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
	SliderCtrl *SymmetrySlider;
	SliderCtrl *HscaleSlider;

	SliderCtrl *ShineSlider;
	ColorSlider *AmbientSlider;
	ColorSlider *SpecularSlider;
	ColorSlider *EmissionSlider;
	ColorSlider *DiffuseSlider;
	ColorSlider *ShadowSlider;

	void AddObjectTab(wxWindow *panel);
	void AddLightingTab(wxWindow *panel);

public:
	VtxPlanetTabs(wxWindow *parent, wxWindowID id, const wxPoint &pos =
			wxDefaultPosition, const wxSize &size = wxDefaultSize, long style =
			0, const wxString &name = wxNotebookNameStr);
	~VtxPlanetTabs() {
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
		delete SymmetrySlider;
		delete HscaleSlider;
	}
	bool Create(wxWindow *parent, wxWindowID id, const wxPoint &pos =
			wxDefaultPosition, const wxSize &size = wxDefaultSize, long style =
			0, const wxString &name = wxNotebookNameStr);

	int showMenu(bool);

	Planetoid* object() {
		return ((Planetoid*) (object_node->node));
	}

	void updateControls();
	void OnEndSizeSlider(wxScrollEvent &event) {
		SizeSlider->setValueFromSlider();
		Planetoid *obj = object();
		double val = SizeSlider->getValue() * MILES;
		obj->size = val;
		TheView->set_changed_detail();
		TheScene->rebuild_all();
	}
	void OnSizeSlider(wxScrollEvent &event) {
		SizeSlider->setValueFromSlider();
//		Planetoid *obj = object();
//		double val = SizeSlider->getValue() * MILES;
//		obj->size = val;
//		TheView->set_changed_detail();
//		TheScene->rebuild_all();
	}
	void OnSizeText(wxCommandEvent &event) {
		SizeSlider->setValueFromText();
		double val = SizeSlider->getValue() * MILES;
		object()->size = val;
		TheView->set_changed_detail();
		TheScene->rebuild_all();
	}

	void OnEndSymmetrySlider(wxScrollEvent &event) {
		SymmetrySlider->setValueFromSlider();
		Planetoid *obj = object();
		double val = SymmetrySlider->getValue();
		obj->symmetry = val;
		TheView->set_changed_detail();
		TheScene->rebuild_all();
	}
	void OnSymmetrySlider(wxScrollEvent &event) {
		SymmetrySlider->setValueFromSlider();
	}
	void OnSymmetryText(wxCommandEvent &event) {
		SymmetrySlider->setValueFromText();
		Planetoid *obj = object();
		double val = SymmetrySlider->getValue();
		obj->symmetry = val;
		TheView->set_changed_detail();
		TheScene->rebuild_all();
	}
	void OnEndHscaleSlider(wxScrollEvent &event) {
		HscaleSlider->setValueFromSlider();
		Planetoid *obj = object();
		double val = HscaleSlider->getValue();
		obj->hscale = val * MILES / obj->size;
		obj->invalidate();
		TheView->set_changed_detail();
		TheScene->rebuild();
	}
	void OnHscaleSlider(wxScrollEvent &event) {
		HscaleSlider->setValueFromSlider();
	}
	void OnHscaleText(wxCommandEvent &event) {
		HscaleSlider->setValueFromText();
		Planetoid *obj = object();
		double val = HscaleSlider->getValue();
		object()->hscale = val * MILES / obj->size;
		object()->invalidate();
		TheView->set_changed_detail();
		TheScene->rebuild();
	}

	void OnEndCellSizeSlider(wxScrollEvent &event) {
		Spheroid *obj = object();
		OnSliderValue(CellSizeSlider, obj->detail);
		obj->invalidate();
		TheView->set_changed_detail();
		TheScene->rebuild();
	}
	void OnCellSizeSlider(wxScrollEvent &event) {
		CellSizeSlider->setValueFromSlider();
	}
	void OnCellSizeText(wxCommandEvent &event) {
		Spheroid *obj = object();
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

#endif /*PLANET_TABS_H*/
