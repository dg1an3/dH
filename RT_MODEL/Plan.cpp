// Plan.cpp : implementation of the CPlan class
//

#include "stdafx.h"

#include <UtilMacros.h>

#include <MatrixBase.inl>

#include <Plan.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CPlan::CPlan
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CPlan::CPlan()
	: m_pSeries(NULL),
		m_bRecomputeTotalDose(TRUE)
{
}	// CPlan::CPlan


///////////////////////////////////////////////////////////////////////////////
// CPlan::~CPlan
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CPlan::~CPlan()
{
	// delete the beams
	for (int nAt = 0; nAt < m_arrBeams.GetSize(); nAt++)
	{
		delete m_arrBeams[nAt];
	}

	// delete the histograms
	POSITION pos = m_mapHistograms.GetStartPosition();
	while (NULL != pos)
	{
		CStructure *pStructure = NULL;
		CHistogram *pHistogram = NULL;
		m_mapHistograms.GetNextAssoc(pos, 
			(void*&) pStructure, (void*&) pHistogram); 

		delete pHistogram;
	}

	// delete the target DVHs
	pos = m_mapTargetDVHs.GetStartPosition();
	while (NULL != pos)
	{
		CString strStructureName;
		CMatrixNxM<> *pmTargetDVH = NULL;
		m_mapTargetDVHs.GetNextAssoc(pos, strStructureName, 
			(void *&)pmTargetDVH);

		delete pmTargetDVH;
	}

}	// CPlan::~CPlan


///////////////////////////////////////////////////////////////////////////////
// CPlan::GetSeries
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CSeries * CPlan::GetSeries()
{
	return m_pSeries;

}	// CPlan::GetSeries


///////////////////////////////////////////////////////////////////////////////
// CPlan::SetSeries
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlan::SetSeries(CSeries *pSeries)
{
	m_pSeries = pSeries;

	m_dose.ConformTo(m_pSeries->m_pDens);

}	// CPlan::SetSeries


///////////////////////////////////////////////////////////////////////////////
// CPlan::GetBeamCount
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int CPlan::GetBeamCount() const
{
	return m_arrBeams.GetSize();

}	// CPlan::GetBeamCount


///////////////////////////////////////////////////////////////////////////////
// CPlan::GetTotalBeamletCount
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int CPlan::GetTotalBeamletCount(int nLevel)
{
	int nBeamlets = 0;

	for (int nAtBeam = 0; nAtBeam < GetBeamCount(); nAtBeam++)
	{
		nBeamlets += GetBeamAt(nAtBeam)->GetBeamletCount(nLevel);
	}

	return nBeamlets;

}	// CPlan::GetTotalBeamletCount


///////////////////////////////////////////////////////////////////////////////
// CPlan::GetBeamAt
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CBeam * CPlan::GetBeamAt(int nAt)
{
	return m_arrBeams.GetAt(nAt);

}	// CPlan::GetBeamAt


///////////////////////////////////////////////////////////////////////////////
// CPlan::AddBeam
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
int CPlan::AddBeam(CBeam *pBeam)
{
	int nIndex = m_arrBeams.Add(pBeam);
	pBeam->GetChangeEvent().AddObserver(this, (ListenerFunction) OnBeamChange);

	m_bRecomputeTotalDose = TRUE;

	// a change has occurred, so fire
	GetChangeEvent().Fire();

	return nIndex;

}	// CPlan::AddBeam



///////////////////////////////////////////////////////////////////////////////
// CPlan::GetDoseMatrix
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CVolume<REAL> *CPlan::GetDoseMatrix()
{
	if (TRUE) // m_bRecomputeTotalDose)
	{
		// total the dose for all beams
		if (GetBeamCount() > 0)
		{
/*			CVolume<REAL> *pBeamDose = GetBeamAt(0)->GetDoseMatrix();
			// TODO: move this to SetSeries
			m_dose.SetDimensions(pBeamDose->GetWidth(), 
				pBeamDose->GetHeight(),
				pBeamDose->GetDepth());

			CMatrixD<4> mBasis;
			mBasis[3][0] = -(m_dose.GetWidth() - 1) / 2;
			mBasis[3][1] = -(m_dose.GetHeight() - 1) / 2;
			m_dose.SetBasis(mBasis); */

			// clear the dose matrix
			m_dose.ClearVoxels();

			for (int nAt = 0; nAt < GetBeamCount(); nAt++)
			{
				static CVolume<REAL> beamDoseRot;
				beamDoseRot.ConformTo(&m_dose);

				Resample(GetBeamAt(nAt)->GetDoseMatrix(), &beamDoseRot);

				// add this beam's dose matrix to the total
				m_dose.Accumulate(&beamDoseRot, GetBeamAt(nAt)->GetWeight());
			}
		}

#ifdef NORMALIZE_DOSE
		// compute voxel count
		int nVoxelCount = m_dose.GetHeight() * m_dose.GetWidth() 
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
#endif

		m_bRecomputeTotalDose = FALSE;
	}

	return &m_dose;

}	// CPlan::GetDoseMatrix


