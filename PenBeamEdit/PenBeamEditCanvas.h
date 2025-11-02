// PenBeamEditCanvas.h - Main drawing canvas
#ifndef PENBEAMEDITCANVAS_H
#define PENBEAMEDITCANVAS_H

#include <wx/wx.h>
#include <wx/panel.h>
#include <vector>

class CPlan;
class CHistogram;
class CGraph;

class PenBeamEditCanvas : public wxPanel
{
public:
    PenBeamEditCanvas(wxWindow* parent);
    virtual ~PenBeamEditCanvas();

    void SetPlan(CPlan* plan);
    void UpdateView();

private:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseDown(wxMouseEvent& event);

    void LoadColormap();
    wxColour GetDoseColor(double dose, double intensity);

    CPlan* m_plan;
    CHistogram* m_histogram;
    wxPanel* m_graphPanel;

    std::vector<wxColour> m_colormap;

    wxDECLARE_EVENT_TABLE();
};

#endif // PENBEAMEDITCANVAS_H
