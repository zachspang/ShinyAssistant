#include "VirtualController.h"

void VirtualController::SendAction(const MacroAction& action) {
    switch (action.type) {
        case ActionType::PressButton:          PressButton(action.button); break;
        case ActionType::ReleaseButton:        ReleaseButton(action.button); break;
        case ActionType::MoveJoystick:         MoveJoystick(action.joystickX, action.joystickY); break;
        default: break;
    }
}

void VirtualController::PressButton(ControllerButton button)   { /* TODO */ }
void VirtualController::ReleaseButton(ControllerButton button) { /* TODO */ }
void VirtualController::MoveJoystick(int x, int y) { /* TODO */ }