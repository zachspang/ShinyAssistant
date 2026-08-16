#pragma once
#include <wx/wx.h>
#include "ScaledBitmap.h"
#include "VideoStream.h"
#include "MacroEditorFrame.h"
#include "MacroLibrary.h"
#include <thread>
#include <atomic>

class MainFrame : public wxFrame
{
public: 
    MainFrame(const wxString& title);

    enum ControllerType {
        xinput = 0,
        sysbotbase = 1,
        rosalinaIR = 2
    };
private:
    //Reference to a VideoStream object handling the webcam
    VideoStream* m_videoStream;

    //Bitmap image representing 1 frame from m_videoStream
    ScaledBitmap* m_videoBitmap;

    //Timer that ticks to update m_videoBitmap
    wxTimer* m_videoTimer;
    
    //Static text that displays encounter number under video
    wxStaticText* m_encounterCounter;

    //True if virtual controller checkbox is enabled
    bool m_vcEnabled = false;

    //Radiobox to select ControllerType
    wxRadioBox* m_controllerTypeRadioBox;

    //Selection from controllerTypeRadioBox
    ControllerType m_controllerType = xinput;

    //Local device ip from deviceIPCtrl
    wxString m_deviceIP;

    //Text box to enter device IP
    wxTextCtrl* m_deviceIPCtrl;

    //Confirm button to submit IP 
    wxButton* m_deviceIPConfirm;

    //Second window that pops up to edit macros
    MacroEditorFrame* m_macroEditorFrame = nullptr;

    //Choice widget used to select macro
    wxChoice* m_macroChoice;

    //Owns every saved macro, loaded from / saved to m_macroFilePath
    MacroLibrary m_macroLibrary;

    //Path to the single file all macros are stored in
    wxString m_macroFilePath;

    //Pointer to the currently selected macro in m_macroLibrary, or nullptr if none selected.
    //Recomputed via UpdateSelectedMacroPointer() whenever the selection or library contents change,
    //since m_macroLibrary's underlying vector can reallocate on add/remove.
    Macro* m_selectedMacro = nullptr;

    void UpdateSelectedMacroPointer();
    std::thread m_macroThread;
    std::atomic<bool> m_macroRunning{false};

    //True while MacroEditorFrame is open, used to block Start while editing
    bool m_macroEditorOpen = false;

    //Start/Stop button, kept as a member so it can be enabled/disabled from other handlers
    wxButton* m_macroToggleButton = nullptr;

    //Rebuilds m_macroChoice's items from m_macroLibrary, preserving selection where possible
    void RefreshMacroChoice();

    //Used for showing logs
    wxTextCtrl* m_logCtrl = nullptr;

    void OnEncounterUpdate(wxSpinEvent& evt);
    void OnVCToggle(wxCommandEvent& evt);
    void OnVCTypeChange(wxCommandEvent& evt);
    void OnIPConfirm(wxCommandEvent& evt);
    void UpdateVideo(wxTimerEvent& evt);
    void OnTestDetect(wxCommandEvent& evt);
    void OnDetectionXUpdate(wxCommandEvent& evt);
    void OnDetectionYUpdate(wxCommandEvent& evt);
    void OnDetectionWUpdate(wxCommandEvent& evt);
    void OnDetectionHUpdate(wxCommandEvent& evt);
    void OnMacroChange(wxCommandEvent& evt);
    void OnDeleteMacro(wxCommandEvent& evt);
    void OnEditMacro(wxCommandEvent& evt);
    void OnCreateMacro(wxCommandEvent& evt);
    void OnMacroEditorClosed(wxShowEvent& evt);
    void OnMacroToggle(wxCommandEvent& evt);
    void StartMacroThread();
    void StopMacroThread();
};