#include "MainFrame.h"
#include "ScaledBitmap.h"
#include "VideoStream.h"
#include "MacroEditorFrame.h"
#include "VirtualXInput.h"
#include "VirtualRosalina.h"
#include "VirtualBotbase.h"
#include "Logging.h"
#include "WebhookNotifier.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/textctrl.h>
#include <iostream>
#include <thread>

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
    
    //Anything that calls Log() must call it after this block or there is a risk of crashing/freezing the thread that calls log
    m_logCtrl = new wxTextCtrl(leftPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 150), wxTE_MULTILINE | wxTE_READONLY);
    m_logCtrl->SetMinSize(wxSize(-1, 150));
    //wxLogTextCtrl takes ownership; wxWidgets deletes it at shutdown
    wxLog::SetActiveTarget(new wxLogTextCtrl(m_logCtrl));

    //Settings are loaded so widget construction below can use them as defaults.
    //This isnt a child of the left panel but it Logs so it has to run after wxLog::SetActiveTarget(new wxLogTextCtrl(m_logCtrl));
    wxString settingsDir = wxStandardPaths::Get().GetUserDataDir();
    wxFileName::Mkdir(settingsDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    m_settingsFilePath = settingsDir + wxFILE_SEP_PATH + "settings.json";
    m_settings.LoadFromFile(m_settingsFilePath); //this fails if no settings saved yet which is ok

    m_videoStream = new VideoStream();
    int webcamCount = m_videoStream->GetWebcamCount();

    //Default the saved webcam index back to 0 if it's out of range for the webcams actually detected
    if (m_settings.selectedWebcamIndex < 0 || m_settings.selectedWebcamIndex >= webcamCount) {
        m_settings.selectedWebcamIndex = 0;
    }
    m_videoStream->StartCapture(m_settings.selectedWebcamIndex);

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString webcams;
    
    for (int i = 1; i <= webcamCount; i++){
        webcams.Add(wxString::Format("Webcam #%d", i));
    }

    m_webcamChoice = new wxChoice(leftPanel, wxID_ANY, wxDefaultPosition, wxSize(100, 40), webcams);
    m_webcamChoice->SetSelection(m_settings.selectedWebcamIndex);
    m_webcamChoice->Bind(wxEVT_CHOICE, &MainFrame::OnWebcamChanged, this);

    m_videoBitmap = new ScaledBitmap(leftPanel);
    m_videoTimer = new wxTimer();
    m_videoTimer->Bind(wxEVT_TIMER, &MainFrame::UpdateVideo, this);
    bool started = m_videoTimer->Start(33);

    m_encounterCounter = new wxStaticText(leftPanel, wxID_ANY, "Encounters: 100000", wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxST_ELLIPSIZE_START);
    auto font = m_encounterCounter->GetFont();
    font.SetPixelSize(wxSize(0,40));
    m_encounterCounter->SetFont(font);
    m_encounterCounter->SetMinSize(m_encounterCounter->GetBestSize());
    m_encounterCounter->SetLabel(wxString::Format("Encounters: %d", m_encounterValue));

    m_macroToggleButton = new wxButton(leftPanel, wxID_ANY, "Start Macro");
    m_macroToggleButton->Bind(wxEVT_BUTTON, &MainFrame::OnMacroToggle, this);

    leftSizer->AddSpacer(20);
    leftSizer->Add(m_webcamChoice, wxSizerFlags().Center().Shaped());
    leftSizer->AddSpacer(20);
    leftSizer->Add(m_videoBitmap, wxSizerFlags().Expand().Shaped().Border(wxALL, 10).Center());
    leftSizer->Add(m_encounterCounter, wxSizerFlags().CenterHorizontal().Border(wxLEFT|wxRIGHT, 5));
    leftSizer->Add(m_logCtrl, wxSizerFlags().Expand().Border(wxALL, 5));
    leftSizer->Add(m_macroToggleButton, wxSizerFlags().Center());
    
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
    m_encounterCtrl = new wxSpinCtrl(rightPanel, wxID_ANY, wxString::Format("%d", m_encounterValue), wxDefaultPosition, wxSize(100, -1), wxSP_WRAP, 0, 100000, m_encounterValue);
    encounterCtrlSizer->Add(encounterCtrlLabel, wxSizerFlags().CenterVertical());
    encounterCtrlSizer->Add(m_encounterCtrl, wxSizerFlags().CenterVertical());
    m_encounterCtrl->Bind(wxEVT_SPINCTRL, &MainFrame::OnEncounterCtrlUpdate, this);

    wxCheckBox* vcCheckBox = new wxCheckBox(rightPanel, wxID_ANY, "Enable Virtual Controller");
    vcCheckBox->Bind(wxEVT_CHECKBOX, &MainFrame::OnVCToggle, this);

    wxArrayString controllerTypes;
    controllerTypes.Add("sys-botbase   ");
    controllerTypes.Add("Rosalina IR");
    controllerTypes.Add("XInput");
    m_controllerTypeRadioBox = new wxRadioBox(rightPanel,wxID_ANY, "Controller Mode", wxDefaultPosition, wxDefaultSize, controllerTypes);
    m_controllerTypeRadioBox->Bind(wxEVT_RADIOBOX, &MainFrame::OnVCTypeChange, this);

    wxBoxSizer* deviceIPSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* deviceIPLabel = new wxStaticText(rightPanel, wxID_ANY, "Device IP: ");
    m_deviceIPCtrl = new wxTextCtrl(rightPanel, wxID_ANY, "192.168.123.456", wxDefaultPosition, wxSize(125, -1));
    m_deviceIPCtrl->Enable(false);
    m_deviceIPConfirm = new wxButton(rightPanel, wxID_ANY, "Connect");
    deviceIPSizer->Add(deviceIPLabel, wxSizerFlags().CenterVertical());
    deviceIPSizer->Add(m_deviceIPCtrl, wxSizerFlags().CenterVertical());
    deviceIPSizer->Add(m_deviceIPConfirm, wxSizerFlags().CenterVertical().Border(wxALL,5));
    m_deviceIPConfirm->Bind(wxEVT_BUTTON, &MainFrame::OnIPConfirm, this);

    wxStaticBoxSizer* webhookSizer = new wxStaticBoxSizer(wxVERTICAL, rightPanel, "Discord Alert");

    wxBoxSizer* webhookUrlRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* webhookUrlLabel = new wxStaticText(rightPanel, wxID_ANY, "Webhook URL: ");
    m_webhookUrlCtrl = new wxTextCtrl(rightPanel, wxID_ANY, m_settings.webhookUrl, wxDefaultPosition, wxSize(200, -1), wxTE_PASSWORD);
    m_webhookUrlCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { SaveSettings(); });
    webhookUrlRow->Add(webhookUrlLabel, wxSizerFlags().CenterVertical());
    webhookUrlRow->Add(m_webhookUrlCtrl, wxSizerFlags().CenterVertical());

    wxBoxSizer* webhookUserRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* webhookUserLabel = new wxStaticText(rightPanel, wxID_ANY, "User ID: ");
    m_webhookUsernameCtrl = new wxTextCtrl(rightPanel, wxID_ANY, m_settings.userID, wxDefaultPosition, wxSize(200, -1), wxTE_PASSWORD);
    m_webhookUsernameCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { SaveSettings(); });
    webhookUserRow->Add(webhookUserLabel, wxSizerFlags().CenterVertical());
    webhookUserRow->Add(m_webhookUsernameCtrl, wxSizerFlags().CenterVertical());

    webhookSizer->Add(webhookUrlRow, wxSizerFlags().Border(wxALL, 5));
    webhookSizer->Add(webhookUserRow, wxSizerFlags().Border(wxALL, 5));

    //Macros are all stored in a single file, loaded once here and shared with the editor
    wxString macroDir = wxStandardPaths::Get().GetUserDataDir();
    wxFileName::Mkdir(macroDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    m_macroFilePath = macroDir + wxFILE_SEP_PATH + "macros.json";
    m_macroLibrary.LoadFromFile(m_macroFilePath); //this fails if no macros are made which is ok

    wxBoxSizer* macroChoiceSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* macroChoiceLabel = new wxStaticText(rightPanel, wxID_ANY, "Selected Macro: ");
    m_macroChoice = new wxChoice(rightPanel, wxID_ANY, wxDefaultPosition, wxSize(100,-1));
    macroChoiceSizer->Add(macroChoiceLabel, wxSizerFlags().CenterVertical());
    macroChoiceSizer->Add(m_macroChoice, wxSizerFlags().CenterVertical());
    m_macroChoice->Bind(wxEVT_CHOICE, &MainFrame::OnMacroChange, this);

    wxButton* editMacroButton = new wxButton(rightPanel, wxID_ANY, "Edit Macro");
    editMacroButton->Bind(wxEVT_BUTTON, &MainFrame::OnEditMacro, this);

    wxButton* createMacroButton = new wxButton(rightPanel, wxID_ANY, "Create New Macro");
    createMacroButton->Bind(wxEVT_BUTTON, &MainFrame::OnCreateMacro, this);

    wxButton* deleteMacroButton = new wxButton(rightPanel, wxID_ANY, "Delete Macro");
    deleteMacroButton->Bind(wxEVT_BUTTON, &MainFrame::OnDeleteMacro, this);

    wxBoxSizer* macroButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    macroButtonSizer->Add(createMacroButton, wxSizerFlags().Border(wxALL, 5));
    macroButtonSizer->Add(deleteMacroButton, wxSizerFlags().Border(wxALL, 5));

    m_macroEditorFrame = new MacroEditorFrame(this, &m_macroLibrary, m_macroFilePath);
    m_macroEditorFrame->Bind(wxEVT_SHOW, &MainFrame::OnMacroEditorClosed, this);

    RefreshMacroChoice();

    //Restore the previously selected macro if it's still a valid index
    if (m_settings.selectedMacroIndex >= 0 && m_settings.selectedMacroIndex < (int)m_macroLibrary.GetMacros().size()) {
        m_macroChoice->SetSelection(m_settings.selectedMacroIndex);
        UpdateSelectedMacroPointer();
    }

    wxBoxSizer* detectionXYSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* detectionXLabel = new wxStaticText(rightPanel, wxID_ANY, "X: ");
    m_detectionXCtrl = new wxSpinCtrl(rightPanel, wxID_ANY, "78", wxDefaultPosition, wxSize(80, -1), wxSP_WRAP, 0, 4000, 78);
    m_detectionXCtrl->Bind(wxEVT_SPINCTRL, &MainFrame::OnDetectionRectChanged, this);
    wxStaticText* detectionYLabel = new wxStaticText(rightPanel, wxID_ANY, "Y: ");
    m_detectionYCtrl = new wxSpinCtrl(rightPanel, wxID_ANY, "365", wxDefaultPosition, wxSize(80, -1), wxSP_WRAP, 0, 4000, 365);
    m_detectionYCtrl->Bind(wxEVT_SPINCTRL, &MainFrame::OnDetectionRectChanged, this);
    detectionXYSizer->Add(detectionXLabel, wxSizerFlags().CenterVertical());
    detectionXYSizer->Add(m_detectionXCtrl, wxSizerFlags().CenterVertical().Border(wxRIGHT, 10));
    detectionXYSizer->Add(detectionYLabel, wxSizerFlags().CenterVertical());
    detectionXYSizer->Add(m_detectionYCtrl, wxSizerFlags().CenterVertical());

    wxBoxSizer* detectionWHSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* detectionWLabel = new wxStaticText(rightPanel, wxID_ANY, "W: ");
    m_detectionWCtrl = new wxSpinCtrl(rightPanel, wxID_ANY, "293", wxDefaultPosition, wxSize(80, -1), wxSP_WRAP, 0, 4000, 293);
    m_detectionWCtrl->Bind(wxEVT_SPINCTRL, &MainFrame::OnDetectionRectChanged, this);
    wxStaticText* detectionHLabel = new wxStaticText(rightPanel, wxID_ANY, "H: ");
    m_detectionHCtrl = new wxSpinCtrl(rightPanel, wxID_ANY, "51", wxDefaultPosition, wxSize(80, -1), wxSP_WRAP, 0, 4000, 51);
    m_detectionHCtrl->Bind(wxEVT_SPINCTRL, &MainFrame::OnDetectionRectChanged, this);
    detectionWHSizer->Add(detectionWLabel, wxSizerFlags().CenterVertical());
    detectionWHSizer->Add(m_detectionWCtrl, wxSizerFlags().CenterVertical().Border(wxRIGHT, 10));
    detectionWHSizer->Add(detectionHLabel, wxSizerFlags().CenterVertical());
    detectionWHSizer->Add(m_detectionHCtrl, wxSizerFlags().CenterVertical());

    wxStaticBoxSizer* detectionBoundsSizer = new wxStaticBoxSizer(wxVERTICAL, rightPanel, "Detection Bounds");
    detectionBoundsSizer->Add(detectionXYSizer, wxSizerFlags().Border(wxALL, 5).Center());
    detectionBoundsSizer->Add(detectionWHSizer, wxSizerFlags().Border(wxALL, 5).Center());

    //Load per-macro encounter/detection settings (or built-in defaults) for whichever macro ended up selected above.
    //This sets m_encounterValue, the detection spin controls, and pushes bounds into VideoStream.
    LoadMacroSettings(m_macroChoice->GetSelection());

    rightSizer->AddSpacer(20);
    rightSizer->Add(encounterCtrlSizer, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(detectionBoundsSizer, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(macroChoiceSizer, wxSizerFlags().Border(wxALL, 5).Center());
    rightSizer->Add(macroButtonSizer, wxSizerFlags().Center());
    rightSizer->Add(editMacroButton, wxSizerFlags().Border(wxALL, 5).Center());
    rightSizer->Add(vcCheckBox, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(m_controllerTypeRadioBox, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(deviceIPSizer, wxSizerFlags().Border(wxALL, 10).Center());
    rightSizer->Add(webhookSizer, wxSizerFlags().Border(wxALL, 10).Center());

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
// Helper Implementations
//

void MainFrame::UpdateSelectedMacroPointer() {
    int index = m_macroChoice->GetSelection();
    auto& macros = m_macroLibrary.GetMacros();

    if (index == wxNOT_FOUND || index >= (int)macros.size()) {
        m_selectedMacro = nullptr;
        return;
    }

    m_selectedMacro = &macros[(size_t)index];
}

void MainFrame::RefreshMacroChoice() {
    wxString previouslySelected = m_macroChoice->GetStringSelection();

    wxArrayString names = m_macroLibrary.GetMacroNames();
    m_macroChoice->Set(names);

    if (!names.IsEmpty()) {
        int restoredIndex = m_macroChoice->FindString(previouslySelected);
        m_macroChoice->SetSelection(restoredIndex != wxNOT_FOUND ? restoredIndex : 0);
    }

    UpdateSelectedMacroPointer();
}

void MainFrame::LoadMacroSettings(int macroIndex) {
    MacroSettings ms; // defaults if nothing saved yet for this index

    auto savedEntry = m_settings.macroSettings.find(macroIndex);
    bool hasSavedSettings = (macroIndex >= 0 && savedEntry != m_settings.macroSettings.end());
    if (hasSavedSettings) {
        ms = savedEntry->second;
    }

    m_lastSettingsMacroIndex = macroIndex;

    m_encounterValue = ms.encounterValue;
    m_encounterCtrl->SetValue(m_encounterValue);
    m_encounterCounter->SetLabel(wxString::Format("Encounters: %d", m_encounterValue));
    m_encounterCounter->SetMinSize(m_encounterCounter->GetBestSize());

    m_detectionXCtrl->SetValue(ms.detectionX);
    m_detectionYCtrl->SetValue(ms.detectionY);
    m_detectionWCtrl->SetValue(ms.detectionW);
    m_detectionHCtrl->SetValue(ms.detectionH);

    m_videoStream->m_rectX = ms.detectionX;
    m_videoStream->m_rectY = ms.detectionY;
    m_videoStream->m_rectW = ms.detectionW;
    m_videoStream->m_rectH = ms.detectionH;
}

void MainFrame::SaveSettings() {
    m_settings.webhookUrl = m_webhookUrlCtrl->GetValue();
    m_settings.userID = m_webhookUsernameCtrl->GetValue();
    m_settings.selectedMacroIndex = m_macroChoice->GetSelection();
    m_settings.selectedWebcamIndex = m_webcamChoice->GetSelection();

    if (m_lastSettingsMacroIndex >= 0) {
        MacroSettings ms;
        ms.encounterValue = m_encounterValue;
        ms.detectionX = m_detectionXCtrl->GetValue();
        ms.detectionY = m_detectionYCtrl->GetValue();
        ms.detectionW = m_detectionWCtrl->GetValue();
        ms.detectionH = m_detectionHCtrl->GetValue();
        m_settings.macroSettings[m_lastSettingsMacroIndex] = ms;
    }

    m_settings.SaveToFile(m_settingsFilePath);
}

void MainFrame::StartMacroThread() {
    if (!m_selectedMacro || m_macroRunning) return;

    if (m_controller == nullptr) {
        Log("Error: Attempted to start macro with no controller pointer");
        return;
    }

    m_macroToggleButton->SetLabel("Stop Macro");

    Macro macroCopy = *m_selectedMacro; //snapshot of macro, thread never touches the library after this
    m_macroRunning = true;

    //Lambda wrapper to pass into Macro.Play() so it can safely increment encouterCounter from a different thread
    auto OnEncounterIncrementWrapper = [this](int amount) {
        wxTheApp->CallAfter([this, amount]() {
            OnEncounterIncrement(amount);
        });
    };

    wxString webhookUrl = m_webhookUrlCtrl->GetValue();
    wxString pingName = m_webhookUsernameCtrl->GetValue();
    //Lambda wrapper for discord webhook alert
    auto OnShinyDetectedWrapper = [webhookUrl, pingName](const wxImage& frame) {
        // Blocking HTTP call is ok, the macro thread is stopping right after this anyway
        WebhookNotifier::SendShinyAlert(frame, webhookUrl, pingName);
    };

    //Reset the previous detection frame so the first detection on this run is considered the baseline
    m_videoStream->resetDetectionFrame();

    //Macro loop thread, keeps looping macro until m_macroRunning = false
    m_macroThread = std::thread([this, macroCopy, OnEncounterIncrementWrapper, OnShinyDetectedWrapper]() mutable {
        Log(wxString::Format("Macro \"%s\" started", macroCopy.GetName()));

        while (m_macroRunning) {
            bool stopMacro = macroCopy.Play(m_macroRunning, m_controller, m_videoStream, OnEncounterIncrementWrapper, OnShinyDetectedWrapper);
            if (stopMacro) {
                wxTheApp->CallAfter([this]() {
                    StopMacroThread(); //join from ui thread
                });
                break;
            }
        }
        Log(wxString::Format("Macro \"%s\" stopped", macroCopy.GetName()));
    });
}

void MainFrame::StopMacroThread() {
    m_macroToggleButton->SetLabel("Start Macro");
    m_macroRunning = false;
    if (m_macroThread.joinable()) m_macroThread.join(); //wait for it to actually stop
}

void MainFrame::SyncControllerState() {
    if (m_vcEnabled) {
        switch(m_controllerType) {
            case ControllerType::xinput:
                if (m_controller == nullptr) {
                    m_controller = new VirtualXInput();
                }
                break;
            case ControllerType::sysbotbase:
                if (m_controller == nullptr) {
                    m_controller = new VirtualBotbase();
                }
                break;
            case ControllerType::rosalinaIR:
                if (m_controller == nullptr) {
                    m_controller = new VirtualRosalina();
                }
                break;
            default:
                delete m_controller;
                m_controller = nullptr;
                Log("Error: Attempted to create unimplemented controller");
                break;
        }
    } else {
        delete m_controller;
        m_controller = nullptr;
    }
}

//
// Event handler implementations
//

void MainFrame::OnWebcamChanged(wxCommandEvent& evt){
    int deviceIndex = evt.GetSelection();
    if (deviceIndex == wxNOT_FOUND) return;

    Log(wxString::Format("Switching to webcam #%d", deviceIndex + 1));
    m_videoStream->SwitchCamera(deviceIndex);
    SaveSettings();
}

void MainFrame::OnEncounterCtrlUpdate(wxSpinEvent& evt){
    int val = evt.GetValue();
    m_encounterValue = val;
    m_encounterCounter->SetLabel(wxString::Format("Encounters: %d", val));
    m_encounterCounter->SetMinSize(m_encounterCounter->GetBestSize());
    SaveSettings();
};

void MainFrame::OnEncounterIncrement(int amount) {
    m_encounterValue += amount;
    m_encounterCounter->SetLabel(wxString::Format("Encounters: %d", m_encounterValue));
    m_encounterCounter->SetMinSize(m_encounterCounter->GetBestSize());
    m_encounterCtrl->SetValue(m_encounterValue);
    SaveSettings();
}

void MainFrame::OnVCToggle(wxCommandEvent& evt){
    m_vcEnabled = evt.IsChecked();

    m_controllerTypeRadioBox->Enable(m_vcEnabled);
    m_deviceIPCtrl->Enable(m_vcEnabled && m_controllerType != xinput);
    m_deviceIPConfirm->Enable(m_vcEnabled && m_controllerType != xinput);

    if (!m_vcEnabled && m_macroRunning) {
        StopMacroThread();
    }

    SyncControllerState();
}

void MainFrame::OnVCTypeChange(wxCommandEvent& evt){
    m_controllerType = static_cast<ControllerType>(evt.GetSelection());
    m_deviceIPCtrl->Enable(m_vcEnabled && m_controllerType != xinput);
    m_deviceIPConfirm->Enable(m_vcEnabled && m_controllerType != xinput);
    
    if (m_macroRunning) {
        StopMacroThread();
    }
    
    delete m_controller;
    m_controller = nullptr;

    SyncControllerState();
}

void MainFrame::OnIPConfirm(wxCommandEvent& evt){
    m_deviceIP = m_deviceIPCtrl->GetValue();
    
    wxString port;

    if (m_controllerType == ControllerType::rosalinaIR) {
        port = "4950";
    } else if (m_controllerType == ControllerType::sysbotbase){
        port = "6000";
    }

    Log(wxString::Format("Attempting to connect to %s:%s", m_deviceIP, port));

    if (m_controllerType == ControllerType::rosalinaIR) {
        VirtualRosalina* rosalinaPtr = dynamic_cast<VirtualRosalina*>(m_controller);

        if (rosalinaPtr != nullptr) {
            rosalinaPtr->Connect(m_deviceIP);
        }
    } else if (m_controllerType == ControllerType::sysbotbase){
        VirtualBotbase* botbasePtr = dynamic_cast<VirtualBotbase*>(m_controller);

        if (botbasePtr != nullptr) {
            botbasePtr->Connect(m_deviceIP);
        }
    }

}

void MainFrame::UpdateVideo(wxTimerEvent& evt){
    m_videoBitmap->SetImage(m_videoStream->GetWxImageFromFrame());
}

void MainFrame::OnDetectionRectChanged(wxSpinEvent&) {
    m_videoStream->m_rectX = m_detectionXCtrl->GetValue();
    m_videoStream->m_rectY = m_detectionYCtrl->GetValue();
    m_videoStream->m_rectW = m_detectionWCtrl->GetValue();
    m_videoStream->m_rectH = m_detectionHCtrl->GetValue();
    SaveSettings();
};

void MainFrame::OnMacroChange(wxCommandEvent& evt){
    StopMacroThread();
    UpdateSelectedMacroPointer();
    LoadMacroSettings(m_macroChoice->GetSelection());
    SaveSettings();
}

void MainFrame::OnEditMacro(wxCommandEvent& evt){
    int index = m_macroChoice->GetSelection();
    if (index == wxNOT_FOUND || index >= (int)m_macroLibrary.GetMacros().size()) return;

    StopMacroThread();

    m_macroEditorFrame->EditMacro((size_t)index);

    m_macroEditorFrame->Raise();
    m_macroEditorFrame->SetFocus();
    m_macroEditorFrame->Restore();
}

void MainFrame::OnCreateMacro(wxCommandEvent& evt){
    StopMacroThread();

    m_macroEditorFrame->EditNewMacro();

    m_macroEditorFrame->Raise();
    m_macroEditorFrame->SetFocus();
    m_macroEditorFrame->Restore();
}

void MainFrame::OnDeleteMacro(wxCommandEvent& evt){
    int index = m_macroChoice->GetSelection();
    if (index == wxNOT_FOUND || index >= (int)m_macroLibrary.GetMacros().size()) return;

    wxString name = m_macroLibrary.GetMacros()[index].GetName();
    int result = wxMessageBox(wxString::Format("Delete macro \"%s\"? This cannot be undone.", name),
        "Delete Macro", wxYES_NO | wxICON_WARNING, this);
    if (result != wxYES) return;

    StopMacroThread();

    m_macroLibrary.RemoveMacro((size_t)index);
    m_macroLibrary.SaveToFile(m_macroFilePath);

    //Reindex per-macro settings, drop the deleted index then shift every index above it down by 1
    std::map<int, MacroSettings> reindexed;
    for (const auto& [key, value] : m_settings.macroSettings) {
        if (key == index) continue;
        int newKey = (key > index) ? key - 1 : key;
        reindexed[newKey] = value;
    }
    m_settings.macroSettings = reindexed;

    RefreshMacroChoice();
    LoadMacroSettings(m_macroChoice->GetSelection());
    SaveSettings();
}

void MainFrame::OnMacroEditorClosed(wxShowEvent& evt) {
    evt.Skip();
    m_macroEditorOpen = evt.IsShown();
    m_macroToggleButton->Enable(!m_macroEditorOpen);

    if (evt.IsShown()) return; //only refresh once the editor is dismissed, not when it opens
    RefreshMacroChoice();
    LoadMacroSettings(m_macroChoice->GetSelection());
    SaveSettings();
}

void MainFrame::OnMacroToggle(wxCommandEvent& evt) {
    if (m_macroRunning) {
        StopMacroThread();
    } else if (!m_vcEnabled) {
        Log("Virtual Controller Disabled, cant start macro");
    } else {
        StartMacroThread();
    }
}