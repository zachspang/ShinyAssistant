#include "Macro.h"
#include <wx/textfile.h>
#include <wx/filefn.h>
#include <wx/utils.h>

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

void Macro::Play() {
    for (const auto& action : m_actions) {
        if (action.type == ActionType::Delay) {
            //TODO: Change to more precise delay
            wxMilliSleep(action.delayMs);
        } else if (action.type == ActionType::CheckForShiny) {
            //TODO: Call videoStream.checkShiny() and stop playing based on result
        } else if (action.type == ActionType::AddToEncounterNumber) {
            //TODO: inc encouterCounter by action.encounterIncrement
        } else {
            m_controller.SendAction(action);
        }
    }
}

bool Macro::SaveToFile(const wxString& path) const {
    wxTextFile file;
    if (wxFileExists(path)) { if (!file.Open(path)) return false; file.Clear(); }
    else { file.Create(path); }

    file.AddLine(m_name);
    for (const auto& action : m_actions) file.AddLine(action.Serialize());

    bool ok = file.Write();
    file.Close();
    return ok;
}

bool Macro::LoadFromFile(const wxString& path) {
    wxTextFile file;
    if (!file.Open(path)) return false;
    if (file.GetLineCount() == 0) { file.Close(); return false; }

    m_actions.clear();
    m_name = file.GetFirstLine();
    for (wxString line = file.GetNextLine(); !file.Eof(); line = file.GetNextLine()) {
        if (!line.IsEmpty()) m_actions.push_back(MacroAction::Deserialize(line));
    }
    file.Close();
    return true;
}