// GradientCalculator.cpp: implementation of the CGradientCalculator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GradientCalculator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template<typename VOXEL_TYPE>
CGradientCalculator<VOXEL_TYPE>::CGradientCalculator()
{

}

template<typename VOXEL_TYPE>
CGradientCalculator<VOXEL_TYPE>::~CGradientCalculator()
{

}



template<typename VOXEL_TYPE>
void CGradientCalculator<VOXEL_TYPE>::CalculateGradient()
{
	m_pVolumeDx->SetDimensions(
		m_pVolumeIn->GetWidth(), 
		m_pVolumeIn->GetHeight(),
		m_pVolumeIn->GetDepth()
	);

	m_pVolumeDy->SetDimensions(
		m_pVolumeIn->GetWidth(), 
		m_pVolumeIn->GetHeight(),
		m_pVolumeIn->GetDepth()
	);

	const VOXEL_TYPE ***pVoxelsIn = m_pVolumeIn->GetVoxels();
	VOXEL_TYPE ***pVoxelsDx = m_pVolumeDx->GetVoxels();
	VOXEL_TYPE ***pVoxelsDy = m_pVolumeDy->GetVoxels();

	for (int nAtZ = 0; nAtZ < m_pVolumeIn->GetDepth(); nAtZ++)
	{
		for (int nAtY = 1; nAtY < m_pVolumeIn->GetHeight()-1; nAtY++)
		{
			for (int nAtX = 1; nAtX < m_pVolumeIn->GetWidth()-1; nAtX++)
			{
				pVoxelsDx[nAtZ][nAtY][nAtX] = 
					(pVoxelsIn[nAtZ][nAtY][nAtX+1] - pVoxelsIn[nAtZ][nAtY][nAtX-1]) / 2.0
				pVoxelsDy[nAtZ][nAtY][nAtX] = 
					(pVoxelsIn[nAtZ][nAtY+1][nAtX] - pVoxelsIn[nAtZ][nAtY+1][nAtX]) / 2.0
			}
		}
	}
}