// Volume.h: interface for the CVolume class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_)
#define AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Value.h"

template<class VOXEL_TYPE>
class CVolume : public CObject, public CObserver
{
public:
	CVolume()
		: width(0), 
			height(0), 
			depth(0),
			m_pVoxels(NULL)
	{
		width.AddObserver(this);
		height.AddObserver(this);
		depth.AddObserver(this);
	}

	virtual ~CVolume()
	{
	}

	CValue< int > width;
	CValue< int > height;
	CValue< int > depth;

	VOXEL_TYPE ***GetVoxels()
	{
		return &m_arrppVoxels[0];
	}

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
	}

	virtual void Serialize(CArchive& ar)
	{
		// only serialize the dimensions (voxels are serialized separately)
		width.Serialize(ar);
		height.Serialize(ar);
		depth.Serialize(ar);

		if (ar.IsStoring())
			ar.Write(m_pVoxels, depth.Get() * height.Get() * width.Get() * sizeof(VOXEL_TYPE));
		else
			ar.Read(m_pVoxels, depth.Get() * height.Get() * width.Get() * sizeof(VOXEL_TYPE));
	}

	virtual void OnChange(CObservable *pSource)
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
};

#endif // !defined(AFX_VOLUME_H__82547A50_0C10_11D5_9E4E_00B0D0609AB0__INCLUDED_)
