#include "UniverseModel.h"
#include "SceneClass.h"

#include "VtxExprEdit.h"
//#include "VtxScene.h"
#include "VtxFunctDialog.h"

#include <wx/textctrl.h>
#include <wx/tokenzr.h>

// define all resource ids here
enum {
	ID_TABKEY,
	ID_EDIT_ITEM,
	ID_ADD_NOISE,
	ID_ADD_CRATERS,
	ID_ADD_FRACTAL,
    ID_ADD_ERODE,
};

//************************************************************
// Class VtxExprEdit
//************************************************************

BEGIN_EVENT_TABLE(VtxExprEdit, wxTextCtrl)
EVT_CHAR(VtxExprEdit::OnTabKey)
EVT_RIGHT_DOWN(VtxExprEdit::OnRightMouse)
EVT_LEFT_DCLICK(VtxExprEdit::OnDoubleClick)
EVT_LEFT_UP(VtxExprEdit::OnSingleClick)
EVT_MENU(ID_EDIT_ITEM,VtxExprEdit::OnEditItem)
EVT_MENU(ID_ADD_NOISE,VtxExprEdit::OnAddItem)
EVT_MENU(ID_ADD_CRATERS,VtxExprEdit::OnAddItem)
EVT_MENU(ID_ADD_FRACTAL,VtxExprEdit::OnAddItem)
EVT_MENU(ID_ADD_ERODE,VtxExprEdit::OnAddItem)


END_EVENT_TABLE()

IMPLEMENT_CLASS(VtxExprEdit, wxTextCtrl )

VtxExprEdit::VtxExprEdit(wxWindow* parent,wxWindowID id,const wxSize& size) :
	wxTextCtrl(parent,id,wxString(""),wxDefaultPosition,size,wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB)
{
	showing_symbols=false;
	insertion=0;
	failed_expr=false;
}

//-------------------------------------------------------------
// VtxExprEdit::editSelected() edit selected token
//-------------------------------------------------------------
void VtxExprEdit::editSelected(){
	wxString selected = GetStringSelection();
	wxString symbol="";
	wxString value;
	SymbolHash::iterator it;

	for (it = symbols.begin(); it != symbols.end(); ++it) {
		wxString key;
		if(showing_symbols)
			key = it->first;
		else
			key = it->second;
		if(selected.IsSameAs(key)){
			symbol=it->first;
			value=it->second;
		}
	}
	if(symbol.IsEmpty())
		return;
	if(symbol.StartsWith("Noise")){
		noiseDialog->setEditor(this);
		noiseDialog->setFunct(symbol,value);
		if(!noiseDialog->IsShown())
			noiseDialog->Show(true);
	}
	else{
		functDialog->setEditor(this);
		functDialog->setFunct(symbol,value);
		if (!functDialog->IsShown())
			functDialog->Show(true);
	}
}

//-------------------------------------------------------------
// VtxExprEdit::selectToken() highlight token key
//-------------------------------------------------------------
void VtxExprEdit::selectToken(wxString key){
	wxString expr=GetValue();
	int start=expr.Find(key);
	int end=start+key.Length();
	SetSelection(start, end);
	if(VtxFunctDialog::auto_edit)
		editSelected();
	SetSelection(start, end);
}

//-------------------------------------------------------------
// VtxExprEdit::selectedToken() check if insertion point is in a token
//-------------------------------------------------------------
bool VtxExprEdit::selectedToken(wxString &sel_tok) {
	wxString expr=GetValue();
	int pt=GetInsertionPoint();
	SymbolHash::iterator it;

	for (it = symbols.begin(); it != symbols.end(); ++it) {
		wxString key;
		if(showing_symbols)
			key = it->first;
		else
			key = it->second;

		int start=expr.Find(key);
		if (start<=pt) {
			int end = start+key.Length();
			if (pt<end) {
				sel_tok=key;
				return true;
			}
		}
	}
	return false;
}

