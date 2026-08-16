#pragma once
#include "MacroAction.h"

class VirtualController {
public:
    virtual ~VirtualController() = default;
    void SendAction(const MacroAction& action);
    virtual void Reset() = 0;
    
protected:
    virtual void PressButton(ControllerButton button) = 0;
    virtual void ReleaseButton(ControllerButton button) = 0;
    virtual void MoveLeftJoystick(int x, int y) = 0;
    virtual void MoveRightJoystick(int x, int y) = 0;
};