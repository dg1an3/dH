// Collection.cpp: implementation of the CCollection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Collection.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//template<class TYPE>
//class CTestSocket
//{
//public:
//	CTestSocket() { }
//	virtual ~CTestSocket() { }
//
//	static CValue<TYPE>& CreateRandomValue()
//	{
//		CValue<TYPE> *pNew = new CValue<TYPE>();
//		m_arrRandom.Add(pNew);
//		return (*pNew);
//	}
//
//	static void CycleRandom()
//	{
//		for (int nAt = 0; nAt < m_arrRandom.GetSize(); nAt++)
//		{
//			m_arrRandom[nAt]->Set((*m_pGenerateRandom)());
//		}
//	}
//
//	static TYPE (*m_pGenerateRandom)();
//
//	static CArray<CValue<TYPE> *, CValue<TYPE> *&> m_arrRandom;
//
//	static CArray<CValue<TYPE> *, CValue<TYPE> *&> m_arrFunctions;
//	static CArray<CValue<TYPE> *, CValue<TYPE> *&> m_arrInvFunctions;
//};
