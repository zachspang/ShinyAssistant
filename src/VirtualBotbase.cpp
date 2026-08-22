#include "VirtualBotbase.h"
#include "Logging.h"
#include <wx/string.h>

VirtualBotbase::VirtualBotbase(): VirtualController() {
    //Does nothing right now
}

VirtualBotbase::~VirtualBotbase() {
    if(m_isConnected) {
        SendInputPacket("detachController");
        this->Reset();
    }
    
    if (socket) {
        socket->Destroy();
        socket = nullptr;
    }
}

wxString VirtualBotbase::ButtonToString(ControllerButton button) {
    switch (button) {
        case ControllerButton::A:
            return "A";
        case ControllerButton::B:
            return "B";
        case ControllerButton::X:
            return "X";
        case ControllerButton::Y:
            return "Y";
        case ControllerButton::L:
            return "L";
        case ControllerButton::R:
            return "R";
        case ControllerButton::ZL:
            return "ZL";
        case ControllerButton::ZR:
            return "ZR";
        case ControllerButton::L3:
            return "LSTICK";
        case ControllerButton::R3:
            return "RSTICK";
        case ControllerButton::Plus:
            return "PLUS";
        case ControllerButton::Minus:
            return "MINUS";
        case ControllerButton::Home:
            return "HOME";
        case ControllerButton::DpadUp:
            return "DUP";
        case ControllerButton::DpadDown:
            return "DDOWN";
        case ControllerButton::DpadLeft:
            return "DLEFT";
        case ControllerButton::DpadRight:
            return "DRIGHT";
        default:
            return "UNUSED";
    }
}

void VirtualBotbase::PressButton(ControllerButton button) {
    SendInputPacket(wxString::Format("press %s", ButtonToString(button)));
}

void VirtualBotbase::ReleaseButton(ControllerButton button) {
    SendInputPacket(wxString::Format("release %s", ButtonToString(button)));
}

wxString VirtualBotbase::FormatStickValue(int16_t value) {
    if (value < 0) return wxString::Format("-0x%X", -value);

    return wxString::Format("0x%X", value);
}

void VirtualBotbase::MoveLeftJoystick(int x, int y) {
    x = std::clamp(x, -100, 100);
    y = std::clamp(y, -100, 100);

    int16_t scaledX = static_cast<int16_t>((x / 100.0) * JOYSTICK_MAX);
    int16_t scaledY = static_cast<int16_t>((y / 100.0) * JOYSTICK_MAX);

    wxString hexX = FormatStickValue(scaledX);
    wxString hexY = FormatStickValue(scaledY);

    SendInputPacket(wxString::Format("setStick LEFT %s %s", hexX, hexY));
}

void VirtualBotbase::MoveRightJoystick(int x, int y) {
    x = std::clamp(x, -100, 100);
    y = std::clamp(y, -100, 100);

    int16_t scaledX = static_cast<int16_t>((x / 100.0) * JOYSTICK_MAX);
    int16_t scaledY = static_cast<int16_t>((y / 100.0) * JOYSTICK_MAX);

    wxString hexX = FormatStickValue(scaledX);
    wxString hexY = FormatStickValue(scaledY);

    SendInputPacket(wxString::Format("setStick RIGHT %s %s", hexX, hexY));
}

void VirtualBotbase::Reset() {
    //Click sequence to release all buttons
    SendInputPacket("clickSeq -A,-B,-X,-Y,-L,-R,-ZL,-ZR,-LSTICK,-RSTICK,-PLUS,-MINUS,-HOME,-DUP,-DDOWN,-DLEFT,-DRIGHT");

    SendInputPacket("setStick LEFT 0x0 0x0");
    SendInputPacket("setStick RIGHT 0x0 0x0");
}

void VirtualBotbase::OnSocketEvent(wxSocketEvent& event) {
    switch (event.GetSocketEvent()) {
        case wxSOCKET_CONNECTION:
            Log("Connected successfully.");
            m_isConnected = true;
            break;

        case wxSOCKET_LOST:
            Log("Connection lost or failed.");
            m_isConnected = false;
            break;

        default:
            break;
    }
}

bool VirtualBotbase::Connect(wxString deviceIP) {
    //Resolve target address
    if (!peerAddr.Hostname(deviceIP)) {
        Log(wxString::Format("Invalid IP address: %s", deviceIP));
        return false;
    }
    peerAddr.Service(BOTBASE_INPUT_PORT);

    //Clean up any previous socket
    if (socket) {
        socket->Destroy();
        socket = nullptr;
    }

    //Create TCP client socket
    socket = new wxSocketClient();

    socket->SetEventHandler(socketEvtHandler, ID_SOCKET_EVENT);
    socket->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
    socket->Notify(true);

    socketEvtHandler.Bind(wxEVT_SOCKET, &VirtualBotbase::OnSocketEvent, this, ID_SOCKET_EVENT);

    socket->Connect(peerAddr, false);

    return true;
}

bool VirtualBotbase::SendInputPacket(wxString command) {
    if (!m_isConnected || !socket || !socket->IsOk()) {
        Log("Cant send packet, not connected to a device");
        return false;
    }

    command += "\r\n";
    wxScopedCharBuffer buffer = command.utf8_str();

    socket->Write(buffer.data(), buffer.length());

    if (socket->Error()) {
        Log(wxString::Format("Send failed, wxSocket error code: %d", (int)socket->LastError()));
        return false;
    }
    return true;
}