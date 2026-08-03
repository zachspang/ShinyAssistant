#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#include <wx/wx.h>
#include <ViGEm/Client.h>

class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
};

class MyFrame : public wxFrame {
public:
    MyFrame();

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
};

enum {
    ID_Hello = 1
};

wxIMPLEMENT_APP(MyApp);
PVIGEM_CLIENT client;
PVIGEM_TARGET pad;

bool MyApp::OnInit() {
    // vigemclient test

    client = vigem_alloc();

    if (client == nullptr)
    {
        std::cerr << "Uh, not enough memory to do that?!" << std::endl;
        return -1;
    }
    const auto retval = vigem_connect(client);

    if (!VIGEM_SUCCESS(retval))
    {
        std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << retval << std::endl;
        return -1;
    }

    //
    // Allocate handle to identify new pad
    //
    pad = vigem_target_x360_alloc();

    //
    // Add client to the bus, this equals a plug-in event
    //
    const auto pir = vigem_target_add(client, pad);

    //
    // Error handling
    //
    if (!VIGEM_SUCCESS(pir))
    {
        std::cerr << "Target plugin failed with error code: 0x" << std::hex << pir << std::endl;
        return -1;
    }

    MyFrame* frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Hello wxWidgets") {

    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H", "Help string shown in status bar");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Ready");

    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent& event) {
    vigem_target_remove(client, pad);
    vigem_target_free(pad);
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox("This is a wxWidgets Hello World example",
                 "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event) {
    XINPUT_STATE state;

    XInputGetState(0, &state);

    XUSB_REPORT report;
    report.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_DPAD_LEFT;
    report.sThumbLX = 0;
    report.sThumbLY = 0;
    report.sThumbRX = 0;
    report.sThumbRY = 0;
    report.bLeftTrigger = 0;
    report.bRightTrigger = 0;

    vigem_target_x360_update(client, pad, report);
}