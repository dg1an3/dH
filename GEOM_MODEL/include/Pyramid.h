// Pyramid.h: interface for the CPyramid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PYRAMID_H__D29DF52C_B3D2_46E9_B9CD_2D92D31D8CDB__INCLUDED_)
#define AFX_PYRAMID_H__D29DF52C_B3D2_46E9_B9CD_2D92D31D8CDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Volumep.h>

class CPyramid  
{
public:
	CPyramid(CVolume<VOXEL_REAL> *pVolume);
	virtual ~CPyramid();

	CVolume<VOXEL_REAL> * GetLevel(int nLevel);

	int SetLevelBasis(const CMatrixD<4>& mBasis);

private:
	CVolume<VOXEL_REAL> * m_pVolume;

	CVolume<VOXEL_REAL> m_kernel;

	CTypedPtrArray<CPtrArray, CVolume<VOXEL_REAL> * > m_arrLevels;
};

#endif // !defined(AFX_PYRAMID_H__D29DF52C_B3D2_46E9_B9CD_2D92D31D8CDB__INCLUDED_)
