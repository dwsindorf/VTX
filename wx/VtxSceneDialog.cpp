#include "wx/wxprec.h"
#include "wx/imagpng.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/clrpicker.h>
#include <wx/radiobox.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <wx/xml/xml.h>
#include <wx/gdicmn.h>
#include <malloc.h>

#include "VtxSceneDialog.h"
#include "OrbitalClass.h"
#include "UniverseModel.h"
#include "FileUtil.h"

#include "VtxSceneTabs.h"
#include "VtxPlanetTabs.h"
#include "VtxMoonTabs.h"
#include "VtxStarTabs.h"
#include "VtxCoronaTabs.h"
#include "VtxTexTabs.h"
#include "VtxDefaultTabs.h"
#include "VtxCloudsTabs.h"
#include "VtxColorTabs.h"
#include "VtxSkyTabs.h"
#include "VtxPointTabs.h"
#include "VtxGlossTabs.h"
#include "VtxLayerTabs.h"
#include "VtxMapTabs.h"
#include "VtxFogTabs.h"
#include "VtxWaterTabs.h"
#include "VtxSurfaceTabs.h"
#include "VtxRocksTabs.h"
#include "VtxRingTabs.h"
#include "VtxSnowTabs.h"
#include "VtxSystemTabs.h"
#include "VtxGalaxyTabs.h"
#include "VtxFractalTabs.h"
#include "VtxErodeTabs.h"
#include "VtxHaloTabs.h"
#include "VtxSpritesTabs.h"
#include "VtxPlantTabs.h"
#include "VtxBranchTabs.h"
#include "VtxLeafTabs.h"

#define PRINT_TREE

#define DEBUG_TREE_ACTIONS

#define TREE_WIDTH 250
#define PAGE_WIDTH TABS_WIDTH+5
#define TEST
// define all resource ids here
enum {
	ID_OK               = 0,
    ID_SAVE  			= 1,
    ID_RESTORE  		= 2,
    ID_TREE  	        = 3,
};

wxColourDatabase *colors;

//########################### VtxSceneDialog Class Methods #############################
IMPLEMENT_CLASS( VtxSceneDialog, wxFrame )

BEGIN_EVENT_TABLE( VtxSceneDialog, wxFrame )

EVT_CLOSE(VtxSceneDialog::OnClose)

EVT_TREE_SEL_CHANGED(ID_TREE, VtxSceneDialog::OnTreeSelect)

EVT_TREE_BEGIN_DRAG(ID_TREE, VtxSceneDialog::OnBeginDrag)
EVT_TREE_BEGIN_RDRAG(ID_TREE, VtxSceneDialog::OnBeginCopyDrag) // note: only works on windows
EVT_TREE_END_DRAG(ID_TREE, VtxSceneDialog::OnEndDrag)
//EVT_TREE_KEY_DOWN(ID_TREE, VtxSceneDialog::OnTreeKey)
EVT_BUTTON(wxID_OK, VtxSceneDialog::OnOk)
EVT_TREE_ITEM_MENU(ID_TREE, VtxSceneDialog::OnTreeMenuSelect)


END_EVENT_TABLE()

#define DLG_HEIGHT (TABS_HEIGHT+20)
#define DLG_WIDTH (TREE_WIDTH+TABS_WIDTH+100)

#define DLG_TOP 0
#define DLG_LEFT 0

VtxSceneDialog::VtxSceneDialog( wxWindow* parent,
  wxWindowID id, const wxString& caption,
  const wxPoint& pos, const wxSize& size, long style )
{
	colors=new wxColourDatabase();
	copying=false;

    Create(parent, id, caption, wxPoint(DLG_LEFT,DLG_TOP),wxSize(DLG_WIDTH,DLG_HEIGHT), style);
}

void VtxSceneDialog::OnClose(wxCloseEvent& event){
	event.Veto(true);
	Show(false);
}

bool VtxSceneDialog::Create( wxWindow* parent,
  wxWindowID id, const wxString& title,
  const wxPoint& pos, const wxSize& size, long style )
{

    if (!wxFrame::Create( parent, id, wxT("Properties"),
    		pos, size,  wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)))
        return false;
    rebuilding=true;
    topSizer = new wxBoxSizer(wxHORIZONTAL);

    rightSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->SetMinSize(wxSize(TREE_WIDTH,DLG_HEIGHT));
    rightSizer->SetMinSize(wxSize(PAGE_WIDTH,DLG_HEIGHT));

    topSizer->Add(leftSizer,0,wxALL|wxFIXED_MINSIZE ,0);
    topSizer->Add(rightSizer,0,wxALL|wxFIXED_MINSIZE ,0);

    treepanel=new wxTreeCtrl(this,ID_TREE,wxPoint(0,0),wxSize(TREE_WIDTH,TABS_HEIGHT),
	wxTR_HAS_BUTTONS|wxTR_SINGLE);

    leftSizer->Add(treepanel,0,wxALL|wxFIXED_MINSIZE);
    // note: couldn't get any VtxTabsMgr constructor except the one with the following arguments
    //       to work (get either a system crash on exit or compilation failures).
