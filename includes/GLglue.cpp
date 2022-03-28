// glcode.c

// Requisite headers

#include "GLglue.h"
#include <stdio.h>
#include "KeyIF.h"

#ifndef MFC // Window using MFC
#include <strings.h>
#include <string.h>
#endif
#include <iostream>
using namespace std;

double font_width=6;
double font_height=13;
double view_font_width=12;
double view_font_height=24;

#define FONT1 1000
#define FONT2 2000
#define FONT3 3000

int Font1=FONT1;
int Font2=FONT2;
int Font3=FONT3;

extern void mouse_down(int x, int y);
extern void resize_window(int x, int y);
//===============================================================
// common OGL or GLUT code
//===============================================================
#ifdef GLUT
#include <GL/glut.h>
extern "C" {
	void special_keys(int c, int x, int y);
	void standard_keys(unsigned char c, int x, int y);
	void window_event(int width, int height){
        resize_window(width,height);
	}
	void glut_mouse_event(int button, int state,int x, int y){
	    if(state==GLUT_DOWN && button==GLUT_LEFT_BUTTON)
	        mouse_down(x,y);
	}
}
#endif  // GLUT

int glerror()
{
	int n=glGetError();
	if(n == GL_NO_ERROR)
		return 0;
	return n;
}
#define SHOWERROR(x) case x: printf(#x "\n"); break;
void show_glerrors()
{
	int n=glGetError();
	while(n != GL_NO_ERROR){
		switch(n){
		SHOWERROR(GL_INVALID_ENUM);
		SHOWERROR(GL_INVALID_VALUE);
		SHOWERROR(GL_INVALID_OPERATION);
		SHOWERROR(GL_STACK_OVERFLOW);
		SHOWERROR(GL_STACK_UNDERFLOW);
		SHOWERROR(GL_OUT_OF_MEMORY);
		default: printf("OGL ERROR %x\n",n); break;
		}
		n=glerror();
	}
}

/* draw a string on the screen */ 
void draw_char(int c)
{
#ifdef GLUT
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
#else
	glCallLists(1,GL_UNSIGNED_BYTE,&c);
#endif
}

/* draw a string on the screen */ 
void draw_text(int x, int y, char *stg)
{
	int viewport[4];

	glGetIntegerv(GL_VIEWPORT,viewport);

	double vw=viewport[2];
	double vh=viewport[3];
	double yf=25.0/vh;
	double xf=20.0/vw;

	glRasterPos2d(x*xf-1, 1-(y+1)*yf); 
#ifdef GLUT
    for (int i = 0; i < strlen(stg); i++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, stg[i]);
    }
#else
	glCallLists(strlen(stg),GL_UNSIGNED_BYTE,stg);
#endif
}

//#ifndef GLUT
#ifdef WIN32
bool set_fixed_font()
{
#ifndef JNI	 
    HDC hDC;   
    HFONT hFont;
     LOGFONT logfont;

    logfont.lfHeight = -12;
    logfont.lfWidth = 0;
    logfont.lfEscapement = 0;
    logfont.lfOrientation = 0;
    logfont.lfWeight = FW_NORMAL;
    logfont.lfItalic = FALSE;
    logfont.lfUnderline = FALSE;
    logfont.lfStrikeOut = FALSE;
    logfont.lfCharSet = ANSI_CHARSET;
    logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily = FIXED_PITCH;
    strcpy(logfont.lfFaceName,"Courier");
    
    hDC = wglGetCurrentDC();
    hFont = CreateFontIndirect(&logfont);
    SelectObject (hDC, hFont); 
    wglUseFontBitmaps(hDC, 0, 255, FONT1);
    DeleteObject(hFont);
#endif
    return true;
}
bool set_select_font(){
	return set_fixed_font();
}
bool set_fonts(){
	return fixed_font()
}
#ifdef MFC
//===============================================================
// MFC glue code
//===============================================================

unsigned map_key(unsigned nChar) {
	BYTE keys[256];
	WORD ret[2];
	GetKeyboardState(keys);	
 	int skey=ToAscii(nChar,0,keys,ret,0);
	if(skey==1)
		return (unsigned)ret[0];
	switch(nChar){
	case VK_F1:		return KEY_F1;
	case VK_F2:		return KEY_F2;
	case VK_F3:		return KEY_F3;
	case VK_F4:		return KEY_F4;
	case VK_F5:		return KEY_F5;
	case VK_F6:		return KEY_F6;
	case VK_F7:		return KEY_F7;
	case VK_F8:		return KEY_F8;
	case VK_F9:		return KEY_F9;
	case VK_F10:	return KEY_F10;
	case VK_RIGHT:	return KEY_RIGHT;
	case VK_DOWN:	return KEY_BACKWARD;
	case VK_UP:	    return KEY_FORWARD;
	case VK_LEFT:	return KEY_LEFT;
	}
    return 0;
}

