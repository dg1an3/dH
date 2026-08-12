// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
//
// RenderControlDlg.h : modeless dialog that drives CPlanarView rendering
// through ordinary Win32 controls so that UIAutomation / FlaUI exploratory
// tests can set window/level, zoom, pan, slice and dose-blend deterministically
// (i.e. without synthesizing mouse drags) and capture the graphical windows.
//
#if !defined(AFX_RENDERCONTROLDLG_H__INCLUDED_)
#define AFX_RENDERCONTROLDLG_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBrimstoneView;

/////////////////////////////////////////////////////////////////////////////
// CRenderControlDlg
//
// A modeless dialog. Every render parameter that is normally only reachable
// through a mouse gesture in CPlanarView is exposed here as a labelled edit
// box with a stable control id (which Win32 surfaces to UIAutomation as the
// AutomationId), plus push buttons to apply/refresh/step-slice/capture.
/////////////////////////////////////////////////////////////////////////////
class CRenderControlDlg : public CDialog
{
// Construction
public:
	CRenderControlDlg(CBrimstoneView* pView, CWnd* pParent = NULL);

	enum { IDD = IDD_RENDERCTRL };

// Operations
public:
	// pushes the current planar-view render state into the dialog controls
	void RefreshFromView();

// Overrides
	//{{AFX_VIRTUAL(CRenderControlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// the view that owns the planar view and the two graphs
	CBrimstoneView* m_pView;

	// reads a double from an edit control (returns bDefault on parse failure)
	bool GetDlgItemDouble(int nID, double& value);

	// pushes the dialog values into the planar view and forces a redraw
	void ApplyToView();

	// steps the displayed slice by nSlices (+/-) using the volume spacing
	void StepSlice(int nSlices);

	// captures all graphical child windows to .bmp files using the prefix edit
	void CaptureWindows();

	// saves a window's client area to a 24-bit .bmp file; returns success
	static bool SaveWndToBmp(CWnd* pWnd, LPCTSTR lpszPath);

	// shows a one-line message in the read-only status field
	void SetStatus(LPCTSTR lpszText);

	//{{AFX_MSG(CRenderControlDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnApply();
	afx_msg void OnRefresh();
	afx_msg void OnSlicePrev();
	afx_msg void OnSliceNext();
	afx_msg void OnCapture();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_RENDERCONTROLDLG_H__INCLUDED_)
