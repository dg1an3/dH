// Plan.cpp : implementation of the CPlan class
//

#include "stdafx.h"

#include <UtilMacros.h>

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
		m_bRecomputeTotalDose(TRUE)
{
}

CPlan::~CPlan()
{
	// delete the beams
	for (int nAt = 0; nAt < m_arrBeams.GetSize(); nAt++)
	{
		delete m_arrBeams[nAt];
	}

	// delete the histograms
	for (nAt = 0; nAt < m_arrHistograms.GetSize(); nAt++)
	{
		delete m_arrHistograms[nAt];
	}

	// delete the target DVHs
	POSITION pos = m_mapTargetDVHs.GetStartPosition();
	while (NULL != pos)
	{
		CString strStructureName;
		CMatrixNxM<> *pmTargetDVH = NULL;
		m_mapTargetDVHs.GetNextAssoc(pos, strStructureName, 
			(void *&)pmTargetDVH);

		delete pmTargetDVH;
	}
}

CSeries * CPlan::GetSeries()
{
	return m_pSeries;
}

void CPlan::SetSeries(CSeries *pSeries)
{
	m_pSeries = pSeries;
}

int CPlan::GetBeamCount() const
{
	return m_arrBeams.GetSize();
}

CBeam * CPlan::GetBeamAt(int nAt)
{
	return (CBeam *) m_arrBeams.GetAt(nAt);
}

int CPlan::AddBeam(CBeam *pBeam)
{
	int nIndex = m_arrBeams.Add(pBeam);
	pBeam->GetChangeEvent().AddObserver(this, (ListenerFunction) OnBeamChange);

	m_bRecomputeTotalDose = TRUE;

	// a change has occurred, so update views
	UpdateAllViews(NULL);

	return nIndex;
}

CVolume<double> *CPlan::GetDoseMatrix()
{
	if (m_bRecomputeTotalDose)
	{
		// total the dose for all beams
		if (GetBeamCount() > 0)
		{
			// clear the total dose matrix
			m_dose.SetDimensions(GetBeamAt(0)->GetDoseMatrix()->GetWidth(), 
				GetBeamAt(0)->GetDoseMatrix()->GetHeight(),
				GetBeamAt(0)->GetDoseMatrix()->GetDepth());

			// clear the dose matrix
			m_dose.ClearVoxels();

			for (int nAt = 0; nAt < GetBeamCount(); nAt++)
			{
				// add this beam's dose matrix to the total
				m_dose.Accumulate(GetBeamAt(nAt)->GetDoseMatrix(),
					GetBeamAt(nAt)->GetWeight());
			}
		}

		// compute voxel count
/*		int nVoxelCount = m_dose.GetHeight() * m_dose.GetWidth() 
			* m_dose.GetDepth();

		if (nVoxelCount > 0)
		{
			// normalize the max dose
			double maxDose = m_dose.GetMax();
			double *pVoxels = &m_dose.GetVoxels()[0][0][0];
			for (int nAt = 0; nAt < nVoxelCount; nAt++)
			{
				pVoxels[nAt] /= maxDose;
			}
		}
*/
		m_bRecomputeTotalDose = FALSE;
	}

	return &m_dose;
}


