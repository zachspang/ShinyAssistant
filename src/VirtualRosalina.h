#include "VirtualController.h"
#include <wx/string.h>
#include <wx/socket.h>
#include <cstdint>

enum N3DSButton {
    Unsuported = -1,
    A = 0,
    B = 1,
    SELECT = 2,
    START = 3,
    RIGHT = 4,
    LEFT = 5,
    UP = 6,
    DOWN = 7,
    R = 8,
    L = 9,
    X = 10,
    Y = 11,
};

class VirtualRosalina : public VirtualController {
public:
    VirtualRosalina();
    ~VirtualRosalina();
    void Reset() override;
    bool Connect(wxString deviceIP);
    bool SendInputPacket();
private:
    //CPAD_BOUND and MoveLeftJoystick referenced from TuxSH's InputRedirectionClient-Qt
    const int32_t CPAD_BOUND = 0x5d0;
    const uint16_t ROSALINA_INPUT_PORT = 4950;

    wxDatagramSocket *socket = nullptr;
    wxIPV4address peerAddr;

    uint32_t buttons = 0x00000FFF;   //idle: all buttons unpressed (active-low)
    uint32_t touch = 0x02000000;     //idle: no touch
    uint32_t circlePad = 0x007FF7FF; //idle: centered

    N3DSButton ControllerButtonToN3DSButton (ControllerButton button);

    void PressButton(ControllerButton button) override;
    void ReleaseButton(ControllerButton button) override;
    void MoveLeftJoystick(int x, int y) override;
    void MoveRightJoystick(int x, int y) override;
};