// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__3E80E028_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_)
#define AFX_MAINFRM_H__3E80E028_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_

#include <TabControlBar.h>
#include <ObjectExplorer.h>

#include <BeamParamPosCtrl.h>
#include <BeamParamCollimCtrl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

	// the object explorer
	CDialogBar m_wndExplorerCtrl;
	CObjectExplorer m_wndExplorer;

	// stores the explorer font
	CFont *m_pExplorerFont;

	// the beam parameter control
	CTabControlBar m_wndBeamParamCtrl;
	CBeamParamPosCtrl m_wndPosCtrl;
	CBeamParamCollimCtrl m_wndCollimCtrl;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	BOOL m_bViewLightpatch;
	BOOL m_bViewBeam;

	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__3E80E028_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_)