static HDC		sDC=0;
static HGLRC	sRC=0;

void screen_rect(RECT *wrect)
{
	if(sDC)
		DrawFocusRect(sDC,wrect);
}
void swap_buffers()
{
	if(sDC)
		SwapBuffers(sDC);
}

void set_context(HDC hDC,HGLRC hRC)
{
	VERIFY(wglMakeCurrent(hDC,hRC));
	sDC=hDC;
	if(hRC!=NULL)
	sRC=hRC;
}
void restore_context()
{
	VERIFY(wglMakeCurrent(sDC,sRC));
}

void clr_context()
{
	wglMakeCurrent(NULL,NULL);
	sDC=0;
	sRC=0;
}

void set_pixel_format(HDC hdc) 
{
	int nPixelFormat;					// Pixel format index
	
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// Size of this structure
		1,								// Version of this structure	
		PFD_DRAW_TO_WINDOW |			// Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |			// Support OpenGL calls in window
		PFD_DOUBLEBUFFER,				// Double buffered mode
		PFD_TYPE_RGBA,					// RGBA Color mode
		24,								// Want 24bit color 
		0,0,0,0,0,0,					// Not used to select mode
		0,0,							// Not used to select mode
		16,0,0,0,0,						// Accum buffer
		32,								// Size of depth buffer
		0,								// Not used to select mode
		0,								// Not used to select mode
		PFD_MAIN_PLANE,					// Draw in main plane
		0,								// Not used to select mode
		0,0,0 };						// Not used to select mode

	// Choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(hdc, &pfd);

	VERIFY(SetPixelFormat(hdc, nPixelFormat, &pfd));
}

void init_palette(HDC hdc, CPalette &palette)
	{
	PIXELFORMATDESCRIPTOR pfd;	// Pixel Format Descriptor
	LOGPALETTE *pPal;			// Pointer to memory for logical palette
	int nPixelFormat;			// Pixel format index
	int nColors;				// Number of entries in palette
	int i;						// Counting variable
	BYTE RedRange,GreenRange,BlueRange;
								// Range for each color entry (7,7,and 3)

	// Get the pixel format index and retrieve the pixel format description
	nPixelFormat = GetPixelFormat(hdc);
	DescribePixelFormat(hdc, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	// Does this pixel format require a palette?  If not, do not create a
	// palette and just return NULL
	if(!(pfd.dwFlags & PFD_NEED_PALETTE))
		return;

	// Number of entries in palette.  8 bits yeilds 256 entries
	nColors = 1 << pfd.cColorBits;	

	// Allocate space for a logical palette structure plus all the palette entries
	pPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) +nColors*sizeof(PALETTEENTRY));

	// Fill in palette header 
	pPal->palVersion = 0x300;		// Windows 3.0
	pPal->palNumEntries = nColors; // table size

	// Build mask of all 1's.  This creates a number represented by having
	// the low order x bits set, where x = pfd.cRedBits, pfd.cGreenBits, and
	// pfd.cBlueBits.  
	RedRange = (1 << pfd.cRedBits) -1;
	GreenRange = (1 << pfd.cGreenBits) - 1;
	BlueRange = (1 << pfd.cBlueBits) -1;

	// Loop through all the palette entries
	for(i = 0; i < nColors; i++)
		{
		// Fill in the 8-bit equivalents for each component
		pPal->palPalEntry[i].peRed = (i >> pfd.cRedShift) & RedRange;
		pPal->palPalEntry[i].peRed = (unsigned char)(
			(double) pPal->palPalEntry[i].peRed * 255.0 / RedRange);

		pPal->palPalEntry[i].peGreen = (i >> pfd.cGreenShift) & GreenRange;
		pPal->palPalEntry[i].peGreen = (unsigned char)(
			(double)pPal->palPalEntry[i].peGreen * 255.0 / GreenRange);

		pPal->palPalEntry[i].peBlue = (i >> pfd.cBlueShift) & BlueRange;
		pPal->palPalEntry[i].peBlue = (unsigned char)(
			(double)pPal->palPalEntry[i].peBlue * 255.0 / BlueRange);

		pPal->palPalEntry[i].peFlags = (unsigned char) NULL;
		}
		
	// Create the palette
	palette.CreatePalette(pPal);

	// Go ahead and select and realize the palette for this device context
	SelectPalette(hdc,(HPALETTE)palette,FALSE);
	RealizePalette(hdc);

	// Free the memory used for the logical palette structure
	free(pPal);
	}

