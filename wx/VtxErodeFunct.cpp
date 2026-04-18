#include "VtxErodeFunct.h"
#include "VtxFunctDialog.h"
#include "Erode.h"

//########################### VtxErodeFunct Class ########################
// Image mode erode: generates a 2D procedural pattern (Voronoi or ridged
// noise) for use in image() expressions. No terrain height logic — just
// the drainage pattern parameters and amplitude controls.

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30
#define VALUE1 60

enum {
	ID_SQR, ID_NEG,
	ID_FILL_CHANNELS_SLDR, ID_FILL_CHANNELS_TEXT,
	ID_FILL_ORDERS_SLDR,   ID_FILL_ORDERS_TEXT,
	ID_FILL_FALLOFF_SLDR,  ID_FILL_FALLOFF_TEXT,
	ID_FILL_FREQ_SLDR,     ID_FILL_FREQ_TEXT,
	ID_FILL_DRAMPL_SLDR,   ID_FILL_DRAMPL_TEXT,
	ID_FILL_DRAINMIX_SLDR, ID_FILL_DRAINMIX_TEXT,
	ID_FILL_AMPL_SLDR,     ID_FILL_AMPL_TEXT,
};

IMPLEMENT_CLASS(VtxErodeFunct, wxNotebook)

BEGIN_EVENT_TABLE(VtxErodeFunct, wxNotebook)
EVT_CHECKBOX(ID_SQR, VtxErodeFunct::OnChangeEvent)
EVT_CHECKBOX(ID_NEG, VtxErodeFunct::OnChangeEvent)
SET_SLIDER_EVENTS(FILL_CHANNELS, VtxErodeFunct, FillChannels)
SET_SLIDER_EVENTS(FILL_ORDERS,   VtxErodeFunct, FillOrders)
SET_SLIDER_EVENTS(FILL_FALLOFF,  VtxErodeFunct, FillFalloff)
SET_SLIDER_EVENTS(FILL_FREQ,     VtxErodeFunct, FillFreq)
SET_SLIDER_EVENTS(FILL_DRAMPL,   VtxErodeFunct, FillDrAmpl)
SET_SLIDER_EVENTS(FILL_DRAINMIX, VtxErodeFunct, FillDrainMix)
SET_SLIDER_EVENTS(FILL_AMPL,     VtxErodeFunct, FillAmpl)
END_EVENT_TABLE()

