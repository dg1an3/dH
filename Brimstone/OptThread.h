#pragma once

class CPrescription;

// COptThread

class COptThread : public CWinThread
{
	DECLARE_DYNCREATE(COptThread)

protected:
	COptThread();           // protected constructor used by dynamic creation
	virtual ~COptThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()

private:
	// stores pointer to the current (internal) optimizer prescription
	CPrescription *m_pPresc;
	// pointer to parameter prescription object
	CPrescription *m_pPrescParam;
	// flag to indicate parameter change event
	bool m_evtParamChanged;
	// flag to indicate new result is available
	bool m_evtNewResult;
};


/* 
class COptThread : public CWinThread
{
public:

	virtual ~COptThread();

	DECLARE_DYNCREATE(COptThread);

	virtual BOOL InitInstance() { return TRUE; }

	void Immediate();

	virtual int Run();

	virtual BOOL Callback(REAL value, const CVectorN<>& vDir);

	// external prescription object (used to transfer parameters)
	CCriticalSection m_csPrescParam;

	void UpdatePlan();

	int nIteration;
	BOOL m_bDone;
public:
	// results
	CCriticalSection m_csResult;
	REAL m_bestValue;
	CVectorN<> m_vResult;
public:
	BOOL m_evtNewResult;

public:
	// internal prescription object
	CPrescription *m_pPresc;
};
*/

