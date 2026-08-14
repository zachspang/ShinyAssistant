#include "MacroEditorFrame.h"

MacroEditorFrame::MacroEditorFrame(wxWindow* parent, MacroLibrary* library, const wxString& filePath)
    : wxFrame(parent, wxID_ANY, "Macro Editor"), m_library(library), m_filePath(filePath) {

    m_panel = new wxPanel(this);
    Bind(wxEVT_CLOSE_WINDOW, &MacroEditorFrame::OnClose, this);

    wxArrayString macroCommands;
    //TODO: Load from saved macros

    m_macroCommandsBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(100,300), macroCommands, wxLB_ALWAYS_SB);
    m_macroCommandsBox->Bind(wxEVT_LISTBOX, &MacroEditorFrame::OnMacroListSelected, this);

    wxPanel* rightPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100,300));

    wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* nameLabel = new wxStaticText(rightPanel, wxID_ANY, "Name: ");
    m_nameCtrl = new wxTextCtrl(rightPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1));
    m_nameCtrl->Bind(wxEVT_TEXT, &MacroEditorFrame::OnNameChanged, this);
    nameSizer->Add(nameLabel, wxSizerFlags().Center());
    nameSizer->Add(m_nameCtrl, wxSizerFlags().Center());

    actionList.Add("Press Button");
    actionList.Add("Release Button");
    actionList.Add("Move Joystick");
    actionList.Add("Delay");
    actionList.Add("Check For Shiny");
    actionList.Add("Add to encounter number");

    wxBoxSizer* actionChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* actionLabel = new wxStaticText(rightPanel, wxID_ANY, "Action: ");
    m_actionChoice = new wxChoice(rightPanel, wxID_ANY, wxDefaultPosition, wxSize(200,-1), actionList);
    m_actionChoice->Bind(wxEVT_CHOICE, &MacroEditorFrame::OnActionChoice, this);
    m_actionChoice->SetSelection(0);
    actionChoiceSizer->Add(actionLabel, wxSizerFlags().Center());
    actionChoiceSizer->Add(m_actionChoice, wxSizerFlags().Center());

    m_actionSettingSizer = new wxBoxSizer(wxHORIZONTAL);
    PopulateActionSettings(rightPanel, actionList[0]);

    wxButton* addButton = new wxButton(rightPanel, wxID_ANY, "Add action");
    wxButton* deleteButton = new wxButton(rightPanel, wxID_ANY, "Delete action");
    addButton->Bind(wxEVT_BUTTON, &MacroEditorFrame::OnAddAction, this);
    deleteButton->Bind(wxEVT_BUTTON, &MacroEditorFrame::OnDeleteAction, this);
    wxBoxSizer* addDeleteSizer = new wxBoxSizer(wxHORIZONTAL);
    addDeleteSizer->Add(addButton, wxSizerFlags().Border(wxALL, 5));
    addDeleteSizer->Add(deleteButton, wxSizerFlags().Border(wxALL, 5));

    wxButton* saveButton = new wxButton(rightPanel, wxID_SAVE, "Save");
    wxButton* cancelButton = new wxButton(rightPanel, wxID_CANCEL, "Cancel");
    saveButton->Bind(wxEVT_BUTTON, &MacroEditorFrame::OnSave, this);
    cancelButton->Bind(wxEVT_BUTTON, &MacroEditorFrame::OnCancel, this);

    wxBoxSizer* saveCloseSizer = new wxBoxSizer(wxHORIZONTAL);
    saveCloseSizer->AddStretchSpacer(1);
    saveCloseSizer->Add(saveButton, wxSizerFlags().Border(wxALL, 5));
    saveCloseSizer->Add(cancelButton, wxSizerFlags().Border(wxALL, 5));

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    rightSizer->Add(nameSizer, wxSizerFlags().Center());
    rightSizer->AddSpacer(20);
    rightSizer->Add(actionChoiceSizer, wxSizerFlags().Center());
    rightSizer->AddSpacer(40);
    rightSizer->Add(m_actionSettingSizer, wxSizerFlags().Center());
    rightSizer->AddSpacer(40);
    rightSizer->Add(addDeleteSizer, wxSizerFlags().Center());
    rightSizer->AddStretchSpacer(1);
    rightSizer->Add(saveCloseSizer, wxSizerFlags().Expand());

    rightPanel->SetSizer(rightSizer);
    rightSizer->SetSizeHints(rightPanel);

    wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(m_macroCommandsBox, wxSizerFlags().Proportion(1).Expand());
    boxSizer->AddSpacer(2);
    boxSizer->Add(rightPanel, wxSizerFlags().Proportion(1).Expand());
    SetSizerAndFit(boxSizer);
    boxSizer->SetSizeHints(this);
}

