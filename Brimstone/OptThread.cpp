// OptThread.cpp : implementation file
//

#include "stdafx.h"
#include "Brimstone.h"
#include "OptThread.h"


// COptThread

IMPLEMENT_DYNCREATE(COptThread, CWinThread)

COptThread::COptThread()
: m_pPresc(NULL)
, m_pPrescParam(NULL)
, m_evtParamChanged(false)
, m_evtNewResult(false)
{
}

COptThread::~COptThread()
{
}

BOOL COptThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int COptThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(COptThread, CWinThread)
END_MESSAGE_MAP()


// COptThread message handlers


/* 


COptThread::~COptThread() 
{ 
	delete m_pPresc;
	delete m_pPrescParam;
}


IMPLEMENT_DYNCREATE(COptThread, CWinThread);


BOOL ExtCallback(REAL lambda, const CVectorN<>& vDir, 
							 void *pParam)
{
	COptThread *pOptThread = (COptThread *) pParam;
	return pOptThread->Callback(lambda, vDir);
}

void COptThread::Immediate()
{
	m_evtParamChanged = FALSE;

	// call optimize
	CVectorN<> vInit;
	m_pPresc->GetInitStateVector(vInit);
	if (m_pPresc->Optimize(vInit, ExtCallback, (void *) this))
	{
		m_evtNewResult = TRUE;

		// set updated results
		// ...
		m_pPrescParam->SetStateVectorToPlan(m_vResult);
	}
}

int COptThread::Run()
{
	while (m_pPresc)
	{
		nIteration = 0;

		m_bDone = FALSE;

		// call optimize
		CVectorN<> vInit;
		m_pPresc->GetInitStateVector(vInit);
		if (m_pPresc->Optimize(vInit, ExtCallback, (void *) this))
		{
			// set updated results
			// ...
			m_evtNewResult = TRUE;
			m_bDone = TRUE;

			// wait for new params
			SuspendThread();
			while (!m_evtParamChanged) 
			{
			}
		}

		m_csPrescParam.Lock();

		// transfer parameters
		// ...
		m_pPresc->UpdateTerms(m_pPrescParam);

		m_evtParamChanged = FALSE;

		m_csPrescParam.Unlock();
	}

	return 0;
}

BOOL COptThread::Callback(REAL value, const CVectorN<>& vRes)
{
	if (m_evtParamChanged)
	{
		return FALSE;
	}

	nIteration++;

	if (vRes.GetDim() == m_pPresc->GetPlan()->GetTotalBeamletCount(0))
	{
		// m_csResult.Lock();

		// set results
		m_bestValue = value;
		m_vResult.SetDim(vRes.GetDim());
		m_vResult = vRes;
		m_pPresc->Transform(&m_vResult);

		// TODO: get rid of this check
		for (int nAt = 0; nAt < m_vResult.GetDim(); nAt++)
		{
			ASSERT(_finite(m_vResult[nAt]));
		}

		// flag new result
//		m_evtNewResult = TRUE;

		// m_csResult.Unlock();
	}

	return TRUE;
}

void COptThread::UpdatePlan()
{
	if (m_evtNewResult)
	{
//		m_csResult.Lock();

		m_pPrescParam->SetStateVectorToPlan(m_vResult);
				
		m_evtNewResult = FALSE;

//		m_csResult.Unlock();
	}
}

*/