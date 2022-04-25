/*
 * VtxMovieDialog.cpp
 *
 *  Created on: Oct 7, 2020
 *      Author: dean
 */

#include "VtxMovieDialog.h"
#include "FileUtil.h"

#include "SceneClass.h"
#include "FrameClass.h"
#include "VtxScene.h"

// define all resource ids here
enum {
	ID_OK,
	ID_CLEAR,
	ID_REWIND,

	ID_RECORD_PATH,
	ID_LOAD_PATH,
	ID_PLAY_PATH,
	ID_SAVE_PATH,

	ID_RECORD_MOVIE,
	ID_SAVE_MOVIE,
	ID_TEST_MOVIE,
	ID_PLAY_MOVIE,

	ID_TEST_CHECK,
};

#define BTNSIZE wxSize(60,30)

//########################### VtxImageDialog Class Methods #############################
IMPLEMENT_CLASS(VtxMovieDialog, wxFrame)

void VtxMovieDialog::Stop(){
	TheScene->movie_stop();
	TheScene->view->clr_automv();
	TheScene->view->clr_autotm();

}
void VtxMovieDialog::OnClear(wxCommandEvent &event) {
	TheScene->views_rewind();
	TheScene->movie_clip();
	TheScene->movie->free();
	TheScene->delete_frames();
}

//TheScene->movie_open();
void VtxMovieDialog::OnRecordPath(wxCommandEvent &event) {
	bool recording=TheScene->recording();
	if(!recording)
		TheScene->movie_record();
	else
		Stop();
}

void VtxMovieDialog::OnPlayPath(wxCommandEvent &event) {
	if(!TheScene->playing()){
		TheScene->movie_rewind();
		TheScene->set_intrp(0);
		TheScene->movie_play();
	}
	else{
		Stop();
	}
}

void VtxMovieDialog::OnRecordMovie(wxCommandEvent &event) {
	bool recording=TheScene->jsave();
	if(!recording)
		TheScene->record_movie_from_path();
	else
		Stop();
}

void VtxMovieDialog::OnTestMovie(wxCommandEvent &event) {
	TheScene->movie_play_video();
}

void VtxMovieDialog::OnUpdateRecordPath(wxUpdateUIEvent &event) {
	bool recording=TheScene->recording();
	if(recording)
		m_record_path->SetBackgroundColour(m_pressed_color);
	else
		m_record_path->SetBackgroundColour(m_button_color);
	event.Check(recording);
}

void VtxMovieDialog::OnUpdateRecordMovie(wxUpdateUIEvent &event) {
	bool recording=TheScene->jsave();
	if(recording)
		m_record_movie->SetBackgroundColour(m_pressed_color);
	else
		m_record_movie->SetBackgroundColour(m_button_color);
	m_record_movie->Enable(TheScene->has_movie_path());
}

void VtxMovieDialog::OnUpdatePlayPath(wxUpdateUIEvent &event) {
	bool playing=TheScene->playing();
	if(playing)
		m_play_path->SetBackgroundColour(m_pressed_color);
	else
		m_play_path->SetBackgroundColour(m_button_color);

	m_play_path->Enable(TheScene->has_movie_path());
}

void VtxMovieDialog::OnRewind(wxCommandEvent &event) {
	TheScene->movie_rewind();
}

void VtxMovieDialog::OnUpdateTestMovie(wxUpdateUIEvent &event) {
	m_test_movie->Enable(TheScene->has_movie_frames());
}

void VtxMovieDialog::OnUpdateSaveMovie(wxUpdateUIEvent &event) {
	m_save_movie->Enable(TheScene->has_movie_frames());
}

void VtxMovieDialog::OnUpdateSavePath(wxUpdateUIEvent &event) {
	m_save_path->Enable(TheScene->has_movie_path());
}

void VtxMovieDialog::OnUpdateRewind(wxUpdateUIEvent &event) {
	m_rewind->Enable(TheScene->has_movie_path());
}


BEGIN_EVENT_TABLE( VtxMovieDialog, wxFrame )

