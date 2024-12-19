#ifndef WATERTABS_H_
#define WATERTABS_H_

#include "VtxTabsMgr.h"

#include <TerrainClass.h>
class VtxWaterTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxWaterTabs)

protected:
	TNwater *water() 	{ return (TNwater *)object_node->node;}
	Planetoid *getOrbital()
	{
		NodeIF *obj=water();
		while(obj && !(obj->typeValue() & ID_ORBITAL))
			obj=obj->getParent();
		return (Planetoid*)obj;
	}

	void AddPropertiesTab(wxWindow *panel);
	void AddLiquidTab(wxWindow *panel);
	void AddSolidTab(wxWindow *panel);

    void invalidateObject(){
    	water()->invalidate();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
    void invalidateRender(){
    	TheView->set_changed_render();
    }
	void getObjAttributes();
	void setObjAttributes();
	void getDefaultState();
	void setState();
	void OnUpdateState(wxUpdateUIEvent &event);


	StaticTextCtrl *planet_temp;
	wxCheckBox     *auto_state;

	SliderCtrl   *LevelSlider;

	SliderCtrl   *LiquidTempSlider;
	SliderCtrl   *LiquidTransTempSlider;
	SliderCtrl   *LiquidAlbedoSlider;
	SliderCtrl   *LiquidShineSlider;
	SliderCtrl   *LiquidMixSlider;
	ExprTextCtrl *LiquidFunction;
	ColorSlider  *LiquidTransmitSlider;
	ColorSlider  *LiquidReflectSlider;

	SliderCtrl   *SolidTempSlider;
	SliderCtrl   *SolidTransTempSlider;
	SliderCtrl   *SolidAlbedoSlider;
	SliderCtrl   *SolidShineSlider;
	SliderCtrl   *SolidMixSlider;
	ExprTextCtrl *SolidFunction;
	ColorSlider  *SolidTransmitSlider;
	ColorSlider  *SolidReflectSlider;

	ExprTextCtrl *OceanFunction;

	wxChoice   *Composition;
	wxRadioBox *State;
	wxButton	*default_mod;
	wxButton	*default_liquid;
	wxButton	*default_solid;

public:
	VtxWaterTabs(wxWindow* parent,
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

	~VtxWaterTabs(){
		delete LevelSlider;
		delete LiquidAlbedoSlider;
		delete LiquidShineSlider;
		delete LiquidFunction;
		delete LiquidTransmitSlider;
		delete LiquidReflectSlider;
		delete LiquidTempSlider;
		delete LiquidTransTempSlider;

		delete SolidAlbedoSlider;
		delete SolidShineSlider;
		delete SolidFunction;
		delete SolidTransmitSlider;
		delete SolidReflectSlider;
		delete SolidTempSlider;
		delete SolidTransTempSlider;
		delete OceanFunction;
	}
	void updateControls();
	int showMenu(bool);
    void OnEnable(wxCommandEvent& event);
    void OnUpdateEnable(wxUpdateUIEvent& event);
    void OnChangeComposition(wxCommandEvent& event);
    void OnAutoState(wxCommandEvent& event);
    void OnSetDefaultMod(wxCommandEvent& event);
    void OnSetDefaultLiquid(wxCommandEvent& event);
    void OnSetDefaultSolid(wxCommandEvent& event);
    void OnSetState(wxCommandEvent& event);

    void OnSurfaceFunctionEnter(wxCommandEvent& event){
    	setObjAttributes();
    }

    void OnEndLevelSlider(wxScrollEvent& event){
    	LevelSlider->setValueFromSlider();
    	setObjAttributes();
    }
    void OnLevelSlider(wxScrollEvent& event){
    	LevelSlider->setValueFromSlider();
    }
    void OnLevelText(wxCommandEvent& event){
    	LevelSlider->setValueFromText();
    	setObjAttributes();
    }
    void OnLiquidTransmitColor(wxColourPickerEvent& WXUNUSED(event)){
    	setObjAttributes();
    }
    void OnLiquidReflectColor(wxColourPickerEvent& WXUNUSED(event)){
    	setObjAttributes();
    }

	DEFINE_SLIDER_EVENTS(LiquidTemp)
	DEFINE_SLIDER_EVENTS(LiquidTransTemp)
	DEFINE_SLIDER_EVENTS(LiquidMix)
	DEFINE_SLIDER_EVENTS(LiquidReflect)
	DEFINE_SLIDER_EVENTS(LiquidTransmit)
    DEFINE_SLIDER_EVENTS(LiquidAlbedo)
	DEFINE_SLIDER_EVENTS(LiquidShine)

    void OnSolidTransmitColor(wxColourPickerEvent& WXUNUSED(event)){
    	setObjAttributes();
    	invalidateRender();
    }
    void OnSolidReflectColor(wxColourPickerEvent& WXUNUSED(event)){
    	setObjAttributes();
    	invalidateRender();
    }

	DEFINE_SLIDER_EVENTS(SolidTemp)
	DEFINE_SLIDER_EVENTS(SolidTransTemp)
	DEFINE_SLIDER_EVENTS(SolidMix)
	DEFINE_SLIDER_EVENTS(SolidReflect)
	DEFINE_SLIDER_EVENTS(SolidTransmit)
    DEFINE_SLIDER_EVENTS(SolidAlbedo)
	DEFINE_SLIDER_EVENTS(SolidShine)

	DECLARE_EVENT_TABLE()
};
#endif /*WATERTABS_H_*/
