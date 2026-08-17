#pragma once
#include "VideoStream.h"
#include <vector>
#include <wx/string.h>
#include <wx/arrstr.h>
#include "MacroAction.h"
#include "VirtualController.h"
#include <atomic>

//Owns a vector of actions and a pointer to a VirtualController to playback the actions
class Macro {
public:
    Macro(const wxString& name = wxEmptyString);

    const wxString& GetName() const { return m_name; }
    void SetName(const wxString& name) { m_name = name; }

    const std::vector<MacroAction>& GetActions() const { return m_actions; }

    //Inserts before index. index >= GetActions().size() appends to the end.
    //Returns the actual index the action was inserted at.
    size_t InsertAction(size_t index, const MacroAction& action);
    void UpdateAction(size_t index, const MacroAction& action);
    void RemoveAction(size_t index);

    //Returns true of macro should stop looping
    bool Play(const std::atomic<bool>& keepRunning, VirtualController* &controller, 
        VideoStream* videoStream, const std::function<void(int)>& OnEncounterIncrement,
        const std::function<void(const wxImage&)>& OnShinyDetected);

    //Serialize a block of lines representing just this macro: name, action count, then one line per action.
    wxArrayString SerializeLines() const;
    //Deserialize a block of lines representing just this macro: name, action count, then one line per action.
    static Macro DeserializeLines(const wxArrayString& lines);

private:
    wxString m_name;
    std::vector<MacroAction> m_actions;
};