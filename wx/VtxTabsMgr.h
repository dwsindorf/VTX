#ifndef TABSMGR_
#define TABSMGR_

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/clrpicker.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <wx/xml/xml.h>
#include <wx/gdicmn.h>
#include <wx/notebook.h>
#include <wx/dynarray.h>

#include "VtxControls.h"
#include "SceneClass.h"

#define TABS_BORDER 10
#ifndef TABS_HEIGHT
#define TABS_HEIGHT 320
#endif
#ifndef TABS_WIDTH
#define TABS_WIDTH 420
#endif
#define LABEL    60
#ifndef LABEL2
#define LABEL2   60
#endif
#ifndef LABEL2B
#define LABEL2B  LABEL2+15
#endif
#define LABEL2S  LABEL2-15

#define VALUE    40
#define VALUE2   40
#define SLIDER   260
#define CBOX1    40
#define CSLIDER  SLIDER-CBOX1
#define VALUE3   30
#define CBOX3    30
#define SLIDER2  80
#define CSLIDER2 100-CBOX3

#define DEFINE_SLIDER_EVENTS(NAME) \
    void OnEnd##NAME##Slider(wxScrollEvent& event){ \
		NAME##Slider->setValueFromSlider(); \
		setObjAttributes(); \
	} \
    void On##NAME##Slider(wxScrollEvent& event){ \
		NAME##Slider->setValueFromSlider(); \
	} \
    void On##NAME##Text (wxCommandEvent& event){ \
		NAME##Slider->setValueFromText();\
		setObjAttributes(); \
	}

#define DEFINE_SLIDER_ADAPT_EVENTS(NAME) \
    void OnEnd##NAME##Slider(wxScrollEvent& event){ \
		NAME##Slider->setValueFromSlider(); \
		changing=false; \
		setObjAttributes(); \
    	TheView->set_changed_detail(); \
	} \
    void On##NAME##Slider(wxScrollEvent& event){ \
		if(TheScene->motion()) changing=true; \
		NAME##Slider->setValueFromSlider(); \
	} \
    void On##NAME##Text (wxCommandEvent& event){ \
		if(TheScene->motion()) changing=true; \
		NAME##Slider->setValueFromText();\
		setObjAttributes(); \
    	TheView->set_changed_detail(); \
	}

#define DEFINE_SLIDER_RENDER_EVENTS(NAME) \
    void OnEnd##NAME##Slider(wxScrollEvent& event){ \
	    NAME##Slider->setValueFromSlider(); \
		changing=false; \
    	invalidateRender(); \
    	TheView->set_changed_render(); \
	} \
    void On##NAME##Slider(wxScrollEvent& event){ \
		if(TheScene->motion()) changing=true; \
	    NAME##Slider->setValueFromSlider(); \
    	invalidateRender(); \
    	TheView->set_changed_render(); \
	} \
    void On##NAME##Text (wxCommandEvent& event){ \
		if(TheScene->motion()) changing=true; \
	    NAME##Slider->setValueFromText(); \
     	invalidateRender(); \
    	TheView->set_changed_render(); \
	} \

#define DEFINE_SLIDER_VAR_EVENTS(NAME,VAR) \
    void OnEnd##NAME##Slider(wxScrollEvent& event){ \
		OnSliderValue( NAME##Slider, VAR); \
		changing=false; \
	} \
    void On##NAME##Slider(wxScrollEvent& event){ \
		if(TheScene->motion()) changing=true; \
		NAME##Slider->setValueFromSlider(); \
	} \
    void On##NAME##Text (wxCommandEvent& event){ \
		if(TheScene->motion()) changing=true; \
		OnSliderText( NAME##Slider, VAR);\
	}

#define DEFINE_COLOR_VAR_EVENTS(NAME,VAR) \
    void OnEnd##NAME##Slider(wxScrollEvent& event){ \
		OnColorSlider( NAME##Slider, VAR); \
		changing=false; \
	} \
    void On##NAME##Slider(wxScrollEvent& event){ \
		if(TheScene->motion()) changing=true; \
		NAME##Slider->setValueFromSlider(); \
	} \
    void On##NAME##Text (wxCommandEvent& event){ \
		if(TheScene->motion()) changing=true; \
		OnColorText( NAME##Slider, VAR); \
	} \
    void On##NAME##Color(wxColourPickerEvent& WXUNUSED(event)){\
		OnColorValue(NAME##Slider, VAR); \
	}

