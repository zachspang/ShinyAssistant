#include "VirtualXInput.h"
#include "Logging.h"
#include <wx/string.h>

VirtualXInput::VirtualXInput(): VirtualController() {
    client = vigem_alloc();

    if (client == nullptr) {
        Log("VirtualXInput: Not enough memory to allocate virtual controller");
        return;
    }
    
    const auto retval = vigem_connect(client);

    if (!VIGEM_SUCCESS(retval)) { 
        Log(wxString::Format("ViGEm Bus connection failed with error code: 0x%X", retval));
        return;
    }

    // Allocate handle to identify new pad
    pad = vigem_target_x360_alloc();

    // Add client to the bus, this equals a plug-in event
    const auto pir = vigem_target_add(client, pad);

    if (!VIGEM_SUCCESS(pir))
    {
        Log(wxString::Format("Target plugin failed with error code: 0x%X", pir));
        return;
    }

    this->Reset();
}

VirtualXInput::~VirtualXInput() {
    vigem_disconnect(client);
    vigem_free(client);
}

void VirtualXInput::PressButton(ControllerButton button) {
    switch (button) {
        case ControllerButton::A:
            report.wButtons |= XUSB_GAMEPAD_B;
            break;
        case ControllerButton::B:
            report.wButtons |= XUSB_GAMEPAD_X;
            break;
        case ControllerButton::X:
            report.wButtons |= XUSB_GAMEPAD_Y;
            break;
        case ControllerButton::Y:
            report.wButtons |= XUSB_GAMEPAD_X;
            break;
        case ControllerButton::L:
            report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
            break;
        case ControllerButton::R:
            report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;
            break;
        case ControllerButton::ZL:
            report.bLeftTrigger = 255;
            break;
        case ControllerButton::ZR:
            report.bRightTrigger = 255;
            break;
        case ControllerButton::L3:
            report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;
            break;
        case ControllerButton::R3:
            report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
            break;
        case ControllerButton::Plus:
            report.wButtons |= XUSB_GAMEPAD_START;
            break;
        case ControllerButton::Minus:
            report.wButtons |= XUSB_GAMEPAD_BACK;
            break;
        case ControllerButton::Home:
            //Do nothing
            break;
        case ControllerButton::DpadUp:
            report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
            break;
        case ControllerButton::DpadDown:
            report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
            break;
        case ControllerButton::DpadLeft:
            report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
            break;
        case ControllerButton::DpadRight:
            report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;
            break;
        default:
            break;
    }
    
    vigem_target_x360_update(client, pad, report);
}

void VirtualXInput::ReleaseButton(ControllerButton button) {
    switch (button) {
        case ControllerButton::A:
            report.wButtons &= ~XUSB_GAMEPAD_B;
            break;
        case ControllerButton::B:
            report.wButtons &= ~XUSB_GAMEPAD_X;
            break;
        case ControllerButton::X:
            report.wButtons &= ~XUSB_GAMEPAD_Y;
            break;
        case ControllerButton::Y:
            report.wButtons &= ~XUSB_GAMEPAD_X;
            break;
        case ControllerButton::L:
            report.wButtons &= ~XUSB_GAMEPAD_LEFT_SHOULDER;
            break;
        case ControllerButton::R:
            report.wButtons &= ~XUSB_GAMEPAD_RIGHT_SHOULDER;
            break;
        case ControllerButton::ZL:
            report.bLeftTrigger = 0;
            break;
        case ControllerButton::ZR:
            report.bRightTrigger = 0;
            break;
        case ControllerButton::L3:
            report.wButtons &= ~XUSB_GAMEPAD_LEFT_THUMB;
            break;
        case ControllerButton::R3:
            report.wButtons &= ~XUSB_GAMEPAD_RIGHT_THUMB;
            break;
        case ControllerButton::Plus:
            report.wButtons &= ~XUSB_GAMEPAD_START;
            break;
        case ControllerButton::Minus:
            report.wButtons &= ~XUSB_GAMEPAD_BACK;
            break;
        case ControllerButton::Home:
            //Do nothing
            break;
        case ControllerButton::DpadUp:
            report.wButtons &= ~XUSB_GAMEPAD_DPAD_UP;
            break;
        case ControllerButton::DpadDown:
            report.wButtons &= ~XUSB_GAMEPAD_DPAD_DOWN;
            break;
        case ControllerButton::DpadLeft:
            report.wButtons &= ~XUSB_GAMEPAD_DPAD_LEFT;
            break;
        case ControllerButton::DpadRight:
            report.wButtons &= ~XUSB_GAMEPAD_DPAD_RIGHT;
            break;
        default:
            break;
    }
    
    vigem_target_x360_update(client, pad, report);
}

void VirtualXInput::MoveLeftJoystick(int x, int y) {
    int convertedX = JOYSTICK_MAX * (x / 100.0);
    int convertedY = JOYSTICK_MAX * (y / 100.0);

    report.sThumbLX = convertedX;
    report.sThumbLY = convertedY;
    vigem_target_x360_update(client, pad, report);
}

void VirtualXInput::MoveRightJoystick(int x, int y) {
    int convertedX = JOYSTICK_MAX * (x / 100.0);
    int convertedY = JOYSTICK_MAX * (y / 100.0);

    report.sThumbRX = convertedX;
    report.sThumbRY = convertedY;
    vigem_target_x360_update(client, pad, report);
}

void VirtualXInput::Reset() {
    report.wButtons = 0;
    report.sThumbLX = 0;
    report.sThumbLY = 0;
    report.sThumbRX = 0;
    report.sThumbRY = 0;
    report.bLeftTrigger = 0;
    report.bRightTrigger = 0;

    vigem_target_x360_update(client, pad, report);
}