#include "MacroEditorWindow.h"

MacroEditorFrame::MacroEditorFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, "Macro Editor") {
    m_panel = new wxPanel(this);
    new wxStaticText(m_panel, wxID_ANY, "Popup window test");

    Bind(wxEVT_CLOSE_WINDOW, &MacroEditorFrame::OnClose, this);
}

void MacroEditorFrame::OnClose(wxCloseEvent& evt) {
    evt.Skip(); 
}