#define SET_TABS_OBJECT(ENUM,CLASS) \
	tabs[ENUM]=new CLASS(this,wxID_ANY,wxPoint(0,0),wxSize(PAGE_WIDTH,TABS_HEIGHT));

    SET_TABS_OBJECT(TN_SCENE,VtxSceneTabs)
    SET_TABS_OBJECT(TN_PLANET,VtxPlanetTabs)
	SET_TABS_OBJECT(TN_STAR,VtxStarTabs)
	SET_TABS_OBJECT(TN_MOON,VtxMoonTabs)
	SET_TABS_OBJECT(TN_CORONA,VtxCoronaTabs)
	SET_TABS_OBJECT(TN_HALO,VtxHaloTabs)
	SET_TABS_OBJECT(TN_CLOUDS,VtxCloudsTabs)
	SET_TABS_OBJECT(TN_SKY,VtxSkyTabs)
	SET_TABS_OBJECT(TN_TEXTURE,VtxTexTabs)
	SET_TABS_OBJECT(TN_SPRITE,VtxSpritesTabs)
	SET_TABS_OBJECT(TN_PLANT,VtxPlantTabs)
	SET_TABS_OBJECT(TN_PLANT_BRANCH,VtxBranchTabs)
	SET_TABS_OBJECT(TN_PLANT_LEAF,VtxLeafTabs)
	SET_TABS_OBJECT(TN_COLOR,VtxColorTabs)
	SET_TABS_OBJECT(TN_POINT,VtxPointTabs)
	SET_TABS_OBJECT(TN_GLOSS,VtxGlossTabs)
	SET_TABS_OBJECT(TN_LAYER,VtxLayerTabs)
	SET_TABS_OBJECT(TN_MAP,VtxMapTabs)
	SET_TABS_OBJECT(TN_FOG,VtxFogTabs)
	SET_TABS_OBJECT(TN_WATER,VtxWaterTabs)
	SET_TABS_OBJECT(TN_SURFACE,VtxSurfaceTabs)
	SET_TABS_OBJECT(TN_ROCKS,VtxRocksTabs)
	SET_TABS_OBJECT(TN_RING,VtxRingTabs)
	SET_TABS_OBJECT(TN_SNOW,VtxSnowTabs)
	SET_TABS_OBJECT(TN_SYSTEM,VtxSystemTabs)
	SET_TABS_OBJECT(TN_GALAXY,VtxGalaxyTabs)
	SET_TABS_OBJECT(TN_FCHNL,VtxFractalTabs)
	SET_TABS_OBJECT(TN_ERODE,VtxErodeTabs)

	currentTabs=TN_SCENE;
	lastTabs=TN_SCENE;

    wxColor col=tabs[TN_SCENE]->GetBackgroundColour();
    SetBackgroundColour(col);

    addTabs();
    //topSizer->SetMinSize(wxSize(DLG_WIDTH,DLG_HEIGHT));
    SetSizerAndFit(topSizer);

    topSizer->SetSizeHints(this);

    Centre();

    position=GetScreenPosition();
    Move(position.x+size.GetWidth()-100,position.y-100);
    //Move(position.x+300,position.y+460);
    position=GetScreenPosition();
    selected=0;
    root=0;
    //selected=TheScene->viewobj;
    updateTabs();

    return true;
}

void VtxSceneDialog::addTabs(){

	VtxTabsMgrHash::iterator it;
	for(it = tabs.begin(); it != tabs.end(); ++it){
		VtxTabsMgr *tab=it->second;
		tab->Show(false);
	}
	if(tabs.find(TN_SCENE) != tabs.end()){
		tabs[TN_SCENE]->Show(true);
		rightSizer->Add(tabs[TN_SCENE],0,wxALIGN_RIGHT|wxALL|wxFIXED_MINSIZE);
	}
}
void VtxSceneDialog::setTitle(){
	wxString title("Properties ");
	if(selected)
	    title+=selected->name();
	SetTitle(title);

}
bool VtxSceneDialog::Show(const bool b){
	if(b)
		Move(position.x,position.y);
	else{
		position=GetScreenPosition();
		// for some reason can't restore position after a move unless
		// we change some position attribute manually
		position.x-=1;
	}
	if(b)
		updateControls();
	return wxFrame::Show(b);
}

void VtxSceneDialog::OnOk(wxCommandEvent& event){
	Show(false);
}


TreeNode *VtxSceneDialog::getData(TreeDataNode *node){
	if(!node)
		return 0;
	return node->tnode;
}

