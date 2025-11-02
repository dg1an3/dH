// PenBeamEditCanvas.cpp - Main drawing canvas implementation
#include "PenBeamEditCanvas.h"
#include <Plan.h>
#include <Series.h>
#include <Volumep.h>
#include <Histogram.h>
#include <wx/dcbuffer.h>

wxBEGIN_EVENT_TABLE(PenBeamEditCanvas, wxPanel)
    EVT_PAINT(PenBeamEditCanvas::OnPaint)
    EVT_SIZE(PenBeamEditCanvas::OnSize)
    EVT_LEFT_DOWN(PenBeamEditCanvas::OnMouseDown)
wxEND_EVENT_TABLE()

PenBeamEditCanvas::PenBeamEditCanvas(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_plan(nullptr),
      m_histogram(nullptr),
      m_graphPanel(nullptr)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT); // For double buffering
    LoadColormap();

    // Create histogram object
    m_histogram = new CHistogram();

    // Create graph panel (right side)
    m_graphPanel = new wxPanel(this, wxID_ANY);
    m_graphPanel->SetBackgroundColour(*wxWHITE);
}

PenBeamEditCanvas::~PenBeamEditCanvas()
{
    if (m_histogram)
        delete m_histogram;
}

void PenBeamEditCanvas::SetPlan(CPlan* plan)
{
    m_plan = plan;
    UpdateView();
}

void PenBeamEditCanvas::UpdateView()
{
    if (!m_plan || !m_plan->GetSeries())
        return;

    // Update histogram
    m_histogram->SetVolume(m_plan->GetDoseMatrix());

    // TODO: Set histogram region from structure

    Refresh();
}

void PenBeamEditCanvas::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);

    // Clear background
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();

    if (!m_plan || !m_plan->GetSeries())
    {
        dc.SetTextForeground(*wxWHITE);
        dc.DrawText("No plan loaded", 10, 10);
        return;
    }

    CVolume<short>& density = m_plan->GetSeries()->volume;
    CVolume<double>& totalDose = *m_plan->GetDoseMatrix();

    wxSize clientSize = GetClientSize();
    int drawWidth = clientSize.x / 2; // Left half for dose display
    int drawHeight = clientSize.y;

    int nRowSize = density.GetWidth();
    if (nRowSize == 0)
        return;

    int nDrawSize = wxMin(drawWidth, drawHeight);
    int nPixelSize = nDrawSize / nRowSize;
    if (nPixelSize < 1)
        nPixelSize = 1;

    // Draw dose distribution
    for (int nAtY = 0; nAtY < nRowSize; nAtY++)
    {
        for (int nAtX = 0; nAtX < nRowSize; nAtX++)
        {
            double dose = totalDose.GetVoxels()[0][nAtY][nAtX];
            double intensity = density.GetVoxels()[0][nAtY][nAtX] / 1000.0;

            wxColour color = GetDoseColor(dose, intensity);

            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(color));

            dc.DrawRectangle(
                nAtX * nPixelSize,
                nAtY * nPixelSize,
                nPixelSize + 1,
                nPixelSize + 1
            );
        }
    }
}

void PenBeamEditCanvas::OnSize(wxSizeEvent& event)
{
    wxSize clientSize = GetClientSize();

    // Position graph panel on right half
    if (m_graphPanel)
    {
        m_graphPanel->SetSize(clientSize.x / 2, 0,
                             clientSize.x / 2, clientSize.y);
    }

    Refresh();
    event.Skip();
}

void PenBeamEditCanvas::OnMouseDown(wxMouseEvent& event)
{
    // TODO: Handle mouse interaction
    event.Skip();
}

void PenBeamEditCanvas::LoadColormap()
{
    // Create a rainbow colormap (similar to the original)
    // Red -> Yellow -> Green -> Cyan -> Blue
    int numColors = 256;
    m_colormap.reserve(numColors);

    for (int i = 0; i < numColors; i++)
    {
        double t = (double)i / (numColors - 1);

        int r, g, b;

        if (t < 0.25)
        {
            // Blue to Cyan
            double local_t = t / 0.25;
            r = 0;
            g = (int)(255 * local_t);
            b = 255;
        }
        else if (t < 0.5)
        {
            // Cyan to Green
            double local_t = (t - 0.25) / 0.25;
            r = 0;
            g = 255;
            b = (int)(255 * (1.0 - local_t));
        }
        else if (t < 0.75)
        {
            // Green to Yellow
            double local_t = (t - 0.5) / 0.25;
            r = (int)(255 * local_t);
            g = 255;
            b = 0;
        }
        else
        {
            // Yellow to Red
            double local_t = (t - 0.75) / 0.25;
            r = 255;
            g = (int)(255 * (1.0 - local_t));
            b = 0;
        }

        m_colormap.push_back(wxColour(r, g, b));
    }
}

wxColour PenBeamEditCanvas::GetDoseColor(double dose, double intensity)
{
    if (m_colormap.empty())
        return *wxBLACK;

    int colorIndex = (int)(dose * (m_colormap.size() - 1));
    colorIndex = wxMin(colorIndex, (int)m_colormap.size() - 1);
    colorIndex = wxMax(colorIndex, 0);

    wxColour color = m_colormap[colorIndex];

    // Modulate by density intensity
    int r = (int)(color.Red() * intensity);
    int g = (int)(color.Green() * intensity);
    int b = (int)(color.Blue() * intensity);

    return wxColour(r, g, b);
}
