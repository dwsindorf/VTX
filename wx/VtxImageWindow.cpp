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
}

void VtxImageWindow::setImage(wxString name){
	m_name=name;
	Refresh();
}

void VtxImageWindow::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

    wxSize sz = GetSize();

	Image *img=images.load(m_name.ToAscii(),BMP|JPG);
	if(img !=0){
		int h=img->height;
		int w=img->width;
		unsigned char*pixels = (unsigned char*)img->data;

		if(img->gltype()==GL_RGBA){
		    //unsigned char*alpha = (unsigned char*)malloc( h*w*1 );
		    unsigned char*rgb=(unsigned char*)malloc( h*w*3 );
		    for(int i=0;i<h;i++){
		    	for(int j=0;j<w;j++){
		    		rgb[i*w*3+j*3]=pixels[i*w*4+j*4];
		    		rgb[i*w*3+j*3+1]=pixels[i*w*4+j*4+1];
		    		rgb[i*w*3+j*3+2]=pixels[i*w*4+j*4+2];
		    		//alpha[i*w+j]=pixels[i*w*4+j*4+3];
		    	}
		    }
		    wxImage wim(w,h,rgb,false);
		    //wim.SetAlpha(alpha,false);
			wxBitmap bmp(wim,-1);
#if wxCHECK_VERSION(3, 0, 0)
			dc.DrawBitmap(bmp, 0, 0, false);
#else
			wxEffects effects;
			effects.TileBitmap(wxRect(0, 0, sz.x, sz.y), dc, bmp);
#endif
		}
		else{
			wxImage wim(w,h,pixels,true);
			wxBitmap bmp(wim,-1);
#if wxCHECK_VERSION(3, 0, 0)
			dc.DrawBitmap(bmp, 0, 0, false);
#else
			wxEffects effects;
			effects.TileBitmap(wxRect(0, 0, sz.x, sz.y), dc, bmp);
#endif
		}
	}
 }
