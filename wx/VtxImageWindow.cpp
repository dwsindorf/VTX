#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <malloc.h>
#include <cmath>
#include <algorithm>

#include "VtxImageWindow.h"
#include "ImageMgr.h"

IMPLEMENT_CLASS(VtxImageWindow, wxPanel)

BEGIN_EVENT_TABLE(VtxImageWindow, wxPanel)
    EVT_PAINT(VtxImageWindow::OnPaint)
END_EVENT_TABLE()

VtxImageWindow::VtxImageWindow(wxWindow* parent, wxWindowID id,
        const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxPanel(parent, id, pos, size, style, name), option(SCALE)
{
}

void VtxImageWindow::setImage(wxString name, int opt)
{
    m_name = name;
    option = opt;
    m_direct_bitmap = wxNullBitmap; // clear any direct data so cache path is used
    Refresh();
}

void VtxImageWindow::setImageData(const float *rgba, int w, int h, int mode)
{
    // Convert float RGBA [0..1] → packed RGB + alpha and store as wxBitmap
    unsigned char *rgb   = (unsigned char*)malloc(w * h * 3);
    unsigned char *alpha = (unsigned char*)malloc(w * h);
    auto cl = [](float v) -> unsigned char {
        return (unsigned char)(std::max(0.0f, std::min(1.0f, v)) * 255.0f + 0.5f);
    };
    for (int i = 0; i < w * h; i++) {
        rgb[i*3+0] = cl(rgba[i*4+0]);
        rgb[i*3+1] = cl(rgba[i*4+1]);
        rgb[i*3+2] = cl(rgba[i*4+2]);
        alpha[i]   = cl(rgba[i*4+3]);
    }
    wxImage img(w, h, rgb, false);  // takes ownership of rgb
    img.SetAlpha(alpha, false);     // takes ownership of alpha
    m_direct_bitmap = wxBitmap(img, -1);
    m_name = wxString();            // suppress cache lookup in OnPaint
    option = mode;
    Refresh();
}

void VtxImageWindow::clearImageData()
{
    m_direct_bitmap = wxNullBitmap;
    Refresh();
}

void VtxImageWindow::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    wxSize sz = GetSize();

    // Fast path: direct bitmap set by setImageData()
    if (m_direct_bitmap.IsOk()) {
        if (option == TILE)
            TileBitmap(wxRect(0, 0, sz.x, sz.y), dc, m_direct_bitmap);
        else
            ScaleBitmap(wxRect(0, 0, sz.x, sz.y), dc, m_direct_bitmap);
        return;
    }

    Image *img = images.load((char*)m_name.ToAscii(), BMP|JPG|PNG);
    if (img != 0) {
        int h = img->height;
        int w = img->width;
        unsigned char *pixels = (unsigned char*)img->data;

        if (img->gltype() == GL_RGBA) {
            unsigned char *alpha = (unsigned char*)malloc(h*w*1);
            unsigned char *rgb   = (unsigned char*)malloc(h*w*3);
            for (int i = 0; i < h; i++) {
                for (int j = 0; j < w; j++) {
                    rgb[i*w*3+j*3]   = pixels[i*w*4+j*4];
                    rgb[i*w*3+j*3+1] = pixels[i*w*4+j*4+1];
                    rgb[i*w*3+j*3+2] = pixels[i*w*4+j*4+2];
                    alpha[i*w+j]     = pixels[i*w*4+j*4+3];
                }
            }
            wxImage wim(w, h, rgb, false);
            wim.SetAlpha(alpha, false);
            wxBitmap bmp(wim, -1);
            if (option == TILE)
                TileBitmap(wxRect(0, 0, sz.x, sz.y), dc, bmp);
            else
                ScaleBitmap(wxRect(0, 0, sz.x, sz.y), dc, bmp);
        } else {
            wxImage wim(w, h, pixels, true);
            wxBitmap bmp(wim, -1);
            if (option == TILE)
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
    if (w < 1 || h < 1) return false;
    for (int y = rect.y; y < rect.y + rect.height; y += h)
        for (int x = rect.x; x < rect.x + rect.width; x += w)
            dc.DrawBitmap(bitmap, x, y, false);
    return true;
}

bool VtxImageWindow::ScaleBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap)
{
    if (bitmap.GetWidth() < 1 || bitmap.GetHeight() < 1) return false;
    wxImage img = bitmap.ConvertToImage();
    img = img.Scale(rect.width, rect.height, wxIMAGE_QUALITY_HIGH);
    dc.DrawBitmap(wxBitmap(img), rect.x, rect.y, false);
    return true;
}
