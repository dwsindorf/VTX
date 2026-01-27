#ifndef ROCKSTABS_H_
#define ROCKSTABS_H_


#include "VtxTabsMgr.h"
#include "SceneClass.h"
#include "Rocks.h"
#include "TerrainClass.h"

class VtxRocksTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxRocksTabs)
protected:
	void AddPropertiesTab(wxWindow *panel);
	SliderCtrl *ScaleSlider;
	ExprSliderCtrl *DeltaSizeSlider;
	ExprSliderCtrl *DensitySlider;
	ExprSliderCtrl *FlatnessSlider;
	ExprSliderCtrl *DropSlider;
	ExprSliderCtrl *AmplSlider;
	ExprTextCtrl   *NoiseExpr;
	ExprSliderCtrl *SlopeBiasSlider;
	ExprSliderCtrl *PhiBiasSlider;
	ExprSliderCtrl *HtBiasSlider;
	ExprSliderCtrl *HardBiasSlider;

	wxChoice *m_scale_exp;
	wxChoice *m_orders;
	
	wxCheckBox *m_3d;
	//wxChoice *m_seed;
	
	bool changed_model;

	TNrocks *object() 	{ return (TNrocks *)object_node->node;}
	Planetoid *getOrbital()
	{
		NodeIF *obj=object_node->node;
		while(obj && !(obj->typeValue() & ID_ORBITAL))
			obj=obj->getParent();

		return (Planetoid*)obj;
	}

    void invalidateObject(){
    	object()->invalidate();
    	TheView->set_changed_render();
     	//TheView->set_changed_detail();
    	//TheScene->rebuild_all();
    }
	void getObjAttributes();
	void setObjAttributes();

public:
	VtxRocksTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxRocksTabs(){
		delete ScaleSlider;
		delete DeltaSizeSlider;
		delete DensitySlider;
		delete FlatnessSlider;
		delete DropSlider;
		delete AmplSlider;
		delete NoiseExpr;
		
	}
	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);

	int showMenu(bool);
	void updateControls();
	
	void OnChangedDim(wxCommandEvent& event);
	
	DEFINE_SLIDER_EVENTS(Scale)
	DEFINE_SLIDER_EVENTS(DeltaSize)
	DEFINE_SLIDER_EVENTS(Density)
	DEFINE_SLIDER_EVENTS(Flatness)
	DEFINE_SLIDER_EVENTS(Drop)
	DEFINE_SLIDER_EVENTS(Ampl)
	DEFINE_SLIDER_EVENTS(SlopeBias)
	DEFINE_SLIDER_EVENTS(PhiBias)
	DEFINE_SLIDER_EVENTS(HardBias)
	DEFINE_SLIDER_EVENTS(HtBias)

    void OnDelete(wxCommandEvent& event);
	void OnEnable(wxCommandEvent& event);
    void OnChanged(wxCommandEvent& event){
		setObjAttributes();
		invalidateObject();
		
    }
    void OnNameText(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};


#endif /*ROCKSTABS_H_*/
