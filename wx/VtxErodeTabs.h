#ifndef VTX_ERODE_TABS_H_
#define VTX_ERODE_TABS_H_

#include "VtxTabsMgr.h"
#include "VtxControls.h"
#include "Erode.h"

enum {
    ID_ETABS_DELETE, ID_ETABS_ENABLE,
    ID_ETABS_AMPL_SLDR,   ID_ETABS_AMPL_TEXT,
    ID_ETABS_TRANS_SLDR,  ID_ETABS_TRANS_TEXT,
    ID_ETABS_LEVEL_SLDR,  ID_ETABS_LEVEL_TEXT,
    ID_ETABS_MARGIN_SLDR, ID_ETABS_MARGIN_TEXT,
    ID_ETABS_CHAN_SLDR,   ID_ETABS_CHAN_TEXT,
    ID_ETABS_ORD_SLDR,    ID_ETABS_ORD_TEXT,
    ID_ETABS_ATT_SLDR,    ID_ETABS_ATT_TEXT,
    ID_ETABS_DELF_SLDR,   ID_ETABS_DELF_TEXT,
    ID_ETABS_DRAMPL_SLDR, ID_ETABS_DRAMPL_TEXT,
    ID_ETABS_DRMIX_SLDR,  ID_ETABS_DRMIX_TEXT,
    ID_ETABS_INV,
    ID_ETABS_SQR,
    ID_ETABS_SS,
};

class VtxErodeTabs : public VtxTabsMgr
{
    DECLARE_CLASS(VtxErodeTabs)

    void AddControlsTab(wxWindow *panel);

    // Options checkboxes
    wxCheckBox *m_inv;
    wxCheckBox *m_sqr;
    wxCheckBox *m_ss;

public:
    // Sliders — named to match DEFINE_SLIDER_EVENTS macros
    ExprSliderCtrl *FillAmplSlider;
    ExprSliderCtrl *FillTransportSlider;
    ExprSliderCtrl *FillLevelSlider;
    ExprSliderCtrl *FillMarginSlider;
    ExprSliderCtrl *FillChannelsSlider;
    ExprSliderCtrl *FillOrdersSlider;
    ExprSliderCtrl *FillFalloffSlider;
    ExprSliderCtrl *FillFreqSlider;
    ExprSliderCtrl *FillDrAmplSlider;
    ExprSliderCtrl *FillDrainMixSlider;

    VtxErodeTabs(wxWindow* parent, wxWindowID id,
        const wxPoint& pos  = wxDefaultPosition,
        const wxSize&  size = wxDefaultSize,
        long style = 0,
        const wxString& name = wxNotebookNameStr);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxPoint& pos  = wxDefaultPosition,
        const wxSize&  size = wxDefaultSize,
        long style = 0,
        const wxString& name = wxNotebookNameStr);

    TNerode *object() { return (TNerode*)object_node->node; }

    int  showMenu(bool expanded) override;
    void updateControls() override;
    void getObjAttributes() override;
    void setObjAttributes() override;

    void OnChangeEvent(wxCommandEvent&) { setObjAttributes(); }

    DEFINE_SLIDER_EVENTS(FillAmpl)
    DEFINE_SLIDER_EVENTS(FillTransport)
    DEFINE_SLIDER_EVENTS(FillLevel)
    DEFINE_SLIDER_EVENTS(FillMargin)
    DEFINE_SLIDER_EVENTS(FillChannels)
    DEFINE_SLIDER_EVENTS(FillOrders)
    DEFINE_SLIDER_EVENTS(FillFalloff)
    DEFINE_SLIDER_EVENTS(FillFreq)
    DEFINE_SLIDER_EVENTS(FillDrAmpl)
    DEFINE_SLIDER_EVENTS(FillDrainMix)

    DECLARE_EVENT_TABLE()
};

#endif