void MacroEditorFrame::LoadWorkingCopy(const Macro& source, int libraryIndex) {
    m_originalMacro = source;
    m_macro = source;
    m_libraryIndex = libraryIndex;
    m_selectedActionIndex = -1;
    m_currentAction = MacroAction{};

    RefreshFromWorkingCopy();
}

void MacroEditorFrame::RefreshFromWorkingCopy() {
    m_nameCtrl->ChangeValue(m_macro.GetName());
    RefreshMacroListBox();

    m_actionChoice->SetSelection(0);
    PopulateActionSettings(m_actionChoice->GetParent(), actionList[0]);
}

void MacroEditorFrame::EditMacro(size_t libraryIndex) {
    if (libraryIndex >= m_library->GetMacros().size()) return;
    LoadWorkingCopy(m_library->GetMacros()[libraryIndex], (int)libraryIndex);
    Show();
    Raise();
}

void MacroEditorFrame::EditNewMacro() {
    LoadWorkingCopy(Macro("New Macro"), -1);
    Show();
    Raise();
}

void MacroEditorFrame::OnClose(wxCloseEvent& evt) {
    if (evt.CanVeto()) evt.Veto(); //veto the default close behavior and hide instead
    m_macro = m_originalMacro; //discard any unsaved edits, same as Cancel
    RefreshFromWorkingCopy();
    Hide();
}

void MacroEditorFrame::PopulateActionSettings(wxWindow* settingsParent, const wxString& actionName) {
    m_actionSettingSizer->Clear(true);

    ActionType type = ChoiceLabelToActionType(actionName);
    m_currentAction.type = type;

    switch (type) {
        case ActionType::PressButton:
        case ActionType::ReleaseButton: {
            wxString label = (type == ActionType::PressButton) ? "Button to press: " : "Button to release: ";
            wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, label);

            wxArrayString buttonChoices;
            for (int i = 0; i < (int)ControllerButton::COUNT; ++i)
                buttonChoices.Add(ButtonToString((ControllerButton)i));

            wxChoice* buttonChoice = new wxChoice(settingsParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, buttonChoices);
            buttonChoice->SetSelection((int)m_currentAction.button);
            buttonChoice->Bind(wxEVT_CHOICE, &MacroEditorFrame::OnButtonChoiceChanged, this);

            m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
            m_actionSettingSizer->Add(buttonChoice, wxSizerFlags().Center().Border(wxLEFT, 5));
            break;
        }
        case ActionType::MoveJoystick: {
            wxStaticText* xLabel = new wxStaticText(settingsParent, wxID_ANY, "X: ");
            wxSpinCtrl* xSpin = new wxSpinCtrl(settingsParent, wxID_ANY, wxEmptyString,
                wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS | wxSP_WRAP, -100, 100, m_currentAction.joystickX);
            xSpin->Bind(wxEVT_SPINCTRL, &MacroEditorFrame::OnJoystickXChanged, this);

            wxStaticText* yLabel = new wxStaticText(settingsParent, wxID_ANY, "Y: ");
            wxSpinCtrl* ySpin = new wxSpinCtrl(settingsParent, wxID_ANY, wxEmptyString,
                wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS | wxSP_WRAP, -100, 100, m_currentAction.joystickY);
            ySpin->Bind(wxEVT_SPINCTRL, &MacroEditorFrame::OnJoystickYChanged, this);

            m_actionSettingSizer->Add(xLabel, wxSizerFlags().Center());
            m_actionSettingSizer->Add(xSpin, wxSizerFlags().Center().Border(wxLEFT, 5));
            m_actionSettingSizer->Add(yLabel, wxSizerFlags().Center().Border(wxLEFT, 15));
            m_actionSettingSizer->Add(ySpin, wxSizerFlags().Center().Border(wxLEFT, 5));
            break;
        }
        case ActionType::Delay: {
            wxStaticText* delayLabel = new wxStaticText(settingsParent, wxID_ANY, "Delay: ");
            wxSpinCtrl* delaySpin = new wxSpinCtrl(settingsParent, wxID_ANY, wxEmptyString,
                wxDefaultPosition, wxSize(90, -1), wxSP_ARROW_KEYS, 0, 100000, m_currentAction.delayMs);
            delaySpin->Bind(wxEVT_SPINCTRL, &MacroEditorFrame::OnDelayChanged, this);
            wxStaticText* msLabel = new wxStaticText(settingsParent, wxID_ANY, " ms");

            m_actionSettingSizer->Add(delayLabel, wxSizerFlags().Center());
            m_actionSettingSizer->Add(delaySpin, wxSizerFlags().Center().Border(wxLEFT, 5));
            m_actionSettingSizer->Add(msLabel, wxSizerFlags().Center());
            break;
        }
        case ActionType::CheckForShiny:
            // No settings for this action.
            break;
        case ActionType::AddToEncounterNumber: {
            wxStaticText* incLabel = new wxStaticText(settingsParent, wxID_ANY, "Amount: ");
            wxSpinCtrl* incSpin = new wxSpinCtrl(settingsParent, wxID_ANY, wxEmptyString,
                wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 0, 1000, m_currentAction.encounterIncrement);
            incSpin->Bind(wxEVT_SPINCTRL, &MacroEditorFrame::OnEncounterIncrementChanged, this);

            m_actionSettingSizer->Add(incLabel, wxSizerFlags().Center());
            m_actionSettingSizer->Add(incSpin, wxSizerFlags().Center().Border(wxLEFT, 5));
            break;
        }
        default: {
            wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, "Invalid Action");
            m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
            break;
        }
    }

    settingsParent->Layout();
}

