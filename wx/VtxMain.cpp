/////////////////////////////////////////////////////////////////////////////
// Name:    wxmain.cpp
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

#include "GLglue.h"

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#include "VtxMain.h"
#include "VtxFrame.h"
#include "FileUtil.h"


static wxChar *sargs[256];
static int  nargs=0; 
static int repeat_rate=10;


// `Main program' equivalent, creating windows and returning main app frame
bool VtxApp::OnInit()
{
	char *infile=0;
    int h=MED_H;
    int w=MED_W;
    nargs=0;
    for(int i=0;i<argc;i++){	
       if (argv[i][0]=='-' && argv[i][1]=='-'){
    	    char token=(char)argv[i][2];
            switch(token){
            case 's':
                cout<< "VtxMain: window size small"<<endl;   	
                h=SMALL_H;
                w=SMALL_W;
                break;            	
            case 'b':
                cout<< "VtxMain: window size big"<<endl;   	
                h=BIG_H;
                w=BIG_W;
                break;
            case 'm':
                cout<< "VtxMain: window size med"<<endl;   	
                h=MED_H;
                w=MED_W;
                break;
            case 'w':
                cout<< "VtxMain: window size wide"<<endl;    
                h=WIDE_H;
                w=WIDE_W;
                break;
            case 'p':
                cout<< "VtxMain: window size pan"<<endl;    
                h=PAN_H;
                w=PAN_W;
                break;
            default:
                sargs[nargs++]=argv[i];
               // sargs[nargs++]=(wxChar*)argv[i].wx_str();
                break;
            }
        }
        else{
        	if(i>0 && argv[i][0]!='-')
        		//infile=(char*)argv[i].wx_str();
        	    infile=(char*)argv[i];
        	sargs[nargs++]=argv[i];
        	//sargs[nargs++]=(wxChar*)argv[i].wx_str();
        }	
    }
    // Create the main frame window
    VtxFrame *frame = new VtxFrame(NULL, wxT("VTX"),
        wxPoint(10,10), wxSize(w, h));

	VtxScene *vs=new VtxScene(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER);
    frame->SetCanvas(vs);
    vs->setargs(nargs,sargs);
    h+=FRAME_BORDER;
    w+=FRAME_BORDER;
    frame->SetClientSize(w,h);
    
    frame->Show(true);
     
    // wait until gl context stabilizes.
    // there seems to be a bug in wxWidgets where the first
    // call to OnPaint fails to do any gl drawing
    
    //wxSafeYield();
    //wxPaintDC dc(vs);
    vs->SetCurrent();
 
    set_fixed_font(); // set fixed font for opengl text
    
    wxString  filename("Startup Scene");
    if(infile){
    	char name[64];
    	File.getFileName(infile,name);
        filename=wxString(name);
    }
    
    vs->clear_canvas(filename);
    vs->SetFocus();
    //printf("long=%d int=%d\n",sizeof(long),sizeof(int));
    frame->start_timer(repeat_rate); // the first tick builds the "scene" object
    return true;
}

DECLARE_APP(VtxApp)
IMPLEMENT_APP(VtxApp)
