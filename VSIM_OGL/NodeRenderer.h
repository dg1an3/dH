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

class CMolding;

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

	// stores the radius of the border
	CValue< double > borderRadius;

	// association to the node to be rendered
	// CAssociation< CNode > forNode;

protected:
	// computes the node's x-size (ellipse a)
	CValue< double > width;

	// computes the node's rectangularity (from sqrt(2.0e) to 1.0))
	CValue< double > rectangularity;

	// evaluates the elliptangle at a point
	CVector<3> EvalEllipseMolding(double theta, double a, double b);
	CMolding *RenderMoldingSection(double angle_start, double angle_stop, 
		double a, double b, CMolding *pOld = NULL);
	virtual void OnRenderScene();

	// handler for activation change
	void OnActivationChanged(CObservableObject *pSource, void *pOldValue);
};

#endif // !defined(AFX_NODERENDERER_H__35348172_09C8_401B_A1D6_955CE7CE536D__INCLUDED_)
