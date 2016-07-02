#include "VtxFunctMgr.h"

IMPLEMENT_CLASS( VtxFunctMgr, wxNotebook )

VtxFunctMgr::VtxFunctMgr(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size ,
		long style,
		const wxString& name)
{
	Show(false);
}
