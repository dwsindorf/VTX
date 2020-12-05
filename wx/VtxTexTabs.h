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
	double bias;
	double bump_orders;
	double bump_damp;

	double orders;
	double orders_delta;
	double orders_atten;
	uint   interp_state;
	uint clamp   		: 1;
	uint norm   		: 1;
	uint alpha_enable   : 1;
	uint tex_enable   	: 1;
	uint bump_enable   	: 1;

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

	void makeFileList();

    void invalidateObject(){
    	object()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
    void invalidateTexture(){
    	if(texture())
        texture()->invalidate();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
    void invalidateRender(){
      	TheView->set_changed_render();
    }

	void setInvalid(){
		//object()->invalid=true;
	}
	wxArrayString files;
	wxChoice *choices;

	tex_state state[4];

	int m_image_type;
	bool m_invert;
	int m_type;

	wxString m_name;

	SliderCtrl *StartSlider;
	SliderCtrl *OrdersSlider;
	SliderCtrl *AlphaSlider;
	SliderCtrl *BumpAmpSlider;
	SliderCtrl *BumpDampSlider;
	SliderCtrl *BiasSlider;
	SliderCtrl *OrdersDeltaSlider;
	SliderCtrl *OrdersAttenSlider;
	VtxImageWindow *m_image_window;

	ExprTextCtrl *m_amp_expr;
	ExprTextCtrl *m_alpha_expr;

	wxBitmapButton *m_edit_button;
	wxRadioButton *m_2D_button;
	wxRadioButton *m_1D_button;
	wxRadioButton *m_img_button;
	wxRadioButton *m_map_button;

	wxCheckBox *m_tex_check;
	wxCheckBox *m_bump_check;
	wxRadioBox *interp_mode;

	wxCheckBox *m_clamp_check;
	wxCheckBox *m_norm_check;

	void getObjAttributes();
	void setObjAttributes();
	void set_start(TNode *node);
	void set_ampl(TNode *node);
	void set_bump(TNode *node);
	void set_bias(TNode *node);
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
	DEFINE_SLIDER_EVENTS(Start)
	DEFINE_SLIDER_EVENTS(Bias)
	DEFINE_SLIDER_EVENTS(Alpha)
	DEFINE_SLIDER_EVENTS(Orders)
	DEFINE_SLIDER_EVENTS(OrdersDelta)
	DEFINE_SLIDER_EVENTS(OrdersAtten)
	DEFINE_SLIDER_EVENTS(BumpAmp)
	DEFINE_SLIDER_EVENTS(BumpDamp)

    void OnFileSelect(wxCommandEvent& event);
    void OnShowBands(wxCommandEvent& event);
    void OnShowImage(wxCommandEvent& event);
    void OnShowImport(wxCommandEvent& event);
    void OnShowMap(wxCommandEvent& event);
    void OnImageEdit(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};
#endif /*IMAGETABS_H_*/
