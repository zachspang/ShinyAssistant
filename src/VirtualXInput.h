#include "VirtualController.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#include <ViGEm/Client.h>

class VirtualXInput : public VirtualController {
public:
    VirtualXInput();
    ~VirtualXInput();
    void Reset() override;
private:
    const int JOYSTICK_MAX = 32767;
    PVIGEM_CLIENT client;
    PVIGEM_TARGET pad;
    XUSB_REPORT report;

    void PressButton(ControllerButton button) override;
    void ReleaseButton(ControllerButton button) override;
    void MoveLeftJoystick(int x, int y) override;
    void MoveRightJoystick(int x, int y) override;
};