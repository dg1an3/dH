#if !defined(AFX_BEAMPARAMCOLLIMCTRL_H__76008145_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_BEAMPARAMCOLLIMCTRL_H__76008145_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Beam.h"
#include "Association.h"

// BeamParamCollimCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBeamParamCollimCtrl dialog

class CBeamParamCollimCtrl : public CDialog, public CObserver
{
// Construction
public:
	CBeamParamCollimCtrl(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBeamParamCollimCtrl)
	enum { IDD = IDD_BEAMPARAMCOLLIM };
	int		m_nCollimAngle;
	int		m_nJawX1;
	int		m_nJawX2;
	int		m_nJawY1;
	int		m_nJawY2;
	//}}AFX_DATA

	CAssociation<CBeam> forBeam;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBeamParamCollimCtrl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBeamParamCollimCtrl)
	virtual BOOL OnInitDialog();
	afx_msg void OnApplyChanges();
	afx_msg void OnApplySpinChanges(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BEAMPARAMCOLLIMCTRL_H__76008145_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
