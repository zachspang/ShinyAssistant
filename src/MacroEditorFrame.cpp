#include "MacroEditorFrame.h"

MacroEditorFrame::MacroEditorFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, "Macro Editor") {
    m_panel = new wxPanel(this);
    Bind(wxEVT_CLOSE_WINDOW, &MacroEditorFrame::OnClose, this);

    wxArrayString macroCommands;
    //TODO: Load from saved macros

    wxListBox* macroCommandsBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(100,300), macroCommands, wxLB_ALWAYS_SB);

    wxPanel* rightPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100,300));

    wxArrayString actionList;
    actionList.Add("Press Button");
    actionList.Add("Release Button");
    actionList.Add("Move Joystick");
    actionList.Add("Delay");
    actionList.Add("Check For Shiny");
    actionList.Add("Add to encounter number");

    wxBoxSizer* actionChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* actionLabel = new wxStaticText(rightPanel, wxID_ANY, "Action: ");
    wxChoice* actionChoice = new wxChoice(rightPanel, wxID_ANY, wxDefaultPosition, wxSize(200,-1), actionList);
    actionChoice->SetSelection(0);
    actionChoiceSizer->Add(actionLabel, wxSizerFlags().Center());
    actionChoiceSizer->Add(actionChoice, wxSizerFlags().Center());

    wxBoxSizer* actionSettingSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* actionSettingLabel = new wxStaticText(rightPanel, wxID_ANY, "Something: ");
    //Dynamic control here, wxChoice for buttons, Two wxSpinCtrl for x,y of joystick, wxSpinCtrl for delay
    //Nothing for check shiny, wxSpinCtrl for Inc counter. Labels also dynamic. Maybe have premade sizers for each action
    //and swap them out when action changes?
    actionSettingSizer->Add(actionSettingLabel, wxSizerFlags().Center());

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
    rightSizer->Add(actionSettingSizer, wxSizerFlags().Center());
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