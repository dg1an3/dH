//////////////////////////////////////////////////////////////////////
// Volume.h: declaration of the CVolume class
//
// Copyright (C) 2000-2001
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_)
#define AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ModelObject.h"

#undef max
#undef min

#include <limits>
using namespace std;

//////////////////////////////////////////////////////////////////////
// class CVolume<VOXEL_TYPE>
//
// template class representing an arbitrary-typed volume.
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
class CVolume : public CModelObject
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CVolume()
		: m_nWidth(0), 
			m_nHeight(0), 
			m_nDepth(0),
			m_pVoxels(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////
	// destructor
	//////////////////////////////////////////////////////////////////
	virtual ~CVolume()
	{
	}

	//////////////////////////////////////////////////////////////////
	// dimension attributes
	//////////////////////////////////////////////////////////////////
	int GetWidth() 
	{ 
		return m_nWidth; 
	}

	//////////////////////////////////////////////////////////////////
	// dimension attributes
	//////////////////////////////////////////////////////////////////
	int GetHeight() 
	{ 
		return m_nHeight; 
	}

	//////////////////////////////////////////////////////////////////
	// dimension attributes
	//////////////////////////////////////////////////////////////////
	int GetDepth() 
	{ 
		return m_nDepth; 
	}

	//////////////////////////////////////////////////////////////////
	// sets the attributes
	//////////////////////////////////////////////////////////////////
	void SetDimensions(int nWidth, int nHeight, int nDepth)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_nDepth = nDepth;

		SetVoxels(m_pVoxels, m_nWidth, m_nHeight, m_nDepth);
	}

	//////////////////////////////////////////////////////////////////
	// accessor for the voxels
	//////////////////////////////////////////////////////////////////
	VOXEL_TYPE ***GetVoxels()
	{
		return &m_arrppVoxels[0];
	}

	//////////////////////////////////////////////////////////////////
	// sets the raw voxel pointer
	//////////////////////////////////////////////////////////////////
	void SetVoxels(VOXEL_TYPE *pVoxels, int nWidth, int nHeight, int nDepth)
	{
		// set the default array of voxels
		m_arrVoxels.SetSize(nDepth * nHeight * nWidth);

		if (pVoxels == NULL)
		{
			// set the raw voxel pointer
			m_pVoxels = (m_arrVoxels.GetSize() > 0) ? &m_arrVoxels[0] : NULL;
		}
		else
		{
			// set the raw voxel pointer to NULL
			m_pVoxels = pVoxels;
		}

		// now set the size of the indirection arrays
		m_arrpVoxels.SetSize(nDepth * nHeight);
		m_arrppVoxels.SetSize(nDepth);

		if (nDepth > 0 && nHeight > 0 && nWidth > 0)
		{
			for (int nAtZ = 0; nAtZ < nDepth; nAtZ++)
			{
				// set the elements in the outer indirection array
				m_arrppVoxels[nAtZ] = &m_arrpVoxels[nAtZ * m_nHeight];


				for (int nAtY = 0; nAtY < nHeight; nAtY++)
				{
					// set the elements in the inner indirection array
					m_arrpVoxels[nAtZ * nHeight + nAtY] =
						&m_pVoxels[nAtZ * nHeight * nWidth + nAtY * nHeight];
				}
			}
		}

		// now set the dimension member variables
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_nDepth = nDepth;

		m_bRecomputeSum = TRUE;
	}

	//////////////////////////////////////////////////////////////////
	// sets all voxels to zero
	//////////////////////////////////////////////////////////////////
	void ClearVoxels()
	{
		for (int nAtZ = 0; nAtZ < m_nDepth; nAtZ++)
		{
			for (int nAtY = 0; nAtY < m_nHeight; nAtY++)
			{
				for (int nAtX = 0; nAtX < m_nWidth; nAtX++)
				{
					GetVoxels()[nAtZ][nAtY][nAtX] = (VOXEL_TYPE) 0;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// sets all voxels to zero
	//////////////////////////////////////////////////////////////////
	void Accumulate(CVolume *pVolume, double weight = 1.0)
	{
		for (int nAtZ = 0; nAtZ < m_nDepth; nAtZ++)
		{
			for (int nAtY = 0; nAtY < m_nHeight; nAtY++)
			{
				for (int nAtX = 0; nAtX < m_nWidth; nAtX++)
				{
					GetVoxels()[nAtZ][nAtY][nAtX] += weight * 
						pVolume->GetVoxels()[nAtZ][nAtY][nAtX];
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// forms the sum of the volume
	//////////////////////////////////////////////////////////////////
	VOXEL_TYPE GetSum()
	{
		VOXEL_TYPE sum = (VOXEL_TYPE) 0;
		for (int nAtZ = 0; nAtZ < m_nDepth; nAtZ++)
		{
			for (int nAtY = 0; nAtY < m_nHeight; nAtY++)
			{
				for (int nAtX = 0; nAtX < m_nWidth; nAtX++)
				{
					sum += GetVoxels()[nAtZ][nAtY][nAtX];
				}
			}
		}

		return sum;
	}

	//////////////////////////////////////////////////////////////////
	// forms the max of the volume voxel values
	//////////////////////////////////////////////////////////////////
	VOXEL_TYPE GetMax()
	{
		VOXEL_TYPE nMaxValue = -numeric_limits<VOXEL_TYPE>::max();
		for (int nAtZ = 0; nAtZ < m_nDepth; nAtZ++)
		{
			for (int nAtY = 0; nAtY < m_nHeight; nAtY++)
			{
				for (int nAtX = 0; nAtX < m_nWidth; nAtX++)
				{
					nMaxValue = 
						__max(nMaxValue, GetVoxels()[nAtZ][nAtY][nAtX]);
				}
			}
		}

		return nMaxValue;
	}

	//////////////////////////////////////////////////////////////////
	// forms the max of the volume voxel values
	//////////////////////////////////////////////////////////////////
	VOXEL_TYPE GetMin()
	{
		VOXEL_TYPE nMinValue = numeric_limits<VOXEL_TYPE>::max();
		for (int nAtZ = 0; nAtZ < m_nDepth; nAtZ++)
		{
			for (int nAtY = 0; nAtY < m_nHeight; nAtY++)
			{
				for (int nAtX = 0; nAtX < m_nWidth; nAtX++)
				{
					nMinValue = 
						__min(nMinValue, GetVoxels()[nAtZ][nAtY][nAtX]);
				}
			}
		}

		return nMinValue;
	}

	//////////////////////////////////////////////////////////////////
	// serializes the volume
	//////////////////////////////////////////////////////////////////
	virtual void Serialize(CArchive& ar)
	{
		if (ar.IsStoring())
		{
			ar << m_nWidth;
			ar << m_nHeight;
			ar << m_nDepth;

			UINT nBytes = m_nDepth * m_nHeight * m_nWidth * sizeof(VOXEL_TYPE);
			ar.Write(m_pVoxels, nBytes);
		}
		else
		{
			ar >> m_nWidth;
			ar >> m_nHeight;
			ar >> m_nDepth;

			SetVoxels(m_pVoxels, m_nWidth, m_nHeight, m_nDepth);

			UINT nBytes = m_nDepth * m_nHeight * m_nWidth * sizeof(VOXEL_TYPE);
			UINT nActBytes = ar.Read(m_pVoxels, nBytes);
			ASSERT(nActBytes == nBytes);
		}
	}

private:
	// dimensions
	int m_nWidth;
	int m_nHeight;
	int m_nDepth;

	// pointer to raw voxels
	VOXEL_TYPE *m_pVoxels;

	// array of raw voxels (if no replacement is set)
	CArray<VOXEL_TYPE, VOXEL_TYPE&> m_arrVoxels;

	// indirection arrays for voxels
	CArray<VOXEL_TYPE *, VOXEL_TYPE *&> m_arrpVoxels;
	CArray<VOXEL_TYPE **, VOXEL_TYPE **&> m_arrppVoxels;

	// flag to recompute the volume's sum
	BOOL m_bRecomputeSum;
	VOXEL_TYPE m_sum;
};

//////////////////////////////////////////////////////////////////////
// DEPRECATED
// function operator>>(CArchive &ar, CVolume<VOXEL_TYPE>*&)
//
// CArchive de-serialization of a volume
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
CArchive& operator>>(CArchive& ar, CVolume<VOXEL_TYPE>*&pOb) 
{ 
	// if no pointer is passed in,
	if (pOb == NULL)
	{
		// allocate a new volume
		pOb = new CVolume<VOXEL_TYPE>; 
	}

	// serialize the object
	pOb->Serialize(ar);

	// return the archive
	return ar; 
} 

#endif // !defined(AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_)
