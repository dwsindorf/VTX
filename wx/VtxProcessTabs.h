#ifndef VTX_PROCESS_TABS_H_
#define VTX_PROCESS_TABS_H_

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/dir.h>
#include <vector>

#include "VtxImageWindow.h"
#include "VtxControls.h"
#include "FileUtil.h"

enum ProcOp {
    PROC_NONE = 0,
    PROC_BLUR,
    PROC_SHARPEN,
    PROC_DILATE,
    PROC_ERODE_IMG,
    PROC_NORMALIZE,
    PROC_CONTRAST,
    PROC_BRIGHTNESS,
    PROC_HYDRAULIC,
    PROC_DENDRITIC,
    PROC_OP_COUNT
};

class VtxProcessTabs : public wxPanel
{
    DECLARE_CLASS(VtxProcessTabs)

    wxChoice        *m_file_menu;
    wxChoice        *m_op_menu;
    SliderCtrl      *m_radius_slider;
    SliderCtrl      *m_strength_slider;
    SliderCtrl      *m_iters_slider;
    wxCheckBox      *m_gray_check;
    wxCheckBox      *m_tile_check;
    VtxImageWindow  *m_image_window;

    std::vector<float> m_buf;
    std::vector<float> m_orig;
    std::vector<float> m_prev_buf;  // snapshot before last runOperation(), for single-step Revert
    int m_last_op;                  // op selection to restore when re-entering the tab
    int m_w, m_h;
    bool m_has_image;
    double m_str;
    double m_rad;
    double m_iters;
    bool   m_gray;
    bool m_modified;
    wxString m_name;

    void buildUI(wxPanel *panel);
    void makeFileList();
    bool loadImage(const wxString &path);
    bool saveImage(const wxString &path);
    void displayBuffer();
    void ensureProcessedDir();

    void opDilate();
    void opErodeImg();
    void opBlur();
    void opSharpen();
    void opNormalize();
    void opContrast();
    void opBrightness();
    void opHydraulic();
    void opDendritic();
    void toGrayscale(); // collapse m_buf to R=G=B=luminance

public:
    VtxProcessTabs(wxWindow *parent, wxWindowID id,
        const wxPoint &pos  = wxDefaultPosition,
        const wxSize  &size = wxDefaultSize,
        long style = 0,
        const wxString &name = wxPanelNameStr);

    void loadFromPath(const wxString &path, const wxString &name);
    void runOperation();

    void Save();
    void Rename(const wxString &new_name) { m_name = new_name; }
    void Revert();
    void Delete();
    bool canSave()   const { return m_has_image; }
    bool canRevert() const { return m_has_image && !m_prev_buf.empty(); }

    wxString getSelection() { return m_file_menu->GetStringSelection(); }
    void updateControls();

    void OnFileSelect(wxCommandEvent &event);
    void OnOpSelect(wxCommandEvent &event);
    void OnRadiusSlider(wxScrollEvent &event);
    void OnStrengthSlider(wxScrollEvent &event);
    void OnItersSlider(wxScrollEvent &event);
    void OnRadiusText(wxCommandEvent &event);
    void OnStrengthText(wxCommandEvent &event);
    void OnItersText(wxCommandEvent &event);
    void OnGrayCheck(wxCommandEvent &event);
    void OnTileCheck(wxCommandEvent &event);

    DECLARE_EVENT_TABLE()
};

#endif // VTX_PROCESS_TABS_H_
