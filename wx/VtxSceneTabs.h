#ifndef SCENETABS_H_
#define SCENETABS_H_


#include "VtxTabsMgr.h"
#include "SceneClass.h"
#include "RenderOptions.h"
#include "AdaptOptions.h"
#include "Effects.h"

class VtxSceneTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxSceneTabs)
protected:
	void AddAdaptTab(wxWindow *panel);
	void AddDisplayTab(wxWindow *panel);
	void AddFilterTab(wxWindow *panel);
	void AddRenderTab(wxWindow *panel);
	void AddOptionsTab(wxWindow *panel);

	Scene *object() 		{ return ((Scene*)(object_node->node));}

	wxChoice *m_time_scale;
	wxChoice *m_rate_scale;

	SliderCtrl *ColorMipSlider;
	SliderCtrl *BumpMipSlider;
	SliderCtrl *TexMipSlider;
	SliderCtrl *FreqMipSlider;

	SliderCtrl *TimeSlider;
	SliderCtrl *RateSlider;
	SliderCtrl *FOVSlider;
	SliderCtrl *LODSlider;
	SliderCtrl *GeomSlider;

	wxRadioBox *drawmode;
	wxRadioBox *lightmode;
	wxRadioBox *filtermode;
	wxRadioBox *quality;

	wxCheckBox *m_ave_check;
	wxCheckBox *m_aa_check;
	wxCheckBox *m_show_check;
	wxCheckBox *m_hdr;
	wxCheckBox *m_big;
	wxCheckBox *m_aniso;

	wxCheckBox *m_shadows;
	wxCheckBox *m_water;
	wxCheckBox *m_grid;
	wxCheckBox *m_contours;
	wxCheckBox *m_autogrid;

	wxCheckBox *m_occlusion;
	wxCheckBox *m_clip;
	wxCheckBox *m_backface;
	wxCheckBox *m_curvature;

	SliderCtrl *ColorAmpSlider;
	SliderCtrl *NormalAmpSlider;
	SliderCtrl *HDRMinSlider;
	SliderCtrl *HDRMaxSlider;
	SliderCtrl *ShadowBlurSlider;
	SliderCtrl *ShadowResSlider;
	SliderCtrl *ShadowFovSlider;
	SliderCtrl *ShadowDovSlider;

	SliderCtrl *GridSpacingSlider;
	SliderCtrl *ContourSpacingSlider;
	wxColourPickerCtrl *m_contour_color;
	wxColourPickerCtrl *m_phi_color;
	wxColourPickerCtrl *m_theta_color;
	wxColourPickerCtrl *m_text_color;

	wxChoice   *m_tesslevel;


