#include "VtxImageDialog.h"
#include "FileUtil.h"
#include <wx/filefn.h>
#include <wx/dir.h>
#include "RenderOptions.h"

extern wxString last_gradient;

//########################### VtxImageTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
    ID_NORM,
    ID_INVERT,
    ID_GRAYS,
	ID_FILELIST,
    ID_IMAGE_EXPR,
    ID_IMAGE_WIDTH,
    ID_IMAGE_HEIGHT,
    ID_IMAGE_MAP,
	ID_GRADIENT,
	ID_GRADIENT_LIST,
	ID_SHOW_TMPS,
};

#define LABEL1 50
#define VALUE1 50
#define SLIDER1 120
#define IMAGE_H  250

IMPLEMENT_CLASS(VtxImageTabs, wxPanel)

BEGIN_EVENT_TABLE(VtxImageTabs, wxPanel)
EVT_TEXT_ENTER(ID_IMAGE_EXPR,VtxImageTabs::OnExprEdit)
EVT_CHOICE(ID_IMAGE_WIDTH,   VtxImageTabs::OnImageSize)
EVT_CHOICE(ID_IMAGE_HEIGHT,  VtxImageTabs::OnImageSize)
EVT_CHOICE(ID_IMAGE_MAP,     VtxImageTabs::OnChanged)
EVT_CHECKBOX(ID_NORM,        VtxImageTabs::OnChanged)
EVT_CHECKBOX(ID_INVERT,      VtxImageTabs::OnChanged)
EVT_CHECKBOX(ID_GRAYS,       VtxImageTabs::OnGraysMode)
EVT_CHECKBOX(ID_GRADIENT,    VtxImageTabs::OnGradientMode)
EVT_CHOICE(ID_FILELIST,      VtxImageTabs::OnFileSelect)
EVT_CHOICE(ID_GRADIENT_LIST, VtxImageTabs::OnGradientSelect)
EVT_CHECKBOX(ID_SHOW_TMPS,   VtxImageTabs::OnShowTmps)
END_EVENT_TABLE()

VtxImageTabs::VtxImageTabs(wxWindow* parent, wxWindowID id,
		const wxPoint& pos, const wxSize& size, long style, const wxString& name)
		: wxPanel(parent, id, pos, size, style, name)
{
	AddImageTab(this);
	update_needed = true;
	type = IMAGE|SPX;
	image_list  = 0;
	revert_list = 0;
	setImageName("");
	gradient_list = 0;
}

