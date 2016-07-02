#ifndef SNOWTABS_H_
#define SNOWTABS_H_

#include "VtxTabsMgr.h"
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <TerrainClass.h>
#include <defs.h>


class VtxSnowTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxSnowTabs)

protected:
	TNsnow *object() 	{ return (TNsnow *)object_node->node;}
	void AddControlsTab(wxWindow *panel);
	void getObjAttributes();
	void setObjAttributes();

	SliderCtrl *ThreshSlider;
	SliderCtrl *HeightSlider;
	SliderCtrl *SlopeSlider;
	SliderCtrl *LatSlider;
	SliderCtrl *BumpSlider;
	SliderCtrl *DepthSlider;

public:
	VtxSnowTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxSnowTabs(){
		delete ThreshSlider;
		delete HeightSlider;
		delete SlopeSlider;
		delete LatSlider;
		delete BumpSlider;
		delete DepthSlider;
	}
	bool Create(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	void updateControls();
	int showMenu(bool);

	DEFINE_SLIDER_RENDER_EVENTS(Height)
	DEFINE_SLIDER_ADAPT_EVENTS(Thresh)
	DEFINE_SLIDER_ADAPT_EVENTS(Lat)
	DEFINE_SLIDER_RENDER_EVENTS(Depth)
	DEFINE_SLIDER_RENDER_EVENTS(Bump)
	DEFINE_SLIDER_ADAPT_EVENTS(Slope)

	DECLARE_EVENT_TABLE()
};
#endif /*SNOWTABS_H_*/
