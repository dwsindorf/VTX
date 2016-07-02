#ifndef FOGTABS_H_
#define FOGTABS_H_

#include "VtxTabsMgr.h"
#include <TerrainClass.h>
class VtxFogTabs : public VtxTabsMgr

{
	DECLARE_CLASS(VtxFogTabs)

protected:
	TNfog *fog() 	{ return (TNfog *)object_node->node;}
	TNfog *object() 	{ return ((TNfog *)object_node->node);}
	Planetoid *getOrbital()
	{
		NodeIF *obj=fog();
		while(obj && !(obj->typeValue() & ID_ORBITAL))
			obj=obj->getParent();

		return (Planetoid*)obj;
	}
	void AddFogTab(wxWindow *panel);
    void invalidateObject(){
    	fog()->invalidate();
    	setObjAttributes();
    	TheView->set_changed_detail();
    	TheScene->rebuild();
    }
    void invalidateRender(){
    	setObjAttributes();
    	TheView->set_changed_render();
    }
	void getObjAttributes();
	void setObjAttributes();

	ExprTextCtrl *m_density_expr;
	SliderCtrl   *m_hmin_sldr;
	SliderCtrl   *m_hmax_sldr;
	SliderCtrl   *m_vmin_sldr;
	SliderCtrl   *m_vmax_sldr;
	ColorSlider  *m_fog_color;
public:
	VtxFogTabs(wxWindow* parent,
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
	~VtxFogTabs(){
		delete m_density_expr;
		delete m_hmin_sldr;
		delete m_hmax_sldr;
		delete m_vmin_sldr;
		delete m_vmax_sldr;
		delete m_fog_color;
	}

	void updateControls();
	int showMenu(bool);
    void OnEnable(wxCommandEvent& event);
    void OnUpdateEnable(wxUpdateUIEvent& event);

    void OnEndHminSlider(wxScrollEvent& event){
    	m_hmin_sldr->setValueFromSlider();
    	invalidateRender();
    }
    void OnHminSlider(wxScrollEvent& event){
    	m_hmin_sldr->setValueFromSlider();
    }
    void OnHminEnter(wxCommandEvent& event){
    	m_hmin_sldr->setValueFromText();
    	invalidateRender();
    }

    void OnEndHmaxSlider(wxScrollEvent& event){
    	m_hmax_sldr->setValueFromSlider();
    	invalidateRender();
    }
    void OnHmaxSlider(wxScrollEvent& event){
    	m_hmax_sldr->setValueFromSlider();
    }
    void OnHmaxEnter(wxCommandEvent& event){
    	m_hmax_sldr->setValueFromText();
    	invalidateRender();
    }

    void OnEndVminSlider(wxScrollEvent& event){
    	m_vmin_sldr->setValueFromSlider();
    	invalidateRender();
    }
    void OnVminSlider(wxScrollEvent& event){
    	m_vmin_sldr->setValueFromSlider();
    }
    void OnVminEnter(wxCommandEvent& event){
    	m_vmin_sldr->setValueFromText();
    	invalidateRender();
    }

    void OnEndVmaxSlider(wxScrollEvent& event){
    	m_vmax_sldr->setValueFromSlider();
    	invalidateRender();
    }
    void OnVmaxSlider(wxScrollEvent& event){
    	m_vmax_sldr->setValueFromSlider();
    }
    void OnVmaxEnter(wxCommandEvent& event){
    	m_vmax_sldr->setValueFromText();
    	invalidateRender();
    }

    void OnEndFogSlider(wxScrollEvent& event){
    	m_fog_color->setValueFromSlider();
    	invalidateRender();
    }
    void OnFogSlider(wxScrollEvent& event){
    	m_fog_color->setValueFromSlider();
    }
    void OnFogEnter(wxCommandEvent& event){
    	m_fog_color->setValueFromText();
    	invalidateRender();
     }
    void OnFogColor(wxColourPickerEvent& WXUNUSED(event)){
    	invalidateRender();
    }

    void OnChangedExpr(wxCommandEvent& event){
    	invalidateObject();
    }

	DECLARE_EVENT_TABLE()
};
#endif /*FOGTABS_H_*/
