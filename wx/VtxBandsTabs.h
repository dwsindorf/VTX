#ifndef VTXBANDSTABS_H_
#define VTXBANDSTABS_H_
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/arrstr.h>
#include <ImageClass.h>
#include <TextureClass.h>
#include <defs.h>
#include "VtxImageWindow.h"

#define MAX_PAGE_COLORS 6
#define MAX_COLOR_PAGES 2
#define MAX_COLORS (MAX_PAGE_COLORS*MAX_COLOR_PAGES)

class VtxBandsTabs : public wxPanel

{

	DECLARE_CLASS(VtxBandsTabs)
protected:
	void AddBandsTab(wxPanel *panel);
	void splineString(char *buff);
	void makeImageList();
	void freeImageList();
    void setModified(bool);
    bool isModified();
	void makeRevertList();
	void freeRevertList();
	void displayImage(char *);
	void getObjAttributes();
	void setObjAttributes();
	void makeNewImage(char *, char *);
	void setControlsFromColors();
	void setColorsFromControls();
	bool alpha();
	bool Clone(wxString name,bool rename);

	ColorSlider *m_mix_color;
	SliderCtrl *m_mod_slider;

	wxChoice *m_file_menu;
	ColorSlider *csliders[MAX_COLORS];
	wxCheckBox  *enables[MAX_COLORS];

	wxStaticBoxSizer* spline_colors;
	wxBoxSizer* color_pages[MAX_COLOR_PAGES];
	wxString m_name;
	VtxImageWindow *m_image_window;

	wxCheckBox *m_invert_check;
	wxCheckBox *m_norm_check;
	wxCheckBox *m_clamp_check;
	wxCheckBox *m_nearest_check;
	wxCheckBox *m_alpha_check;
	wxCheckBox *m_reflect_check;

	wxButton *m_next_colors;
	wxButton *m_prev_colors;
	wxChoice *m_image_size;

	NameList<ImageSym*> *image_list;
	NameList<ImageSym*> *revert_list;

	LinkedList<Color*>colors;
	Color cmix;
	bool update_needed;
	int type;
	int color_page;
	int num_colors;
	int num_sliders;
public:
	VtxBandsTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);
	~VtxBandsTabs()
	{
		freeImageList();
		freeRevertList();
		delete m_mix_color;
		delete m_mod_slider;
		for(int i=0;i<num_sliders;i++)
			delete csliders[i];
		colors.free();
	}

	void updateControls();
    void Save();
	bool New(wxString name);
    void Revert();
	void Build();
    void Delete();
	bool canDelete();
	bool canRevert();
	bool canSave();
    void Invalidate();
    bool Rename(wxString name);


    void setSelection(wxString name);
    wxString getSelection(){return m_file_menu->GetStringSelection();}

    void OnNextColors(wxCommandEvent& event);
    void OnPrevColors(wxCommandEvent& event);
    void OnChanged(wxCommandEvent& event);
    void OnFileSelect(wxCommandEvent& event);
    void OnImageSize(wxCommandEvent& event);

    void OnMixSlider(wxScrollEvent& event);
    void OnEndMixSlider(wxScrollEvent& event);
    void OnMixText(wxCommandEvent& event);
    void OnMixColor(wxColourPickerEvent& event);

    void OnModSlider(wxScrollEvent& event);
    void OnEndModSlider(wxScrollEvent& event);
    void OnModText(wxCommandEvent& event);

    void OnSetColors(wxCommandEvent& event);

    wxString getImageString(wxString name);
    void OnSplineColorSlider(wxScrollEvent& event);
    void OnEndSplineColorSlider(wxScrollEvent& event);
    void OnSplineColorText(wxCommandEvent& event);
    void OnSplineColorValue(wxColourPickerEvent&event);
    void OnSplineColorEnable(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};
#endif /*VTXBANDSTABS_H_*/
