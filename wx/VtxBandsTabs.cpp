#include "VtxImageDialog.h"

#include "FileUtil.h"
#include "RenderOptions.h"

#include <wx/filefn.h>
#include <wx/dir.h>


//########################### VtxBandsTabs Class ########################
enum{
	ID_ENABLE,
	ID_DELETE,
    ID_RANDMIX,
    ID_PREV_COLORS,
    ID_NEXT_COLORS,
    ID_NORM,
    ID_INVERT,
    ID_CLAMP,
    ID_NEAREST,
    ID_REFLECT,
    ID_ALPHA,
	ID_FILELIST,
    ID_IMAGE_SIZE,
    ID_MIX_SLDR,
    ID_MIX_TEXT,
    ID_MIX_COLOR,
    ID_MOD_SLDR,
    ID_MOD_TEXT,
    ID_COLORS,
	ID_SHOW_TMPS,
};

#define VALUE1   60
#define CVALUE   40
#define CLABEL   15
#define CBOX     30
#define SLIDER1  100

#define CSLIDER  110-CBOX

#define BTNSIZE wxSize(50,25)

//########################### VtxBandsTabs Class ########################

#define COLOR_EVENT(i) \
EVT_COMMAND_SCROLL_THUMBRELEASE(ID_COLORS+i*4,VtxBandsTabs::OnEndSplineColorSlider) \
EVT_COMMAND_SCROLL(ID_COLORS+i*4,VtxBandsTabs::OnSplineColorSlider) \
EVT_TEXT_ENTER(ID_COLORS+i*4+1,VtxBandsTabs::OnSplineColorText) \
EVT_COLOURPICKER_CHANGED(ID_COLORS+i*4+2,VtxBandsTabs::OnSplineColorValue) \
EVT_CHECKBOX(ID_COLORS+i*4+3,VtxBandsTabs::OnSplineColorEnable)


IMPLEMENT_CLASS(VtxBandsTabs, wxPanel )

BEGIN_EVENT_TABLE(VtxBandsTabs, wxPanel)

EVT_CHECKBOX(ID_NORM,VtxBandsTabs::OnChanged)
EVT_CHECKBOX(ID_INVERT,VtxBandsTabs::OnChanged)
EVT_CHECKBOX(ID_CLAMP,VtxBandsTabs::OnChanged)
EVT_CHECKBOX(ID_ALPHA,VtxBandsTabs::OnChanged)
EVT_CHECKBOX(ID_NEAREST,VtxBandsTabs::OnChanged)
EVT_CHECKBOX(ID_REFLECT,VtxBandsTabs::OnChanged)
EVT_CHECKBOX(ID_SHOW_TMPS,VtxBandsTabs::OnShowTmps)

EVT_CHOICE(ID_FILELIST,VtxBandsTabs::OnFileSelect)
EVT_CHOICE(ID_IMAGE_SIZE, VtxBandsTabs::OnImageSize)
EVT_BUTTON(ID_NEXT_COLORS, VtxBandsTabs::OnNextColors)
EVT_BUTTON(ID_PREV_COLORS, VtxBandsTabs::OnPrevColors)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_MIX_SLDR,VtxBandsTabs::OnEndMixSlider)
EVT_COMMAND_SCROLL(ID_MIX_SLDR,VtxBandsTabs::OnMixSlider)
EVT_TEXT_ENTER(ID_MIX_TEXT,VtxBandsTabs::OnMixText)
EVT_COLOURPICKER_CHANGED(ID_MIX_COLOR,VtxBandsTabs::OnMixColor)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_MOD_SLDR,VtxBandsTabs::OnEndModSlider)
EVT_COMMAND_SCROLL(ID_MOD_SLDR,VtxBandsTabs::OnModSlider)
EVT_TEXT_ENTER(ID_MOD_TEXT,VtxBandsTabs::OnModText)

COLOR_EVENT(0)
COLOR_EVENT(1)
COLOR_EVENT(2)
COLOR_EVENT(3)
COLOR_EVENT(4)
COLOR_EVENT(5)
COLOR_EVENT(6)