public:
	VtxSceneTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);

	~VtxSceneTabs(){
		delete ColorMipSlider;
		delete BumpMipSlider;
		delete TexMipSlider;
		delete FreqMipSlider;
		delete TimeSlider;
		delete RateSlider;
		delete FOVSlider;
		delete LODSlider;
		delete ColorAmpSlider;
		delete NormalAmpSlider;
		delete HDRMinSlider;
		delete HDRMaxSlider;
		delete ShadowBlurSlider;
		delete ShadowResSlider;
		delete ShadowFovSlider;
		delete ShadowDovSlider;
		delete GridSpacingSlider;
		delete ContourSpacingSlider;

	}
	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);

	int showMenu(bool);

	void updateControls();
	void setTime();
	void getTime();
	void setRate();
	void getRate();
	void OnTesslevel(wxCommandEvent& event);
    void OnDrawMode(wxCommandEvent& event);
    void OnUpdateDrawMode(wxUpdateUIEvent& event);

    void OnLightMode(wxCommandEvent& event);
    void OnUpdateLightMode(wxUpdateUIEvent& event);

    void OnQuality(wxCommandEvent& event);
    void OnUpdateQuality(wxUpdateUIEvent& event);

    void OnEndTimeSlider(wxScrollEvent& event);
    void OnTimeSlider(wxScrollEvent& event);
    void OnTimeText(wxCommandEvent& event);

    void OnEndRateSlider(wxScrollEvent& event){
    	setRate();
     	//OnSliderValue(RateSlider, TheScene->delt);
    }
    void OnRateSlider(wxScrollEvent& event){
    	RateSlider->setValueFromSlider();
    	//setRate();
    	TheScene->set_changed_render();
    }
    void OnRateText(wxCommandEvent& event){
    	setRate();
     	//OnSliderText(RateSlider, TheScene->delt);
    }

    void OnEndFOVSlider(wxScrollEvent& event){
     	OnSliderValue(FOVSlider, TheScene->fov);
     	TheScene->rebuild_all();
    }
    void OnFOVSlider(wxScrollEvent& event){
    	FOVSlider->setValueFromSlider();
    }
    void OnFOVText(wxCommandEvent& event){
     	OnSliderText(FOVSlider, TheScene->fov);
     	TheScene->rebuild_all();
    }

    void OnEndLODSlider(wxScrollEvent& event){
     	OnSliderValue(LODSlider, TheScene->cellsize);
     	TheScene->rebuild_all();
     	changing=false;

    }
    void OnLODSlider(wxScrollEvent& event){
     	changing=true;
    	LODSlider->setValueFromSlider();
    }
    void OnLODText(wxCommandEvent& event){
     	OnSliderText(LODSlider, TheScene->cellsize);
     	TheScene->rebuild_all();
    }
    void OnUpdateLOD(wxUpdateUIEvent& event);

    void OnUpdateTime(wxUpdateUIEvent& event);
    void OnTimeScale(wxCommandEvent& event);
    void OnRateScale(wxCommandEvent& event);

    void OnAnimate(wxCommandEvent& event){
     	if(TheScene->autotm())
     		TheScene->clr_autotm();
     	else
     		TheScene->set_autotm();
     	TheScene->set_changed_render();
    }
    void OnUpdateContourSpacing(wxUpdateUIEvent& event);

    void OnUpdateAnimate(wxUpdateUIEvent& event) {
    	event.Check(TheScene->autotm());
    }
    void OnUpdateAAEnable(wxUpdateUIEvent& event) {
    	event.Check(Render.dealias());
    }
    void OnUpdateAveEnable(wxUpdateUIEvent& event) {
    	event.Check(Render.avenorms());
    }
    void OnUpdateShowMask(wxUpdateUIEvent& event) {
    	event.Check(Raster.filter_show());
    }
    void OnUpdateWater(wxUpdateUIEvent& event) {
    	event.Check(Render.show_water());
    }
    void OnUpdateHDR(wxUpdateUIEvent& event) {
    	event.Check(Raster.hdr());
    }
    void OnUpdateShadows(wxUpdateUIEvent& event) {
    	event.Check(Raster.shadows());
    }
    void OnUpdateAniso(wxUpdateUIEvent& event) {
    	event.Check(Raster.filter_aniso());
    }
    void OnAAEnable(wxCommandEvent& event);
    void OnAveEnable(wxCommandEvent& event);

    void OnBigEnable(wxCommandEvent& event);

    void OnColorAmpSlider(wxScrollEvent& event){
    	OnSliderValue(ColorAmpSlider, Raster.filter_color_ampl);
    }
    void OnColorAmpText(wxCommandEvent& event){
    	OnSliderText(ColorAmpSlider, Raster.filter_color_ampl);
    }
    void OnNormalAmpSlider(wxScrollEvent& event){
    	if(TheScene->motion()) changing=true;
    	OnSliderValue(NormalAmpSlider, Raster.filter_normal_ampl);
    }
    void OnNormalAmpText(wxCommandEvent& event){
    	OnSliderText(NormalAmpSlider, Raster.filter_normal_ampl);
    }
    void OnShowMask(wxCommandEvent& event){
        if(Raster.filter_show())
            Raster.set_filter_show(0);
        else
            Raster.set_filter_show(1);
     	TheScene->set_changed_render();
    }
    void OnShowWater(wxCommandEvent& event){
    	Render.set_water_show(!Render.show_water());
       	TheScene->set_changed_render();
    }

    void OnShowGrid(wxCommandEvent& event){
    	TheScene->enable_grid=event.IsChecked();
       	TheScene->set_changed_render();
    }
    void OnShowContours(wxCommandEvent& event){
     	TheScene->enable_contours=event.IsChecked();
        TheScene->set_changed_render();
     }

    void OnAutogrid(wxCommandEvent& event){
      	 TheScene->set_autogrid(event.IsChecked());
         TheScene->set_changed_render();
      }

    void OnHDR(wxCommandEvent& event){
        Raster.set_hdr(event.IsChecked());
     	TheScene->set_changed_render();
    }
    void OnShadows(wxCommandEvent& event){
        Raster.set_shadows(event.IsChecked());
     	TheScene->set_changed_render();
    }
    void OnWater(wxCommandEvent& event){
        Render.set_water_show(event.IsChecked());
      	TheScene->set_changed_render();
    }
    void OnAniso(wxCommandEvent& event){
        Raster.set_filter_aniso(event.IsChecked());
        if(event.IsChecked())
        	cout << "Anisotopic filtering enabled"<<endl;
        else
        	cout << "Anisotopic filtering disabled"<<endl;
    	Render.invalidate_textures();
        TheScene->set_changed_detail();
    }
    void OnOcclusion(wxCommandEvent& event){
        Adapt.set_overlap_test(event.IsChecked());
        TheScene->rebuild_all();
    }
    void OnUpdateOcclusion(wxUpdateUIEvent& event) {
    	event.Check(Adapt.overlap_test());
    }

    void OnBackfacing(wxCommandEvent& event){
        Adapt.set_back_test(event.IsChecked());
        TheScene->rebuild_all();
    }
    void OnCurvature(wxCommandEvent& event){
        Adapt.set_curve_test(event.IsChecked());
        TheScene->rebuild_all();
    }
    void OnClipped(wxCommandEvent& event){
        Adapt.set_clip_test(event.IsChecked());
        TheScene->rebuild_all();
    }

    void OnPhiColor(wxColourPickerEvent& WXUNUSED(event)){
    	wxColor col=m_phi_color->GetColour();
    	TheScene->phi_color.set(col.Red(),col.Green(),col.Blue());
    	TheScene->set_changed_render();
    }
    void OnThetaColor(wxColourPickerEvent& WXUNUSED(event)){
    	wxColor col=m_theta_color->GetColour();
    	TheScene->theta_color.set(col.Red(),col.Green(),col.Blue());
    	TheScene->set_changed_render();
    }
    void OnTextColor(wxColourPickerEvent& WXUNUSED(event)){
     	wxColor col=m_text_color->GetColour();
     	Color c;
     	c.set(col.Red(),col.Green(),col.Blue());
     	TheScene->text_color=c;
     	TheScene->syscolor[INFO_COLOR]=c;
     	TheScene->set_changed_render();
     }
   void OnContourColor(wxColourPickerEvent& WXUNUSED(event)){
     	wxColor col=m_contour_color->GetColour();
     	TheScene->contour_color.set(col.Red(),col.Green(),col.Blue());
     	TheScene->set_changed_render();
     }
    void OnAutoGrid(wxCommandEvent& event){
    	TheScene->set_autogrid(event.IsChecked());
    	TheScene->set_changed_render();
    }
    void setObjAttributes();

    DEFINE_SLIDER_VAR_EVENTS(ShadowRes,Raster.shadow_vsteps)
    DEFINE_SLIDER_VAR_EVENTS(ShadowBlur,Raster.shadow_blur)
    DEFINE_SLIDER_VAR_EVENTS(HDRMax,Raster.hdr_max_base)
    DEFINE_SLIDER_VAR_EVENTS(HDRMin,Raster.hdr_min_base)
    DEFINE_SLIDER_VAR_EVENTS(ShadowFov,Raster.shadow_fov)
    DEFINE_SLIDER_VAR_EVENTS(ShadowDov,Raster.shadow_dov)

    DEFINE_SLIDER_VAR_EVENTS(GridSpacing,TheScene->grid_spacing)
    DEFINE_SLIDER_VAR_EVENTS(ContourSpacing,TheScene->contour_spacing)

	DEFINE_SLIDER_EVENTS(ColorMip)
	DEFINE_SLIDER_EVENTS(TexMip)
	DEFINE_SLIDER_EVENTS(BumpMip)
	DEFINE_SLIDER_EVENTS(FreqMip)

	DECLARE_EVENT_TABLE()
};


#endif /*SCENETABS_H_*/