//-------------------------------------------------------------
// VtxExprEdit::symbolsToExpr() expand symbol string
//-------------------------------------------------------------
void VtxExprEdit::symbolsToExpr(){
	wxString str=symbol_expr;
	SymbolHash::iterator it;
    for( it = symbols.begin(); it != symbols.end(); ++it){
        wxString key = it->first;
        wxString value = it->second;
        str.Replace(key, value, true);
    }
    value_expr=str.Trim();
}

//-------------------------------------------------------------
// VtxExprEdit::exprToSymbols() create symbol string from value string
//-------------------------------------------------------------
void VtxExprEdit::exprToSymbols(){
	char buff[1024];

	wxString expr=value_expr;
	if(expr.IsEmpty()){
		showing_symbols=true;
		symbol_expr=value_expr;
		return;
	}
	sprintf(buff,"%s\n",(const char*)expr.ToAscii());
	TNode *n=(TNode*)TheScene->parse_node(buff);
	if(!n){
		cout<<"exprToSymbols failed:"<<buff<<endl;
		failed_expr=true;
		value_expr=last_good_value_expr;
		symbol_expr=last_good_symbol_expr;
		SetValue(symbol_expr);
		return;
	}
	Scope *old_scope=CurrentScope;
	Scope scope;
	CurrentScope=&scope;

	LinkedList<TNode*> list;
	n->addToken(list); // creates a list of all TNnodes that support tokens

	int noise=1;
	int fractal=1;
	int craters=1;
    int erodes=1;


	symbols.clear();
	TNode *tn=0;
	list.ss();
	char symb[256];
	char value[256];

	// - set TNnode token field only for tokens that are to be tracked
	// - add new tokens to hash table
	while((tn=list++)>0){
		symb[0]=0;
		switch(tn->typeValue()){
		case ID_NOISE:
			sprintf(symb,"Noise%d()",noise++);
			break;
		case ID_FCHNL:
			sprintf(symb,"Fractal%d()",fractal++);
			break;
		case ID_CRATERS:
			sprintf(symb,"Craters%d()",craters++);
			break;
        case ID_ERODE:
            sprintf(symb,"Erode%d()",erodes++);
            break;
		}
		if(symb[0]){
			value[0]=0;
			tn->propertyString(value);
			wxString str=wxString(symb);
			tn->setToken(str.ToAscii());
			if(symbols.find(str) == symbols.end())
				symbols[str]=wxString(value);
		}
	}
	scope.set_tokens(1);
	buff[0]=0;
	n->valueString(buff);  // extract token string

	symbol_expr=wxString(buff).Trim();
	scope.set_tokens(0);
	showing_symbols=true;
	delete n;
	CurrentScope=old_scope;
	if(failed_expr)
		failed_expr=false;

	last_good_value_expr=value_expr;
	last_good_symbol_expr=symbol_expr;
}
//-------------------------------------------------------------
// VtxExprEdit::showExprString() show value_expr
//-------------------------------------------------------------
void VtxExprEdit::showExprString(){
	SetValue(value_expr);
}
//-------------------------------------------------------------
// VtxExprEdit::showSymbolString() show symbol_expr
//-------------------------------------------------------------
void VtxExprEdit::showSymbolString(){
	SetValue(symbol_expr);
	//cout << getShaderString() << endl;
}

//-------------------------------------------------------------
// VtxExprEdit::setExprString() set expr string
//-------------------------------------------------------------
void VtxExprEdit::setExprString(wxString str){
	value_expr=str;
	//cout << "VtxExprEdit::setExprString:"<< str.ToAscii()<< endl;
	exprToSymbols();
	if(VtxFunctDialog::show_symbols){
		showing_symbols=true;
		showSymbolString();
	}
	else{
		showing_symbols=false;
		showExprString();
	}
}

