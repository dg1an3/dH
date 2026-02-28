// Pyramid.cpp: implementation of the CPyramid class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Pyramid.h"

static CVectorD<3> CalcPixelSpacing(const CMatrixD<4>& mBasis)
{
	CMatrixD<4> mUnit;
	mUnit[0][3] = 1.0;
	mUnit[1][3] = 1.0;
	mUnit[2][3] = 1.0;

	CMatrixD<4> mXformUnit = mBasis * mUnit;

	CVectorD<3> vSpacing;
	vSpacing[0] = (FromHG<3, REAL>(mXformUnit[0]) - FromHG<3, REAL>(mXformUnit[3])).GetLength();
	vSpacing[1] = (FromHG<3, REAL>(mXformUnit[1]) - FromHG<3, REAL>(mXformUnit[3])).GetLength();
	vSpacing[2] = (FromHG<3, REAL>(mXformUnit[2]) - FromHG<3, REAL>(mXformUnit[3])).GetLength();

	return vSpacing;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPyramid::CPyramid(CVolume<VOXEL_REAL> *pVolume)
: m_pVolume(pVolume)
{
	m_arrLevels.Add(m_pVolume);

	m_kernel.SetDimensions(5, 5, 1);
	CalcBinomialFilter(&m_kernel);
}

CPyramid::~CPyramid()
{
	if (m_arrLevels[0] != m_pVolume)
	{
		delete m_arrLevels[0];
	}

	for (int nAt = 1; nAt < m_arrLevels.GetSize(); nAt++)
	{
		delete m_arrLevels[nAt];
	}
}

CVolume<VOXEL_REAL> * CPyramid::GetLevel(int nLevel)
{
	if (m_arrLevels.GetSize() <= nLevel)
	{
		CVolume<VOXEL_REAL> *pPrev = GetLevel(nLevel-1);

		CVolume<VOXEL_REAL> convRegion;
		convRegion.ConformTo(pPrev);
		Convolve(pPrev, &m_kernel, &convRegion);

		CVolume<VOXEL_REAL> *pNewRegion = new CVolume<VOXEL_REAL>();
		Decimate(&convRegion, pNewRegion);
		m_arrLevels.Add(pNewRegion);
	}

	return m_arrLevels[nLevel];
}

int CPyramid::SetLevelBasis(const CMatrixD<4> &mBasis)
{
	// compute G0 pixel spacing
	CVectorD<3> vSpacing0 = CalcPixelSpacing(m_arrLevels[0]->GetBasis());

/*	CMatrixD<4> mShrink;
	mShrink[0][0] = 0.5;
	mShrink[1][1] = 0.5;
	mShrink[2][2] = 0.5; */

	CMatrixD<4> mNewBasis0 = mBasis;

	// compute desired pixel spacing
	CVectorD<3> vSpacingN = CalcPixelSpacing(mNewBasis0);

	// count levels
	int nLevel = 0;

	// TODO: account for basis rotations (to handle anisotropic bases)
	while (vSpacing0[0] < vSpacingN[0]
		|| vSpacing0[1] < vSpacingN[1]
		|| vSpacing0[2] < vSpacingN[2])
	{
		mNewBasis0[0] *= 0.5;
		mNewBasis0[1] *= 0.5;
		mNewBasis0[2] *= 0.5;
		vSpacingN = CalcPixelSpacing(mNewBasis0);
		nLevel++;
	}

	// resample G0
	CVolume<VOXEL_REAL> * pG0 = new CVolume<VOXEL_REAL>();
	pG0->ConformTo(m_pVolume);
	pG0->SetBasis(mNewBasis0);
	::Resample(m_pVolume, pG0, TRUE);

	// replace G0 level
	if (m_arrLevels[0] != m_pVolume)
	{
		delete m_arrLevels[0];
	}
	m_arrLevels[0] = pG0;

	return nLevel;
}
