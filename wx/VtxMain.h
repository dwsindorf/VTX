/////////////////////////////////////////////////////////////////////////////
// Name:        VtxMain.h
/////////////////////////////////////////////////////////////////////////////

#ifndef _VTX_MAIN_H_
#define _VTX_MAIN_H_

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/menu.h"
#include "wx/dcclient.h"

#define MAX_MAIN_ARGS 32
/* Define a new application type */
class VtxApp: public wxApp
{
public:
    bool OnInit();
};

#endif

