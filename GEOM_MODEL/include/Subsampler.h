// Subsampler.h: interface for the CSubsampler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SUBSAMPLER_H__BFD2DF05_E331_4331_91DF_A83C2EED5420__INCLUDED_)
#define AFX_SUBSAMPLER_H__BFD2DF05_E331_4331_91DF_A83C2EED5420__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "INCLUDE\ModelObject.h"
#include <Volumep.h>

template<typename VOXEL_TYPE>
class CSubsampler : public CModelObject  
{
public:
	CSubsampler();
	virtual ~CSubsampler();

	void Subsample();

public:
	CVolume<VOXEL_TYPE> * m_pVolumeOut;
	CVolume<VOXEL_TYPE> * m_pVolumeIn;
};

#endif // !defined(AFX_SUBSAMPLER_H__BFD2DF05_E331_4331_91DF_A83C2EED5420__INCLUDED_)
