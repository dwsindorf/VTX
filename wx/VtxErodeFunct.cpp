#include "VtxErodeFunct.h"
#include "VtxFunctDialog.h"
#include "Erode.h"

//########################### VtxErodeFunct Class ########################

#define BOX_WIDTH TABS_WIDTH-TABS_BORDER
#define LINE_HEIGHT 30
#define VALUE1 60

enum {
	OBJ_DELETE, OBJ_SAVE,
	ID_SQR, ID_SS,
	ID_FILL_AMPL_SLDR,    ID_FILL_AMPL_TEXT,
	ID_FILL_TRANS_SLDR,   ID_FILL_TRANS_TEXT,
	ID_FILL_LEVEL_SLDR,   ID_FILL_LEVEL_TEXT,
	ID_FILL_MARGIN_SLDR,  ID_FILL_MARGIN_TEXT,
	ID_FILL_CHANNELS_SLDR,ID_FILL_CHANNELS_TEXT,
	ID_FILL_DRAMPL_SLDR,  ID_FILL_DRAMPL_TEXT,
	ID_FILL_DRAINMIX_SLDR,ID_FILL_DRAINMIX_TEXT,
	ID_FILL_ORDERS_SLDR,  ID_FILL_ORDERS_TEXT,
	ID_FILL_FREQ_SLDR,    ID_FILL_FREQ_TEXT,
	ID_FILL_FALLOFF_SLDR, ID_FILL_FALLOFF_TEXT,
};

IMPLEMENT_CLASS(VtxErodeFunct, wxNotebook)

