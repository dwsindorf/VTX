#ifndef IMAGETABS_H_
#define IMAGETABS_H_

#include "VtxTabsMgr.h"
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <ImageClass.h>
#include <TextureClass.h>
#include <defs.h>
#include "VtxImageWindow.h"
#include "VtxExprEdit.h"

typedef struct tex_state {
	wxString file;
	wxString amp_expr;
	wxString alpha_expr;
	double start;
	double alpha;
	double bump;
	double hmap_bias;
	double hmap;
	double bias;
	double bump_orders;
	double bump_damp;
	double phi_bias;
	double ht_bias;
	double bump_bias;
	double slope_bias;

	double orders;
	double orders_delta;
	double orders_atten;
	uint   interp_state;
	uint clamp   		: 1;
	uint norm   		: 1;
	uint randomize   	: 1;
	uint alpha_enable   : 1;
	uint tex_enable   	: 1;
	uint bump_enable   	: 1;
	uint hmap_enable   	: 1;
	uint tilt_enable    : 1;

} tex_state;
class VtxTexTabs : public VtxTabsMgr

{

	DECLARE_CLASS(VtxTexTabs)
 
protected:
	void saveState(int which);
	void restoreState(int which);
	void AddImageTab(wxWindow *panel);
	void AddFilterTab(wxWindow *panel);
	TNtexture *object() 	{ return ((TNtexture *)object_node->node);}
	Texture *texture() 		{ return object()->texture;}
	void setHtmap();

	void makeFileList();

    void invalidateObject(){
    	object()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_render();
    	TheScene->rebuild();
    }
    void invalidateTexture(){
    	if(texture())
    		texture()->invalidate();
    	TheView->set_changed_render();
    	TheScene->rebuild();
    }
    void invalidateRender(){
      	TheView->set_changed_render();
    }

	void setInvalid(){
		//object()->invalid=true;
	}
	void setSelected(TreeNode *s);

	wxArrayString files;
	wxChoice *choices;
	wxChoice *mode;

	tex_state state[5];

	int m_image_type;
	bool m_invert;
	int m_type;
	bool d_data;

	wxString m_name;

	SliderCtrl *StartSlider;
	SliderCtrl *OrdersSlider;
	SliderCtrl *AlphaSlider;
	SliderCtrl *BumpAmpSlider;
	SliderCtrl *BumpDampSlider;
	SliderCtrl *HmapAmpSlider;
	SliderCtrl *HmapBiasSlider;
	SliderCtrl *PhiBiasSlider;
	SliderCtrl *HtBiasSlider;
	SliderCtrl *BumpBiasSlider;
	SliderCtrl *SlopeBiasSlider;

	SliderCtrl *BiasSlider;
	SliderCtrl *OrdersDeltaSlider;
	SliderCtrl *OrdersAttenSlider;
	VtxImageWindow *m_image_window;

	ExprTextCtrl *m_amp_expr;
	ExprTextCtrl *m_alpha_expr;

	wxBitmapButton *m_edit_button;
//	wxRadioButton *m_2D_button;
//	wxRadioButton *m_1D_button;
//	wxRadioButton *m_img_button;
//	wxRadioButton *m_map_button;

	wxCheckBox *m_tex_check;
	wxCheckBox *m_bump_check;
	wxCheckBox *m_hmap_check;

	wxRadioBox *interp_mode;

	wxCheckBox *m_clamp_check;
	wxCheckBox *m_norm_check;
	wxCheckBox *m_rand_check;
	wxCheckBox *m_tilt_check;

	void getObjAttributes();
	void setObjAttributes();
	void set_start(TNode *node);
	void set_ampl(TNode *node);
	void set_bump(TNode *node);
	void set_bias(TNode *node);
	void set_hmap(TNode *node);
	void set_hmap_bias(TNode *node);
	void set_pbias(TNode *node);
	void set_hbias(TNode *node);
	void set_bbias(TNode *node);
	void set_sbias(TNode *node);

	void set_orders(TNode *node);
	void set_orders_delta(TNode *node);
	void set_orders_atten(TNode *node);
	void set_damp(TNode *node);
	void set_image();
	void get_files(int);

public:
	VtxTexTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxTexTabs(){
		delete StartSlider;
		delete OrdersSlider;
		delete AlphaSlider;
		delete BumpAmpSlider;
		delete BumpDampSlider;
		delete BiasSlider;
		delete OrdersDeltaSlider;
		delete OrdersAttenSlider;
		delete HmapAmpSlider;
		delete HmapBiasSlider;
		delete PhiBiasSlider;
		delete HtBiasSlider;
		delete BumpBiasSlider;
		delete SlopeBiasSlider;
		delete m_amp_expr;
		delete m_alpha_expr;
	}
	bool Create(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	void updateControls();
	int showMenu(bool);

    void OnTextEnter(wxCommandEvent& event);

    void OnTexChanged(wxCommandEvent& event){
		setObjAttributes();
    }
    void OnModeChanged(wxCommandEvent& event){
    	bool hm=m_hmap_check->GetValue();
    	if(hm){
    		texture()->invalidate();
    		invalidateObject();
    	}
    	else
    		setObjAttributes();
    }
    void OnHmapChanged(wxCommandEvent& event){
		bool hm=m_hmap_check->GetValue();
		texture()->hmap_active=hm;
		texture()->invalidate();
		invalidateObject();
    }

	void OnEndHmapAmpSlider(wxScrollEvent &event) {
		HmapAmpSlider->setValueFromSlider();
		setHtmap();
	}
	void OnHmapAmpSlider(wxScrollEvent &event) {
		HmapAmpSlider->setValueFromSlider();
	}
	void OnHmapAmpText(wxCommandEvent &event) {
		HmapAmpSlider->setValueFromText();
		setHtmap();
	}
	void OnEndHmapBiasSlider(wxScrollEvent &event) {
		HmapBiasSlider->setValueFromSlider();
		double val = HmapBiasSlider->getValue();
		texture()->hmap_bias=val;
		texture()->invalidate();
		bool hm=m_hmap_check->GetValue();
		if(hm)
			invalidateObject();

	}
	void OnHmapBiasSlider(wxScrollEvent &event) {
		HmapBiasSlider->setValueFromSlider();
	}
	void OnHmapBiasText(wxCommandEvent &event) {
		HmapBiasSlider->setValueFromText();
		double val = HmapBiasSlider->getValue();
		texture()->hmap_bias=val;
		texture()->invalidate();
		bool hm=m_hmap_check->GetValue();
		if(hm)
			invalidateObject();
	}

	void OnModeSelect(wxCommandEvent& event);
	DEFINE_SLIDER_EVENTS(Start)
	DEFINE_SLIDER_EVENTS(Bias)
	DEFINE_SLIDER_EVENTS(Alpha)
	DEFINE_SLIDER_EVENTS(Orders)
	DEFINE_SLIDER_EVENTS(OrdersDelta)
	DEFINE_SLIDER_EVENTS(OrdersAtten)
	DEFINE_SLIDER_EVENTS(BumpAmp)
	DEFINE_SLIDER_EVENTS(BumpDamp)
	DEFINE_SLIDER_EVENTS(PhiBias)
	DEFINE_SLIDER_EVENTS(HtBias)
	DEFINE_SLIDER_EVENTS(BumpBias)
	DEFINE_SLIDER_EVENTS(SlopeBias)

    void OnFileSelect(wxCommandEvent& event);
    void OnImageEdit(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};
#endif /*IMAGETABS_H_*/