EVT_CLOSE(VtxMovieDialog::OnClose)

EVT_BUTTON(wxID_OK, VtxMovieDialog::OnOk)
EVT_BUTTON(ID_CLEAR, VtxMovieDialog::OnClear)
EVT_BUTTON(ID_RECORD_PATH, VtxMovieDialog::OnRecordPath)
EVT_BUTTON(ID_REWIND, VtxMovieDialog::OnRewind)

EVT_BUTTON(ID_PLAY_PATH, VtxMovieDialog::OnPlayPath)
EVT_BUTTON(ID_SAVE_PATH, VtxMovieDialog::OnSavePath)
EVT_BUTTON(ID_LOAD_PATH, VtxMovieDialog::OnLoadPath)

EVT_BUTTON(ID_RECORD_MOVIE, VtxMovieDialog::OnRecordMovie)
EVT_BUTTON(ID_SAVE_MOVIE, VtxMovieDialog::OnSaveMovie)
EVT_BUTTON(ID_TEST_MOVIE, VtxMovieDialog::OnTestMovie)
EVT_BUTTON(ID_PLAY_MOVIE, VtxMovieDialog::OnPlayMovie)

EVT_UPDATE_UI(ID_RECORD_PATH,  VtxMovieDialog::OnUpdateRecordPath)
EVT_UPDATE_UI(ID_RECORD_MOVIE,  VtxMovieDialog::OnUpdateRecordMovie)
EVT_UPDATE_UI(ID_TEST_MOVIE,  VtxMovieDialog::OnUpdateTestMovie)
EVT_UPDATE_UI(ID_SAVE_MOVIE,  VtxMovieDialog::OnUpdateSaveMovie)
EVT_UPDATE_UI(ID_REWIND,  VtxMovieDialog::OnUpdateRewind)
EVT_UPDATE_UI(ID_PLAY_PATH,  VtxMovieDialog::OnUpdatePlayPath)
EVT_UPDATE_UI(ID_SAVE_PATH,  VtxMovieDialog::OnUpdateSavePath)


END_EVENT_TABLE()

VtxMovieDialog::VtxMovieDialog(wxWindow *parent, wxWindowID id,
		const wxString &caption, const wxPoint &pos, const wxSize &size,
		long style) {
	Create(parent, id, caption, pos, wxSize(DLG_WIDTH, DLG_HEIGHT), style);
}

void VtxMovieDialog::OnClose(wxCloseEvent &event) {
	event.Veto(true);
	Show(false);
}

