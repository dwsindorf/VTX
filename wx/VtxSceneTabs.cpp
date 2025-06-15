
#include "VtxSceneTabs.h"
#include "MapClass.h"
#include <GLSLMgr.h>

//########################### VtxSceneTabs Class ########################

#define LINE_HEIGHT 30
#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define SLIDER1 100

// define all resource ids here
enum {
	OBJ_DELETE,
    ID_DRAWTYPE,
    ID_RENDER_QUALITY,
    ID_GENERATE_QUALITY,
    ID_LMODE,
	ID_MERGE_MODE,
    ID_ANIMATE,
	ID_TIME_FORWARD,
    ID_TIME_SLDR,
    ID_TIME_TEXT,
    ID_RATE_SLDR,
    ID_RATE_TEXT,
    ID_FOV_SLDR,
    ID_FOV_TEXT,
    ID_LOD_SLDR,
    ID_LOD_TEXT,
    ID_TESSLEVEL,
    ID_FILTER_MODE,
    ID_AVE_ENABLE,
    ID_AA_ENABLE,
    ID_COLOR_SLDR,
    ID_COLOR_TEXT,
    ID_SHOW_MASK,
    ID_HDR,
	ID_BIG_KERNEL,
    ID_ANISO,
    ID_NORMAL_SLDR,
    ID_NORMAL_TEXT,
    ID_HDRMIN_SLDR,
    ID_HDRMIN_TEXT,
    ID_HDRMAX_SLDR,
    ID_HDRMAX_TEXT,
    ID_TIME_SCALE,
    ID_RATE_SCALE,
    ID_TEX_MIP_SLDR,
    ID_TEX_MIP_TEXT,
    ID_COLOR_MIP_SLDR,
    ID_COLOR_MIP_TEXT,
    ID_BUMP_MIP_SLDR,
    ID_BUMP_MIP_TEXT,
    ID_FREQ_MIP_SLDR,
    ID_FREQ_MIP_TEXT,

    ID_SHADOWS,
    ID_SHADOW_BLUR_SLDR,
    ID_SHADOW_BLUR_TEXT,
    ID_SHADOW_RES_SLDR,
    ID_SHADOW_RES_TEXT,
    ID_SHADOW_FOV_SLDR,
    ID_SHADOW_FOV_TEXT,
    ID_SHADOW_DOV_SLDR,
    ID_SHADOW_DOV_TEXT,
    ID_SHOW_WATER,
    ID_SHOW_GRID,
    ID_GRID_SPACING_SLDR,
    ID_GRID_SPACING_TEXT,
	
	ID_KEEP_TMPS,
	ID_USE_TMPS,
	ID_SET_LAYERS,

    ID_PHI_COLOR,
    ID_THETA_COLOR,
    ID_SHOW_CONTOURS,
    ID_CONTOUR_SPACING_SLDR,
    ID_CONTOUR_SPACING_TEXT,

    ID_CONTOUR_COLOR,
    ID_AUTOGRID,
    ID_TEXT_COLOR,
	ID_TEMPMODE,

    ID_ADAPT_OCCLUSION,
    ID_ADAPT_CURVATURE,
    ID_ADAPT_BACKFACING,
    ID_ADAPT_CLIPPED,

};

IMPLEMENT_CLASS(VtxSceneTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxSceneTabs, wxNotebook)

EVT_CHECKBOX(ID_ANIMATE,VtxSceneTabs::OnAnimate)
EVT_UPDATE_UI(ID_ANIMATE, VtxSceneTabs::OnUpdateAnimate)
EVT_CHECKBOX(ID_TIME_FORWARD,VtxSceneTabs::OnForwardTime)
EVT_UPDATE_UI(ID_TIME_FORWARD, VtxSceneTabs::OnUpdateTimeDirection)


EVT_RADIOBOX(ID_DRAWTYPE, VtxSceneTabs::OnDrawMode)
EVT_UPDATE_UI(ID_DRAWTYPE, VtxSceneTabs::OnUpdateDrawMode)

EVT_UPDATE_UI(ID_RENDER_QUALITY, VtxSceneTabs::OnUpdateRenderQuality)
EVT_CHOICE(ID_RENDER_QUALITY,VtxSceneTabs::OnRenderQualitySelect)

EVT_UPDATE_UI(ID_GENERATE_QUALITY, VtxSceneTabs::OnUpdateGenerateQuality)
EVT_CHOICE(ID_GENERATE_QUALITY,VtxSceneTabs::OnGenerateQualitySelect)

EVT_UPDATE_UI(ID_SET_LAYERS, VtxSceneTabs::OnUpdateSetLayers)
EVT_CHOICE(ID_SET_LAYERS,VtxSceneTabs::OnSetLayersSelect)

EVT_RADIOBOX(ID_TEMPMODE, VtxSceneTabs::OnTempMode)