//-------------------------------------------------------------
// VtxSceneDialog::OnTreeSelect() event handler for popup menu select
//-------------------------------------------------------------
void VtxSceneDialog::OnTreeMenuSelect(wxTreeEvent&event){
	wxMouseState state=::wxGetMouseState();
	if(state.ControlDown())
		return;
    //cout<<"VtxSceneDialog::OnTreeMenuSelect"<<endl;
	wxTreeItemId id=event.GetItem();

	TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(id);
	TreeNode *data=getData(node);

	TheScene->suspend();
	bool expanded=treepanel->IsExpanded(id);
	bool empty=!treepanel->ItemHasChildren(id);
	int type=data->getFlag(TN_TYPES);

	if(tabs.find(type) == tabs.end()){
		TheScene->unsuspend();
		return;
	}

	VtxTabsMgr* mgr=tabs[type];

	NodeIF *newobj=0;
	NodeIF *obj=mgr->getObject();
	if(!obj){
		TheScene->unsuspend();
		return;
	}
	if(expanded  && !empty)
		obj->set_expanded();
	else
		obj->set_collapsed();

	int menu_code=mgr->showMenu(expanded||empty);
	int menu_id=menu_code&TABS_ID_MASK;
	int menu_choice=menu_code&TABS_ACTION_MASK;

	char sbuff[1024];
	ModelSym* sym=0;
	bool rand_flag=false;
	if((menu_choice & TABS_ADD) || (menu_choice & TABS_REPLACE)){
		bool rand=false;
		char sbuff[1024];
		LinkedList<ModelSym*> *reflist=0;
		if(menu_choice & TABS_ADD)
			reflist=&add_list;
		else
			reflist=&replace_list;
		LinkedList<ModelSym*> &list=*reflist;
		sym=list[menu_id];
		wxString name(sym->name());
		if(!sym->isFile() && name == "<Random>"){
			LinkedList<ModelSym*>flist;
			TheScene->model->getFileList(sym->value,flist);
			int ival=std::rand() % flist.size;
			sym=flist[ival];
			rand_flag=true;
		}

		if(sym){
			sbuff[0]=0;
			if(menu_choice & TABS_REPLACE)
				TheScene->model->setActionMode(Model::REPLACING);
			else
				TheScene->model->setActionMode(Model::ADDING);

			if(sym->isFile()){
				TheScene->model->getFullPath(sym,sbuff);
				newobj=TheScene->open_node(obj,sbuff);
			}			
			else if( name == "Simple")
				newobj=TheScene->getPrototype(obj,sym->value);
			else
				newobj=TheScene->makeObject(obj,sym->value);
		}
	}
	switch(menu_choice){
	default:
		cout<<"ERROR UNSUPPORTED MENU CHOICE:"<<menu_choice<<endl;
		break;
	case TABS_NONE:
	case TABS_ENABLE:
		break;
	case TABS_VIEWOBJ:
		 updateObjectTree();
		 TheScene->set_changed_detail();
		break;
	case TABS_SAVE:
		saveSelected();
		break;
	case TABS_DEFAULT:
		obj->setRandom(false);
		obj->setDefault();
		obj->invalidate();
        TheScene->rebuild();
		break;
	case TABS_RANDOMIZE:
		obj->setRandom(true);
		obj->randomize();
        TheScene->rebuild();
		break;
	case TABS_REMOVE:
		removeMenuFile(menu_id);
		break;
	case TABS_REPLACE:
		if(newobj){
#ifdef DEBUG_TREE_ACTIONS
			cout<<"replace in tree "<<newobj<<endl;
#endif
			replaceSelected(newobj);
		}
		break;
	case TABS_DELETE:
		{
			newobj=deleteSelected();
			TheScene->regroup();
			TheScene->rebuild_all();
			rebuildObjectTree();
			if(newobj)
				selectObject(newobj);
		}
		break;
	case TABS_ADD:
		if(newobj){
			newobj->setRandom(rand_flag);
#ifdef DEBUG_TREE_ACTIONS
			cout<<"add to tree "<<newobj<<endl;
#endif
			newobj=addToTree(newobj,id);
			if(newobj->newViewObj()){
				ObjectNode *obj=newobj;
				TheScene->set_viewobj(obj);
				obj->init_view();
			}
			TheScene->regroup();
	        obj->invalidate();
	        TheScene->rebuild_all();
	        rebuildObjectTree();
	        selectObject(newobj);
	       // setNodeName(sym->name());
		}
		break;
	}
	add_list.free();
	replace_list.free();
	TheScene->unsuspend();
}

void VtxSceneDialog::updateTabs(){
	if(selected && currentTabs>=0){
		if(tabs.find(currentTabs) == tabs.end()){
			return;
		}
		if(!TheScene->automv()&& !TheScene->autotm())
			tabs[currentTabs]->setSelected(selected);
		tabs[currentTabs]->updateControls();
	}
}

bool VtxSceneDialog::setTabs(TreeNode *new_select){
	int new_type=new_select->getFlag(TN_TYPES);
	selected=new_select;
	setTabs(new_type);
	return true;
}

//-------------------------------------------------------------
// VtxSceneDialog::removeMenuFile() remove menu item from file system
//-------------------------------------------------------------
void VtxSceneDialog::removeMenuFile(int menu_id){
	ModelSym* sym=replace_list[menu_id+3]; // skip over simple and random
	if(sym->isFile()){
		char sbuff[1024];
		sbuff[0]=0;
		TheScene->model->getFullPath(sym,sbuff);
		::wxRemoveFile(sbuff);
	}
}
//-------------------------------------------------------------
// VtxSceneDialog::OnTreeSelect() event handler for tree node select
//-------------------------------------------------------------
void VtxSceneDialog::OnTreeSelect(wxTreeEvent&event){
	if(rebuilding)
		return;
	setSelected(event.GetItem());
	TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(selectedId);
	setTabs(node->tnode);
	setTitle();
}

//-------------------------------------------------------------
// VtxSceneDialog::OnEndDrag() event handler for start of lft-mouse drag
//-------------------------------------------------------------
void VtxSceneDialog::OnBeginDrag(wxTreeEvent&event){
	//cout << "begin move drag " << event.GetKeyEvent().ControlDown() << endl;
	wxMouseState state=::wxGetMouseState();
	if(state.ControlDown()){
		cout << "begin copy drag " << event.GetKeyEvent().ControlDown() << endl;
		copying=true;
	}
	else{
		cout << "begin move drag " << event.GetKeyEvent().ControlDown() << endl;
		copying=false;
	}

	setSelected(event.GetItem());
	event.Allow();
}
//-------------------------------------------------------------
// VtxSceneDialog::OnEndDrag() event handler for start of rt-mouse drag
// note: on GTX, a rt-mouse drag event gets preempted by a rt-mouse
//       down event which opens the pop-up menu instead on initializing
//       a copy-drag. Need to have the ctrl button down when copying to avoid
//       the problem.
//-------------------------------------------------------------
void VtxSceneDialog::OnBeginCopyDrag(wxTreeEvent&event){
#ifdef DEBUG_TREE_ACTIONS
	cout << "begin rt-mouse copy drag " << event.GetKeyEvent().ControlDown() << endl;
#endif
	setSelected(event.GetItem());
	copying=true;
	event.Allow();
}