//-------------------------------------------------------------
// VtxExprEdit::update() update expr text
// - called by VtxFunctDialog when show tokens check changes
//-------------------------------------------------------------
void VtxExprEdit::update(){
	//cout<<"VtxExprEdit::update():"<<GetValue()<<endl;

	if(VtxFunctDialog::show_symbols){
		if(!showing_symbols){
			value_expr=GetValue();
			exprToSymbols();
			showing_symbols=true;
		}
		showSymbolString();
	}
	else{
		if(showing_symbols){
			symbol_expr=GetValue();
			symbolsToExpr();
			showing_symbols=false;
		}
		showExprString();
	}
}

//-------------------------------------------------------------
// VtxExprEdit::getExprString() return symbol expanded expr
//-------------------------------------------------------------
wxString VtxExprEdit::getExprString(){
	if(showing_symbols){
		if(symbol_expr.IsEmpty() && !value_expr.IsEmpty()){
			exprToSymbols();
		}
		else
			symbol_expr=GetValue();
		symbolsToExpr();
	}
	else
		value_expr=GetValue();
	return value_expr;
}

//-------------------------------------------------------------
// VtxExprEdit::getShaderString() return shader expr
//-------------------------------------------------------------
wxString VtxExprEdit::getShaderString(){
	wxString str=symbol_expr;
	char tmp1[64];
	char tmp2[64];

	for(int i=0;i<MAX_NOISE;i++){
		sprintf(tmp1,"Noise%d",i+1);
		sprintf(tmp2,"Noise(%d)",i);
		str.Replace(wxString(tmp1),wxString(tmp2),true);
		sprintf(tmp1,"Fractal%d()",i+1);
		sprintf(tmp2,"0");
		str.Replace(wxString(tmp1),wxString(tmp2),true);
		sprintf(tmp1,"Craters%d()",i+1);
		sprintf(tmp2,"0");
		str.Replace(wxString(tmp1),wxString(tmp2),true);
        sprintf(tmp1,"Erode%d()",i+1);
        sprintf(tmp2,"0");
        str.Replace(wxString(tmp1),wxString(tmp2),true);
	}
	return str.Trim();
}
//-------------------------------------------------------------
// VtxExprEdit::setToken() replace token value
// - called from VtxFunctDialog after a control change event
// - if functDialog->autoApply() is true a EVT_TEXT_ENTER event
//   is sent to the containing class
//-------------------------------------------------------------
void VtxExprEdit::setToken(wxString key,wxString value){
	SymbolHash::iterator it=symbols.find(key);
	if(it==symbols.end())
		return;
	symbols[key]=value;
	if(!VtxFunctDialog::show_symbols && !showing_symbols){
		symbolsToExpr();
		showExprString();
	}
	if(VtxFunctDialog::auto_apply)
		fireChangeEvent();
}
//-------------------------------------------------------------
// VtxExprEdit::fireChangeEvent() on changed text (from program)
// - send a EVT_TEXT_ENTER event is sent to the containing class
//-------------------------------------------------------------
void VtxExprEdit::fireChangeEvent(){
	wxCommandEvent evt(wxEVT_COMMAND_TEXT_ENTER);
	evt.SetEventObject(this);
	evt.SetId(GetId());
	GetEventHandler()->ProcessEvent(evt) ;
}

//-------------------------------------------------------------
// static VtxExprEdit::getPrototype() return a prototype string
//-------------------------------------------------------------
wxString VtxExprEdit::getPrototype(int type){
	char prototype[256];
	prototype[0]=0;
	switch(type){
	case TN_NOISE:
	case TN_FCHNL:
	case TN_CRATERS:
    case TN_ERODE:
		if(TheScene->getPrototype(0,type,prototype))
			return wxString(prototype).Trim();
		break;
	}
	return wxString("");
}

// ################# Event Handlers ###########################

//-------------------------------------------------------------
// VtxExprEdit::OnRightMouse() show pop-up menu
//-------------------------------------------------------------
void VtxExprEdit::OnRightMouse(wxMouseEvent& event){
	wxMenu menu;
	wxString selected=GetStringSelection();
	if(!selected.IsEmpty()){
		menu.Append(ID_EDIT_ITEM,wxT("Edit"));
	}
	else{
		wxMenu *submenu=new wxMenu();
		submenu->Append(ID_ADD_NOISE,wxT("Noise"));
		submenu->Append(ID_ADD_FRACTAL,wxT("Fractal"));
		submenu->Append(ID_ADD_CRATERS,wxT("Craters"));
        submenu->Append(ID_ADD_ERODE,wxT("Erode"));
		menu.AppendSubMenu(submenu, "Add ..");
	}
	PopupMenu(&menu);
}