// histogram accessor
CHistogram *CPlan::GetHistogram(CMesh *pStructure)
{
	for (int nAt = 0; nAt < GetSeries()->GetStructureCount(); nAt++)
	{
		if (GetSeries()->GetStructureAt(nAt) == pStructure)
		{
			if (m_arrHistograms.GetSize() <= nAt)
			{
				m_arrHistograms.SetSize(nAt+1);
			}

			if (NULL == m_arrHistograms[nAt])
			{
				CHistogram *pHisto = new CHistogram();
				pHisto->SetVolume(GetDoseMatrix());
				pHisto->SetRegion(pStructure->GetRegion());

				m_arrHistograms[nAt] = pHisto;
			}

			return (CHistogram *) m_arrHistograms[nAt];
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CPlan serialization

#define PLAN_SCHEMA 3
	// Schema 1: initial plan schema
	// Schema 2: + target DVH curves
	// Schema 3: + number of fields

IMPLEMENT_SERIAL(CPlan, CDocument, VERSIONABLE_SCHEMA | PLAN_SCHEMA)

void CPlan::Serialize(CArchive& ar)
{
	// store the schema for the beam object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : PLAN_SCHEMA;

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
		m_arrBeams.RemoveAll();
	}

	m_arrBeams.Serialize(ar);

	if (ar.IsLoading())
	{
		// set up as change listener for beams
		for (int nAt = 0; nAt < GetBeamCount(); nAt++)
		{
			GetBeamAt(nAt)->GetChangeEvent().AddObserver(this, 
				(ListenerFunction) OnBeamChange);
		}
	}

	BOOL m_bDoseValid = m_dose.GetWidth() > 0;

	// serialize the dose matrix valid flag
	SERIALIZE_VALUE(ar, m_bDoseValid);

	// empty the dose matrix if it is not valid
	if (ar.IsStoring() && !m_bDoseValid)
	{
		m_dose.SetDimensions(0, 0, 0);
	}

	// serialize the dose matrix
	m_dose.Serialize(ar);

	// trigger recalculation of total dose
	m_bRecomputeTotalDose = TRUE;

	// now make sure the series is loaded
	if (ar.IsStoring())
	{ 
		// now save the associated series
		GetSeries()->OnSaveDocument(GetSeries()->GetFileName());
	}

	CString strStructureName;
	CMatrixNxM<> *pmTargetDVH = NULL;
	if (ar.IsLoading())
	{
		ar >> strStructureName;
		pmTargetDVH = new CMatrixNxM<>;
		ar >> (*pmTargetDVH);
		m_mapTargetDVHs.SetAt(strStructureName, (void *) pmTargetDVH);
	}
	else
	{
		// delete the target DVHs
		POSITION pos = m_mapTargetDVHs.GetStartPosition();
		while (NULL != pos)
		{
			m_mapTargetDVHs.GetNextAssoc(pos, strStructureName, 
				(void *&)pmTargetDVH);

			ar << strStructureName;
			ar << (*pmTargetDVH);
		}
	}

	SERIALIZE_VALUE(ar, m_nFields);
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
	m_arrBeams.RemoveAll();

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

void CPlan::OnBeamChange(CObservableEvent *, void *)
{
	m_bRecomputeTotalDose = TRUE;
	m_dose.GetChangeEvent().Fire();
}

// the beam weights, as a vector
void CPlan::GetBeamWeights(CVectorN<>& vWeights)
{
	vWeights.SetDim(m_nFields);
	int nBeamletCount = GetBeamCount() / m_nFields;
	for (int nAtField = 0; nAtField < m_nFields; nAtField++)
	{
		vWeights[nAtField] = GetBeamAt(nAtField * nBeamletCount + nBeamletCount / 2)
			->GetWeight();
	}
#ifdef STAGGER_BEAM_WEIGHTS
	vWeights.SetDim(GetBeamCount());
	int nAtElem = 0;
	int nBeamletCount = GetBeamCount() / m_nFields;
	for (int nAtBeamlet = 0; nAtBeamlet < nBeamletCount; nAtBeamlet++)
	{
		for (int nAtField = 0; nAtField < m_nFields; nAtField++)
		{
			int nAtBeamletAlt = (nAtBeamlet + 1) / 2;
			int nAtBeamletSign = nAtBeamlet % 2 ? 1 : -1;
			vWeights[nAtElem] = GetBeamAt(nAtField * nBeamletCount + nBeamletCount / 2 + nAtBeamletAlt * nAtBeamletSign)
				->GetWeight();
			nAtElem++;
		}
	}
#endif
#ifdef STRAIGHT_BEAM_WEIGHTS
	for (int nAt = 0; nAt < GetBeamCount(); nAt++)
	{
		vWeights[nAt] = GetBeamAt(nAt)->GetWeight();
	}
#endif
}

void CPlan::SetBeamWeights(const CVectorN<>& vWeights)
{
	ASSERT(vWeights.GetDim() == m_nFields);
	int nBeamletCount = GetBeamCount() / m_nFields;
	for (int nAtField = 0; nAtField < m_nFields; nAtField++)
	{
		for (int nAtBeamlet = 0; nAtBeamlet < nBeamletCount; nAtBeamlet++)
		{
			double normWeight = Gauss<double>(nAtBeamlet - nBeamletCount / 2, 3.0)
				/ Gauss<double>(0.0, 3.0);
			GetBeamAt(nAtField * nBeamletCount + nAtBeamlet)
				->SetWeight(vWeights[nAtField] * normWeight);
		}
	}
#ifdef STAGGER_BEAM_WEIGHTS
	ASSERT(vWeights.GetDim() == GetBeamCount());
	int nAtElem = 0;
	int nBeamletCount = GetBeamCount() / m_nFields;
	for (int nAtBeamlet = 0; nAtBeamlet < nBeamletCount; nAtBeamlet++)
	{
		for (int nAtField = 0; nAtField < m_nFields; nAtField++)
		{
			int nAtBeamletAlt = (nAtBeamlet + 1) / 2;
			int nAtBeamletSign = nAtBeamlet % 2 ? 1 : -1;
			GetBeamAt(nAtField * nBeamletCount + nBeamletCount / 2 + nAtBeamletAlt * nAtBeamletSign)
				->SetWeight(vWeights[nAtElem]);
			nAtElem++;
		}
	}
#endif
	m_bRecomputeTotalDose = TRUE;
}

// DVH accessors
const CMatrixNxM<> *CPlan::GetTargetDVH(CMesh *pStructure)
{
	CMatrixNxM<> *pmTargetDVH;
	if (m_mapTargetDVHs.Lookup(pStructure->GetName(), (void *&) pmTargetDVH))
	{
		return pmTargetDVH;
	}

	return NULL;
}

void CPlan::SetTargetDVH(CMesh *pStructure, CMatrixNxM<> *pmTargetDVH)
{
	CMatrixNxM<> *pmOldTargetDVH = (CMatrixNxM<> *)GetTargetDVH(pStructure);
	delete pmOldTargetDVH;
	if (NULL != pmTargetDVH)
	{
		m_mapTargetDVHs.SetAt(pStructure->GetName(), (void *) pmTargetDVH);
	}
	else
	{
		m_mapTargetDVHs.RemoveKey(pStructure->GetName());
	}
}


