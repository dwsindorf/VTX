#ifndef WX_VTXBITMAPPANEL_H_
#define WX_VTXBITMAPPANEL_H_

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class VtxBitmapPanel : public wxPanel

{
	DECLARE_CLASS(VtxImageWindow)

protected:
	wxString m_name;
	wxBitmap m_bitmap;
public:
	VtxBitmapPanel(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = "");

	void setImage(wxString);
	void setScaledImage(wxString);
	bool scaleImage();

	void OnPaint(wxPaintEvent& event);
	void render(wxDC& dc);
	bool tileBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap);
	bool getSubImage(const wxRect& rect, const wxBitmap& bitmap);
	bool setSubImage(const wxRect& rect);
	bool imageOk()  { return m_bitmap.IsOk();}
	char *getName() { return (char*)m_name.ToAscii();}
	void setName(char *name) { m_name=name;}
	DECLARE_EVENT_TABLE()
};

#endif /* WX_VTXBITMAPPANEL_H_ */
