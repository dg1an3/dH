// Plan.cpp : implementation of the CPlan class
//

#include "stdafx.h"
#include ".\include\plan.h"

#include <UtilMacros.h>

#include <EnergyDepKernel.h>

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
		m_bRecomputeTotalDose(TRUE),
		m_bCalcMassDensity(TRUE)
{
	m_pKernel = new CEnergyDepKernel(15.0);

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
		CString strName;
		CHistogram *pHistogram = NULL;
		m_mapHistograms.GetNextAssoc(pos, strName, pHistogram); 

		delete pHistogram;
	}

	// delete the kernel
	delete m_pKernel;

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

	m_bCalcMassDensity = TRUE;

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
CVolume<VOXEL_REAL> *CPlan::GetDoseMatrix()
{
	if (TRUE) // m_bRecomputeTotalDose)
	{
		// total the dose for all beams
		if (GetBeamCount() > 0)
		{
/*			CVolume<VOXEL_REAL> *pBeamDose = GetBeamAt(0)->GetDoseMatrix();
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
				static CVolume<VOXEL_REAL> beamDoseRot;
				beamDoseRot.ConformTo(&m_dose);

				Resample(GetBeamAt(nAt)->GetDoseMatrix(), &beamDoseRot, TRUE);

				// add this beam's dose matrix to the total
				m_dose.Accumulate(&beamDoseRot, GetBeamAt(nAt)->GetWeight());
			}

			m_dose.VoxelsChanged();
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
CHistogram *CPlan::GetHistogram(CStructure *pStructure, bool bCreate)
{
	CHistogram *pHisto = NULL;
	if (!m_mapHistograms.Lookup(pStructure->GetName(), pHisto))
	{
		if (bCreate)
		{
			pHisto = new CHistogram();

			pHisto->SetBinning((REAL) 0.0, (REAL) 0.02, GBINS_BUFFER);
			pHisto->SetVolume(GetDoseMatrix());

			// resample region, if needed
			CVolume<VOXEL_REAL> *pResampRegion = pStructure->GetConformRegion(GetDoseMatrix());
			pHisto->SetRegion(pResampRegion);

			// add to map
			m_mapHistograms[pStructure->GetName()] = pHisto;
		}
	}

	return pHisto;

}	// CPlan::GetHistogram


///////////////////////////////////////////////////////////////////////////////
// CPlan::RemoveHistogram
// 
// histogram accessor
///////////////////////////////////////////////////////////////////////////////
void CPlan::RemoveHistogram(CStructure *pStructure)
{
	CHistogram *pHisto = NULL;
	if (m_mapHistograms.Lookup(pStructure->GetName(), pHisto))
	{
		m_mapHistograms.RemoveKey(pStructure->GetName());
		delete pHisto;
	}

}	// CPlan::RemoveHistogram


/////////////////////////////////////////////////////////////////////////////
// CPlan serialization

#define PLAN_SCHEMA 5
	// Schema 1: initial plan schema
	// Schema 2: + target DVH curves
	// Schema 3: + number of fields
	// Schema 4: + target DVH count
	// Schema 5: + DVHs

IMPLEMENT_SERIAL(CPlan, CModelObject, VERSIONABLE_SCHEMA | PLAN_SCHEMA)

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

	// DEPRACATED flag to serialize dose matrix
	BOOL bDoseValid = FALSE; 
	SERIALIZE_VALUE(ar, bDoseValid);

	// serialize the dose matrix
	m_dose.Serialize(ar);

	// trigger recalculation of total dose
	m_bRecomputeTotalDose = TRUE;

	// for the schema 2, serialize target DVHs
	if (nSchema >= 2)
	{
		if (ar.IsLoading())
		{
			int nCount = 0;
			if (nSchema >= 4)
			{
				ar >> nCount;
			}

			for (int nAt = 0; nAt < nCount; nAt++)
			{
				CString strStructureName;
				ar >> strStructureName;

				CMatrixNxM<> mTargetDVH;
				ar >> mTargetDVH;
			}
		}
		else 
		{
			if (nSchema >= 4)
			{
				// DEPRECATED
				// ar << m_mapTargetDVHs.GetCount();
				ar << 0;
			}

			// DEPRECATED
			// delete the target DVHs
			// POSITION pos = m_mapTargetDVHs.GetStartPosition();
			// while (NULL != pos)
			// {
			//	m_mapTargetDVHs.GetNextAssoc(pos, strStructureName, 
			//		(void *&)pmTargetDVH);

			//	ar << strStructureName;
			//	ar << (*pmTargetDVH);
			// }
		}
	}

	if (nSchema >= 3)
	{
		int m_nFields = 0;
		SERIALIZE_VALUE(ar, m_nFields);
	}

	if (nSchema >= 5)
	{
		SERIALIZE_VALUE(ar, m_pSeries);
		ASSERT(m_pSeries != NULL);

		m_mapHistograms.Serialize(ar);

		if (ar.IsLoading())
		{
			// set up regions and volumes for histograms
			//		because serialization doesn't restore this
			POSITION pos = m_mapHistograms.GetStartPosition();
			while (NULL != pos)
			{
				CString strName;
				CHistogram *pHistogram = NULL;
				m_mapHistograms.GetNextAssoc(pos, strName, pHistogram); 

				// set volume
				pHistogram->SetVolume(GetDoseMatrix());

				// set region
				CStructure *pStruct = GetSeries()->GetStructureFromName(strName);
				CVolume<VOXEL_REAL> *pConformRegion = pStruct->GetConformRegion(GetDoseMatrix());
				pHistogram->SetRegion(pConformRegion);
			}
		}
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
// CPlan::GetMassDensity
// 
// used to format the mass density array, conformant to dose matrix
///////////////////////////////////////////////////////////////////////////////
CVolume<VOXEL_REAL> * CPlan::GetMassDensity()
{
	if (m_bCalcMassDensity)
	{
		// fix mass density
		m_massDensity.ConformTo(m_pSeries->m_pDens);

		// lookup values
		VOXEL_REAL *pCTVoxels = &m_pSeries->m_pDens->GetVoxels()[0][0][0];
		VOXEL_REAL *pMDVoxels = &m_massDensity.GetVoxels()[0][0][0];
		int nVoxels = m_massDensity.GetWidth() * m_massDensity.GetHeight();
		for (int nAtVoxel = 0; nAtVoxel < nVoxels; nAtVoxel++)
		{
			if (pCTVoxels[nAtVoxel] < 0.0)
			{
				pMDVoxels[nAtVoxel] = VOXEL_REAL(1.0 / 3.0 + (pCTVoxels[nAtVoxel] - -1024.0) * 2.0 / 3.0 / 1024.0);
			}
			else if (pCTVoxels[nAtVoxel] < 1024.0)
			{
				pMDVoxels[nAtVoxel] = VOXEL_REAL(1.0 + pCTVoxels[nAtVoxel] * 0.5 / 1024.0);
			}
			else
			{
				pMDVoxels[nAtVoxel] = 1.5;
			}
		}

		m_bCalcMassDensity = FALSE;
	}

	return &m_massDensity;

}	// CPlan::GetMassDensity
