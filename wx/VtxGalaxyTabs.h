#ifndef GALAXY_TABS_H_
#define GALAXY_TABS_H_


#include "VtxTabsMgr.h"
#include "SceneClass.h"
#include "OrbitalClass.h"
#include "Octree.h"


class VtxGalaxyTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxGalaxyTabs)
protected:
	enum consts{
		NUM_COLORS=8
	};
	void AddPropertiesTab(wxWindow *panel);
	void AddViewTab(wxWindow *panel);
	void AddShapeTab(wxWindow *panel);
	void AddDensityTab(wxWindow *panel);
	void AddStarsTab(wxWindow *panel);

	// properties tab controls

	SliderCtrl *SizeSlider;
	SliderCtrl *CellSizeSlider;
	SliderCtrl *OriginXSlider;
	SliderCtrl *OriginYSlider;
	SliderCtrl *OriginZSlider;
	SliderCtrl *RotationXSlider;
	SliderCtrl *RotationYSlider;
	SliderCtrl *RotationZSlider;

	// shape tab controls

	SliderCtrl *ViewThetaSlider;
	SliderCtrl *ViewPhiSlider;
	SliderCtrl *ViewRadiusSlider;
	SliderCtrl *EyeAngleSlider;
	SliderCtrl *EyeTiltSlider;
	SliderCtrl *EyeSkewSlider;

	wxButton	*m_overhead;
	wxButton	*m_oblique;
	wxButton	*m_core;
	wxButton	*m_rim;
	wxButton	*m_edgeon;

	// shape tab controls

	SliderCtrl *CenterRadiusSlider;
	SliderCtrl *OuterRadiusSlider;
	SliderCtrl *TwistRadiusSlider;
	SliderCtrl *TwistAngleSlider;
	SliderCtrl *ShapeExtSlider;
	SliderCtrl *ShapeIntSlider;
	SliderCtrl *CompressionSlider;
	SliderCtrl *CtrBiasSlider;

	wxButton	*m_spiral;
	wxButton	*m_irregular;
	wxButton	*m_barred;
	wxButton	*m_globular;
	wxButton	*m_elyptical;

	// density tab controls

	SliderCtrl *SaturationSlider;
	SliderCtrl *AmplitudeSlider;
	SliderCtrl *ScatterSlider;
	SliderCtrl *GradientSlider;
	SliderCtrl *CutoffSlider;
	SliderCtrl *DiffusionSlider;
	SliderCtrl *DensitySlider;
	SliderCtrl *VerticalSlider;

	ExprTextCtrl *NoiseExpr;
	wxButton	 *m_default;

	// stars tab controls

	SliderCtrl	*NearSizeSlider;
	SliderCtrl	*NearMaxSlider;
	SliderCtrl	*FarSizeSlider;
	SliderCtrl	*VariabilitySlider;
	SliderCtrl	*NovaSizeSlider;
	SliderCtrl	*NovaDensitySlider;

	ExprTextCtrl *ColorExpr;

	wxButton *colors[NUM_COLORS];

	SliderCtrl	*CMixSlider;
	SliderCtrl	*CBiasSlider;

	void set_view(int t,double x,double y,double z);
	Point viewpoint;
	double size;

	Galaxy *object() 		{ return ((Galaxy*)(object_node->node));}
	void invalidate_object(){
        object()->invalidate();
        TheView->set_changed_detail();
        TheScene->rebuild();
    	update_needed=true;
	}
	void invalidateRender();

