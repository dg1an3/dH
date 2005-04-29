// BufferND.h: interface for the CBufferND class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFERND_H__829C280C_CF64_4287_BEAC_AFC75C0A0AD9__INCLUDED_)
#define AFX_BUFFERND_H__829C280C_CF64_4287_BEAC_AFC75C0A0AD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#undef min
#undef max

#include <limits>

#include "BufferIndex.h"

#define INLINE __forceinline

/*
class CBufferBase : public CModelObject
{
public:
	virtual const type_info& GetSampleType() = 0;
	virtual int GetInDim() = 0;
	virtual int GetOutDim() = 0;
};
*/

template<int DIM, class TYPE>
class CBufferND  : public CModelObject
{
public:
	CBufferND();
	CBufferND(const CBufferBounds<DIM>& extent);
	virtual ~CBufferND();

	// CBufferBase functions
	virtual const type_info& GetSampleType(); 
	virtual int GetInDim();
	virtual int GetOutDim();

	// sizing the buffer
	const CBufferBounds<DIM>& GetExtent() const;
	void SetExtent(const CBufferBounds<DIM>& extent, 
		TYPE *pSamples = NULL);

	// conforming to an existing buffer's size
	template<class OTHER_TYPE>
	void ConformTo(const CBufferND<DIM, OTHER_TYPE>& otherBuffer)
	{
		SetExtent(otherBuffer.GetExtent());
	}

	// useful counting helper
	int GetTotalSamples() const;

	// initializing to zeroes
	void ClearSamples();

	// index accessors
	TYPE& operator[](const CBufferIndex<DIM>& vIndex);
	const TYPE& operator[](const CBufferIndex<DIM>& vIndex) const;

	// lies, damn lies, and statistics
	TYPE GetMax() const;
	TYPE GetMin() const;
	TYPE GetAvg() const;

	// operations
	void Accumulate(const CBufferND& otherBuffer, const TYPE& weight); // = 1.0);

	void Convolve(const CBufferND<DIM, TYPE>& buf_in,
			  const CBufferND<DIM, TYPE>& kernel);

	// indicates the samples have been altered
	void SamplesChanged();

	////////////////////////////////////////////////////////////////////////
	// class IliffeVector
	//
	// IliffeVector<ILIFFE_DIM> are critical to function of CBufferND.  
	//		this internal template class provides all functions for set-up 
	//		and	access to samples using nested iliffe vectors
	////////////////////////////////////////////////////////////////////////
	template<int ILIFFE_DIM>
	class IliffeVector 
	{
	public:
		// array accessors
		INLINE IliffeVector<ILIFFE_DIM-1>& operator[](int nIndex)
		{
			return m_ptr[nIndex];
		}

		INLINE const IliffeVector<ILIFFE_DIM-1>& operator[](int nIndex) const
		{
			return m_ptr[nIndex];
		}

		// index-based accessors
		INLINE TYPE& operator[](const CBufferIndex<DIM>& vIndex) 
		{
			return m_ptr[vIndex[ILIFFE_DIM-1]][vIndex];
		}

		INLINE const TYPE& operator[](const CBufferIndex<DIM>& vIndex) const
		{
			return m_ptr[vIndex[ILIFFE_DIM-1]][vIndex];
		}

	protected:
		friend class CBufferND<DIM, TYPE>;
		friend class IliffeVector<ILIFFE_DIM+1>;

		// constructor
		IliffeVector<ILIFFE_DIM>()
			: m_ptr(NULL)
		{
		}

