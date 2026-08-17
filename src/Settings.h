#pragma once
#include <wx/string.h>

//Holds user configurable settings persisted to settings.json, used as defaults on startup
struct Settings {
    int encounterValue = 0;
    int detectionX = 78;
    int detectionY = 365;
    int detectionW = 293;
    int detectionH = 51;
    wxString webhookUrl;
    wxString pingName;
    int selectedMacroIndex = -1;
    int selectedWebcamIndex = 0;

    bool LoadFromFile(const wxString& path);
    bool SaveToFile(const wxString& path) const;
};