bool VtxMovieDialog::Create(wxWindow *parent, wxWindowID id,
		const wxString &title, const wxPoint &pos, const wxSize &size,
		long style) {

	if (!wxFrame::Create(parent, id, wxT("Movie Editor"), pos, size,
			wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)))
		return false;

	char moviedir[MAXSTR];
	TheScene->make_movie_dir(moviedir);
	m_last_path=wxString(moviedir)+"/movie/movie.spx";
	m_last_movie=wxString(moviedir)+"/Videos/movie.mp4";

	m_last_ext==FILE_MP4;

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(boxSizer, 0, wxALIGN_LEFT | wxALL, 2);

	wxStaticBoxSizer *path = new wxStaticBoxSizer(wxHORIZONTAL, this,
			wxT("Path"));
	wxStaticBoxSizer *movie = new wxStaticBoxSizer(wxHORIZONTAL, this,
			wxT("Movie"));

	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	m_clear = new wxButton(this, ID_CLEAR, "Clear", wxDefaultPosition,
			BTNSIZE);
	m_load_path = new wxButton(this, ID_LOAD_PATH, "Load", wxDefaultPosition,
			BTNSIZE);

	m_record_path = new wxButton(this, ID_RECORD_PATH, "Record", wxDefaultPosition,
			BTNSIZE);
	m_rewind = new wxButton(this, ID_REWIND, "Rewind", wxDefaultPosition, BTNSIZE);
	m_play_path = new wxButton(this, ID_PLAY_PATH, "Play", wxDefaultPosition,
			BTNSIZE);
	m_save_path = new wxButton(this, ID_SAVE_PATH, "Save", wxDefaultPosition,
			BTNSIZE);

	hline->Add(m_clear, 0, wxALIGN_LEFT | wxALL, 0);
	hline->Add(m_load_path, 0, wxALIGN_LEFT | wxALL, 0);
	hline->Add(m_record_path, 0, wxALIGN_LEFT | wxALL, 0);

	hline->Add(m_rewind, 0, wxALIGN_LEFT | wxALL, 0);

	hline->Add(m_play_path, 0, wxALIGN_LEFT | wxALL, 0);
	hline->Add(m_save_path, 0, wxALIGN_LEFT | wxALL, 0);

	path->Add(hline, 0, wxALIGN_LEFT | wxALL);
	topSizer->Add(path, 0, wxALIGN_LEFT | wxALL);

	hline = new wxBoxSizer(wxHORIZONTAL);
	m_record_movie = new wxButton(this, ID_RECORD_MOVIE, "Record",
			wxDefaultPosition, BTNSIZE);
	m_test_movie = new wxButton(this, ID_TEST_MOVIE, "Test", wxDefaultPosition,
			BTNSIZE);
	m_save_movie = new wxButton(this, ID_SAVE_MOVIE, "Save", wxDefaultPosition,
			BTNSIZE);
	m_play_movie = new wxButton(this, ID_PLAY_MOVIE, "Play", wxDefaultPosition,
			BTNSIZE);

	hline->Add(m_record_movie, 0, wxALIGN_LEFT | wxALL, 0);
	hline->Add(m_test_movie, 0, wxALIGN_LEFT | wxALL, 0);
	hline->Add(m_save_movie, 0, wxALIGN_LEFT | wxALL, 0);
	hline->Add(m_play_movie, 0, wxALIGN_LEFT | wxALL, 0);

	movie->Add(hline, 0, wxALIGN_LEFT | wxALL);

	topSizer->Add(movie, 0, wxALIGN_LEFT | wxALL);

	SetSizerAndFit(topSizer);

	Centre();

	position = GetScreenPosition();
	Move(position.x + size.GetWidth() + 150, position.y);
	position = GetScreenPosition();
	m_button_color=m_play_movie->GetBackgroundColour();
	m_pressed_color=m_button_color.ChangeLightness(80);

	return true;
}

void VtxMovieDialog::setTitle() {
	wxString title("Movie Editor");
	SetTitle(title);

}
void VtxMovieDialog::OnOk(wxCommandEvent &event) {
	TheScene->set_movie_mode();
	TheScene->set_function(DFLT);
	TheScene->set_changed_render();
	Show(false);
}
bool VtxMovieDialog::Show(const bool b) {
	if (b)
		UpdateControls();
	else
		TheScene->set_function(DFLT);
	return wxFrame::Show(b);
}

void VtxMovieDialog::UpdateControls() {
	TheScene->set_movie_mode();
	TheScene->set_function(MOVIE);
	TheScene->set_changed_render();
	//if(TheScene->movie->size()==0)
	//	TheScene->movie_open();
}
void VtxMovieDialog::Invalidate() {
}

void VtxMovieDialog::OnLoadPath(wxCommandEvent &event) {
	char filename[256];
    char dir[256];
    filename[0]=0;
	File.getFilePath((char*)m_last_path.ToAscii(),dir);
	File.getFileName((char*)m_last_path.ToAscii(), filename);

    wxString ext("*");
    wxFileDialog dialog
                 (
                    this,
                    _T("Open Movie Path"),
                    wxString(dir),
                    wxString(filename),
					_T("*.spx"),
					wxFD_OPEN
                  );
    if (dialog.ShowModal() == wxID_OK) {
    	char path[256];
        strcpy(path, dialog.GetPath().ToAscii());
        File.getFilePath(path, dir);
        File.getFileName(path, filename);
        File.addToPath(dir,filename);
        m_last_path=wxString(dir);
    	vtxScene->open_scene(path);
    	TheScene->movie_open(path);
    	vtxScene->rebuild_all();
    }
}

