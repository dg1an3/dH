// Resampler.h: interface for the CResampler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESAMPLER_H__DC5AB918_46BD_420C_AE11_14C81DAA0826__INCLUDED_)
#define AFX_RESAMPLER_H__DC5AB918_46BD_420C_AE11_14C81DAA0826__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Matrix.h>

#include "Volumep.h"

template<class VOXEL_TYPE>
class CResampler : public CObject  
{
public:
	CResampler()
		: m_pSource(NULL),
			m_pDest(NULL)
	{
	}

	virtual ~CResampler()
	{
	}

	CVolume<VOXEL_TYPE> *GetSource()
	{
		return m_pSource;
	}

	void SetSource(CVolume<VOXEL_TYPE> *pSource)
	{
		if (m_pSource != NULL)
		{
			m_pSource->GetChangeEvent().RemoveListener(this, (ChangeFunc) OnChange);
		}

		m_pSource = pSource;

		if (m_pSource != NULL)
		{
			m_pSource->GetChangeEvent().AddListener(this, (ChangeFunc) OnChange);
		}
	}

	CVolume<VOXEL_TYPE> *GetDest()
	{
		return m_pDest;
	}

	void SetDest(CVolume<VOXEL_TYPE> *pDest)
	{
		m_pDest = pDest;
		Resample();
	}

	const CMatrix& GetTransform()
	{
		return m_transform;
	}

	void SetTransform(const CMatrix& mTransform)
	{
		m_transform = mTransform;
		Resample();
	}

	// handles changes from contained objects
	virtual void OnChange(CObservableEvent *pSource, void *pOldValue)
	{
		if (pSource->GetParent() == m_pSource)
		{
			Resample();
		}
	}

protected:
	void Resample();

private:
	CVolume<VOXEL_TYPE> *m_pSource;
	CVolume<VOXEL_TYPE> *m_pDest;

	CMatrix<4> m_transform;
};

#endif // !defined(AFX_RESAMPLER_H__DC5AB918_46BD_420C_AE11_14C81DAA0826__INCLUDED_)
