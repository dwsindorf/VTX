#ifndef IMAGEWINDOW_H_
#define IMAGEWINDOW_H_
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class VtxImageWindow : public wxPanel

{
	DECLARE_CLASS(VtxImageWindow)

protected:
	wxString m_name;
	int option;
public:
	enum {
		TILE,SCALE
	};
	VtxImageWindow(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = "");

	void setImage(wxString,int option );
	void OnPaint(wxPaintEvent& event);
	bool TileBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap);
	bool ScaleBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap);

	DECLARE_EVENT_TABLE()
};
#endif /*IMAGEWINDOW_H_*/