COLOR_EVENT(7)
COLOR_EVENT(8)
COLOR_EVENT(9)
COLOR_EVENT(10)
COLOR_EVENT(11)
COLOR_EVENT(12)

END_EVENT_TABLE()

VtxBandsTabs::VtxBandsTabs(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
		: wxPanel(parent, id, pos, size,  style,name)
{
	   m_file_menu=0;
		AddBandsTab(this);
		update_needed=true;
		m_name="";
		image_list=0;
		revert_list=0;
		num_colors=0;
		color_page=0;
		type=BANDS|T1D|SPX;

}
void VtxBandsTabs::AddBandsTab(wxPanel *panel){
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(topSizer);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 0, wxALIGN_LEFT|wxALL, 5);

	// image file line

    wxBoxSizer *topline = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBoxSizer* fileio = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("File"));

	m_file_menu=new wxChoice(panel,ID_FILELIST,wxPoint(-1,4),wxSize(120,-1));
	m_file_menu->SetColumns(5);

	fileio->Add(m_file_menu,0,wxALIGN_LEFT|wxALL,2);

	wxString exps[]={"1","2","4","8","16","32","64","128","256","512","1024"};
	m_image_size=new wxChoice(panel, ID_IMAGE_SIZE, wxDefaultPosition,wxSize(60,-1),11, exps);

	m_image_size->SetSelection(8);

	hline->Add(m_image_size,0,wxALIGN_LEFT|wxALL,2);

	fileio->Add(hline,0,wxALIGN_LEFT|wxALL);

	topline->Add(fileio, 0, wxALIGN_LEFT|wxALL,0);
	wxSize size=fileio->GetMinSize();
	int w=size.GetWidth();
	int h=size.GetHeight();

    wxStaticBoxSizer* image_options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Options"));
    m_norm_check=new wxCheckBox(panel, ID_NORM, "Norm");
    image_options->Add(m_norm_check, 0, wxALIGN_LEFT|wxALL,5);
    m_invert_check=new wxCheckBox(panel, ID_INVERT, "Invert");
    image_options->Add(m_invert_check, 0, wxALIGN_LEFT|wxALL,5);

    m_show_tmps=new wxCheckBox(panel, ID_SHOW_TMPS, "Show Tmps");
    image_options->Add(m_show_tmps, 0, wxALIGN_LEFT|wxALL,5);

    image_options->SetMinSize(wxSize(TABS_WIDTH-w,h));

    topline->Add(image_options, 0, wxALIGN_LEFT|wxALL,0);
	boxSizer->Add(topline, 0, wxALIGN_LEFT|wxALL,0);

	// image bitmap display

	wxStaticBoxSizer* image_display = new wxStaticBoxSizer(wxVERTICAL,panel,wxT("Image"));
	m_image_window = new VtxImageWindow(this,wxID_ANY,wxDefaultPosition,wxSize(TABS_WIDTH-20,32));
	image_display->Add(m_image_window, 0, wxALIGN_LEFT|wxALL,2);

	image_display->SetMinSize(wxSize(TABS_WIDTH,-1));
	boxSizer->Add(image_display, 0, wxALIGN_LEFT|wxALL,0);

	// modulate line

	wxStaticBoxSizer* mod_colors = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Mix/Modulate"));

	mod_colors->Add(new wxStaticText(panel, -1, "Mix", wxDefaultPosition, wxSize(40,-1)),0,wxALIGN_LEFT|wxALL,8);
	m_mix_color=new ColorSlider(panel,ID_MIX_SLDR,"",0,CVALUE,CSLIDER,CBOX);
	mod_colors->Add(m_mix_color->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
	mod_colors->AddSpacer(2);
	mod_colors->Add(new wxStaticText(panel, -1, "Random", wxDefaultPosition, wxSize(60,-1)),0,wxALIGN_LEFT|wxALL,8);

	m_mod_slider=new SliderCtrl(panel,ID_MOD_SLDR,"",0, CVALUE,CSLIDER);
	m_mod_slider->setRange(0.0,1.0);
	mod_colors->Add(m_mod_slider->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
    //m_random_mix_check=new wxCheckBox(panel, ID_RANDMIX, "Random");
   // mod_colors->Add(m_random_mix_check, 0, wxALIGN_LEFT|wxALL,0);
	mod_colors->SetMinSize(wxSize(TABS_WIDTH,-1));
	boxSizer->Add(mod_colors, 0, wxALIGN_LEFT|wxALL,0);

	// colors

	spline_colors = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Colors"));
	spline_colors->SetMinSize(wxSize(TABS_WIDTH,-1));

	int n=0;
	for(int k=0;k<MAX_COLOR_PAGES;k++){
		color_pages[k]=new wxBoxSizer(wxHORIZONTAL);
		color_pages[k]->SetMinSize(wxSize(TABS_WIDTH-20,-1));
		for(int j=0;j<2;j++){
			wxBoxSizer *texline = new wxBoxSizer(wxVERTICAL);
			for(int i=0;i<MAX_PAGE_COLORS/2;i++,n++){
				wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
				hline->SetMinSize(wxSize(TABS_WIDTH/2-10,30));
				char id[16];
				sprintf(id,"%d",n+1);
				csliders[n]=new ColorSlider(panel,ID_COLORS+n*4,id,CLABEL,CVALUE,CSLIDER,CBOX);
				wxCheckBox *enable=new wxCheckBox(panel,ID_COLORS+n*4+3,"");
				enable->SetValue(true);
				enables[n]=enable;
				hline->Add(enable, 0, wxALIGN_LEFT|wxALL,0);
				hline->Add(csliders[n]->getSizer(), 0, wxALIGN_LEFT|wxALL,0);
				hline->SetItemMinSize(enable, 16, 20);
				texline->Add(hline, 0, wxALIGN_LEFT|wxALL,-1);
			}
			color_pages[k]->Add(texline, 0, wxALIGN_LEFT|wxALL);
			color_pages[k]->AddSpacer(10);
		}
		color_pages[k]->Show(false);
	}
	num_sliders=n;
	color_page=0;
	color_pages[color_page]->Show(true);
	spline_colors->Add(color_pages[0], 0, wxALIGN_LEFT|wxALL);
    boxSizer->Add(spline_colors, 0, wxALIGN_LEFT|wxALL,0);

	// color options

    h=50;
    wxStaticBoxSizer* color_options = new wxStaticBoxSizer(wxHORIZONTAL,panel,wxT("Color Options"));
    m_clamp_check=new wxCheckBox(panel, ID_CLAMP, "Clamp");
    color_options->Add(m_clamp_check, 0, wxALIGN_LEFT|wxALL,0);
    m_nearest_check=new wxCheckBox(panel, ID_NEAREST, "Nearest");
    color_options->Add(m_nearest_check, 0, wxALIGN_LEFT|wxALL,0);
    m_reflect_check=new wxCheckBox(panel, ID_REFLECT, "Reflect");
    color_options->Add(m_reflect_check, 0, wxALIGN_LEFT|wxALL,0);
    m_alpha_check=new wxCheckBox(panel, ID_ALPHA, "Alpha");
    color_options->Add(m_alpha_check, 0, wxALIGN_LEFT|wxALL,0);

    color_options->AddSpacer(10);

    m_next_colors = new wxButton(this,ID_NEXT_COLORS,">",wxDefaultPosition,BTNSIZE);
    color_options->Add(m_next_colors, 0, wxALIGN_LEFT|wxALL,0);
    m_prev_colors = new wxButton(this,ID_PREV_COLORS,"<",wxDefaultPosition,BTNSIZE);
    color_options->Add(m_prev_colors, 0, wxALIGN_LEFT|wxALL,0);

    m_prev_colors->Enable(false);
    color_options->SetMinSize(wxSize(TABS_WIDTH,h));
    boxSizer->Add(color_options, 0, wxALIGN_LEFT|wxALL,0);
}


//-------------------------------------------------------------
// VtxBandsTabs::updateControls() widget refresh handler
//-------------------------------------------------------------
void VtxBandsTabs::updateControls(){
	//if(update_needed){
		getObjAttributes();
	//}
}

//-------------------------------------------------------------
// VtxBandsTabs::Invalidate() force rebuild of file list
//-------------------------------------------------------------
void VtxBandsTabs::Invalidate(){
	update_needed=true;
	getObjAttributes();
	makeRevertList();
}

//-------------------------------------------------------------
// VtxBandsTabs::canRevert() return true if image can be deleted
// - delete denied if image is being used by a Texture
//-------------------------------------------------------------
bool VtxBandsTabs::canDelete()  {
	Image *img=images.find((char*)m_name.ToAscii());
	if(!img || (img && !img->accessed()))
		return true;
	return false;
}

//-------------------------------------------------------------
// VtxBandsTabs::canSave() return true if can save image
// - image not in revert list or
// - image has been edited after last save or load
//-------------------------------------------------------------
bool VtxBandsTabs::canSave()  {
	return true;
}

//-------------------------------------------------------------
// VtxBandsTabs::canRevert() return true if image can be reverted
// - image is in revert list and
// - image has been edited after last save or load
//-------------------------------------------------------------
bool VtxBandsTabs::canRevert(){
	if(!revert_list)
		return false;
	if(!isModified())
		return false;
	char *name=(char*)m_name.ToAscii();
	ImageSym *is=revert_list->inlist(name);
	if(is)
		return true;
	return false;
}

//-------------------------------------------------------------
// VtxBandsTabs::setModified() set modified flag
//-------------------------------------------------------------
void VtxBandsTabs::setModified(bool b){
	char *name=(char*)m_name.ToAscii();
	ImageSym *is=image_list->inlist(name);
	if(is){
		if(b)
			BIT_ON(is->info,CHANGED);
		else
			BIT_OFF(is->info,CHANGED);
	}
	Image *img=images.find(name);
	if(img)
		img->set_changed(b?1:0);
}

//-------------------------------------------------------------
// VtxBandsTabs::isModified() return modified flag
//-------------------------------------------------------------
bool VtxBandsTabs::isModified()  {
	if(!image_list)
		return false;
	char *name=(char*)m_name.ToAscii();
	ImageSym *is=image_list->inlist(name);
	if(is && (is->info&CHANGED))
		return true;
	return false;
}

//-------------------------------------------------------------
// VtxBandsTabs::freeImageList() free image symbol list
//-------------------------------------------------------------
void VtxBandsTabs::freeImageList() {
	if(image_list){
		image_list->free();
		delete image_list;
		image_list=0;
	}
}
//-------------------------------------------------------------
// VtxBandsTabs::freeRevertList() free image file list
//-------------------------------------------------------------
void VtxBandsTabs::freeRevertList() {
	if(revert_list){
		revert_list->free();
		delete revert_list;
		revert_list=0;
	}
}

//-------------------------------------------------------------
// VtxBandsTabs::makeImageList() build image file list
//-------------------------------------------------------------
void VtxBandsTabs::makeImageList(){
	cout<<"VtxBandsTabs::makeImageList()"<<endl;

    images.makeImagelist();

	LinkedList<ImageSym *> list;
	images.getImageInfo(type, list);
	freeImageList();
	image_list=new NameList<ImageSym*>(list);
	image_list->ss();
	
    m_file_menu->Clear();
	ImageSym *is;
	while((is=(*image_list)++)){
		if(!m_show_tmps->GetValue() && (is->info & TMP))
			continue;
		m_file_menu->Append(is->name());
	}
	int index=m_file_menu->FindString(m_name);
	if(index== wxNOT_FOUND){
		index=0;
	}
	if(m_file_menu->GetCount()>0){
		m_file_menu->SetSelection(index);
		m_name=m_file_menu->GetStringSelection();
	}
}

//-------------------------------------------------------------
// VtxBandsTabs::makeRevertList() build image revert list
//-------------------------------------------------------------
void VtxBandsTabs::makeRevertList(){
	char path[256];
	FileUtil::getBitmapsDir(path);

    wxDir dir(path);
    if (!dir.IsOpened() ){
     	return;
    }
	LinkedList<ImageSym *> list;
	images.getImageInfo(type, list);
	freeRevertList();
	revert_list=new NameList<ImageSym*>(list);
}

//-------------------------------------------------------------
// VtxBandsTabs::setObjAttributes() when switched out
//-------------------------------------------------------------
void VtxBandsTabs::setObjAttributes(){
	//if(!update_needed)
	//	return;
	wxString istr=getImageString(m_name);
	char buff[512];
	strcpy(buff,istr.ToAscii());
	//cout << buff << endl;
	TNinode *n=(TNinode*)TheScene->parse_node(buff);
	if(!n)
		return;
	n->init();
	delete n;

	m_image_window->setImage(m_name.ToAscii(),m_image_window->TILE);
	if(update_needed){
		TheScene->set_changed_detail();
		TheScene->rebuild_all();
		setModified(true);
	}
	else
		setModified(true);
	Render.invalidate_textures();
    TNbands::show_tmps=m_show_tmps->GetValue();
	update_needed=false;
	imageDialog->UpdateControls();
}

//-------------------------------------------------------------
// VtxBandsTabs::alpha() return true if colors has alpha member
//-------------------------------------------------------------
bool VtxBandsTabs::alpha()
{
	if(cmix.alpha())
		return true;
	Color *cp;
	colors.ss();
	bool aflag=false;
	while((cp=colors++)){
		if(cp->alpha()){
			aflag=true;
			break;
		}
	}
	return aflag;
}

//-------------------------------------------------------------
// VtxBandsTabs::splineString() get spline color string
//-------------------------------------------------------------
void VtxBandsTabs::splineString(char *buff)
{
	Node<Color*> *ptr=colors.ptr;
	Color *cp;

	colors.ss();
	int i=0;
	while((cp=colors++)){
		Color c=*cp;
		if(!m_alpha_check->GetValue())
			c.set_alpha(1.0);
		c.toString(buff+strlen(buff));
		i++;
		if(i<colors.size)
			strcat(buff,",");
		if(i%6==0)
			strcat(buff,"\n");
	}
	colors.ptr=ptr;
}

//-------------------------------------------------------------
// VtxBandsTabs::getImageString() return expr string
// base     bands       c  h  args
//-------------------------------------
// 1 color    list      n  0  Color,...
// 2 spline   none      n  h  h,[Color,...]
// 3 color    dark		1  h  h,delta,[Color]
// 4 spline   dark		n  h  h,delta,[Color,..]
// 5 spline	  dark,mix	n  h  h,delta,mix,cb,[Color,..]
//-------------------------------------------------------------
wxString VtxBandsTabs::getImageString(wxString name){
	char buff[4096];
	char opts_str[62]="";

	int opts=0;
	if(m_norm_check->GetValue())
		opts |= NORM;
	if(m_invert_check->GetValue())
		opts |= INVT;
	if(m_clamp_check->GetValue())
		opts |= CLAMP;
	if(m_nearest_check->GetValue())
		opts |= NEAREST;
	if(m_reflect_check->GetValue())
		opts |= REFLECT;

	if(m_alpha_check->GetValue())
		opts |= ACHNL;
	TNinode::optionString(opts_str,opts);

	double dmix=m_mix_color->getValue();
	double dmod=m_mod_slider->getValue();

	Color c=m_mix_color->getColor();
	if(m_alpha_check->GetValue())
		c.set_alpha(dmix);
	else
		c.set_alpha(1.0);

	int size=m_image_size->GetSelection();

	//if(size>0)
	//	size=1<<(size-1);
	size=1<<size;
	setColorsFromControls();
	sprintf(buff,"bands(\"%s\",%s",(const char*)name.ToAscii(),opts_str);
	if(size==0)								    // case 1
		splineString(buff+strlen(buff));
	else if(!dmod && !dmix){					// case 2
		sprintf(buff+strlen(buff),"%d,",size);
		splineString(buff+strlen(buff));
	}
	else{                                       // 3,4,5
		sprintf(buff+strlen(buff),"%d,%g",size,dmod);
		sprintf(buff+strlen(buff),",%g,",dmix);
		//if(m_random_mix_check->GetValue())
		//	sprintf(buff+strlen(buff),"RAND");
		//else
			c.toString(buff+strlen(buff));
		if(num_colors>0){                        // 5
			strcat(buff,",");
			splineString(buff+strlen(buff));
		}
	}
	strcat(buff,");\n");
	return wxString(buff);
}

//-------------------------------------------------------------
// VtxBandsTabs::setControlsFromColors() set controls from spline colors
//-------------------------------------------------------------
void VtxBandsTabs::setControlsFromColors()
{
	num_colors=colors.size;
	Color *cp;

	colors.ss();
	int i=0;
	for(i=0;i<MAX_COLORS;i++){
		csliders[i]->setColor(WHITE);
		enables[i]->SetValue(false);
	}
	i=0;
	while((cp=colors++)){
		Color c=*cp;
		csliders[i]->setColor(c);
		enables[i]->SetValue(true);
		i++;
		if(i>=MAX_COLORS)
			break;
	}
}
//-------------------------------------------------------------
// VtxBandsTabs::setColorsFromControls() set spline colors from controls
//-------------------------------------------------------------
void VtxBandsTabs::setColorsFromControls()
{
	num_colors=colors.size;
	colors.free();
	for(int i=0;i<MAX_COLORS;i++){
		if(enables[i]->GetValue()){
			Color c=csliders[i]->getColor();
			c.set_alpha(csliders[i]->getValue());
			colors.add(new Color(c));
		}
	}
}

//-------------------------------------------------------------
// VtxBandsTabs::displayImage() display image data
//-------------------------------------------------------------
void VtxBandsTabs::displayImage(char *name){

	ImageSym *is=0;
	if(name){
		is=image_list->inlist(name);
	}
	if(!is && image_list->size)
		is=image_list->base[0];
	if(!is)
		return;

	is=images.images.inlist(is->name());
	if(!is)
		return;
	//cout << is->istring << endl;

	NodeIF *n=TheScene->parse_node(is->istring);
    if(!n)
    	return;

	TNbands *inode=(TNbands*)n;

	inode->init();

	int opts=inode->opts;

    m_norm_check->SetValue((opts&NORM)?true:false);
    m_invert_check->SetValue((opts&INVT)?true:false);
    m_clamp_check->SetValue((opts&CLAMP)?true:false);
    m_nearest_check->SetValue((opts&NEAREST)?true:false);
    m_reflect_check->SetValue((opts&REFLECT)?true:false);

 	TNarg *arg=(TNarg*)inode->right;

	arg->eval();
	int size = 0;
	if(S0.svalid()){
		size=(int)S0.s;
		double logs=log2(size);
		m_image_size->SetSelection((int)logs);
	}
	else
		m_image_size->SetSelection(0);

	int i=0;
	int mflag=0;
	double mval=0;
	double dval=0;

	cmix=WHITE;
	arg=arg->next();
	colors.free();

	bool random_mix=false;
	while(arg && size){
		arg->eval();
		if(S0.cvalid()){
			if(mflag){
				cmix=S0.c;
				mflag=0;
			}
			else
				colors.add(new Color(S0.c));
		}
		else if(S0.s==RANDID){
			if(mflag){
				random_mix=true;
				cmix=Color(Rand(),Rand(),Rand());
				mflag=0;
			}
			else{
				colors.add(new Color(Rand(),Rand(),Rand(),Rand()));
			}
		}
		else if(i==0){
			dval=S0.s;
		}
		else {//if(S0.s){
			mval=S0.s;
			mflag=1;
		}
		arg=arg->next();
		i++;
	}
	m_mod_slider->setValue(dval);
	cmix.set_alpha(mval);
	m_mix_color->setColor(cmix);
	//m_random_mix_check->SetValue(random_mix);
	setControlsFromColors();
	m_alpha_check->SetValue(alpha());
	m_image_window->setImage(wxString(name),m_image_window->TILE);
 	delete inode;

}
//-------------------------------------------------------------
// VtxBandsTabs::getObjAttributes() called node when switched in
//-------------------------------------------------------------
void VtxBandsTabs::getObjAttributes(){
	//if(!update_needed)
	//	return;
	//cout<<"num tmps="<<TheScene->tmp_files<<endl;
	//m_show_tmps->SetValue(TNbands::show_tmps);
	m_show_tmps->SetValue(TNbands::show_tmps||TheScene->tmp_files>0);
	makeImageList();
	displayImage((char*)m_name.ToAscii());
}

//-------------------------------------------------------------
// VtxBandsTabs::makeNewImage() create a new image
//-------------------------------------------------------------
void VtxBandsTabs::makeNewImage(char *name, char *iexpr){
	TNinode *n=(TNinode*)TheScene->parse_node(iexpr);
	if(!n)
		return;
	n->init();
	delete n;
	displayImage(name);
}

//-------------------------------------------------------------
// VtxBandsTabs::OnChanged() generic handler attribute change event
//-------------------------------------------------------------
void VtxBandsTabs::OnChanged(wxCommandEvent& event){
	setObjAttributes();
}

//-------------------------------------------------------------
// VtxBandsTabs::OnShowTmps() handler for image show event
//-------------------------------------------------------------
void VtxBandsTabs::OnShowTmps(wxCommandEvent& event){
	TNbands::show_tmps=m_show_tmps->GetValue();
	makeImageList();
}
//-------------------------------------------------------------
// VtxBandsTabs::OnFileSelect() handler for image file select event
//-------------------------------------------------------------
void VtxBandsTabs::OnFileSelect(wxCommandEvent& event){
	m_name=m_file_menu->GetStringSelection();
    displayImage((char*)m_name.ToAscii());
	imageDialog->UpdateControls();
}

//-------------------------------------------------------------
// VtxBandsTabs::setSelection() select file name
//-------------------------------------------------------------
void VtxBandsTabs::setSelection(wxString name){
	m_file_menu->SetStringSelection(name);
	m_name=m_file_menu->GetStringSelection();
    displayImage((char*)m_name.ToAscii());
	imageDialog->UpdateControls();
}
//-------------------------------------------------------------
// VtxBandsTabs::OnImageSize() handler for image size select event
//-------------------------------------------------------------
void VtxBandsTabs::OnImageSize(wxCommandEvent& event){
	setObjAttributes();
	TheView->set_changed_render();
}

//-------------------------------------------------------------
// VtxBandsTabs::Save() handler for Save button press event
//-------------------------------------------------------------
void VtxBandsTabs::Save(){
	char *name=(char*)m_name.ToAscii();
	ImageSym *isc=image_list->inlist(name);
	if(!isc)
		return;
	ImageSym *isr=revert_list->inlist(name);
	ImageSym *isn=new ImageSym(isc);
	if(isr){
		revert_list->free(isr);
	}
	revert_list->add(isn);
	revert_list->sort();

	setModified(false);
	imageDialog->UpdateControls();
}

//-------------------------------------------------------------
// VtxBandsTabs::Revert() handler for Revert button press event
//-------------------------------------------------------------
void VtxBandsTabs::Revert(){
	if(!revert_list)
		return;
	char *name=(char*)m_name.ToAscii();
	ImageSym *is=revert_list->inlist(name);
	if(is && is->istring){
		makeNewImage(name, is->istring);
	}
	Render.invalidate_textures();
	TheScene->set_changed_detail();
	TheScene->rebuild_all();
	setModified(false);
	imageDialog->UpdateControls();
}

bool VtxBandsTabs::Clone(wxString new_name, bool rename){
	char *name=(char*)new_name.ToAscii();
	ImageSym *is=image_list->inlist((char*)new_name.ToAscii());
	if(is)
		return false; // name exists
	is=image_list->inlist((char*)m_name.ToAscii());
	if(is && is->istring){
		wxString iexpr=getImageString(new_name);
		TNinode *n=(TNinode*)TheScene->parse_node((char*)iexpr.ToAscii());
		if(!n)
			return false;
		if(rename)
			images.removeAll((char*)m_name.ToAscii());

		n->init();
		m_name=new_name;
		makeImageList();
		makeRevertList();
		displayImage((char*)m_name.ToAscii());
	    setSelection(name);
		return true;	
	}
	return false;	
}
bool VtxBandsTabs::New(wxString name){
	return Clone(name,false);	
}
bool VtxBandsTabs::Rename(wxString name){
	return Clone(name,true);
}

//-------------------------------------------------------------
// VtxBandsTabs::Delete() handler for Delete button press event
//-------------------------------------------------------------
void VtxBandsTabs::Delete(){
	if(canDelete()){
		images.removeAll((char*)m_name.ToAscii());
		m_name="";
		makeImageList();
		displayImage((char*)m_name.ToAscii());
		imageDialog->UpdateControls();
	}
}

//-------------------------------------------------------------
// VtxBandsTabs::OnNextColors() show next color page
//-------------------------------------------------------------
void VtxBandsTabs::OnNextColors(wxCommandEvent& event){
	if(color_page<MAX_COLOR_PAGES-1){
		spline_colors->Detach(color_pages[color_page]);
		color_pages[color_page]->Show(false);
		color_page++;
		color_pages[color_page]->Show(true);
		spline_colors->Add(color_pages[color_page]);
		spline_colors->Layout();
	}
	m_next_colors->Enable((color_page>=MAX_COLOR_PAGES-1)?false:true);
	m_prev_colors->Enable((color_page==0)?false:true);
}

//-------------------------------------------------------------
// VtxBandsTabs::OnNextColors() show prev color page
//-------------------------------------------------------------
void VtxBandsTabs::OnPrevColors(wxCommandEvent& event){
	if(color_page>0){
		spline_colors->Detach(color_pages[color_page]);
		color_pages[color_page]->Show(false);
		color_page--;
		color_pages[color_page]->Show(true);
		spline_colors->Add(color_pages[color_page]);
		spline_colors->Layout();
	}
	m_next_colors->Enable((color_page>=MAX_COLOR_PAGES-1)?false:true);
	m_prev_colors->Enable((color_page==0)?false:true);
}

void VtxBandsTabs::OnSetColors(wxCommandEvent& event){
	setObjAttributes();
}

void VtxBandsTabs::OnMixSlider(wxScrollEvent& event){
	m_mix_color->setValueFromSlider();
}
void VtxBandsTabs::OnEndMixSlider(wxScrollEvent& event){
	m_mix_color->setValueFromSlider();
	setObjAttributes();
}
void VtxBandsTabs::OnMixText(wxCommandEvent& event){
	m_mix_color->setValueFromText();
	setObjAttributes();
}
void VtxBandsTabs::OnMixColor(wxColourPickerEvent& event){
	setObjAttributes();
}

void VtxBandsTabs::OnModSlider(wxScrollEvent& event){
	m_mod_slider->setValueFromSlider();
}
void VtxBandsTabs::OnEndModSlider(wxScrollEvent& event){
	m_mod_slider->setValueFromSlider();
	setObjAttributes();
}
void VtxBandsTabs::OnModText(wxCommandEvent& event){
	m_mod_slider->setValueFromText();
	setObjAttributes();
}

void VtxBandsTabs::OnSplineColorSlider(wxScrollEvent& event){
	int i=(event.GetId()-ID_COLORS)/4;
	ColorSlider *s=csliders[i];
	s->setValueFromSlider();
	//setObjAttributes();
}

void VtxBandsTabs::OnEndSplineColorSlider(wxScrollEvent& event){
	int i=(event.GetId()-ID_COLORS)/4;
	ColorSlider *s=csliders[i];
	s->setValueFromSlider();
	setObjAttributes();
}

void VtxBandsTabs::OnSplineColorText(wxCommandEvent& event){
	int i=(event.GetId()-ID_COLORS)/4;
	ColorSlider *s=csliders[i];
	s->setValueFromText();
	setObjAttributes();
 }

void VtxBandsTabs::OnSplineColorValue(wxColourPickerEvent&event){
	setObjAttributes();
}
void VtxBandsTabs::OnSplineColorEnable(wxCommandEvent& event){
	setObjAttributes();
}

