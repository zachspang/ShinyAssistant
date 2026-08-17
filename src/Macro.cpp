#include "Macro.h"
#include "VirtualXInput.h"
#include "Logging.h"
#include <thread>
#include <wx/textfile.h>
#include <wx/filefn.h>
#include <wx/utils.h>
#include <chrono>
#include <nlohmann/json.hpp>

Macro::Macro(const wxString& name) {
    m_name = name;
}

size_t Macro::InsertAction(size_t index, const MacroAction& action) {
    if (index > m_actions.size()) index = m_actions.size();
    m_actions.insert(m_actions.begin() + index, action);
    return index;
}

void Macro::UpdateAction(size_t index, const MacroAction& action) {
    if (index < m_actions.size()) m_actions[index] = action;
}

void Macro::RemoveAction(size_t index) {
    if (index < m_actions.size()) m_actions.erase(m_actions.begin() + index);
}

bool Macro::Play(const std::atomic<bool>& keepRunning, VirtualController* &controller,  
    VideoStream* videoStream, const std::function<void(int)>& OnEncounterIncrement,
    const std::function<void(const wxImage&)>& OnShinyDetected) {
    for (const auto& action : m_actions) {
        if (!keepRunning) {
            controller->Reset();
            return true;
        }

        if (action.type == ActionType::Delay) {
            auto start = std::chrono::steady_clock::now();
            const int spinThresholdMs = 20; // last few ms done with busy loop for precision

            // Sleep in small chunks to stay responsive to keepRunning going false
            while (keepRunning) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
                long remaining = action.delayMs - elapsed;

                if (remaining <= spinThresholdMs) break;

                long sleepChunkMs = std::min<long>(remaining - spinThresholdMs, 5);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepChunkMs));
            }

            // Final precision spin for the last few ms
            while (keepRunning) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
                if (elapsed >= action.delayMs) break;
            }
        } else if (action.type == ActionType::CheckForShiny) {
            if (videoStream && videoStream->checkShiny(keepRunning)) {
                OnShinyDetected(videoStream->GetWxImageFromFrame());
                controller->Reset();
                return true;
            }
        } else if (action.type == ActionType::AddToEncounterNumber) {
            OnEncounterIncrement(action.encounterIncrement);
        } else {
            controller->SendAction(action);
        }
    }

    return false;
}

nlohmann::json Macro::ToJson() const {
    nlohmann::json j;
    j["name"] = m_name.ToStdString();
    nlohmann::json actionsJson = nlohmann::json::array();
    for (const auto& action : m_actions) actionsJson.push_back(action.ToJson());
    j["actions"] = actionsJson;
    return j;
}

Macro Macro::FromJson(const nlohmann::json& j) {
    Macro macro;
    macro.m_name = wxString::FromUTF8(j.value("name", ""));
    if (j.contains("actions") && j["actions"].is_array()) {
        for (const auto& actionJson : j["actions"]) {
            macro.m_actions.push_back(MacroAction::FromJson(actionJson));
        }
    }
    return macro;
}