#pragma once
#include <vector>
#include <wx/string.h>
#include "MacroAction.h"
#include "VirtualController.h"

class Macro {
public:
    explicit Macro(const wxString& name = wxEmptyString) : m_name(name) {}

    const wxString& GetName() const { return m_name; }
    void SetName(const wxString& name) { m_name = name; }

    const std::vector<MacroAction>& GetActions() const { return m_actions; }

    //Inserts before index. index >= GetActions().size() appends to the end.
    //Returns the actual index the action was inserted at.
    size_t InsertAction(size_t index, const MacroAction& action);

    void UpdateAction(size_t index, const MacroAction& action);
    void RemoveAction(size_t index);

    void Play();

    bool SaveToFile(const wxString& path) const;
    bool LoadFromFile(const wxString& path);

private:
    wxString m_name;
    std::vector<MacroAction> m_actions;
    VirtualController m_controller;
};