EVT_RADIOBOX(ID_LMODE, VtxSceneTabs::OnLightMode)
EVT_UPDATE_UI(ID_LMODE, VtxSceneTabs::OnUpdateLightMode)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_TIME_SLDR,VtxSceneTabs::OnEndTimeSlider)
EVT_COMMAND_SCROLL(ID_TIME_SLDR,VtxSceneTabs::OnTimeSlider)
EVT_TEXT_ENTER(ID_TIME_TEXT,VtxSceneTabs::OnTimeText)
//EVT_UPDATE_UI(ID_TIME_SLDR, VtxSceneTabs::OnUpdateTime)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_RATE_SLDR,VtxSceneTabs::OnEndRateSlider)
EVT_COMMAND_SCROLL(ID_RATE_SLDR,VtxSceneTabs::OnRateSlider)
EVT_TEXT_ENTER(ID_RATE_TEXT,VtxSceneTabs::OnRateText)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_FOV_SLDR,VtxSceneTabs::OnEndFOVSlider)
EVT_COMMAND_SCROLL(ID_FOV_SLDR,VtxSceneTabs::OnFOVSlider)
EVT_TEXT_ENTER(ID_FOV_TEXT,VtxSceneTabs::OnFOVText)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_LOD_SLDR,VtxSceneTabs::OnEndLODSlider)
EVT_COMMAND_SCROLL(ID_LOD_SLDR,VtxSceneTabs::OnLODSlider)
EVT_TEXT_ENTER(ID_LOD_TEXT,VtxSceneTabs::OnLODText)
EVT_UPDATE_UI(ID_LOD_SLDR, VtxSceneTabs::OnUpdateLOD)

EVT_CHECKBOX(ID_AA_ENABLE,VtxSceneTabs::OnAAEnable)
EVT_UPDATE_UI(ID_AA_ENABLE, VtxSceneTabs::OnUpdateAAEnable)

EVT_CHECKBOX(ID_AVE_ENABLE,VtxSceneTabs::OnAveEnable)
EVT_UPDATE_UI(ID_AVE_ENABLE, VtxSceneTabs::OnUpdateAveEnable)

EVT_CHECKBOX(ID_BIG_KERNEL,VtxSceneTabs::OnBigEnable)

EVT_CHECKBOX(ID_WATER,VtxSceneTabs::OnWater)
EVT_UPDATE_UI(ID_WATER, VtxSceneTabs::OnUpdateWater)

EVT_CHECKBOX(ID_ANISO,VtxSceneTabs::OnAniso)
EVT_UPDATE_UI(ID_ANISO, VtxSceneTabs::OnUpdateAniso)

EVT_CHECKBOX(ID_SHOW_MASK,VtxSceneTabs::OnShowMask)
EVT_UPDATE_UI(ID_SHOW_MASK, VtxSceneTabs::OnUpdateShowMask)

EVT_CHECKBOX(ID_SHOW_WATER,VtxSceneTabs::OnShowWater)

EVT_CHECKBOX(ID_SHOW_GRID,VtxSceneTabs::OnShowGrid)
EVT_CHECKBOX(ID_SHOW_CONTOURS,VtxSceneTabs::OnShowContours)

EVT_CHECKBOX(ID_HDR,VtxSceneTabs::OnHDR)
EVT_UPDATE_UI(ID_HDR, VtxSceneTabs::OnUpdateHDR)

EVT_CHECKBOX(ID_SHADOWS,VtxSceneTabs::OnShadows)
EVT_UPDATE_UI(ID_SHADOWS, VtxSceneTabs::OnUpdateShadows)

EVT_CHECKBOX(ID_ADAPT_OCCLUSION,VtxSceneTabs::OnOcclusion)
EVT_UPDATE_UI(ID_ADAPT_OCCLUSION, VtxSceneTabs::OnUpdateOcclusion)
EVT_CHECKBOX(ID_ADAPT_CURVATURE,VtxSceneTabs::OnCurvature)
EVT_CHECKBOX(ID_ADAPT_BACKFACING,VtxSceneTabs::OnBackfacing)
EVT_CHECKBOX(ID_ADAPT_CLIPPED,VtxSceneTabs::OnClipped)

EVT_COMMAND_SCROLL(ID_COLOR_SLDR,VtxSceneTabs::OnColorAmpSlider)
EVT_TEXT_ENTER(ID_COLOR_TEXT,VtxSceneTabs::OnColorAmpText)

EVT_COMMAND_SCROLL(ID_NORMAL_SLDR,VtxSceneTabs::OnNormalAmpSlider)
EVT_TEXT_ENTER(ID_NORMAL_TEXT,VtxSceneTabs::OnNormalAmpText)

SET_SLIDER_EVENTS(HDRMIN,VtxSceneTabs,HDRMin)
SET_SLIDER_EVENTS(HDRMAX,VtxSceneTabs,HDRMax)
SET_SLIDER_EVENTS(SHADOW_BLUR,VtxSceneTabs,ShadowBlur)
SET_SLIDER_EVENTS(SHADOW_RES,VtxSceneTabs,ShadowRes)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxSceneTabs::OnAddItem)
EVT_MENU(OBJ_DELETE,VtxSceneTabs::OnDelete)

EVT_CHOICE(ID_TIME_SCALE, VtxSceneTabs::OnTimeScale)
EVT_CHOICE(ID_RATE_SCALE, VtxSceneTabs::OnRateScale)
EVT_CHOICE(ID_TESSLEVEL, VtxSceneTabs::OnTesslevel)

SET_SLIDER_EVENTS(TEX_MIP,VtxSceneTabs,TexMip)
SET_SLIDER_EVENTS(COLOR_MIP,VtxSceneTabs,ColorMip)
SET_SLIDER_EVENTS(BUMP_MIP,VtxSceneTabs,BumpMip)
SET_SLIDER_EVENTS(FREQ_MIP,VtxSceneTabs,FreqMip)
SET_SLIDER_EVENTS(SHADOW_FOV,VtxSceneTabs,ShadowFov)
SET_SLIDER_EVENTS(SHADOW_DOV,VtxSceneTabs,ShadowDov)

