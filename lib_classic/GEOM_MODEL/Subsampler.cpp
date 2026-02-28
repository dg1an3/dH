// Subsampler.cpp: implementation of the CSubsampler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Subsampler.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template<typename VOXEL_TYPE>
CSubsampler<VOXEL_TYPE>::CSubsampler()
{

}

template<typename VOXEL_TYPE>
CSubsampler<VOXEL_TYPE>::~CSubsampler()
{

}



template<typename VOXEL_TYPE>
void CSubsampler<VOXEL_TYPE>::Subsample()
{
	m_pVolumeOut->SetDimensions(
		__min(m_pVolumeIn->GetWidth() / 2, 1),
		__min(m_pVolumeIn->GetHeight() / 2, 1),
		__min(m_pVolumeIn->GetDepth() / 2, 1),
	);

	const VOXEL_TYPE ***pVoxelsIn = m_pVolumeIn->GetVoxels();
	VOXEL_TYPE ***pVoxelsOut = m_pVolumeOut->GetVoxels();

	for (int nAtZ = 0; nAtZ < m_pVolumeOut->GetDepth(); nAtZ++)
	{
		for (int nAtY = 0; nAtY < m_pVolumeOut->GetHeight(); nAtY++)
		{
			for (int nAtX = 0; nAtX < m_pVolumeOut->GetWidth(); nAtX++)
			{
				int nAtZInLower = __min(nAtZ/2, m_pVolumeIn->GetDepth());
				int nAtZInUpper = __min(nAtZ/2+1, m_pVolumeIn->GetDepth());

				pVoxelsOut[nAtZ][nAtY][nAtX] = 
					( pVoxelsIn[nAtZInLower][nAtY/2  ][nAtX/2  ]
					+ pVoxelsIn[nAtZInLower][nAtY/2  ][nAtX/2+1]
					+ pVoxelsIn[nAtZInLower][nAtY/2+1][nAtX/2  ]
					+ pVoxelsIn[nAtZInLower][nAtY/2+1][nAtX/2+1]
					+ pVoxelsIn[nAtZInUpper][nAtY/2  ][nAtX/2  ]
					+ pVoxelsIn[nAtZInUpper][nAtY/2  ][nAtX/2+1]
					+ pVoxelsIn[nAtZInUpper][nAtY/2+1][nAtX/2  ]
					+ pVoxelsIn[nAtZInUpper][nAtY/2+1][nAtX/2+1] ) / 8.0;
			}
		}
	}
}
