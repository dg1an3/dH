// PenBeamEditApp.cpp - wxWidgets application implementation
#include "PenBeamEditApp.h"
#include "MainFrame.h"
#include "PenBeamEditCanvas.h"

#include <Plan.h>

wxIMPLEMENT_APP(PenBeamEditApp);

bool PenBeamEditApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    // Create the document manager
    m_docManager = new wxDocManager;

    // Create main frame
    MainFrame* frame = new MainFrame(nullptr);
    frame->Show(true);

    return true;
}

int PenBeamEditApp::OnExit()
{
    delete m_docManager;
    return wxApp::OnExit();
}
