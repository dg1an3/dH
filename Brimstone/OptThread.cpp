// OptThread.cpp : implementation file
//

#include "stdafx.h"
// #include "Brimstone.h"
#include "OptThread.h"
// #include "BrimstoneDoc.h"


// COptThread

IMPLEMENT_DYNCREATE(COptThread, CWinThread)

COptThread::COptThread()
: m_pPresc(NULL)
	// , m_pbOptimizerRun(NULL)
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

COptThread::COptIterData::COptIterData(void)
: m_nLevel(0)
, m_nIteration(0)
{
}

BEGIN_MESSAGE_MAP(COptThread, CWinThread)
	ON_THREAD_MESSAGE(WM_OPTIMIZER_START, &COptThread::OnOptimizerStart)
	ON_THREAD_MESSAGE(WM_OPTIMIZER_STOP, &COptThread::OnOptimizerStop)
END_MESSAGE_MAP()


// COptThread message handlers

BOOL COptThread::OnIteration(COptimizer *pOpt, void *pParam)
{
	COptThread *pThread = static_cast<COptThread*>(pParam);
	CPrescription *pPresc = pThread->m_pPresc;

	COptIterData *pOID = NULL;
	for (int nLevel = MAX_SCALES-1; nLevel >= 0; nLevel--)
	{
		CPrescription *pPrescLevel = pPresc->GetLevel(nLevel);
		COptimizer *pOptLevel = pPrescLevel->GetOptimizer();			

		if (pOpt == pOptLevel)
		{
			pOID = new COptIterData();
			pOID->m_nLevel = nLevel;
			pOID->m_nIteration = pOpt->GetIterations();
			pOID->m_ofvalue = pOpt->GetFinalValue();
			pOID->m_vParam.SetDim(pOpt->GetFinalParameter().GetDim());
			pOID->m_vParam = pOpt->GetFinalParameter();
			pPresc->Transform(&pOID->m_vParam);

			// if we are still above G0, then proceed to filter down to G0
			for (; nLevel > 0; nLevel--)
			{
				// now filter to proper level
				pPresc->InvFilterStateVector(pOID->m_vParam, nLevel, pOID->m_vParam, false);
			}
		}
	}
	ASSERT(pOID != NULL);

	pThread->m_pMsgTarget->PostMessage(WM_OPTIMIZER_UPDATE, 0, (LPARAM) pOID);

	// get current thread state
	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// and determine whether any messages are pending, if so...
	bool bContinueOptimizer = 
		!PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE);

	// quit optimizer
	return bContinueOptimizer;

	// return (*pThread->m_pbOptimizerRun);
}

void COptThread::OnOptimizerStart(WPARAM wParam, LPARAM lParam)
{
	CVectorN<> m_vParam;

	// call optimize
	m_pPresc->GetInitStateVector(m_vParam);

	COptIterData *pOID = new COptIterData();
	pOID->m_nLevel = 0;
	pOID->m_ofvalue = m_pPresc->Optimize(m_vParam, &COptThread::OnIteration, this);

	pOID->m_vParam.SetDim(m_vParam.GetDim());
	pOID->m_vParam = m_vParam;

	m_pMsgTarget->PostMessage(WM_OPTIMIZER_DONE, 0, (LPARAM) pOID);

	LOG("Done with thread");

	// end the thread
	// return 0; // ExitInstance();
}

void COptThread::OnOptimizerStop(WPARAM wParam, LPARAM lParam)
{
	TRACE("Optimizer stopping\n");
	m_pMsgTarget->PostMessage(WM_OPTIMIZER_STOP, NULL, NULL);
}
