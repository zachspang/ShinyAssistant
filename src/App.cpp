#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>
#include <windows.h>
#include <curl/curl.h>

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    MainFrame* mainFrame = new MainFrame("ShinyAssistant");
    mainFrame->SetClientSize(800, 650);
    mainFrame->Center();
    mainFrame->Show();
    return true;
}

int App::OnExit() {
    curl_global_cleanup();
    return wxApp::OnExit();
}