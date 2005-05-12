#if !defined(VECTOROPS_H)
#define VECTOROPS_H

#ifdef USE_IPP
#include <ipps.h>
#include <ippm.h>
#endif


template<class TYPE> __forceinline
void AllocValues(int nCount, TYPE*& pValues)
{
	pValues = new TYPE[nCount];
}

template<class TYPE> __forceinline
void FreeValues(TYPE*& pValues)
{
	delete pValues;
	pValues = NULL;
}


#ifdef USE_IPP

template<> __forceinline			
void AllocValues(int nCount, float *& pValues)
{											
//	TRACE("Allocating %i values of type %s\n", nCount, "32f");
	pValues = ippsMalloc_32f(nCount);	
}											

template<> __forceinline					
void FreeValues(float *& pValues)				
{											
	ippsFree(pValues);						
	pValues = NULL;							
}

#define DECLARE_ALLOC_FREE(TYPE, TYPE_IPP) \
	template<> __forceinline					\
	void AllocValues(int nCount, TYPE*& pValues) \
	{											\
		TRACE("Allocating %i values of type %s\n", nCount, #TYPE_IPP);	\
		pValues = ippsMalloc_##TYPE_IPP(nCount);	\
	}											\
	template<> __forceinline					\
	void FreeValues(TYPE*& pValues)				\
	{											\
		ippsFree(pValues);						\
		pValues = NULL;							\
	}

DECLARE_ALLOC_FREE(UCHAR, 8u);
DECLARE_ALLOC_FREE(USHORT, 16u);
DECLARE_ALLOC_FREE(SHORT, 16s);
DECLARE_ALLOC_FREE(UINT, 32u);
DECLARE_ALLOC_FREE(int, 32s);
/// DECLARE_ALLOC_FREE(float, 32f);
DECLARE_ALLOC_FREE(double, 64f);

#endif


template<class TYPE> __forceinline
void AssignValues(TYPE *pTo, const TYPE *pFrom, int nCount)
{
	// copy the elements
	memcpy(pTo, pFrom, nCount * sizeof(TYPE));
}

#ifdef USE_IPP
template<> __forceinline
void AssignValues(UCHAR *pTo, const UCHAR *pFrom, int nCount)
{
	// copy the elements
	ippsCopy_8u(pFrom, pTo, nCount);
}

template<> __forceinline
void AssignValues(short *pTo, const short *pFrom, int nCount)
{
	// copy the elements
	ippsCopy_16s(pFrom, pTo, nCount);
}

template<> __forceinline
void AssignValues(float *pTo, const float *pFrom, int nCount)
{
	// copy the elements
	ippsCopy_32f(pFrom, pTo, nCount);
}

template<> __forceinline
void AssignValues(double *pTo, const double *pFrom, int nCount)
{
	// copy the elements
	ippsCopy_64f(pFrom, pTo, nCount);
}
#endif


template<class TYPE> __forceinline
void SumValues(TYPE *pSrc1Dst, const TYPE *pSrc2, int nCount)
{
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		(*pSrc1Dst++) += (*pSrc2++);
	}
}

#ifdef USE_IPP
template<> __forceinline
void SumValues(short *pSrc1Dst, const short *pSrc2, int nCount)
{
	ippsAdd_16s_I(pSrc2, pSrc1Dst, nCount);
}

template<> __forceinline
void SumValues(float *pSrc1Dst, const float *pSrc2, int nCount)
{
	ippsAdd_32f_I(pSrc2, pSrc1Dst, nCount);
}


template<> __forceinline
void SumValues(double *pSrc1Dst, const double *pSrc2, int nCount)
{
	ippsAdd_64f_I(pSrc2, pSrc1Dst, nCount);
}
#endif


template<class TYPE> __forceinline
void DiffValues(TYPE *pSrc1Dst, const TYPE *pSrc2, int nCount)
{
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		(*pSrc1Dst++) -= (*pSrc2++);
	}
}

#ifdef USE_IPP
template<> __forceinline
void DiffValues(short *pSrc1Dst, const short *pSrc2, int nCount)
{
	ippsSub_16s_I(pSrc2, pSrc1Dst, nCount);
}