VtxErodeFunct::VtxErodeFunct(wxWindow* parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: VtxFunctMgr(parent, id, pos, size, style, name)
{ Create(parent, id, pos, size, style, name); }

bool VtxErodeFunct::Create(wxWindow* parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
	if (!VtxFunctMgr::Create(parent, id, pos, size, style, name)) return false;
	wxNotebookPage *page = new wxPanel(this, wxID_ANY);
	AddControlsTab(page);
	AddPage(page, wxT("Erode"), true);
	Show(false);
	return true;
}

void VtxErodeFunct::AddControlsTab(wxWindow *panel)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(topSizer);
	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	// ── Overlays ───────────────────────────────────────────────
	wxBoxSizer* overlay_box = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Overlays"));

	wxBoxSizer* ov1 = new wxBoxSizer(wxHORIZONTAL);
	FillChannelsSlider = new ExprSliderCtrl(panel, ID_FILL_CHANNELS_SLDR, "Start", LABEL2, VALUE1, SLIDER2);
	FillChannelsSlider->setRange(0.0, 20.0); FillChannelsSlider->setValue(6.0);
	ov1->Add(FillChannelsSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	FillOrdersSlider = new ExprSliderCtrl(panel, ID_FILL_ORDERS_SLDR, "Orders", LABEL2, VALUE1, SLIDER2);
	FillOrdersSlider->setRange(1.0, 8.0); FillOrdersSlider->setValue(5.0);
	ov1->Add(FillOrdersSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	overlay_box->Add(ov1, 0, wxALIGN_LEFT|wxALL, 0);

	wxBoxSizer* ov2 = new wxBoxSizer(wxHORIZONTAL);
	FillFalloffSlider = new ExprSliderCtrl(panel, ID_FILL_FALLOFF_SLDR, "Atten", LABEL2, VALUE1, SLIDER2);
	FillFalloffSlider->setRange(0.0, 1.0); FillFalloffSlider->setValue(0.5);
	ov2->Add(FillFalloffSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	FillFreqSlider = new ExprSliderCtrl(panel, ID_FILL_FREQ_SLDR, "Delf", LABEL2, VALUE1, SLIDER2);
	FillFreqSlider->setRange(1.1, 4.0); FillFreqSlider->setValue(2.0);
	ov2->Add(FillFreqSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	overlay_box->Add(ov2, 0, wxALIGN_LEFT|wxALL, 0);
	boxSizer->Add(overlay_box, 0, wxALIGN_LEFT|wxALL, 0);

	// ── Amplitude ─────────────────────────────────────────────
	// Drainage: weight of Voronoi/ridged pattern
	// Erosion:  weight of inverse (background level)
	// Scale:    overall output amplitude
	wxBoxSizer* ampl_box = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Amplitude"));

	wxBoxSizer* ampl_row1 = new wxBoxSizer(wxHORIZONTAL);
	FillDrAmplSlider = new ExprSliderCtrl(panel, ID_FILL_DRAMPL_SLDR, "Drainage", LABEL2, VALUE1, SLIDER2);
	FillDrAmplSlider->setRange(0.0, 2.0); FillDrAmplSlider->setValue(1.0);
	ampl_row1->Add(FillDrAmplSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	FillDrainMixSlider = new ExprSliderCtrl(panel, ID_FILL_DRAINMIX_SLDR, "Bias", LABEL2, VALUE1, SLIDER2);
	FillDrainMixSlider->setRange(0.0, 2.0); FillDrainMixSlider->setValue(0.0);
	ampl_row1->Add(FillDrainMixSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	ampl_box->Add(ampl_row1, 0, wxALIGN_LEFT|wxALL, 0);

	wxBoxSizer* ampl_row2 = new wxBoxSizer(wxHORIZONTAL);
	FillAmplSlider = new ExprSliderCtrl(panel, ID_FILL_AMPL_SLDR, "Scale", LABEL2, VALUE1, SLIDER2);
	FillAmplSlider->setRange(0.0, 2.0); FillAmplSlider->setValue(1.0);
	ampl_row2->Add(FillAmplSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	ampl_box->Add(ampl_row2, 0, wxALIGN_LEFT|wxALL, 0);
	boxSizer->Add(ampl_box, 0, wxALIGN_LEFT|wxALL, 0);

	// ── Options ───────────────────────────────────────────────
	wxBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("Options"));
	m_neg = new wxCheckBox(panel, ID_NEG, "Rounded");
	options->Add(m_neg, 0, wxALIGN_LEFT|wxALL, 2);
	m_sqr = new wxCheckBox(panel, ID_SQR, "Square");
	options->Add(m_sqr, 0, wxALIGN_LEFT|wxALL, 2);
	options->SetMinSize(wxSize(BOX_WIDTH, LINE_HEIGHT + TABS_BORDER));
	boxSizer->Add(options, 0, wxALIGN_LEFT|wxALL, 0);
}

void VtxErodeFunct::setFunction(wxString f)
{
	// Image erode args: [0]scale [1]level(unused) [2]edge(unused) [3]shape(unused)
	//                   [4]channels [5]drain_ampl [6]erode_ampl [7]orders [8]delf [9]atten
	TNerode *tc = (TNerode*)TheScene->parse_node(f.ToAscii());
	if (!tc) return;
	int type = tc->options;
	m_neg->SetValue((type & NEG) ? true : false);
	m_sqr->SetValue((type & SQR) ? true : false);
	TNarg &args = *((TNarg*)tc->left);
	int idx = 0; TNode *a;
	auto get = [&](ExprSliderCtrl *s, double def) {
		a = args[idx++]; if (a) s->setValue(a); else s->setValue(def);
	};
	get(FillAmplSlider,      1.0);  // [0] scale
	idx += 3;                        // skip [1]level [2]edge [3]shape
	get(FillChannelsSlider,  6.0);  // [4] start
	get(FillDrAmplSlider,    1.0);  // [5] drainage ampl
	get(FillDrainMixSlider,  0.0);  // [6] erosion ampl
	get(FillOrdersSlider,    5.0);  // [7] orders
	get(FillFreqSlider,      2.0);  // [8] delf
	get(FillFalloffSlider,   0.5);  // [9] atten
	delete tc;
}

void VtxErodeFunct::getFunction()
{
	wxString s = "erode(";
	wxString n = "";
	if (m_neg->GetValue()) n += "NEG";
	if (m_sqr->GetValue()) n += n.IsEmpty() ? "SQR" : "|SQR";
	if (!n.IsEmpty()) s += n + ",";

	// Emit all 10 args so terrain erode and image erode share same arg layout.
	// Image-irrelevant args [1-3] are fixed at neutral values.
	s += FillAmplSlider->getText()     + ","; // [0] scale (depth)
	s += "0,";                                 // [1] level  (unused in image mode)
	s += "1,";                                 // [2] edge   (unused in image mode)
	s += "0,";                                 // [3] shape  (unused in image mode)
	s += FillChannelsSlider->getText() + ","; // [4] start
	s += FillDrAmplSlider->getText()   + ","; // [5] drainage ampl
	s += FillDrainMixSlider->getText() + ","; // [6] erosion ampl
	s += FillOrdersSlider->getText()   + ","; // [7] orders
	s += FillFreqSlider->getText()     + ","; // [8] delf
	s += FillFalloffSlider->getText();        // [9] atten
	s += ")";

	TNerode *tn = (TNerode*)TheScene->parse_node(s.ToAscii());
	if (!tn) return;
	char buff[256]; buff[0] = 0;
	tn->valueString(buff);
	s = wxString(buff);
	functDialog->getFunct(s);
}
