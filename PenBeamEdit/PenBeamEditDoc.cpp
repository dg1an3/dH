// PenBeamEditDoc.cpp : implementation of the CPenBeamEditDoc class
//

#include "stdafx.h"
#include "PenBeamEdit.h"

#include "PenBeamEditDoc.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

template<class VOXEL_TYPE>
BOOL ReadAsciiImage(LPCTSTR lpszPathName, CVolume<VOXEL_TYPE> *pImage)
{
	CStdioFile inFile;
	
	if (!inFile.Open(lpszPathName, CFile::modeRead | CFile::typeText))
		return FALSE;

	int nSize = 0;
	CArray<VOXEL_TYPE, VOXEL_TYPE> arrIntensity;
	CString strLine;
	while (inFile.ReadString(strLine))
	{
		strLine.TrimLeft();
		strLine.TrimRight();
		if (strLine == "")
		{
			if (nSize == 0)
				nSize = arrIntensity.GetSize();
		}
		else
		{
			double density;
			sscanf(strLine.GetBuffer(100), "%lf", &density);
			arrIntensity.Add((VOXEL_TYPE) density);
		}
	}

	pImage->width.Set(nSize);
	pImage->height.Set(nSize);
	pImage->depth.Set(1);

	int nAtVoxel = 0;
	for (int nAtY = 0; nAtY < nSize; nAtY++)
		for (int nAtX = 0; nAtX < nSize; nAtX++)
			pImage->GetVoxels()[0][nAtY][nAtX] = arrIntensity[nAtVoxel++];

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditDoc

IMPLEMENT_DYNCREATE(CPenBeamEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CPenBeamEditDoc, CDocument)
	//{{AFX_MSG_MAP(CPenBeamEditDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditDoc construction/destruction

CPenBeamEditDoc::CPenBeamEditDoc()
: m_histogram(&m_density, &m_region)
{
	// TODO: add one-time construction code here

}

CPenBeamEditDoc::~CPenBeamEditDoc()
{
}

BOOL CPenBeamEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditDoc serialization

void CPenBeamEditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditDoc diagnostics

#ifdef _DEBUG
void CPenBeamEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPenBeamEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPenBeamEditDoc commands

// opens a PenBeam density file
BOOL CPenBeamEditDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
//	if (!CDocument::OnOpenDocument(lpszPathName))
//		return FALSE;
	
	CString strDensityFilename(lpszPathName);
	strDensityFilename += "\\density.dat";
	BOOL bResult = ReadAsciiImage(strDensityFilename, &m_density);

	// initialize the region
	m_region.width.Set(m_density.width.Get());
	m_region.height.Set(m_density.height.Get());
	m_region.depth.Set(m_density.depth.Get());

	// initialize the region to all ones
	for (int nAtY = 0; nAtY < m_region.height.Get(); nAtY++)
		for (int nAtX = 0; nAtX < m_region.width.Get(); nAtX++)
			if (nAtX > 45 && nAtX < 55)
				m_region.GetVoxels()[0][nAtY][nAtX] = 1.0;

#define PI (atan(1.0) * 4.0)
#define SIGMA 7.0

	// and form the total dose
	m_totalDose.width.Set(m_density.width.Get());
	m_totalDose.height.Set(m_density.height.Get());
	m_totalDose.depth.Set(m_density.depth.Get());

	// read the pencil beams, forming the summed dose
	double maxDose = 0.0;
	for (int nAt = 1; nAt < 100; nAt++)
	{
		CString strPencilBeamFilename;
		strPencilBeamFilename.Format("%s\\dose%i.dat", lpszPathName, nAt);

		CVolume<double> *pPencilBeam = new CVolume<double>();
		bResult = bResult && ReadAsciiImage(strPencilBeamFilename, pPencilBeam);
		m_arrPencilBeams.Add(pPencilBeam);

		// set the weights to a gaussian distribution
		double weight = 1.0 / sqrt(2 * PI * SIGMA) 
			* exp(- (double)((50 - nAt) * (50 - nAt)) / (SIGMA * SIGMA));
		m_arrWeights.Add(weight);

		for (int nAtY = 0; nAtY < m_totalDose.height.Get(); nAtY++)
			for (int nAtX = 0; nAtX < m_totalDose.width.Get(); nAtX++)
			{
				m_totalDose.GetVoxels()[0][nAtY][nAtX] += 
					weight * pPencilBeam->GetVoxels()[0][nAtY][nAtX];
				maxDose = max(maxDose, m_totalDose.GetVoxels()[0][nAtY][nAtX]);
			}
	}

	// normalize the total dose
	for (nAtY = 0; nAtY < m_totalDose.height.Get(); nAtY++)
		for (int nAtX = 0; nAtX < m_totalDose.width.Get(); nAtX++)
			m_totalDose.GetVoxels()[0][nAtY][nAtX] /= maxDose;

	m_histogram.volume.Set(&m_totalDose);

	if (bResult)
		UpdateAllViews(NULL);

	return bResult;
}
