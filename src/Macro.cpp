#include "Macro.h"
#include "VirtualController.h"
#include "VirtualRosalina.h"
#include "Logging.h"
#include <thread>
#include <wx/textfile.h>
#include <wx/filefn.h>
#include <wx/utils.h>
#include <chrono>
#include <nlohmann/json.hpp>
#include <random>

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

    // True if during the delay packets need to be send out. Used to keep controler state up to date if a packet drops
    bool shouldSendDuringDelay = false;
    VirtualRosalina* rosalinaPtr = dynamic_cast<VirtualRosalina*>(controller);

    if (rosalinaPtr != nullptr) {
        shouldSendDuringDelay = true;
    }

    int jumpIndex = 0;
    bool breakRELoop = false;

    for (size_t i = 0; i < m_actions.size(); i++) {
        const auto& action = m_actions.at(i);

        Log(action.ToDisplayString());
        if (!keepRunning) {
            controller->Reset();
            return true;
        }

        if (videoStream && videoStream->IsCheckingChange()) {
            breakRELoop = videoStream->CheckChange();
        }

        //Send press part of click
        if (action.type == ActionType::ClickButton) {
            MacroAction tempAction = MacroAction();
            tempAction.type = ActionType::PressButton;
            tempAction.button = action.button;
            controller->SendAction(tempAction);
        }

        int64_t timeBetweenSends = 40;
        int64_t nextSendTime = timeBetweenSends;

        if (action.type == ActionType::Delay || action.type == ActionType::RandomDelay || action.type == ActionType::ClickButton) {
            int targetDelayMs = action.delayMs;
            if (action.type == ActionType::RandomDelay) {
                static thread_local std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<int> dist(0, std::max(0, action.delayMs));
                targetDelayMs = dist(rng);
                Log(wxString::Format("Rand delay: %d", targetDelayMs));
            }

            auto start = std::chrono::steady_clock::now();
            const int spinThresholdMs = 20; // last few ms done with busy loop for precision

            // Sleep in small chunks to stay responsive to keepRunning going false
            while (keepRunning) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();

                
                if (shouldSendDuringDelay && elapsed >= nextSendTime) {
                    rosalinaPtr->SendInputPacket();
                    nextSendTime += timeBetweenSends;
                }

                long remaining = targetDelayMs - elapsed;

                if (remaining <= spinThresholdMs) break;

                if (videoStream && videoStream->IsCheckingChange()) {
                    breakRELoop = videoStream->CheckChange();
                }
                if (breakRELoop) break;

                long sleepChunkMs = std::min<long>(remaining - spinThresholdMs, 5);
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepChunkMs));
            }

            // Send extra packet before precision loop
            if (shouldSendDuringDelay) {
                rosalinaPtr->SendInputPacket();
            }

            // Final precision spin for the last few ms
            while (keepRunning && !breakRELoop) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
                if (elapsed >= targetDelayMs) break;
            }
        } else if (action.type == ActionType::CheckForShiny) {
            if (videoStream && videoStream->CheckShiny(keepRunning)) {
                OnShinyDetected(videoStream->GetWxImageFromFrame());
                controller->Reset();
                return true;
            }
        } else if (action.type == ActionType::RELoop) {
            jumpIndex = i;
            breakRELoop = videoStream->CheckChange();
            if (!breakRELoop) {
                // Reset macro to start if RELoop should keep looping
                i = -1;
            }
        } else if (action.type == ActionType::AddToEncounterNumber) {
            OnEncounterIncrement(action.encounterIncrement);
        } else {
            controller->SendAction(action);
        }

        //Send release part of click
        if (action.type == ActionType::ClickButton) {
            MacroAction tempAction = MacroAction();
            tempAction.type = ActionType::ReleaseButton;
            tempAction.button = action.button;
            controller->SendAction(tempAction);
        }

        // Jump to action after RELoop
        if (breakRELoop) {
            controller->Reset();
            i = jumpIndex;
            breakRELoop = false;
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