
#include "VtxGalaxyTabs.h"
#include "RenderOptions.h"
#include "VtxSceneDialog.h"
#include <wx/event.h>
#include <wx/colordlg.h>
#undef LABEL2
#define LABEL2   55
#define LABEL1   70
#define VALUE1   50
#define PLABEL   0
#define PVALUE   50
#define PSLIDER  70

#define LINE_HEIGHT 30
#define LINE_WIDTH TABS_WIDTH-TABS_BORDER
#define BOX_WIDTH TABS_WIDTH

#define BTNWIDTH 75
#define BTNSIZE wxSize(BTNWIDTH,30)
//########################### VtxGalaxyTabs Class ########################

// define all resource ids here
enum {
	OBJ_VIEWOBJ,
	OBJ_DELETE,
	OBJ_SAVE,
	ID_NAME_TEXT,
	ID_TYPE_TEXT,
	ID_CELLSIZE_SLDR,
	ID_CELLSIZE_TEXT,
    ID_RADIUS_SLDR,
    ID_RADIUS_TEXT,
    ID_ORIGIN_X_SLDR,
    ID_ORIGIN_X_TEXT,
    ID_ORIGIN_Y_SLDR,
    ID_ORIGIN_Y_TEXT,
    ID_ORIGIN_Z_SLDR,
    ID_ORIGIN_Z_TEXT,
    ID_ROTATION_X_SLDR,
    ID_ROTATION_X_TEXT,
    ID_ROTATION_Y_SLDR,
    ID_ROTATION_Y_TEXT,
    ID_ROTATION_Z_SLDR,
    ID_ROTATION_Z_TEXT,
    ID_VIEW_THETA_SLDR,
    ID_VIEW_THETA_TEXT,
    ID_VIEW_PHI_SLDR,
    ID_VIEW_PHI_TEXT,
    ID_VIEW_RADIUS_SLDR,
    ID_VIEW_RADIUS_TEXT,
    ID_EYE_ANGLE_SLDR,
    ID_EYE_ANGLE_TEXT,
    ID_EYE_TILT_SLDR,
    ID_EYE_TILT_TEXT,
    ID_EYE_SKEW_SLDR,
    ID_EYE_SKEW_TEXT,
    ID_CENTER_RADIUS_SLDR,
    ID_CENTER_RADIUS_TEXT,
    ID_OUTER_RADIUS_SLDR,
    ID_OUTER_RADIUS_TEXT,
    ID_TWIST_RADIUS_SLDR,
    ID_TWIST_RADIUS_TEXT,
    ID_TWIST_ANGLE_SLDR,
    ID_TWIST_ANGLE_TEXT,
    ID_SHAPE_INT_SLDR,
    ID_SHAPE_INT_TEXT,
    ID_SHAPE_EXT_SLDR,
    ID_SHAPE_EXT_TEXT,
    ID_COMPRESSION_SLDR,
    ID_COMPRESSION_TEXT,
    ID_CTR_BIAS_SLDR,
    ID_CTR_BIAS_TEXT,
    ID_OVERHEAD,
    ID_OBLIQUE,
    ID_CORE,
    ID_RIM,
    ID_EDGEON,
    ID_SPIRAL,
    ID_BARRED,
    ID_GLOBULAR,
    ID_ELYPTICAL,
    ID_IRREGULAR,
    ID_DENSITY_SLDR,
    ID_DENSITY_TEXT,
    ID_GRADIENT_SLDR,
    ID_GRADIENT_TEXT,
    ID_DIFFUSION_SLDR,
    ID_DIFFUSION_TEXT,
    ID_SCATTER_SLDR,
    ID_SCATTER_TEXT,
    ID_AMPLITUDE_SLDR,
    ID_AMPLITUDE_TEXT,
    ID_VERTICAL_SLDR,
    ID_VERTICAL_TEXT,
    ID_SATURATION_SLDR,
    ID_SATURATION_TEXT,
    ID_CUTOFF_SLDR,
    ID_CUTOFF_TEXT,
    ID_DEFAULT,
    ID_NOISE_EXPR,
    ID_NEAR_SIZE_SLDR,
    ID_NEAR_SIZE_TEXT,
    ID_NEAR_MAX_SLDR,
    ID_NEAR_MAX_TEXT,
    ID_FAR_SIZE_SLDR,
    ID_FAR_SIZE_TEXT,
    ID_EXT_BIAS_SLDR,
    ID_EXT_BIAS_TEXT,
    ID_NOVA_SIZE_SLDR,
    ID_NOVA_SIZE_TEXT,
    ID_NOVA_DENSITY_SLDR,
    ID_NOVA_DENSITY_TEXT,
    ID_CMIX_SLDR,
    ID_CMIX_TEXT,
    ID_CBIAS_SLDR,
    ID_CBIAS_TEXT,
    ID_COLOR_EXPR,
    ID_COLORS,
};


IMPLEMENT_CLASS(VtxGalaxyTabs, wxNotebook )

BEGIN_EVENT_TABLE(VtxGalaxyTabs, wxNotebook)

EVT_TEXT_ENTER(ID_NAME_TEXT,VtxGalaxyTabs::OnNameText)

EVT_TEXT_ENTER(ID_NOISE_EXPR,VtxGalaxyTabs::OnChanged)
EVT_TEXT_ENTER(ID_COLOR_EXPR,VtxGalaxyTabs::OnChanged)

EVT_BUTTON(ID_DEFAULT,VtxGalaxyTabs::OnSetDefault)

