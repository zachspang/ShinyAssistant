#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#include <wx/wx.h>
#include <ViGEm/Client.h>

//  // vigemclient test
// PVIGEM_CLIENT client;
// PVIGEM_TARGET pad;


// client = vigem_alloc();

// if (client == nullptr)
// {
//     std::cerr << "Uh, not enough memory to do that?!" << std::endl;
//     return -1;
// }
// const auto retval = vigem_connect(client);

// if (!VIGEM_SUCCESS(retval))
// {
//     std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << retval << std::endl;
//     return -1;
// }

// //
// // Allocate handle to identify new pad
// //
// pad = vigem_target_x360_alloc();

// //
// // Add client to the bus, this equals a plug-in event
// //
// const auto pir = vigem_target_add(client, pad);

// //
// // Error handling
// //
// if (!VIGEM_SUCCESS(pir))
// {
//     std::cerr << "Target plugin failed with error code: 0x" << std::hex << pir << std::endl;
//     return -1;
// }

// XUSB_REPORT report;
// report.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_DPAD_LEFT;
// report.sThumbLX = 0;
// report.sThumbLY = 0;
// report.sThumbRX = 0;
// report.sThumbRY = 0;
// report.bLeftTrigger = 0;
// report.bRightTrigger = 0;

// vigem_target_x360_update(client, pad, report);