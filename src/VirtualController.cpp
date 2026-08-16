#include "VirtualController.h"

void VirtualController::SendAction(const MacroAction& action) {
    switch (action.type) {
        case ActionType::PressButton:          PressButton(action.button); break;
        case ActionType::ReleaseButton:        ReleaseButton(action.button); break;
        case ActionType::MoveLeftJoystick:     MoveLeftJoystick(action.leftJoystickX, action.leftJoystickY); break;
        case ActionType::MoveRightJoystick:    MoveRightJoystick(action.rightJoystickX, action.rightJoystickY); break;
        default: break;
    }
}