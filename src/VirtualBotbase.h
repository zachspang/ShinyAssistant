#include "VirtualController.h"
#include <wx/string.h>
#include <wx/socket.h>
#include <cstdint>

enum {
    ID_SOCKET_EVENT = wxID_HIGHEST + 1
};

class VirtualBotbase : public VirtualController {
public:
    VirtualBotbase();
    ~VirtualBotbase();
    void Reset() override;
    bool Connect(wxString deviceIP);
    bool SendInputPacket(wxString command);
    bool m_isConnected = false;
private:
    const int32_t JOYSTICK_MAX= 0x7FFF;
    const uint16_t BOTBASE_INPUT_PORT = 6000;

    wxSocketClient* socket = nullptr;
    wxIPV4address peerAddr;
    wxEvtHandler socketEvtHandler;
    void OnSocketEvent(wxSocketEvent& event);

    wxString ButtonToString(ControllerButton button);
    wxString FormatStickValue(int16_t value);
    
    void PressButton(ControllerButton button) override;
    void ReleaseButton(ControllerButton button) override;
    void MoveLeftJoystick(int x, int y) override;
    void MoveRightJoystick(int x, int y) override;
};