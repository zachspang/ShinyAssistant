#pragma once
#include <wx/wx.h>
#include "ScaledBitmap.h"
#include "VideoStream.h"
#include "MacroEditorFrame.h"

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

    //List of available macros
    wxArrayString m_macroList;

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
    void OnEditMacro(wxCommandEvent& evt);
    void OnCreateMacro(wxCommandEvent& evt);
    void OnMacroEditorClosed(wxCloseEvent& evt);
};