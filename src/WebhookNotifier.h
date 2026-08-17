#pragma once
#include <wx/image.h>
#include <wx/string.h>

class WebhookNotifier {
public:
    // Blocking network call never call from the UI thread.
    static bool SendShinyAlert(const wxImage& frame, const wxString& webhookUrl, const wxString& pingName);
};