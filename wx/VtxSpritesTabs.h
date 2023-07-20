/*
 * VtxSpritesTabs.h
 *
 *  Created on: Jul 10, 2023
 *      Author: deans
 */

#ifndef WX_VTXSPRITESTABS_H_
#define WX_VTXSPRITESTABS_H_

#include "VtxTabsMgr.h"
#include <Sprites.h>
#include "VtxBitmapPanel.h"

class VtxSpritesTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxSpritesTabs)
protected:
	void AddImageTab(wxWindow *panel);
	void AddDistribTab(wxWindow *panel);
	TNsprite *object() 	{ return ((TNsprite *)object_node->node);}
	Sprite *sprite() 		{ return object()->sprite;}
	void makeFileList(int,wxString);
	void getObjAttributes();
	void setObjAttributes();
	void setImagePanel();
	void setViewPanel();
    void OnDimSelect(wxCommandEvent& event);
    void OnChangedFile(wxCommandEvent& event);
    void OnViewSelect(wxCommandEvent& event);
    void OnChangedLevels(wxCommandEvent& event);
    
    wxString exprString();

	wxChoice *sprites_dim;
	wxString sprites_file;
	wxArrayString files;
	wxChoice *choices;
	wxChoice *select;
	VtxBitmapPanel *image_window;
	wxStaticBoxSizer *image_sizer;
	wxString image_path;
	VtxBitmapPanel *cell_window;
	
	wxChoice *m_orders;
	SliderCtrl *SizeSlider;
	ExprSliderCtrl *DeltaSizeSlider;
	ExprSliderCtrl *LevelDeltaSlider;
	ExprSliderCtrl *DensitySlider;
	ExprSliderCtrl *SlopeBiasSlider;
	ExprSliderCtrl *PhiBiasSlider;
	ExprSliderCtrl *HtBiasSlider;

	bool changed_cell_expr;
	uint image_dim;
	wxString image_name;
public:
	VtxSpritesTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxSpritesTabs(){}
	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);
	int showMenu(bool);
	void updateControls();

	DEFINE_SLIDER_EVENTS(Size)
	DEFINE_SLIDER_EVENTS(DeltaSize)
	DEFINE_SLIDER_EVENTS(LevelDelta)
	DEFINE_SLIDER_EVENTS(Density)
	DEFINE_SLIDER_EVENTS(SlopeBias)
	DEFINE_SLIDER_EVENTS(PhiBias)
	DEFINE_SLIDER_EVENTS(HtBias)
	DECLARE_EVENT_TABLE()
};


#endif /* WX_VTXSPRITESTABS_H_ */