template<> __forceinline
void DiffValues(float *pSrc1Dst, const float *pSrc2, int nCount)
{
	ippsSub_32f_I(pSrc2, pSrc1Dst, nCount);
}


template<> __forceinline
void DiffValues(double *pSrc1Dst, const double *pSrc2, int nCount)
{
	ippsSub_64f_I(pSrc2, pSrc1Dst, nCount);
}
#endif



template<class TYPE> __forceinline
void ScaleValues(TYPE *pSrcDst, TYPE scale, int nCount)
{
	for (int nAt = 0; nAt < nCount; nAt++)
	{
		(*pSrcDst++) *= scale;
	}
}

#ifdef USE_IPP
template<> __forceinline
void ScaleValues(short *pSrcDst, short scale, int nCount)
{
	ippsMulC_16s_I(scale, pSrcDst, nCount);
}

template<> __forceinline
void ScaleValues(float *pSrcDst, float scale, int nCount)
{
	ippsMulC_32f_I(scale, pSrcDst, nCount);
}

template<> __forceinline
void ScaleValues(double *pSrcDst, double scale, int nCount)
{
	ippsMulC_64f_I(scale, pSrcDst, nCount);
}
#endif



template<class ELEM_TYPE> __forceinline
ELEM_TYPE VectorLength(const ELEM_TYPE *pV, int nLength)
{
	// form the sum of the square of each element
	ELEM_TYPE lenSq = (ELEM_TYPE) 0.0;
	for (int nAt = 0; nAt < nLength; nAt++)
	{
		lenSq += pV[nAt] * pV[nAt];
	}
	return (ELEM_TYPE) sqrt(lenSq);
}




template<class ELEM_TYPE> __forceinline
ELEM_TYPE DotProduct(const ELEM_TYPE *pLeft, 
						const ELEM_TYPE *pRight, int nLength)
{
	// stores the dot product
	ELEM_TYPE prod = (ELEM_TYPE) 0.0;

	// sum the element-wise multiplication
	for (int nAt = 0; nAt < nLength; nAt++)
	{
		prod += pLeft[nAt] * pRight[nAt];
	}
	
	return prod;
}



//////////////////////////////////////////////////////////////////////
// RandomVector
//
// initializes a random vector
//////////////////////////////////////////////////////////////////////
template<class ELEM_TYPE>
void RandomVector(ELEM_TYPE range, ELEM_TYPE *pV, int nLength)
{
	for (int nAt = 0; nAt < nLength; nAt++)
	{
		pV[nAt] = range - (ELEM_TYPE) 2.0 * range 
			* (ELEM_TYPE) rand() 
				/ (ELEM_TYPE) RAND_MAX;
	}

}	// RandomVector



#ifdef USE_XMLLOGGING
//////////////////////////////////////////////////////////////////////
// LogExprExt
//
// helper function for XML logging of vectors
//////////////////////////////////////////////////////////////////////
inline void LogExprExt(const CRect& rect, const char *pszName, const char *pszModule)
{
	// get the global log file
	CXMLLogFile *pLog = CXMLLogFile::GetLogFile();

	// only if we are logging --
	if (pLog->IsLogging())
	{
		// create a new expression element
		CXMLElement *pVarElem = pLog->NewElement("lx", pszModule);

		// if there is a name,
		if (strlen(pszName) > 0)
		{
			// set it.
			pVarElem->Attribute("name", pszName);
		}

		// set type to generice "CVector"
		pVarElem->Attribute("type", "CRect");
		
		// get the current format for the element type
		const char *pszFormat = pLog->GetFormat(rect.bottom);

		// format each element
		pLog->Format(pszFormat, rect.top);
		pLog->Format(pszFormat, rect.left);
		pLog->Format(pszFormat, rect.bottom);
		pLog->Format(pszFormat, rect.right);

		// done.
		pLog->GetLogFile()->CloseElement();
	}

}	// LogExprExt


#endif	// USE_XMLLOGGING



#endif
