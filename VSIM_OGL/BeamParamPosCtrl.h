#if !defined(AFX_BEAMPARAMPOSCTRL_H__76008146_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
#define AFX_BEAMPARAMPOSCTRL_H__76008146_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Beam.h"
#include "Association.h"

// BeamParamPosCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBeamParamPosCtrl dialog

class CBeamParamPosCtrl : public CDialog, public CObserver
{
// Construction
public:
	CBeamParamPosCtrl(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBeamParamPosCtrl)
	enum { IDD = IDD_BEAMPARAMPOSITION };
	int		m_nCouchAngle;
	int		m_nGantryAngle;
	int		m_nTableX;
	int		m_nTableY;
	int		m_nTableZ;
	//}}AFX_DATA

	CAssociation<CBeam> forBeam;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBeamParamPosCtrl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void OnChange(CObservableObject *pFromObject, void *pOldValue);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBeamParamPosCtrl)
	virtual BOOL OnInitDialog();
	afx_msg void OnApplyChanges();
	afx_msg void OnApplySpinChanges(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BEAMPARAMPOSCTRL_H__76008146_F6DF_11D4_9E3E_00B0D0609AB0__INCLUDED_)
