// PenBeamEditApp.h - wxWidgets application class
#ifndef PENBEAMEDIAPP_H
#define PENBEAMEDIAPP_H

#include <wx/wx.h>
#include <wx/docview.h>

class PenBeamEditApp : public wxApp
{
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;

private:
    wxDocManager* m_docManager;
};

wxDECLARE_APP(PenBeamEditApp);

#endif // PENBEAMEDIAPP_H
