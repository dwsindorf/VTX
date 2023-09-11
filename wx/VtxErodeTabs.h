#ifndef ERODE_TABS_H_
#define ERODE_TABS_H_

#include "VtxTabsMgr.h"
#include "VtxExprEdit.h"
#include "Erode.h"

class VtxErodeTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxErodeTabs)
protected:
	TNerode *object() 	{ return (TNerode *)object_node->node;}

	void AddControlsTab(wxWindow *panel);
	//SliderCtrl *StartSlider;
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
	VtxErodeTabs(wxWindow* parent,
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

	~VtxErodeTabs(){
		//delete StartSlider;
		delete FillAmplSlider;
		delete FillLevelSlider;
		delete FillMarginSlider;
        delete FillTransportSlider;
		delete SlopeAmplSlider;
        delete SlopeMinSlider;
		delete SlopeMaxSlider;
		delete SlopeDropSlider;
	}
	//DEFINE_SLIDER_EVENTS(Start)
	DEFINE_SLIDER_EVENTS(FillAmpl)
	DEFINE_SLIDER_EVENTS(FillLevel)
	DEFINE_SLIDER_EVENTS(FillMargin)
    DEFINE_SLIDER_EVENTS(FillTransport)

	DEFINE_SLIDER_EVENTS(SlopeAmpl)
    DEFINE_SLIDER_EVENTS(SlopeMin)
	DEFINE_SLIDER_EVENTS(SlopeMax)
	DEFINE_SLIDER_EVENTS(SlopeDrop)

    void OnChangeEvent(wxCommandEvent& event){
		setObjAttributes();
    }

	void getObjAttributes();
	void setObjAttributes();

	void updateControls();
	int showMenu(bool);

	DECLARE_EVENT_TABLE()
};


#endif /*FRACTAL_FUNCT_H_*/
