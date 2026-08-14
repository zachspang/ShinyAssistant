#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "Macro.h"
#include "MacroLibrary.h"

class MacroEditorFrame : public wxFrame {
public:
    MacroEditorFrame(wxWindow* parent, MacroLibrary* library, const wxString& filePath);

    // Loads the macro at libraryIndex into the editor (as a working copy) and shows the frame.
    void EditMacro(size_t libraryIndex);
    // Starts a fresh, blank macro. Only added to the library when Save is clicked.
    void EditNewMacro();

private:
    wxPanel* m_panel = nullptr;
    wxBoxSizer* m_actionSettingSizer = nullptr;
    wxArrayString actionList;
    wxChoice* m_actionChoice = nullptr;
    wxListBox* m_macroCommandsBox = nullptr;
    wxTextCtrl* m_nameCtrl = nullptr;

    MacroLibrary* m_library = nullptr;
    wxString m_filePath;
    int m_libraryIndex = -1; // -1 == new macro, not yet in the library

    Macro m_macro;
    Macro m_originalMacro; //snapshot taken when editing started, restored on Cancel
    MacroAction m_currentAction;
    int m_selectedActionIndex = -1;

    void LoadWorkingCopy(const Macro& source, int libraryIndex);
    void RefreshFromWorkingCopy(); //Reset name, list box, and settings panel from m_macro

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
    void OnNameChanged(wxCommandEvent& evt);
    void OnSave(wxCommandEvent& evt);
    //Discard everything since editing started
    void OnCancel(wxCommandEvent& evt);
};