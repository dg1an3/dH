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

// #include <Mesh.h>
#include "Structure.h"

/////////////////////////////////////////////////////////////////////////////
// CSeries document

class CSeries : public CDocument
{
public:
	CSeries();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSeries)

// Attributes
public:
	// Transform to position the volume in 3-space
	CMatrixD<4> m_volumeTransform;

	// Volume data for the series
	CVolume< short > volume;

	// Structures for the series
	int GetStructureCount() const;
	CStructure *GetStructureAt(int nAt);

	CObArray m_arrStructures;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSeries)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	//virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	CStructure * CreateSphereStructure(const CString& strName);
	CString GetFileRoot();
	CString GetFileName();
	BOOL OnNewDocument();
	virtual ~CSeries();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSeries)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIES_H__731877C4_EE46_11D4_9E36_00B0D0609AB0__INCLUDED_)
