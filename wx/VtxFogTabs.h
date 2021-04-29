#ifndef FOGTABS_H_
#define FOGTABS_H_

#include "VtxTabsMgr.h"
#include <TerrainClass.h>
class VtxFogTabs: public VtxTabsMgr

{
DECLARE_CLASS(VtxFogTabs)

protected:
	TNfog* fog() {
		return (TNfog*) object_node->node;
	}
	TNfog* object() {
		return ((TNfog*) object_node->node);
	}
	Planetoid* getOrbital() {
		NodeIF *obj = fog();
		while (obj && !(obj->typeValue() & ID_ORBITAL))
			obj = obj->getParent();

		return (Planetoid*) obj;
	}
	void AddFogTab(wxWindow *panel);
	void invalidateObject() {
		fog()->invalidate();
		setObjAttributes();
		TheView->set_changed_detail();
		TheScene->rebuild();
	}
	void invalidateRender() {
		setObjAttributes();
		TheView->set_changed_render();
	}
	void getObjAttributes();
	void setObjAttributes();

	ExprTextCtrl *DensityExpr;
	SliderCtrl *HMinSlider;
	SliderCtrl *HMaxSlider;
	SliderCtrl *VMinSlider;
	SliderCtrl *VMaxSlider;
	SliderCtrl *FogGlowSlider;
	ColorSlider *FogColorSlider;

public:
	VtxFogTabs(wxWindow *parent, wxWindowID id, const wxPoint &pos =
			wxDefaultPosition, const wxSize &size = wxDefaultSize, long style =
			0, const wxString &name = wxNotebookNameStr);

	bool Create(wxWindow *parent, wxWindowID id, const wxPoint &pos =
			wxDefaultPosition, const wxSize &size = wxDefaultSize, long style =
			0, const wxString &name = wxNotebookNameStr);
	~VtxFogTabs() {
		delete DensityExpr;
		delete HMinSlider;
		delete HMaxSlider;
		delete VMinSlider;
		delete VMaxSlider;
		delete FogColorSlider;
		delete FogGlowSlider;
	}

	void updateControls();
	int showMenu(bool);
	void OnEnable(wxCommandEvent &event);
	void OnUpdateEnable(wxUpdateUIEvent &event);

	DEFINE_SLIDER_RENDER_EVENTS(FogGlow)
	DEFINE_COLOR_VAR_EVENTS(FogColor,getOrbital()->fog_color)

	DEFINE_SLIDER_RENDER_EVENTS(HMin)
	DEFINE_SLIDER_RENDER_EVENTS(HMax)
	DEFINE_SLIDER_RENDER_EVENTS(VMin)
	DEFINE_SLIDER_RENDER_EVENTS(VMax)

	void OnChangedExpr(wxCommandEvent &event) {
		invalidateObject();
	}

	DECLARE_EVENT_TABLE()
};
#endif /*FOGTABS_H_*/
