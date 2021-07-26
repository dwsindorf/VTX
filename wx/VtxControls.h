#ifndef VTXCONTROLS_H_
#define VTXCONTROLS_H_

#include "VtxExprEdit.h"

//#ifndef WX_PRECOMP
//#include "wx/wx.h"
//#endif

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/menu.h"
#include "wx/dcclient.h"
#include "wx/statusbr.h"
#include <wx/colour.h>
#include <wx/clrpicker.h>

#include "ColorClass.h"

class TNode;

class TextCtrl
{
public:
	wxTextCtrl *text;
	wxStaticText *label;
	int text_id;
	void Init(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts){
		text_id=id;
		if(ls>0)
			label=new wxStaticText(parent,-1,_label,wxDefaultPosition,wxSize(ls,-1));
		else
			label=0;
	    text=new wxTextCtrl(parent,text_id,"",wxDefaultPosition,wxSize(ts,-1),wxTE_PROCESS_ENTER);
	}
	TextCtrl(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts){
		Init(parent,id,_label,ls,ts);
	}
	wxSizer* getSizer(){
		wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
		if(label){
			boxSizer->Add(label, 0, wxALIGN_LEFT|wxALL, 3);
			boxSizer->Add(text, 0, wxALIGN_LEFT|wxALL, 3);
		}
		else
			boxSizer->Add(text, 0, wxALIGN_LEFT|wxALL, 0);
		return boxSizer;
	}
	void SetValue(char  *value);
	void setValue(char  *p) {SetValue(p);}
	void setValue(TNode  *arg);
	wxString GetValue();
	wxString getText() { return GetValue();}
	void SetEditable(bool editable){
		text->SetEditable(editable);
	}
};

class StaticTextCtrl
{
public:
	wxStaticText *text;
	wxStaticText *label;
	int text_id;
	void Init(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts){
		text_id=id;
		label=new wxStaticText(parent,-1,_label
	    		,wxDefaultPosition,wxSize(ls,-1));
	    text=new wxStaticText(parent,text_id,""
	    		,wxDefaultPosition,wxSize(ts,-1));
	}
	StaticTextCtrl(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts){
		Init(parent,id,_label,ls,ts);
	}
	wxSizer* getSizer(){
		wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
		boxSizer->Add(label, 0, wxALIGN_LEFT|wxALL, 3);
		boxSizer->Add(text, 0, wxALIGN_LEFT|wxALL, 3);
		return boxSizer;
	}
	void SetValue(char  *value);
	void setValue(TNode  *arg);
	wxString GetValue();
	wxString getText() { return GetValue();}
};

class ExprTextCtrl
{
public:
	VtxExprEdit *text;
	wxStaticText *label;
	int text_id;
	void Init(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts){
		text_id=id;
		if(ls>0)
			label=new wxStaticText(parent,-1,_label,wxDefaultPosition,wxSize(ls,-1));
		else
			label=0;
	    text=new VtxExprEdit(parent,text_id,wxSize(ts,-1));
	}
	ExprTextCtrl(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts){
		Init(parent,id,_label,ls,ts);
	}
	wxSizer* getSizer(){
		wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
		if(label){
			boxSizer->Add(label, 0, wxALIGN_LEFT|wxALL, 3);
			boxSizer->Add(text, 0, wxALIGN_LEFT|wxALL, 3);
		}
		else
			boxSizer->Add(text, 0, wxALIGN_LEFT|wxALL, 0);
		return boxSizer;
	}
	void SetValue(char  *p);
	void SetValue(wxString p);
	void SetValue(TNode  *arg);
	void setValue(TNode  *arg) { SetValue(arg);}
	wxString SetValue();
	void SetEditable(bool editable){
		text->SetEditable(editable);
	}
	wxString GetValue();
	wxString getText() { return GetValue();}

};


class Slider
{
protected:
	enum {
		MAX_SLIDER_VALUE = 50000,
	};
	wxString valueString(){
		double m=(max-min)/(umax-umin);
		double b=min-m*umin;
		double tval=(value-b)/m;
		wxString s;
		if(format.Length()>0)
			s = wxString::Format(format, tval);
		else if(fabs(umax-umin)<4.0)
			s = wxString::Format("%0.2f", tval);
		else if(fabs(umax-umin)<1000.0)
			s = wxString::Format("%0.1f", tval);
		else
			s = wxString::Format("%0.0f", tval);
		return s;
	}
	void setValueFromString(wxString s);
public:
	wxSlider   *slider;
	wxStaticText *label;

