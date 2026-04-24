#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/image.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <cmath>
#include <algorithm>

#include "VtxProcessTabs.h"
#include "VtxImageDialog.h"
#include "ImageMgr.h"

#define BOX_W  (TABS_WIDTH - TABS_BORDER)
#define IMAGE_H  250
#define LABEL1  60
#define VALUE1  50
#define SLIDER1 120

enum {
    ID_PROC_FILE,
    ID_PROC_OP,
    ID_PROC_RADIUS_SLDR,
    ID_PROC_RADIUS_TEXT,     // must be SLDR+1 (Slider base uses id+1 for text)
    ID_PROC_STRENGTH_SLDR,
    ID_PROC_STRENGTH_TEXT,
    ID_PROC_ITERS_SLDR,
    ID_PROC_ITERS_TEXT,
    ID_PROC_GRAY,
    ID_PROC_TILE,
};

static const char *op_names[] = {
    "-- Select --",
    "Blur", "Sharpen","Dilate", "Erode",
    "Normalize", "Contrast", "Brightness",
    "Hydraulic Erosion",
    "Dendritic Erosion",
    nullptr
};

IMPLEMENT_CLASS(VtxProcessTabs, wxPanel)

BEGIN_EVENT_TABLE(VtxProcessTabs, wxPanel)
EVT_CHOICE(ID_PROC_FILE, VtxProcessTabs::OnFileSelect)
EVT_CHOICE(ID_PROC_OP,   VtxProcessTabs::OnOpSelect)
EVT_CHECKBOX(ID_PROC_GRAY, VtxProcessTabs::OnGrayCheck)
EVT_CHECKBOX(ID_PROC_TILE, VtxProcessTabs::OnTileCheck)
EVT_COMMAND_SCROLL(ID_PROC_RADIUS_SLDR,   VtxProcessTabs::OnRadiusSlider)
EVT_COMMAND_SCROLL(ID_PROC_STRENGTH_SLDR, VtxProcessTabs::OnStrengthSlider)
EVT_COMMAND_SCROLL(ID_PROC_ITERS_SLDR,   VtxProcessTabs::OnItersSlider)
EVT_TEXT_ENTER(ID_PROC_RADIUS_TEXT,   VtxProcessTabs::OnRadiusText)
EVT_TEXT_ENTER(ID_PROC_STRENGTH_TEXT, VtxProcessTabs::OnStrengthText)
EVT_TEXT_ENTER(ID_PROC_ITERS_TEXT,   VtxProcessTabs::OnItersText)
END_EVENT_TABLE()

VtxProcessTabs::VtxProcessTabs(wxWindow *parent, wxWindowID id,
    const wxPoint &pos, const wxSize &size, long style, const wxString &name)
    : wxPanel(parent, id, pos, size, style, name),
      m_w(0), m_h(0), m_has_image(false), m_modified(false), m_last_op(PROC_BLUR)
{
    buildUI(this);
    wxInitAllImageHandlers();
}

