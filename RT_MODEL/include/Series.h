// Series.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIES_H__731877C4_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
#define AFX_SERIES_H__731877C4_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MatrixD.h>

#include <Volumep.h>

#include <Structure.h>

///////////////////////////////////////////////////////////////////////////////
// class CSeries
// 
// <description>
///////////////////////////////////////////////////////////////////////////////
class CSeries : public CModelObject
{
public:
	CSeries();          
	virtual ~CSeries();

	DECLARE_SERIAL(CSeries)
	virtual void Serialize(CArchive& ar);

	// Structures for the series
	int GetStructureCount() const;
	CStructure *GetStructureAt(int nAt);

	// miscellaneous operations
	CStructure * CreateSphereStructure(const CString& strName);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	void AddStructure(CStructure *pStruct);
	CStructure * GetStructureFromName(const CString& strName);

	// Volume data for the series
	CVolume<REAL> *m_pDens;

	CTypedPtrArray<CObArray, CStructure*> m_arrStructures;
};


//{{AFX_INSERT_LOCATION}	// CSeries}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIES_H__731877C4_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
