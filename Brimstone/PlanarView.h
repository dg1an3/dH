#if !defined(AFX_PLANARVIEW_H__AB0EB4F4_0BF4_4135_935C_FBE17772A6A8__INCLUDED_)
#define AFX_PLANARVIEW_H__AB0EB4F4_0BF4_4135_935C_FBE17772A6A8__INCLUDED_

#include <Dib.h>
#include <Volumep.h>
#include <Structure.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlanarView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlanarView window

class CPlanarView : public CWnd
{
// Construction
public:
	CPlanarView();

// Attributes
public:
	void SetVolume(CVolume<VOXEL_REAL> *pVolume, int nVolumeAt = 0);
	void SetBasis(const CMatrixD<4>& mBasis);
	void SetWindowLevel(REAL win, REAL cen, int nVolumeAt = 0);
	void SetLUT(CArray<COLORREF, COLORREF>& arrLUT, int nVolumeAt = 0);

	void AddStructure(CStructure *pStruct);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanarView)
	//}}AFX_VIRTUAL

// Implementation
public:
	CSeries * m_pSeries;
	void DrawContours(CDC *pDC);
	void DrawImages(CDC *pDC);
	void DrawIsocurves(CVolume<VOXEL_REAL> *pVolume, REAL c, CDC *pDC);
	REAL m_alpha;
	void SetAlpha(REAL alpha);
	CVolume<VOXEL_REAL> * m_pVolume[2];
	CVolume<VOXEL_REAL> m_volumeResamp[2];
	REAL m_window[2];
	REAL m_level[2];

	REAL m_windowStart;
	REAL m_levelStart;
	CPoint m_ptWinLevStart;
	BOOL m_bWindowLeveling;

	CArray<COLORREF, COLORREF> m_arrLUT[2];

	CArray<COLORREF, COLORREF> m_arrPixels;
	CDib m_dib;

	CTypedPtrArray<CPtrArray, CStructure *> m_arrStructures;

	virtual ~CPlanarView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlanarView)
	afx_msg void OnPaint();
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLANARVIEW_H__AB0EB4F4_0BF4_4135_935C_FBE17772A6A8__INCLUDED_)
