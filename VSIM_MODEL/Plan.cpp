// Plan.cpp : implementation of the CPlan class
//

#include "stdafx.h"

#include "Plan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlan

BEGIN_MESSAGE_MAP(CPlan, CDocument)
	//{{AFX_MSG_MAP(CPlan)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlan 

CPlan::CPlan()
	: m_pSeries(NULL),
		isDoseValid(FALSE)
{
}

CPlan::~CPlan()
{
}

CSeries * CPlan::GetSeries()
{
	return m_pSeries;
}

void CPlan::SetSeries(CSeries *pSeries)
{
	m_pSeries = pSeries;
}

/////////////////////////////////////////////////////////////////////////////
// CPlan serialization

IMPLEMENT_DYNCREATE(CPlan, CDocument)

void CPlan::Serialize(CArchive& ar)
{
	// serialize the document
	CDocument::Serialize(ar);

	if (ar.IsStoring())
	{
		// write the filename for the series
		ar << ((m_pSeries != NULL) ? m_pSeries->GetFileName() : "");
	}
	else
	{
		// just read the filename into the member variable (OnOpenDocument
		//		actually finds the series and connects this object)
		ar >> m_strSeriesFilename;

		// now clear out any existing beams
		beams.RemoveAll();
	}

	beams.Serialize(ar);

	// serialize the dose matrix valid flag
	isDoseValid.Serialize(ar);

	// empty the dose matrix if it is not valid
	if (ar.IsStoring() && !isDoseValid.Get())
	{
		dose.width.Set(0);
		dose.height.Set(0);
		dose.depth.Set(0);
	}

	// serialize the dose matrix
	dose.Serialize(ar);

	// now make sure the series is loaded
	if (ar.IsStoring())
	{ 
		// now save the associated series
		GetSeries()->SaveModified();
	}
}

CDocTemplate * CPlan::m_pSeriesDocTemplate = NULL;

void CPlan::SetSeriesDocTemplate(CDocTemplate *pTemplate)
{
	m_pSeriesDocTemplate = pTemplate;
}

/////////////////////////////////////////////////////////////////////////////
// CPlan diagnostics

#ifdef _DEBUG
void CPlan::AssertValid() const
{
	CDocument::AssertValid();

	// make sure the plan's beams are valid
	// m_arrBeams.AssertValid();
}

void CPlan::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);

	int nOrigDepth = dc.GetDepth();
	dc.SetDepth(nOrigDepth + 1);

	// dump the plan's beams
	// m_arrBeams.Dump(dc);

	dc.SetDepth(nOrigDepth);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPlan commands

BOOL CPlan::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// now clear out any existing beams
	beams.RemoveAll();

	// delete pointer to series
	m_pSeries = NULL;

	return TRUE;
}

BOOL CPlan::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// call CDocument::OnOpenDocument for serialization behavior
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// now try to find the series
	m_pSeries = NULL;

	// pass through the path name to the series
	POSITION pos = m_pSeriesDocTemplate->GetFirstDocPosition();
	do 
	{
		// get the next loaded CSeries
		m_pSeries = (CSeries *)m_pSeriesDocTemplate->GetNextDoc(pos);	

		// see if it's filename is correct
		if ((m_pSeries != NULL) 
				&& (m_pSeries->GetFileName() != m_strSeriesFilename))
		{
			m_pSeries = NULL;
		}
	} while ((m_pSeries == NULL) && (pos != NULL));

	// if the series was not found...
	if (m_pSeries == NULL)
	{
		m_pSeries = (CSeries *)m_pSeriesDocTemplate->CreateNewDocument();
		if (!m_pSeries->OnOpenDocument(m_strSeriesFilename))
		{
			// series file not found
			delete m_pSeries;
			m_pSeries = NULL;
		}
		else
		{
			// set the file name for the series
			m_pSeries->SetPathName(m_strSeriesFilename);
		}
	}

	// we succesfully loaded the plan
	return TRUE;
}

CString CPlan::GetFileName()
{
	// see if its filename is correct
	int nAt = GetPathName().ReverseFind('\\');
	CString strFilename = GetPathName().Mid(nAt+1);

	return strFilename;
}

CString CPlan::GetFileRoot()
{
	CString strName = GetFileName();
	int nDot = strName.ReverseFind('.');
	if (nDot >= 0)
		return strName.Left(nDot);

	return strName;
}
