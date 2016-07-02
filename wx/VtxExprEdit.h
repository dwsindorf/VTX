#ifndef EXPREDIT_H_
#define EXPREDIT_H_

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

WX_DECLARE_STRING_HASH_MAP( wxString, SymbolHash );

class VtxExprEdit : public wxTextCtrl
{
	DECLARE_CLASS(VtxExprEdit)
	void showSymbolString();
	void showExprString();
	void symbolsToExpr();
	void exprToSymbols();
	void editSelected();

	void fireChangeEvent();

	int insertion;

	bool selectedToken(wxString &);
	bool showing_symbols;
public:
	wxString value_expr;
	wxString symbol_expr;

	SymbolHash symbols;

	static void showSymbols(bool);
	static bool showingSymbols();
	static wxString getPrototype(int type);

	void setExprString(wxString str);
	wxString getExprString();
	wxString getShaderString();

	void update();
	VtxExprEdit(wxWindow* parent,wxWindowID id,const wxSize& size = wxDefaultSize);

    void OnTabKey(wxKeyEvent& event);
    void OnRightMouse(wxMouseEvent& event);
    void OnDoubleClick(wxMouseEvent& event);
    void OnSingleClick(wxMouseEvent& event);
    void OnAddItem(wxCommandEvent& event);
    void OnEditItem(wxCommandEvent& event);

    void setToken(wxString,wxString);
	void selectToken(wxString key);

	DECLARE_EVENT_TABLE()

};


#endif /*EXPREDIT_H_*/
