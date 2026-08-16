#include "App.h"
#include "MainFrame.h"
#include "VideoStream.h"
#include <wx/wx.h>
#include <windows.h>

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    
    MainFrame* mainFrame = new MainFrame("ShinyAssistant");
    mainFrame->SetClientSize(800, 650);
    mainFrame->Center();
    mainFrame->Show();
    return true;
}