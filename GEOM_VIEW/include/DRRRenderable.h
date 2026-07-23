// DRRRenderable.h: interface for the CDRRRenderable class.
//
// HISTORICAL FILE - Restored from commit 40e1350a (June 7, 2002)
// This file is NOT built - preserved for historical reference.
// Direct3D version of DRR rendering, refactored from VSIM_OGL/DRRRenderer.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRRRenderable_H__88FB0320_0C35_11D5_9E4E_00B0D0609AB0__INCLUDED_)
#define AFX_DRRRenderable_H__88FB0320_0C35_11D5_9E4E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Renderable.h>

#include <Volumep.h>

class CDRRRenderable : public CRenderable
{
public:
	CDRRRenderable(CSceneView *pView);
	virtual ~CDRRRenderable();

	// association with a CVolume that contains the volumetric data
	CVolume<short> *m_pVolume;

	CMatrix<4> m_mVolumeTransform;

	void ComputeDRR();

	// renders the DRR
	virtual void DescribeOpaque();

	virtual void Render();

	virtual void OnChange(CObservableObject *pSource, void *pOldValue);

	int m_nShift;
	int m_nSteps;

	int m_nResDiv;

public:
	// flag to indicate the DRR needs to be recomputed
	BOOL m_bRecomputeDRR;

	// image size
	int m_nImageWidth;
	int m_nImageHeight;
	int m_viewport[4];

	// pixels for the DRR
	CArray<int, int> m_arrPixels;

	// background for hi-res DRR
	CWinThread *m_pThread;
};

#endif // !defined(AFX_DRRRenderable_H__88FB0320_0C35_11D5_9E4E_00B0D0609AB0__INCLUDED_)
