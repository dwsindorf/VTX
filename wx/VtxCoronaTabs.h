#ifndef CORONATABS_H_
#define CORONATABS_H_

#include "VtxTabsMgr.h"

class VtxCoronaTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxCoronaTabs)
protected:
	SliderCtrl  *SizeSlider;
	ColorSlider *OuterSlider;
	ColorSlider *InnerSlider;
	SliderCtrl  *GradientSlider;
	void AddObjectTab(wxWindow *panel);

public:
	VtxCoronaTabs(wxWindow* parent,
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

	~VtxCoronaTabs(){
		delete SizeSlider;
		delete OuterSlider;
		delete InnerSlider;
		delete GradientSlider;
	}
	int showMenu(bool);
	Corona *object() 		{ return ((Corona*)object_node->node);}

	double parentSize(){
		return ((Star*)(object()->getParent()))->size;
		//return object()->inner_radius;
	}

	void updateControls();
    void OnSizeSlider(wxScrollEvent& event){
    	double val;
    	OnSliderValue(SizeSlider, val);
    	object()->size=val*parentSize();
    	object()->ht=object()->size-parentSize();
        object()->invalidate();
        TheScene->rebuild();
    }
    void OnSizeText(wxCommandEvent& event){
    	double val;
    	OnSliderText(SizeSlider, val);
    	object()->size=val*parentSize();
    	object()->ht=object()->size-parentSize();
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

#endif /*CORONATABS_H_*/
