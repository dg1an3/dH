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

#include "Value.h"

//////////////////////////////////////////////////////////////////////
// class CVolume<VOXEL_TYPE>
//
// template class representing an arbitrary-typed volume.
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
class CVolume : public CObservableObject
{
public:
	//////////////////////////////////////////////////////////////////
	// default constructor
	//////////////////////////////////////////////////////////////////
	CVolume()
		: width(0), 
			height(0), 
			depth(0),
			m_pVoxels(NULL)
	{
		::AddObserver<CVolume>(&width, this, OnChange);
		::AddObserver<CVolume>(&height, this, OnChange);
		::AddObserver<CVolume>(&depth, this, OnChange);
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
	CValue< int > width;
	CValue< int > height;
	CValue< int > depth;

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
			for (int nAtZ = 0; nAtZ < nDepth; nAtZ++)
			{
				// set the elements in the outer indirection array
				m_arrppVoxels[nAtZ] = &m_arrpVoxels[nAtZ * height.Get()];


				for (int nAtY = 0; nAtY < nHeight; nAtY++)
					// set the elements in the inner indirection array
					m_arrpVoxels[nAtZ * nHeight + nAtY] =
						&m_pVoxels[nAtZ * nHeight * nWidth + nAtY * nHeight];
			}

		// now set the dimension member variables
		width.Set(nWidth);
		height.Set(nHeight);
		depth.Set(nDepth);

		m_bRecomputeSum = TRUE;
	}

	//////////////////////////////////////////////////////////////////
	// forms the sum of the volume
	//////////////////////////////////////////////////////////////////
	VOXEL_TYPE GetSum()
	{
		VOXEL_TYPE sum = (VOXEL_TYPE) 0;
		for (int nAtZ = 0; nAtZ < depth.Get(); nAtZ++)
			for (int nAtY = 0; nAtY < height.Get(); nAtY++)
				for (int nAtX = 0; nAtX < width.Get(); nAtX++)
					sum += GetVoxels()[nAtZ][nAtY][nAtX];

		return sum;
	}

	//////////////////////////////////////////////////////////////////
	// serializes the volume
	//////////////////////////////////////////////////////////////////
	virtual void Serialize(CArchive& ar)
	{
		// only serialize the dimensions (voxels are serialized separately)
		width.Serialize(ar);
		height.Serialize(ar);
		depth.Serialize(ar);

		UINT nBytes = depth.Get() * height.Get() * width.Get() * sizeof(VOXEL_TYPE);
		if (ar.IsStoring())
		{
			ar.Write(m_pVoxels, nBytes);
		}
		else
		{
			UINT nActBytes = ar.Read(m_pVoxels, nBytes);
			ASSERT(nActBytes == nBytes);
		}
	}

	//////////////////////////////////////////////////////////////////
	// fires a change on the volume
	//////////////////////////////////////////////////////////////////
	virtual void OnChange(CObservableObject *pSource, void *pOldValue)
	{
		SetVoxels(m_pVoxels, width.Get(), height.Get(), depth.Get());
	}

private:
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
// function operator>>(CArchive &ar, CVolume<VOXEL_TYPE>*&)
//
// CArchive de-serialization of a volume
//////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
CArchive& operator>>(CArchive& ar, CVolume<VOXEL_TYPE>*&pOb) 
{ 
	// if no pointer is passed in,
	if (pOb == NULL)
		// allocate a new volume
		pOb = new CVolume<VOXEL_TYPE>; 

	// serialize the object
	pOb->Serialize(ar);

	// return the archive
	return ar; 
} 

#endif // !defined(AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_)
