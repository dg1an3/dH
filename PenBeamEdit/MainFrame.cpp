// MainFrame.cpp - Main application window implementation
#include "MainFrame.h"
#include "PenBeamEditCanvas.h"
#include <Plan.h>
#include <Series.h>
#include <Structure.h>
#include <Beam.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/aboutdlg.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_NEW, MainFrame::OnFileNew)
    EVT_MENU(wxID_OPEN, MainFrame::OnFileOpen)
    EVT_MENU(wxID_SAVE, MainFrame::OnFileSave)
    EVT_MENU(ID_FILE_IMPORT, MainFrame::OnFileImport)
    EVT_MENU(wxID_EXIT, MainFrame::OnFileExit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnHelpAbout)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(wxWindow* parent)
    : wxFrame(parent, wxID_ANY, "PenBeamEdit",
              wxDefaultPosition, wxSize(1024, 768)),
      m_canvas(nullptr),
      m_plan(nullptr)
{
    // Create menu bar
    CreateMenuBar();

    // Create toolbar
    CreateToolBar();

    // Create status bar
    CreateStatusBar(4);
    SetStatusText("Ready", 0);

    // Create main canvas
    m_canvas = new PenBeamEditCanvas(this);

    // Create new plan
    m_plan = new CPlan();
}

MainFrame::~MainFrame()
{
    if (m_plan)
        delete m_plan;
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar;

    // File menu
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(wxID_NEW, "&New\tCtrl+N", "Create a new plan");
    fileMenu->Append(wxID_OPEN, "&Open\tCtrl+O", "Open an existing plan");
    fileMenu->Append(wxID_SAVE, "&Save\tCtrl+S", "Save the current plan");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_FILE_IMPORT, "&Import...", "Import density data");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Exit the application");
    menuBar->Append(fileMenu, "&File");

    // Help menu
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, "&About...", "About PenBeamEdit");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);
}

void MainFrame::CreateToolBar()
{
    wxToolBar* toolbar = CreateToolBar(wxTB_HORIZONTAL | wxTB_FLAT);

    toolbar->AddTool(wxID_NEW, "New", wxArtProvider::GetBitmap(wxART_NEW),
                    "New plan");
    toolbar->AddTool(wxID_OPEN, "Open", wxArtProvider::GetBitmap(wxART_FILE_OPEN),
                    "Open plan");
    toolbar->AddTool(wxID_SAVE, "Save", wxArtProvider::GetBitmap(wxART_FILE_SAVE),
                    "Save plan");

    toolbar->Realize();
}

void MainFrame::OnFileNew(wxCommandEvent& event)
{
    if (m_plan)
        delete m_plan;

    m_plan = new CPlan();

    if (m_canvas)
        m_canvas->SetPlan(m_plan);

    SetStatusText("New plan created", 0);
}

void MainFrame::OnFileOpen(wxCommandEvent& event)
{
    wxFileDialog dialog(this, "Open Plan File", "", "",
                       "Plan files (*.pln)|*.pln|All files (*.*)|*.*",
                       wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxString path = dialog.GetPath();
        // TODO: Implement plan loading from file
        SetStatusText("Plan opened: " + path, 0);
    }
}

void MainFrame::OnFileSave(wxCommandEvent& event)
{
    wxFileDialog dialog(this, "Save Plan File", "", "",
                       "Plan files (*.pln)|*.pln|All files (*.*)|*.*",
                       wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxString path = dialog.GetPath();
        // TODO: Implement plan saving to file
        SetStatusText("Plan saved: " + path, 0);
    }
}

void MainFrame::OnFileImport(wxCommandEvent& event)
{
    wxFileDialog dialog(this, "Import Density Data", "", "density.dat",
                       "Data files (*.dat)|*.dat|All files (*.*)|*.*",
                       wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxString path = dialog.GetPath();
        // TODO: Implement density data import
        // This would be similar to CPenBeamEditApp::OnFileImport() from the MFC version
        SetStatusText("Imported: " + path, 0);
    }
}

void MainFrame::OnFileExit(wxCommandEvent& event)
{
    Close(true);
}

void MainFrame::OnHelpAbout(wxCommandEvent& event)
{
    wxAboutDialogInfo info;
    info.SetName("PenBeamEdit");
    info.SetVersion("1.0");
    info.SetDescription("Pencil beam radiotherapy treatment plan editor");
    info.SetCopyright("(C) 2007-2025");

    wxAboutBox(info);
}