void VtxImageTabs::AddImageTab(wxPanel *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer *topline = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *hline   = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer* fileio = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("File"));

	m_file_menu = new wxChoice(panel, ID_FILELIST, wxDefaultPosition, wxSize(160,-1));
	m_file_menu->SetColumns(5);
    hline->Add(m_file_menu, 0, wxALIGN_LEFT|wxALL, 0);

	wxString sizes[]={"1","2","4","8","16","32","64","128","256","512","1024"};
	m_image_height = new wxChoice(panel, ID_IMAGE_HEIGHT, wxDefaultPosition, wxSize(60,-1), 11, sizes);
	m_image_height->SetSelection(7);
	hline->Add(m_image_height, 0, wxALIGN_LEFT|wxALL, 0);

	m_image_width = new wxChoice(panel, ID_IMAGE_WIDTH, wxDefaultPosition, wxSize(60,-1), 11, sizes);
	m_image_width->SetSelection(7);
	hline->Add(m_image_width, 0, wxALIGN_LEFT|wxALL, 0);
	hline->AddSpacer(2);

	wxString maps[]={"Tile","Decal","Sphere","Cylinder"};
	m_image_map = new wxChoice(panel, ID_IMAGE_MAP, wxDefaultPosition, wxSize(100,-1), 4, maps);
	m_image_map->SetSelection(0);
	hline->Add(m_image_map, 0, wxALIGN_LEFT|wxALL, 0);

    m_show_tmps = new wxCheckBox(panel, ID_SHOW_TMPS, "Show Tmps");
    hline->Add(m_show_tmps, 0, wxALIGN_LEFT|wxALL, 5);

	fileio->Add(hline, 0, wxALIGN_LEFT|wxALL);
	fileio->SetMinSize(wxSize(TABS_WIDTH,-1));
	topline->Add(fileio, 0, wxALIGN_LEFT|wxALL, 0);
	boxSizer->Add(topline, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticBoxSizer* options = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("Options"));
    m_norm_check   = new wxCheckBox(panel, ID_NORM,    "Norm");    options->Add(m_norm_check,   0, wxALIGN_LEFT|wxALL, 0);
    m_invert_check = new wxCheckBox(panel, ID_INVERT,  "Invert");  options->Add(m_invert_check, 0, wxALIGN_LEFT|wxALL, 0);
    m_grays_check  = new wxCheckBox(panel, ID_GRAYS,   "Gray");    options->Add(m_grays_check,  0, wxALIGN_LEFT|wxALL, 0);
    m_gradient_check = new wxCheckBox(panel, ID_GRADIENT, "Color");options->Add(m_gradient_check,0,wxALIGN_LEFT|wxALL, 0);

    m_gradient_file_menu = new wxChoice(panel, ID_GRADIENT_LIST, wxPoint(-1,4), wxSize(120,-1));
    m_gradient_file_menu->SetColumns(5);
    options->Add(m_gradient_file_menu, 0, wxALIGN_LEFT|wxALL, 0);
	m_gradient_image = new VtxImageWindow(panel, wxID_ANY, wxDefaultPosition, wxSize(100,20));
	options->Add(m_gradient_image, 0, wxALIGN_LEFT|wxALL, 2);
    options->SetMinSize(wxSize(TABS_WIDTH,-1));
    boxSizer->Add(options, 0, wxALIGN_LEFT|wxALL, 0);

	wxStaticBoxSizer* image_expr_line = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Function"));
	m_image_expr = new ExprTextCtrl(panel, ID_IMAGE_EXPR, "", 0, TABS_WIDTH-TABS_BORDER);
	image_expr_line->Add(m_image_expr->getSizer(), 0, wxALIGN_LEFT|wxALL, 0);
	image_expr_line->SetMinSize(wxSize(TABS_WIDTH,-1));
	boxSizer->Add(image_expr_line, 0, wxALIGN_LEFT|wxALL, 0);

	wxStaticBoxSizer* image_display = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Image"));
	m_image_window = new VtxImageWindow(this, wxID_ANY, wxDefaultPosition, wxSize(TABS_WIDTH-TABS_BORDER,IMAGE_H));
	image_display->Add(m_image_window, 0, wxALIGN_LEFT|wxALL, 0);
	image_display->SetMinSize(wxSize(TABS_WIDTH,-1));
	boxSizer->Add(image_display, 0, wxALIGN_LEFT|wxALL, 0);
}

VtxImageTabs::~VtxImageTabs(){
	freeImageList();
	freeRevertList();
	delete m_image_expr;
}

// ── Temp name helpers ────────────────────────────────────────
// Working copy uses "_name" (underscore prefix, TMP flag).

wxString VtxImageTabs::tmpName(wxString name){
	return wxString("_") + name;
}

bool VtxImageTabs::isTmp(wxString name){
	return !name.IsEmpty() && name[0] == '_';
}

wxString VtxImageTabs::baseName(wxString name){
	if (isTmp(name)) return name.Mid(1);
	return name;
}

// ── File list management ─────────────────────────────────────

void VtxImageTabs::freeImageList(){
	if(image_list){ image_list->free(); delete image_list; image_list=0; }
}

void VtxImageTabs::freeRevertList(){
	if(revert_list){ revert_list->free(); delete revert_list; revert_list=0; }
}

void VtxImageTabs::makeImageList(){
	// Sort before makeImagelist() so addImages' inlist() check finds
	// any entries already registered by n->init(), preventing duplicates
	images.images.sort();
	images.makeImagelist();
	LinkedList<ImageSym*> list;
	images.getImageInfo(type, list);
	freeImageList();
	image_list = new NameList<ImageSym*>(list);
	image_list->ss();

	m_file_menu->Clear();
	ImageSym *is;
	while((is=(*image_list)++)){
		if(is->name()[0] == '_') continue;
		if(!m_show_tmps->GetValue() && (is->info & TMP)) continue;
		m_file_menu->Append(is->name());
	}
	// Select best matching entry
	wxString base = baseName(m_name);
	int index = m_file_menu->FindString(base);
	if(index == wxNOT_FOUND) index = 0;
	if(m_file_menu->GetCount() > 0){
		m_file_menu->SetSelection(index);
		// If m_name is empty or not yet set, auto-load the first entry
		if(m_name.IsEmpty()){
			m_name = m_file_menu->GetStringSelection();
		}
	}
}