//-------------------------------------------------------------
// VtxSceneDialog::OnEndDrag() event handler for end of drag
//-------------------------------------------------------------
void VtxSceneDialog::OnEndDrag(wxTreeEvent&event){
	wxTreeItemId dstId=event.GetItem();
	wxTreeItemId srcId=selectedId;
    TreeDataNode *dst_node=(TreeDataNode*)treepanel->GetItemData(dstId);
    TreeDataNode *src_node=(TreeDataNode*)treepanel->GetItemData(srcId);
    if(!dst_node||!src_node)
    	return;
    cout<<"VtxSceneDialog::OnEndDrag"<<endl;
	LinkedList<ModelSym*> list;
	NodeIF *dst=dst_node->getObject();
	NodeIF *src=src_node->getObject();
	if(!dst || !src)
		return;

	bool expanded=treepanel->IsExpanded(dstId);
	bool empty=!treepanel->ItemHasChildren(dstId);
	if(expanded  && !empty)
		dst->set_expanded();
	else
		dst->set_collapsed();
	TheScene->model->setActionMode(Model::DROPPING);
	TheScene->model->getAddList(dst,list);
	list.ss();
	ModelSym *sym=0;
	wxString sstr=src_node->getLabel();
	char sname[256];
	strcpy(sname,src_node->tnode->node->nodeName());
	wxString dstr=dst_node->getName();
	int type=0;
	while((sym=list++)){
		if(sym->value==src->getFlag(TN_TYPES)){
			type=sym->value;
			break;
		}
	}
	if(type){
#ifdef DEBUG_TREE_ACTIONS
		cout << "Ok to drop " << sstr << " at " << dstr << endl;
#endif
		char path[256];
	    File.getBaseDirectory(path);
	    File.addToPath(path,File.system);
	    File.addToPath(path,"drag_item.spx");
		TheScene->save_node(src,path);

		NodeIF *newobj=TheScene->open_node(dst,path);
		//wxTreeItemId itemId=addToTree(newobj, dstId);
		newobj=addToTree(newobj, dstId);

		//cout << "drop " << event.GetKeyEvent().ControlDown() << endl;
		if(!copying)
			deleteItem(srcId);
	    TheScene->set_changed_detail();
		//if(newobj->typeValue() & ID_OBJECT){
			TheScene->regroup();
	        TheScene->rebuild_all();
		//}
		    rebuildObjectTree();

		selectObject(newobj);
		setNodeName(sname);
	}
#ifdef DEBUG_TREE_ACTIONS
	else
		cout << "Can't drop " << sstr << " at " << dstr << endl;
#endif
    //treepanel->SelectItem(dstId);
	copying=false;
    list.free();
}
//-------------------------------------------------------------
// VtxSceneDialog::OnTreeKey() event handler for key down in tree
//-------------------------------------------------------------
void VtxSceneDialog::OnTreeKey(wxTreeEvent&event){
	// doesn't seem to catch control down event on drag
	//cout << "OnTreeKey "<< event.GetKeyCode() << endl;
	event.Skip();
}

