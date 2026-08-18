#include "VirtualRosalina.h"
#include "Logging.h"
#include <wx/string.h>

VirtualRosalina::VirtualRosalina(): VirtualController() {
    this->Reset();
}

VirtualRosalina::~VirtualRosalina() {
    this->Reset();
    if (socket) {
        socket->Destroy();
        socket = nullptr;
    }
}

N3DSButton VirtualRosalina::ControllerButtonToN3DSButton (ControllerButton button) {
    switch (button) {
        case ControllerButton::A:
            return N3DSButton::A;

        case ControllerButton::B:
            return N3DSButton::B;

        case ControllerButton::X:
            return N3DSButton::X;

        case ControllerButton::Y:
            return N3DSButton::Y;

        case ControllerButton::L:
            return N3DSButton::L;

        case ControllerButton::R:
            return N3DSButton::R;

        case ControllerButton::Plus:
            return N3DSButton::START;

        case ControllerButton::Minus:
            return N3DSButton::SELECT;

        case ControllerButton::DpadUp:
            return N3DSButton::UP;

        case ControllerButton::DpadDown:
            return N3DSButton::DOWN;

        case ControllerButton::DpadLeft:
            return N3DSButton::LEFT;

        case ControllerButton::DpadRight:
            return N3DSButton::RIGHT;

        default:
            return N3DSButton::Unsuported;
    }
}

void VirtualRosalina::PressButton(ControllerButton button) {
    int bit = ControllerButtonToN3DSButton(button);
    if (bit >= 0)
        buttons &= ~(1u << bit); //clear bit = pressed
    SendInputPacket();
}

void VirtualRosalina::ReleaseButton(ControllerButton button) {
    int bit = ControllerButtonToN3DSButton(button);
    if (bit >= 0)
        buttons |= (1u << bit); //set bit = released
    SendInputPacket();
}

void VirtualRosalina::MoveLeftJoystick(int x, int y) {
    x = std::clamp(x, -100, 100);
    y = std::clamp(y, -100, 100);

    double lx = x / 100.0;
    double ly = y / 100.0;

    uint32_t xRaw = static_cast<uint32_t>(lx * CPAD_BOUND + 0x800);
    uint32_t yRaw = static_cast<uint32_t>(ly * CPAD_BOUND + 0x800);

    xRaw = xRaw >= 0xfff ? (lx < 0.0 ? 0x000 : 0xfff) : xRaw;
    yRaw = yRaw >= 0xfff ? (ly < 0.0 ? 0x000 : 0xfff) : yRaw;

    circlePad = (yRaw << 12) | xRaw;

    SendInputPacket();
}
void VirtualRosalina::MoveRightJoystick(int x, int y) {
    //no right stick, does nothing
    return;
}

void VirtualRosalina::Reset() {
    buttons = 0x00000FFF;
    touch = 0x02000000;   
    circlePad = 0x007FF7FF;
    SendInputPacket();
}

bool VirtualRosalina::Connect(wxString deviceIP) {
    //Resolve target address
    if (!peerAddr.Hostname(deviceIP)) {
        Log(wxString::Format("Invalid IP address: %s", deviceIP));
        return false;
    }
    peerAddr.Service(ROSALINA_INPUT_PORT);

    //Clean up any previous socket
    if (socket) {
        socket->Destroy();
        socket = nullptr;
    }

    //Create local UDP socket
    wxIPV4address localAddr;
    localAddr.AnyAddress();

    socket = new wxDatagramSocket(localAddr, wxSOCKET_NONE);

    if (!socket->IsOk()) {
        Log(wxString::Format("Failed to open UDP socket for %s:%d", deviceIP, ROSALINA_INPUT_PORT));
        socket->Destroy();
        socket = nullptr;
        return false;
    }

    Log(wxString::Format("Ready to send input to %s:%d", deviceIP, ROSALINA_INPUT_PORT));
    return true;
}

bool VirtualRosalina::SendInputPacket() {
    if (!socket || !socket->IsOk())
        return false;

    uint32_t packet[3] = { buttons, touch, circlePad };
    socket->SendTo(peerAddr, packet, sizeof(packet));

    if (socket->Error()) {
        Log(wxString::Format("Send failed, wxSocket error code: %d", (int)socket->LastError()));
        return false;
    }
    return true;
}