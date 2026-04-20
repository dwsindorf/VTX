#ifndef ERODE_FUNCT_H_
#define ERODE_FUNCT_H_

#include "VtxFunctMgr.h"
#include "VtxControls.h"
#include "Erode.h"

enum {
    ID_EFUNCT_AMPL_SLDR,   ID_EFUNCT_AMPL_TEXT,
    ID_EFUNCT_TRANS_SLDR,  ID_EFUNCT_TRANS_TEXT,
    ID_EFUNCT_LEVEL_SLDR,  ID_EFUNCT_LEVEL_TEXT,
    ID_EFUNCT_MARGIN_SLDR, ID_EFUNCT_MARGIN_TEXT,
    ID_EFUNCT_CHAN_SLDR,   ID_EFUNCT_CHAN_TEXT,
    ID_EFUNCT_ORD_SLDR,    ID_EFUNCT_ORD_TEXT,
    ID_EFUNCT_ATT_SLDR,    ID_EFUNCT_ATT_TEXT,
    ID_EFUNCT_DELF_SLDR,   ID_EFUNCT_DELF_TEXT,
    ID_EFUNCT_DRAMPL_SLDR, ID_EFUNCT_DRAMPL_TEXT,
    ID_EFUNCT_DRMIX_SLDR,  ID_EFUNCT_DRMIX_TEXT,
    ID_EFUNCT_INV,
    ID_EFUNCT_SQR,
    ID_EFUNCT_SS,
};

class VtxErodeFunct : public VtxFunctMgr
{
    DECLARE_CLASS(VtxErodeFunct)

    void AddControlsTab(wxWindow *panel);

    wxCheckBox *m_inv;
    wxCheckBox *m_sqr;
    wxCheckBox *m_ss;

public:
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

    VtxErodeFunct(wxWindow* parent, wxWindowID id,
        const wxPoint& pos  = wxDefaultPosition,
        const wxSize&  size = wxDefaultSize,
        long style = 0,
        const wxString& name = wxNotebookNameStr);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxPoint& pos  = wxDefaultPosition,
        const wxSize&  size = wxDefaultSize,
        long style = 0,
        const wxString& name = wxNotebookNameStr);

    void setFunction(wxString f) override;
    void getFunction() override;

    void OnChangeEvent(wxCommandEvent&) { getFunction(); }

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