SET_SLIDER_EVENTS(NEAR_SIZE,VtxGalaxyTabs,NearSize)
SET_SLIDER_EVENTS(NEAR_MAX,VtxGalaxyTabs,NearMax)
SET_SLIDER_EVENTS(FAR_SIZE,VtxGalaxyTabs,FarSize)
SET_SLIDER_EVENTS(EXT_BIAS,VtxGalaxyTabs,Variability)
SET_SLIDER_EVENTS(CMIX,VtxGalaxyTabs,CMix)
SET_SLIDER_EVENTS(CBIAS,VtxGalaxyTabs,CBias)
SET_SLIDER_EVENTS(NOVA_SIZE,VtxGalaxyTabs,NovaSize)
SET_SLIDER_EVENTS(NOVA_DENSITY,VtxGalaxyTabs,NovaDensity)

SET_SLIDER_EVENTS(DENSITY,VtxGalaxyTabs,Density)
SET_SLIDER_EVENTS(GRADIENT,VtxGalaxyTabs,Gradient)
SET_SLIDER_EVENTS(DIFFUSION,VtxGalaxyTabs,Diffusion)
SET_SLIDER_EVENTS(SCATTER,VtxGalaxyTabs,Scatter)
SET_SLIDER_EVENTS(AMPLITUDE,VtxGalaxyTabs,Amplitude)
SET_SLIDER_EVENTS(VERTICAL,VtxGalaxyTabs,Vertical)
SET_SLIDER_EVENTS(SATURATION,VtxGalaxyTabs,Saturation)
SET_SLIDER_EVENTS(CUTOFF,VtxGalaxyTabs,Cutoff)

SET_SLIDER_EVENTS(CENTER_RADIUS,VtxGalaxyTabs,CenterRadius)
SET_SLIDER_EVENTS(OUTER_RADIUS,VtxGalaxyTabs,OuterRadius)
SET_SLIDER_EVENTS(TWIST_RADIUS,VtxGalaxyTabs,TwistRadius)
SET_SLIDER_EVENTS(TWIST_ANGLE,VtxGalaxyTabs,TwistAngle)
SET_SLIDER_EVENTS(SHAPE_INT,VtxGalaxyTabs,ShapeInt)
SET_SLIDER_EVENTS(SHAPE_EXT,VtxGalaxyTabs,ShapeExt)
SET_SLIDER_EVENTS(COMPRESSION,VtxGalaxyTabs,Compression)
SET_SLIDER_EVENTS(CTR_BIAS,VtxGalaxyTabs,CtrBias)

EVT_BUTTON(ID_SPIRAL,VtxGalaxyTabs::OnSpiral)
EVT_BUTTON(ID_BARRED,VtxGalaxyTabs::OnBarred)
EVT_BUTTON(ID_GLOBULAR,VtxGalaxyTabs::OnGlobular)
EVT_BUTTON(ID_ELYPTICAL,VtxGalaxyTabs::OnElyptical)
EVT_BUTTON(ID_IRREGULAR,VtxGalaxyTabs::OnIrregular)

EVT_BUTTON(ID_OVERHEAD,VtxGalaxyTabs::OnOverheadView)
EVT_BUTTON(ID_OBLIQUE,VtxGalaxyTabs::OnObliqueView)
EVT_BUTTON(ID_CORE,VtxGalaxyTabs::OnCoreView)
EVT_BUTTON(ID_RIM,VtxGalaxyTabs::OnRimView)
EVT_BUTTON(ID_EDGEON,VtxGalaxyTabs::OnEdgeView)

SET_SLIDER_EVENTS(VIEW_THETA,VtxGalaxyTabs,ViewTheta)
SET_SLIDER_EVENTS(VIEW_PHI,VtxGalaxyTabs,ViewPhi)
SET_SLIDER_EVENTS(VIEW_RADIUS,VtxGalaxyTabs,ViewRadius)
SET_SLIDER_EVENTS(EYE_ANGLE,VtxGalaxyTabs,EyeAngle)
SET_SLIDER_EVENTS(EYE_TILT,VtxGalaxyTabs,EyeTilt)
SET_SLIDER_EVENTS(EYE_SKEW,VtxGalaxyTabs,EyeSkew)

SET_SLIDER_EVENTS(CELLSIZE,VtxGalaxyTabs,CellSize)
SET_SLIDER_EVENTS(RADIUS,VtxGalaxyTabs,Size)

SET_SLIDER_EVENTS(ORIGIN_X,VtxGalaxyTabs,OriginX)
SET_SLIDER_EVENTS(ORIGIN_Y,VtxGalaxyTabs,OriginY)
SET_SLIDER_EVENTS(ORIGIN_Z,VtxGalaxyTabs,OriginZ)

SET_SLIDER_EVENTS(ROTATION_X,VtxGalaxyTabs,RotationX)
SET_SLIDER_EVENTS(ROTATION_Y,VtxGalaxyTabs,RotationY)
SET_SLIDER_EVENTS(ROTATION_Z,VtxGalaxyTabs,RotationZ)

EVT_MENU(TABS_ENABLE,VtxGalaxyTabs::OnEnable)
EVT_UPDATE_UI(TABS_ENABLE,VtxGalaxyTabs::OnUpdateEnable)

EVT_MENU(OBJ_VIEWOBJ,VtxGalaxyTabs::OnViewObj)
EVT_UPDATE_UI(OBJ_VIEWOBJ,VtxGalaxyTabs::OnUpdateViewObj)

EVT_MENU(OBJ_DELETE,VtxGalaxyTabs::OnDelete)
EVT_MENU(OBJ_SAVE,VtxGalaxyTabs::OnSave)

EVT_MENU_RANGE(TABS_ADD,TABS_ADD+TABS_MAX_IDS,VtxGalaxyTabs::OnAddItem)
EVT_COMMAND_RANGE(ID_COLORS, ID_COLORS+NUM_COLORS,wxEVT_COMMAND_BUTTON_CLICKED,VtxGalaxyTabs::OnColorButton)

