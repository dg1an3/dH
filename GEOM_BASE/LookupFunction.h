// LookupFunction.h: interface for the CLookupFunction class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOOKUPFUNCTION_H__4C4A8775_029D_11D5_9E47_00B0D0609AB0__INCLUDED_)
#define AFX_LOOKUPFUNCTION_H__4C4A8775_029D_11D5_9E47_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template<class TYPE>
class CLookupFunction : public CObject  
{
public:
	CLookupFunction(TYPE (*pFunc)(TYPE, TYPE), 
		TYPE minA, TYPE maxA, int nStepA, TYPE minB, TYPE maxB, int nStepB,
		const CString& strCacheFilename)
		: m_pFunc(pFunc),
			m_minA(minA),
			m_maxA(maxA),
			m_minB(minB),
			m_maxB(maxB),
			m_strCacheFileName(strCacheFilename)
	{
		m_deltaA = (m_maxA - m_minA) / (TYPE)(nStepA);
		m_deltaB = (m_maxB - m_minB) / (TYPE)(nStepB);

		m_arrValues.SetSize(nStepA * nStepB);

		BOOL bRead = FALSE;
		if (m_strCacheFileName != "")
		{
			CFile inFile;
			if (inFile.Open(m_strCacheFileName, CFile::modeRead))
			{
				CArchive ar(&inFile, CArchive::load);
				Serialize(ar);
				ar.Close();
				inFile.Close();
				bRead = TRUE;
			}
		}

		m_arrValuesIliffe.SetSize(nStepA);
		int nAt;
		for (nAt = 0; nAt < nStepA; nAt++)
			m_arrValuesIliffe[nAt] = &m_arrValues[nAt * nStepB];

		if (bRead)
			return;

		nAt = 0;

		TYPE a = minA;
		while (a < maxA)
		{
			TYPE b = minB;
			while (b < maxB)
			{
				m_arrValues[nAt++] = (*pFunc)(a, b);
				b += m_deltaB;
			}
			a += m_deltaA;
		}

		if (m_strCacheFileName != "")
		{
			CFile outFile;
			if (outFile.Open(m_strCacheFileName, CFile::modeWrite | CFile::modeCreate))
			{
				CArchive ar(&outFile, CArchive::store);
				Serialize(ar);
				ar.Close();
				outFile.Close();
			}
		}
	}

	virtual ~CLookupFunction()
	{
	}

	TYPE operator()(TYPE a, TYPE b)
	{
		if ((a > m_minA) && (a < m_maxA)
			&& (b > m_minB) && (b < m_maxB))
		{
			int nA = (int)((a - m_minA) / m_deltaA - 0.5f);
			ASSERT(nA >= 0 && nA < m_arrValuesIliffe.GetSize());

			int nB = (int)((b - m_minB) / m_deltaB - 0.5f);
			ASSERT(nB >= 0 && nB < m_arrValues.GetSize() / m_arrValuesIliffe.GetSize());

			return m_arrValuesIliffe[nA][nB];
		}
		return (TYPE) 0.0; // (*m_pFunc)(a, b);
	}

	void Serialize(CArchive& ar)
	{
		m_arrValues.Serialize(ar);
	}

private:
	CArray<TYPE, TYPE&> m_arrValues;
	CArray<TYPE *, TYPE *&> m_arrValuesIliffe;

	TYPE (*m_pFunc)(TYPE, TYPE);

	TYPE m_minA, m_maxA, m_deltaA;
	TYPE m_minB, m_maxB, m_deltaB;

	CString m_strCacheFileName;
};

#endif // !defined(AFX_LOOKUPFUNCTION_H__4C4A8775_029D_11D5_9E47_00B0D0609AB0__INCLUDED_)
