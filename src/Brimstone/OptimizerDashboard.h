#include <Prescription.h>
#include <Graph.h>

#pragma once


// COptimizerDashboard dialog

class COptimizerDashboard : public CDialog
{
	DECLARE_DYNAMIC(COptimizerDashboard)

public:
	COptimizerDashboard(CWnd* pParent = NULL);   // standard constructor
	virtual ~COptimizerDashboard();

// Dialog Data
	enum { IDD = IDD_OPTIMIZERTEST_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// the prescription to drive
	CPrescription *m_pPresc;

	// displays graph of iterations
	CGraph m_graphIterations;
	CDataSeries m_dsIter;

	// graph of param for current iteration
	CGraph m_graphParam;
	CTypedPtrArray<CPtrArray, CDataSeries*> m_arrParamDS;

	// graph of gradient for current iteration
	CGraph m_graphGrad;
	CTypedPtrArray<CPtrArray, CDataSeries*> m_arrGradDS;
	CTypedPtrArray<CPtrArray, CDataSeries*> m_arrDiffGradDS;

	// call for each new BrentIteration
	void AddBrentIter(int nIter , const CVectorN<>& vParam , REAL value);
	void AddCGIter(int nIter, const CVectorN<>&  vParam, REAL value);

	// message handler
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnNext();
	int m_nAtIter;
	afx_msg void OnBnClickedBtnPrev();
	REAL m_minParam;
	REAL m_maxParam;
	REAL m_minGrad;
	REAL m_maxGrad;
	// sets up axes for current data
	void SetAxes(void);
};
