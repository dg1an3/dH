// PenBeamEditDoc.h : interface of the CPenBeamEditDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PENBEAMEDITDOC_H__34348F46_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
#define AFX_PENBEAMEDITDOC_H__34348F46_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Volumep.h>

#include "Histogram.h"

/* class CImage : public CObject
{
public:
	CImage();
	virtual ~CImage();

	BOOL Read(LPCTSTR lpszPathName);

	int m_nSize;
	CArray<double, double> m_arrIntensity;

	double m_angle;
};
*/

class CPenBeamEditDoc : public CDocument
{
protected: // create from serialization only
	CPenBeamEditDoc();
	DECLARE_DYNCREATE(CPenBeamEditDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPenBeamEditDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	CVolume<double> m_density;

	int m_nAngles;
	CObArray m_arrPencilBeams;

	CArray<double, double> m_arrWeights;
	CVolume<double> m_totalDose;

	CVolume<double> m_region;
	CHistogram m_histogram;

//	CImage m_region2;
//	CHistogram m_histogram2;

	virtual ~CPenBeamEditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPenBeamEditDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PENBEAMEDITDOC_H__34348F46_5130_11D5_ABBE_00B0D0AB90D6__INCLUDED_)
