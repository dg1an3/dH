// Series.cpp : implementation file
//

#include "stdafx.h"

#include <UtilMacros.h>

#include <MatrixBase.inl>

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
	// delete the structures
	for (int nAt = 0; nAt < m_arrStructures.GetSize(); nAt++)
	{
		delete m_arrStructures[nAt];
	}
}

// Structures for the series
int CSeries::GetStructureCount() const
{
	return m_arrStructures.GetSize();
}

CStructure *CSeries::GetStructureAt(int nAt)
{
	return (CStructure *) m_arrStructures.GetAt(nAt);
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

/*
class CSurface : public CMesh
{
public:
	CSurface() : CMesh() { }

	DECLARE_SERIAL(CSurface)

	void Serialize(CArchive& ar);
};


#define MESH_SCHEMA 4
// IMPLEMENT_SERIAL(CSurface, CMesh, VERSIONABLE_SCHEMA | MESH_SCHEMA)

void CSurface::Serialize(CArchive& ar)
{
//	if (ar.IsStoring())
	{
		CMesh::Serialize(ar);
		return;
	}

	// store the schema for the beam object
	UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : MESH_SCHEMA;

	// serialize the surface name
	// name.Serialize(ar);
	if (ar.IsLoading())
	{
		CString strName;
		ar >> strName;
		SetName(strName);
	}
	else
	{
		ar << GetName();
	}

	// serialize the contours
	// m_arrContours.Serialize(ar);
	if (ar.IsLoading())
	{
		// delete any existing structures
		// m_arrContours.RemoveAll();
		m_arrContours3D.erase(m_arrContours3D.begin(),
			m_arrContours3D.end());

		DWORD nCount = ar.ReadCount();
		for (int nAt = 0; nAt < nCount; nAt++)
		{
			CPolygon *pPolygon = 
				(CPolygon *) ar.ReadObject(RUNTIME_CLASS(CPolygon));
			ASSERT(NULL != pPolygon);
			// m_arrContours.Add(pPolygon);

			// create the Polygon3D object
			CComObject<CPolygon3D> *pPoly3D = NULL;
			CComObject<CPolygon3D>::CreateInstance(&pPoly3D);
			pPoly3D->AddRef();

			pPoly3D->m_mVertex.Reshape(pPolygon->m_mVertex.GetCols(), 2);
			pPoly3D->m_mVertex = pPolygon->m_mVertex;

			// and add it to the array
			m_arrContours3D.push_back(pPoly3D);
		}
	}
	else
	{
		ar.WriteCount(GetContourCount());
		for (int nAt = 0; nAt < GetContourCount(); nAt++)
		{
			get_Contour(nAt)->Serialize(ar);
			// ar.WriteObject(&GetContour(nAt));
		}
	}

	// serialize the reference distance
	m_arrRefDist.Serialize(ar);

	// serialize vertices
	CArray<int, int&> arrVertIndex;
	if (!ar.IsLoading())
	{
		arrVertIndex.SetSize(m_arrTriIndex.size() * 3);
		memcpy(arrVertIndex.GetData(), &m_arrTriIndex[0][0], 
			m_arrTriIndex.size() * 3 * sizeof(int));
	}
	arrVertIndex.Serialize(ar);
	if (ar.IsLoading())
	{
		m_arrTriIndex.resize(arrVertIndex.GetSize() / 3);
		memcpy(&m_arrTriIndex[0][0], arrVertIndex.GetData(), 
			m_arrTriIndex.size() * 3 * sizeof(int));
	}

	////////////////////////////////////////

	CArray< CPackedVectorD<3>, CPackedVectorD<3>& > arrVertex;
	if (!ar.IsLoading())
	{
		arrVertex.SetSize(m_mVertex.GetCols());
		memcpy(arrVertex.GetData(), &m_mVertex[0][0],
			m_mVertex.GetCols() * 3 * sizeof(REAL));
	}
	arrVertex.Serialize(ar);
	if (ar.IsLoading())
	{
		m_mVertex.Reshape(arrVertex.GetSize(), 3);
		memcpy(&m_mVertex[0][0], arrVertex.GetData(), 
			m_mVertex.GetCols() * 3 * sizeof(REAL));
	}

	/////////////////////////////////////////////

	CArray< CPackedVectorD<3>, CPackedVectorD<3>& > arrNormal;
	if (!ar.IsLoading())
	{
		arrNormal.SetSize(m_mVertex.GetCols());
		memcpy(arrNormal.GetData(), &m_mNormal[0][0],
			m_mNormal.GetCols() * 3 * sizeof(REAL));
	}
	arrNormal.Serialize(ar);
	if (ar.IsLoading())
	{
		m_mNormal.Reshape(arrNormal.GetSize(), 3);
		memcpy(&m_mNormal[0][0], arrNormal.GetData(), 
			m_mNormal.GetCols() * 3 * sizeof(REAL));
	}

	// if schema >= 4
	if (nSchema >= 4)
	{
		// serialize the region also
		if (m_pRegion == NULL)
		{
			m_pRegion = new CVolume<int>();
		}
		m_pRegion->Serialize(ar);
	}
}
*/

void CSeries::Serialize(CArchive& ar)
{
	CDocument::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_volumeTransform;
	}
	else
	{
		ar << m_volumeTransform;
	}
	volume.Serialize(ar);

	DWORD nStructureCount = GetStructureCount();
	SERIALIZE_VALUE(ar, nStructureCount);

	if (ar.IsLoading())
	{
		// delete any existing structures
		m_arrStructures.RemoveAll();

		for (int nAt = 0; nAt < nStructureCount; nAt++)
		{
			CMesh *pMesh = new CMesh;
			m_arrStructures.Add(pMesh);
		}
	}

	for (int nAt = 0; nAt < nStructureCount; nAt++)
	{
		GetStructureAt(nAt)->Serialize(ar);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSeries commands

BOOL CSeries::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// delete any existing structures
	m_arrStructures.RemoveAll();

	return TRUE;
}

/*
CMesh *CSeries::CreateSphereStructure(const CString &strName)
{
	return NULL;
}
*/