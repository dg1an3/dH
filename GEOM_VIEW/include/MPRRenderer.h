// MPRRenderer.h: interface for the CMPRRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPRRENDERER_H__AA354005_2AF1_42D3_9D11_C0FFAD91DC53__INCLUDED_)
#define AFX_MPRRENDERER_H__AA354005_2AF1_42D3_9D11_C0FFAD91DC53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OpenGLRenderer.h"

class CMPRRenderer : public COpenGLRenderer  
{
public:
	CMPRRenderer(COpenGLView *pView);
	virtual ~CMPRRenderer();

};

#endif // !defined(AFX_MPRRENDERER_H__AA354005_2AF1_42D3_9D11_C0FFAD91DC53__INCLUDED_)
