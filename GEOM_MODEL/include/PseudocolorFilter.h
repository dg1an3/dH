// PseudocolorFilter.h: interface for the CPseudocolorFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSEUDOCOLORFILTER_H__45FAA2CF_1E6D_4112_B457_C2726FFCE3ED__INCLUDED_)
#define AFX_PSEUDOCOLORFILTER_H__45FAA2CF_1E6D_4112_B457_C2726FFCE3ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Dib.h"

class CPseudocolorFilter : public CObject  
{
public:
	CPseudocolorFilter();
	virtual ~CPseudocolorFilter();

	// input CVolume
	CDib *GetSource();
	void SetSource(CDib *pSource);

	// output CDib
	CDib *GetDest();
	void SetDest(CDib *pDest);

	// typedef for the lookup table
	typedef CArray<COLORREF, COLORREF> CColorLUT;

	// filter parameters: color lookup table
	const CColorLUT& GetLUT();
	void SetLUT(const CColorLUT& arrLUT);
};

#endif // !defined(AFX_PSEUDOCOLORFILTER_H__45FAA2CF_1E6D_4112_B457_C2726FFCE3ED__INCLUDED_)
