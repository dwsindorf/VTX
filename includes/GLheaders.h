// GLheaders.h
// Declarations for external OpenGL module.  These functions are
// defined in glcode.c and are called appropriately by the CView
// derived classes.
#ifndef _GLHEADERS
#define _GLHEADERS

//#ifdef MFC
//#include <afxwin.h>
//#else
#include <string.h>
//#endif
#ifdef GLEW
#include <GL/glew.h>
#if defined (WIN32) || defined(WIN7)
#include <GL/wglew.h>
#else
#include <GL/glxew.h>
#endif
#else
//#include <GLee.h>
#endif
#include <GL/glu.h>
#endif 
