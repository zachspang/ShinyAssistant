#include "Macro.h"
#include <wx/textfile.h>
#include <wx/filefn.h>
#include <wx/utils.h>
#include <chrono>

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

void Macro::Play(const std::atomic<bool>& keepRunning) {
    for (const auto& action : m_actions) {
        if (!keepRunning) return; //check before every action

        if (action.type == ActionType::Delay) {
            //Busy loop for duration of delay or until keepRunning == false
            auto start = std::chrono::steady_clock::now();
            while (keepRunning) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
                if (elapsed >= action.delayMs) break;
            }
        } else if (action.type == ActionType::CheckForShiny) {
            //TODO: Call videoStream.checkShiny() and stop playing based on result
            //TODO: If shiny send have discord bot ping user with current frame to show shiny
        } else if (action.type == ActionType::AddToEncounterNumber) {
            //TODO: inc encouterCounter by action.encounterIncrement
        } else {
            m_controller->SendAction(action);
        }
    }
}

wxArrayString Macro::SerializeLines() const {
    wxArrayString lines;
    lines.Add(m_name);
    lines.Add(wxString::Format("%zu", m_actions.size()));
    for (const auto& action : m_actions) lines.Add(action.Serialize());
    return lines;
}

Macro Macro::DeserializeLines(const wxArrayString& lines) {
    Macro macro;
    size_t i = 0;
    if (i >= lines.GetCount()) return macro;
    macro.m_name = lines[i++];

    long actionCount = 0;
    if (i < lines.GetCount()) lines[i++].ToLong(&actionCount);

    for (long n = 0; n < actionCount && i < lines.GetCount(); ++n, ++i) {
        macro.m_actions.push_back(MacroAction::Deserialize(lines[i]));
    }
    return macro;
}