	wxString format;

	double min,max,value;
	double umin,umax;

	int text_id;
	int slider_id;
	int text_width;
	void Init(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts, int ss){
		slider_id=id;
		text_id=id+1;
		min=0;
		max=100.0;
		value=0;
		umin=min;
		umax=max;
		format="";
		text_width=ts;

		if(ls<=0)
			label=NULL;
		else
			label=new wxStaticText(parent,-1,_label
	    		,wxDefaultPosition,wxSize(ls,-1));
	    slider=new wxSlider(parent,slider_id,0,0,MAX_SLIDER_VALUE
	    		,wxDefaultPosition,wxSize(ss,-1)
	     		);

	}
	Slider(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts, int ss){
		Init(parent,id,_label,ls,ts,ss);
	}
	Slider(wxWindow* parent, wxWindowID id, const wxString& _label) {
		Init(parent,id,_label,-1, 35,80);
	}

	void setSliderFromValue(){
		int svalue=(int)((MAX_SLIDER_VALUE)*(value-min)/(max-min));
		slider->SetValue(svalue);
	}
	void setValueFromSlider(){
		int svalue=slider->GetValue();
		value=(max-min)*svalue/MAX_SLIDER_VALUE+min;
		setTextFromValue();
	}
	virtual void setValue(double x){
		value=x;
		setSliderFromValue();
		setTextFromValue();
	}
	//void SetValue(double x){ setValue(x);}
	double getValue(){
		return value;
	}
	double getValueFromText(){
		setValueFromText();
		return value;
	}
	double getValueFromSlider(){
		setValueFromSlider();
		return value;
	}
	void setRange(double _min, double _max){
		min=umin=_min;
		max=umax=_max;
	}
	void setRange(double _min, double _max,double u1, double u2){
		min=_min; max=_max;	umin=u1; umax=u2;
	}
	virtual void setValue(TNode  *arg);
	virtual void setValueFromText() {}
	virtual void setToolBar(wxToolBar *tb) {}
	virtual wxSizer* getSizer()     { return 0;}
	virtual void setTextFromValue() {}
	virtual void setValue(char  *p) {}
	virtual wxString getText()      { return wxString("");}
};

class SliderCtrl : public Slider
{
public:
	wxTextCtrl *text;
	SliderCtrl(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts, int ss)
	 : Slider(parent,id,_label,ls,ts,ss)
	{
	    text=new wxTextCtrl(parent,text_id,"",wxDefaultPosition,wxSize(ts,-1),wxTE_PROCESS_ENTER);
	}
	SliderCtrl(wxWindow* parent, wxWindowID id, const wxString& _label)
	 : Slider(parent,id,_label,-1,35,80)
	{
	    text=new wxTextCtrl(parent,text_id,"",wxDefaultPosition,wxSize(text_width,-1),wxTE_PROCESS_ENTER);
	}
	void setValue(double x){
		Slider::setValue(x);
	}
	void setValue(TNode  *arg){
		Slider::setValue(arg);
	}
	void setTextFromValue(){
		wxString s=valueString();
		text->SetValue(s);
	}
	void setValue(char  *p){
		text->ChangeValue(wxString(p));
		setValueFromText();
		setSliderFromValue();
	}

	void setValueFromText(){
		setValueFromString(text->GetValue());
	}

	wxString getText() { return text->GetValue();}

	void setToolBar(wxToolBar *tb){
		if(label !=NULL)
			tb->AddControl(label);
		tb->AddControl(text);
		tb->AddControl(slider);
	}
	wxSizer* getSizer(){
		wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
		if(label !=NULL)
			boxSizer->Add(label, 0, wxALIGN_LEFT|wxALL, 3);
		boxSizer->Add(text, 0, wxALIGN_LEFT|wxALL, 3);
		boxSizer->Add(slider, 0, wxALIGN_LEFT|wxALL, 3);
		return boxSizer;
	}
};

