#ifndef FUNCTMGR_
#define FUNCTMGR_

#include "VtxControls.h"

#include <wx/notebook.h>

#include "SceneClass.h"

#define LABEL    60
#ifndef LABEL2
#define LABEL2   50
#endif
#ifndef LABEL2B
#define LABEL2B  LABEL2+5
#endif
#define LABEL2S  LABEL2-15
#define VALUE    40
#define VALUE2   40
#define SLIDER   200
#define CBOX1    40
#define CSLIDER  SLIDER-CBOX1
#define VALUE3   30
#define CBOX3    30
#define SLIDER2  80
#define CSLIDER2 100-CBOX3

#ifndef DEFINE_SLIDER_EVENTS
#define DEFINE_SLIDER_EVENTS(NAME) \
    void OnEnd##NAME##Slider(wxScrollEvent& event){ \
		NAME##Slider->setValueFromSlider(); \
		getFunction(); \
	} \
    void On##NAME##Slider(wxScrollEvent& event){ \
		NAME##Slider->setValueFromSlider(); \
	} \
    void On##NAME##Text (wxCommandEvent& event){ \
		NAME##Slider->setValueFromText();\
		getFunction(); \
	}
#endif
#define SET_SLIDER_EVENTS(ENUM,CLASS,NAME) \
	EVT_COMMAND_SCROLL_THUMBRELEASE(ID_##ENUM##_SLDR,CLASS::OnEnd##NAME##Slider) \
	EVT_COMMAND_SCROLL(ID_##ENUM##_SLDR,CLASS::On##NAME##Slider) \
	EVT_TEXT_ENTER(ID_##ENUM##_TEXT,CLASS::On##NAME##Text)

class VtxFunctMgr : public wxNotebook
{
	DECLARE_CLASS(VtxFunctMgr)
protected:
	virtual void OnSliderValue(SliderCtrl *s, double &var){
    	s->setValueFromSlider();
    	var=s->getValue();
    	TheView->set_changed_render();
       	getFunction();
    }
    virtual void OnSliderText(SliderCtrl *s, double &var){
    	s->setValueFromText();
    	var=s->getValue();
    	TheView->set_changed_render();
    	getFunction();
    }
public:

	VtxFunctMgr(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	virtual void setFunction(wxString ){}
	virtual void getFunction(){}
};


#endif /*FUNCTMGR_*/
