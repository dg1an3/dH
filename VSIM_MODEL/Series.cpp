// Series.cpp : implementation file
//

#include "stdafx.h"
// #include "vsim_ogl.h"
#include "Series.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSeries

IMPLEMENT_DYNCREATE(CSeries, CDocument)

CSeries::CSeries()
{
}

CSeries::~CSeries()
{
}

CString CSeries::GetFileName()
{
	// see if its filename is correct
	int nAt = GetPathName().ReverseFind('\\');
	CString strFilename = GetPathName().Mid(nAt+1);

	return strFilename;
}

CString CSeries::GetFileRoot()
{
	CString strName = GetFileName();
	int nDot = strName.ReverseFind('.');
	if (nDot >= 0)
		return strName.Left(nDot);

	return strName;
}

BEGIN_MESSAGE_MAP(CSeries, CDocument)
	//{{AFX_MSG_MAP(CSeries)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSeries diagnostics

#ifdef _DEBUG
void CSeries::AssertValid() const
{
	CDocument::AssertValid();

	// m_arrStructures.AssertValid();
}

void CSeries::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);

	PUSH_DUMP_DEPTH(dc);

	// dump the plan's beams
	// m_arrStructures.Dump(dc);

	POP_DUMP_DEPTH(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSeries serialization

void CSeries::Serialize(CArchive& ar)
{
	CDocument::Serialize(ar);

	if (!ar.IsStoring())
	{
		// delete any existing structures
//		for (int nAt = 0; nAt < m_arrStructures.GetSize(); nAt++)
//			delete m_arrStructures[nAt];
//
//		m_arrStructures.RemoveAll();
		structures.RemoveAll();
	}

//	m_arrStructures.Serialize(ar);
	structures.Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CSeries commands

BOOL CSeries::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// delete any existing structures
//	for (int nAt = 0; nAt < m_arrStructures.GetSize(); nAt++)
//		delete m_arrStructures[nAt];
//
//	m_arrStructures.RemoveAll();
	structures.RemoveAll();

	return TRUE;
}