		// static -- for blocks of iliffe
		static IliffeVector<ILIFFE_DIM> *AllocIliffe(const CBufferBounds<DIM>& bounds,
												int& nElemCount, TYPE*& pSamples)
		{
			IliffeVector<ILIFFE_DIM> *pPtrs = new IliffeVector<ILIFFE_DIM>[nPtrCount];

			int nSubCount = bounds.GetElemCount(ILIFFE_DIM-1);

			int nTotalElements = nElemCount * nSubCount;
			IliffeVector<ILIFFE_DIM-1> *pSubPtrs = 
				IliffeVector<ILIFFE_DIM-1>::AllocIliffe(bounds, 
					nTotalElements, pSamples);

			for (int nAt = 0; nAt < nElemCount; nAt++)
			{
				pPtrs[nAt].m_ptr = pSubPtrs; // - bounds.m_vMin[ILIFFE_DIM-1];
				pSubPtrs += nSubCount;
			}

			nElemCount = nTotalElements;

			return pPtrs - bounds.GetMin()[ILIFFE_DIM-1];
		}

		// static -- for blocks of iliffe
		static void FreeIliffe(IliffeVector<ILIFFE_DIM> *pIliffeVector)
		{
			IliffeVector<ILIFFE_DIM-1>::FreeIliffe(pIliffeVector->m_ptr);
			delete [] (pIliffeVector + bounds.GetMin()[ILIFFE_DIM]);
		}

	private:
		// subordinate iliffe vectors
		IliffeVector<ILIFFE_DIM-1> *m_ptr;

	};	// class IliffeVector

	////////////////////////////////////////////////////////////////////////
	// class IliffeVector<1>
	//
	// IliffeVector<1> is a specific instantiation to terminate the 
	//		recursive definition of IliffeVector<N>.
	////////////////////////////////////////////////////////////////////////
	template<>
	class IliffeVector<1>
	{
	public:
		INLINE TYPE& operator[](int nIndex)
		{
			return m_ptr[nIndex];
		}

		INLINE const TYPE& operator[](int nIndex) const
		{
			return m_ptr[nIndex];
		}

		INLINE TYPE& operator[](const CBufferIndex<DIM>& vIndex) 
		{
			return m_ptr[vIndex[0]];
		}

		INLINE const TYPE& operator[](const CBufferIndex<DIM>& vIndex) const
		{
			return m_ptr[vIndex[0]];
		}

	protected:
		friend class CBufferND<DIM, TYPE>;
		friend class IliffeVector<2>;

		IliffeVector<1>()
			: m_ptr(NULL)
		{
		}

		// static -- for blocks of iliffe
		static IliffeVector<1> *AllocIliffe(const CBufferBounds<DIM>& bounds,
										int& nElemCount, TYPE*& pSamples)
		{
			IliffeVector<1> *pPtrs = new IliffeVector<1>[nElemCount];

			int nSubCount = bounds.GetElemCount(0);
			int nSampCount = nElemCount * nSubCount;

			if (NULL == pSamples)
			{
				pSamples = new TYPE[nSampCount];
			}

			TYPE *pSampPtr = pSamples - bounds.GetMin()[0];
			for (int nAt = 0; nAt < nElemCount; nAt++)
			{
				pPtrs[nAt].m_ptr = pSampPtr;
				pSampPtr += nSubCount;
			}

			nElemCount = nSampCount;

			return pPtrs - bounds.GetMin()[1];
		}

		// static -- for blocks of iliffe
		static void FreeIliffe(IliffeVector<1> *pIliffeVector, 
			const CBufferBounds<DIM>& bounds)
		{
			IliffeVector<1> *pStartPtr = pIliffeVector + bounds.GetMin()[1];
			if (pStartPtr->m_ptr)
			{
				delete [] (pStartPtr->m_ptr + bounds.GetMin()[0]);
			}
			delete [] pStartPtr;
		}

	private:
		TYPE *m_ptr;

	};	// class IliffeVector<1>

	// iliffe-based accessors
	IliffeVector<DIM-1>& operator[](int nIndex);
	const IliffeVector<DIM-1>& operator[](int nIndex) const;

private:
	// the extent of the buffer
	CBufferBounds<DIM> m_extent;

	// stores total count
	int m_nTotalSamples;

	// the main iliffe
	IliffeVector<DIM> m_iliffeVector;

	// easy access to samples
	TYPE *m_pSamples;

	mutable TYPE m_avg;
	mutable BOOL m_bRecalcAvg;

