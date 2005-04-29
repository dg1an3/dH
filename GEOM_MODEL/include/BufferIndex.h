// BufferIndex.h: interface for the CBufferIndex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFERINDEX_H__92CD5024_8D8C_4B6E_A6F2_F18F88E135F4__INCLUDED_)
#define AFX_BUFFERINDEX_H__92CD5024_8D8C_4B6E_A6F2_F18F88E135F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>

#define INLINE __forceinline

template<int DIM>
class CBufferBounds;

template<int DIM>
class CBufferIndex : public CVectorD<DIM, int>
{
public:
	CBufferIndex(const CBufferBounds<DIM>& bounds)
		: CVectorD<DIM, int>(), m_bounds(bounds) 
	{ 
		CVectorD<DIM, int>::operator=(m_bounds.GetMin());
	}
	CBufferIndex(const CBufferBounds<DIM>& bounds, int n1)
		: CVectorD<DIM, int>(n1), m_bounds(bounds)
	{
	}

	CBufferIndex(const CBufferBounds<DIM>& bounds, int n1, int n2)
		: CVectorD<DIM, int>(n1, n2), m_bounds(bounds)
	{
	}

	CBufferIndex(const CBufferBounds<DIM>& bounds, int n1, int n2, int n3)
		: CVectorD<DIM, int>(n1, n2, n3), m_bounds(bounds)
	{
	}

	CBufferIndex(const CBufferIndex& fromIndex)
		: CVectorD<DIM, int>(fromIndex), m_bounds(fromIndex.m_bounds)
	{
	}

	INLINE CBufferIndex& operator++(int n)
	{
		int nAt = 0;
		(*this)[nAt]++;
		while ((*this)[nAt] > m_bounds.GetMax()[nAt]
			&& nAt < DIM-1)
		{
			(*this)[nAt] = m_bounds.GetMin()[nAt];
			(*this)[++nAt]++;
		}
		return (*this);
	}

	INLINE CBufferIndex& operator--(int n)
	{
		int nAt = 0;
		(*this)[nAt]--;
		while ((*this)[nAt] < m_bounds.GetMin()[nAt]
			&& nAt < DIM-1)
		{
			(*this)[nAt] = m_bounds.GetMax()[nAt];
			(*this)[--nAt]--;
		}
		return (*this);
	}

	INLINE BOOL IsInRange() const
	{
		for (int nAt = 0; nAt < DIM; nAt++)
		{
			if ((*this)[nAt] > m_bounds.GetMax()[nAt]
				|| (*this)[nAt] < m_bounds.GetMin()[nAt])
			{
				return FALSE;
			}
		}
		return TRUE;
	}

private:
	const CBufferBounds<DIM>& m_bounds;
};

template<int DIM>
class CBufferBounds 
{
public:
	CBufferBounds() 
	{ 
	}

	CBufferBounds(const CVectorD<DIM, int>& vMin,
		const CVectorD<DIM, int>& vMax)
		: m_vMin(vMin), m_vMax(vMax) 
	{ 
	}

	INLINE const CVectorD<DIM, int>& GetMin() const
	{
		return m_vMin;
	}

	INLINE const CVectorD<DIM, int>& GetMax() const
	{
		return m_vMax;
	}

	INLINE int GetElemCount(int nDim) const
	{
		return m_vMax[nDim] - m_vMin[nDim] + 1;
	}

private:
	CVectorD<DIM, int> m_vMin;
	CVectorD<DIM, int> m_vMax;
};

template<int DIM>
bool operator==(const CBufferBounds<DIM>& vLeft, const CBufferBounds<DIM>& vRight)
{
	return vLeft.GetMin() == vRight.GetMin()
		&& vLeft.GetMax() == vRight.GetMax();
}

template<int DIM>
bool operator!=(const CBufferBounds<DIM>& vLeft, const CBufferBounds<DIM>& vRight)
{
	return !(vLeft == vRight);
}

#endif // !defined(AFX_BUFFERINDEX_H__92CD5024_8D8C_4B6E_A6F2_F18F88E135F4__INCLUDED_)
