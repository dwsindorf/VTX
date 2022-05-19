#ifndef FRACTAL_TABS_H_
#define FRACTAL_TABS_H_

#include "VtxTabsMgr.h"
#include "Fractal.h"

class VtxFractalTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxFractalTabs)
protected:
	TNfractal *object() 	{ return (TNfractal *)object_node->node;}

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
	VtxFractalTabs(wxWindow* parent,
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

	~VtxFractalTabs(){
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
		setObjAttributes();
    }

	void getObjAttributes();
	void setObjAttributes();

	void updateControls();
	int showMenu(bool);

	DECLARE_EVENT_TABLE()
};


#endif /*FRACTAL_TABS_H_*/