void VtxProcessTabs::buildUI(wxPanel *panel)
{
    wxBoxSizer *top = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(top);
    wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
    top->Add(box, 0, wxALIGN_LEFT|wxALL, 5);

    // ── File list (Textures/Processed/ only) ─────────────────────────
    wxStaticBoxSizer *file_box = new wxStaticBoxSizer(wxHORIZONTAL, panel, "File");
    m_file_menu = new wxChoice(panel, ID_PROC_FILE, wxDefaultPosition, wxSize(200,-1));
    file_box->Add(m_file_menu, 0, wxALIGN_LEFT|wxALL, 2);
    m_gray_check = new wxCheckBox(panel, ID_PROC_GRAY, "Grayscale");
    file_box->Add(m_gray_check, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    m_tile_check = new wxCheckBox(panel, ID_PROC_TILE, "Tile");
    m_tile_check->SetValue(true);
    file_box->Add(m_tile_check, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);

    file_box->SetMinSize(wxSize(BOX_W+10,-1));
    box->Add(file_box, 0, wxALIGN_LEFT|wxALL, 0);

    // ── Operation ─────────────────────────────────────────────────────
    wxStaticBoxSizer *op_box = new wxStaticBoxSizer(wxVERTICAL, panel, "Operation");

    wxBoxSizer *op_row = new wxBoxSizer(wxHORIZONTAL);
    m_op_menu = new wxChoice(panel, ID_PROC_OP, wxDefaultPosition, wxSize(160,-1));
    for (int i = 0; op_names[i]; i++) m_op_menu->Append(op_names[i]);
    m_op_menu->SetSelection(PROC_BLUR);
    op_row->Add(m_op_menu, 0, wxALIGN_LEFT|wxALL, 2);
    m_strength_slider = new SliderCtrl(panel, ID_PROC_STRENGTH_SLDR, "Strength",
           LABEL1, VALUE1, SLIDER1);
    m_strength_slider->setRange(0.0, 2.0); m_strength_slider->setValue(1.0);
    op_row->Add(m_strength_slider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
    op_box->Add(op_row, 0, wxALIGN_LEFT|wxALL, 0);

    wxBoxSizer *param_row = new wxBoxSizer(wxHORIZONTAL);
    m_radius_slider = new SliderCtrl(panel, ID_PROC_RADIUS_SLDR, "Radius",
        LABEL1, VALUE1, SLIDER1);
    m_radius_slider->setRange(1, 20); m_radius_slider->setValue(3);
    param_row->Add(m_radius_slider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);

    m_iters_slider = new SliderCtrl(panel, ID_PROC_ITERS_SLDR, "Iterations",
        LABEL1, VALUE1, SLIDER1);
    m_iters_slider->setRange(1, 500); m_iters_slider->setValue(50);
    param_row->Add(m_iters_slider->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);

    op_box->Add(param_row, 0, wxALIGN_LEFT|wxALL, 0);

    op_box->SetMinSize(wxSize(BOX_W,-1));
    box->Add(op_box, 0, wxALIGN_LEFT|wxALL, 0);

    // ── Image display ─────────────────────────────────────────────────
    wxStaticBoxSizer *img_box = new wxStaticBoxSizer(wxVERTICAL, panel, "Image");
    m_image_window = new VtxImageWindow(panel, wxID_ANY,
        wxDefaultPosition, wxSize(BOX_W, IMAGE_H));
    img_box->Add(m_image_window, 0, wxALIGN_LEFT|wxALL, 0);
    box->Add(img_box, 0, wxALIGN_LEFT|wxALL, 0);

    makeFileList();
}

// ── Directory / file helpers ──────────────────────────────────────────

void VtxProcessTabs::ensureProcessedDir()
{
    char path[512];
    FileUtil::getProcessedDir(path);
    int len = strlen(path);
    if (len>0 && (path[len-1]=='/'||path[len-1]=='\\')) path[len-1]=0;
    if (!wxDirExists(wxString(path))) wxMkdir(wxString(path));
}

void VtxProcessTabs::makeFileList()
{
    wxString sel = m_file_menu->GetStringSelection();
    m_file_menu->Clear();
    char dir[512]; dir[0] = 0;
    FileUtil::getProcessedDir(dir);
    if (dir[0] == 0) return;   // dir not yet configured
    wxString wxdir(dir);
    if (!wxDirExists(wxdir)) return;
    wxDir d(wxdir);
    wxString f;
    wxArrayString files;
    for (bool ok=d.GetFirst(&f,"*.bmp",wxDIR_FILES); ok; ok=d.GetNext(&f)) files.Add(wxFileName(f).GetName());
    for (bool ok=d.GetFirst(&f,"*.jpg",wxDIR_FILES); ok; ok=d.GetNext(&f)) files.Add(wxFileName(f).GetName());
    for (bool ok=d.GetFirst(&f,"*.png",wxDIR_FILES); ok; ok=d.GetNext(&f)) files.Add(wxFileName(f).GetName());
    files.Sort();
    for (size_t i=0; i<files.Count(); i++){
        if (!files[i].StartsWith("_"))   // skip temp/preview files
            m_file_menu->Append(files[i]);
    }
    // Restore previous selection if still present, else select first
    int idx = sel.IsEmpty() ? wxNOT_FOUND : m_file_menu->FindString(sel);
    if (idx != wxNOT_FOUND)
        m_file_menu->SetSelection(idx);
    else if (m_file_menu->GetCount() > 0)
        m_file_menu->SetSelection(0);
}

// ── Image load / save / display ───────────────────────────────────────

bool VtxProcessTabs::loadImage(const wxString &path)
{
    wxImage img;
    if (!img.LoadFile(path)) return false;
    m_w = img.GetWidth();
    m_h = img.GetHeight();
    bool has_alpha = img.HasAlpha();
    m_buf.resize(m_w * m_h * 4);
    unsigned char *rgb   = img.GetData();
    unsigned char *alpha = has_alpha ? img.GetAlpha() : nullptr;
    for (int i=0; i<m_h*m_w; i++) {
        m_buf[i*4+0] = rgb[i*3+0]/255.0f;
        m_buf[i*4+1] = rgb[i*3+1]/255.0f;
        m_buf[i*4+2] = rgb[i*3+2]/255.0f;
        m_buf[i*4+3] = alpha ? alpha[i]/255.0f : 1.0f;
    }
    m_orig = m_buf;
    m_has_image = true;
    m_modified  = false;
    return true;
}

bool VtxProcessTabs::saveImage(const wxString &path)
{
    if (!m_has_image) return false;
    wxImage img(m_w, m_h);
    bool has_alpha = false;
    for (int i=0;i<m_w*m_h;i++) if (m_buf[i*4+3]<0.999f){has_alpha=true;break;}
    if (has_alpha) img.InitAlpha();
    unsigned char *rgb = img.GetData(), *alpha = has_alpha ? img.GetAlpha() : nullptr;
    auto cl=[](float v)->unsigned char{
        return (unsigned char)(std::max(0.0f,std::min(1.0f,v))*255.0f+0.5f);};
    for (int i=0;i<m_h*m_w;i++){
        rgb[i*3+0]=cl(m_buf[i*4+0]); rgb[i*3+1]=cl(m_buf[i*4+1]); rgb[i*3+2]=cl(m_buf[i*4+2]);
        if(alpha) alpha[i]=cl(m_buf[i*4+3]);
    }
    return img.SaveFile(path, wxBITMAP_TYPE_BMP);
}

void VtxProcessTabs::displayBuffer()
{
    if (!m_has_image) return;
    int mode = m_tile_check->GetValue() ? VtxImageWindow::TILE : VtxImageWindow::SCALE;
    m_image_window->setImageData(m_buf.data(), m_w, m_h, mode);
}

// ── Public interface ──────────────────────────────────────────────────

void VtxProcessTabs::loadFromPath(const wxString &path, const wxString &name)
{
    wxString imgname = path.IsEmpty() ? name : path;
    if (imgname.IsEmpty()) return;

    Image *img = images.load((char*)imgname.ToAscii(), BMP|JPG|PNG);
    if (!img) {
        wxMessageBox("Cannot load image: " + imgname, "Process");
        return;
    }

    m_w = img->width;
    m_h = img->height;
    m_buf.resize(m_w * m_h * 4);

    unsigned char *pixels = (unsigned char*)img->data;
    bool rgba = (img->gltype() == GL_RGBA);

    for (int i = 0; i < m_h * m_w; i++) {
        if (rgba) {
            m_buf[i*4+0] = pixels[i*4+0] / 255.0f;
            m_buf[i*4+1] = pixels[i*4+1] / 255.0f;
            m_buf[i*4+2] = pixels[i*4+2] / 255.0f;
            m_buf[i*4+3] = pixels[i*4+3] / 255.0f;
        } else {
            m_buf[i*4+0] = pixels[i*3+0] / 255.0f;
            m_buf[i*4+1] = pixels[i*3+1] / 255.0f;
            m_buf[i*4+2] = pixels[i*3+2] / 255.0f;
            m_buf[i*4+3] = 1.0f;
        }
    }
    m_orig = m_buf;
    m_prev_buf.clear();
    m_has_image = true;
    m_modified  = false;
    m_name = name.IsEmpty() ? imgname : name;

    int idx = m_file_menu->FindString(m_name);
    if (idx != wxNOT_FOUND) m_file_menu->SetSelection(idx);

    // Restore the last op the user had selected (defaults to PROC_DILATE on first use)
    m_op_menu->SetSelection(m_last_op);

    // Detect whether the image is already grayscale (R==G==B for every pixel).
    // If so, check and disable the checkbox — there are no color components to lose.
    // If the image has color, uncheck and enable so the user can choose.
    bool is_gray = true;
    for (int i = 0; i < m_w * m_h && is_gray; i++) {
        float r = m_buf[i*4+0], g = m_buf[i*4+1], b = m_buf[i*4+2];
        if (fabsf(r - g) > 1.0f/255.0f || fabsf(r - b) > 1.0f/255.0f)
            is_gray = false;
    }
    m_gray_check->SetValue(is_gray);
    m_gray_check->Enable(!is_gray);

    displayBuffer();
}

void VtxProcessTabs::runOperation()
{
    if (!m_has_image) return;
    int   op    = m_op_menu->GetSelection();
    if (op == PROC_NONE) return;
    int   r     = (int)m_radius_slider->getValue();
    float str   = (float)m_strength_slider->getValue();
    int   iters = (int)m_iters_slider->getValue();
    bool  gray  = m_gray_check->GetValue();
    m_prev_buf = m_buf;   // snapshot for single-step Revert
    m_last_op  = op;
    if (gray) {
        m_buf = m_orig;   // start from color original so toGrayscale has full data
        toGrayscale();
    }
    switch (op) {
    case PROC_DILATE:    opDilate(r, gray);       break;
    case PROC_ERODE_IMG: opErodeImg(r, gray);     break;
    case PROC_BLUR:      opBlur(r, gray);         break;
    case PROC_SHARPEN:   opSharpen(str, gray);    break;
    case PROC_NORMALIZE: opNormalize(gray);       break;
    case PROC_CONTRAST:  opContrast(str, gray);   break;
    case PROC_BRIGHTNESS:opBrightness(str, gray); break;
    case PROC_HYDRAULIC:  opHydraulic(iters, str);              break;
    case PROC_DENDRITIC:  opDendritic(iters, (float)m_radius_slider->getValue()*0.05f, str); break;
    default: return;
    }
    m_modified = true;
    displayBuffer();
    imageDialog->UpdateControls();
}

void VtxProcessTabs::Save()
{
    if (!m_has_image) return;
    ensureProcessedDir();
    wxString name = m_name.IsEmpty() ? wxString("processed") : m_name;
    wxTextEntryDialog dlg(this, "Save as:", "Save Processed", name);
    if (dlg.ShowModal() != wxID_OK) return;
    name = dlg.GetValue();
    if (name.IsEmpty()) return;
    char dir[512]; FileUtil::getProcessedDir(dir);
    wxString path = wxString(dir) + name + ".bmp";
    if (saveImage(path)) {
        m_name = name;
        m_modified = false;
        makeFileList();
        int idx = m_file_menu->FindString(name);
        if (idx != wxNOT_FOUND) m_file_menu->SetSelection(idx);
    } else {
        wxMessageBox("Save failed: "+path, "Process");
    }
}

void VtxProcessTabs::Delete()
{
    if(m_name.IsEmpty()) return;
    char dir[512]; FileUtil::getProcessedDir(dir);
    wxString base = wxString(dir) + m_name;
    if(wxFileExists(base+".bmp")) wxRemoveFile(base+".bmp");
    if(wxFileExists(base+".jpg")) wxRemoveFile(base+".jpg");
    if(wxFileExists(base+".png")) wxRemoveFile(base+".png");
    m_has_image = false;
    m_name = "";
    m_buf.clear();
    m_orig.clear();
    m_prev_buf.clear();
    updateControls();
}

void VtxProcessTabs::Revert()
{
    if (!m_has_image || m_prev_buf.empty()) return;
    m_buf = m_prev_buf;
    m_prev_buf.clear();
    m_modified = !m_buf.empty() && (m_buf != m_orig);
    displayBuffer();
    imageDialog->UpdateControls();
}

void VtxProcessTabs::updateControls()
{
    makeFileList();
    // If no image is loaded yet, auto-load the first file in the list
    if(!m_has_image && m_file_menu->GetCount() > 0){
        m_file_menu->SetSelection(0);
        wxString name = m_file_menu->GetString(0);
        char dir[512]; dir[0]=0;
        FileUtil::getProcessedDir(dir);
        wxString base = wxString(dir) + name;
        wxString path;
        if      (wxFileExists(base+".bmp")) path = base+".bmp";
        else if (wxFileExists(base+".jpg")) path = base+".jpg";
        else if (wxFileExists(base+".png")) path = base+".png";
        if(!path.IsEmpty()){
            loadImage(path);
            m_name = name;
            displayBuffer();
        }
    }
}

// ── Slider / text-entry event handlers ───────────────────────────────

void VtxProcessTabs::OnRadiusSlider(wxScrollEvent &event)
{
    m_radius_slider->setValueFromSlider();
}
void VtxProcessTabs::OnStrengthSlider(wxScrollEvent &event)
{
    m_strength_slider->setValueFromSlider();
}
void VtxProcessTabs::OnItersSlider(wxScrollEvent &event)
{
    m_iters_slider->setValueFromSlider();
}
void VtxProcessTabs::OnRadiusText(wxCommandEvent &event)
{
    m_radius_slider->setValueFromText();
}
void VtxProcessTabs::OnStrengthText(wxCommandEvent &event)
{
    m_strength_slider->setValueFromText();
}
void VtxProcessTabs::OnItersText(wxCommandEvent &event)
{
    m_iters_slider->setValueFromText();
}

void VtxProcessTabs::OnGrayCheck(wxCommandEvent &event)
{
    if (!m_has_image) return;
    if (m_gray_check->GetValue()) {
        // Checked: convert current m_buf to grayscale and show
        toGrayscale();
    } else {
        // Unchecked: restore color from m_orig, then re-apply the last op if any
        m_buf = m_orig;
        if (!m_prev_buf.empty()) {
            // An op was previously applied — re-run it without gray so color is preserved
            int op = m_last_op;
            int   r     = (int)m_radius_slider->getValue();
            float str   = (float)m_strength_slider->getValue();
            int   iters = (int)m_iters_slider->getValue();
            switch (op) {
            case PROC_BLUR:      opBlur(r, false);         break;
            case PROC_SHARPEN:   opSharpen(str, false);    break;
            case PROC_DILATE:    opDilate(r, false);       break;
            case PROC_ERODE_IMG: opErodeImg(r, false);     break;
            case PROC_NORMALIZE: opNormalize(false);       break;
            case PROC_CONTRAST:  opContrast(str, false);   break;
            case PROC_BRIGHTNESS:opBrightness(str, false); break;
            case PROC_HYDRAULIC:  opHydraulic(iters, str);  break;
            case PROC_DENDRITIC:  opDendritic(iters, (float)m_radius_slider->getValue()*0.05f, str); break;
            default: break;
            }
        }
    }
    displayBuffer();
}

void VtxProcessTabs::OnTileCheck(wxCommandEvent &event)
{
    displayBuffer();
}

void VtxProcessTabs::OnFileSelect(wxCommandEvent &event)
{
    wxString name = m_file_menu->GetStringSelection();
    if (name.IsEmpty()) return;
    char dir[512]; FileUtil::getProcessedDir(dir);
    wxString base = wxString(dir) + name;
    wxString path;
    if      (wxFileExists(base+".bmp")) path=base+".bmp";
    else if (wxFileExists(base+".jpg")) path=base+".jpg";
    else if (wxFileExists(base+".png")) path=base+".png";
    else return;
    loadImage(path);
    m_name = name;
    displayBuffer();
}

void VtxProcessTabs::OnOpSelect(wxCommandEvent &event)
{
    int op = m_op_menu->GetSelection();
    switch (op) {
    case PROC_HYDRAULIC:
        m_iters_slider->setRange(1,500); m_iters_slider->setValue(100);
        m_radius_slider->setRange(1,10);  m_radius_slider->setValue(2);
        break;
    case PROC_DENDRITIC:
        m_iters_slider->setRange(1,200);  m_iters_slider->setValue(30);
        m_radius_slider->setRange(1,20);  m_radius_slider->setValue(5);   // branch prob * 0.05
        m_strength_slider->setRange(0.0,2.0); m_strength_slider->setValue(1.0);
        break;
    case PROC_CONTRAST: case PROC_BRIGHTNESS:
        m_strength_slider->setRange(-2.0,2.0); m_strength_slider->setValue(0.0);
        break;
    default:
        m_radius_slider->setRange(1,20);      m_radius_slider->setValue(3);
        m_strength_slider->setRange(0.0,2.0); m_strength_slider->setValue(1.0);
        break;
    }
}

// ── Operations ────────────────────────────────────────────────────────

void VtxProcessTabs::toGrayscale()
{
    for (int i = 0; i < m_w * m_h; i++) {
        float luma = 0.299f * m_buf[i*4+0]
                   + 0.587f * m_buf[i*4+1]
                   + 0.114f * m_buf[i*4+2];
        m_buf[i*4+0] = m_buf[i*4+1] = m_buf[i*4+2] = luma;
    }
}

void VtxProcessTabs::opDilate(int r, bool gray)
{
    std::vector<float> out=m_buf;
    for (int y=0;y<m_h;y++) for (int x=0;x<m_w;x++) {
        float mx[4]={0,0,0,0};
        for (int dy=-r;dy<=r;dy++){int ny=std::max(0,std::min(m_h-1,y+dy));
        for (int dx=-r;dx<=r;dx++){int nx=std::max(0,std::min(m_w-1,x+dx));
            if(gray){float v=0.299f*m_buf[(ny*m_w+nx)*4+0]+0.587f*m_buf[(ny*m_w+nx)*4+1]+0.114f*m_buf[(ny*m_w+nx)*4+2];
                mx[0]=mx[1]=mx[2]=std::max(mx[0],v);}
            else for(int c=0;c<4;c++) mx[c]=std::max(mx[c],m_buf[(ny*m_w+nx)*4+c]);
        }}
        for(int c=0;c<4;c++) out[(y*m_w+x)*4+c]=mx[c];
    }
    m_buf=out;
}

void VtxProcessTabs::opErodeImg(int r, bool gray)
{
    std::vector<float> out=m_buf;
    for (int y=0;y<m_h;y++) for (int x=0;x<m_w;x++) {
        float mn[4]={1,1,1,1};
        for (int dy=-r;dy<=r;dy++){int ny=std::max(0,std::min(m_h-1,y+dy));
        for (int dx=-r;dx<=r;dx++){int nx=std::max(0,std::min(m_w-1,x+dx));
            if(gray){float v=0.299f*m_buf[(ny*m_w+nx)*4+0]+0.587f*m_buf[(ny*m_w+nx)*4+1]+0.114f*m_buf[(ny*m_w+nx)*4+2];
                mn[0]=mn[1]=mn[2]=std::min(mn[0],v);}
            else for(int c=0;c<4;c++) mn[c]=std::min(mn[c],m_buf[(ny*m_w+nx)*4+c]);
        }}
        for(int c=0;c<4;c++) out[(y*m_w+x)*4+c]=mn[c];
    }
    m_buf=out;
}

void VtxProcessTabs::opBlur(int r, bool gray)
{
    int ksize=2*r+1; std::vector<float> k(ksize); float sigma=r/2.0f+0.5f,sum=0;
    for(int i=0;i<ksize;i++){float x=i-r;k[i]=expf(-x*x/(2*sigma*sigma));sum+=k[i];}
    for(auto &v:k) v/=sum;
    int nch=gray?3:4;
    std::vector<float> tmp=m_buf,out=m_buf;
    for(int y=0;y<m_h;y++) for(int x=0;x<m_w;x++) for(int c=0;c<nch;c++){
        float acc=0;
        for(int dx=-r;dx<=r;dx++){int nx=std::max(0,std::min(m_w-1,x+dx));acc+=k[dx+r]*m_buf[(y*m_w+nx)*4+c];}
        tmp[(y*m_w+x)*4+c]=acc;}
    for(int y=0;y<m_h;y++) for(int x=0;x<m_w;x++) for(int c=0;c<nch;c++){
        float acc=0;
        for(int dy=-r;dy<=r;dy++){int ny=std::max(0,std::min(m_h-1,y+dy));acc+=k[dy+r]*tmp[(ny*m_w+x)*4+c];}
        out[(y*m_w+x)*4+c]=acc;}
    m_buf=out;
}

void VtxProcessTabs::opSharpen(float strength, bool gray)
{
    std::vector<float> orig=m_buf; opBlur(2,gray);
    std::vector<float> blur=m_buf; m_buf=orig;
    int nch=gray?3:4;
    for(int i=0;i<m_w*m_h;i++) for(int c=0;c<nch;c++){
        float v=orig[i*4+c]+strength*(orig[i*4+c]-blur[i*4+c]);
        m_buf[i*4+c]=std::max(0.0f,std::min(1.0f,v));}
}

void VtxProcessTabs::opNormalize(bool gray)
{
    int nch=gray?3:4; float mn=1e9f,mx=-1e9f;
    for(int i=0;i<m_w*m_h;i++) for(int c=0;c<nch;c++){mn=std::min(mn,m_buf[i*4+c]);mx=std::max(mx,m_buf[i*4+c]);}
    float range=mx-mn; if(range<1e-6f) return;
    for(int i=0;i<m_w*m_h;i++) for(int c=0;c<nch;c++) m_buf[i*4+c]=(m_buf[i*4+c]-mn)/range;
}

void VtxProcessTabs::opContrast(float strength, bool gray)
{
    float factor=(strength>=0)?1.0f+strength:1.0f/(1.0f-strength);
    int nch=gray?3:4;
    for(int i=0;i<m_w*m_h;i++) for(int c=0;c<nch;c++){
        float v=(m_buf[i*4+c]-0.5f)*factor+0.5f;
        m_buf[i*4+c]=std::max(0.0f,std::min(1.0f,v));}
}

void VtxProcessTabs::opBrightness(float amount, bool gray)
{
    int nch=gray?3:4;
    for(int i=0;i<m_w*m_h;i++) for(int c=0;c<nch;c++)
        m_buf[i*4+c]=std::max(0.0f,std::min(1.0f,m_buf[i*4+c]+amount));
}

void VtxProcessTabs::opHydraulic(int iters, float strength)
{
    int N=m_w*m_h; std::vector<float> h(N),sed(N,0.0f);
    for(int i=0;i<N;i++) h[i]=0.299f*m_buf[i*4+0]+0.587f*m_buf[i*4+1]+0.114f*m_buf[i*4+2];
    const float Kr=0.01f*strength,Kd=0.005f*strength,Ke=0.5f;
    std::vector<float> dh(N);
    for(int iter=0;iter<iters;iter++){
        std::fill(dh.begin(),dh.end(),0.0f);
        for(int y=1;y<m_h-1;y++) for(int x=1;x<m_w-1;x++){
            int idx=y*m_w+x; float hc=h[idx];
            int nbrs[4]={idx-m_w,idx+m_w,idx-1,idx+1};
            float max_diff=0; int lowest=-1;
            for(int n:nbrs){float d=hc-h[n];if(d>max_diff){max_diff=d;lowest=n;}}
            if(lowest<0) continue;
            float cap=Ke*max_diff,carry=sed[idx];
            if(carry<cap){float amt=Kr*(cap-carry);dh[idx]-=amt;sed[idx]+=amt;}
            else{float amt=Kd*(carry-cap);dh[lowest]+=amt;sed[idx]-=amt;}
            dh[idx]-=0.5f*max_diff; dh[lowest]+=0.5f*max_diff*0.99f;
        }
        for(int i=0;i<N;i++) h[i]=std::max(0.0f,std::min(1.0f,h[i]+dh[i]));
    }
    for(int i=0;i<N;i++){m_buf[i*4+0]=m_buf[i*4+1]=m_buf[i*4+2]=h[i];}
}

void VtxProcessTabs::opDendritic(int seeds, float branchProb, float strength)
{
    // Flow accumulation approach: every pixel contributes flow downhill.
    // Pixels where many upstream pixels drain through become dark channels.
    // seeds    = threshold — minimum flow to show as channel (lower = more channels)
    // branchProb * 20 = flow exponent (higher = sharper channel contrast)
    // strength = carving depth

    int N = m_w * m_h;

    std::vector<float> h(N);
    for(int i = 0; i < N; i++)
        h[i] = 0.299f*m_buf[i*4+0] + 0.587f*m_buf[i*4+1] + 0.114f*m_buf[i*4+2];

    // flow[i] = number of pixels that drain through pixel i (including itself)
    std::vector<float> flow(N, 1.0f);

    const int dx[8] = { 0, 0, 1,-1, 1,-1, 1,-1};
    const int dy[8] = {-1, 1, 0, 0,-1,-1, 1, 1};

    // For each pixel, find the lowest neighbor using wraparound so tiled
    // images produce seamless flow across the tile boundary.
    std::vector<int> drain(N, -1);
    for(int y = 0; y < m_h; y++){
        for(int x = 0; x < m_w; x++){
            int idx = y*m_w+x;
            float hc = h[idx];
            int   best = -1;
            float best_drop = -1e9f;
            for(int d = 0; d < 8; d++){
                int nx = (x+dx[d]+m_w) % m_w;
                int ny = (y+dy[d]+m_h) % m_h;
                float dist = (d<4)?1.0f:1.414f;
                float drop = (hc - h[ny*m_w+nx]) / dist;
                if(drop > best_drop){ best_drop = drop; best = ny*m_w+nx; }
            }
            if(best_drop < -0.001f) drain[idx] = -1;
            else drain[idx] = best;
        }
    }

    // Propagate flow: sort pixels by height descending, then push flow downhill
    std::vector<int> order(N);
    for(int i = 0; i < N; i++) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b){ return h[a] > h[b]; });

    for(int idx : order){
        if(drain[idx] >= 0)
            flow[drain[idx]] += flow[idx];
    }

    // Find max flow for normalisation
    float max_flow = *std::max_element(flow.begin(), flow.end());
    if(max_flow < 2.0f) return;

    // Threshold: fraction of max_flow needed to show as channel
    // seeds=1 -> almost everything, seeds=200 -> only major channels
    float threshold = (seeds / 1000.0f) * max_flow;
    // Exponent controls sharpness: higher = only major rivers show
    float exponent = 1.0f + branchProb * 20.0f;

    for(int i = 0; i < N; i++){
        if(flow[i] <= threshold) continue;
        float t = (flow[i] - threshold) / (max_flow - threshold);
        t = powf(t, 1.0f / exponent);  // gamma — makes thin branches visible
        h[i] = std::max(0.0f, h[i] - t * strength);
    }

    for(int i = 0; i < N; i++)
        m_buf[i*4+0] = m_buf[i*4+1] = m_buf[i*4+2] = h[i];
}
