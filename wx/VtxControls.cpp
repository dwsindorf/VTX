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
	char p[256]="";
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
	char p[256]="";
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