class ExprSliderCtrl : public Slider
{
public:
	VtxExprEdit *text;
	ExprSliderCtrl(wxWindow* parent, wxWindowID id, const wxString& _label, int ls, int ts, int ss)
	 : Slider(parent,id,_label,ls,ts,ss)
	{
		text=new VtxExprEdit(parent,text_id,wxSize(ts,-1));
	}
	ExprSliderCtrl(wxWindow* parent, wxWindowID id, const wxString& _label)
	 : Slider(parent,id,_label,-1,35,80)
	{
		text=new VtxExprEdit(parent,text_id,wxSize(text_width,-1));
	}
	void setValue(double x){
		Slider::setValue(x);
	}
	void setValue(TNode  *arg);
	void setTextFromValue(){
		wxString s=valueString();
		text->setExprString(s);
	}
	void setValueFromText(){
		setValueFromString(text->getExprString());
	}

	void setValue(char  *p){
		text->SetValue(p);
		setValueFromText();
		setSliderFromValue();
	}

	wxString getText() { return text->getExprString();}

	void setToolBar(wxToolBar *tb){
		if(label !=NULL)
			tb->AddControl(label);
		tb->AddControl(text);
		tb->AddControl(slider);
	}
	wxSizer* getSizer(){
		wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
		if(label !=NULL)
			boxSizer->Add(label, 0, wxALIGN_LEFT|wxALL, 3);
		boxSizer->Add(text, 0, wxALIGN_LEFT|wxALL, 3);
		boxSizer->Add(slider, 0, wxALIGN_LEFT|wxALL, 3);
		return boxSizer;
	}
};

class ColorSlider
{
	SliderCtrl *slider;
	wxColourPickerCtrl *chooser;
	int csize;
	void Init(wxWindow* parent, wxWindowID id, const wxString& label, int ls, int ts, int ss, int sc){
		csize=sc;
		wxColor col("RGB(255,255,255)");
	    slider=new SliderCtrl(parent, id, label,ls,ts,ss);
	    slider->setRange(0,1);
	    slider->setValue(1.0);
	    chooser=new wxColourPickerCtrl(parent,id+2,col);
	}

public:
	ColorSlider(wxWindow* parent, wxWindowID id, const wxString& label, int ls, int ts, int ss, int sc){
		Init(parent, id,label,ls,ts,ss,sc);
	}
	ColorSlider(wxWindow* parent, wxWindowID id, const wxString& label, int ls, int ts, int ss){
		Init(parent, id,label,ls,ts,ss,0);
	}
	~ColorSlider(){
		delete slider;
	}
	double getValueFromSlider(){
		return slider->getValueFromSlider();
	}
	double getValueFromText(){
		return slider->getValueFromText();
	}
	void setValueFromSlider(){
		slider->setValueFromSlider();
	}
	void setValueFromText(){
		slider->setValueFromText();
	}
	void setValue(double x){
		slider->setValue(x);
	}
	double getValue(){
		return slider->getValue();
	}
	void setRange(double min, double max){
		slider->setRange(min,max);
	}
	void setRange(double min, double max,double u1, double u2){
		slider->setRange(min,max,u1,u2);
	}

	wxSizer* getSizer(){
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(slider->getSizer(),0,wxALIGN_LEFT|wxALL,0);
		sizer->Add(chooser, 0, wxALIGN_LEFT|wxALL, 0);
		if(csize>0)
			sizer->SetItemMinSize(chooser, csize, sizer->GetMinSize().GetHeight()-4);
		return sizer;
	}
	void setColor(Color c){
		wxColor col(c.rb(),c.gb(),c.bb(),c.ab());
		chooser->SetColour(col);
		setValue(c.alpha());
	}
	Color getColor(){
		wxColor col=chooser->GetColour();
		Color c;
		c.set_rb(col.Red());
		c.set_gb(col.Green());
		c.set_bb(col.Blue());
		c.set_alpha(getValue());
		return c;
	}
	void setColor(float *v){
		unsigned char r,g,b,a;
		r=(unsigned char)(v[0]*255.0f);
		g=(unsigned char)(v[1]*255.0f);
		b=(unsigned char)(v[2]*255.0f);
		a=(unsigned char)(v[3]*255.0f);
		wxColor col(r,g,b,a);
		chooser->SetColour(col);
		setValue(v[3]);
	}
	void getColor(float *v){
		wxColor col=chooser->GetColour();
		v[0]=((float)col.Red())/255.0f;
		v[1]=((float)col.Green())/255.0f;
		v[2]=((float)col.Blue())/255.0f;
		v[3]=getValue();
	}
};

#endif /*VTXCONTROLS_H_*/
