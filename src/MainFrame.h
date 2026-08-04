#pragma once
#include <wx/wx.h>

class MainFrame : public wxFrame
{
public: 
    MainFrame(const wxString& title);

    enum ControllerType {
        xinput = 1,
        sysbotbase = 2,
        rosalinaIR = 3
    };
private:
    //Static text that displays encounter number under video
    wxStaticText* m_encounterCounter;
    //True if virtual controller checkbox is enabled
    bool m_vcEnabled = false;
    //Radiobox to select ControllerType
    wxRadioBox* m_controllerTypeRadioBox;
    //Selection from controllerTypeRadioBox
    ControllerType m_controllerType;
    //Local device ip from deviceIPCtrl
    wxString m_deviceIP;
    //Text box to enter device IP
    wxTextCtrl* m_deviceIPCtrl;
    //Confirm button to submit IP 
    wxButton* m_deviceIPConfirm;

    void OnEncounterUpdate(wxSpinEvent& evt);
    void OnVCToggle(wxCommandEvent& evt);
    void OnVCTypeChange(wxCommandEvent& evt);
    void OnIPConfirm(wxCommandEvent& evt);
};