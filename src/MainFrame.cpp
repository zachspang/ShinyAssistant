#include "MainFrame.h"
#include "ScaledBitmap.h"
#include "VideoStream.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <iostream>
#include <thread>

MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title){
    wxInitAllImageHandlers();

    m_videoStream = new VideoStream();
    m_videoStream->StartCapture();

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
    webcamChoice->SetSelection(0);

    m_videoBitmap = new ScaledBitmap(leftPanel, "C:/Users/spang/Desktop/Projects/ShinyAssistant/battletemp.png");
    m_videoTimer = new wxTimer();
    m_videoTimer->Bind(wxEVT_TIMER, &MainFrame::UpdateVideo, this);
    bool started = m_videoTimer->Start(33);

    m_encounterCounter = new wxStaticText(leftPanel, wxID_ANY, "Encounters: 100000", wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxST_ELLIPSIZE_START);
    auto font = m_encounterCounter->GetFont();
    font.SetPixelSize(wxSize(0,40));
    m_encounterCounter->SetFont(font);
    m_encounterCounter->SetMinSize(m_encounterCounter->GetBestSize());
    m_encounterCounter->SetLabel("Encounters: 0");

    //TODO: REMOVE test button
    wxButton* testDetectShiny = new wxButton(leftPanel, wxID_ANY, "run checkShiny()");
    testDetectShiny->Bind(wxEVT_BUTTON, &MainFrame::OnTestDetect, this);

    leftSizer->AddSpacer(20);
    leftSizer->Add(webcamChoice, wxSizerFlags().Center().Shaped());
    leftSizer->AddSpacer(20);
    leftSizer->Add(m_videoBitmap, wxSizerFlags().Expand().Shaped().Border(wxALL, 10).Center());
    leftSizer->Add(m_encounterCounter, wxSizerFlags().CenterHorizontal().Border(wxLEFT|wxRIGHT, 5));
    //TODO: REMOVE test button
    leftSizer->Add(testDetectShiny);
    
    
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


    wxBoxSizer* macroChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* macroChoiceLabel = new wxStaticText(rightPanel, wxID_ANY, "Selected Macro: ");
    wxArrayString macroList;
    macroList.Add("None");
    /*
    for macro in saved macros
        macroList.add(macro);
    */
    wxChoice* macroChoice = new wxChoice(rightPanel, wxID_ANY, wxDefaultPosition, wxSize(100,-1), macroList);
    macroChoice->SetSelection(0);
    macroChoiceSizer->Add(macroChoiceLabel, wxSizerFlags().CenterVertical());
    macroChoiceSizer->Add(macroChoice, wxSizerFlags().CenterVertical());
    macroChoice->Bind(wxEVT_CHOICE, &MainFrame::OnMacroChange, this);

    wxButton* editMacroButton = new wxButton(rightPanel, wxID_ANY, "Edit Macros");
    editMacroButton->Bind(wxEVT_BUTTON, &MainFrame::OnEditMacro, this);

    //TODO: Move detection sizing controls
    
    // wxSpinCtrl* detectionX = new wxSpinCtrl(rightPanel, wxID_ANY, "78", wxDefaultPosition, wxSize(100, -1), wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 4000, 78);
    // detectionX->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnDetectionXUpdate, this);
    // detectionX->Bind(wxEVT_SPINCTRL, &MainFrame::OnDetectionXUpdate, this);

    // wxSpinCtrl* detectionY = new wxSpinCtrl(rightPanel, wxID_ANY, "365", wxDefaultPosition, wxSize(100, -1), wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 4000, 365);
    // detectionY->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnDetectionYUpdate, this);
    // detectionY->Bind(wxEVT_SPINCTRL, &MainFrame::OnDetectionYUpdate, this);

    // wxSpinCtrl* detectionW = new wxSpinCtrl(rightPanel, wxID_ANY, "293", wxDefaultPosition, wxSize(100, -1), wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 4000, 293);
    // detectionW->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnDetectionWUpdate, this);
    // detectionW->Bind(wxEVT_SPINCTRL, &MainFrame::OnDetectionWUpdate, this);

    // wxSpinCtrl* detectionH = new wxSpinCtrl(rightPanel, wxID_ANY, "51", wxDefaultPosition, wxSize(100, -1), wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 4000, 51);
    // detectionH->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnDetectionHUpdate, this);
    // detectionH->Bind(wxEVT_SPINCTRL, &MainFrame::OnDetectionHUpdate, this);

    //


    rightSizer->AddSpacer(20);
    rightSizer->Add(encounterCtrlSizer, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(vcCheckBox, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(m_controllerTypeRadioBox, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(deviceIPSizer, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(macroChoiceSizer, wxSizerFlags().Border(wxALL, 5).Center());
    rightSizer->Add(editMacroButton, wxSizerFlags().Border(wxALL, 5).Center());
    // rightSizer->Add(detectionX);
    // rightSizer->Add(detectionY);
    // rightSizer->Add(detectionW);
    // rightSizer->Add(detectionH);

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

//
// Event handler implementations
//

void MainFrame::OnEncounterUpdate(wxSpinEvent& evt){
    int val = evt.GetValue();
    m_encounterCounter->SetLabel(wxString::Format("Encounters: %d", val));
    m_encounterCounter->SetMinSize(m_encounterCounter->GetBestSize());
};

void MainFrame::OnVCToggle(wxCommandEvent& evt){
    m_vcEnabled = evt.IsChecked();

    m_controllerTypeRadioBox->Enable(m_vcEnabled);
    m_deviceIPCtrl->Enable(m_vcEnabled && m_controllerType != xinput);
    m_deviceIPConfirm->Enable(m_vcEnabled && m_controllerType != xinput);
}

void MainFrame::OnVCTypeChange(wxCommandEvent& evt){
    m_controllerType = static_cast<ControllerType>(evt.GetSelection());
    m_deviceIPCtrl->Enable(m_vcEnabled && m_controllerType != xinput);
    m_deviceIPConfirm->Enable(m_vcEnabled && m_controllerType != xinput);
    //TODO: disconnect if currently connected
}

void MainFrame::OnIPConfirm(wxCommandEvent& evt){
    m_deviceIP = m_deviceIPCtrl->GetValue();
    //TODO: attempt to connect based on output mode
    wxString port = ":1234";
    wxMessageBox(wxString::Format("Attempting to connect to %s%s", m_deviceIP, port));
}

void MainFrame::UpdateVideo(wxTimerEvent& evt){
    m_videoBitmap->SetImage(m_videoStream->GetWxImageFromFrame());
}

//TODO: Remove this, checkShiny() will get called from the macro thread and actually handle the result
void MainFrame::OnTestDetect(wxCommandEvent& evt){
    std::thread detectionThread([this]() {
        m_videoStream->checkShiny();
    });
    detectionThread.detach();
}

void MainFrame::OnDetectionXUpdate(wxCommandEvent& evt){
    wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(evt.GetEventObject());
    int value = spin->GetValue();
    m_videoStream->m_rectX = value;
}

void MainFrame::OnDetectionYUpdate(wxCommandEvent& evt){
    wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(evt.GetEventObject());
    int value = spin->GetValue();
    m_videoStream->m_rectY = value;
}

void MainFrame::OnDetectionWUpdate(wxCommandEvent& evt){
    wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(evt.GetEventObject());
    int value = spin->GetValue();
    m_videoStream->m_rectW = value; 
}

void MainFrame::OnDetectionHUpdate(wxCommandEvent& evt){
    wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(evt.GetEventObject());
    int value = spin->GetValue();
    m_videoStream->m_rectH = value;
}

void MainFrame::OnMacroChange(wxCommandEvent& evt){
    //TODO: implement this
}

void MainFrame::OnEditMacro(wxCommandEvent& evt){
    //TODO: implement this
}