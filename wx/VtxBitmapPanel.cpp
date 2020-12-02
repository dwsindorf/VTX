/*
 * VtxBitmapPanel.cpp
 *
 *  Created on: Nov 18, 2020
 *      Author: dean
 */

#include "VtxBitmapPanel.h"
#include <wx/bitmap.h>
#include "wx/effects.h"
#include "ImageMgr.h"

//########################### ImageWindow Class ########################

IMPLEMENT_CLASS(VtxBitmapPanel, wxPanel )

BEGIN_EVENT_TABLE(VtxBitmapPanel, wxPanel)
EVT_PAINT(VtxBitmapPanel::OnPaint)
END_EVENT_TABLE()

VtxBitmapPanel::VtxBitmapPanel(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: wxPanel(parent, id, pos, size,  style, name)
{
	m_name="";
}

void VtxBitmapPanel::setImage(wxString name){
	m_name=name;
	m_bitmap.LoadFile(m_name, wxBITMAP_TYPE_BMP);
	Refresh();
}
void VtxBitmapPanel::setScaledImage(wxString name){
	m_name=name;
	wxSize sz = GetSize();
	m_bitmap.LoadFile(m_name, wxBITMAP_TYPE_BMP);
	wxImage img=m_bitmap.ConvertToImage().Rescale(sz.x, sz.y);
	m_bitmap=wxBitmap(img);
	Refresh();
}

bool VtxBitmapPanel::scaleImage(){
	if(!m_bitmap.IsOk())
		return false;
	wxSize sz = GetSize();
	wxImage img=m_bitmap.ConvertToImage().Rescale(sz.x, sz.y);
	m_bitmap=wxBitmap(img);
	Refresh();
	return true;
}
bool VtxBitmapPanel::setSubImage(const wxRect& rect){
	if(m_name!=""){
		m_bitmap.LoadFile(m_name, wxBITMAP_TYPE_BMP);
		if(m_bitmap.IsOk())
			return getSubImage(rect, m_bitmap);
	}
	return false;
}

void VtxBitmapPanel::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    render(dc);
}

void VtxBitmapPanel::render(wxDC&  dc)
{
	if(m_bitmap.IsOk())
		dc.DrawBitmap( m_bitmap, 0, 0, true );
}

bool VtxBitmapPanel::getSubImage(const wxRect& rect, const wxBitmap& bitmap){
	if(!imageOk())
		return false;
	wxSize sz = m_bitmap.GetSize();
    int w = m_bitmap.GetWidth();
    int h = m_bitmap.GetHeight();
    int sw=w/rect.width;
    int sh=h/rect.height;
    int x=rect.x*sw;
    int y=rect.y*sh;
    wxRect srect(x,y,sw,sh);

    bitmap=m_bitmap.GetSubBitmap(srect);
	Refresh();

	return true;
}
bool VtxBitmapPanel::tileBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap)
{
    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();

    wxMemoryDC dcMem;

    dcMem.SelectObjectAsSource(bitmap);

    int i, j;
    for (i = rect.x; i < rect.x + rect.width; i += w)
    {
        for (j = rect.y; j < rect.y + rect.height; j+= h)
            dc.Blit(i, j, bitmap.GetWidth(), bitmap.GetHeight(), & dcMem, 0, 0);
    }
    dcMem.SelectObject(wxNullBitmap);
    return true;
}

