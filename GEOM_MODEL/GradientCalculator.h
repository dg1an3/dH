// GradientCalculator.h: interface for the CGradientCalculator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRADIENTCALCULATOR_H__7ECF4AD6_5BDF_49AF_9944_195F47EEDF2A__INCLUDED_)
#define AFX_GRADIENTCALCULATOR_H__7ECF4AD6_5BDF_49AF_9944_195F47EEDF2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "INCLUDE\ModelObject.h"

#include <Volumep.h>

template<typename VOXEL_TYPE>
class CGradientCalculator : public CModelObject  
{
public:
	CGradientCalculator();
	virtual ~CGradientCalculator();

	void CalculateGradient();

public:
	CVolume<VOXEL_TYPE> * m_pVolumeIn;
	CVolume<VOXEL_TYPE> * m_pVolumeDx;
	CVolume<VOXEL_TYPE> * m_pVolumeDy;
};


#endif // !defined(AFX_GRADIENTCALCULATOR_H__7ECF4AD6_5BDF_49AF_9944_195F47EEDF2A__INCLUDED_)
