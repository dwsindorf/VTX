#ifndef CRATERS_FUNCT_H_
#define CRATERS_FUNCT_H_

#include "VtxFunctMgr.h"
#include "VtxExprEdit.h"

class VtxCratersFunct : public VtxFunctMgr
{
	DECLARE_CLASS(VtxCratersFunct)
protected:
	void AddCratersTab(wxWindow *panel);
	void AddShapeTab(wxWindow *panel);
	SliderCtrl *ScaleSlider;
	ExprSliderCtrl *DeltaSizeSlider;
	ExprSliderCtrl *DensitySlider;
	ExprSliderCtrl *ImpactSlider;
	ExprSliderCtrl *AmplSlider;
	ExprSliderCtrl *DropSlider;
	ExprSliderCtrl *RiseSlider;
	ExprSliderCtrl *RimSlider;
	ExprSliderCtrl *FloorSlider;
	ExprSliderCtrl *CenterSlider;
	ExprSliderCtrl *OffsetSlider;

	ExprSliderCtrl *VNoiseSlider;
	ExprSliderCtrl *RNoiseSlider;
	ExprSliderCtrl *NoiseBiasSlider;
	ExprSliderCtrl *NoiseBiasMinSlider;
	
	wxChoice *m_scale_exp;
	wxChoice *m_orders;
	wxChoice *m_seed;

public:
	VtxCratersFunct(wxWindow* parent,
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

	~VtxCratersFunct(){
		delete ScaleSlider;
		delete DeltaSizeSlider;
		delete DensitySlider;
		delete ImpactSlider;
		delete AmplSlider;
		delete DropSlider;
		delete RiseSlider;
		delete RimSlider;
		delete FloorSlider;
		delete CenterSlider;
		delete VNoiseSlider;
		delete RNoiseSlider;
		delete NoiseBiasSlider;
		delete NoiseBiasMinSlider;
	}
	DEFINE_SLIDER_EVENTS(Scale)
	DEFINE_SLIDER_EVENTS(Density)
	DEFINE_SLIDER_EVENTS(DeltaSize)
	DEFINE_SLIDER_EVENTS(Impact)
	DEFINE_SLIDER_EVENTS(Ampl)
	DEFINE_SLIDER_EVENTS(Drop)
	DEFINE_SLIDER_EVENTS(Rise)
	DEFINE_SLIDER_EVENTS(Rim)
	DEFINE_SLIDER_EVENTS(Floor)
	DEFINE_SLIDER_EVENTS(Center)
	DEFINE_SLIDER_EVENTS(RNoise)
	DEFINE_SLIDER_EVENTS(VNoise)
	DEFINE_SLIDER_EVENTS(Offset)
	DEFINE_SLIDER_EVENTS(NoiseBias)
	DEFINE_SLIDER_EVENTS(NoiseBiasMin)

    void OnChangeEvent(wxCommandEvent& event){
    	getFunction();
    }

	void setFunction(wxString);
	void getFunction();

	DECLARE_EVENT_TABLE()
};


#endif /*CRATERS_FUNCT_H_*/