BEGIN_EVENT_TABLE(VtxErodeFunct, wxNotebook)
EVT_CHECKBOX(ID_SQR, VtxErodeFunct::OnChangeEvent)
EVT_CHECKBOX(ID_SS,  VtxErodeFunct::OnChangeEvent)
SET_SLIDER_EVENTS(FILL_AMPL,    VtxErodeFunct, FillAmpl)
SET_SLIDER_EVENTS(FILL_TRANS,   VtxErodeFunct, FillTransport)
SET_SLIDER_EVENTS(FILL_LEVEL,   VtxErodeFunct, FillLevel)
SET_SLIDER_EVENTS(FILL_MARGIN,  VtxErodeFunct, FillMargin)
SET_SLIDER_EVENTS(FILL_CHANNELS,VtxErodeFunct, FillChannels)
SET_SLIDER_EVENTS(FILL_DRAMPL,  VtxErodeFunct, FillDrAmpl)
SET_SLIDER_EVENTS(FILL_DRAINMIX,VtxErodeFunct, FillDrainMix)
SET_SLIDER_EVENTS(FILL_ORDERS,  VtxErodeFunct, FillOrders)
SET_SLIDER_EVENTS(FILL_FREQ,    VtxErodeFunct, FillFreq)
SET_SLIDER_EVENTS(FILL_FALLOFF, VtxErodeFunct, FillFalloff)
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

	// ── Erosion ───────────────────────────────────────────────
	wxBoxSizer* erosion_box = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Erosion"));

	wxBoxSizer* dep_row = new wxBoxSizer(wxHORIZONTAL);
	FillAmplSlider = new ExprSliderCtrl(panel, ID_FILL_AMPL_SLDR, "Depth", LABEL2, VALUE1, SLIDER2);
	FillAmplSlider->setRange(0.0, 2.0); FillAmplSlider->setValue(0.3);
	dep_row->Add(FillAmplSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	FillTransportSlider = new ExprSliderCtrl(panel, ID_FILL_TRANS_SLDR, "Power", LABEL2, VALUE1, SLIDER2);
	FillTransportSlider->setRange(0.1, 4.0); FillTransportSlider->setValue(1.0);
	dep_row->Add(FillTransportSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	erosion_box->Add(dep_row, 0, wxALIGN_LEFT|wxALL, 0);

	wxBoxSizer* ht_row = new wxBoxSizer(wxHORIZONTAL);
	FillLevelSlider = new ExprSliderCtrl(panel, ID_FILL_LEVEL_SLDR, "Start", LABEL2, VALUE1, SLIDER2);
	FillLevelSlider->setRange(-2.0, 2.0); FillLevelSlider->setValue(0.0);
	ht_row->Add(FillLevelSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	FillMarginSlider = new ExprSliderCtrl(panel, ID_FILL_MARGIN_SLDR, "Range", LABEL2, VALUE1, SLIDER2);
	FillMarginSlider->setRange(0.01, 4.0); FillMarginSlider->setValue(0.4);
	ht_row->Add(FillMarginSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	erosion_box->Add(ht_row, 0, wxALIGN_LEFT|wxALL, 0);
	boxSizer->Add(erosion_box, 0, wxALIGN_LEFT|wxALL, 0);

	// ── Drainage ──────────────────────────────────────────────
	wxBoxSizer* drain_box = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Drainage"));

	wxBoxSizer* ov1 = new wxBoxSizer(wxHORIZONTAL);
	FillChannelsSlider = new ExprSliderCtrl(panel, ID_FILL_CHANNELS_SLDR, "Start", LABEL2, VALUE1, SLIDER2);
	FillChannelsSlider->setRange(0.0, 20.0); FillChannelsSlider->setValue(12.0);
	ov1->Add(FillChannelsSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	FillOrdersSlider = new ExprSliderCtrl(panel, ID_FILL_ORDERS_SLDR, "Orders", LABEL2, VALUE1, SLIDER2);
	FillOrdersSlider->setRange(1.0, 8.0); FillOrdersSlider->setValue(3.0);
	ov1->Add(FillOrdersSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	drain_box->Add(ov1, 0, wxALIGN_LEFT|wxALL, 0);

	wxBoxSizer* ov2 = new wxBoxSizer(wxHORIZONTAL);
	FillFalloffSlider = new ExprSliderCtrl(panel, ID_FILL_FALLOFF_SLDR, "Atten", LABEL2, VALUE1, SLIDER2);
	FillFalloffSlider->setRange(0.0, 1.0); FillFalloffSlider->setValue(0.5);
	ov2->Add(FillFalloffSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	FillFreqSlider = new ExprSliderCtrl(panel, ID_FILL_FREQ_SLDR, "Delf", LABEL2, VALUE1, SLIDER2);
	FillFreqSlider->setRange(1.1, 4.0); FillFreqSlider->setValue(2.0);
	ov2->Add(FillFreqSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	drain_box->Add(ov2, 0, wxALIGN_LEFT|wxALL, 0);
	boxSizer->Add(drain_box, 0, wxALIGN_LEFT|wxALL, 0);

	// ── Amplitude ─────────────────────────────────────────────
	wxBoxSizer* ampl_box = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Amplitude"));
	wxBoxSizer* ampl_row = new wxBoxSizer(wxHORIZONTAL);
	FillDrAmplSlider = new ExprSliderCtrl(panel, ID_FILL_DRAMPL_SLDR, "Drainage", LABEL2, VALUE1, SLIDER2);
	FillDrAmplSlider->setRange(0.0, 2.0); FillDrAmplSlider->setValue(0.5);
	ampl_row->Add(FillDrAmplSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	FillDrainMixSlider = new ExprSliderCtrl(panel, ID_FILL_DRAINMIX_SLDR, "Erosion", LABEL2, VALUE1, SLIDER2);
	FillDrainMixSlider->setRange(0.0, 2.0); FillDrainMixSlider->setValue(1.0);
	ampl_row->Add(FillDrainMixSlider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	ampl_box->Add(ampl_row, 0, wxALIGN_LEFT|wxALL, 0);
	boxSizer->Add(ampl_box, 0, wxALIGN_LEFT|wxALL, 0);

	// ── Options ───────────────────────────────────────────────
	wxBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("Options"));
	m_sqr = new wxCheckBox(panel, ID_SQR, "Square");
	options->Add(m_sqr, 0, wxALIGN_LEFT|wxALL, 2);
	m_ss = new wxCheckBox(panel, ID_SS, "SmoothStep");
	options->Add(m_ss, 0, wxALIGN_LEFT|wxALL, 2);
	options->SetMinSize(wxSize(BOX_WIDTH, LINE_HEIGHT + TABS_BORDER));
	boxSizer->Add(options, 0, wxALIGN_LEFT|wxALL, 0);
}

void VtxErodeFunct::setFunction(wxString f)
{
	TNerode *tc = (TNerode*)TheScene->parse_node(f.ToAscii());
	if (!tc) return;
	int type = tc->options;
	m_sqr->SetValue((type & SQR) ? true : false);
	m_ss->SetValue((type & SS)   ? true : false);
	TNarg &args = *((TNarg*)tc->left);
	int idx = 0; TNode *a;
	auto get = [&](ExprSliderCtrl *s, double def) {
		a = args[idx++]; if (a) s->setValue(a); else s->setValue(def);
	};
	get(FillAmplSlider,      0.3);
	get(FillLevelSlider,     0.0);
	get(FillMarginSlider,    0.4);
	get(FillTransportSlider, 1.0);
	get(FillChannelsSlider,  12.0);
	get(FillDrAmplSlider,    0.5);
	get(FillDrainMixSlider,  1.0);
	get(FillOrdersSlider,    3.0);
	get(FillFreqSlider,      2.0);
	get(FillFalloffSlider,   0.5);
	delete tc;
}

void VtxErodeFunct::getFunction()
{
	wxString s = "erode(";
	wxString n = "";
	if (m_sqr->GetValue()) n += "SQR";
	if (m_ss->GetValue())  n += n.IsEmpty() ? "SS" : "|SS";
	if (!n.IsEmpty()) s += n + ",";
	s += FillAmplSlider->getText()      + ","; // depth
	s += FillLevelSlider->getText()     + ","; // start
	s += FillMarginSlider->getText()    + ","; // range
	s += FillTransportSlider->getText() + ","; // power
	s += FillChannelsSlider->getText()  + ","; // channels
	s += FillDrAmplSlider->getText()    + ","; // drain ampl
	s += FillDrainMixSlider->getText()  + ","; // erode ampl
	s += FillOrdersSlider->getText()    + ","; // orders
	s += FillFreqSlider->getText()      + ","; // delf
	s += FillFalloffSlider->getText();         // atten
	s += ")";
	TNerode *tn = (TNerode*)TheScene->parse_node(s.ToAscii());
	if (!tn) return;
	char buff[256]; buff[0] = 0;
	tn->valueString(buff);
	s = wxString(buff);
	functDialog->getFunct(s);
}