//-------------------------------------------------------------
// VtxExprEdit::OnEditItem() on menu-edit event
//-------------------------------------------------------------
void VtxExprEdit::OnEditItem(wxCommandEvent& event) {
	editSelected();
}

//-------------------------------------------------------------
// VtxExprEdit::OnDoubleClick() process double-click event
//-------------------------------------------------------------
void VtxExprEdit::OnDoubleClick(wxMouseEvent& event){
	wxString key;
	if(selectedToken(key))
		selectToken(key);
}

//-------------------------------------------------------------
// VtxExprEdit::OnSingleClick() process single-click event
//-------------------------------------------------------------
void VtxExprEdit::OnSingleClick(wxMouseEvent& event){
	event.Skip();
	insertion=GetInsertionPoint();
	//cout << "single-click:" << insertion << endl;
}
//-------------------------------------------------------------
// VtxExprEdit::OnTabKey() key-press event handler
//-------------------------------------------------------------
void VtxExprEdit::OnTabKey(wxKeyEvent& event) {
	int key=event.GetKeyCode();

	if (key != WXK_TAB) {
		event.Skip(); // not a tab key
		if(key=='n'||key=='c'||key=='f')
			insertion=GetInsertionPoint();
		if(key !=WXK_RETURN)
			return;
	}
	wxString expr=GetValue();
	wxString start=expr.Mid(0, insertion);
	wxString end=expr.Mid(insertion);
	wxStringTokenizer tkz(end, wxT("*/+- =,()[]"), wxTOKEN_STRTOK);
	int di=0;
	if (tkz.HasMoreTokens()){
		wxString token = tkz.GetNextToken();
		wxString alias;
		if (token.StartsWith("n"))
			alias=getPrototype(TN_NOISE);
		else if (token.StartsWith("c"))
			alias=getPrototype(TN_CRATERS);
		else if (token.StartsWith("f"))
			alias=getPrototype(TN_FCHNL);
		else if (token.StartsWith("e"))
			alias=getPrototype(TN_ERODE);
		if(!alias.IsEmpty()){
			di=end.Find(token);
			end.Replace(token, alias, false);
			expr=start+end;
		}
	}
	//cout<<"OnTabKey:"<<key<<" "<<expr<<endl;

	if (showing_symbols) {
		symbol_expr=expr;
		symbolsToExpr();
		exprToSymbols();
		showSymbolString();
	} else {
		value_expr=expr;
		showExprString();
	}
	SetInsertionPoint(insertion+di);
}

//-------------------------------------------------------------
// VtxExprEdit::OnAddItem() add new noise function
//-------------------------------------------------------------
void VtxExprEdit::OnAddItem(wxCommandEvent& event){
	wxString str;
	switch(event.GetId()){
	case ID_ADD_NOISE:
		str=getPrototype(TN_NOISE);
		break;
	case ID_ADD_CRATERS:
		str=getPrototype(TN_CRATERS);
		break;
	case ID_ADD_FRACTAL:
		str=getPrototype(TN_FCHNL);
		break;
    case ID_ADD_ERODE:
        str=getPrototype(TN_ERODE);
        break;
	}
	wxString expr=GetValue();
	int pt=GetInsertionPoint();
	if(value_expr.IsEmpty())
		expr=str;
	else
		expr=expr.Mid(0,pt)+str+expr.Mid(pt);

	if(showing_symbols){
		symbol_expr=expr;
		symbolsToExpr();
		exprToSymbols();
		showSymbolString();
	}
	else{
		value_expr=expr;
		showExprString();
	}
	SetInsertionPoint(pt);
}
