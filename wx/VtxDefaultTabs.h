#ifndef DFLTTABS_H_
#define DFTLTABS_H_


#include "VtxTabsMgr.h"
#include "SceneClass.h"

class VtxDefaultTabs : public VtxTabsMgr
{
	DECLARE_CLASS(VtxDefaultTabs)
protected:
	void AddDisplayTab(wxWindow *panel);

public:
	VtxDefaultTabs(wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxNotebookNameStr);

	bool Create(wxWindow* parent,
				wxWindowID id,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = 0,
				const wxString& name = wxNotebookNameStr);

	int showMenu(bool);

	DECLARE_EVENT_TABLE()
};


#endif /*SCENETABS_H_*/
