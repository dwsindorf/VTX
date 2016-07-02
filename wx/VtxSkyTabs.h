#ifndef SKYTABS_H_
#define SKYTABS_H_

#include "VtxTabsMgr.h"
#include <RenderOptions.h>

class VtxSkyTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxSkyTabs)
protected:
	SliderCtrl *CellSizeSlider;
	SliderCtrl  *HeightSlider;
	SliderCtrl  *DensitySlider;
	ColorSlider *HazeColor;
	ColorSlider *SkyColor;
	ColorSlider *TwiliteColor;
	ColorSlider *HaloColor;
	ColorSlider *NightColor;
	SliderCtrl  *SkyGrad;
	SliderCtrl  *TwiliteGrad;
	SliderCtrl  *HaloGrad;
	SliderCtrl  *HazeGrad;
	SliderCtrl  *NightGrad;

	void AddObjectTab(wxWindow *panel);
	void AddLightingTab(wxWindow *panel);
    void invalidateObject(){
    	object()->invalidate();
     	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
public:
	VtxSkyTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxSkyTabs(){
		delete CellSizeSlider;
		delete HeightSlider;
		delete DensitySlider;
		delete HazeColor;
		delete SkyColor;
		delete TwiliteColor;
		delete HaloColor;
		delete NightColor;
		delete SkyGrad;
		delete TwiliteGrad;
		delete HaloGrad;
		delete HazeGrad;
		delete NightGrad;
	}
	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);

	int showMenu(bool);

	Sky *object() 		{ return ((Sky*)(object_node->node));}
	double parentSize(){
		return ((Planet*)(object()->getParent()))->size;
	}

	void updateControls();
    void OnHeightSlider(wxScrollEvent& event){
    	double val;
    	OnSliderValue(HeightSlider, val);
    	object()->size=MILES*val+parentSize();
    	invalidateObject();
    }
    void OnHeightText(wxCommandEvent& event){
    	double val;
    	OnSliderText(HeightSlider, val);
    	object()->size=MILES*val+parentSize();
    	invalidateObject();
    }
    void OnEndCellSizeSlider(wxScrollEvent& event){
    	Spheroid *obj=object();
    	OnSliderValue(CellSizeSlider, obj->detail);
    	invalidateObject();
    }
    void OnCellSizeSlider(wxScrollEvent& event){
    	CellSizeSlider->setValueFromSlider();
    }
    void OnCellSizeText(wxCommandEvent& event){
    	Spheroid *obj=object();
    	OnSliderText(CellSizeSlider, obj->detail);
    	invalidateObject();
    }
    void OnDensitySlider(wxScrollEvent& event){
        OnSliderValue(DensitySlider, object()->density);
     }
    void OnDensityText(wxCommandEvent& event){
		OnSliderText(DensitySlider, object()->density);
       	TheView->set_changed_render();
    }
    void OnHaloSlider(wxScrollEvent& event){
       	OnColorSlider(HaloColor,object()->halo_color);
    }
    void OnHaloText(wxCommandEvent& event){
      	OnColorText(HaloColor,object()->halo_color);
     }
    void OnHaloColor(wxColourPickerEvent& WXUNUSED(event)){
      	OnColorValue(HaloColor,object()->halo_color);
    }
    void OnHaloGradSlider(wxScrollEvent& event){
	    OnSliderValue(HaloGrad, object()->shine);
    }
    void OnHaloGradText(wxCommandEvent& event){
	    OnSliderText(HaloGrad, object()->shine);
    }
    void OnTwiliteSlider(wxScrollEvent& event){
    	OnColorSlider(TwiliteColor,object()->twilite_color);
    }
    void OnTwiliteText(wxCommandEvent& event){
    	OnColorText(TwiliteColor,object()->twilite_color);
     }
    void OnTwiliteColor(wxColourPickerEvent& WXUNUSED(event)){
    	OnColorValue(TwiliteColor,object()->twilite_color);
    }
    void OnTwiliteGradSlider(wxScrollEvent& event){
    	OnSliderValue(TwiliteGrad,object()->twilite_max);
    }
    void OnTwiliteGradText(wxCommandEvent& event){
    	OnSliderText(TwiliteGrad,object()->twilite_max);
    }
    void OnNightSlider(wxScrollEvent& event){
    	OnColorSlider(NightColor,object()->night_color);
    }
    void OnNightText(wxCommandEvent& event){
    	OnColorText(NightColor,object()->night_color);
    }
    void OnNightColor(wxColourPickerEvent& WXUNUSED(event)){
    	OnColorValue(NightColor,object()->night_color);
    }
    void OnNightGradSlider(wxScrollEvent& event){
    	OnSliderValue(NightGrad,object()->twilite_min);
    }
    void OnNightGradText(wxCommandEvent& event){
    	OnSliderText(NightGrad,object()->twilite_min);
    }
    void OnSkySlider(wxScrollEvent& event){
    	Color c=object()->color();
    	OnColorSlider(SkyColor,c);
       	object()->set_color(c);
   }
    void OnSkyText(wxCommandEvent& event){
    	Color c=object()->color();
    	OnColorText(SkyColor,c);
    	object()->set_color(c);
    }
    void OnSkyColor(wxColourPickerEvent& WXUNUSED(event)){
    	Color c=object()->color();
    	OnColorValue(SkyColor,c);
       	object()->set_color(c);
     }
    void OnSkyGradSlider(wxScrollEvent& event){
	    OnSliderValue(SkyGrad, object()->sky_grad);
    }
    void OnSkyGradText(wxCommandEvent& event){
	    OnSliderText(SkyGrad, object()->sky_grad);
    }
    void OnHazeSlider(wxScrollEvent& event){
    	OnColorSlider(HazeColor,object()->haze_color);
    }
    void OnHazeText(wxCommandEvent& event){
    	OnColorText(HazeColor,object()->haze_color);
     }
    void OnHazeColor(wxColourPickerEvent& WXUNUSED(event)){
    	OnColorValue(HazeColor,object()->haze_color);
    }
    void OnHazeGradSlider(wxScrollEvent& event);
    void OnHazeGradText(wxCommandEvent& event);
    DECLARE_EVENT_TABLE()
};

#endif /*SKYTABS_H_*/
