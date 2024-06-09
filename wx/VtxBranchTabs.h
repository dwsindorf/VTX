#ifndef BRANCHTABS_H_
#define BRANCHTABS_H_

#include "VtxTabsMgr.h"
#include <Plants.h>

//	if(n>0)max_level=arg[0];
//	if(n>1)max_splits=arg[1];
//	if(n>2)length=arg[2];
//	if(n>3)width=arg[3];
//	if(n>4)randomness=arg[4];
//	if(n>5)divergence=arg[5];
//	if(n>6)flatness=arg[6];
//	if(n>7)width_taper=arg[7];
//	if(n>8)length_taper=arg[8];	
//	if(n>9)first_bias=arg[9];
//	if(n>10)min_level=arg[10];

class VtxBranchTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxBranchTabs)

protected:
	TNBranch *object() 	{ return (TNBranch *)object_node->node;}
	void AddPropertiesTab(wxWindow *panel);
    void invalidateObject(){
    	object()->invalidate();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();
	
	wxChoice *m_min_level;
	wxChoice *m_max_level;

	SliderCtrl   *SplitsSlider;
	SliderCtrl   *LengthSlider;
	SliderCtrl   *LengthTaperSlider;
	SliderCtrl   *WidthSlider;
	SliderCtrl   *WidthTaperSlider;
	SliderCtrl   *RandSlider;
	SliderCtrl   *FirstBiasSlider;
	SliderCtrl   *DivergenceSlider;
	SliderCtrl   *FlatnessSlider;
	
	wxCheckBox *m_from_end;

public:
	VtxBranchTabs(wxWindow* parent,
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

	~VtxBranchTabs(){}
	void updateControls();
	int showMenu(bool);
	
	DEFINE_SLIDER_EVENTS(Splits)
	DEFINE_SLIDER_EVENTS(Length)
	DEFINE_SLIDER_EVENTS(Width)
	DEFINE_SLIDER_EVENTS(Rand)
	DEFINE_SLIDER_EVENTS(Divergence)
	DEFINE_SLIDER_EVENTS(Flatness)
	DEFINE_SLIDER_EVENTS(WidthTaper)
	DEFINE_SLIDER_EVENTS(LengthTaper)
	DEFINE_SLIDER_EVENTS(FirstBias)

    void OnChanged(wxCommandEvent& event){
		setObjAttributes();
    }
    void OnChangedLevels(wxCommandEvent& event);
    wxString exprString();
	DECLARE_EVENT_TABLE()
};
#endif /*BRANCHTABS_H_*/
