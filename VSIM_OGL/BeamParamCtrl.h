#if !defined(AFX_BEAMPARAMCTRL_H__3E80E034_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_)
#define AFX_BEAMPARAMCTRL_H__3E80E034_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BeamParamCtrl.h : header file
//

#include <Beam.h>

/////////////////////////////////////////////////////////////////////////////
// CBeamParamCtrl dialog

class CBeamParamCtrl : public CDialogBar
{
// Construction
public:
	void SetBeam(CBeam *pBeam);
	CBeamParamCtrl(); // CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBeamParamCtrl)
	enum { IDD = IDD_BEAMPARAMDIALOG };
	CSpinButtonCtrl	m_spinCouch;
	CSpinButtonCtrl	m_spinGantry;
	CTabCtrl	m_tabCtrl;
	int		m_nCollimAngle;
	int		m_nGantryAngle;
	int		m_nCouchAngle;
	double	m_jawx1;
	double	m_jawx2;
	//}}AFX_DATA

	CBeam * m_pBeam;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBeamParamCtrl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBeamParamCtrl)
    afx_msg LONG OnInitDialog( UINT wParam, LONG lParam );
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnApplyChanges();
	afx_msg void OnDeltaposSpinCollim(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCouch(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinGantry(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BEAMPARAMCTRL_H__3E80E034_F21B_11D4_9E3C_00B0D0609AB0__INCLUDED_)
