#ifndef CLOUDSTABS_H_
#define CLOUDSTABS_H_

#include "VtxTabsMgr.h"
#include <wx/bitmap.h>
#include "VtxBitmapPanel.h"

class VtxCloudsTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxCloudsTabs)
protected:
	//TextCtrl *sprites_file;
	wxChoice *sprites_dim;

	SliderCtrl *CellSizeSlider;
	SliderCtrl *HeightSlider;
	SliderCtrl *TiltSlider;
	SliderCtrl *DaySlider;
	SliderCtrl *RotPhaseSlider;

	SliderCtrl *ShineSlider;
	ColorSlider *AmbientSlider;
	ColorSlider *SpecularSlider;
	ColorSlider *EmissionSlider;
	ColorSlider *DiffuseSlider;

	ExprTextCtrl *top_expr;
	ExprTextCtrl *bottom_expr;
	ExprTextCtrl *type_expr;

	SliderCtrl *CminSlider;
	SliderCtrl *CmaxSlider;
	SliderCtrl *SmaxSlider;
	SliderCtrl *CrotSlider;

	SliderCtrl *NumSpritesSlider;
	SliderCtrl *DiffusionSlider;

	wxNotebookPage *geometry_page;
	wxNotebookPage *sprites_page;

	wxArrayString files;
	wxChoice *choices;

	VtxBitmapPanel *image_window;
	wxStaticBoxSizer *image_sizer;
	wxString image_path;
	VtxBitmapPanel *cell_window;

	bool changed_cell_expr;
	uint image_dim;
	wxString image_name;

	void AddObjectTab(wxWindow *panel);
	void AddLightingTab(wxWindow *panel);
	void AddGeometryTab(wxWindow *panel);
	void AddImagesTab(wxWindow *panel);
	void add3DTabs();

	CloudLayer *object() 		{ return ((CloudLayer*)(object_node->node));}
	double parentSize(){
		return ((Planet*)(object()->getParent()))->size;
	}
    void OnChangedExpr(wxCommandEvent& event);
    void OnChangedTypeExpr(wxCommandEvent& event);

    void OnChangedFile(wxCommandEvent& event);
    void OnFileSelect(wxCommandEvent& event);
    void OnDimSelect(wxCommandEvent& event);

    void makeFileList(int,char*);
	wxString getCloudsExpr();
	void setCloudsExpr();
	void getObjAttributes();
	void setObjAttributes();
	void setImagePanel();
	void invalidateObject();
	void setTypePanel();


public:
	VtxCloudsTabs(wxWindow* parent,
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
	~VtxCloudsTabs();
	int showMenu(bool);
	void updateControls();

    void OnHeightSlider(wxScrollEvent& event){
    	double val;
    	OnSliderValue(HeightSlider, val);
    	object()->ht=MILES*val;
    	object()->size=object()->ht+parentSize();
    	invalidateObject();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
    void OnHeightText(wxCommandEvent& event){
    	double val;
    	OnSliderText(HeightSlider, val);
    	object()->ht=MILES*val;
    	object()->size=object()->ht+parentSize();
    	invalidateObject();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }

    void OnEndCellSizeSlider(wxScrollEvent& event){
    	CloudLayer *obj=object();
    	OnSliderValue(CellSizeSlider, obj->detail);
        obj->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
    }
    void OnCellSizeSlider(wxScrollEvent& event){
    	CellSizeSlider->setValueFromSlider();
    }
    void OnCellSizeText(wxCommandEvent& event){
    	CloudLayer *obj=object();
    	OnSliderText(CellSizeSlider, obj->detail);
        obj->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
    }
    void OnEndRotPhaseSlider(wxScrollEvent& event){
		OnSliderValue( RotPhaseSlider, object()->rot_phase);
		object()->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
	}
    void OnRotPhaseSlider(wxScrollEvent& event){
		RotPhaseSlider->setValueFromSlider();
	}
    void OnRotPhaseText (wxCommandEvent& event){
		OnSliderText(RotPhaseSlider, object()->rot_phase);
		object()->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
	}
    void OnEndDaySlider(wxScrollEvent& event){
		OnSliderValue(DaySlider, object()->day);
		object()->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
	}
    void OnDaySlider(wxScrollEvent& event){
    	DaySlider->setValueFromSlider();
	}
    void OnDayText (wxCommandEvent& event){
		OnSliderText(DaySlider, object()->day);
		object()->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
	}

	DEFINE_SLIDER_VAR_EVENTS(Tilt,object()->tilt)
	DEFINE_SLIDER_VAR_EVENTS(Shine,object()->shine)
    DEFINE_SLIDER_VAR_EVENTS(NumSprites,object()->num_sprites)
	DEFINE_SLIDER_VAR_EVENTS(Diffusion,object()->diffusion)
	DEFINE_SLIDER_VAR_EVENTS(Cmin,object()->cmin)
	DEFINE_SLIDER_VAR_EVENTS(Cmax,object()->cmax)
	DEFINE_SLIDER_VAR_EVENTS(Smax,object()->smax)
	DEFINE_SLIDER_VAR_EVENTS(Crot,object()->crot)
	DEFINE_COLOR_VAR_EVENTS(Ambient,object()->ambient)
	DEFINE_COLOR_VAR_EVENTS(Specular,object()->specular)
	DEFINE_COLOR_VAR_EVENTS(Diffuse,object()->diffuse)
	DEFINE_COLOR_VAR_EVENTS(Emission,object()->emission)

	//void OnAddItem(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif /*CLOUDSTABS*/