void VtxImageTabs::freeGradientsList(){
	if(gradient_list){ gradient_list->free(); delete gradient_list; gradient_list=0; }
}

void VtxImageTabs::makeGradientsList(){
	LinkedList<ImageSym*> list;
	images.getImageInfo(BANDS|T1D|SPX, list);
	freeGradientsList();
	gradient_list = new NameList<ImageSym*>(list);
	gradient_list->ss();

	m_gradient_file_menu->Clear();
	ImageSym *is;
	while((is=(*gradient_list)++)){
		if(!m_show_tmps->GetValue() && (is->info & TMP)) continue;
		m_gradient_file_menu->Append(is->name());
	}
	m_gradient_file_menu->SetSelection(0);
}

// ── Load a name into the working copy ────────────────────────
// Creates a temp "_name" copy of the spx for editing.
// The displayed image uses the temp copy so the original is untouched.

void VtxImageTabs::loadIntoWorkingCopy(wxString name){
	if(name.IsEmpty()) return;
	wxString tmp = tmpName(name);

	// Copy the spx file content into a temp working copy
	char *spx = images.readSpxFile((char*)name.ToAscii());
	if(spx){
		// Build a modified istring that uses the tmp name
		// Replace "name" with "_name" in the image() call
		wxString expr(spx);
		FREE(spx);
		// Replace first quoted name occurrence
		expr.Replace(wxString("\"") + name + wxString("\""),
		             wxString("\"") + tmp  + wxString("\""), false);
		images.saveSpxFile((char*)tmp.ToAscii(), (char*)expr.ToAscii());

		// Parse and init the temp image
		char buff[4096];
		strcpy(buff, expr.ToAscii());
		TNinode *n = (TNinode*)TheScene->parse_node(buff);
		if(n){ BIT_ON(n->opts, TMP|CHANGED); n->init(); delete n; }
	}

	m_name = tmp;
	update_needed = false;
}

// ── Display helpers ──────────────────────────────────────────

void VtxImageTabs::setGradientName(){
	int index = m_gradient_file_menu->FindString(m_gradient_name);
	if(index == wxNOT_FOUND) index = 0;
	if(m_gradient_file_menu->GetCount() > 0){
		m_gradient_file_menu->SetSelection(index);
		m_gradient_name = m_gradient_file_menu->GetStringSelection();
	}
	m_gradient_image->setImage(wxString(m_gradient_name), m_gradient_image->TILE);
}

void VtxImageTabs::displayImage(char *name){
	if(!name || name[0]==0) return;

	// Look up istring from full image list (includes tmps)
	// Fall back to reading spx from disk if not in memory yet
	ImageSym *is = images.images.inlist(name);
	char *spx_buf = 0;
	const char *istring = 0;

	if(is && is->istring){
		istring = is->istring;
	} else {
		// Not in memory — read from disk (base name)
		wxString base = baseName(wxString(name));
		spx_buf = images.readSpxFile((char*)base.ToAscii());
		if(!spx_buf){
			// Try the name as-is
			spx_buf = images.readSpxFile(name);
		}
		istring = spx_buf;
	}
	if(!istring) return;

	NodeIF *n = TheScene->parse_node(const_cast<char*>(istring));
	FREE(spx_buf);
	if(!n) return;
	TNimage *inode = (TNimage*)n;
	int opts = inode->opts;
	int map_type = opts & IMAP;
	int display_mode = VtxImageWindow::TILE;
	switch(map_type){
		default: case TILE: m_image_map->SetSelection(0); break;
		case PMAP: m_image_map->SetSelection(1); break;
		case SMAP: m_image_map->SetSelection(2); display_mode=VtxImageWindow::SCALE; break;
		case CMAP: m_image_map->SetSelection(3); display_mode=VtxImageWindow::SCALE; break;
	}
	m_norm_check->SetValue((opts&NORM)?true:false);
	m_invert_check->SetValue((opts&INVT)?true:false);
	m_grays_check->SetValue((opts&GRAY)?true:false);
	m_gradient_check->SetValue((opts&ACHNL)?true:false);

	TNarg *arg = (TNarg*)inode->right;
	arg->eval(); m_image_width->SetSelection((int)log2((int)S0.s));
	arg=arg->next(); arg->eval(); m_image_height->SetSelection((int)log2((int)S0.s));
	arg=arg->next();
	TNode *vnode=arg->left; vnode->init(); vnode->eval();
	char vals[512]=""; vnode->valueString(vals);
	m_image_expr->SetValue(vals);
	arg=arg->next();
	if(arg){ arg->eval(); if(S0.strvalid()){ m_gradient_name=S0.string; last_gradient=m_gradient_name; } }

	// Show the working copy (tmp) in the image window
	m_image_window->setImage(wxString(name), display_mode);
	setGradientName();
	delete inode;
}

