#pragma once
#include <wx/string.h>
#include <wx/arrstr.h>
#include <nlohmann/json.hpp>

enum class ActionType {
    ClickButton,
    PressButton,
    ReleaseButton,
    MoveLeftJoystick,
    MoveRightJoystick,
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
wxString ActionTypeToString(ActionType t);       // already exists in .cpp, just needs declaring here
ActionType StringToActionType(const wxString& s); // already exists in .cpp, just needs declaring here

//A single macro action (ex. Pressing A, moving joystick or a delay)
struct MacroAction {
    ActionType type = ActionType::Invalid;

    //Press release button
    ControllerButton button = ControllerButton::A;

    //Move left joystick to these coords
    int leftJoystickX = 0;   // -100..100
    int leftJoystickY = 0;   // -100..100

    //Move right joystick to these coords
    int rightJoystickX = 0;   // -100..100
    int rightJoystickY = 0;   // -100..100

    //Delay
    int delayMs = 50;     // 0..100000

    //Number to increment encounter counter by
    int encounterIncrement = 1; // 0..1000


    wxString ToDisplayString() const;
    nlohmann::json ToJson() const;
    static MacroAction FromJson(const nlohmann::json& j);
};