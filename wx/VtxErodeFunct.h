#ifndef ERODE_FUNCT_H_
#define ERODE_FUNCT_H_

#include "VtxFunctMgr.h"
#include "VtxExprEdit.h"

class VtxErodeFunct : public VtxFunctMgr
{
	DECLARE_CLASS(VtxErodeFunct)
protected:
	void AddControlsTab(wxWindow *panel);
	// Overlays
	ExprSliderCtrl *FillChannelsSlider;  // Start
	ExprSliderCtrl *FillOrdersSlider;    // Orders
	ExprSliderCtrl *FillFalloffSlider;   // Atten
	ExprSliderCtrl *FillFreqSlider;      // Delf
	// Amplitude
	ExprSliderCtrl *FillDrAmplSlider;    // Drainage
	ExprSliderCtrl *FillDrainMixSlider;  // Erosion
	ExprSliderCtrl *FillAmplSlider;      // Scale
	// Options
	wxCheckBox *m_neg;
	wxCheckBox *m_sqr;

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
		delete FillChannelsSlider; delete FillOrdersSlider;
		delete FillFalloffSlider;  delete FillFreqSlider;
		delete FillDrAmplSlider;   delete FillDrainMixSlider;
		delete FillAmplSlider;
	}

	DEFINE_SLIDER_EVENTS(FillChannels)
	DEFINE_SLIDER_EVENTS(FillOrders)
	DEFINE_SLIDER_EVENTS(FillFalloff)
	DEFINE_SLIDER_EVENTS(FillFreq)
	DEFINE_SLIDER_EVENTS(FillDrAmpl)
	DEFINE_SLIDER_EVENTS(FillDrainMix)
	DEFINE_SLIDER_EVENTS(FillAmpl)

	void OnChangeEvent(wxCommandEvent& event) { getFunction(); }
	void setFunction(wxString);
	void getFunction();
	DECLARE_EVENT_TABLE()
};

#endif /* ERODE_FUNCT_H_ */
