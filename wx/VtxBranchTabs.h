#ifndef BRANCHTABS_H_
#define BRANCHTABS_H_

#include "VtxTabsMgr.h"
#include "VtxBitmapPanel.h"
#include <wx/clrpicker.h>

#include <Plants.h>

class VtxBranchTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxBranchTabs)

protected:
	wxString image_name;
	wxArrayString files;
	VtxBitmapPanel *image_window;
	wxStaticBoxSizer *image_sizer;
	wxString image_path;


	TNBranch *object() 	{ return (TNBranch *)object_node->node;}
	void AddPropertiesTab(wxWindow *panel);
	void AddImageTab(wxWindow *panel);
	void AddColorTab(wxWindow *panel);
	void makeFileList(wxString);
	void setImagePanel();
    void invalidateObject(){
    	object()->invalidate();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
	void getObjAttributes();
	void setObjAttributes();
	
    void OnChangedFile(wxCommandEvent& event);

	
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
	wxCheckBox *m_enable;
	wxCheckBox *m_tex_enable;
	wxCheckBox *m_col_enable;
	wxChoice *choices;
	
	ExprTextCtrl *m_r_expr;
	ExprTextCtrl *m_g_expr;
	ExprTextCtrl *m_b_expr;
	ExprTextCtrl *m_a_expr;
	wxColourPickerCtrl *m_color_chooser;
	wxButton *m_revert;
	bool revert_needed;
	wxString m_last_expr;
	    
    wxString getColorExpr();
	void setColorFromExpr();
	void setExprFromColor();
	TNcolor* getColorFromExpr();

	void saveLastExpr();
	void restoreLastExpr();

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
    void OnChangedExpr(wxCommandEvent& event);
    void OnChangedColor(wxColourPickerEvent& event);
    void OnRevert(wxCommandEvent& event);

    wxString exprString();
	DECLARE_EVENT_TABLE()
};
#endif /*BRANCHTABS_H_*/