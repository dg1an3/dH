// Series.cpp : implementation file
//

#include "stdafx.h"

#include <UtilMacros.h>

// #include <MatrixBase.inl>

#include <Series.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSeries

IMPLEMENT_SERIAL(CSeries, CModelObject, 1)

///////////////////////////////////////////////////////////////////////////////
// CSeries::CSeries
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CSeries::CSeries()
	: m_pDens(new CVolume<REAL>())
{
}	// CSeries::CSeries


///////////////////////////////////////////////////////////////////////////////
// CSeries::~CSeries
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CSeries::~CSeries()
{
	// delete the structures
	for (int nAt = 0; nAt < m_arrStructures.GetSize(); nAt++)
	{
		delete m_arrStructures[nAt];
	}

	delete m_pDens;

}	// CSeries::~CSeries


///////////////////////////////////////////////////////////////////////////////
// CSeries::GetStructureCount
// 
// Structures for the series
///////////////////////////////////////////////////////////////////////////////
int CSeries::GetStructureCount() const
{
	return m_arrStructures.GetSize();

}	// CSeries::GetStructureCount


///////////////////////////////////////////////////////////////////////////////
// CSeries::GetStructureAt
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
CStructure *CSeries::GetStructureAt(int nAt)
{
	return (CStructure *) m_arrStructures.GetAt(nAt);

}	// CSeries::GetStructureAt


/////////////////////////////////////////////////////////////////////////////
// CSeries diagnostics

#ifdef _DEBUG
void CSeries::AssertValid() const
{
	CModelObject::AssertValid();

	// m_arrStructures.AssertValid();
}

void CSeries::Dump(CDumpContext& dc) const
{
	CModelObject::Dump(dc);

	PUSH_DUMP_DEPTH(dc);

	// dump the plan's beams
	// m_arrStructures.Dump(dc);

	POP_DUMP_DEPTH(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSeries serialization

///////////////////////////////////////////////////////////////////////////////
// CSeries::Serialize
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
void CSeries::Serialize(CArchive& ar)
{
	CModelObject::Serialize(ar);

	m_pDens->Serialize(ar);
	m_arrStructures.Serialize(ar);

	/* DWORD nStructureCount = GetStructureCount();
	SERIALIZE_VALUE(ar, nStructureCount);

	if (ar.IsLoading())
	{
		// delete any existing structures
		m_arrStructures.RemoveAll();

		for (int nAt = 0; nAt < nStructureCount; nAt++)
		{
			CStructure *pStruct = new CStructure;
			AddStructure(pStruct);
		}
	}

	for (int nAt = 0; nAt < nStructureCount; nAt++)
	{
		GetStructureAt(nAt)->Serialize(ar);
	}
*/
}	// CSeries::Serialize

/////////////////////////////////////////////////////////////////////////////
// CSeries commands

/*
BOOL CSeries::OnNewDocument()
{
/*	if (!CDocument::OnNewDocument())
		return FALSE; * /

	// delete any existing structures
	m_arrStructures.RemoveAll();

	return TRUE;
}
*/
/*
CMesh *CSeries::CreateSphereStructure(const CString &strName)
{
	return NULL;
}
*/

CStructure * CSeries::GetStructureFromName(const CString &strName)
{
	for (int nAt = 0; nAt < GetStructureCount(); nAt++)
	{
		if (GetStructureAt(nAt)->GetName() == strName)
		{
			return GetStructureAt(nAt);
		}
	}

	return NULL;
}

void CSeries::AddStructure(CStructure *pStruct)
{
	pStruct->m_pSeries = this;
	m_arrStructures.Add(pStruct);
}