END_EVENT_TABLE()

VtxGalaxyTabs::VtxGalaxyTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size,  style,name)
{
	Create(parent, id, pos,size, style, name);
}

bool VtxGalaxyTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size ,
	long style,
	const wxString& name)
{
    if (!VtxTabsMgr::Create(parent, id, pos, size,  style,name))
        return false;

	wxNotebookPage *page=new wxPanel(this,wxID_ANY);
    AddPropertiesTab(page);
    AddPage(page,wxT("Object"),true);

    page=new wxPanel(this,wxID_ANY);
    AddViewTab(page);
    AddPage(page,wxT("View"),false);

    page=new wxPanel(this,wxID_ANY);
    AddShapeTab(page);
    AddPage(page,wxT("Shape"),false);

    page=new wxPanel(this,wxID_ANY);
    AddDensityTab(page);
    AddPage(page,wxT("Density"),false);

    page=new wxPanel(this,wxID_ANY);
    AddStarsTab(page);
    AddPage(page,wxT("Render"),false);

    return true;
}

int VtxGalaxyTabs::showMenu(bool expanded){
	menu_action=TABS_NONE;
	wxMenu menu;
	menu.AppendCheckItem(TABS_ENABLE,wxT("Show"));

	menu.AppendCheckItem(OBJ_VIEWOBJ,wxT("Set View"));
	menu.AppendSeparator();
	menu.Append(OBJ_DELETE,wxT("Delete"));
	menu.Append(OBJ_SAVE,wxT("Save.."));

	wxMenu *addmenu=getAddMenu(object());

	if(addmenu){
		menu.AppendSeparator();
		menu.AppendSubMenu(addmenu,"Add");
	}
	PopupMenu(&menu);
	return menu_action;
}