//-------------------------------------------------------------
// VtxSceneDialog::addToTree() insert an NodeIF object into the tree
//-------------------------------------------------------------
NodeIF *VtxSceneDialog::addToTree(NodeIF *newObj, wxTreeItemId dstId) {
	int num_children=treepanel->GetChildrenCount(dstId, false);
	bool expanded=treepanel->IsExpanded(dstId);
	bool empty=!treepanel->ItemHasChildren(dstId);

	TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(dstId);
	TreeNode *tnode=node->tnode;
	TreeNode *tree_node;
	NodeIF *dstObj=tnode->node;
#ifdef DEBUG_TREE_ACTIONS	
	cout<<"add proto valid:"<<newObj->protoValid()<<" "<<newObj<<endl;
#endif
	wxTreeItemId itemId=dstId;
	if (expanded || (empty && !dstObj->isLeaf())) {
		if (num_children) {
			wxTreeItemId first=getFirstChild(dstId);
			wxTreeItemId addId=getAddPosition(dstId, newObj->typeLevel());
			node=(TreeDataNode*)treepanel->GetItemData(addId);
			TreeNode *data=getData(node);
			NodeIF *obj=node->getObject();
			if (addId==first) { // first child > new child or one child
				if(dstObj->typeLevel()>=newObj->typeLevel()){ // 2nd child < first : insert
					newObj=dstObj->addAfter(0, newObj);
#ifdef DEBUG_TREE_ACTIONS
					cout<<"2nd child < first : insert "<<newObj->typeName()<<" after "<<dstObj->typeName()<<endl;
#endif
					tree_node=TheScene->model->addToTree(tnode, 0, newObj);
					TheScene->setPrototype(dstObj, newObj);
					addToTree(dstId, 0, tree_node);
				}
				else{ // add new child after last child with lower level
#ifdef DEBUG_TREE_ACTIONS
					cout<<"add new child after last child with lower level"<<endl;
					cout<<"parent="<<dstObj->typeName()<<" newobj="<<newObj->typeName()<<" childobj="<<obj->typeName()<<endl;
#endif
					newObj=dstObj->addAfter(obj, newObj);
					tree_node=TheScene->model->addToTree(tnode, data, newObj);
					TheScene->setPrototype(dstObj, newObj);
					addToTree(dstId, addId, tree_node);
				}
			} else { // insert before first child with a higher level
#ifdef DEBUG_TREE_ACTIONS
				cout<<"insert before first child with a higher level"<<endl;
#endif
				newObj=dstObj->addAfter(obj, newObj);
				tree_node=TheScene->model->addToTree(tnode, data, newObj);
				TheScene->setPrototype(dstObj, newObj);
				addToTree(dstId, addId, tree_node);
			}
		} else { // no previous children: insert as first child
#ifdef DEBUG_TREE_ACTIONS
			cout<<"no previous children: insert as first child"<<endl;
#endif
			newObj=dstObj->addChild(newObj);
			tree_node=TheScene->model->addToTree(tnode, newObj);
			TheScene->setPrototype(dstObj, newObj);
			addToTree(dstId, tree_node);
		}
	} else {
#ifdef DEBUG_TREE_ACTIONS
		cout<<"insert new child in parent branch after the selected child"<<endl;
#endif
		// insert new child in parent branch after the selected child,
		// usually a sibling of the same type (e.g. texture, layer)
		wxTreeItemId pid=treepanel->GetItemParent(dstId);
		TreeDataNode *pdata=(TreeDataNode*)treepanel->GetItemData(pid);
		TreeNode *tree_parent=pdata->getSubTree();
		NodeIF *parobj=pdata->getObject();

		newObj=parobj->addAfter(dstObj, newObj);

		// make a TreeNode from newobj insert it in the TreeNode tree
		tree_node=TheScene->model->addToTree(tree_parent, tnode, newObj);
#ifdef DEBUG_TREE_ACTIONS
		cout<<"add 2 proto valid:"<<newObj->protoValid()<<endl;
#endif
		TheScene->setPrototype(tree_parent->node, newObj);
		// add new node to wxTreeCntrl
		addToTree(pid, dstId, tree_node);
	}
	return newObj;
}

//-------------------------------------------------------------
// VtxSceneDialog::setTabs() action handler for tree node select
//-------------------------------------------------------------
bool VtxSceneDialog::setTabs(int t){
	if(tabs.find(t) == tabs.end()){
		cout << "dialog for class id : "<< t << " not yet supported" <<endl;
		return false;
	}
#ifdef DEBUG_TREE_ACTIONS
	cout<<"VtxSceneDialog::setTabs "<<selected->name()<<" "<<selected->node<<endl;
#endif
	VtxTabsMgr* mgr=tabs[t];
	mgr->setSelected(selected);
	//mgr->updateControls();
	if(t != currentTabs){
		if(lastTabs>=0){
			rightSizer->Detach(0);
			if(tabs.find(lastTabs) != tabs.end())
				tabs[lastTabs]->Show(false);
		}
		mgr->Show(true);
		rightSizer->Add(mgr,0,wxALIGN_LEFT|wxALL);
		lastTabs=t;
		currentTabs=t;
		rightSizer->Layout();
	}
	updateTabs();
	return true;
}

//-------------------------------------------------------------
// VtxSceneDialog::updateControls() refresh dialog GUI
//-------------------------------------------------------------
void VtxSceneDialog::updateControls() {
	updateTabs();
	updateObjectTree();
	setTitle();
	Refresh();
}

//-------------------------------------------------------------
// VtxSceneDialog::addToTree() append an item to the tree
//-------------------------------------------------------------
wxTreeItemId VtxSceneDialog::addToTree(wxTreeItemId id, TreeNode *obj){
	wxTreeItemId itemId=id;
	if(obj->isShowing()){
		TreeDataNode *data=new TreeDataNode(obj);
		itemId=treepanel->AppendItem(id,wxString(data->getLabel()),-1,-1,data);
		if(TheScene->viewobj==obj->node)
			setSelected(itemId);
	}
	for(int j=0;j<obj->numChildren();j++){
		addToTree(itemId,(TreeNode*)obj->children[j]);
	}
	return itemId;
}

//-------------------------------------------------------------
// VtxSceneDialog::addToTree() add an item to the tree at pos
//-------------------------------------------------------------
wxTreeItemId VtxSceneDialog::addToTree(wxTreeItemId pid, int pos, TreeNode *obj){
	wxTreeItemId itemId=pid;
	if(obj->isShowing()){
		TreeDataNode *data=new TreeDataNode(obj);
		itemId=treepanel->InsertItem(pid, pos, wxString(data->getLabel()),-1,-1,data);
	}
	for(int j=0;j<obj->numChildren();j++){
		addToTree(itemId,(TreeNode*)obj->children[j]);
	}
	return itemId;
}

//-------------------------------------------------------------
// VtxSceneDialog::addToTree() add an item to the tree at pos
//-------------------------------------------------------------
wxTreeItemId VtxSceneDialog::addToTree(wxTreeItemId pid, wxTreeItemId cid, TreeNode *obj){
	wxTreeItemId itemId=cid;
	if(obj->isShowing()){
		TreeDataNode *data=new TreeDataNode(obj);
		itemId=treepanel->InsertItem(pid, cid, wxString(data->getLabel()),-1,-1,data);
	}
	for(int j=0;j<obj->numChildren();j++){
		addToTree(itemId,(TreeNode*)obj->children[j]);
	}
	return itemId;
}

