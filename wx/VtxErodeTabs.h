#ifndef ERODE_TABS_H_
#define ERODE_TABS_H_

#include "VtxTabsMgr.h"
#include "VtxExprEdit.h"
#include "Erode.h"

class VtxErodeTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxErodeTabs)
protected:
	TNerode *object()  { return (TNerode *)object_node->node; }
	void AddControlsTab(wxWindow *panel);

	// Erosion box
	ExprSliderCtrl *FillAmplSlider;       // Depth
	ExprSliderCtrl *FillTransportSlider;  // Power
	ExprSliderCtrl *FillLevelSlider;      // Start
	ExprSliderCtrl *FillMarginSlider;     // Range
	// Drainage box (was Overlays)
	ExprSliderCtrl *FillChannelsSlider;   // Start
	ExprSliderCtrl *FillOrdersSlider;     // Orders
	ExprSliderCtrl *FillFalloffSlider;    // Atten
	ExprSliderCtrl *FillFreqSlider;       // Delf
	// Amplitude box (was Drainage)
	ExprSliderCtrl *FillDrAmplSlider;     // Drainage ampl
	ExprSliderCtrl *FillDrainMixSlider;   // Erosion ampl

	wxCheckBox *m_inv;
	wxCheckBox *m_sqr;
	wxCheckBox *m_ss;

public:
	VtxErodeTabs(wxWindow* parent, wxWindowID id,
		const wxPoint& pos  = wxDefaultPosition,
		const wxSize&  size = wxDefaultSize,
		long style = 0,
		const wxString& name = wxNotebookNameStr);

	bool Create(wxWindow* parent, wxWindowID id,
		const wxPoint& pos  = wxDefaultPosition,
		const wxSize&  size = wxDefaultSize,
		long style = 0,
		const wxString& name = wxNotebookNameStr);

	~VtxErodeTabs() {
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

	void OnChangeEvent(wxCommandEvent& event) { setObjAttributes(); }
	void getObjAttributes();
	void setObjAttributes();
	void updateControls();
	int  showMenu(bool);
	DECLARE_EVENT_TABLE()
};

#endif /* ERODE_TABS_H_ */