//-------------------------------------------------------------
// VtxGalaxyTabs::AddStarsTab() Add stars controls tab
//-------------------------------------------------------------
void VtxGalaxyTabs::AddStarsTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxBoxSizer* star_size = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Star Size"));

	NearSizeSlider=new SliderCtrl(panel,ID_NEAR_SIZE_SLDR,"Near",LABEL2, VALUE2,SLIDER2);
	NearSizeSlider->setRange(0,5);

	star_size->Add(NearSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	FarSizeSlider=new SliderCtrl(panel,ID_FAR_SIZE_SLDR,"Far",LABEL2, VALUE2,SLIDER2);
	FarSizeSlider->setRange(0,5);

	star_size->Add(FarSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	star_size->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(star_size, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* nova_stars = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Nova"));

	NovaSizeSlider=new SliderCtrl(panel,ID_NOVA_SIZE_SLDR,"Size",LABEL2, VALUE2,SLIDER2);
	NovaSizeSlider->setRange(0,5);

	nova_stars->Add(NovaSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	NovaDensitySlider=new SliderCtrl(panel,ID_NOVA_DENSITY_SLDR," % ",LABEL2, VALUE2,SLIDER2);
	NovaDensitySlider->setRange(0,1,0,100);

	nova_stars->Add(NovaDensitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	nova_stars->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(nova_stars, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* distance = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Stars"));

	NearMaxSlider=new SliderCtrl(panel,ID_NEAR_MAX_SLDR,"Far",LABEL2, VALUE2,SLIDER2);
	NearMaxSlider->setRange(10,500);
	distance->Add(NearMaxSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);


	VariabilitySlider=new SliderCtrl(panel,ID_EXT_BIAS_SLDR,"Random",LABEL2, VALUE2,SLIDER2);
	VariabilitySlider->setRange(0.5,4);
	distance->Add(VariabilitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	distance->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(distance, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* color = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Color"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	ColorExpr=new ExprTextCtrl(panel,ID_COLOR_EXPR,"",0,260);
	hline->Add(ColorExpr->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
	hline->AddSpacer(10);

	for(int i=0;i<NUM_COLORS;i++){
		wxColor col("RGB(255,255,255)");
		colors[i]=new wxButton(panel,ID_COLORS+i,"",wxDefaultPosition,wxSize(20,25));
		colors[i]->SetBackgroundColour(col);
		hline->Add(colors[i], 0, wxALIGN_LEFT|wxALL,0);
	}
	hline->SetMinSize(wxSize(LINE_WIDTH-TABS_BORDER,LINE_HEIGHT));

	color->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	hline = new wxBoxSizer(wxHORIZONTAL);

	CMixSlider=new SliderCtrl(panel,ID_CMIX_SLDR,"Intensity",LABEL2, VALUE2,SLIDER2);
	CMixSlider->setRange(0,1);

	hline->Add(CMixSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	CBiasSlider=new SliderCtrl(panel,ID_CBIAS_SLDR,"Bias",LABEL2, VALUE2,SLIDER2);
	CBiasSlider->setRange(0,2);
	hline->Add(CBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH-TABS_BORDER,LINE_HEIGHT));

	color->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(color, 0, wxALIGN_LEFT|wxALL,0);
}

//-------------------------------------------------------------
// VtxGalaxyTabs::AddDensityTab() Add density controls tab
//-------------------------------------------------------------
void VtxGalaxyTabs::AddDensityTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	wxBoxSizer* gradient = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Gradient"));

	//wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	DensitySlider=new SliderCtrl(panel,ID_DENSITY_SLDR,"Vertical",LABEL1, VALUE2,SLIDER2);
	DensitySlider->setRange(0,2,0,1);
	DensitySlider->setValue(0.1);

	gradient->Add(DensitySlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	GradientSlider=new SliderCtrl(panel,ID_GRADIENT_SLDR,"Radial",LABEL2, VALUE2,SLIDER2);
	GradientSlider->setRange(0,2,0,1);
	GradientSlider->setValue(1);
	gradient->Add(GradientSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	//gradient->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT));
	boxSizer->Add(gradient, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* distribution = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Distribution"));

	//hline = new wxBoxSizer(wxHORIZONTAL);

	DiffusionSlider=new SliderCtrl(panel,ID_DIFFUSION_SLDR,"Diffusion",LABEL1, VALUE2,SLIDER2);
	DiffusionSlider->setRange(0.01,4,0,1.0);
	DiffusionSlider->setValue(1.0);

	distribution->Add(DiffusionSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	ScatterSlider=new SliderCtrl(panel,ID_SCATTER_SLDR,"Scatter",LABEL2, VALUE2,SLIDER2);
	ScatterSlider->setRange(0,20,0,1);
	ScatterSlider->setValue(0.0);
	distribution->Add(ScatterSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	//distribution->SetMinSize(wxSize(BOX_WIDTH,LINE_HEIGHT));
	//density->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	boxSizer->Add(distribution, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* random = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Noise"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	AmplitudeSlider=new SliderCtrl(panel,ID_AMPLITUDE_SLDR,"Amplitude",LABEL1, VALUE2,SLIDER2);
	AmplitudeSlider->setRange(0,1);
	AmplitudeSlider->setValue(0.0);

	hline->Add(AmplitudeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	VerticalSlider=new SliderCtrl(panel,ID_VERTICAL_SLDR,"Vertical",LABEL2, VALUE2,SLIDER2);
	VerticalSlider->setRange(0,1);
	VerticalSlider->setValue(0.0);
	hline->Add(VerticalSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	//hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	random->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	SaturationSlider=new SliderCtrl(panel,ID_SATURATION_SLDR,"Saturation",LABEL1, VALUE2,SLIDER2);
	SaturationSlider->setRange(0,4,0,1.0);
	SaturationSlider->setValue(1.0);

	hline->Add(SaturationSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	CutoffSlider=new SliderCtrl(panel,ID_CUTOFF_SLDR,"Cutoff",LABEL2, VALUE2,SLIDER2);
	CutoffSlider->setRange(-1,1);
	CutoffSlider->setValue(0.0);
	hline->Add(CutoffSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	//hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	random->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	NoiseExpr=new ExprTextCtrl(panel,ID_NOISE_EXPR,"",0,LINE_WIDTH-100);
	hline->Add(NoiseExpr->getSizer(), 0, wxALIGN_LEFT|wxALL,3);

	hline->AddSpacer(2);

	m_default=new wxButton(panel,ID_DEFAULT,"Default",wxDefaultPosition,wxSize(60,25));
	hline->Add(m_default,0,wxALIGN_LEFT|wxALL,2);

	//hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));

	random->Add(hline, 0, wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(random,0,wxALIGN_LEFT|wxALL,0);
}

//-------------------------------------------------------------
// VtxGalaxyTabs::AddShapeTab() Add shape controls tab
//-------------------------------------------------------------
void VtxGalaxyTabs::AddShapeTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxBoxSizer* dflt_shapes = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Default Shape"));

	wxButton	*m_spiral;
	wxButton	*m_irregular;
	wxButton	*m_barred;
	wxButton	*m_globular;
	wxButton	*m_elyptical;

	m_spiral=new wxButton(panel,ID_SPIRAL,"Spiral",wxDefaultPosition,BTNSIZE);
	m_barred=new wxButton(panel,ID_BARRED,"Barred",wxDefaultPosition,BTNSIZE);
	m_globular=new wxButton(panel,ID_GLOBULAR,"Globular",wxDefaultPosition,BTNSIZE);
	m_elyptical=new wxButton(panel,ID_ELYPTICAL,"Elyptical",wxDefaultPosition,BTNSIZE);
	m_irregular=new wxButton(panel,ID_IRREGULAR,"Irregular",wxDefaultPosition,BTNSIZE);

	dflt_shapes->Add(m_spiral,0,wxALIGN_LEFT|wxALL,0);
	dflt_shapes->Add(m_barred,0,wxALIGN_LEFT|wxALL,0);
	dflt_shapes->Add(m_globular,0,wxALIGN_LEFT|wxALL,0);
	dflt_shapes->Add(m_elyptical,0,wxALIGN_LEFT|wxALL,0);
	dflt_shapes->Add(m_irregular,0,wxALIGN_LEFT|wxALL,0);
	dflt_shapes->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(dflt_shapes, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* radius = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Radius"));

	CenterRadiusSlider=new SliderCtrl(panel,ID_CENTER_RADIUS_SLDR,"Core",LABEL2, VALUE2,SLIDER2);
	CenterRadiusSlider->setRange(0,1);
	CenterRadiusSlider->setValue(0.1);

	radius->Add(CenterRadiusSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	OuterRadiusSlider=new SliderCtrl(panel,ID_OUTER_RADIUS_SLDR,"Rim",LABEL2S, VALUE2,SLIDER2);
	OuterRadiusSlider->setRange(0,1);
	OuterRadiusSlider->setValue(1);
	radius->Add(OuterRadiusSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	radius->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(radius, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* twist = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Twist"));

	TwistRadiusSlider=new SliderCtrl(panel,ID_TWIST_RADIUS_SLDR,"Radius",LABEL2, VALUE2,SLIDER2);
	TwistRadiusSlider->setRange(0,2.0);
	TwistRadiusSlider->setValue(1.0);

	twist->Add(TwistRadiusSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	TwistAngleSlider=new SliderCtrl(panel,ID_TWIST_ANGLE_SLDR,"Angle",LABEL2S, VALUE2,SLIDER2);
	TwistAngleSlider->setRange(0,720);
	TwistAngleSlider->setValue(20.0);
	twist->Add(TwistAngleSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	twist->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(twist, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* shape = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Surface"));

	ShapeIntSlider=new SliderCtrl(panel,ID_SHAPE_INT_SLDR,"In",LABEL2, VALUE2,SLIDER2);
	ShapeIntSlider->setRange(0,1,0,100);
	ShapeIntSlider->setValue(0.0);

	shape->Add(ShapeIntSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	ShapeExtSlider=new SliderCtrl(panel,ID_SHAPE_EXT_SLDR,"Out",LABEL2S, VALUE2,SLIDER2);
	ShapeExtSlider->setRange(0.0001,0.1,0,100);
	ShapeExtSlider->setValue(0.0);
	shape->Add(ShapeExtSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	shape->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(shape, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* compression = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Compression"));

	CompressionSlider=new SliderCtrl(panel,ID_COMPRESSION_SLDR,"Vertical",LABEL2, VALUE2,SLIDER2);
	CompressionSlider->setRange(0,1.0);
	CompressionSlider->setValue(1.0);

	compression->Add(CompressionSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	CtrBiasSlider=new SliderCtrl(panel,ID_CTR_BIAS_SLDR,"Radial",LABEL2S, VALUE2,SLIDER2);
	CtrBiasSlider->setRange(0,1);
	CtrBiasSlider->setValue(0.0);
	compression->Add(CtrBiasSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	compression->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(compression, 0, wxALIGN_LEFT|wxALL,0);
}

//-------------------------------------------------------------
// VtxGalaxyTabs::AddViewTab() Add view controls tab
//-------------------------------------------------------------
void VtxGalaxyTabs::AddViewTab(wxWindow *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* set_view = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Set View"));

	m_core=new wxButton(panel,ID_CORE,"Core",wxDefaultPosition,BTNSIZE);
	m_rim=new wxButton(panel,ID_RIM,"Rim",wxDefaultPosition,BTNSIZE);
	m_overhead=new wxButton(panel,ID_OVERHEAD,"Top",wxDefaultPosition,BTNSIZE);
	m_edgeon=new wxButton(panel,ID_EDGEON,"Edge",wxDefaultPosition,BTNSIZE);
	m_oblique=new wxButton(panel,ID_OBLIQUE,"Oblique",wxDefaultPosition,BTNSIZE);
	set_view->Add(m_core,0,wxALIGN_LEFT|wxALL,0);
	set_view->Add(m_rim,0,wxALIGN_LEFT|wxALL,0);
	set_view->Add(m_overhead,0,wxALIGN_LEFT|wxALL,0);
	set_view->Add(m_edgeon,0,wxALIGN_LEFT|wxALL,0);
	set_view->Add(m_oblique,0,wxALIGN_LEFT|wxALL,0);
	set_view->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(set_view, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* view = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Position   Radius-Theta-Phi"));
	ViewRadiusSlider = new SliderCtrl(panel,ID_VIEW_RADIUS_SLDR,"R",PLABEL,PVALUE,PSLIDER);
	ViewRadiusSlider->setRange(0,1e5);
	ViewThetaSlider = new SliderCtrl(panel,ID_VIEW_THETA_SLDR,"T",PLABEL,PVALUE,PSLIDER);
	ViewThetaSlider->setRange(0,360);
	ViewPhiSlider = new SliderCtrl(panel,ID_VIEW_PHI_SLDR,"P",PLABEL,PVALUE,PSLIDER);
	ViewPhiSlider->setRange(0,360);
	view->Add(ViewRadiusSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	view->Add(ViewThetaSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	view->Add(ViewPhiSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	view->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(view, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* eye = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Rotation   X-Y-Z"));
	EyeAngleSlider = new SliderCtrl(panel,ID_EYE_ANGLE_SLDR,"X",PLABEL,PVALUE,PSLIDER);
	EyeAngleSlider->setRange(0,360);
	EyeTiltSlider = new SliderCtrl(panel,ID_EYE_TILT_SLDR,"Y",PLABEL,PVALUE,PSLIDER);
	EyeTiltSlider->setRange(0,360);
	EyeSkewSlider = new SliderCtrl(panel,ID_EYE_SKEW_SLDR,"Z",PLABEL,PVALUE,PSLIDER);
	EyeSkewSlider->setRange(0,360);
	eye->Add(EyeAngleSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	eye->Add(EyeTiltSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	eye->Add(EyeSkewSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	eye->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));

	boxSizer->Add(eye, 0, wxALIGN_LEFT|wxALL,0);

	topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}

//-------------------------------------------------------------
// VtxGalaxyTabs::AddPropertiesTab() Add properties controls tab
//-------------------------------------------------------------
void VtxGalaxyTabs::AddPropertiesTab(wxWindow *panel){

    // A top-level sizer

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    // A second box sizer to give more space around the controls

    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	object_name=new TextCtrl(panel,ID_NAME_TEXT,"Name",LABEL2,VALUE1+SLIDER2);
	hline->Add(object_name->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	hline->AddSpacer(5);

	object_type=new StaticTextCtrl(panel,ID_TYPE_TEXT,"Type",LABEL2S,VALUE1+SLIDER2);
	hline->Add(object_type->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	hline = new wxBoxSizer(wxHORIZONTAL);

	SizeSlider=new SliderCtrl(panel,ID_RADIUS_SLDR,"Size(ly)",LABEL2, VALUE1,SLIDER2);
	SizeSlider->setRange(100,100000);
	SizeSlider->setValue(10000);

	hline->Add(SizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	CellSizeSlider=new SliderCtrl(panel,ID_CELLSIZE_SLDR,"Grid",LABEL2S, VALUE1,SLIDER2);
	CellSizeSlider->setRange(0.25,4);
	CellSizeSlider->setValue(1);
	hline->Add(CellSizeSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);

	hline->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));
	boxSizer->Add(hline, 0, wxALIGN_LEFT|wxALL,0);

	wxBoxSizer* origin = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Origin"));
	OriginXSlider = new SliderCtrl(panel,ID_ORIGIN_X_SLDR,"X",PLABEL,PVALUE,PSLIDER);
	OriginXSlider->setRange(-1e5,1e5);
	OriginYSlider = new SliderCtrl(panel,ID_ORIGIN_Y_SLDR,"Y",PLABEL,PVALUE,PSLIDER);
	OriginYSlider->setRange(-1e5,1e5);
	OriginZSlider = new SliderCtrl(panel,ID_ORIGIN_Z_SLDR,"Z",PLABEL,PVALUE,PSLIDER);
	OriginZSlider->setRange(-1e5,1e5);
	origin->Add(OriginXSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	origin->Add(OriginYSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	origin->Add(OriginZSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(origin, 0, wxALIGN_LEFT|wxALL,0);

	origin->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));

	wxBoxSizer* rotation = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Rotation"));
	RotationXSlider = new SliderCtrl(panel,ID_ROTATION_X_SLDR,"X",PLABEL,PVALUE,PSLIDER);
	RotationXSlider->setRange(0,360);
	RotationYSlider = new SliderCtrl(panel,ID_ROTATION_Y_SLDR,"Y",PLABEL,PVALUE,PSLIDER);
	RotationYSlider->setRange(0,360);
	RotationZSlider = new SliderCtrl(panel,ID_ROTATION_Z_SLDR,"Z",PLABEL,PVALUE,PSLIDER);
	RotationZSlider->setRange(0,360);
	rotation->Add(RotationXSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	rotation->Add(RotationYSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	rotation->Add(RotationZSlider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
	rotation->SetMinSize(wxSize(LINE_WIDTH,LINE_HEIGHT));

	boxSizer->Add(rotation, 0, wxALIGN_LEFT|wxALL,0);

	topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

}

void VtxGalaxyTabs::updateControls(){
	if(update_needed)
		getObjAttributes();

}

void VtxGalaxyTabs::OnViewObj(wxCommandEvent& event){
	if(!is_viewobj()){
		TheScene->set_viewobj(object());
		TheScene->selobj=object();
		set_view(ORBITAL,0,30,size);
		//TheScene->change_view(ORBITAL);
		menu_action=TABS_VIEWOBJ;
	}
}
void VtxGalaxyTabs::OnUpdateViewObj(wxUpdateUIEvent& event){
	event.Check(is_viewobj());
}

void VtxGalaxyTabs::invalidateRender(){
	Galaxy *galaxy=(Galaxy*)object();
	StarTree *tree=(StarTree*)galaxy->tree;

	TheScene->theta=ViewThetaSlider->getValue();
	TheScene->phi=ViewPhiSlider->getValue();
	TheScene->view_angle=EyeAngleSlider->getValue();
	TheScene->view_skew=EyeSkewSlider->getValue();
	TheScene->view_tilt=EyeTiltSlider->getValue();

	tree->fgpt1=NearSizeSlider->getValue();
	tree->fgfar=NearMaxSlider->getValue()*LY;
	tree->bgpt1=FarSizeSlider->getValue();
	tree->variability=VariabilitySlider->getValue();
	tree->nova_size=NovaSizeSlider->getValue();
	tree->nova_density=NovaDensitySlider->getValue();

	tree->color_mix=CMixSlider->getValue();
	tree->color_bias=CBiasSlider->getValue();

}
void VtxGalaxyTabs::setObjAttributes()
{
	Galaxy *galaxy=(Galaxy*)object();
	StarTree *tree=(StarTree*)galaxy->tree;

	galaxy->origin.x=OriginXSlider->getValue()*LY;
	galaxy->origin.y=OriginYSlider->getValue()*LY;
	galaxy->origin.z=OriginZSlider->getValue()*LY;
	tree->origin=galaxy->origin;
	galaxy->tilt=RotationXSlider->getValue();
	galaxy->orbit_angle=RotationYSlider->getValue();
	galaxy->pitch=RotationZSlider->getValue();

	tree->size=SizeSlider->getValue()*LY;
	galaxy->size=tree->size;
	galaxy->detail=CellSizeSlider->getValue();
	galaxy->setName((char*)object_name->GetValue().ToAscii());

	TheScene->phi=ViewPhiSlider->getValue();
	TheScene->theta=ViewThetaSlider->getValue();
	TheScene->radius=ViewRadiusSlider->getValue()*LY;
	TheScene->height=TheScene->radius;
	TheScene->view_angle=EyeAngleSlider->getValue();
	TheScene->view_tilt=EyeTiltSlider->getValue();
	TheScene->view_skew=EyeSkewSlider->getValue();

	tree->outer_radius=OuterRadiusSlider->getValue();
	tree->center_radius=CenterRadiusSlider->getValue();
	tree->compression=CompressionSlider->getValue();
	tree->twist_angle=TwistAngleSlider->getValue()*RPD;
	tree->twist_radius=TwistRadiusSlider->getValue();
	tree->center_bias=CtrBiasSlider->getValue();
	tree->shape_interior=ShapeIntSlider->getValue();
	tree->shape_exterior=ShapeExtSlider->getValue();

	tree->density_value=DensitySlider->getValue();
	tree->noise_cutoff=CutoffSlider->getValue();
	tree->dispersion=ScatterSlider->getValue();
	tree->diffusion=DiffusionSlider->getValue();
	tree->gradient=GradientSlider->getValue();
	tree->noise_saturation=SaturationSlider->getValue();
	tree->noise_amplitude=AmplitudeSlider->getValue();
	tree->noise_vertical=VerticalSlider->getValue();

	galaxy->setNoiseFunction((char*)NoiseExpr->GetValue().ToAscii());
	galaxy->applyNoiseFunction();

	galaxy->setColorFunction((char*)ColorExpr->GetValue().ToAscii());
	galaxy->applyColorFunction();

	invalidate_object();
}

void VtxGalaxyTabs::getObjAttributes()
{
	if(!update_needed)
		return;
	Galaxy *galaxy=object();
	StarTree *tree=(StarTree*)galaxy->tree;
	size=tree->size;

	updateSlider(SizeSlider,galaxy->size/LY);
	updateSlider(CellSizeSlider,galaxy->detail);

	object_name->SetValue(object_node->node->nodeName());
	object_type->SetValue((char*)tree->typeName());

	updateSlider(OriginXSlider,galaxy->origin.x/LY);
	updateSlider(OriginYSlider,galaxy->origin.y/LY);
	updateSlider(OriginZSlider,galaxy->origin.z/LY);
	updateSlider(RotationXSlider,galaxy->tilt);
	updateSlider(RotationYSlider,galaxy->orbit_angle);
	updateSlider(RotationZSlider,galaxy->pitch);

	updateSlider(ViewRadiusSlider,TheScene->radius/LY);
	updateSlider(ViewThetaSlider,TheScene->theta);
	updateSlider(ViewPhiSlider,TheScene->phi);
	updateSlider(EyeAngleSlider,TheScene->view_angle);
	updateSlider(EyeTiltSlider,TheScene->view_tilt);
	updateSlider(EyeSkewSlider,TheScene->view_skew);

	updateSlider(CenterRadiusSlider,tree->center_radius);
	updateSlider(OuterRadiusSlider,tree->outer_radius);
	updateSlider(TwistRadiusSlider,tree->twist_radius);
	updateSlider(TwistAngleSlider,tree->twist_angle*DPR);
	updateSlider(CompressionSlider,tree->compression);
	updateSlider(CtrBiasSlider,tree->center_bias);
	updateSlider(ShapeIntSlider,tree->shape_interior);
	updateSlider(ShapeExtSlider,tree->shape_exterior);

	updateSlider(DensitySlider,tree->density_value);
	updateSlider(CutoffSlider,tree->noise_cutoff);
	updateSlider(ScatterSlider,tree->dispersion);
	updateSlider(DiffusionSlider,tree->diffusion);
	updateSlider(GradientSlider,tree->gradient);
	updateSlider(SaturationSlider,tree->noise_saturation);
	updateSlider(AmplitudeSlider,tree->noise_amplitude);
	updateSlider(VerticalSlider,tree->noise_vertical);

	char buff[256];
	galaxy->getNoiseFunction(buff);
	NoiseExpr->SetValue(buff);

	updateSlider(NearSizeSlider,tree->fgpt1);
	updateSlider(NearMaxSlider,tree->fgfar/LY);
	updateSlider(FarSizeSlider,tree->bgpt1);
	updateSlider(VariabilitySlider,tree->variability);
	updateSlider(NovaSizeSlider,tree->nova_size);
	updateSlider(NovaDensitySlider,tree->nova_density);

	for(int i=0;i<NUM_COLORS;i++){
		Color c=galaxy->getColor(i);
		wxColor wc(c.rb(),c.gb(),c.bb(),c.ab());
		colors[i]->SetBackgroundColour(wc);
	}

	galaxy->getColorFunction(buff);
	ColorExpr->SetValue(buff);

	updateSlider(CMixSlider,tree->color_mix);
	updateSlider(CBiasSlider,tree->color_bias);

	update_needed=false;

}
//-------------------------------------------------------------
// VtxGalaxyTabs::OnColorButton() set galaxy spline color
//-------------------------------------------------------------
void VtxGalaxyTabs::OnColorButton(wxCommandEvent& event){
	int i=event.GetId()-ID_COLORS;
	//cout << "Color:"<<i<< endl;
	Galaxy *galaxy=(Galaxy*)object();
	Color c=galaxy->getColor(i);
	wxColor wc(c.rb(),c.gb(),c.bb(),c.ab());
	wxColourData data;
	data.SetChooseAlpha(true);
	data.SetColour(wc);
	wxColourDialog dlg(this,&data);

    if (dlg.ShowModal() == wxID_OK){
	    wxColourData retData = dlg.GetColourData();
	    wxColour col = retData.GetColour();
	    colors[i]->SetBackgroundColour(col);
	    //colors[i]->Refresh();
	    Color c;
		c.set_rb(col.Red());
		c.set_gb(col.Green());
		c.set_bb(col.Blue());
		c.set_ab(col.Alpha());

		galaxy->setColor(i,c);
		TheView->set_changed_render();
	}
}

//-------------------------------------------------------------
// VtxGalaxyTabs::set_view() set view point
//-------------------------------------------------------------
void VtxGalaxyTabs::set_view(int type, double t, double p, double r)
{
	Galaxy *galaxy=(Galaxy*)object();

	TheScene->viewtype=ORBITAL;
	TheScene->set_changed_view();
	galaxy->init_view();
	TheScene->theta=t;
	TheScene->phi=p;
	TheScene->height=TheScene->radius=r;
	TheScene->selobj=TheScene->focusobj=0;
	TheScene->set_viewobj(galaxy);
	TheScene->views_mark();
	TheScene->clr_automv();
	if(type==ORBITAL)
		TheScene->gstride=TheScene->vstride=0.1001*size;
	else
		TheScene->gstride=TheScene->vstride=0.1*LY;
	TheScene->minr=0.0;
	TheScene->view->speed=TheScene->gstride;
	TheScene->rebuild_all();
	update_needed=true;
	getObjAttributes();
}

void VtxGalaxyTabs::OnOverheadView(wxCommandEvent& event){
	set_view(ORBITAL,0,90,0.9*size);
}
void VtxGalaxyTabs::OnObliqueView(wxCommandEvent& event){
	set_view(ORBITAL,0,30,size);
}
void VtxGalaxyTabs::OnCoreView(wxCommandEvent& event){
	set_view(GLOBAL,0,0,0);
}
void VtxGalaxyTabs::OnRimView(wxCommandEvent& event){
	set_view(GLOBAL,0,0,0.3*size);
}
void VtxGalaxyTabs::OnEdgeView(wxCommandEvent& event){
	set_view(ORBITAL,0,0,size);
}

void VtxGalaxyTabs::OnSetDefault(wxCommandEvent& event){
	Galaxy *galaxy=(Galaxy*)object();
	StarTree *tree=(StarTree*)galaxy->tree;

	NoiseExpr->SetValue((char*)tree->def_noise_expr);

	invalidate_object();
}

void VtxGalaxyTabs::OnIrregular(wxCommandEvent& event)
{
	Galaxy *galaxy=(Galaxy*)object();
	StarTree *tree=(StarTree*)galaxy->tree;

	tree->dispersion=2;
	tree->compression=0.9;
	tree->noise_vertical=0.05;
	tree->noise_amplitude=1;
	tree->twist_radius=1.5;
	tree->inner_radius=0.8;
	tree->center_bias=1;

	tree->twist_angle=1.4;
	tree->twist_radius=1.2;
	tree->gradient=1.5;
	tree->outer_radius=0.7;
	tree->density_value=0.4;
	tree->center_radius=0.0;
	tree->noise_cutoff=0.1;
	tree->diffusion=0.1;
	tree->noise_amplitude=1;
	tree->shape_exterior=0.0001;
	tree->shape_interior=0;

	galaxy->setNoiseFunction((char*)"noise(NLOD,2,12,0.65,0.2,2,10)\n");
	galaxy->applyNoiseFunction();
	invalidate_object();
	getObjAttributes();
}

void VtxGalaxyTabs::OnSpiral(wxCommandEvent& event)
{
	Galaxy *galaxy=(Galaxy*)object();
	StarTree *tree=(StarTree*)galaxy->tree;

	tree->dispersion=2;
	tree->diffusion=0.05;
	tree->compression=0.9;
	tree->noise_cutoff=0.07;
	tree->noise_amplitude=1;
	tree->noise_vertical=0.05;
	tree->twist_radius=1.5;
	tree->twist_angle=1.7;
	tree->center_radius=0.3;
	tree->outer_radius=0.8;
	tree->inner_radius=0.8;
	tree->gradient=0.9;
	tree->density_value=1;
	tree->shape_exterior=0.0001;
	tree->shape_interior=0;

	galaxy->setDefaultNoise();
	galaxy->applyNoiseFunction();

	invalidate_object();
	getObjAttributes();
}
void VtxGalaxyTabs::OnBarred(wxCommandEvent& event)
{
	Galaxy *galaxy=(Galaxy*)object();
	StarTree *tree=(StarTree*)galaxy->tree;

	tree->dispersion=2;
	tree->diffusion=0.01;
	tree->compression=0.9;
	tree->noise_cutoff=0.0;
	tree->noise_amplitude=1;
	tree->noise_vertical=0.05;
	tree->twist_radius=1.5;
	tree->twist_angle=0.7;
	tree->center_radius=0.3;
	tree->outer_radius=0.8;
	tree->inner_radius=0.8;
	tree->gradient=0.9;
	tree->density_value=1;
	tree->shape_exterior=0.0001;
	tree->shape_interior=0;

	galaxy->setDefaultNoise();
	galaxy->applyNoiseFunction();

	invalidate_object();
	getObjAttributes();
}

void VtxGalaxyTabs::OnGlobular(wxCommandEvent& event)
{
	Galaxy *galaxy=(Galaxy*)object();
	StarTree *tree=(StarTree*)galaxy->tree;

	tree->center_bias=0;

	tree->outer_radius=0.6;
	tree->inner_radius=0.6;
	tree->center_radius=0.0;
	tree->gradient=1;
	tree->compression=0.5;
	tree->noise_cutoff=0.1;
	tree->density_value=0.5;
	tree->diffusion=3;
	tree->dispersion=20;
	tree->noise_amplitude=0;
	tree->noise_vertical=0;
	tree->shape_exterior=0.0001;
	tree->shape_interior=0;

	tree->twist_angle=0.0;
	tree->twist_radius=1;

	galaxy->setNoiseFunction((char*)"noise(NLOD,14,5,0.5)\n");
	galaxy->applyNoiseFunction();

	invalidate_object();
	getObjAttributes();
}
void VtxGalaxyTabs::OnElyptical(wxCommandEvent& event)
{
	Galaxy *galaxy=(Galaxy*)object();
	StarTree *tree=(StarTree*)galaxy->tree;

	tree->center_bias=0;
	tree->outer_radius=0.9;
	tree->inner_radius=0.6;
	tree->center_radius=0.8;
	tree->gradient=2;
	tree->compression=0.4;
	tree->noise_cutoff=0.1;
	tree->density_value=0.5;
	tree->diffusion=3;
	tree->dispersion=10;
	tree->noise_amplitude=0.1;
	tree->noise_vertical=1;
	tree->shape_exterior=0.0001;
	tree->shape_interior=0;

	galaxy->setNoiseFunction((char*)"noise(NLOD,14,5,0.5)\n");
	galaxy->applyNoiseFunction();
	invalidate_object();
	getObjAttributes();
}