#define SET_SLIDER_EVENTS(ENUM,CLASS,NAME) \
	EVT_COMMAND_SCROLL_THUMBRELEASE(ID_##ENUM##_SLDR,CLASS::OnEnd##NAME##Slider) \
	EVT_COMMAND_SCROLL(ID_##ENUM##_SLDR,CLASS::On##NAME##Slider) \
	EVT_TEXT_ENTER(ID_##ENUM##_TEXT,CLASS::On##NAME##Text)

#define SET_COLOR_EVENTS(ENUM,CLASS,NAME) \
	SET_SLIDER_EVENTS(ENUM,CLASS,NAME) \
	EVT_COLOURPICKER_CHANGED(ID_##ENUM##_COLOR,CLASS::On##NAME##Color)

#define SET_FILE_EVENTS(CLASS) \
	    EVT_MENU(TABS_SAVE,CLASS::OnSave) \
		EVT_MENU_RANGE(TABS_REPLACE,TABS_REPLACE+TABS_MAX_IDS,CLASS::OnOpen) \
		EVT_MENU_RANGE(TABS_REMOVE,TABS_REMOVE+TABS_MAX_IDS,CLASS::OnRemove) \

enum{
	TABS_MAX_IDS  			= 0x0100,
	TABS_ID_MASK 			= 0x00FF,
	TABS_ACTION_MASK 		= 0xFF00,
	TABS_NONE               = 0x0000,
	TABS_ADD    			= 0x0100,
	TABS_ADD_CHILD  		= 0x0100,
	TABS_ADD_SIBLING  		= 0x0300,
	TABS_INSERT_CHILD  		= 0x0500,
	TABS_OPEN  				= 0x0800,

	TABS_DELETE  			= 0x0800,
	TABS_SAVE  				= 0x1000,
	TABS_REPLACE  			= 0x1800,
	TABS_REMOVE  			= 0x2000,
	TABS_ENABLE   			= 0x3000,
	TABS_VIEWOBJ  			= 0x4000,
};

WX_DEFINE_ARRAY_PTR(wxCheckBox*, CheckBoxList);

class VtxTabsMgr : public wxNotebook
{
	DECLARE_CLASS(VtxTabsMgr)
protected:
	TreeNode *object_node;
	TextCtrl *object_name;
	StaticTextCtrl *object_type;

	bool update_needed;
	bool changing;


	wxMenu *getAddMenu(NodeIF *);
	//wxMenu *getOpenMenu(NodeIF *);
	wxMenu *getRemoveMenu(NodeIF *);

	~VtxTabsMgr(){
		delete object_name;
	}

	bool is_viewobj(){
		return TheScene->viewobj==object_node->node?true:false;
	}

	virtual void OnSliderValue(SliderCtrl *s, double &var){
    	s->setValueFromSlider();
    	var=s->getValue();
    	TheView->set_changed_render();
    }
    virtual void OnSliderText(SliderCtrl *s, double &var){
    	s->setValueFromText();
    	var=s->getValue();
    	TheView->set_changed_render();
    }
    void updateSlider(SliderCtrl *s, double var){
    	s->setValue(var);
    }

    virtual void OnColorSlider(ColorSlider *s, Color &col) 	{
    	TheView->set_changed_render();
    	s->setValueFromSlider();
    	double a=s->getValue();
    	col.set_alpha(a);
    }
    virtual void OnColorText(ColorSlider *s, Color &col)	{
    	TheView->set_changed_render();
    	s->setValueFromText();
    	double a=s->getValue();
    	col.set_alpha(a);
    }
    virtual void OnColorValue(ColorSlider *s,  Color &col){
    	TheView->set_changed_render();
        float v[4];
     	s->getColor(v);
     	col.set_red((double)v[0]);
     	col.set_green((double)v[1]);
     	col.set_blue((double)v[2]);
    }
    void updateColor(ColorSlider *s, Color &col){
	    float v[4];
	    v[0]=col.red();
	    v[1]=col.green();
	    v[2]=col.blue();
	    v[3]=col.alpha();
		s->setColor(v);
    }

    virtual bool updateOk(){
    	return true;
    }

    virtual void OnEnable(wxCommandEvent& event);

    virtual void OnUpdateEnable(wxUpdateUIEvent& event) {
    	if(object_node)
     	event.Check(isEnabled());
    }
    virtual void OnDelete(wxCommandEvent& event){
     	menu_action=TABS_DELETE;
    }
    virtual void OnSave(wxCommandEvent& event){
     	menu_action=TABS_SAVE;
    }
	virtual void getObjAttributes();
	virtual void setObjAttributes();
	virtual void invalidateObject();
	virtual void invalidateRender();
	virtual void invalidate();

public:
	int menu_action;

	TreeNode *getTreeNode()     { return object_node;}
	NodeIF * getObject()        { return object_node->node;}
	bool isEnabled()     		{ return object_node->isEnabled();}
	void setEnabled(bool b) 	{ object_node->setEnabled(b);}

	VtxTabsMgr(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);

	virtual void updateControls(){}

	virtual int showMenu(bool expanded){
		menu_action=TABS_NONE;
		return menu_action;
	}
	virtual void setSelected(TreeNode *s){
		update_needed=true;
		object_node=s;
	}
    virtual void OnNameText(wxCommandEvent& event);
	virtual void OnAddItem(wxCommandEvent& event){
		menu_action=event.GetId();
	}
    virtual void OnOpen(wxCommandEvent& event){
     	menu_action=event.GetId();
    }
    virtual void OnRemove(wxCommandEvent& event){
     	menu_action=TABS_REMOVE;
    }
	//DECLARE_EVENT_TABLE()

};


#endif /*TABSMGR_*/
