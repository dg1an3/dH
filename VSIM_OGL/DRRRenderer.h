// DRRRenderer.h: interface for the CDRRRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRRRENDERER_H__88FB0320_0C35_11D5_9E4E_00B0D0609AB0__INCLUDED_)
#define AFX_DRRRENDERER_H__88FB0320_0C35_11D5_9E4E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <OpenGLRenderer.h>
#include <Association.h>
#include <Volumep.h>

class CDRRRenderer : public COpenGLRenderer  
{
public:
	CDRRRenderer(COpenGLView *pView);
	virtual ~CDRRRenderer();

	// association with a CVolume that contains the volumetric data
	CAssociation< CVolume< short > > forVolume;

	CValue< CMatrix<4> > volumeTransform;

	void ComputeDRR();

	// renders the DRR
	virtual void OnRenderScene();

	virtual void DrawScene();

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

#endif // !defined(AFX_DRRRENDERER_H__88FB0320_0C35_11D5_9E4E_00B0D0609AB0__INCLUDED_)
