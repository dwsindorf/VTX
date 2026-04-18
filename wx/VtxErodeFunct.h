#ifndef ERODE_FUNCT_H_
#define ERODE_FUNCT_H_

#include "VtxFunctMgr.h"
#include "VtxExprEdit.h"

class VtxErodeFunct : public VtxFunctMgr
{
	DECLARE_CLASS(VtxErodeFunct)
protected:
	void AddControlsTab(wxWindow *panel);
	// Erosion box
	ExprSliderCtrl *FillAmplSlider;
	ExprSliderCtrl *FillTransportSlider;
	ExprSliderCtrl *FillLevelSlider;
	ExprSliderCtrl *FillMarginSlider;
	// Drainage box
	ExprSliderCtrl *FillChannelsSlider;
	ExprSliderCtrl *FillOrdersSlider;
	ExprSliderCtrl *FillFalloffSlider;   // Atten
	ExprSliderCtrl *FillFreqSlider;      // Delf
	// Amplitude box
	ExprSliderCtrl *FillDrAmplSlider;    // Drainage ampl
	ExprSliderCtrl *FillDrainMixSlider;  // Erosion ampl
	wxCheckBox *m_sqr;
	wxCheckBox *m_ss;

public:
	VtxErodeFunct(wxWindow* parent, wxWindowID id,
		const wxPoint& pos  = wxDefaultPosition,
		const wxSize&  size = wxDefaultSize,
		long style = 0,
		const wxString& name = wxNotebookNameStr);

	bool Create(wxWindow* parent, wxWindowID id,
		const wxPoint& pos  = wxDefaultPosition,
		const wxSize&  size = wxDefaultSize,
		long style = 0,
		const wxString& name = wxNotebookNameStr);

	~VtxErodeFunct() {
		delete FillAmplSlider;     delete FillTransportSlider;
		delete FillLevelSlider;    delete FillMarginSlider;
		delete FillChannelsSlider; delete FillOrdersSlider;
		delete FillFalloffSlider;  delete FillFreqSlider;
		delete FillDrAmplSlider;   delete FillDrainMixSlider;
	}

	DEFINE_SLIDER_EVENTS(FillAmpl)
	DEFINE_SLIDER_EVENTS(FillTransport)
	DEFINE_SLIDER_EVENTS(FillLevel)
	DEFINE_SLIDER_EVENTS(FillMargin)
	DEFINE_SLIDER_EVENTS(FillChannels)
	DEFINE_SLIDER_EVENTS(FillOrders)
	DEFINE_SLIDER_EVENTS(FillFalloff)
	DEFINE_SLIDER_EVENTS(FillFreq)
	DEFINE_SLIDER_EVENTS(FillDrAmpl)
	DEFINE_SLIDER_EVENTS(FillDrainMix)

	void OnChangeEvent(wxCommandEvent& event) { getFunction(); }
	void setFunction(wxString);
	void getFunction();
	DECLARE_EVENT_TABLE()
};

#endif /* ERODE_FUNCT_H_ */