///////////////////////////////////////////////////////////////////////////////
// CPlan::GetHistogram
// 
// histogram accessor
///////////////////////////////////////////////////////////////////////////////
CHistogram *CPlan::GetHistogram(CStructure *pStructure)
{
	CHistogram *pHisto = NULL;
	if (!m_mapHistograms.Lookup((void *) pStructure, (void*&) pHisto))
	{
		pHisto = new CHistogram();
		pHisto->SetVolume(GetDoseMatrix());
		pHisto->SetRegion(pStructure->GetRegion());
		m_mapHistograms[pStructure] = pHisto;
	}

	return pHisto;

}	// CPlan::GetHistogram

/////////////////////////////////////////////////////////////////////////////
// CPlan serialization

#define PLAN_SCHEMA 4
	// Schema 1: initial plan schema
	// Schema 2: + target DVH curves
	// Schema 3: + number of fields
	// Schema 4: + target DVH count

IMPLEMENT_SERIAL(CPlan, CDocument, VERSIONABLE_SCHEMA | PLAN_SCHEMA)


///////////////////////////////////////////////////////////////////////////////
// CPlan::Serialize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlan::Serialize(CArchive& ar)
{
	// schema for the plan object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : PLAN_SCHEMA;

	CModelObject::Serialize(ar);

	if (ar.IsLoading())
	{
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

	// never serialize dose matrix
	BOOL m_bDoseValid = FALSE; // m_arrDose[0].GetWidth() > 0;

	// serialize the dose matrix valid flag
	SERIALIZE_VALUE(ar, m_bDoseValid);

	// empty the dose matrix if it is not valid
	// if (ar.IsStoring() && !m_bDoseValid)
	{
	//	m_dose.SetDimensions(0, 0, 0);
	}

	// serialize the dose matrix
	m_dose.Serialize(ar);

	// trigger recalculation of total dose
	m_bRecomputeTotalDose = TRUE;

	// for the schema 2, serialize target DVHs
	if (nSchema >= 2)
	{
		CString strStructureName;
		CMatrixNxM<> *pmTargetDVH = NULL;
		if (ar.IsLoading())
		{
			int nCount = 1;
			if (nSchema >= 4)
			{
				ar >> nCount;
			}

			for (int nAt = 0; nAt < nCount; nAt++)
			{
				ar >> strStructureName;
				pmTargetDVH = new CMatrixNxM<>;
				ar >> (*pmTargetDVH);
				m_mapTargetDVHs.SetAt(strStructureName, (void *) pmTargetDVH);
			}
		}
		else 
		{
			if (nSchema >= 4)
			{
				ar << m_mapTargetDVHs.GetCount();
			}

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
	}

	if (nSchema >= 3)
	{
		SERIALIZE_VALUE(ar, m_nFields);
	}

}	// CPlan::Serialize



/////////////////////////////////////////////////////////////////////////////
// CPlan diagnostics

#ifdef _DEBUG
void CPlan::AssertValid() const
{
	CModelObject::AssertValid();

	// make sure the plan's beams are valid
	// m_arrBeams.AssertValid();
}

void CPlan::Dump(CDumpContext& dc) const
{
	CModelObject::Dump(dc);

	int nOrigDepth = dc.GetDepth();
	dc.SetDepth(nOrigDepth + 1);

	// dump the plan's beams
	// m_arrBeams.Dump(dc);

	dc.SetDepth(nOrigDepth);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////////////
// CPlan::OnBeamChange
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CPlan::OnBeamChange(CObservableEvent *, void *)
{
	m_bRecomputeTotalDose = TRUE;
	m_dose.GetChangeEvent().Fire();

}	// CPlan::OnBeamChange


///////////////////////////////////////////////////////////////////////////////
// CPlan::GetBeamWeights
// 
// the beam weights, as a vector
///////////////////////////////////////////////////////////////////////////////
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

}	// CPlan::GetBeamWeights


///////////////////////////////////////////////////////////////////////////////
// CPlan::SetBeamWeights
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
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

}	// CPlan::SetBeamWeights


///////////////////////////////////////////////////////////////////////////////
// CPlan::GetTargetDVH
// 
// DVH accessors
///////////////////////////////////////////////////////////////////////////////
const CMatrixNxM<> *CPlan::GetTargetDVH(CMesh *pStructure)
{
	CMatrixNxM<> *pmTargetDVH;
	if (m_mapTargetDVHs.Lookup(pStructure->GetName(), (void *&) pmTargetDVH))
	{
		return pmTargetDVH;
	}

	return NULL;

}	// CPlan::GetTargetDVH


///////////////////////////////////////////////////////////////////////////////
// CPlan::SetTargetDVH
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
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

}	// CPlan::SetTargetDVH