SET_SLIDER_EVENTS(GRID_SPACING,VtxSceneTabs,GridSpacing)
SET_SLIDER_EVENTS(CONTOUR_SPACING,VtxSceneTabs,ContourSpacing)

EVT_COLOURPICKER_CHANGED(ID_PHI_COLOR,VtxSceneTabs::OnPhiColor)
EVT_COLOURPICKER_CHANGED(ID_THETA_COLOR,VtxSceneTabs::OnThetaColor)
EVT_COLOURPICKER_CHANGED(ID_CONTOUR_COLOR,VtxSceneTabs::OnContourColor)
EVT_COLOURPICKER_CHANGED(ID_TEXT_COLOR,VtxSceneTabs::OnTextColor)

EVT_CHECKBOX(ID_AUTOGRID,VtxSceneTabs::OnAutogrid)
EVT_UPDATE_UI(ID_CONTOUR_SPACING_SLDR, VtxSceneTabs::OnUpdateContourSpacing)
EVT_UPDATE_UI(ID_GRID_SPACING_SLDR, VtxSceneTabs::OnUpdateContourSpacing)

EVT_CHOICE(ID_KEEP_TMPS,VtxSceneTabs::OnKeepTmps)
EVT_CHOICE(ID_USE_TMPS,VtxSceneTabs::OnUseTmps)

EVT_RADIOBOX(ID_MERGE_MODE, VtxSceneTabs::OnMergeMode)
EVT_UPDATE_UI(ID_MERGE_MODE, VtxSceneTabs::OnUpdateMergeMode)


END_EVENT_TABLE()