	mutable TYPE m_max;
	mutable BOOL m_bRecalcMax;

	mutable TYPE m_min;
	mutable BOOL m_bRecalcMin;
};


template<int DIM, class TYPE>
CBufferND<DIM, TYPE>::CBufferND<DIM, TYPE>() 
: m_nTotalSamples(0),
	m_pSamples(NULL),
	m_bRecalcMax(TRUE),
	m_bRecalcMin(TRUE)
{ 
}

template<int DIM, class TYPE>
CBufferND<DIM, TYPE>::CBufferND<DIM, TYPE>(const CBufferBounds<DIM>& extent) 
: m_nTotalSamples(0),
	m_pSamples(NULL),
	m_bRecalcMax(TRUE),
	m_bRecalcMin(TRUE)
{
	SetExtent(extent);
}

template<int DIM, class TYPE>
CBufferND<DIM, TYPE>::~CBufferND() 
{
	if (m_iliffeVector.m_ptr)
	{
		IliffeVector<DIM-1>::FreeIliffe(m_iliffeVector.m_ptr, GetExtent());
	}
}

template<int DIM, class TYPE>
const type_info& CBufferND<DIM, TYPE>::GetSampleType() 
{ 
	return typeid(TYPE); 
}

template<int DIM, class TYPE>
int CBufferND<DIM, TYPE>::GetInDim() 
{ 
	return DIM; 
}

template<int DIM, class TYPE>
int CBufferND<DIM, TYPE>::GetOutDim() 
{ 
	return 1; // GetDim<TYPE>(); 
}

template<int DIM, class TYPE>
const CBufferBounds<DIM>& CBufferND<DIM, TYPE>::GetExtent() const
{ 
	return m_extent; 
}

template<int DIM, class TYPE>
void CBufferND<DIM, TYPE>::SetExtent(const CBufferBounds<DIM>& extent, TYPE *pSamples)
{
	if (m_extent != extent)
	{
		if (m_iliffeVector.m_ptr)
		{
			IliffeVector<DIM-1>::FreeIliffe(m_iliffeVector.m_ptr, GetExtent());
		}

		m_pSamples = pSamples;
		m_extent = extent;
		m_nTotalSamples = extent.GetElemCount(DIM-1);
		m_iliffeVector.m_ptr = IliffeVector<DIM-1>::AllocIliffe(extent, 
			m_nTotalSamples, m_pSamples);
	}
}

template<int DIM, class TYPE>
int CBufferND<DIM, TYPE>::GetTotalSamples() const
{
	return m_nTotalSamples;
}

template<int DIM, class TYPE>
void CBufferND<DIM, TYPE>::ClearSamples()
{
	memset(m_pSamples, 0, 
		GetTotalSamples() * sizeof(TYPE));
}


template<int DIM, class TYPE> INLINE
TYPE& CBufferND<DIM, TYPE>::operator[](const CBufferIndex<DIM>& vIndex)
{
	return m_iliffeVector[vIndex];
}

template<int DIM, class TYPE> INLINE
const TYPE& CBufferND<DIM, TYPE>::operator[](const CBufferIndex<DIM>& vIndex) const
{
	return m_iliffeVector[vIndex];
} 

template<int DIM, class TYPE> INLINE
CBufferND<DIM, TYPE>::IliffeVector<DIM-1>& CBufferND<DIM, TYPE>::operator[](int nIndex)
{
	return m_iliffeVector[nIndex];
}

template<int DIM, class TYPE> INLINE
const CBufferND<DIM, TYPE>::IliffeVector<DIM-1>& CBufferND<DIM, TYPE>::operator[](int nIndex) const
{
	return m_iliffeVector[nIndex];
}

template<int DIM, class TYPE>
TYPE CBufferND<DIM, TYPE>::GetMax() const
{
	if (m_bRecalcMax)
	{
		m_max = numeric_limits<TYPE>::min();

		TYPE *pSampleAt = m_pSamples;
		for (int nSampleCount = GetTotalSamples(); nSampleCount >= 0; 
			nSampleCount--, pSampleAt++)
		{
			m_max = __max(*pSampleAt, m_max);
		}

		m_bRecalcMax = FALSE;
	}

	return m_max;
}

