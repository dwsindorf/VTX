#ifndef ERODE_FUNCT_H_
#define ERODE_FUNCT_H_

#include "VtxFunctMgr.h"
#include "VtxExprEdit.h"

class VtxErodeFunct : public VtxFunctMgr
{
	DECLARE_CLASS(VtxErodeFunct)
protected:
	void AddControlsTab(wxWindow *panel);
	SliderCtrl *StartSlider;
	ExprSliderCtrl *FillAmplSlider;
	ExprSliderCtrl *FillLevelSlider;
	ExprSliderCtrl *FillMarginSlider;
    ExprSliderCtrl *FillTransportSlider;

	ExprSliderCtrl *SlopeAmplSlider;
	ExprSliderCtrl *SlopeMinSlider;
    ExprSliderCtrl *SlopeMaxSlider;

	ExprSliderCtrl *SlopeDropSlider;
	wxCheckBox *m_sqr;
	wxCheckBox *m_ss;

public:
	VtxErodeFunct(wxWindow* parent,
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

	~VtxErodeFunct(){
		delete StartSlider;
		delete FillAmplSlider;
		delete FillLevelSlider;
		delete FillMarginSlider;
        delete FillTransportSlider;
		delete SlopeAmplSlider;
        delete SlopeMinSlider;
		delete SlopeMaxSlider;
		delete SlopeDropSlider;
	}
	DEFINE_SLIDER_EVENTS(Start)
	DEFINE_SLIDER_EVENTS(FillAmpl)
	DEFINE_SLIDER_EVENTS(FillLevel)
	DEFINE_SLIDER_EVENTS(FillMargin)
    DEFINE_SLIDER_EVENTS(FillTransport)

	DEFINE_SLIDER_EVENTS(SlopeAmpl)
    DEFINE_SLIDER_EVENTS(SlopeMin)

	DEFINE_SLIDER_EVENTS(SlopeMax)
	DEFINE_SLIDER_EVENTS(SlopeDrop)

    void OnChangeEvent(wxCommandEvent& event){
    	getFunction();
    }

	void setFunction(wxString);
	void getFunction();

	DECLARE_EVENT_TABLE()
};


#endif /*FRACTAL_FUNCT_H_*/
