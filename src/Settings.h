#pragma once
#include <wx/string.h>
#include <map>

struct MacroSettings {
    int encounterValue = 0;
    int detectionX = 78;
    int detectionY = 365;
    int detectionW = 293;
    int detectionH = 51;
};

struct Settings {
    wxString webhookUrl;
    wxString userID;
    int selectedMacroIndex = -1;
    int selectedWebcamIndex = 0;

    // Per-macro encounter/detection overrides, keyed by macro library index.
    std::map<int, MacroSettings> macroSettings;

    bool LoadFromFile(const wxString& path);
    bool SaveToFile(const wxString& path) const;
};