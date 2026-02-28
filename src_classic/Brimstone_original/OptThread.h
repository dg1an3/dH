#pragma once


#define WM_OPTIMIZER_START WM_APP+7
#define WM_OPTIMIZER_STOP  WM_APP+8
#define WM_OPTIMIZER_UPDATE WM_APP+9
#define WM_OPTIMIZER_DONE WM_APP+10

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

	// virtual int Run();

	// callback for optimizer iterations
	static BOOL OnIteration(COptimizer *pOpt, void *pParam);

	// bool *m_pbOptimizerRun;

	// CVectorN<> m_vParam;

	// stores data for current iteration
	class COptIterData
	{
	public:
		COptIterData();

		int m_nLevel;
		int m_nIteration;
		REAL m_ofvalue;
		CVectorN<> m_vParam;
		CVectorN<> m_vGrad;
	};

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnOptimizerStart(WPARAM wParam, LPARAM lParam);
	afx_msg void OnOptimizerStop(WPARAM wParam, LPARAM lParam);

// private:
public:
	// stores pointer to the current (internal) optimizer prescription
	CPrescription *m_pPresc;

	// pointer to window to be notified
	CWnd *m_pMsgTarget;

};	// class COptThread


