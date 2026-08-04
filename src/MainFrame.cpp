#include "MainFrame.h"
#include "ScaledStaticBitmap.h"
#include <wx/wx.h>

MainFrame::MainFrame(const wxString& title): wxFrame(nullptr, wxID_ANY, title){
/*
    
    MainFrame children layout. Os are individual widgets, panels contain a sizer that aranges the widgets inside them.

                leftPanel                       rightPanel    
    ┌───────────────────────────┬───────────────────────────────────────┐
    │                           │                                       │
    │                           │                   O                   │
    │                           │                   O                   │
    │            O              │                                       │
    │                           │              spritePanel              │
    │                           │  ┌──────────────────┬────────────────┐│
    │            O              │  │         O        │       O        ││
    │                           │  ├──────────────────┼────────────────┤│
    │                           │  │         O        │       O        ││
    │                           │  ├──────────────────┼────────────────┤│
    │                           │  │         O        │       O        ││
    │            O              │  ├──────────────────┼────────────────┤│
    │                           │  │         O        │       O        ││
    │                           │  └──────────────────┴────────────────┘│
    │                           │                    O                  │
    │                           │                    O                  │
    |                           |                    O                  |
    │                           │                  ipPanel              │
    │                           │  ┌─────────────────┬─────────────────┐│
    │                           │  │         O       │        O        ││
    │                           │  └─────────────────┴─────────────────┘│
    └───────────────────────────┴───────────────────────────────────────┘
    
*/

    wxInitAllImageHandlers();

    //Panels splitting gui into left and right halves

    wxPanel* leftPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(200,300));
    leftPanel->SetBackgroundColour(*wxWHITE);
    wxPanel* rightPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(200,300));
    rightPanel->SetBackgroundColour(*wxWHITE);

    //
    //Left Panel Children
    //

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString webcams;
    //TODO: populate webcams with actual user devices
    webcams.Add("webcam 1");
    webcams.Add("webcam 2");
    wxChoice* webcamChoice = new wxChoice(leftPanel, wxID_ANY, wxDefaultPosition, wxSize(100, 40), webcams);

    //TODO: Temp image to represent area where video stream would go
    ScaledStaticBitmap* videoBitmap = new ScaledStaticBitmap(leftPanel, "C:/Users/spang/Desktop/Projects/ShinyAssistant/battletemp.png");
    
    wxStaticText* encounterCounter = new wxStaticText(leftPanel, wxID_ANY, "Encounters: 10000", wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxST_ELLIPSIZE_START);
    auto font = encounterCounter->GetFont();
    font.SetPixelSize(wxSize(0,40));
    encounterCounter->SetFont(font);
    //TODO: Load encounter number from config
    encounterCounter->SetLabel("Encounters: 10101");

    leftSizer->AddSpacer(20);
    leftSizer->Add(webcamChoice, wxSizerFlags().Center().Shaped());
    leftSizer->AddSpacer(20);
    leftSizer->Add(videoBitmap, wxSizerFlags().Expand().Shaped().Border(wxALL, 10).Center());
    leftSizer->Add(encounterCounter, wxSizerFlags().CenterHorizontal().Border(wxLEFT|wxRIGHT, 5));
    
    
    leftPanel->SetSizer(leftSizer);
    leftSizer->SetSizeHints(leftPanel);
    ///
    //End of Left Children
    //

    //
    //Right Panel Children
    //

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString species;
    species.Add("Bulbasaur");
    species.Add("Charmander");
    species.Add("Squirtle");
    wxChoice* speciesChoice = new wxChoice(rightPanel, wxID_ANY, wxDefaultPosition, wxSize(100,-1), species);

    rightSizer->AddSpacer(20);
    rightSizer->Add(speciesChoice, wxSizerFlags().Center());

    rightPanel->SetSizer(rightSizer);
    rightSizer->SetSizeHints(rightPanel);
    //
    //End of Right Children
    //

    wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(leftPanel, wxSizerFlags().Proportion(3).Expand());
    boxSizer->AddSpacer(2);
    boxSizer->Add(rightPanel, wxSizerFlags().Proportion(2).Expand());
    SetSizerAndFit(boxSizer);
    boxSizer->SetSizeHints(this);
}