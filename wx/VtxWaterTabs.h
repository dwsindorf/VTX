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

	void AddWaterTab(wxWindow *panel);
    void invalidateObject(){
    	water()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
    void invalidateRender(){
    	//setObjAttributes();
    	TheView->set_changed_render();
    }
	void getObjAttributes();
	void setObjAttributes();

	SliderCtrl   *LevelSlider;
	SliderCtrl   *AlbedoSlider;
	SliderCtrl   *ShineSlider;
	ExprTextCtrl *m_waves;
	ColorSlider  *TransmitSlider;
	ColorSlider  *ReflectSlider;
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
		delete AlbedoSlider;
		delete ShineSlider;
		delete m_waves;
		delete TransmitSlider;
		delete ReflectSlider;
	}
	void updateControls();
	int showMenu(bool);
    void OnEnable(wxCommandEvent& event);
    void OnUpdateEnable(wxUpdateUIEvent& event);

    void OnWavesEnter(wxCommandEvent& event){
     	invalidateObject();
    }

    void OnEndLevelSlider(wxScrollEvent& event){
    	LevelSlider->setValueFromSlider();
    	invalidateObject();
    }
    void OnLevelSlider(wxScrollEvent& event){
    	LevelSlider->setValueFromSlider();
    }
    void OnLevelText(wxCommandEvent& event){
    	LevelSlider->setValueFromText();
    	invalidateObject();
    }
	DEFINE_SLIDER_EVENTS(Transmit)
    void OnTransmitColor(wxColourPickerEvent& WXUNUSED(event)){
		Planetoid *orb=getOrbital();
		orb->water_color2=TransmitSlider->getColor();
    	invalidateRender();
    }
	DEFINE_SLIDER_EVENTS(Reflect)
    void OnReflectColor(wxColourPickerEvent& WXUNUSED(event)){
		Planetoid *orb=getOrbital();
		orb->water_color1=ReflectSlider->getColor();
    	invalidateRender();
    }
	DEFINE_SLIDER_EVENTS(Albedo)
	DEFINE_SLIDER_EVENTS(Shine)

	DECLARE_EVENT_TABLE()
};
#endif /*WATERTABS_H_*/
