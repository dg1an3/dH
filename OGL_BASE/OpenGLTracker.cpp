// OpenGLTracker.cpp: implementation of the COpenGLTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "vsim_ogl.h"
#include "OpenGLTracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenGLTracker::COpenGLTracker(COpenGLView *pView)
: m_pView(pView)
{

}

COpenGLTracker::~COpenGLTracker()
{

}