int color_depth()
{
	PIXELFORMATDESCRIPTOR pfd;	// Pixel Format Descriptor
	int nPixelFormat;			// Pixel format index
	// Get the pixel format index and retrieve the pixel format description
	nPixelFormat = GetPixelFormat(sDC);
	DescribePixelFormat(sDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	return pfd.cColorBits;
}

BOOL query_palette(HDC hdc, CPalette &palette) 
{
	int nRet=0;
	// If the palette was created.
	if((HPALETTE)palette)
	{
		// Selects the palette into the current device context
		SelectPalette(hdc, (HPALETTE)palette, FALSE);

		// Map entries from the currently selected palette to
		// the system palette.  The return value is the number 
		// of palette entries modified.
		nRet = RealizePalette(hdc);
	}
	return nRet;
}

void changed_palette(HDC hdc, CPalette &palette) 
{
	SelectPalette(hdc,(HPALETTE)palette,FALSE);

		// Map entries to system palette
	RealizePalette(hdc);
				
	// Remap the current colors to the newly realized palette
	UpdateColors(hdc);
}


/*
 * 'DisplayErrorMessage()' - Display an error message dialog.
 */

void DisplayErrorMessage(const char *format,...)		/* I - Other arguments as necessary */
{
  va_list	ap;			/* Argument pointer */
  char		s[1024];		/* Output string */


  if (format == NULL)
    return;

  va_start(ap, format);
  vsprintf(s, format, ap);
  va_end(ap);

  MessageBeep(MB_ICONEXCLAMATION);
  MessageBox(NULL, s, "Error", MB_OK | MB_ICONEXCLAMATION);
}
void set_window_size(unsigned width, unsigned height)
{
    RECT r;
	AfxGetMainWnd()->GetWindowRect(&r);	
	r.right=r.left+width;
	r.bottom=r.top+height;
	AfxGetMainWnd()->MoveWindow(&r, TRUE );
}
#endif // end MFC code
//#endif
#else  // not WIN32
#ifndef GLUT
bool set_view_font()
{
    XFontStruct *fontInfo=0;  // font storage
    Display *dpy = XOpenDisplay(NULL); // default to DISPLAY env.
    char *font="12x24";
    fontInfo=XLoadQueryFont(dpy,font);
    if(fontInfo){
    	Font3=FONT3;
		glXUseXFont(fontInfo->fid,0,255, Font3);
    }
    else{
    	cout<< "couldn't load view font:"<<font<<endl;
    	Font3=FONT1;
    }
	 XCloseDisplay(dpy);
    return fontInfo?true:false;
}

bool set_select_font()
{
    XFontStruct *fontInfo=0;  // font storage   
    Display *dpy = XOpenDisplay(NULL); // default to DISPLAY env.
    char *font="9x15bold";
    fontInfo=XLoadQueryFont(dpy,font);
    if(fontInfo){
    	Font2=FONT2;
		glXUseXFont(fontInfo->fid,0,255, Font2);
    }
    else{
    	cout<< "couldn't load select font:"<<font<<endl;
    	Font2=FONT1;
    }
	 XCloseDisplay(dpy);
    return fontInfo?true:false;
}
bool set_fixed_font()
{
    XFontStruct *fontInfo=0;  // font storage
    Display *dpy = XOpenDisplay(NULL); // default to DISPLAY env.
    fontInfo=XLoadQueryFont(dpy,"fixed");
    if(fontInfo){
        glXUseXFont(fontInfo->fid,0,255, FONT1);
        font_height=fontInfo->ascent+fontInfo->descent;
        font_width=fontInfo->max_bounds.width;
        Font1=FONT1;
        //cout << "fixed font ht:"<< font_height << " width:"<<font_width<<endl;
   }
   XCloseDisplay(dpy);
   return fontInfo?true:false;
}
bool set_fonts(){
	set_fixed_font();
	set_select_font();
	set_view_font();
}
#endif // end WXWIN-GTK code
#endif // end WIN32 code
   
//===============================================================
// not MFC (UNIX/LINUX/WXWIN)
//===============================================================

#ifndef MFC

void restore_context()
{
}

int color_depth()
{
	return 24;
}

void screen_rect(RECT *wrect)
{
	// add code to draw a rectangle
}

/*
 * 'DisplayErrorMessage()' - Display an error message dialog.
 */

void DisplayErrorMessage(const char *c,...)		/* I - Other arguments as necessary */
{
	va_list argptr;
	va_start( argptr, c );
	char buff[256];

	vsprintf( buff, c, argptr );
    cout<<  buff << endl;	
	//printf("%s ",buff);
	va_end(argptr);
}

// required extern functions

void debug(int lvl, char *name)
{
}

#ifdef WXWIN // function is in VtxMain.cpp
extern void set_indicator(int n,const char *c, va_list argptr);
#else 
void set_indicator(int n,const char *c, va_list argptr)
{
	char buff[256];

	vsprintf( buff, c, argptr ); 	
    cout << buff<< endl;
}
#endif
void set_action(const char *c, ...)
{
	va_list argptr;
	va_start( argptr, c );
	set_indicator(0,c,argptr);
	va_end(argptr);
}
void set_status(const char *c, ...)
{
	va_list argptr;
	va_start( argptr, c );
	set_indicator(1,c,argptr);
	va_end(argptr);
}
void set_position(const char *c, ...)
{
	va_list argptr;
	va_start( argptr, c );
	set_indicator(2,c,argptr);
	va_end(argptr);
}
void set_info(const char *c, ...)
{
	va_list argptr;
	va_start( argptr, c );
	set_indicator(3,c,argptr);
	va_end(argptr);
}

//===============================================================
// GLUT
//===============================================================
#ifdef GLUT
static int lastkey;

int getkey() {
    int key=lastkey;
    lastkey=0;
    return key;
}

void standard_keys(unsigned char c, int x, int y){
    lastkey=c;
    //printf("key %c\n",c); 
}
void special_keys(int c, int x, int y){
	switch(c){
	case GLUT_KEY_UP:
  		lastkey=KEY_FORWARD;
  		break;
	case GLUT_KEY_DOWN:
  		lastkey=KEY_BACKWARD;
  		break;
	case GLUT_KEY_LEFT:
  		lastkey=KEY_LEFT;
  		break;
	case GLUT_KEY_RIGHT:
  		lastkey=KEY_RIGHT;
  		break;
	case GLUT_KEY_F1:
  		lastkey=KEY_F1;
  		break;
	case GLUT_KEY_F2:
  		lastkey=KEY_F2;
  		break;
	case GLUT_KEY_F3:
  		lastkey=KEY_F3;
  		break;
	case GLUT_KEY_F4:
  		lastkey=KEY_F4;
  		break;
	case GLUT_KEY_F5:
  		lastkey=KEY_F5;
  		break;
	case GLUT_KEY_F6:
  		lastkey=KEY_F6;
  		break;
	case GLUT_KEY_F7:
  		lastkey=KEY_F7;
  		break;
	case GLUT_KEY_F8:
  		lastkey=KEY_F8;
  		break;
	case GLUT_KEY_F9:
  		lastkey=KEY_F9;
  		break;
	case GLUT_KEY_F10:
  		lastkey=KEY_F10;
  		break;
  	default:
    	lastkey=c;
	}
}

static int window=0;

void free_display()
{ 
    glutDestroyWindow(window);
    printf("free display\n");
}

void init_display(unsigned width, unsigned height, int mode)
{ 
    char display[256];

    sprintf(display,"red=8 green=8 blue=8 double=1 rgb");
  	glutInitDisplayString(display);
	glutInitWindowSize(width, height);
  	glutInitWindowPosition(50,50);
  	window = glutCreateWindow("VTX");
  	glutSetWindow(window);
    glutKeyboardFunc(standard_keys);
    glutSpecialFunc(special_keys);
    glutMouseFunc(glut_mouse_event);
    glutReshapeFunc(window_event);
    
    printf("double_buffered=%d depth=%d\n",
       glutGet(GLUT_WINDOW_DOUBLEBUFFER),
       glutGet(GLUT_WINDOW_DEPTH_SIZE)
    );
}

void swap_buffers()
{
    glutSwapBuffers();
}
void set_window_size(unsigned w, unsigned h)
{   
    int x=glutGet(GLUT_WINDOW_X);
    int y=glutGet(GLUT_WINDOW_Y)-29; // fix border calc bug?
    int oldw=glutGet(GLUT_WINDOW_WIDTH);
    int oldh=glutGet(GLUT_WINDOW_HEIGHT);
    int dely=h-oldh;
    int newy=y-dely;
    glutPositionWindow(x,newy);
    glutReshapeWindow(w,h);
}

//===============================================================
// UNIX/LINUX - XWIN
//===============================================================
#else  // not glut
void display_state(){}
void free_display(){}
void set_window_size(unsigned w, unsigned h){  }
//void swap_buffers(){}
#endif  // XWIN
#endif  // XWIN
