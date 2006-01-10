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
// class CPlanarView 
//
// represents a single MPR
/////////////////////////////////////////////////////////////////////////////
class CPlanarView : public CWnd
{
public:
// Construction
	CPlanarView();
	virtual ~CPlanarView();

// Attributes

	// displayed volumes
	void SetVolume(CVolume<VOXEL_REAL> *pVolume, int nVolumeAt = 0);

	// adds another structure for display
	void AddStructure(CStructure *pStruct);

	// display state
	void SetWindowLevel(REAL win, REAL cen, int nVolumeAt = 0);
	void SetLUT(CArray<COLORREF, COLORREF>& arrLUT, int nVolumeAt = 0);
	void SetAlpha(REAL alpha);

	// sets up specifics of zoom / pan
	void InitZoomCenter(void);
	void SetCenter(const CVectorD<3>& vCenter);
	void SetZoom(REAL zoom = 1.0);
	void SetBasis(const CMatrixD<4>& mBasis);

// Operations

	// drawing helpers
	void DrawContours(CDC *pDC);
	void DrawImages(CDC *pDC);
	void DrawIsocurves(CVolume<VOXEL_REAL> *pVolume, REAL c, CDC *pDC);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanarView)
	//}}AFX_VIRTUAL

// Implementation
public:

	// displayed volumes
	CVolume<VOXEL_REAL> * m_pVolume[2];

	// structures
	CSeries * m_pSeries;
	CTypedPtrArray<CPtrArray, CStructure *> m_arrStructures;

	// drawing temporaries
	CVolume<VOXEL_REAL> m_volumeResamp[2];
	CArray<COLORREF, COLORREF> m_arrPixels;
	CDib m_dib;

	// image display parameters
	REAL m_window[2];
	REAL m_level[2];
	REAL m_alpha;
	CArray<COLORREF, COLORREF> m_arrLUT[2];

	// window / level mode 
	bool m_bWindowLeveling;
	REAL m_windowStart;
	REAL m_levelStart;

	// stores zoom / pan 
	CVectorD<3> m_vCenter;
	REAL m_zoom;

	// zoom / pan mode
	bool m_bZooming;
	REAL m_zoomStart;

	bool m_bPanning;
	CVectorD<3> m_vPtStart;
	CVectorD<3> m_vCenterStart;
	CMatrixD<4> m_mBasisStart;

	// helper to start starting point of mouse operation
	CPoint m_ptOpStart;

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlanarView)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// callback for volume change
	void OnVolumeChanged(CObservableEvent * pEv, void * pParam);
};	// class CPlanarView 


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLANARVIEW_H__AB0EB4F4_0BF4_4135_935C_FBE17772A6A8__INCLUDED_)