public:
	VtxGalaxyTabs(wxWindow* parent,
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

	~VtxGalaxyTabs(){
		delete SizeSlider;
		delete CellSizeSlider;
		delete OriginXSlider;
		delete OriginYSlider;
		delete OriginZSlider;
		delete RotationXSlider;
		delete RotationYSlider;
		delete RotationZSlider;
		delete ViewThetaSlider;
		delete ViewPhiSlider;
		delete ViewRadiusSlider;
		delete EyeAngleSlider;
		delete EyeTiltSlider;
		delete EyeSkewSlider;
		delete CenterRadiusSlider;
		delete OuterRadiusSlider;
		delete TwistRadiusSlider;
		delete TwistAngleSlider;
		delete ShapeExtSlider;
		delete ShapeIntSlider;
		delete CompressionSlider;
		delete CtrBiasSlider;
		delete SaturationSlider;
		delete AmplitudeSlider;
		delete ScatterSlider;
		delete GradientSlider;
		delete CutoffSlider;
		delete DiffusionSlider;
		delete DensitySlider;
		delete VerticalSlider;
		delete NoiseExpr;
		delete NearSizeSlider;
		delete NearMaxSlider;
		delete FarSizeSlider;
		delete VariabilitySlider;
		delete NovaSizeSlider;
		delete NovaDensitySlider;
		delete ColorExpr;
		delete CMixSlider;
		delete CBiasSlider;
		delete object_type;
	}

	int showMenu(bool);
	void updateControls();

	void getObjAttributes();
	void setObjAttributes();

	DEFINE_SLIDER_RENDER_EVENTS(ViewTheta)
	DEFINE_SLIDER_RENDER_EVENTS(ViewPhi)
	DEFINE_SLIDER_RENDER_EVENTS(EyeAngle)
	DEFINE_SLIDER_RENDER_EVENTS(EyeTilt)
	DEFINE_SLIDER_RENDER_EVENTS(EyeSkew)
	DEFINE_SLIDER_EVENTS(ViewRadius)

	DEFINE_SLIDER_EVENTS(Size)
	DEFINE_SLIDER_EVENTS(CellSize)
	DEFINE_SLIDER_EVENTS(RotationX)
	DEFINE_SLIDER_EVENTS(RotationY)
	DEFINE_SLIDER_EVENTS(RotationZ)
	DEFINE_SLIDER_EVENTS(OriginX)
	DEFINE_SLIDER_EVENTS(OriginY)
	DEFINE_SLIDER_EVENTS(OriginZ)

	DEFINE_SLIDER_EVENTS(CenterRadius)
	DEFINE_SLIDER_EVENTS(OuterRadius)
	DEFINE_SLIDER_EVENTS(TwistRadius)
	DEFINE_SLIDER_EVENTS(TwistAngle)
	DEFINE_SLIDER_EVENTS(ShapeExt)
	DEFINE_SLIDER_EVENTS(ShapeInt)
	DEFINE_SLIDER_EVENTS(Compression)
	DEFINE_SLIDER_EVENTS(CtrBias)

	DEFINE_SLIDER_EVENTS(Saturation)
	DEFINE_SLIDER_EVENTS(Amplitude)
	DEFINE_SLIDER_EVENTS(Scatter)
	DEFINE_SLIDER_EVENTS(Gradient)
	DEFINE_SLIDER_EVENTS(Cutoff)
	DEFINE_SLIDER_EVENTS(Diffusion)
	DEFINE_SLIDER_EVENTS(Density)
	DEFINE_SLIDER_EVENTS(Vertical)

	DEFINE_SLIDER_RENDER_EVENTS(NearSize)
	DEFINE_SLIDER_RENDER_EVENTS(NearMax)
	DEFINE_SLIDER_RENDER_EVENTS(FarSize)
	DEFINE_SLIDER_RENDER_EVENTS(Variability)
	DEFINE_SLIDER_RENDER_EVENTS(CMix)
	DEFINE_SLIDER_RENDER_EVENTS(CBias)
	DEFINE_SLIDER_RENDER_EVENTS(NovaSize)
	DEFINE_SLIDER_RENDER_EVENTS(NovaDensity)

	void OnOverheadView(wxCommandEvent& event);
	void OnObliqueView(wxCommandEvent& event);
	void OnCoreView(wxCommandEvent& event);
	void OnRimView(wxCommandEvent& event);
	void OnEdgeView(wxCommandEvent& event);
    void OnViewObj(wxCommandEvent& event);

	void OnSpiral(wxCommandEvent& event);
	void OnIrregular(wxCommandEvent& event);
	void OnBarred(wxCommandEvent& event);
	void OnGlobular(wxCommandEvent& event);
	void OnElyptical(wxCommandEvent& event);

    void OnSetDefault(wxCommandEvent& event);
    void OnColorButton(wxCommandEvent& event);

    void OnChangedDensityExpr(wxCommandEvent& event);
    void OnChangedColorExpr(wxCommandEvent& event);

    void OnUpdateViewObj(wxUpdateUIEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif /*GALAXY_TABS_H_*/
