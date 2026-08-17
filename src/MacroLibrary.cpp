#include "MacroLibrary.h"
#include "Logging.h"
#include <nlohmann/json.hpp>
#include <fstream>

bool MacroLibrary::LoadFromFile(const wxString& path) {
    m_macros.clear();

    std::ifstream file(path.ToStdString());
    if (!file.is_open()) return false; //empty file, valid just no macros created

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        Log(wxString::Format("Failed to parse macros.json: %s", e.what()));
        return false;
    }

    if (!j.is_array()) return false;
    for (const auto& macroJson : j) {
        m_macros.push_back(Macro::FromJson(macroJson));
    }

    return true;
}

bool MacroLibrary::SaveToFile(const wxString& path) const {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& macro : m_macros) j.push_back(macro.ToJson());

    std::ofstream file(path.ToStdString());
    if (!file.is_open()) return false;

    file << j.dump(2);
    return true;
}

wxArrayString MacroLibrary::GetMacroNames() const {
    wxArrayString names;
    for (const auto& macro : m_macros) names.Add(macro.GetName());
    return names;
}

size_t MacroLibrary::AddMacro(const Macro& macro) {
    m_macros.push_back(macro);
    return m_macros.size() - 1;
}

void MacroLibrary::UpdateMacro(size_t index, const Macro& macro) {
    if (index < m_macros.size()) m_macros[index] = macro;
}

void MacroLibrary::RemoveMacro(size_t index) {
    if (index < m_macros.size()) m_macros.erase(m_macros.begin() + index);
}