// OpenGLRenderer.h: interface for the COpenGLRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPENGLRENDERER_H__0E2B2434_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_)
#define AFX_OPENGLRENDERER_H__0E2B2434_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Observer.h"
#include "Value.h"
#include "Matrix.h"

class COpenGLView;

class COpenGLRenderer : public CObject, public CObserver  
{
public:
	COpenGLRenderer(COpenGLView *pView);
	virtual ~COpenGLRenderer();

	// color for the renderer (use depends on objects being rendered)
	CValue< COLORREF > myColor;

	// defines the modelview matrix for this renderer
	CValue< CMatrix<4> > myModelviewMatrix;

	// turns on/off the renderer;
	CValue< BOOL > isEnabled;

	// Over-rides
	virtual void OnRenderScene();

	// triggers invalidation
	virtual void OnChange(CObservable *pSource);

	void Invalidate();
	virtual void DrawScene();

protected:
	COpenGLView * m_pView;

private:
	int m_nDrawList;
};

#endif // !defined(AFX_OPENGLRENDERER_H__0E2B2434_E5C1_11D4_9E2F_00B0D0609AB0__INCLUDED_)
