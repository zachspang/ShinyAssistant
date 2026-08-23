#include "Settings.h"
#include "Logging.h"
#include <nlohmann/json.hpp>
#include <fstream>

bool Settings::LoadFromFile(const wxString& path) {
    std::ifstream file(path.ToStdString());
    if (!file.is_open()) return false; //empty file, this is fine defaults will be loaded

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        Log(wxString::Format("Failed to parse settings.json: %s", e.what()));
        return false;
    }

    webhookUrl = wxString::FromUTF8(j.value("webhookUrl", ""));
    userID = wxString::FromUTF8(j.value("userID", ""));
    selectedMacroIndex = j.value("selectedMacroIndex", -1);
    selectedWebcamIndex = j.value("selectedWebcamIndex", 0);

    macroSettings.clear();
    if (j.contains("macroSettings") && j["macroSettings"].is_array()) {
        for (const auto& entry : j["macroSettings"]) {
            int index = entry.value("index", -1);
            if (index < 0) continue;

            MacroSettings ms;
            ms.encounterValue = entry.value("encounterValue", 0);
            ms.detectionX = entry.value("detectionX", 78);
            ms.detectionY = entry.value("detectionY", 365);
            ms.detectionW = entry.value("detectionW", 293);
            ms.detectionH = entry.value("detectionH", 51);
            macroSettings[index] = ms;
        }
    }

    return true;
}

bool Settings::SaveToFile(const wxString& path) const {
    nlohmann::json j;
    j["webhookUrl"] = webhookUrl.ToStdString();
    j["userID"] = userID.ToStdString();
    j["selectedMacroIndex"] = selectedMacroIndex;
    j["selectedWebcamIndex"] = selectedWebcamIndex;

    nlohmann::json macroSettingsJson = nlohmann::json::array();
    for (const auto& [index, ms] : macroSettings) {
        nlohmann::json entry;
        entry["index"] = index;
        entry["encounterValue"] = ms.encounterValue;
        entry["detectionX"] = ms.detectionX;
        entry["detectionY"] = ms.detectionY;
        entry["detectionW"] = ms.detectionW;
        entry["detectionH"] = ms.detectionH;
        macroSettingsJson.push_back(entry);
    }
    j["macroSettings"] = macroSettingsJson;

    std::ofstream file(path.ToStdString());
    if (!file.is_open()) return false;
    file << j.dump(2);
    return true;
}