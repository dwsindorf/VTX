#ifndef NOISE_FUNCT_H_
#define NOISE_FUNCT_H_

#include "VtxFunctMgr.h"

class VtxNoiseFunct : public VtxFunctMgr
{
	DECLARE_CLASS(VtxNoiseFunct)

protected:
	void AddControlsTab(wxWindow *panel);
	void AddTypeTab(wxWindow *panel);
	wxString getTypeStr();
	void setTypeControls(int type);
	SliderCtrl *StartSlider;
	SliderCtrl *OrdersSlider;
	SliderCtrl *HSlider;
	SliderCtrl *LSlider;
	SliderCtrl *HomogSlider;
	SliderCtrl *AmplSlider;
	SliderCtrl *OffsetSlider;
	SliderCtrl *RoundSlider;
	SliderCtrl *ClampSlider;
	SliderCtrl  *RateSlider;
	wxChoice    *rate_scale;
	
	wxChoice   *m_domain;
	wxChoice   *m_map;
	wxChoice   *m_mode;
	wxChoice   *m_noisetype;
	wxCheckBox *m_uns;
	wxCheckBox *m_sqr;
	wxCheckBox *m_neg;
	wxCheckBox *m_norm;
	wxCheckBox *m_scale;
	wxCheckBox *m_lod;
	wxCheckBox *m_abs;
	wxCheckBox *m_animate;

	TNnoise *noise;
	
	void getRate();
	void setRate();
	double rate;

public:
	VtxNoiseFunct(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxNoiseFunct(){
		delete StartSlider;
		delete OrdersSlider;
		delete HSlider;
		delete LSlider;
		delete HomogSlider;
		delete AmplSlider;
		delete OffsetSlider;
		delete RoundSlider;
		delete ClampSlider;
		delete RateSlider;

	}
	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);

	void setFunction(wxString);
	void getFunction();

    void OnChangeEvent(wxCommandEvent& event);
	void OnRateSlider(wxScrollEvent &event) {
		RateSlider->setValueFromSlider();
		getFunction();
	}
	void OnRateText(wxCommandEvent &event) {
		getFunction();
	}

	void OnRateScale(wxCommandEvent &event) {
		getFunction();
	}

	DEFINE_SLIDER_EVENTS(Start)
	DEFINE_SLIDER_EVENTS(Orders)
	DEFINE_SLIDER_EVENTS(H)
	DEFINE_SLIDER_EVENTS(L)
	DEFINE_SLIDER_EVENTS(Homog)
	DEFINE_SLIDER_EVENTS(Ampl)
	DEFINE_SLIDER_EVENTS(Round)
	DEFINE_SLIDER_EVENTS(Offset)
	DEFINE_SLIDER_EVENTS(Clamp)

	DECLARE_EVENT_TABLE()
};


#endif /*NOISE_FUNCT_H_*/
