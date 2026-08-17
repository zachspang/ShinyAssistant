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

    encounterValue = j.value("encounterValue", 0);
    detectionX = j.value("detectionX", 78);
    detectionY = j.value("detectionY", 365);
    detectionW = j.value("detectionW", 293);
    detectionH = j.value("detectionH", 51);
    webhookUrl = wxString::FromUTF8(j.value("webhookUrl", ""));
    pingName = wxString::FromUTF8(j.value("pingName", ""));
    selectedMacroIndex = j.value("selectedMacroIndex", -1);
    selectedWebcamIndex = j.value("selectedWebcamIndex", 0);
    return true;
}

bool Settings::SaveToFile(const wxString& path) const {
    nlohmann::json j;
    j["encounterValue"] = encounterValue;
    j["detectionX"] = detectionX;
    j["detectionY"] = detectionY;
    j["detectionW"] = detectionW;
    j["detectionH"] = detectionH;
    j["webhookUrl"] = webhookUrl.ToStdString();
    j["pingName"] = pingName.ToStdString();
    j["selectedMacroIndex"] = selectedMacroIndex;
    j["selectedWebcamIndex"] = selectedWebcamIndex;

    std::ofstream file(path.ToStdString());
    if (!file.is_open()) return false;
    file << j.dump(2);
    return true;
}