#pragma once
#include "MacroAction.h"

class VirtualController {
public:
    void SendAction(const MacroAction& action);

private:
    void PressButton(ControllerButton button);
    void ReleaseButton(ControllerButton button);
    void MoveJoystick(int x, int y);
};