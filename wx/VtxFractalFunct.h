#ifndef FRACTAL_FUNCT_H_
#define FRACTAL_FUNCT_H_

#include "VtxFunctMgr.h"
#include "VtxExprEdit.h"

class VtxFractalFunct : public VtxFunctMgr
{
	DECLARE_CLASS(VtxFractalFunct)
protected:
	void AddControlsTab(wxWindow *panel);
	ExprSliderCtrl *StartSlider;
	ExprSliderCtrl *OrdersSlider;
	ExprSliderCtrl *BuildSlider;
	ExprSliderCtrl *ErodeSlider;
	ExprSliderCtrl *SlopeSlider;
	ExprSliderCtrl *ClipSlider;
	ExprSliderCtrl *LimitSlider;
	ExprSliderCtrl *BiasSlider;
	ExprSliderCtrl *HtMaxSlider;
	ExprSliderCtrl *HtValSlider;
	wxCheckBox *m_sqr;
	wxCheckBox *m_ss;

	int nargs;

public:
	VtxFractalFunct(wxWindow* parent,
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

	~VtxFractalFunct(){
		delete StartSlider;
		delete OrdersSlider;
		delete BuildSlider;
		delete ErodeSlider;
		delete SlopeSlider;
		delete ClipSlider;
		delete LimitSlider;
		delete BiasSlider;
		delete HtMaxSlider;
		delete HtValSlider;

	}
	DEFINE_SLIDER_EVENTS(Start)
	DEFINE_SLIDER_EVENTS(Orders)
	DEFINE_SLIDER_EVENTS(Build)
	DEFINE_SLIDER_EVENTS(Erode)
	DEFINE_SLIDER_EVENTS(Slope)
	DEFINE_SLIDER_EVENTS(Clip)
	DEFINE_SLIDER_EVENTS(Limit)
	DEFINE_SLIDER_EVENTS(Bias)
	DEFINE_SLIDER_EVENTS(HtMax)
	DEFINE_SLIDER_EVENTS(HtVal)

    void OnChangeEvent(wxCommandEvent& event){
    	getFunction();
    }

	void setFunction(wxString);
	void getFunction();

	DECLARE_EVENT_TABLE()
};


#endif /*FRACTAL_FUNCT_H_*/
