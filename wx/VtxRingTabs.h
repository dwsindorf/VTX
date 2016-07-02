#ifndef RINGTABS_H_
#define RINGTABS_H_

#include "VtxTabsMgr.h"

class VtxRingTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxRingTabs)
protected:
	SliderCtrl  *RadiusSlider;
	SliderCtrl  *WidthSlider;
	SliderCtrl  *CellSizeSlider;
	ColorSlider *EmissionSlider;
	ColorSlider *ShadowSlider;
	void AddObjectTab(wxWindow *panel);
	void AddLightingTab(wxWindow *panel);

public:
	VtxRingTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxRingTabs(){
		delete RadiusSlider;
		delete WidthSlider;
		delete EmissionSlider;
		delete CellSizeSlider;
		delete ShadowSlider;
	}

	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);

	int showMenu(bool);
	Ring *object() 		{ return ((Ring*)object_node->node);}
    void invalidateObject(){
    	object()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();

	double parentSize(){
		return ((Spheroid*)(object()->getParent()))->size;
	}

	void updateControls();
    void OnRadiusSlider(wxScrollEvent& event){
    	RadiusSlider->setValueFromSlider();
    }
    void OnEndRadiusSlider(wxScrollEvent& event){
    	RadiusSlider->setValueFromSlider();
    	invalidateObject();
    }
    void OnRadiusText(wxCommandEvent& event){
    	RadiusSlider->setValueFromText();
    	invalidateObject();
    }
    void OnWidthSlider(wxScrollEvent& event){
    	WidthSlider->setValueFromSlider();
    }
    void OnEndWidthSlider(wxScrollEvent& event){
    	WidthSlider->setValueFromSlider();
    	invalidateObject();
    }
    void OnWidthText(wxCommandEvent& event){
    	WidthSlider->setValueFromText();
    	invalidateObject();
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

    DEFINE_COLOR_VAR_EVENTS(Emission,object()->emission)
	DEFINE_COLOR_VAR_EVENTS(Shadow,object()->shadow_color)
	DECLARE_EVENT_TABLE()
};

#endif /*RINGTABS_H_*/