void MacroEditorFrame::CommitIfEditingExisting() {
    if (m_selectedActionIndex < 0) return; //Action being edited with no selection, not commited until add action is hit
    m_macro.UpdateAction((size_t)m_selectedActionIndex, m_currentAction);
    RefreshMacroListBox();
}

void MacroEditorFrame::RefreshMacroListBox() {
    m_macroCommandsBox->Clear();
    for (const auto& action : m_macro.GetActions()) {
        m_macroCommandsBox->Append(action.ToDisplayString());
    }
    if (m_selectedActionIndex >= 0 && m_selectedActionIndex < (int)m_macroCommandsBox->GetCount()) {
        m_macroCommandsBox->SetSelection(m_selectedActionIndex);
    }
}

void MacroEditorFrame::OnActionChoice(wxCommandEvent& evt) {
    wxWindow* settingsParent = static_cast<wxWindow*>(evt.GetEventObject())->GetParent();
    //fresh defaults for the newly chosen type
    m_currentAction = MacroAction{}; 

    PopulateActionSettings(settingsParent, evt.GetString());
    CommitIfEditingExisting();
}

void MacroEditorFrame::OnButtonChoiceChanged(wxCommandEvent& evt) {
    m_currentAction.button = (ControllerButton)evt.GetSelection();
    CommitIfEditingExisting();
}

void MacroEditorFrame::OnJoystickXChanged(wxSpinEvent& evt) {
    m_currentAction.joystickX = evt.GetPosition();
    CommitIfEditingExisting();
}

void MacroEditorFrame::OnJoystickYChanged(wxSpinEvent& evt) {
    m_currentAction.joystickY = evt.GetPosition();
    CommitIfEditingExisting();
}

void MacroEditorFrame::OnDelayChanged(wxSpinEvent& evt) {
    m_currentAction.delayMs = evt.GetPosition();
    CommitIfEditingExisting();
}

void MacroEditorFrame::OnEncounterIncrementChanged(wxSpinEvent& evt) {
    m_currentAction.encounterIncrement = evt.GetPosition();
    CommitIfEditingExisting();
}

void MacroEditorFrame::OnAddAction(wxCommandEvent& evt) {
    size_t insertIndex = (m_selectedActionIndex >= 0)
        ? (size_t)m_selectedActionIndex + 1
        : m_macro.GetActions().size();

    size_t newIndex = m_macro.InsertAction(insertIndex, m_currentAction);
    m_selectedActionIndex = (int)newIndex;
    RefreshMacroListBox();
}

void MacroEditorFrame::OnDeleteAction(wxCommandEvent& evt) {
    if (m_selectedActionIndex < 0) return;
    m_macro.RemoveAction((size_t)m_selectedActionIndex);
    m_selectedActionIndex = -1;
    RefreshMacroListBox();
}

void MacroEditorFrame::OnMacroListSelected(wxCommandEvent& evt) {
    int index = evt.GetSelection();
    if (index < 0 || index >= (int)m_macro.GetActions().size()) return;

    m_selectedActionIndex = index;
    m_currentAction = m_macro.GetActions()[index];

    wxString label = ActionTypeToChoiceLabel(m_currentAction.type);
    m_actionChoice->SetStringSelection(label);
    PopulateActionSettings(m_actionChoice->GetParent(), label);
}

void MacroEditorFrame::OnNameChanged(wxCommandEvent& evt) {
    m_macro.SetName(m_nameCtrl->GetValue());
}

void MacroEditorFrame::OnSave(wxCommandEvent& evt) {
    if (m_libraryIndex >= 0) {
        m_library->UpdateMacro((size_t)m_libraryIndex, m_macro);
    } else {
        m_libraryIndex = (int)m_library->AddMacro(m_macro);
    }

    if (!m_library->SaveToFile(m_filePath)) {
        wxMessageBox("Failed to save macros to file.", "Save Error", wxOK | wxICON_ERROR, this);
        return;
    }

    m_originalMacro = m_macro;
    Hide();
}

void MacroEditorFrame::OnCancel(wxCommandEvent& evt) {
    m_macro = m_originalMacro; 
    RefreshFromWorkingCopy();
    Hide();
}