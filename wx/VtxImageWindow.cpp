#include "VtxImageWindow.h"
#include <wx/bitmap.h>
#include "wx/effects.h"
#include "ImageMgr.h"


//########################### ImageWindow Class ########################

IMPLEMENT_CLASS(VtxImageWindow, wxPanel )

BEGIN_EVENT_TABLE(VtxImageWindow, wxPanel)
EVT_PAINT(VtxImageWindow::OnPaint)
END_EVENT_TABLE()

VtxImageWindow::VtxImageWindow(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: wxPanel(parent, id, pos, size,  style, name)
{
	m_name="";
	option=TILE;
}

void VtxImageWindow::setImage(wxString name,int opt){
	m_name=name;
	option=opt;
	Refresh();
}

void VtxImageWindow::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

    wxSize sz = GetSize();

	Image *img=images.load((char*)m_name.ToAscii(),BMP|JPG);
	if(img !=0){
		int h=img->height;
		int w=img->width;
		unsigned char*pixels = (unsigned char*)img->data;

		if(img->gltype()==GL_RGBA){
		    unsigned char*alpha = (unsigned char*)malloc( h*w*1 );
		    unsigned char*rgb=(unsigned char*)malloc( h*w*3 );
		    for(int i=0;i<h;i++){
		    	for(int j=0;j<w;j++){
		    		rgb[i*w*3+j*3]=pixels[i*w*4+j*4];
		    		rgb[i*w*3+j*3+1]=pixels[i*w*4+j*4+1];
		    		rgb[i*w*3+j*3+2]=pixels[i*w*4+j*4+2];
		    		alpha[i*w+j]=pixels[i*w*4+j*4+3];
		    	}
		    }
		    wxImage wim(w,h,rgb,false);
		    wim.SetAlpha(alpha,false);
			wxBitmap bmp(wim,-1);
			if(option==TILE)
			    TileBitmap(wxRect(0, 0, sz.x, sz.y), dc, bmp);
			else
				ScaleBitmap(wxRect(0, 0, sz.x, sz.y), dc, bmp);
		}
		else{
			wxImage wim(w,h,pixels,true);
			wxBitmap bmp(wim,-1);
			if(option==TILE)
			    TileBitmap(wxRect(0, 0, sz.x, sz.y), dc, bmp);
			else
				ScaleBitmap(wxRect(0, 0, sz.x, sz.y), dc, bmp);
		}
	}
 }

bool VtxImageWindow::TileBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap)
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

bool VtxImageWindow::ScaleBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap)
{
	wxSize sz = GetSize();
	wxImage img=bitmap.ConvertToImage().Rescale(sz.x, sz.y);
	bitmap=wxBitmap(img);
	dc.DrawBitmap( bitmap, 0, 0, true );
    return true;
}