VtxSceneTabs::VtxSceneTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxSceneTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
	changing=true;
    AddDisplayTab(page);
    AddPage(page,wxT("Display"),true);

    page=new wxPanel(this,wxID_ANY);
    AddAdaptTab(page);
    AddPage(page,wxT("Adapt"),false);

    page=new wxPanel(this,wxID_ANY);
    AddRenderTab(page);
    AddPage(page,wxT("Render"),false);

    page=new wxPanel(this,wxID_ANY);
    AddFilterTab(page);
    AddPage(page,wxT("Filter"),false);

    page=new wxPanel(this,wxID_ANY);
    AddOptionsTab(page);
    AddPage(page,wxT("Options"),false);

    changing=false;
    return true;
}
void VtxSceneTabs::AddFilterTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

    wxStaticBoxSizer* check_options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Filter Options"));

	m_ave_check=new wxCheckBox(panel, ID_AVE_ENABLE, "AveNorms");
	m_ave_check->SetToolTip("Average normals at each vertex");
	check_options->Add(m_ave_check,0,wxALIGN_LEFT|wxALL,1);

	m_aniso=new wxCheckBox(panel, ID_ANISO, "Aniso");
	m_aniso->SetToolTip("Anisotropic texture filtering");
	check_options->Add(m_aniso,0,wxALIGN_LEFT|wxALL,1);

	m_aa_check=new wxCheckBox(panel, ID_AA_ENABLE, "Edges");
	m_aa_check->SetToolTip("Apply image filter");
	check_options->Add(m_aa_check,0,wxALIGN_LEFT|wxALL,1);

	m_big=new wxCheckBox(panel, ID_BIG_KERNEL, "Big Kernel");
	m_big->SetToolTip("use large filter kernel");
	check_options->Add(m_big,0,wxALIGN_LEFT|wxALL,1);

	m_show_check=new wxCheckBox(panel, ID_SHOW_MASK, "Show Mask");
	m_show_check->SetToolTip("Show image filter");
	check_options->Add(m_show_check,0,wxALIGN_LEFT|wxALL,1);

    boxSizer->Add(check_options, 0, wxALIGN_LEFT|wxALL,0);
    check_options->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

    wxStaticBoxSizer* ampl_controls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Edges"));

	hline = new wxBoxSizer(wxHORIZONTAL);

	ColorAmpSlider=new SliderCtrl(panel,ID_COLOR_SLDR,"Color",LABEL2,VALUE2,SLIDER2);
	ColorAmpSlider->setRange(0.0,1.0);
	ColorAmpSlider->slider->SetToolTip("Color change contribution to filter");
	hline->Add(ColorAmpSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    NormalAmpSlider=new SliderCtrl(panel,ID_NORMAL_SLDR,"Normal",LABEL2,VALUE2,SLIDER2);
    NormalAmpSlider->setRange(0.0,1.0);
    NormalAmpSlider->slider->SetToolTip("Normal change contribution to filter");
    hline->Add(NormalAmpSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

    ampl_controls->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
    boxSizer->Add(ampl_controls, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* mip_bias = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Textures"));

	hline = new wxBoxSizer(wxHORIZONTAL);

	TexMipSlider=new SliderCtrl(panel,ID_TEX_MIP_SLDR,"MIP",LABEL2,VALUE2,SLIDER2);
	TexMipSlider->setRange(-1,1.0);
	TexMipSlider->setValue(0.0);
	hline->Add(TexMipSlider->getSizer(), 0, wxALIGN_LEFT | wxALL, 0);

	FreqMipSlider=new SliderCtrl(panel,ID_FREQ_MIP_SLDR,"Freq",LABEL2,VALUE2,SLIDER2);
	FreqMipSlider->setRange(-1,1.0);
	FreqMipSlider->setValue(0.0);
	hline->Add(FreqMipSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	//hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	mip_bias->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	ColorMipSlider=new SliderCtrl(panel,ID_COLOR_MIP_SLDR,"Color",LABEL2,VALUE2,SLIDER2);
	ColorMipSlider->setRange(-1,1.0);
	ColorMipSlider->setValue(0.0);

	hline->Add(ColorMipSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	BumpMipSlider=new SliderCtrl(panel,ID_BUMP_MIP_SLDR,"Bump",LABEL2,VALUE2,SLIDER2);
	//BumpMipSlider->setRange(0,5e-1,0.0,1.0);
	BumpMipSlider->setRange(-1.0,1.0);
	BumpMipSlider->setValue(0.0);
	hline->Add(BumpMipSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	//hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	mip_bias->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(mip_bias, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxSceneTabs::AddRenderTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBoxSizer* check_options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Render Options"));
	m_hdr=new wxCheckBox(panel, ID_HDR, "HDR");
	m_hdr->SetToolTip("Enable High dynamic range");
	check_options->Add(m_hdr,0,wxALIGN_LEFT|wxALL,1);

	m_shadows=new wxCheckBox(panel, ID_SHADOWS, "Shadows");
	m_shadows->SetToolTip("Enable shadows");
	check_options->Add(m_shadows,0,wxALIGN_LEFT|wxALL,1);

	m_water=new wxCheckBox(panel, ID_SHOW_WATER, "Water");
	m_water->SetToolTip("Enable water");
	check_options->Add(m_water,0,wxALIGN_LEFT|wxALL,1);

    boxSizer->Add(check_options, 0, wxALIGN_LEFT|wxALL,0);
    check_options->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	wxStaticBoxSizer* hdr_controls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("High Dynamic Range"));

	HDRMinSlider=new SliderCtrl(panel,ID_HDRMIN_SLDR,"Min",LABEL2,VALUE2,SLIDER2);
	HDRMinSlider->setRange(0.0,30.0);
	HDRMinSlider->slider->SetToolTip("Increase range in dark areas");

	hdr_controls->Add(HDRMinSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	HDRMaxSlider=new SliderCtrl(panel,ID_HDRMAX_SLDR,"Max",LABEL2,VALUE2,SLIDER2);
	HDRMaxSlider->setRange(0.0,10.0);
	HDRMaxSlider->slider->SetToolTip("Decreases range in light areas");

	hdr_controls->Add(HDRMaxSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(hdr_controls, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* shadow_controls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Shadows"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	ShadowBlurSlider=new SliderCtrl(panel,ID_SHADOW_BLUR_SLDR,"Blur",LABEL2,VALUE2,SLIDER2);
	ShadowBlurSlider->setRange(0.1,2.0);
	ShadowBlurSlider->slider->SetToolTip("Soften Shadow Edges");

	hline->Add(ShadowBlurSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	ShadowResSlider=new SliderCtrl(panel,ID_SHADOW_RES_SLDR,"Views",LABEL2,VALUE2,SLIDER2);
	ShadowResSlider->setRange(1,10);
	ShadowResSlider->slider->SetToolTip("Number of Light source views");
	ShadowResSlider->format=wxString("%1.0f");

	hline->Add(ShadowResSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	shadow_controls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	ShadowFovSlider=new SliderCtrl(panel,ID_SHADOW_FOV_SLDR,"FOV",LABEL2,VALUE2,SLIDER2);
	ShadowFovSlider->setRange(0.5,1.5);
	ShadowFovSlider->slider->SetToolTip("Light source field of view");

	hline->Add(ShadowFovSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	ShadowDovSlider=new SliderCtrl(panel,ID_SHADOW_DOV_SLDR,"DOV",LABEL2,VALUE2,SLIDER2);
	ShadowDovSlider->setRange(0.1,1.5);
	ShadowDovSlider->slider->SetToolTip("Light source depth of view");

	hline->Add(ShadowDovSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	shadow_controls->Add(hline,0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(shadow_controls, 0, wxALIGN_LEFT|wxALL,0);

}

void VtxSceneTabs::AddOptionsTab(wxWindow *panel){
  // A top-level sizer

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);

	// A second box sizer to give more space around the controls

	wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	
	wxStaticBoxSizer* check_options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Grid Options"));

	m_grid=new wxCheckBox(panel, ID_SHOW_GRID, "Map");
	m_grid->SetToolTip("Draw Long/Lat lines");
	check_options->Add(m_grid,0,wxALIGN_LEFT|wxALL,1);

	m_contours=new wxCheckBox(panel, ID_SHOW_CONTOURS, "Contours");
	m_contours->SetToolTip("Draw Contour lines");
	check_options->Add(m_contours,0,wxALIGN_LEFT|wxALL,1);

	m_autogrid=new wxCheckBox(panel, ID_AUTOGRID, "Auto");
	m_autogrid->SetToolTip("Autoset intervals");
	check_options->Add(m_autogrid,0,wxALIGN_LEFT|wxALL,1);
	
	hline->Add(check_options, 0, wxALIGN_LEFT|wxALL,0);
	
//	wxStaticBoxSizer* pref_options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT(""));


	wxString tmodes[]={"K","C","F"};
	tempmode=new wxRadioBox(panel,ID_TEMPMODE,wxT(""),wxPoint(-1,-1),wxSize(-1,40),3,tmodes,3,wxRA_SPECIFY_COLS);

	hline->Add(tempmode,0,wxALIGN_LEFT|wxALL,1);

	boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	boxSizer->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	wxStaticBoxSizer* grid_controls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Lines"));

	hline = new wxBoxSizer(wxHORIZONTAL);

	GridSpacingSlider=new SliderCtrl(panel,ID_GRID_SPACING_SLDR,"Grid",LABEL2,VALUE2,SLIDER2);
	GridSpacingSlider->setRange(0.1,10);
	GridSpacingSlider->slider->SetToolTip("Longitude-latitude grid spacing");

	hline->Add(GridSpacingSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	grid_controls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	hline->Add(new wxStaticText(panel, -1, "Miles", wxDefaultPosition, wxSize(40,-1)),5,wxALIGN_LEFT|wxTOP,5);

	wxColor col("RGB(255,255,255)");

	m_phi_color=new wxColourPickerCtrl(panel,ID_PHI_COLOR,col,wxDefaultPosition, wxSize(50,30));
	hline->Add(m_phi_color,0,wxALIGN_LEFT|wxALL,0);

	hline->Add(new wxStaticText(panel, -1, "Lat", wxDefaultPosition, wxSize(40,-1)),0,wxALIGN_LEFT|wxTOP|wxLEFT,5);

	m_theta_color=new wxColourPickerCtrl(panel,ID_THETA_COLOR,col,wxDefaultPosition, wxSize(50,30));
	hline->Add(m_theta_color,0,wxALIGN_LEFT|wxALL,0);

	hline->Add(new wxStaticText(panel, -1, "Long", wxDefaultPosition, wxSize(40,-1)),5,wxALIGN_LEFT|wxALL,5);

	hline = new wxBoxSizer(wxHORIZONTAL);

	ContourSpacingSlider=new SliderCtrl(panel,ID_CONTOUR_SPACING_SLDR,"Contours",LABEL2,VALUE2,SLIDER2);
	ContourSpacingSlider->setRange(100,20000);
	ContourSpacingSlider->slider->SetToolTip("Contour interval");

	hline->Add(ContourSpacingSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->Add(new wxStaticText(panel, -1, "Feet", wxDefaultPosition, wxSize(40,-1)),0,wxALIGN_LEFT|wxTOP|wxLEFT,5);

	m_contour_color=new wxColourPickerCtrl(panel,ID_CONTOUR_COLOR,col,wxDefaultPosition, wxSize(50,30));
	hline->Add(m_contour_color,0,wxALIGN_LEFT|wxALL,0);

	hline->Add(new wxStaticText(panel, -1, "Text", wxDefaultPosition, wxSize(40,-1)),0,wxALIGN_LEFT|wxTOP|wxLEFT,5);
	m_text_color=new wxColourPickerCtrl(panel,ID_TEXT_COLOR,col,wxDefaultPosition, wxSize(50,30));
	hline->Add(m_text_color,0,wxALIGN_LEFT|wxALL,0);

	grid_controls->Add(hline,0,wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(grid_controls, 0, wxALIGN_LEFT|wxALL,0);
}
void VtxSceneTabs::AddAdaptTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxBoxSizer* detail_ctrls = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Vertex Generation"));
	
	detail_ctrls->AddSpacer(5);

	detail_ctrls->Add(new wxStaticText(panel,-1,"Detail",wxDefaultPosition,wxSize(50,-1)), 0, wxALIGN_LEFT|wxTOP, 4);

    wxString qmodes[]={"Draft","Normal","High","Photo"};
    render_quality=new wxChoice(panel,ID_RENDER_QUALITY,wxDefaultPosition,wxSize(80,-1),4,qmodes);
    render_quality->SetSelection(1);
    detail_ctrls->Add(render_quality, 0, wxALIGN_LEFT|wxALL,0);

	LODSlider=new SliderCtrl(panel,ID_LOD_SLDR,"CellSize",50, VALUE,60);
	LODSlider->setRange(1,10);

	detail_ctrls->Add(LODSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	//LODSlider->setRange(20,75);

	wxStaticText *lbl=new wxStaticText(panel,-1,"Geometry Level",wxDefaultPosition,wxSize(-1,-1));
	detail_ctrls->Add(lbl, 5, wxALIGN_LEFT|wxTop|wxLEFT,0);

	wxString offsets[]={"1","2","3","4","5","6"};

	m_tesslevel=new wxChoice(panel, ID_TESSLEVEL, wxDefaultPosition,wxSize(-1,-1),Map::maxtesslevel, offsets);
	m_tesslevel->SetSelection(0);

	detail_ctrls->Add(m_tesslevel,0,wxALIGN_LEFT|wxALL,0);

	detail_ctrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(detail_ctrls, 0, wxALIGN_LEFT|wxALL,0);

	wxStaticBoxSizer* quality_options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Procedural Generation"));
    quality_options->AddSpacer(5);

    wxString gmodes[]={"Low","Medium","High","Max"};
	quality_options->Add(new wxStaticText(panel,-1,"Detail",wxDefaultPosition,wxSize(40,-1)), 0,wxALIGN_LEFT|wxTOP, 4);

    generate_quality=new wxChoice(panel,ID_GENERATE_QUALITY,wxDefaultPosition,wxSize(70,-1),4,gmodes);
    generate_quality->SetSelection(1);
    quality_options->Add(generate_quality, 0, wxALIGN_LEFT|wxALL,0);
    quality_options->AddSpacer(5);
    
    wxString lmodes[]={"Auto","1","2","3","4"};
	quality_options->Add(new wxStaticText(panel,-1,"Layers",wxDefaultPosition,wxSize(50,-1)), 0,wxALIGN_LEFT|wxTOP, 4);
    set_layers=new wxChoice(panel,ID_SET_LAYERS,wxDefaultPosition,wxSize(60,-1),5,lmodes);
    set_layers->SetSelection(0);
    quality_options->Add(set_layers, 0, wxALIGN_LEFT|wxALL,0);
    quality_options->AddSpacer(5);

	wxString tmps[]={"All","2D","None"};
    quality_options->Add(new wxStaticText(panel, -1, "Reuse", wxDefaultPosition, wxSize(40,-1)),0,wxALIGN_LEFT|wxTOP,4);
	use_tmps=new wxChoice(panel, ID_USE_TMPS, wxDefaultPosition,wxSize(60,-1),3, tmps);
	use_tmps->SetSelection(1);
	use_tmps->SetToolTip("ReUse auto-generated Files");
	quality_options->Add(use_tmps,0,wxALIGN_LEFT||wxTOP,4);
    quality_options->AddSpacer(5);

    quality_options->Add(new wxStaticText(panel, -1, "Keep", wxDefaultPosition, wxSize(40,-1)),0,wxALIGN_LEFT|wxTOP,4);

	keep_tmps=new wxChoice(panel, ID_KEEP_TMPS, wxDefaultPosition,wxSize(60,-1),3, tmps);
	keep_tmps->SetSelection(1);
	keep_tmps->SetToolTip("Keep auto-generated Files");
	quality_options->Add(keep_tmps,0,wxALIGN_LEFT||wxTOP,4);

    quality_options->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	boxSizer->Add(quality_options, 0, wxALIGN_LEFT|wxALL,0);
	
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

    wxStaticBoxSizer* check_options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Adapt Tests"));
    m_occlusion=new wxCheckBox(panel, ID_ADAPT_OCCLUSION, "Occlusion");
    m_occlusion->SetToolTip("Decrease resolution in occluded areas");
	check_options->Add(m_occlusion,0,wxALIGN_LEFT|wxALL,1);

	m_clip=new wxCheckBox(panel, ID_ADAPT_CLIPPED, "Clipping");
	m_clip->SetToolTip("Decrease resolution in off-screen areas");
	check_options->Add(m_clip,0,wxALIGN_LEFT|wxALL,1);

	m_backface=new wxCheckBox(panel, ID_ADAPT_BACKFACING, "Backfacing");
	m_backface->SetToolTip("Decrease resolution in backfacing sides");
	check_options->Add(m_backface,0,wxALIGN_LEFT|wxALL,1);

	m_curvature=new wxCheckBox(panel, ID_ADAPT_CURVATURE, "Curvature");
	m_curvature->SetToolTip("Decrease resolution in smooth areas");
	check_options->Add(m_curvature,0,wxALIGN_LEFT|wxALL,1);
	hline->Add(check_options,0,wxALIGN_LEFT|wxALL,0);
	
    wxString mmodes[]={"Overlap","Merge"};
    mergemode=new wxRadioBox(panel,ID_MERGE_MODE,wxT(""),wxPoint(-1,-1),wxSize(200,42),2,
    		mmodes,2,wxRA_SPECIFY_COLS);

    //mergemode->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

    //merge_cntrls->Add(mergemode, 0, wxALIGN_LEFT|wxALL,0);

    hline->Add(mergemode, 0, wxALIGN_LEFT|wxALL, 0);
	
	hline->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

    boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
}

void VtxSceneTabs::AddDisplayTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxBoxSizer* time_ctrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Animation"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	TimeSlider=new SliderCtrl(panel,ID_TIME_SLDR,"Time",LABEL, VALUE,200);
	TimeSlider->setRange(1,10);

	wxString exps[]={"x1","x10","x100","x1000","x10^4","x10^5","x10^6","x10^7"};
	m_time_scale=new wxChoice(panel, ID_TIME_SCALE, wxDefaultPosition,wxSize(80,-1),8, exps);

	m_time_scale->SetSelection(4);

	hline->Add(TimeSlider->getSizer());
	hline->Add(m_time_scale);


	time_ctrls->Add(hline);

	hline = new wxBoxSizer(wxHORIZONTAL);

	m_rate_scale=new wxChoice(panel, ID_RATE_SCALE, wxDefaultPosition,wxSize(80,-1),8, exps);

	m_rate_scale->SetSelection(4);
	RateSlider=new SliderCtrl(panel,ID_RATE_SLDR,"Rate",LABEL, VALUE,200);
	RateSlider->setRange(1,10);

	hline->Add(RateSlider->getSizer());
	hline->Add(m_rate_scale);
	
	m_forward_time=new wxCheckBox(panel, ID_TIME_FORWARD, "+");
	hline->Add(m_forward_time,0, wxALIGN_LEFT|wxALL,5);

	time_ctrls->Add(hline);
	time_ctrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));
	boxSizer->Add(time_ctrls, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* view_ctrls = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("View"));

	FOVSlider=new SliderCtrl(panel,ID_FOV_SLDR,"FOV",LABEL, VALUE,SLIDER);

	view_ctrls->Add(FOVSlider->getSizer());
	FOVSlider->setRange(20,75);

	boxSizer->Add(view_ctrls, 0, wxALIGN_LEFT|wxALL,0);
	view_ctrls->SetMinSize(wxSize(TABS_WIDTH-TABS_BORDER,-1));

	hline = new wxBoxSizer(wxHORIZONTAL);

    wxString dmodes[]={"Points","lines","Solid","Shaded"};
    drawmode=new wxRadioBox(panel,ID_DRAWTYPE,wxT("Draw Mode"),wxPoint(-1,-1),wxSize(-1,50),4,
    		dmodes,4,wxRA_SPECIFY_COLS);

    hline->Add(drawmode, 0, wxALIGN_LEFT|wxALL,0);

    wxString lmodes[]={"OGL","Phong","Mix"};
    lightmode=new wxRadioBox(panel,ID_LMODE,wxT("Lighting Model"),wxPoint(-1,-1),wxSize(-1,50),3,
    		lmodes,3,wxRA_SPECIFY_COLS);

    hline->Add(lightmode, 0, wxALIGN_LEFT|wxALL,0);

    boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
}

int VtxSceneTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;
	wxMenu menu;

	if(!expanded && !object()->isEmpty())
		return menu_action;

	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		menu.AppendSubMenu(addmenu,"Add");
		PopupMenu(&menu);
	}
	return menu_action;
}

void VtxSceneTabs::OnDrawMode(wxCommandEvent& event){
	int mode=event.GetSelection();
	if(mode==0)
		Render.show_points();
	else if(mode==1)
		Render.show_lines();
	else if(mode==2)
		Render.show_solid();
	else
		Render.show_shaded();
	TheScene->set_changed_render();
}

void VtxSceneTabs::OnAAEnable(wxCommandEvent& event){
    Render.set_dealias(!Render.dealias());
	TheScene->set_changed_render();
}

void VtxSceneTabs::OnBigEnable(wxCommandEvent& event){
    Raster.set_filter_big(!Raster.filter_big());
	TheScene->set_changed_render();
}

void VtxSceneTabs::OnAveEnable(wxCommandEvent& event){
	Render.set_avenorms(!Render.avenorms());
	TheScene->set_changed_detail();
}
void VtxSceneTabs::OnUpdateDrawMode(wxUpdateUIEvent& event){
	if(Render.draw_points())
		drawmode->SetSelection(0);
	else if(Render.draw_lines())
		drawmode->SetSelection(1);
	else  if(Render.draw_solid())
		drawmode->SetSelection(2);
	else
		drawmode->SetSelection(3);
}

void VtxSceneTabs::OnLightMode(wxCommandEvent& event){
	int mode=event.GetSelection();
	Render.set_light_mode(mode);
	TheScene->set_changed_render();
}
void VtxSceneTabs::OnUpdateLightMode(wxUpdateUIEvent& event){
	int mode=Render.light_mode();
	lightmode->SetSelection(mode);
}

void VtxSceneTabs::OnRenderQualitySelect(wxCommandEvent& event){
	int mode=event.GetSelection();
	TheScene->set_quality(mode);
}
void VtxSceneTabs::OnUpdateRenderQuality(wxUpdateUIEvent& event){
	int mode=TheScene->render_quality;
	if(mode!=render_quality->GetSelection())
		render_quality->SetSelection(mode);
}
void VtxSceneTabs::OnGenerateQualitySelect(wxCommandEvent& event){
	int mode=event.GetSelection();
	TheScene->generate_quality=mode;
}
void VtxSceneTabs::OnUpdateGenerateQuality(wxUpdateUIEvent& event){
	int mode=TheScene->generate_quality;
	if(mode!=generate_quality->GetSelection())
		generate_quality->SetSelection(mode);
}

void VtxSceneTabs::OnSetLayersSelect(wxCommandEvent& event){
	int mode=event.GetSelection();
	Planetoid::set_layers=mode;
}
void VtxSceneTabs::OnUpdateSetLayers(wxUpdateUIEvent& event){
	int mode=Planetoid::set_layers;
	if(mode!=set_layers->GetSelection())
		set_layers->SetSelection(mode);
}

void VtxSceneTabs::OnTesslevel(wxCommandEvent& event){
	Map::setTessLevel(m_tesslevel->GetSelection()+1);
	TheScene->set_changed_render();
}

void VtxSceneTabs::OnTempMode(wxCommandEvent& event){
	int mode=event.GetSelection();
	TheScene->set_tempmode(mode);
}
void VtxSceneTabs::setObjAttributes(){
	Scene *obj=object();
	obj->tex_mip=TexMipSlider->getValue();
	obj->color_mip=ColorMipSlider->getValue();
	obj->bump_mip=BumpMipSlider->getValue();
	obj->freq_mip=FreqMipSlider->getValue();
	Map::setTessLevel(m_tesslevel->GetSelection()+1);

	TheScene->set_changed_render();
}
void VtxSceneTabs::updateControls(){
	if(changing)
		return;
	Scene *obj=object();
	getTime();
	getRate();
	TexMipSlider->setValue(obj->tex_mip);
	ColorMipSlider->setValue(obj->color_mip);
	BumpMipSlider->setValue(obj->bump_mip);
	FreqMipSlider->setValue(obj->freq_mip);
	FOVSlider->setValue(obj->fov);
	LODSlider->setValue(obj->cellsize);
	updateSlider(ColorAmpSlider,Raster.filter_color_ampl);
	updateSlider(NormalAmpSlider,Raster.filter_normal_ampl);
	updateSlider(HDRMinSlider,Raster.hdr_min);
	updateSlider(HDRMaxSlider,Raster.hdr_max);
	m_aa_check->SetValue(Render.dealias());
	m_show_check->SetValue(Raster.filter_show());
	m_ave_check->SetValue(Render.avenorms());
	m_hdr->SetValue(Raster.hdr());
	m_shadows->SetValue(Raster.shadows());
	m_water->SetValue(Render.show_water());
	m_tesslevel->SetSelection(Map::tessLevel()-1);
	m_big->SetValue(Raster.filter_big());
	m_grid->SetValue(TheScene->enable_grid);
	m_contours->SetValue(TheScene->enable_contours);
	m_autogrid->SetValue(TheScene->autogrid());
	keep_tmps->SetSelection(TheScene->keep_tmps);
	use_tmps->SetSelection(TheScene->use_tmps);

	updateSlider(GridSpacingSlider,TheScene->grid_spacing);
	updateSlider(ContourSpacingSlider,TheScene->contour_spacing);

	Color c=TheScene->phi_color;
	m_phi_color->SetColour(wxColor(c.rb(),c.gb(),c.bb()));
	c=TheScene->theta_color;
	m_theta_color->SetColour(wxColor(c.rb(),c.gb(),c.bb()));
	c=TheScene->contour_color;
	m_contour_color->SetColour(wxColor(c.rb(),c.gb(),c.bb()));
	c=TheScene->text_color;
	m_text_color->SetColour(wxColor(c.rb(),c.gb(),c.bb()));

	m_occlusion->SetValue(Adapt.overlap_test());
	m_clip->SetValue(Adapt.clip_test());
	m_backface->SetValue(Adapt.back_test());
	m_curvature->SetValue(Adapt.curve_test());

	updateSlider(ShadowBlurSlider,Raster.shadow_blur);
	updateSlider(ShadowResSlider,Raster.shadow_vsteps);
	updateSlider(ShadowFovSlider,Raster.shadow_fov);
	updateSlider(ShadowDovSlider,Raster.shadow_dov);
	
	tempmode->SetSelection(TheScene->tempmode());
}
void VtxSceneTabs::OnTimeScale(wxCommandEvent& event){
	if(TheScene->motion()) changing=true;
	setTime();
	changing=false;

}
void VtxSceneTabs::OnRateScale(wxCommandEvent& event){
	setRate();
}
void VtxSceneTabs::OnUpdateTime(wxUpdateUIEvent& event){
	//if(!changing&& TheScene->autotm()){
	if(!changing)
		getTime();
}
void VtxSceneTabs::OnUpdateLOD(wxUpdateUIEvent& event){
	if(!changing)
		LODSlider->setValue(TheScene->cellsize);
}

void VtxSceneTabs::OnUpdateContourSpacing(wxUpdateUIEvent& event) {
	if(!changing && TheScene->autogrid()){
		ContourSpacingSlider->setValue(TheScene->contour_spacing);
		GridSpacingSlider->setValue(TheScene->grid_spacing);
	}
}

void VtxSceneTabs::OnTimeSlider(wxScrollEvent& event){
	if(TheScene->motion()) changing=true;
	TimeSlider->setValueFromSlider();
	setTime();
	//changing=false;
	//TimeSlider->setValueFromSlider();
}

void VtxSceneTabs::OnEndTimeSlider(wxScrollEvent& event){
	setTime();
	changing=false;
}
void VtxSceneTabs::OnTimeText(wxCommandEvent& event){
	if(TheScene->motion()) changing=true;
	TimeSlider->setValueFromText();
	setTime();
	//changing=false;
}
void VtxSceneTabs::OnMergeMode(wxCommandEvent& event){
	int mode=event.GetSelection();
	Adapt.set_mindcnt(mode);
	TheScene->rebuild_all();
}

void VtxSceneTabs::OnUpdateMergeMode(wxUpdateUIEvent& event){
	int mode=Adapt.mindcnt();
	mergemode->SetSelection(mode);
}
void VtxSceneTabs::getTime(){
//	if(!TheScene->autotm())
//		return;
	changing=true;
	double time=object()->time;
	if(time>=1){
		double exp=floor(log10(time));
		double rem=time/pow(10,exp);
		m_time_scale->SetSelection((int)exp);
		TimeSlider->setValue(rem);
	}
	else{
		m_time_scale->SetSelection(0);
		TimeSlider->setValue(0.0);
	}
		
	changing=false;
}

void VtxSceneTabs::setTime(){
	double rem=TimeSlider->getValue();
	double exp=m_time_scale->GetSelection();
	double time=rem*pow(10,exp);
	TheScene->set_time(time);
	TheScene->set_changed_time();
	TheScene->set_changed_render();
}

void VtxSceneTabs::getRate(){
	if(!TheScene->autotm()){
		double rate=object()->delt;
		double exp=floor(log10(rate));
		double rem=rate/pow(10,exp);
		m_rate_scale->SetSelection((int)exp);
		RateSlider->setValue(rem);
	}
}

void VtxSceneTabs::setRate(){
	double rem=RateSlider->getValue();
	double exp=m_rate_scale->GetSelection();
	double rate=rem*pow(10,exp);
	TheScene->delt=rate;
	TheScene->set_changed_render();
}

