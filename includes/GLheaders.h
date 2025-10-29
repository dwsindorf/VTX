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
#include <stddef.h>
/*
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

*/

// CRITICAL: Include GLEW first, before any other GL headers!
#ifdef _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>
//#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#endif 
