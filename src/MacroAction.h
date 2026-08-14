#pragma once
#include <wx/string.h>
#include <wx/arrstr.h>
enum class ActionType {
    PressButton,
    ReleaseButton,
    MoveJoystick,
    Delay,
    CheckForShiny,
    AddToEncounterNumber,
    Invalid
};

enum class ControllerButton {
    A, B, X, Y, L, R, ZL, ZR, L3, R3,
    Plus, Minus, Home,
    DpadUp, DpadDown, DpadLeft, DpadRight,
    COUNT //keep last, used to bound the wxChoice list
};

wxString ButtonToString(ControllerButton button);
wxString ActionTypeToChoiceLabel(ActionType type);
ActionType ChoiceLabelToActionType(const wxString& label);

struct MacroAction {
    ActionType type = ActionType::Invalid;

    //Press release button
    ControllerButton button = ControllerButton::A;

    //Move joystick to these coords
    int joystickX = 0;   // -100..100
    int joystickY = 0;   // -100..100

    //Delay
    int delayMs = 0;     // 0..100000

    //Number to increment encounter counter by
    int encounterIncrement = 0; // 0..1000

    wxString ToDisplayString() const;
    wxString Serialize() const;
    static MacroAction Deserialize(const wxString& line);
};