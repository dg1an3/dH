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
	int m_nShift;
	int m_nSteps;
	virtual void OnChange(CObservable *pSource);
	CDRRRenderer(COpenGLView *pView);
	virtual ~CDRRRenderer();

	// association with a CVolume that contains the volumetric data
	CAssociation< CVolume< short > > forVolume;

	CValue< CMatrix<4> > volumeTransform;

	void ComputeDRR();

	// renders the DRR
	virtual void OnRenderScene();

	virtual void DrawScene();

public:
	CArray<int, int> m_arrPixels;

	BOOL m_bRecomputeDRR;
	float m_scale;
	float m_bias;
};

#endif // !defined(AFX_DRRRENDERER_H__88FB0320_0C35_11D5_9E4E_00B0D0609AB0__INCLUDED_)