// ── Public controls ──────────────────────────────────────────

void VtxImageTabs::updateControls(){
	getObjAttributes();
}

void VtxImageTabs::getObjAttributes(){
	m_show_tmps->SetValue(TNimage::show_tmps || TheScene->tmp_files > 0);
	makeImageList();
	makeGradientsList();
	// Always display whatever is currently selected
	if(!m_name.IsEmpty())
		displayImage((char*)m_name.ToAscii());
}

void VtxImageTabs::Invalidate(){
	update_needed = true;
	setModified(true);
	rebuild();
	getObjAttributes();
}

// ── Build the image expression string from current controls ──

wxString VtxImageTabs::getImageString(wxString name){
	int itype = m_image_map->GetSelection();
	char opts[256];
	switch(itype){
		default: case 0: strcpy(opts,"TILE"); break;
		case 1: strcpy(opts,"PMAP"); break;
		case 2: strcpy(opts,"SMAP"); break;
		case 3: strcpy(opts,"CMAP"); break;
	}
	if(m_norm_check->GetValue())     strcat(opts,"|NORM");
	if(m_invert_check->GetValue())   strcat(opts,"|INVT");
	if(m_grays_check->GetValue())    strcat(opts,"|GRAY");
	if(m_gradient_check->GetValue()) strcat(opts,"|ACHNL");

	wxString istr = m_image_expr->GetValue();
	int width  = 1 << m_image_width->GetSelection();
	int height = 1 << m_image_height->GetSelection();

	char buff[512];
	if(m_gradient_check->GetValue() && !m_gradient_name.IsEmpty())
		sprintf(buff,"image(\"%s\",%s,%d,%d,%s,\"%s\");\n",
			(const char*)name.ToAscii(), opts, width, height,
			(const char*)istr.ToAscii(), m_gradient_name.ToAscii());
	else
		sprintf(buff,"image(\"%s\",%s,%d,%d,%s);\n",
			(const char*)name.ToAscii(), opts, width, height,
			(const char*)istr.ToAscii());
	return wxString(buff);
}

// ── Rebuild the working copy image from current controls ─────

int VtxImageTabs::rebuild(){
	if(m_name.IsEmpty()) return 0;

	// Create working copy on first edit if we're still on the base name.
	if(!isTmp(m_name)){
		m_name = tmpName(m_name);
	}

	// Rebuild into the temp working copy from current UI controls
	wxString istr = getImageString(m_name);
	char buff[512];
	strcpy(buff, istr.ToAscii());
	// Write spx to disk before init() so getImageInfo("_name") finds it
	images.saveSpxFile((char*)m_name.ToAscii(), buff);

	TNinode *n = (TNinode*)TheScene->parse_node(buff);
	if(!n){ cout<<"error parsing expr:"<<buff<<endl; return 0; }
	BIT_ON(n->opts, CHANGED|TMP);
	n->init();
	// addImage appends without sorting — sort so binary inlist() works
	images.images.sort();
	int opts = n->opts;
	delete n;
	return opts;
}

void VtxImageTabs::setObjAttributes(){
	if(!update_needed) return;
	TNimage::show_tmps = m_show_tmps->GetValue();
	int opts = rebuild();
	int map_type = opts & IMAP;
	int display_mode = map_type==SMAP ? VtxImageWindow::SCALE : VtxImageWindow::TILE;
	m_image_window->setImage(m_name, display_mode);
	Render.invalidate_textures();
	TheScene->set_changed_detail();
	TheScene->rebuild_all();
	imageDialog->UpdateControls();
	update_needed = false;
}

