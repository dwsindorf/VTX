//GLglue.h
// Declarations for external OpenGL module.  These functions are
// defined in glcode.c and are called appropriately by the CView
// derived classes.

#ifndef _GLGLUE
#define _GLGLUE

#include "GLheaders.h"
#include <stdarg.h>

extern int print_strings;
extern int draw_strings;
extern int getkey();
extern void swap_buffers();
extern int color_depth();
extern int glerror();
extern void show_glerrors();
extern void draw_text(int x, int y, char *msg);
extern void set_action(const char *c, ...);
extern void set_status(const char *c, ...);
extern void set_position(const char *c, ...);
extern void set_info(const char *c, ...);
extern void DisplayErrorMessage(const char *format,...);
extern void set_window_size(unsigned width, unsigned height);
extern void set_fixed_font();

enum {
    SMALL,
    MEDIUM,
    BIG,
    PAN,
    WIDE,
    CUSTOM
};
enum {
    SMALL_W = 480,  SMALL_H = 360, 
    MED_W   = 640,  MED_H   = 480, 
    BIG_W   = 800,  BIG_H   = 600,
    PAN_W   = 1600,  PAN_H   = 800,
    WIDE_W  = 1200, WIDE_H  = 600,
};
#define FRAME_BORDER 6
enum {
    USE_MAX_RGBA,
    USE_RGBA_16,
    USE_RGBA_8,
    USE_INDEX_8 
};
#define FONT1 1000
#ifdef WIN32     // windows 2000 XP
#ifdef WINDOWS   // mfc wgl
#include <afxwin.h>
void set_context(HDC hDC,HGLRC hRC);
void clr_context();
void set_pixel_format(HDC hDC);
void screen_rect(RECT *wrect);
void init_palette(HDC hdc, CPalette&);
BOOL query_palette(HDC hdc, CPalette&);
void changed_palette(HDC hdc, CPalette&);
void screen_rect(RECT *wrect);

#else   // standard windows libs
#include <windows.h>
#endif

#else // UNIX LINUX

typedef struct RECT {
    int top;
    int bottom;
    int left;
    int right;
} RECT;
#endif

#endif // Include/Define