template<int DIM, class TYPE>
TYPE CBufferND<DIM, TYPE>::GetMin() const
{
	if (m_bRecalcMin)
	{
		m_min = numeric_limits<TYPE>::max();

		TYPE *pSampleAt = m_pSamples;
		for (int nSampleCount = GetTotalSamples(); nSampleCount >= 0; 
			nSampleCount--, pSampleAt++)
		{
			m_min = __min(*pSampleAt, m_min);
		}

		m_bRecalcMin = FALSE;
	}

	return m_min;
}

template<int DIM, class TYPE>
TYPE CBufferND<DIM, TYPE>::GetAvg() const
{
	if (m_bRecalcAvg)
	{
		TYPE sum;

		TYPE *pSampleAt = m_pSamples;
		for (int nSampleCount = GetTotalSamples(); nSampleCount >= 0; 
			nSampleCount--, pSampleAt++)
		{
			sum += (*pSampleAt);
		}

		REAL count = GetTotalSamples();
		m_avg = sum;
		m_avg *= (1.0 / count);

		m_bRecalcAvg = FALSE;
	}

	return m_avg;
}


template<int DIM, class TYPE>
INLINE void AccumProdOp(CVectorD<DIM, TYPE>& vRes, 
				const CVectorD<DIM, TYPE>& vLeft,
				const CVectorD<DIM, TYPE>& vRight)
{
	for (int nAt = 0; nAt < DIM; nAt++)
	{
		vRes[nAt] += vLeft[nAt] * vRight[nAt];
	}
}

INLINE void AccumProdOp(float& vRes, 
				const float& vLeft,
				const float& vRight)
{
	vRes += vLeft * vRight;
}

INLINE void AccumProdOp(double& vRes, 
				const double& vLeft,
				const double& vRight)
{
	vRes += vLeft * vRight;
}


template<int DIM, class TYPE>
void CBufferND<DIM, TYPE>::Accumulate(const CBufferND<DIM, TYPE>& otherBuffer, const TYPE& weight)
{
	ASSERT(otherBuffer.GetExtent().GetMax() == GetExtent().GetMax());

	TYPE *pOtherSampleAt = otherBuffer.m_pSamples;
	TYPE *pSampleAt = m_pSamples;

	for (int nSampleCount = GetTotalSamples(); nSampleCount > 0; 
		nSampleCount--, pSampleAt++, pOtherSampleAt++)
	{
		AccumProdOp((*pSampleAt), weight, (*pOtherSampleAt));
	}

	SamplesChanged();
}


template<int DIM, class TYPE>
void CBufferND<DIM, TYPE>::Convolve(const CBufferND<DIM, TYPE>& buf_in,
			  const CBufferND<DIM, TYPE>& kernel)
{
	SetExtent(buf_in.GetExtent());
	ClearSamples();

	for (CBufferIndex<DIM> vBufIndex(GetExtent()); 
		vBufIndex.IsInRange();
		vBufIndex++)
	{
		for(CBufferIndex<DIM> vKernelNdx(kernel.GetExtent());
			vKernelNdx.IsInRange();
			vKernelNdx++)
		{
			CBufferIndex<DIM> vShiftNdx(vBufIndex);
			vShiftNdx += vKernelNdx;

			if (vShiftNdx.IsInRange())
			{
				AccumProdOp((*this)[vBufIndex], 
					kernel[vKernelNdx], buf_in[vShiftNdx]);
			} 
		}
	}

	SamplesChanged();
}



template<int DIM, class TYPE>
void CBufferND<DIM, TYPE>::SamplesChanged()
{
	m_bRecalcAvg = TRUE;
	m_bRecalcMax = TRUE;
	m_bRecalcMin = TRUE;
}

#endif // !defined(AFX_BUFFERND_H__829C280C_CF64_4287_BEAC_AFC75C0A0AD9__INCLUDED_)
