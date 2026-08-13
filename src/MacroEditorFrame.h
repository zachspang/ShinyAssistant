#pragma once
#include <wx/wx.h>

class MacroEditorFrame : public wxFrame {
public:
    MacroEditorFrame(wxWindow* parent);

    wxArrayString actionList;
private:
    wxPanel* m_panel;
    wxBoxSizer* m_actionSettingSizer;
    
    void OnClose(wxCloseEvent& evt);
    void OnActionChoice(wxCommandEvent& evt);
    
    //Changes m_actionSettingSizer's children depending on the selected action
    void PopulateActionSettings(wxWindow* settingsParent, const wxString& actionName);
};