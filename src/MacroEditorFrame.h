#pragma once
#include <wx/wx.h>

class MacroEditorFrame : public wxFrame {
public:
    MacroEditorFrame(wxWindow* parent);

private:
    void OnClose(wxCloseEvent& evt);

    wxPanel* m_panel;
};