// ── Modified flag ────────────────────────────────────────────

void VtxImageTabs::setModified(bool b){
	if(m_name.IsEmpty()) return;
	// Use base name for flag storage in image_list
	wxString base = baseName(m_name);
	if(image_list){
		ImageSym *is = image_list->inlist((char*)base.ToAscii());
		if(is){ if(b) BIT_ON(is->info,CHANGED); else BIT_OFF(is->info,CHANGED); }
	}
	Image *img = images.find((char*)m_name.ToAscii());
	if(img) img->set_changed(b?1:0);
}

bool VtxImageTabs::isModified(){
	if(m_name.IsEmpty()) return false;
	// Modified if we have a tmp working copy, or CHANGED flag set on base
	if(isTmp(m_name)) return true;
	if(!image_list) return false;
	wxString base = baseName(m_name);
	ImageSym *is = image_list->inlist((char*)base.ToAscii());
	return is && (is->info & CHANGED);
}

// ── Save: write working copy to Bitmaps ─────────────────────
// Saves _name.spx→name.spx and _name.bmp→name.bmp

void VtxImageTabs::Save(){
	wxString base = baseName(m_name);
	if(base.IsEmpty()) return;

	// Build final expression using the real name (not tmp)
	wxString istr = getImageString(base);
	char buff[512];
	strcpy(buff, istr.ToAscii());

	// Write spx to Bitmaps first so getImageInfo finds it during init()
	images.saveSpxFile((char*)base.ToAscii(), buff);

	// Rebuild image under the real name (init() saves bmp internally)
	TNinode *n = (TNinode*)TheScene->parse_node(buff);
	if(n){
		BIT_ON(n->opts, CHANGED);
		BIT_OFF(n->opts, TMP);
		n->init();
		images.images.sort();
		delete n;
	}

	// Clean up working copy
	if(isTmp(m_name))
		images.removeAll((char*)m_name.ToAscii());
	m_name = base;
	setModified(false);
	makeImageList();
	makeRevertList();
	images.images.sort();
	m_file_menu->SetStringSelection(base);
	displayImage((char*)base.ToAscii());
	imageDialog->UpdateControls();
}

// ── Revert: reload from Bitmaps ─────────────────────────────

void VtxImageTabs::Revert(){
	wxString base = baseName(m_name);
	if(base.IsEmpty()) return;

	// Remove current working copy
	if(isTmp(m_name))
		images.removeAll((char*)m_name.ToAscii());

	// Reset to base name and clear modified flag
	m_name = base;
	setModified(false);

	// Reload display from disk
	displayImage((char*)base.ToAscii());

	Render.invalidate_textures();
	TheScene->set_changed_detail();
	TheScene->rebuild_all();
	imageDialog->UpdateControls();
}

// ── makeRevertList: snapshot of Bitmaps state ───────────────

void VtxImageTabs::makeRevertList(){
	char path[256];
	FileUtil::getBitmapsDir(path);
	wxDir dir(path);
	if(!dir.IsOpened()) return;
	LinkedList<ImageSym*> list;
	images.getImageInfo(type, list);
	freeRevertList();
	revert_list = new NameList<ImageSym*>(list);
}

// ── Clone: create a new image from the current working copy ──

bool VtxImageTabs::Clone(wxString new_name, bool rename){
	wxString base = baseName(m_name);
	wxString tmp  = isTmp(m_name) ? m_name : wxString("");

	// Build expression using current UI controls under the new name
	wxString istr = getImageString(new_name);
	char buff[512];
	strcpy(buff, istr.ToAscii());

	// Write spx to disk first so getImageInfo finds it during init()
	images.saveSpxFile((char*)new_name.ToAscii(), buff);

	// Build and save the image under the new name
	TNinode *n = (TNinode*)TheScene->parse_node(buff);
	if(!n) return false;
	BIT_ON(n->opts, CHANGED);
	BIT_OFF(n->opts, TMP);
	n->init();
	delete n;

	// Do all removals before sorting (removeAll corrupts sorted order)
	if(rename){
		if(!tmp.IsEmpty())
			images.removeAll((char*)tmp.ToAscii());
		images.removeAll((char*)base.ToAscii());
	} else {
		if(!tmp.IsEmpty())
			images.removeAll((char*)tmp.ToAscii());
	}

	// Removals may unsort — re-sort before makeImageList scans
	images.images.sort();

	m_name = new_name;
	setModified(false);

	makeImageList();
	makeRevertList();

	m_file_menu->SetStringSelection(new_name);
	displayImage((char*)new_name.ToAscii());
	imageDialog->UpdateControls();
	return true;
}
// ── New: clone current into new name ────────────────────────


