// NodeRenderer.h: interface for the CNodeRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_)
#define AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OpenGLRenderer.h"

class CNodeRenderer : public COpenGLRenderer  
{
public:
	CNodeRenderer(COpenGLView *pView);
	virtual ~CNodeRenderer();

	virtual void OnRenderScene();
};

#endif // !defined(AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_)
