#include "MacroEditorFrame.h"

MacroEditorFrame::MacroEditorFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, "Macro Editor") {
    m_panel = new wxPanel(this);
    Bind(wxEVT_CLOSE_WINDOW, &MacroEditorFrame::OnClose, this);

    wxArrayString macroCommands;
    //TODO: Load from saved macros

    wxListBox* macroCommandsBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(100,300), macroCommands, wxLB_ALWAYS_SB);

    wxPanel* rightPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100,300));

    actionList.Add("Press Button");
    actionList.Add("Release Button");
    actionList.Add("Move Joystick");
    actionList.Add("Delay");
    actionList.Add("Check For Shiny");
    actionList.Add("Add to encounter number");

    wxBoxSizer* actionChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* actionLabel = new wxStaticText(rightPanel, wxID_ANY, "Action: ");
    wxChoice* actionChoice = new wxChoice(rightPanel, wxID_ANY, wxDefaultPosition, wxSize(200,-1), actionList);
    actionChoice->Bind(wxEVT_CHOICE, &MacroEditorFrame::OnActionChoice, this);
    actionChoice->SetSelection(0);
    actionChoiceSizer->Add(actionLabel, wxSizerFlags().Center());
    actionChoiceSizer->Add(actionChoice, wxSizerFlags().Center());

    m_actionSettingSizer = new wxBoxSizer(wxHORIZONTAL);
    //Initialize default settings shown
    PopulateActionSettings(rightPanel, actionList[0]);
    //m_actionSettingSizer's children are changed by OnActionChoice

    wxButton* addButton = new wxButton(rightPanel, wxID_SAVE, "Add action");
    wxButton* deleteButton = new wxButton(rightPanel, wxID_SAVE, "Delete action");
    wxBoxSizer* addDeleteSizer = new wxBoxSizer(wxHORIZONTAL);
    addDeleteSizer->Add(addButton, wxSizerFlags().Border(wxALL, 5));
    addDeleteSizer->Add(deleteButton, wxSizerFlags().Border(wxALL, 5));

    wxButton* saveButton = new wxButton(rightPanel, wxID_SAVE, "Save");
    wxButton* closeButton = new wxButton(rightPanel, wxID_CLOSE, "Close");

    wxBoxSizer* saveCloseSizer = new wxBoxSizer(wxHORIZONTAL);
    saveCloseSizer->AddStretchSpacer(1);
    saveCloseSizer->Add(saveButton, wxSizerFlags().Border(wxALL, 5));
    saveCloseSizer->Add(closeButton, wxSizerFlags().Border(wxALL, 5));

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
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
    boxSizer->Add(macroCommandsBox, wxSizerFlags().Proportion(1).Expand());
    boxSizer->AddSpacer(2);
    boxSizer->Add(rightPanel, wxSizerFlags().Proportion(1).Expand());
    SetSizerAndFit(boxSizer);
    boxSizer->SetSizeHints(this);

}

void MacroEditorFrame::OnClose(wxCloseEvent& evt) {
    evt.Skip(); 
}

void MacroEditorFrame::OnActionChoice(wxCommandEvent& evt) {
    wxWindow* settingsParent = static_cast<wxWindow*>(evt.GetEventObject())->GetParent();
    PopulateActionSettings(settingsParent, evt.GetString());
}

void MacroEditorFrame::PopulateActionSettings(wxWindow* settingsParent, const wxString& actionName) {
    //Delete m_actionSettingSizer's children
    m_actionSettingSizer->Clear(true); 

    if (actionName == "Press Button"){
        wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, "PButton: ");
        m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
    } else if (actionName == "Release Button") {
        wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, "RButton: ");
        m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
    } else if (actionName == "Move Joystick") {
        wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, "Stick: ");
        m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
    } else if (actionName == "Delay") {
        wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, "Delay: ");
        m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
    } else if (actionName == "Check For Shiny") {
        wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, "Check: ");
        m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
    } else if (actionName == "Add to encounter number") {
        wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, "Inc C: ");
        m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
    } else {
        wxStaticText* actionSettingLabel = new wxStaticText(settingsParent, wxID_ANY, "Invalid Action");
        m_actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());
    }

    settingsParent->Layout();

    //Might not need? Test resizing after adding all settings
    //settingsParent->Fit();          //resize settingsParent to fit new sizer content
    //GetSizer()->Layout();       //relayout the frame's own sizer too
}