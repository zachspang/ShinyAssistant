#include "MacroLibrary.h"
#include <wx/textfile.h>
#include <wx/filefn.h>

bool MacroLibrary::LoadFromFile(const wxString& path) {
    m_macros.clear();
    if (!wxFileExists(path)) return false;

    wxTextFile file;
    if (!file.Open(path)) return false;

    wxArrayString allLines;
    for (size_t i = 0; i < file.GetLineCount(); ++i) allLines.Add(file.GetLine(i));
    file.Close();

    if (allLines.IsEmpty()) return true; //empty file, valid just no macros created

    size_t pos = 0;
    long macroCount = 0;
    allLines[pos++].ToLong(&macroCount);

    for (long m = 0; m < macroCount && pos < allLines.GetCount(); ++m) {
        wxArrayString macroLines;

        macroLines.Add(allLines[pos++]); //name
        long actionCount = 0;
        if (pos < allLines.GetCount()) {
            actionCount = 0;
            allLines[pos].ToLong(&actionCount);
            macroLines.Add(allLines[pos++]); //action count
        }
        for (long a = 0; a < actionCount && pos < allLines.GetCount(); ++a) {
            macroLines.Add(allLines[pos++]);
        }

        m_macros.push_back(Macro::DeserializeLines(macroLines));
    }

    return true;
}

bool MacroLibrary::SaveToFile(const wxString& path) const {
    wxTextFile file;
    if (wxFileExists(path)) {
        if (!file.Open(path)) return false;
        file.Clear();
    } else {
        if (!file.Create(path)) return false;
    }

    file.AddLine(wxString::Format("%zu", m_macros.size()));
    for (const auto& macro : m_macros) {
        for (const auto& line : macro.SerializeLines()) {
            file.AddLine(line);
        }
    }

    bool ok = file.Write();
    file.Close();
    return ok;
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