//////////////////////////////////////////////////////////////////////
// LookupFunction.h: interface for the CLookupFunction class, which
// provides a fast lookup-based evaluation of a 2-d function, as well
// as the ability to store and retrieve the table to a disk file.
//
// Copyright (C) 1999-2001
// $Id$
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_LOOKUPFUNCTION_H__4C4A8775_029D_11D5_9E47_00B0D0609AB0__INCLUDED_)
#define AFX_LOOKUPFUNCTION_H__4C4A8775_029D_11D5_9E47_00B0D0609AB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// class CLookupFunction<TYPE>
//
// represents a lookup-table based function of type TYPE.
//////////////////////////////////////////////////////////////////////
template<class TYPE>
class CLookupFunction : public CObject  
{
public:
	//////////////////////////////////////////////////////////////////
	// constructor -- takes a function pointer, ranges for parameters,
	//		and lookup table step sizes as inputs.
	//////////////////////////////////////////////////////////////////	
	CLookupFunction(TYPE (*pFunc)(TYPE, TYPE), 
		TYPE minA, TYPE maxA, int nStepA, TYPE minB, TYPE maxB, int nStepB,
		const CString& strCacheFilename, 
		const CString& strVersion,
		BOOL bIntegralMode = FALSE)

		: m_pFunc(pFunc),
			m_minA(minA),
			m_maxA(maxA),
			m_minB(minB),
			m_maxB(maxB),
			m_bClampValues(FALSE),
			m_bIntegralMode(bIntegralMode)
	{
		// calculate the deltas
		m_deltaA = (m_maxA - m_minA) / (TYPE)(nStepA);
		m_deltaB = (m_maxB - m_minB) / (TYPE)(nStepB);

		// try to read in the table
		if (!ReadFromFile(strCacheFilename, strVersion))
		{
			// set up the lookup table
			SetTableSize(nStepA, nStepB);

			// set the integral mode (computes the table)
			SetIntegralMode(m_bIntegralMode);

			// set up the table to clamp the values
			// SetClampValues();

			// now write out the computed table
			WriteToFile(strCacheFilename, strVersion);
		}
	}

	//////////////////////////////////////////////////////////////////
	// destructor
	//////////////////////////////////////////////////////////////////
	virtual ~CLookupFunction()
	{
	}

	//////////////////////////////////////////////////////////////////
	// SetClampValues -- determines clamping mode and sets the 
	//		clamping values
	//////////////////////////////////////////////////////////////////
	void SetClampValues(BOOL bClamp = TRUE, 
			TYPE clampMinA = 0.0, 
			TYPE clampMaxA = 0.0, 
			TYPE clampMinB = 0.0, 
			TYPE clampMaxB = 0.0)
	{
		m_bClampValues = bClamp;
		m_clampMinA = clampMinA;
		m_clampMaxA = clampMaxA;
		m_clampMinB = clampMinB;
		m_clampMaxB = clampMaxB;
	}

	//////////////////////////////////////////////////////////////////
	// SetIntegralMode -- the lookup function in integral mode
	//		actually returns the integral of the given function
	//////////////////////////////////////////////////////////////////
	void SetIntegralMode(BOOL bIntegral = TRUE)
	{
		// set the flag
		m_bIntegralMode = bIntegral;

		// and re-compute the table
		ComputeTable();

		// finally, compute the table (if necessary)
		if (m_bIntegralMode)
			ComputeIntegral();
	}

	//////////////////////////////////////////////////////////////////
	// operator() -- evaluates the function using the lookup table
	//		or clamp values
	//////////////////////////////////////////////////////////////////
	TYPE operator()(TYPE a, TYPE b)
	{
		if (a > m_minA) 
		{
			if (a < m_maxA)
			{
				if (b > m_minB)
				{
					if (b < m_maxB)
					{
						int nA = (int)((a - m_minA) / m_deltaA - 0.5f);
						ASSERT(nA >= 0 && nA < m_arrValuesIliffe.GetSize());
	
						int nB = (int)((b - m_minB) / m_deltaB - 0.5f);
						ASSERT(nB >= 0 && nB < m_arrValues.GetSize() 
							/ m_arrValuesIliffe.GetSize());

						return m_arrValuesIliffe[nA][nB];
					}
					else
						return m_bClampValues ? m_clampMaxB : (*m_pFunc)(a, b);
				}
				else
					return m_bClampValues ? m_clampMinB : (*m_pFunc)(a, b);
			}
			else
				return m_bClampValues ? m_clampMaxA : (*m_pFunc)(a, b);
		}
		else
			return m_bClampValues ? m_clampMinA : (*m_pFunc)(a, b);
	}

protected:
	//////////////////////////////////////////////////////////////////
	// SetTableSize -- sets up the lookup table to a given size
	//////////////////////////////////////////////////////////////////
	void SetTableSize(int nSizeA, int nSizeB)
	{
		// set up the lookup values table
		m_arrValues.SetSize(nSizeA * nSizeB);

		// set up the iliffe table
		m_arrValuesIliffe.SetSize(nSizeA);
		for (int nAt = 0; nAt < nSizeA; nAt++)
			m_arrValuesIliffe[nAt] = &m_arrValues[nAt * nSizeB];
	}

