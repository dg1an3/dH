// NodeRenderer.h: interface for the CNodeRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_)
#define AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Value.h>
#include <Association.h>

#include "OpenGLRenderer.h"

class CNodeRenderer : public COpenGLRenderer  
{
public:
	// constructor/destructor
	CNodeRenderer(COpenGLView *pView);
	virtual ~CNodeRenderer();

	// stores the current position of the node's center
	CValue< CVector<2> > position;

	// stores the node's current activation value
	CValue< double > activation;

	// stores the node's aspect ratio
	CValue< double > aspectRatio;

	// association to the node to be rendered
	// CAssociation< CNode > forNode;

protected:
	// computes the node's size (X and Y)
	CValue< CVector<2> > size;

	// computes the node's rectangularity (from sqrt(2.0e) to 1.0))
	CValue< double > rectangularity;

	// accessor for the inner and outer rectangles 
	//		(defines the shape of the node view)
	CRect GetOuterRect();
	CRect GetInnerRect();

	// accessors for the left-right and top-bottom ellipse rectangles
	CRect GetTopBottomEllipseRect();
	CRect GetLeftRightEllipseRect();

	virtual void OnRenderScene();

private:
	double m_r;
	CRect m_rectOuter;
};

#endif // !defined(AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_)