//-------------------------------------------------------------
// VtxSceneDialog::getFirstChild() get first child in branch
//-------------------------------------------------------------
wxTreeItemId VtxSceneDialog::getFirstChild(wxTreeItemId pid){
	wxTreeItemIdValue cookie;
	return treepanel->GetFirstChild(pid, cookie);
}
//-------------------------------------------------------------
// VtxSceneDialog::getAddPosition() get insert position for new node
//-------------------------------------------------------------
wxTreeItemId VtxSceneDialog::getAddPosition(wxTreeItemId pid, int level){
	wxTreeItemIdValue cookie;
	wxTreeItemId item = treepanel->GetFirstChild(pid, cookie);
	wxTreeItemId next = item;
	wxTreeItemId last = item;
	int i=0;
	while(next.IsOk()){
		TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(next);
		NodeIF *obj=node->getObject();
		int obj_level=obj->typeLevel();
		if(obj_level>0 && level <=obj_level){
			return last;
		}
		i++;
		last = next;
		next = treepanel->GetNextChild(pid, cookie);
	}
	return last;
}

//-------------------------------------------------------------
// VtxSceneDialog::deleteItem() remove node
//-------------------------------------------------------------
NodeIF *VtxSceneDialog::deleteItem(wxTreeItemId item){
    TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(item);
    NodeIF *obj=node->getObject();
    NodeIF *p=obj->getParent();
    obj=obj->removeNode();
    if(obj)
    	delete obj;
    treepanel->Delete(item);
    return p;
}
//-------------------------------------------------------------
// VtxSceneDialog::deleteSelected() delete selected node
//-------------------------------------------------------------
NodeIF *VtxSceneDialog::deleteSelected(){
	wxTreeItemId item=treepanel->GetSelection();
    wxTreeItemId parent=treepanel->GetItemParent(item);
    NodeIF *p=deleteItem(item);
    //treepanel->SelectItem(parent);
    //updateObjectTree();
    return p;
}
//-------------------------------------------------------------
// VtxSceneDialog::replaceSelected() replace selected node
//-------------------------------------------------------------
void VtxSceneDialog::replaceSelected(NodeIF *newobj){
	TheScene->model->setActionMode(Model::REPLACING);
	wxTreeItemId item=treepanel->GetSelection();
	bool expanded=treepanel->IsExpanded(item);
    TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(item);
    NodeIF *oldobj=node->getObject();
#ifdef DEBUG_TREE_ACTIONS
	cout<<" newobj:"<<newobj->typeName()<<" oldobj:"<<oldobj->typeName()<<endl;
#endif
	bool vo=(oldobj==TheScene->viewobj);
	if(vo)
	   TheScene->viewobj=(ObjectNode *)newobj;  	
    newobj=oldobj->replaceNode(newobj);
    if(!newobj)
    	return;
    int type=newobj->getFlag(TN_TYPES);
	TheScene->regroup();

    TheScene->rebuild_all();
    rebuildObjectTree();

    selectObject(newobj);
    item=treepanel->GetSelection();
    //node=(TreeDataNode*)treepanel->GetItemData(item);
    //node->setObject(newobj);

	if(expanded)
		treepanel->Expand(item);
    //setTabs(type);
}

//#define DEBUG_TREE

//-------------------------------------------------------------
// VtxSceneDialog::rebuildObjectTree()
// - regenerate the tree widget
//-------------------------------------------------------------
void VtxSceneDialog::rebuildObjectTree(){
	TheScene->suspend();
	cout<<"rebuildObjectTree start"<<endl;
	rebuilding=true;
	treepanel->DeleteAllItems();
	if(root)
		TheScene->freeTree(root);

	root=TheScene->buildTree();
#ifdef DEBUG_TREE
	TheScene->printTree(root,"ntest.tmp");
#endif

	TreeDataNode *data=new TreeDataNode(root);
	wxTreeItemId itemId=treepanel->AddRoot(data->getLabel(),-1,-1,data);
	//selectedId=itemId;
	for(int j=0;j<root->numChildren();j++){
		addToTree(itemId,(TreeNode*)root->children[j]);
	}
#ifdef PRINT_TREE
	showObjectTree();
#endif
	rebuilding=false;
	selected=root;
	currentTabs=-1;
	//setTabs(selected->getFlag(TN_TYPES));
	//treepanel->CollapseAll();
	cout<<"rebuildObjectTree 1"<<endl;
	//setSelected(selectedId);
	treepanel->SelectItem(selectedId);
	cout<<"rebuildObjectTree 2"<<endl;

	treepanel->EnsureVisible(selectedId);

    TheScene->set_changed_detail();
	TheScene->unsuspend();
	//TheScene->rebuild();
    cout<<"rebuildObjectTree end"<<endl;

}

