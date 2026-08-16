#pragma once
#include <wx/wx.h>

//Thread safe logging to wxLogTextCtrl
inline void Log(const wxString& message) {
    wxTheApp->CallAfter([message]() {
        wxLogMessage("%s", message);
    });
}