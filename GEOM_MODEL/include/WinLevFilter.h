// WinLevFilter.h: interface for the CWinLevFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINLEVFILTER_H__3CAD6B63_76F7_41DE_8DD8_E1FE161D784A__INCLUDED_)
#define AFX_WINLEVFILTER_H__3CAD6B63_76F7_41DE_8DD8_E1FE161D784A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Dib.h"
#include "Volumep.h"

template<class VOXEL_TYPE>
class CWinLevFilter : public CObject  
{
public:
	CWinLevFilter();
	virtual ~CWinLevFilter();

	// input CVolume
	CVolume<VOXEL_TYPE> *GetSource();
	void SetSource(CVolume<VOXEL_TYPE> *pSource);

	// output CDib
	CDib *GetDest();
	void SetDest(CDib *pDest);

	// filter parameters: window
	int GetWindow();
	void SetWindow(int nWindow);

	// filter parameters: level
	int GetLevel();
	void SetLevel(int nLevel);
};

#endif // !defined(AFX_WINLEVFILTER_H__3CAD6B63_76F7_41DE_8DD8_E1FE161D784A__INCLUDED_)
