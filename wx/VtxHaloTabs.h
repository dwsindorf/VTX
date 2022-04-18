/*
 * VtxHaloTabs.h
 *
 *  Created on: Apr 16, 2022
 *      Author: dean
 */

#ifndef WX_VTXHALOTABS_H_
#define WX_VTXHALOTABS_H_

#include "VtxTabsMgr.h"

class VtxHaloTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxHaloTabs)
protected:
	SliderCtrl  *SizeSlider;
	ColorSlider *InnerSlider;
	ColorSlider *OuterSlider;

	SliderCtrl  *GradientSlider;

	void AddObjectTab(wxWindow *panel);

public:
	VtxHaloTabs(wxWindow* parent,
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

	~VtxHaloTabs(){
		delete SizeSlider;
		delete OuterSlider;
		delete InnerSlider;
		delete GradientSlider;
	}
	int showMenu(bool);
	Halo *object() 		{ return ((Halo*)object_node->node);}

	double parentSize(){
		return ((Star*)(object()->getParent()))->size;
	}

	void updateControls();
    void OnSizeSlider(wxScrollEvent& event){
    	double val;
    	OnSliderValue(SizeSlider, val);
    	object()->ht=val;
    	object()->size=val+parentSize();
        object()->invalidate();
        TheScene->rebuild();
    }
    void OnSizeText(wxCommandEvent& event){
    	double val;
    	OnSliderText(SizeSlider, val);
    	object()->ht=val;
        object()->size=val+parentSize();
        object()->invalidate();
        TheScene->rebuild();
    }
    void OnGradientSlider(wxScrollEvent& event){
    	OnSliderValue(GradientSlider, object()->gradient);
    }
    void OnGradientText(wxCommandEvent& event){
    	OnSliderText(GradientSlider, object()->gradient);
    }
    void OnOuterSlider(wxScrollEvent& event){
     	OnColorSlider(OuterSlider,object()->color2);
     }
    void OnOuterText(wxCommandEvent& event){
    	OnColorText(OuterSlider,object()->color2);
    }
    void OnOuterColor(wxColourPickerEvent& WXUNUSED(event)){
     	OnColorValue(OuterSlider,object()->color2);
    }
    void OnInnerSlider(wxScrollEvent& event){
     	OnColorSlider(InnerSlider,object()->color1);
    }
    void OnInnerText(wxCommandEvent& event){
    	OnColorText(InnerSlider,object()->color1);
    }
    void OnInnerColor(wxColourPickerEvent& WXUNUSED(event)){
    	OnColorValue(InnerSlider,object()->color1);
    }

	DECLARE_EVENT_TABLE()
};

#endif /* WX_VTXHALOTABS_H_ */