	//////////////////////////////////////////////////////////////////
	// ComputeTable -- computes the actual table values
	//////////////////////////////////////////////////////////////////
	void ComputeTable()
	{
		// initial table position
		int nAt = 0;

		// compute the lookup table
		TYPE a = m_minA;
		while (a < m_maxA)
		{
			TYPE b = m_minB;
			while (b < m_maxB)
			{
				m_arrValues[nAt++] = (*m_pFunc)(a, b);
				b += m_deltaB;
			}
			a += m_deltaA;
		}
	}

	//////////////////////////////////////////////////////////////////
	// ComputeIntegral -- forms the integral from the table values
	//////////////////////////////////////////////////////////////////
	void ComputeIntegral()
	{
		// setup the table (again)
		int nSizeA = (int)((m_maxA - m_minA) / m_deltaA);
		int nSizeB = (int)((m_maxB - m_minB) / m_deltaB);

		// integrate w.r.t. A
		for (int nA = 0; nA < nSizeA; nA++)
		{
			TYPE sum = 0.0;
			for (int nB = 0; nB < nSizeB; nB++)
			{
				sum += m_arrValuesIliffe[nA][nB] * m_deltaB;
				m_arrValuesIliffe[nA][nB] = sum;
			}
		}

		// integrate w.r.t. B
		for (int nB = 0; nB < nSizeB; nB++)
		{
			TYPE sum = 0.0;
			for (int nA = 0; nA < nSizeA; nA++)
			{
				sum += m_arrValuesIliffe[nA][nB] * m_deltaA;
				m_arrValuesIliffe[nA][nB] = sum;
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	// ReadFromFile -- reads in the value from a cache file
	//////////////////////////////////////////////////////////////////
	BOOL ReadFromFile(const CString& strFilename, 
			const CString& strVersionRequested)
	{
		// form the temporary path name
		CString strPathname;
		::GetTempPath(_MAX_PATH, strPathname.GetBuffer(_MAX_PATH));
		strPathname.ReleaseBuffer();
		strPathname += strFilename;

		CFile inFile;
		if (inFile.Open(strPathname, CFile::modeRead))
		{
			CArchive ar(&inFile, CArchive::load);

			// read in the version string
			CString strVersion;
			ar >> strVersion;

			// check the version of the file on disk
			if (strVersion != strVersionRequested)
				return FALSE;

			// read in the range values
			ar >> m_minA >> m_maxA >> m_deltaA;
			ar >> m_minB >> m_maxB >> m_deltaB;

			// read in the clamping values
			ar >> m_bClampValues;
			ar >> m_clampMinA >> m_clampMaxA;
			ar >> m_clampMinB >> m_clampMaxB;

			// now read the table values
			m_arrValues.Serialize(ar);

			// setup the table (again)
			int nSizeA = (int)((m_maxA - m_minA) / m_deltaA);
			int nSizeB = (int)((m_maxB - m_minB) / m_deltaB);

			// ensure the read table is the correct size
			ASSERT(m_arrValues.GetSize() == nSizeA * nSizeB);

			// set up the iliffe table
			m_arrValuesIliffe.SetSize(nSizeA);
			for (int nAt = 0; nAt < nSizeA; nAt++)
				m_arrValuesIliffe[nAt] = &m_arrValues[nAt * nSizeB];

			// close the archive and file
			ar.Close();
			inFile.Close();

			// and return TRUE
			return TRUE;
		}

		// problem with read
		return FALSE;
	}

	//////////////////////////////////////////////////////////////////
	// WriteToFile -- writes the values to a cache file
	//////////////////////////////////////////////////////////////////
	BOOL WriteToFile(const CString& strFilename,
			const CString& strVersion)
	{
		// form the temporary path name
		CString strPathname;
		::GetTempPath(_MAX_PATH, strPathname.GetBuffer(_MAX_PATH));
		strPathname.ReleaseBuffer();
		strPathname += strFilename;

		CFile outFile;
		if (outFile.Open(strPathname, CFile::modeWrite | CFile::modeCreate))
		{
			CArchive ar(&outFile, CArchive::store);

			// write the version string
			ar << strVersion;

			// write the range values
			ar << m_minA << m_maxA << m_deltaA;
			ar << m_minB << m_maxB << m_deltaB;

			// write the clamping values
			ar << m_bClampValues;
			ar << m_clampMinA << m_clampMaxA;
			ar << m_clampMinB << m_clampMaxB;

			// write the table values
			m_arrValues.Serialize(ar);

			// close the archive and file
			ar.Close();
			outFile.Close();

			// and return TRUE
			return TRUE;
		}

		// problem with write
		return FALSE;
	}

private:
	// ranges for the function input
	TYPE m_minA, m_maxA, m_deltaA;
	TYPE m_minB, m_maxB, m_deltaB;

	// pointer to the function
	TYPE (*m_pFunc)(TYPE, TYPE);

	// lookup table for the function
	CArray<TYPE, TYPE&> m_arrValues;
	CArray<TYPE *, TYPE *&> m_arrValuesIliffe;

	// clamping flag and values
	BOOL m_bClampValues;
	TYPE m_clampMinA, m_clampMaxA;
	TYPE m_clampMinB, m_clampMaxB;

	// integral mode flag
	BOOL m_bIntegralMode;
};

#endif // !defined(AFX_LOOKUPFUNCTION_H__4C4A8775_029D_11D5_9E47_00B0D0609AB0__INCLUDED_)
