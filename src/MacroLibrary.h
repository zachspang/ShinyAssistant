#pragma once
#include <vector>
#include <wx/string.h>
#include "Macro.h"

//Owns every Macro loaded from / saved to a single file on disk
class MacroLibrary {
public:
    bool LoadFromFile(const wxString& path);
    bool SaveToFile(const wxString& path) const;

    std::vector<Macro>& GetMacros() { return m_macros; }
    const std::vector<Macro>& GetMacros() const { return m_macros; }
    wxArrayString GetMacroNames() const;

    size_t AddMacro(const Macro& macro);
    void UpdateMacro(size_t index, const Macro& macro);
    void RemoveMacro(size_t index);

private:
    std::vector<Macro> m_macros;
};