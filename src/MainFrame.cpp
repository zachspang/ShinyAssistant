#include "MainFrame.h"
#include "ScaledStaticBitmap.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>

MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title){
    wxInitAllImageHandlers();

    //Panels splitting gui into left and right halves

    wxPanel* leftPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(200,300));
    leftPanel->SetBackgroundColour(*wxWHITE);
    wxPanel* rightPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(200,300));
    rightPanel->SetBackgroundColour(*wxWHITE);

    //
    //Left Panel Children
    //

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString webcams;
    //TODO: populate webcams with actual user devices
    webcams.Add("webcam 1");
    webcams.Add("webcam 2");
    wxChoice* webcamChoice = new wxChoice(leftPanel, wxID_ANY, wxDefaultPosition, wxSize(100, 40), webcams);

    //TODO: Temp image to represent area where video stream would go
    ScaledStaticBitmap* videoBitmap = new ScaledStaticBitmap(leftPanel, "C:/Users/spang/Desktop/Projects/ShinyAssistant/battletemp.png");
    
    m_encounterCounter = new wxStaticText(leftPanel, wxID_ANY, "Encounters: 100000", wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxST_ELLIPSIZE_START);
    auto font = m_encounterCounter->GetFont();
    font.SetPixelSize(wxSize(0,40));
    m_encounterCounter->SetFont(font);
    m_encounterCounter->SetMinSize(m_encounterCounter->GetBestSize());
    m_encounterCounter->SetLabel("Encounters: 0");

    leftSizer->AddSpacer(20);
    leftSizer->Add(webcamChoice, wxSizerFlags().Center().Shaped());
    leftSizer->AddSpacer(20);
    leftSizer->Add(videoBitmap, wxSizerFlags().Expand().Shaped().Border(wxALL, 10).Center());
    leftSizer->Add(m_encounterCounter, wxSizerFlags().CenterHorizontal().Border(wxLEFT|wxRIGHT, 5));
    
    
    leftPanel->SetSizer(leftSizer);
    leftSizer->SetSizeHints(leftPanel);
    ///
    //End of Left Children
    //

    //
    //Right Panel Children
    //

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* encounterCtrlSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* encounterCtrlLabel = new wxStaticText(rightPanel, wxID_ANY, "Set Encounters: ");
    wxSpinCtrl* encounterCtrl = new wxSpinCtrl(rightPanel, wxID_ANY, "0", wxDefaultPosition, wxSize(100, -1), wxSP_WRAP, 0, 100000, 0);
    encounterCtrlSizer->Add(encounterCtrlLabel, wxSizerFlags().CenterVertical());
    encounterCtrlSizer->Add(encounterCtrl, wxSizerFlags().CenterVertical());
    encounterCtrl->Bind(wxEVT_SPINCTRL, &MainFrame::OnEncounterUpdate, this);

    wxCheckBox* vcCheckBox = new wxCheckBox(rightPanel, wxID_ANY, "Enable Virtual Controller");
    vcCheckBox->Bind(wxEVT_CHECKBOX, &MainFrame::OnVCToggle, this);

    wxArrayString controllerTypes;
    controllerTypes.Add("XInput");
    controllerTypes.Add("sys-botbase ");
    controllerTypes.Add("Rosalina IR");
    m_controllerTypeRadioBox = new wxRadioBox(rightPanel,wxID_ANY, "Controller Mode", wxDefaultPosition, wxDefaultSize, controllerTypes);
    m_controllerTypeRadioBox->Bind(wxEVT_RADIOBOX, &MainFrame::OnVCTypeChange, this);
    m_controllerTypeRadioBox->Enable(false);

    wxBoxSizer* deviceIPSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* deviceIPLabel = new wxStaticText(rightPanel, wxID_ANY, "Device IP: ");
    m_deviceIPCtrl = new wxTextCtrl(rightPanel, wxID_ANY, "192.168.123.456", wxDefaultPosition, wxSize(125, -1));
    m_deviceIPCtrl->Enable(false);
    m_deviceIPConfirm = new wxButton(rightPanel, wxID_ANY, "Connect");
    deviceIPSizer->Add(deviceIPLabel, wxSizerFlags().CenterVertical());
    deviceIPSizer->Add(m_deviceIPCtrl, wxSizerFlags().CenterVertical());
    deviceIPSizer->Add(m_deviceIPConfirm, wxSizerFlags().CenterVertical().Border(wxALL,5));
    m_deviceIPConfirm->Bind(wxEVT_BUTTON, &MainFrame::OnIPConfirm, this);
    m_deviceIPConfirm->Enable(false);

    rightSizer->AddSpacer(20);
    rightSizer->Add(encounterCtrlSizer, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(vcCheckBox, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(m_controllerTypeRadioBox, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(deviceIPSizer, wxSizerFlags().Border(wxALL, 10).Center());

    rightPanel->SetSizer(rightSizer);
    rightSizer->SetSizeHints(rightPanel);
    //
    //End of Right Children
    //

    wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(leftPanel, wxSizerFlags().Proportion(3).Expand());
    boxSizer->AddSpacer(2);
    boxSizer->Add(rightPanel, wxSizerFlags().Proportion(2).Expand());
    SetSizerAndFit(boxSizer);
    boxSizer->SetSizeHints(this);
}

void MainFrame::OnEncounterUpdate(wxSpinEvent& evt){
    int val = evt.GetValue();
    m_encounterCounter->SetLabel(wxString::Format("Encounters: %d", val));
    m_encounterCounter->SetMinSize(m_encounterCounter->GetBestSize());
};

void MainFrame::OnVCToggle(wxCommandEvent& evt){
    m_vcEnabled = evt.IsChecked();

    m_controllerTypeRadioBox->Enable(m_vcEnabled);
    m_deviceIPCtrl->Enable(m_vcEnabled);
    m_deviceIPConfirm->Enable(m_vcEnabled);
}

void MainFrame::OnVCTypeChange(wxCommandEvent& evt){
    m_controllerType = static_cast<ControllerType>(evt.GetSelection());
    //TODO: disconnect if currently connected
}

void MainFrame::OnIPConfirm(wxCommandEvent& evt){
    m_deviceIP = m_deviceIPCtrl->GetValue();
    //TODO: attempt to connect based on output mode
    wxString port = ":1234";
    wxMessageBox(wxString::Format("Attempting to connect to %s%s", m_deviceIP, port));
}