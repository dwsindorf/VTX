#ifndef SCENEDIALOG_H_
#define SCENEDIALOG_H_

#include <wx/colour.h>

#include "VtxControls.h"
#include <wx/propdlg.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/notebook.h>

#include <wx/generic/propdlg.h>
#include <wx/xml/xml.h>

#include "SceneClass.h"
#include "VtxTabsMgr.h"

#ifndef MAXSTR
#define MAXSTR 64
#endif
class TreeDataNode : public wxTreeItemData
{
	char typestr[MAXSTR];
	char nodestr[2*MAXSTR];

public:
	TreeNode  *tnode;
	TreeDataNode(TreeNode *n){
		tnode=n;
		nodestr[0]=typestr[0]=0;
		strncpy(typestr,n->name(),MAXSTR);
		setLabel(n->label());
	}
	NodeIF *getObject()			{ return tnode->node;}
	void setObject(NodeIF *n)	{ tnode->node=n;}
	TreeNode *getSubTree()		{ return tnode;}
	void setSubTree(TreeNode *n){ tnode=n;}

	void setLabel(char *s) {
		if(s && strlen(s)>0)
			sprintf(nodestr,"%s<%s>",typestr,s);
		else
			strncpy(nodestr,typestr,MAXSTR);
	}
	char *getName() { return typestr;}
	char *getLabel() {
		return nodestr;
	}
};

WX_DECLARE_HASH_MAP( int, VtxTabsMgr*, wxIntegerHash, wxIntegerEqual, VtxTabsMgrHash );

class VtxSceneDialog : public wxFrame
{
	DECLARE_CLASS(VtxSceneDialog)

	bool rebuilding;
	bool copying;

	VtxTabsMgrHash   tabs;
	wxBoxSizer* topSizer;
	wxBoxSizer* rightSizer;
	wxBoxSizer* leftSizer;
	wxTreeCtrl *treepanel;

	TreeDataNode *drag_item;
	
	wxPoint position;

	int currentTabs;
	int lastTabs;

	TreeNode *selected;
	TreeNode *root;
	wxTreeItemId selectedId;

	ModelSym *sym;

	TreeNode *getData(TreeDataNode *);

	bool setTabs(TreeNode *new_select);

	bool setTabs(int t);
	void updateTabs();

	void addTabs();

	void setTitle();
	void updateControls();

	void updateObjectTree(wxTreeItemId root);
	void updateObjectTree();

	void showObjectTree(wxTreeItemId root, int lvl);
	void showObjectTree();

	void selectObject(wxTreeItemId root,NodeIF *n);
	void selectObject(NodeIF *n);

	void saveData();
	void restoreData();

	void OnTreeSelect(wxTreeEvent&event);
	void OnTreeMenuSelect(wxTreeEvent&event);
	void OnBeginDrag(wxTreeEvent&event);
	void OnBeginCopyDrag(wxTreeEvent&event);
	void OnEndDrag(wxTreeEvent&event);
	void OnTreeKey(wxTreeEvent&event);
	wxTreeItemId addToTree(wxTreeItemId pid, wxTreeItemId cid, TreeNode *vnode);
	wxTreeItemId addToTree(wxTreeItemId pid, int pos, TreeNode *vnode);
	wxTreeItemId addToTree(wxTreeItemId id, TreeNode *vnode);
	NodeIF *addToTree(NodeIF *obj, wxTreeItemId targetid);
	wxTreeItemId getAddPosition(wxTreeItemId pid, int type);

	wxTreeItemId getFirstChild(wxTreeItemId pid);
public:
	LinkedList<ModelSym*> add_list;
	LinkedList<ModelSym*> open_list;

	VtxSceneDialog(wxWindow *parent,
			wxWindowID id= wxID_ANY,
			const wxString &caption= wxT(""),
			const wxPoint &pos=wxDefaultPosition,
			const wxSize & size= wxDefaultSize,
			long style = wxCAPTION|wxSYSTEM_MENU
			);

	bool Create( wxWindow* parent,
			wxWindowID id = wxID_ANY,
			const wxString& caption = wxT(""),
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxCAPTION|wxSYSTEM_MENU );

	void rebuildObjectTree();
	void OnClose(wxCloseEvent& event);
	void reset();
	void deleteSelected();
	void removeMenuFile(int);
	void replaceSelected(NodeIF *newob);
	void saveSelected();
	void deleteItem(wxTreeItemId);
	void addChildToSelected(TreeNode *,TreeNode *, int pos);
	void addSiblingToSelected(TreeNode *,TreeNode *);
	void setNodeName(char *s);

	wxMenu *getFileMenu(ModelSym*,int &i);
	void setModelSym(ModelSym*);

	wxMenu *getOpenMenu(wxMenu &menu,NodeIF *);
	wxMenu *getRemoveMenu(NodeIF *);
	wxMenu *getAddMenu(NodeIF *);
	void AddFileMenu(wxMenu &menu,NodeIF *object);
	//--------- event handlers ------------------------------

    // implemented in cpp file

    bool Show(const bool show);
    void OnOk(wxCommandEvent &event);
    DECLARE_EVENT_TABLE()
};

extern VtxSceneDialog *sceneDialog;

#endif /*SCENEDIALOG_H_*/