//-------------------------------------------------------------
// VtxSceneDialog::updateObjectTree() recursive tree annotation update
//-------------------------------------------------------------
void VtxSceneDialog::updateObjectTree(){
	updateObjectTree(treepanel->GetRootItem());
}
void VtxSceneDialog::updateObjectTree(wxTreeItemId root)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId item = treepanel->GetFirstChild( root, cookie );
	wxTreeItemId child;

	while(item.IsOk()){
		TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(item);
		if(!node)
			continue;
		TreeNode *data=node->tnode;
		if(!node)
			continue;
		if(data->isObject()) {
			if(node->getObject()==TheScene->viewobj)
				treepanel->SetItemTextColour(item,colors->Find("BLUE"));
			else if(data->isEnabled())
				treepanel->SetItemTextColour(item,colors->Find("BLACK"));
			else
				treepanel->SetItemTextColour(item,colors->Find("LIGHT GREY"));
		}
		else {
			if(data->isEnabled())
				treepanel->SetItemTextColour(item,colors->Find("BLACK"));
			else
				treepanel->SetItemTextColour(item,colors->Find("LIGHT GREY"));
		}
		if(treepanel->ItemHasChildren(item))
			updateObjectTree(item);
		item = treepanel->GetNextChild(root, cookie);
	}
}

//-------------------------------------------------------------
// VtxSceneDialog::selectObject() find and select object in tree
//-------------------------------------------------------------
void VtxSceneDialog::selectObject(NodeIF *n){
	selectObject(treepanel->GetRootItem(),n);
}

void VtxSceneDialog::setSelected(wxTreeItemId id){
	TreeDataNode *tdnode=(TreeDataNode*)treepanel->GetItemData(id);
	TreeNode  *tnode=tdnode->tnode;
	NodeIF *node=tnode->node;
	cout<<"setSelected:"<< node->typeName()<<endl;
	selectedId=id;
}
void VtxSceneDialog::VtxSceneDialog::selectObject(wxTreeItemId parent,NodeIF *n){
	wxTreeItemIdValue cookie;
	wxTreeItemId child;
	TreeDataNode *tdnode=(TreeDataNode*)treepanel->GetItemData(parent);
	TreeNode  *tnode=tdnode->tnode;
	NodeIF *node=tnode->node;

	if(n==node){
		setSelected(parent);
#ifdef DEBUG_TREE_ACTIONS
		cout << node->typeName();
		if(strlen(tnode->label()))
			cout <<"<"<<tnode->label() <<">";
		cout<<endl;
#endif
		treepanel->SelectItem(selectedId);
		treepanel->EnsureVisible(selectedId);
		return;

	}
	wxTreeItemId item = treepanel->GetFirstChild( parent, cookie );

	while(item.IsOk()){
		selectObject(item,n);
		item = treepanel->GetNextChild(parent, cookie);
	}

}

//-------------------------------------------------------------
// VtxSceneDialog::showObjectTree() recursive tree update
//-------------------------------------------------------------

void VtxSceneDialog::showObjectTree(){
	showObjectTree(treepanel->GetRootItem(),0);
}
void VtxSceneDialog::showObjectTree(wxTreeItemId parent, int lvl){
	wxTreeItemIdValue cookie;
	wxTreeItemId child;

#ifdef PRINT_TREE
	wxString tabs("");
	for(int i=0;i<lvl;i++)
		tabs+="    ";
	TreeDataNode *tdnode=(TreeDataNode*)treepanel->GetItemData(parent);
	TreeNode  *tnode=tdnode->tnode;
	NodeIF *node=tnode->node;
	cout << tabs << node->typeName();
	if(strlen(tnode->label()))
		cout <<"<"<<tnode->label() <<">";
	cout<<endl;

#endif
	wxTreeItemId item = treepanel->GetFirstChild( parent, cookie );

	while(item.IsOk()){
		showObjectTree(item,lvl+1);
		item = treepanel->GetNextChild(parent, cookie);
	}
}
//-------------------------------------------------------------
// VtxSceneDialog::setNodeName() change tree node label
//-------------------------------------------------------------
void VtxSceneDialog::setNodeName(char *s){
	TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(selectedId);
	node->setLabel(s);
	node->tnode->node->setName(s);
	treepanel->SetItemText(selectedId,node->getLabel());
}

//-------------------------------------------------------------
// VtxSceneDialog::saveSelected() save selected node
//-------------------------------------------------------------
void VtxSceneDialog::saveSelected(){
	// following line prevents an annoying popup error even after a successful save
	wxLogNull logNo;  // disable errors
	char path[512];
	char filename[64];
    char dir[512];
    filename[0]=0;
	path[0]=0;
    wxTreeItemId item = treepanel->GetSelection();
    TreeDataNode *node=(TreeDataNode*)treepanel->GetItemData(item);
    NodeIF *obj=node->getObject();
    ModelSym* sym=UniverseModel::getObjectSymbol(obj->getFlag(TN_TYPES));

    char *name=obj->getName();
    if(name && strlen(name)>0)
    	strcpy(sym->name(),name);
	TheScene->model->getFullPath(sym,path);

	File.getFilePath(path,dir);
	File.getFileName(path, filename);

    wxString ext("*");
    ext+=FileUtil::ext;
	
    wxFileDialog dialog
                 (
                    this,
                    _T("Save Object"),
                    wxString(dir),
                    wxString(filename),
                    ext,
                    wxFD_SAVE
                  );
    if (dialog.ShowModal() == wxID_OK) {
        strcpy(filename, dialog.GetFilename().ToAscii());

        File.getFileName(filename,filename);
        sprintf(path,"%s%s%s.spx",dir,"/",filename);

        if(obj->typeValue() & ID_TERRAIN)
        	TheScene->set_keep_variables(0);
        setNodeName(filename);
        TheScene->save_node(obj,path);
        TheScene->set_keep_variables(1);
    }
}