void VtxMovieDialog::OnSavePath(wxCommandEvent &event) {
	char filename[256];
    char dir[256];
    filename[0]=0;
	File.getFilePath((char*)m_last_path.ToAscii(),dir);
	File.getFileName((char*)m_last_path.ToAscii(), filename);

    wxString ext("*");
    wxFileDialog dialog
                 (
                    this,
                    _T("Save Movie Path"),
                    wxString(dir),
                    wxString(filename),
					_T("*.spx"),
                    wxFD_SAVE
                  );
    if (dialog.ShowModal() == wxID_OK) {
    	strcpy(filename, dialog.GetFilename().ToAscii());
    	File.getFileName(filename,filename);
    	m_last_path=wxString(filename);
        strcpy(filename, dialog.GetPath().ToAscii());
        TheScene->movie_save(filename);
    }
}

wxWindow *VtxMovieDialog::test(wxWindow *w){
	wxPanel *panel = new wxPanel(w);
	wxBoxSizer *hline = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox *testbtn=new wxCheckBox(panel, ID_TEST_CHECK, "Exit at end");
	testbtn->SetValue(true);
	hline->Add(testbtn, 0, wxALIGN_LEFT | wxALL, 0);
	panel->SetSizerAndFit(hline);
    return panel;
}
void VtxMovieDialog::OnPlayMovie(wxCommandEvent &event) {
	char filename[256];
    char dir[256];
    filename[0]=0;
	File.getFilePath((char*)m_last_movie.ToAscii(),dir);
	File.getFileName((char*)m_last_movie.ToAscii(), filename);

    wxString ext("*");
    wxFileDialog dialog
                 (
                    this,
                    _T("Play Video"),
                    wxString(dir),
                    wxString(filename),
					_T("*.mp4;*.avi"),
					wxFD_OPEN
                  );
    dialog.SetExtraControlCreator(test);
    if (dialog.ShowModal() == wxID_OK) {
    	wxWindow *w=dialog.GetExtraControl();
    	wxCheckBox *testbtn=(wxCheckBox*)dialog.FindWindowById(ID_TEST_CHECK,w);
    	bool testval=testbtn->IsChecked();
        wxString cmd=wxString("ffplay -fast ");
        if(testval)
        	cmd+=wxString("-autoexit ");
        cmd+=dialog.GetPath().ToAscii();
        wxExecute(cmd);
    }
}

void VtxMovieDialog::OnSaveMovie(wxCommandEvent &event) {
	char filename[256];
    char dir[256];
    filename[0]=0;
	File.getFilePath((char*)m_last_movie.ToAscii(),dir);
	File.getFileName((char*)m_last_movie.ToAscii(), filename);

    wxString ext("*");
    ext+="mp4";
    wxFileDialog dialog
                 (
                    this,
                    _T("Save Video File"),
                    wxString(dir),
                    wxString(filename),
					_T("MPEG *.mp4|*.mp4|AVI *.avi|*.avi"),
                    wxFD_SAVE
                  );
    dialog.SetFilterIndex(m_last_ext==FILE_MP4?0:1);
    if (dialog.ShowModal() == wxID_OK) {
        strcpy(filename, dialog.GetFilename().ToAscii());
        File.getFileName(filename,filename);
        m_last_ext=dialog.GetFilterIndex()==0?FILE_MP4:FILE_AVI;
        m_last_movie=wxString(filename);
        ext=(m_last_ext==FILE_MP4)?wxString(".mp4"):wxString(".avi");
        wxString path=dialog.GetDirectory()+FileUtil::separator+filename+ext;
        wxString cmd=wxString("ffmpeg -y -i ")+wxString(dir)+wxString("/movie/frame_%05d.jpg")+wxString(" -c:v libx264 -crf 19 -framerate 20 ")+path;
        wxExecute(cmd);
    }
}
