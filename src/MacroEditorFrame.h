#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "Macro.h"

class MacroEditorFrame : public wxFrame {
public:
    MacroEditorFrame(wxWindow* parent);

private:
    wxPanel* m_panel = nullptr;
    wxBoxSizer* m_actionSettingSizer = nullptr;
    wxArrayString actionList;
    wxChoice* m_actionChoice = nullptr;
    wxListBox* m_macroCommandsBox = nullptr;

    Macro m_macro;
    MacroAction m_currentAction;
    int m_selectedActionIndex = -1;

    void PopulateActionSettings(wxWindow* settingsParent, const wxString& actionName);
    void CommitIfEditingExisting();
    void RefreshMacroListBox();

    void OnClose(wxCloseEvent& evt);
    void OnActionChoice(wxCommandEvent& evt);
    void OnButtonChoiceChanged(wxCommandEvent& evt);
    void OnJoystickXChanged(wxSpinEvent& evt);
    void OnJoystickYChanged(wxSpinEvent& evt);
    void OnDelayChanged(wxSpinEvent& evt);
    void OnEncounterIncrementChanged(wxSpinEvent& evt);
    void OnAddAction(wxCommandEvent& evt);
    void OnDeleteAction(wxCommandEvent& evt);
    void OnMacroListSelected(wxCommandEvent& evt);
};