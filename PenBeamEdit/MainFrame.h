// MainFrame.h - Main application window
#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/aui/auibar.h>

class PenBeamEditCanvas;
class CPlan;

class MainFrame : public wxFrame
{
public:
    MainFrame(wxWindow* parent);
    virtual ~MainFrame();

private:
    void CreateMenuBar();
    void CreateToolBar();
    void CreateStatusBar();

    // Event handlers
    void OnFileNew(wxCommandEvent& event);
    void OnFileOpen(wxCommandEvent& event);
    void OnFileSave(wxCommandEvent& event);
    void OnFileImport(wxCommandEvent& event);
    void OnFileExit(wxCommandEvent& event);
    void OnHelpAbout(wxCommandEvent& event);

    // Member variables
    PenBeamEditCanvas* m_canvas;
    CPlan* m_plan;

    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum
{
    ID_FILE_IMPORT = wxID_HIGHEST + 1
};

#endif // MAINFRAME_H
