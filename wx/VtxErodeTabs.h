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

	ExprSliderCtrl *FillAmplSlider;      // Depth
	ExprSliderCtrl *FillTransportSlider; // Power
	ExprSliderCtrl *FillLevelSlider;     // Start
	ExprSliderCtrl *FillMarginSlider;    // Range

	wxCheckBox *m_sqr;
	wxCheckBox *m_ss;

public:
	VtxErodeTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos  = wxDefaultPosition,
		const wxSize&  size = wxDefaultSize,
		long style          = 0,
		const wxString& name = wxNotebookNameStr);

	bool Create(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos  = wxDefaultPosition,
		const wxSize&  size = wxDefaultSize,
		long style          = 0,
		const wxString& name = wxNotebookNameStr);

	~VtxErodeTabs() {
		delete FillAmplSlider;
		delete FillTransportSlider;
		delete FillLevelSlider;
		delete FillMarginSlider;
	}

	DEFINE_SLIDER_EVENTS(FillAmpl)
	DEFINE_SLIDER_EVENTS(FillTransport)
	DEFINE_SLIDER_EVENTS(FillLevel)
	DEFINE_SLIDER_EVENTS(FillMargin)

	void OnChangeEvent(wxCommandEvent& event) { setObjAttributes(); }

	void getObjAttributes();
	void setObjAttributes();
	void updateControls();
	int  showMenu(bool);

	DECLARE_EVENT_TABLE()
};

#endif /* ERODE_TABS_H_ */
