#include "MacroAction.h"
#include <wx/tokenzr.h>
#include <wx/utils.h>

wxString ButtonToString(ControllerButton button) {
    switch (button) {
        case ControllerButton::A: return "A";
        case ControllerButton::B: return "B";
        case ControllerButton::X: return "X";
        case ControllerButton::Y: return "Y";
        case ControllerButton::L: return "L";
        case ControllerButton::R: return "R";
        case ControllerButton::ZL: return "ZL";
        case ControllerButton::ZR: return "ZR";
        case ControllerButton::L3: return "L3";
        case ControllerButton::R3: return "R3";
        case ControllerButton::Plus: return "Plus";
        case ControllerButton::Minus: return "Minus";
        case ControllerButton::Home: return "Home";
        case ControllerButton::DpadUp: return "DpadUp";
        case ControllerButton::DpadDown: return "DpadDown";
        case ControllerButton::DpadLeft: return "DpadLeft";
        case ControllerButton::DpadRight: return "DpadRight";
        default: return wxEmptyString;
    }
}

//Helper for Serialize()
wxString ActionTypeToString(ActionType t) {
    switch (t) {
        case ActionType::ClickButton: return "ClickButton";
        case ActionType::PressButton: return "PressButton";
        case ActionType::ReleaseButton: return "ReleaseButton";
        case ActionType::MoveLeftJoystick: return "MoveLeftJoystick";
        case ActionType::MoveRightJoystick: return "MoveRightJoystick";
        case ActionType::Delay: return "Delay";
        case ActionType::RandomDelay: return "RandomDelay";
        case ActionType::CheckForShiny: return "CheckForShiny";
        case ActionType::AddToEncounterNumber: return "AddToEncounterNumber";
        default: return "Invalid";
    }
}

//Helper for Deserialize()
ActionType StringToActionType(const wxString& s) {
    if (s == "ClickButton") return ActionType::ClickButton;
    if (s == "PressButton") return ActionType::PressButton;
    if (s == "ReleaseButton") return ActionType::ReleaseButton;
    if (s == "MoveLeftJoystick") return ActionType::MoveLeftJoystick;
    if (s == "MoveRightJoystick") return ActionType::MoveRightJoystick;
    if (s == "Delay") return ActionType::Delay;
    if (s == "RandomDelay") return ActionType::RandomDelay;
    if (s == "CheckForShiny") return ActionType::CheckForShiny;
    if (s == "AddToEncounterNumber") return ActionType::AddToEncounterNumber;
    return ActionType::Invalid;
}

wxString ActionTypeToChoiceLabel(ActionType type) {
    switch (type) {
        case ActionType::ClickButton: return "Click Button";
        case ActionType::PressButton: return "Press Button";
        case ActionType::ReleaseButton: return "Release Button";
        case ActionType::MoveLeftJoystick: return "Move Left Joystick";
        case ActionType::MoveRightJoystick: return "Move Right Joystick";
        case ActionType::Delay: return "Delay";
        case ActionType::RandomDelay: return "Random Delay";
        case ActionType::CheckForShiny: return "Check For Shiny";
        case ActionType::AddToEncounterNumber: return "Add to encounter number";
        default: return wxEmptyString;
    }
}

ActionType ChoiceLabelToActionType(const wxString& label) {
    if (label == "Click Button") return ActionType::ClickButton;
    if (label == "Press Button") return ActionType::PressButton;
    if (label == "Release Button") return ActionType::ReleaseButton;
    if (label == "Move Left Joystick") return ActionType::MoveLeftJoystick;
    if (label == "Move Right Joystick") return ActionType::MoveRightJoystick;
    if (label == "Delay") return ActionType::Delay;
    if (label == "Random Delay") return ActionType::RandomDelay;
    if (label == "Check For Shiny") return ActionType::CheckForShiny;
    if (label == "Add to encounter number") return ActionType::AddToEncounterNumber;
    return ActionType::Invalid;
}

wxString MacroAction::ToDisplayString() const {
    switch (type) {
        case ActionType::ClickButton:
            return "Click " + ButtonToString(button);
        case ActionType::PressButton:
            return "Press " + ButtonToString(button);
        case ActionType::ReleaseButton:
            return "Release " + ButtonToString(button);
        case ActionType::MoveLeftJoystick:
            return wxString::Format("Move Left Joystick (%d, %d)", leftJoystickX, leftJoystickY);
        case ActionType::MoveRightJoystick:
            return wxString::Format("Move Right Joystick (%d, %d)", rightJoystickX, rightJoystickY);
        case ActionType::Delay:
            return wxString::Format("Delay %dms", delayMs);
        case ActionType::RandomDelay:
            return wxString::Format("Random Delay (0-%dms)", delayMs);
        case ActionType::CheckForShiny:
            return "Check For Shiny";
        case ActionType::AddToEncounterNumber:
            return wxString::Format("Add %d to encounter number", encounterIncrement);
        default:
            return "Invalid Action";
    }
}

nlohmann::json MacroAction::ToJson() const {
    nlohmann::json j;
    j["type"] = ActionTypeToString(type).ToStdString();

    switch (type) {
        case ActionType::ClickButton:
        case ActionType::PressButton:
        case ActionType::ReleaseButton:
            j["button"] = (int)button;
            break;
        case ActionType::MoveLeftJoystick:
            j["leftJoystickX"] = leftJoystickX;
            j["leftJoystickY"] = leftJoystickY;
            break;
        case ActionType::MoveRightJoystick:
            j["rightJoystickX"] = rightJoystickX;
            j["rightJoystickY"] = rightJoystickY;
            break;
        case ActionType::Delay:
        case ActionType::RandomDelay:
            j["delayMs"] = delayMs;
            break;
        case ActionType::AddToEncounterNumber:
            j["encounterIncrement"] = encounterIncrement;
            break;
        case ActionType::CheckForShiny:
        default:
            break; //no extra fields needed
    }

    return j;
}

MacroAction MacroAction::FromJson(const nlohmann::json& j) {
    MacroAction a;
    a.type = StringToActionType(wxString::FromUTF8(j.value("type", "Invalid")));
    a.button = (ControllerButton)j.value("button", 0);
    a.leftJoystickX = j.value("leftJoystickX", 0);
    a.leftJoystickY = j.value("leftJoystickY", 0);
    a.rightJoystickX = j.value("rightJoystickX", 0);
    a.rightJoystickY = j.value("rightJoystickY", 0);
    a.delayMs = j.value("delayMs", 100);
    a.encounterIncrement = j.value("encounterIncrement", 1);
    return a;
}