//-------------------------------------------------------------
// VtxSceneDialog::getFileMenu()
//-------------------------------------------------------------
wxMenu *VtxSceneDialog::getFileMenu(ModelSym *sym,int &i){
	wxMenu *submenu=new wxMenu();
	ModelSym *fsym;
 	LinkedList<ModelSym*>tlist;

	submenu->Append(TABS_ADD|i++,"Simple");
	
	TheScene->model->getTypeList(sym->value,tlist);
	if(tlist.size){
		tlist.ss();
		wxMenu *tmenu=new wxMenu();
		while(fsym=tlist++){
			fsym->value|=sym->value;
			tmenu->Append(TABS_ADD|i++,fsym->name());
			add_list.add(fsym);
		}
		submenu->AppendSubMenu(tmenu,"Generate");
		//tlist.free();
	}
	else 
		submenu->Append(TABS_ADD|i++,"Generate");

 	LinkedList<ModelSym*>flist;
	TheScene->model->getFileList(sym->value,flist);
	if(flist.size>0){
		submenu->AppendSeparator();
		flist.ss();
		add_list.add(new ModelSym("<Random>",sym->value));
		submenu->Append(TABS_ADD|i++,"<Random>");
		while((fsym=flist++)){
			add_list.add(fsym);
			submenu->Append(TABS_ADD|i++,fsym->name());
		}
		//flist.free();
	}
	return submenu;
}
//-------------------------------------------------------------
// VtxSceneDialog::getOpenMenu()
//-------------------------------------------------------------
wxMenu *VtxSceneDialog::getReplaceMenu(wxMenu &menu,NodeIF *obj){
	wxMenu *submenu=new wxMenu();
	ModelSym *fsym;
	menu.Append(TABS_SAVE,wxT("Save.."));
	menu.AppendSeparator();
	menu.Append(TABS_RANDOMIZE, wxT("Randomize"));
	menu.Append(TABS_DEFAULT, wxT("Default"));
	menu.AppendSeparator();
	int i=0;
	int type=obj->getFlag(TN_TYPES);
	
	cout<<"getReplaceMenu:"<<obj->typeName()<<endl;
	
 	replace_list.free();
	ModelSym* sym=UniverseModel::getObjectSymbol(type);
	replace_list.add(sym);
	submenu->Append(TABS_REPLACE|i++,"Simple");
	
	LinkedList<ModelSym*>tlist;
	TheScene->model->getTypeList(type,tlist);
	if(tlist.size){
		tlist.ss();
		wxMenu *typemenu=new wxMenu();
		while(fsym=tlist++){
			fsym->value|=sym->value;
			replace_list.add(fsym);
			typemenu->Append(TABS_REPLACE|i++,fsym->name());
		}
		submenu->AppendSubMenu(typemenu,"Generate");
	}
	else
		replace_list.add(new ModelSym("Generate",type));
	submenu->AppendSeparator();

	LinkedList<ModelSym*>flist;
	TheScene->model->getFileList(type,flist);

	if(flist.size!=0){
		replace_list.ss();
		flist.ss();
		replace_list.add(new ModelSym("<Random>",type));
		submenu->Append(TABS_REPLACE|i++,"<Random>");

		while((fsym=flist++)){
			replace_list.add(fsym);
			submenu->Append(TABS_REPLACE|i++,fsym->name());
		}
	}
	return submenu;
}
//-------------------------------------------------------------
// VtxSceneDialog::getRemoveMenu()
//-------------------------------------------------------------
wxMenu *VtxSceneDialog::getRemoveMenu(NodeIF *obj){
	int i=0;
	replace_list.ss();
	wxMenu *submenu=new wxMenu();
	ModelSym *fsym;
	// skip over simple, complex and random options
	replace_list++;
	replace_list++;
	replace_list++;

	while((fsym=replace_list++)){
		submenu->Append(TABS_REMOVE|i++,fsym->name());
	}
	replace_list.ss();
	return submenu;
}

//-------------------------------------------------------------
// VtxSceneDialog::getAddMenu()
// - add a submenu that contains a list of add objects
//-------------------------------------------------------------
wxMenu *VtxSceneDialog::getAddMenu(NodeIF *obj){
	LinkedList<ModelSym*> dlist;
	add_list.free();
	TheScene->model->setActionMode(Model::ADDING);
	TheScene->model->getAddList(obj,dlist);
	add_list.ss();
	dlist.ss();
	ModelSym *sym;
	wxMenu *addmenu=new wxMenu();
	wxMenu *submenu;
	int i=0;
	while((sym=dlist++)){
		add_list.add(sym);
		wxString label=sym->name();
		submenu=getFileMenu(sym,i);
		if(submenu)
			addmenu->AppendSubMenu(submenu,label);
		else
			addmenu->Append(TABS_ADD|i++,label);
	}
	if(i==0){
		delete addmenu;
		return 0;
	}
	return addmenu;
}
//-------------------------------------------------------------
// VtxSceneDialog::AddFileMenu()
//-------------------------------------------------------------
void VtxSceneDialog::AddFileMenu(wxMenu &menu,NodeIF *object){
	wxMenu *open_menu=getReplaceMenu(menu,object);
	LinkedList<ModelSym*>flist;
	int type=object->getFlag(TN_TYPES);
	TheScene->model->getFileList(type,flist);
	menu.AppendSubMenu(open_menu,"Replace with..");
	if(flist.size>0){
		wxMenu *remove_menu=getRemoveMenu(object);
		menu.AppendSubMenu(remove_menu,"Remove file..");
	}
}