// ── Rename: clone to new name + delete old ──────────────────


// ── Delete: remove files + select first remaining ────────────

void VtxImageTabs::Delete(){
	if(!canDelete()) return;
	wxString base = baseName(m_name);

	// Remove working copy and original files
	images.removeAll((char*)m_name.ToAscii());
	images.removeAll((char*)base.ToAscii());

	setImageName("");
	makeImageList();

	// Select and load first item in list
	if(m_file_menu->GetCount() > 0){
		m_file_menu->SetSelection(0);
		wxString first = m_file_menu->GetStringSelection();
		loadIntoWorkingCopy(first);
		displayImage((char*)m_name.ToAscii());
	}
	imageDialog->UpdateControls();
}

// ── Selection ────────────────────────────────────────────────

void VtxImageTabs::setSelection(wxString name){
	// Clean up any previous working copy
	if(!m_name.IsEmpty() && isTmp(m_name))
		images.removeAll((char*)m_name.ToAscii());

	// Set m_name to the base name — working copy created on first edit
	m_name = name;
	update_needed = false;

	// Update dropdown
	m_file_menu->SetStringSelection(name);

	// Display directly from the existing image/spx
	displayImage((char*)name.ToAscii());
	imageDialog->UpdateControls();
}

void VtxImageTabs::OnFileSelect(wxCommandEvent& event){
	wxString name = m_file_menu->GetStringSelection();
	setSelection(name);
}

// ── Can* flags ───────────────────────────────────────────────

bool VtxImageTabs::canDelete(){
	wxString base = baseName(m_name);
	Image *img = images.find((char*)base.ToAscii());
	return !img || !img->accessed();
}

bool VtxImageTabs::canSave()  { return true; }

bool VtxImageTabs::canRevert(){
	return isModified();
}

// ── Misc name accessors ──────────────────────────────────────

char *VtxImageTabs::getImageName()         { return m_name.mb_str(); }
void  VtxImageTabs::setImageName(char *s)  { m_name = wxString(s); }
void  VtxImageTabs::setImageName(wxString s){ m_name = s; }

// ── Event handlers ───────────────────────────────────────────

void VtxImageTabs::OnExprEdit(wxCommandEvent& event){
	update_needed = true;
	setModified(true);
	setObjAttributes();
}

void VtxImageTabs::OnChanged(wxCommandEvent& event){
	update_needed = true;
	setModified(true);
	setObjAttributes();
}

void VtxImageTabs::OnGradientMode(wxCommandEvent& event){
	update_needed = true;
	setModified(true);
	if(m_gradient_check->IsChecked())
		m_grays_check->SetValue(false);
	setObjAttributes();
}

void VtxImageTabs::OnGraysMode(wxCommandEvent& event){
	update_needed = true;
	setModified(true);
	if(m_grays_check->IsChecked())
		m_gradient_check->SetValue(false);
	setObjAttributes();
}

void VtxImageTabs::OnGradientSelect(wxCommandEvent& event){
	m_gradient_name = m_gradient_file_menu->GetStringSelection();
	m_gradient_image->setImage(wxString(m_gradient_name), m_gradient_image->TILE);
	last_gradient = m_gradient_name;
	if(m_gradient_check->IsChecked()){
		setModified(true);
		update_needed = true;
		imageDialog->UpdateControls();
		setObjAttributes();
	}
}

void VtxImageTabs::OnImageSize(wxCommandEvent& event){
	update_needed = true;
	setModified(true);
	setObjAttributes();
	TheView->set_changed_render();
}

void VtxImageTabs::OnShowTmps(wxCommandEvent& event){
	TNimage::show_tmps = m_show_tmps->GetValue();
	makeImageList();
	makeGradientsList();
}
