#include "VtxErodeTabs.h"
#include "VtxFunctDialog.h"
#include "Erode.h"

//########################### VtxErodeTabs Class ########################

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30
#define VALUE1 60

enum {
	ID_DELETE,
	ID_ENABLE,
	ID_SQR,
	ID_SS,
	ID_FILL_AMPL_SLDR,
	ID_FILL_AMPL_TEXT,
	ID_FILL_TRANS_SLDR,
	ID_FILL_TRANS_TEXT,
	ID_FILL_LEVEL_SLDR,
	ID_FILL_LEVEL_TEXT,
	ID_FILL_MARGIN_SLDR,
	ID_FILL_MARGIN_TEXT,
};

IMPLEMENT_CLASS(VtxErodeTabs, wxNotebook)

BEGIN_EVENT_TABLE(VtxErodeTabs, wxNotebook)
EVT_CHECKBOX(ID_SQR, VtxErodeTabs::OnChangeEvent)
EVT_CHECKBOX(ID_SS,  VtxErodeTabs::OnChangeEvent)
SET_SLIDER_EVENTS(FILL_AMPL,   VtxErodeTabs, FillAmpl)
SET_SLIDER_EVENTS(FILL_TRANS,  VtxErodeTabs, FillTransport)
SET_SLIDER_EVENTS(FILL_LEVEL,  VtxErodeTabs, FillLevel)
SET_SLIDER_EVENTS(FILL_MARGIN, VtxErodeTabs, FillMargin)
EVT_MENU(ID_DELETE,   VtxErodeTabs::OnDelete)
EVT_MENU(ID_ENABLE,   VtxErodeTabs::OnEnable)
EVT_UPDATE_UI(ID_ENABLE, VtxErodeTabs::OnUpdateEnable)
END_EVENT_TABLE()

VtxErodeTabs::VtxErodeTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxString& name)
		: VtxTabsMgr(parent, id, pos, size, style, name)
{
	Create(parent, id, pos, size, style, name);
}

bool VtxErodeTabs::Create(wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
{
	if (!VtxTabsMgr::Create(parent, id, pos, size, style, name))
		return false;

	wxNotebookPage *page = new wxPanel(this, wxID_ANY);
	AddControlsTab(page);
	AddPage(page, wxT("Erode"), true);
	Show(false);
	return true;
}

int VtxErodeTabs::showMenu(bool expanded)
{
	menu_action = TABS_NONE;
	wxMenu menu;
	menu.AppendCheckItem(ID_ENABLE, wxT("Enable"));
	menu.AppendSeparator();
	menu.Append(ID_DELETE, wxT("Delete"));
	PopupMenu(&menu);
	return menu_action;
}

void VtxErodeTabs::updateControls()
{
	if (update_needed)
		getObjAttributes();
}

void VtxErodeTabs::AddControlsTab(wxWindow *panel)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	// ── Amplitude ────────────────────────────────────────────
	// Depth: how deep valleys are carved (Z-units)
	// Power: cliff profile shape (1=linear, 2=concave, 0.5=convex)
	wxBoxSizer* dep_box = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Amplitude"));
	wxBoxSizer* dep_row = new wxBoxSizer(wxHORIZONTAL);

	FillAmplSlider = new ExprSliderCtrl(panel, ID_FILL_AMPL_SLDR, "Depth", LABEL2, VALUE1, SLIDER2);
	FillAmplSlider->setRange(0.0, 2.0);
	FillAmplSlider->setValue(0.3);
	dep_row->Add(FillAmplSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);

	FillTransportSlider = new ExprSliderCtrl(panel, ID_FILL_TRANS_SLDR, "Power", LABEL2, VALUE1, SLIDER2);
	FillTransportSlider->setRange(0.2, 4.0);
	FillTransportSlider->setValue(1.0);
	dep_row->Add(FillTransportSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);

	dep_box->Add(dep_row, 0, wxALIGN_LEFT|wxALL, 0);
	boxSizer->Add(dep_box, 0, wxALIGN_LEFT|wxALL, 0);

	// ── Height ────────────────────────────────────────────────
	// Start: Z threshold — below = full erosion, above = mesa top
	// Range: width of cliff zone above Start (small=sharp, large=gentle)
	wxBoxSizer* ht_box = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Height"));
	wxBoxSizer* ht_row = new wxBoxSizer(wxHORIZONTAL);

	FillLevelSlider = new ExprSliderCtrl(panel, ID_FILL_LEVEL_SLDR, "Start", LABEL2, VALUE1, SLIDER2);
	FillLevelSlider->setRange(-1.0, 1.0);
	FillLevelSlider->setValue(0.5);
	ht_row->Add(FillLevelSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);

	FillMarginSlider = new ExprSliderCtrl(panel, ID_FILL_MARGIN_SLDR, "Range", LABEL2, VALUE1, SLIDER2);
	FillMarginSlider->setRange(0.01, 1.0);
	FillMarginSlider->setValue(0.2);
	ht_row->Add(FillMarginSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);

	ht_box->Add(ht_row, 0, wxALIGN_LEFT|wxALL, 0);
	boxSizer->Add(ht_box, 0, wxALIGN_LEFT|wxALL, 0);

	// ── Options ───────────────────────────────────────────────
	wxBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("Options"));

	m_sqr = new wxCheckBox(panel, ID_SQR, "Square");
	options->Add(m_sqr, 0, wxALIGN_LEFT|wxALL, 2);

	m_ss = new wxCheckBox(panel, ID_SS, "SmoothStep");
	options->Add(m_ss, 0, wxALIGN_LEFT|wxALL, 2);

	options->SetMinSize(wxSize(BOX_WIDTH, LINE_HEIGHT + TABS_BORDER));
	boxSizer->Add(options, 0, wxALIGN_LEFT|wxALL, 0);
}

void VtxErodeTabs::getObjAttributes()
{
	if (!update_needed)
		return;

	TNerode *tc = object();
	int type = tc->options;
	m_sqr->SetValue((type & SQR) ? true : false);
	m_ss->SetValue((type & SS)  ? true : false);

	TNarg &args = *((TNarg *)tc->left);

	// args: [0]depth [1]start [2]range [3]power
	int idx = 0;
	TNode *a = args[idx++];
	if (a) FillAmplSlider->setValue(a);
	else   FillAmplSlider->setValue(0.3);

	a = args[idx++];
	if (a) FillLevelSlider->setValue(a);
	else   FillLevelSlider->setValue(0.5);

	a = args[idx++];
	if (a) FillMarginSlider->setValue(a);
	else   FillMarginSlider->setValue(0.2);

	a = args[idx++];
	if (a) FillTransportSlider->setValue(a);
	else   FillTransportSlider->setValue(1.0);

	update_needed = false;
}

void VtxErodeTabs::setObjAttributes()
{
	TNerode *tnode = object();

	wxString s = "erode(";
	wxString n = "";

	if (m_sqr->GetValue()) n += "SQR";
	if (m_ss->GetValue())  n += n.IsEmpty() ? "SS" : "|SS";
	if (!n.IsEmpty()) s += n + ",";

	s += FillAmplSlider->getText()      + ","; // depth  (arg 0)
	s += FillLevelSlider->getText()     + ","; // start  (arg 1)
	s += FillMarginSlider->getText()    + ","; // range  (arg 2)
	s += FillTransportSlider->getText();       // power  (arg 3)
	s += ")";

	tnode->setExpr((char*)s.ToAscii());
	update_needed = false;
	tnode->applyExpr();
	TheView->set_changed_detail();
	TheScene->rebuild_all();
}
