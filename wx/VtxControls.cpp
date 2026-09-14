/*
 * VtxControls.cpp
 *
 *  Created on: Jan 29, 2009
 *      Author: deans
 */

#include "VtxControls.h"
#include "TerrainMgr.h"
#include "SceneClass.h"

//************************************************************
// Class PlainTextCtrl
//************************************************************
IMPLEMENT_CLASS(PlainTextCtrl, wxTextCtrl)

BEGIN_EVENT_TABLE(PlainTextCtrl, wxTextCtrl)
EVT_RIGHT_DOWN(PlainTextCtrl::OnRightMouse)
EVT_MENU(wxID_UNDO,PlainTextCtrl::OnMenu)
EVT_MENU(wxID_CUT,PlainTextCtrl::OnMenu)
EVT_MENU(wxID_COPY,PlainTextCtrl::OnMenu)
EVT_MENU(wxID_PASTE,PlainTextCtrl::OnMenu)
EVT_MENU(wxID_DELETE,PlainTextCtrl::OnMenu)
EVT_MENU(wxID_SELECTALL,PlainTextCtrl::OnMenu)
END_EVENT_TABLE()

//-------------------------------------------------------------
// PlainTextCtrl::OnRightMouse() show a normal edit menu in place of
// the OS default one (see the class comment in VtxControls.h) - same
// trick VtxExprEdit::OnRightMouse() already uses: catch the mouse-down
// ourselves and PopupMenu() our own menu so the native Win32 Edit
// control's context menu never gets triggered.
//-------------------------------------------------------------
void PlainTextCtrl::OnRightMouse(wxMouseEvent &event){
	wxMenu menu;
	bool editable=IsEditable();
	bool has_sel=!GetStringSelection().IsEmpty();

	menu.Append(wxID_UNDO,"Undo");
	menu.AppendSeparator();
	menu.Append(wxID_CUT,"Cut");
	menu.Append(wxID_COPY,"Copy");
	menu.Append(wxID_PASTE,"Paste");
	menu.Append(wxID_DELETE,"Delete");
	menu.AppendSeparator();
	menu.Append(wxID_SELECTALL,"Select All");

	menu.Enable(wxID_UNDO, editable && CanUndo());
	menu.Enable(wxID_CUT, editable && has_sel);
	menu.Enable(wxID_COPY, has_sel);
	menu.Enable(wxID_PASTE, editable && CanPaste());
	menu.Enable(wxID_DELETE, editable && has_sel);
	menu.Enable(wxID_SELECTALL, !GetValue().IsEmpty());

	PopupMenu(&menu);
}

//-------------------------------------------------------------
// PlainTextCtrl::OnMenu() handle the edit-menu commands above
//-------------------------------------------------------------
void PlainTextCtrl::OnMenu(wxCommandEvent &event){
	switch(event.GetId()){
	case wxID_UNDO:      Undo();              break;
	case wxID_CUT:       Cut();               break;
	case wxID_COPY:      Copy();              break;
	case wxID_PASTE:     Paste();             break;
	case wxID_DELETE:    {
		long from=0,to=0;
		GetSelection(&from,&to);
		if(from!=to)
			Remove(from,to);
		break;
	}
	case wxID_SELECTALL: SetSelection(-1,-1); break;
	}
}

//************************************************************
// Class Slider
//************************************************************

void  Slider::setValue(TNode  *arg){
	if(arg){
		char p[256]="";
		arg->valueString(p);
		setValue(p);
	}
	else
		setValue(0.0);
}

void Slider::setValueFromString(wxString s){
	char p[512]="";
	strcpy(p,s.ToAscii());
	if(strlen(p)>0){
		TNode *tnode=(TNode*)TheScene->parse_node(p);
		if(tnode){
			tnode->eval();
			double m=(max-min)/(umax-umin);
			double b=min-m*umin;
			value=m*S0.s+b;
			setSliderFromValue();
		}
	}

}
//************************************************************
// Class SliderCtrl
//************************************************************

//************************************************************
// Class TextCtrl
//************************************************************

void TextCtrl::setValue(TNode  *arg){
	char p[256]="";
	arg->valueString(p);
	SetValue(p);
}
void TextCtrl::SetValue(char  *p){
	text->SetValue(wxString(p));
}

wxString TextCtrl::GetValue(){
	return text->GetValue();
}

//************************************************************
// Class StaticTextCtrl
//************************************************************

void StaticTextCtrl::setValue(TNode  *arg){
	char p[256]="";
	arg->valueString(p);
	SetValue(p);
}
void StaticTextCtrl::SetValue(char  *value){
	text->SetLabel(value);
}
wxString StaticTextCtrl::GetValue(){
	return text->GetLabel();
}

//************************************************************
// Class ExprSliderCtrl
//************************************************************
void ExprSliderCtrl::setValue(TNode  *arg){
	if(arg){
		char p[256]="";
		arg->valueString(p);
	    text->setExprString(wxString(p));
	}
	else
		text->setExprString(wxString("0.0"));
	setValueFromString(text->getExprString());
}

//************************************************************
// Class ExprTextCtrl
//************************************************************

void ExprTextCtrl::SetValue(TNode  *arg){
	char p[512];
	p[0]=0;
	arg->valueString(p);
	SetValue(p);
}
void ExprTextCtrl::SetValue(wxString p){
	text->setExprString(p);
}

void ExprTextCtrl::SetValue(char  *p){
	text->setExprString(wxString(p));
}

wxString ExprTextCtrl::GetValue(){
	return text->getExprString();
}



