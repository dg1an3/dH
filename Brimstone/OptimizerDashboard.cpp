// OptimizerDashboard.cpp : implementation file
//

#include "stdafx.h"
#include "Brimstone.h"
#include "OptimizerDashboard.h"
#include ".\optimizerdashboard.h"


BOOL BrentOptCB(REAL value, const CVectorN<>& vRes, void *pParam)
{
	COptimizerDashboard *pOptDash = static_cast<COptimizerDashboard *>(pParam);
	CPrescription *pPresc = pOptDash->m_pPresc;
	COptimizer *pOpt = pPresc->GetOptimizer();
	
	pOptDash->AddBrentIter(pOpt->GetIterations(), vRes, value);

	return TRUE;
}

BOOL CGOptCB(REAL value, const CVectorN<>& vRes, void *pParam)
{
	COptimizerDashboard *pOptDash = static_cast<COptimizerDashboard *>(pParam);
	CPrescription *pPresc = pOptDash->m_pPresc;
	COptimizer *pOpt = pPresc->GetOptimizer();

	pOptDash->AddCGIter(pOpt->GetIterations(), vRes, value);

	return TRUE;
}


// COptimizerDashboard dialog

IMPLEMENT_DYNAMIC(COptimizerDashboard, CDialog)
COptimizerDashboard::COptimizerDashboard(CWnd* pParent /*=NULL*/)
	: CDialog(COptimizerDashboard::IDD, pParent)
	, m_pPresc(NULL)
	, m_nAtIter(0)
	, m_minParam(0)
	, m_maxParam(0)
	, m_minGrad(0)
	, m_maxGrad(0)
{
}

COptimizerDashboard::~COptimizerDashboard()
{
}

void COptimizerDashboard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COptimizerDashboard, CDialog)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BTN_NEXT, OnBnClickedBtnNext)
	ON_BN_CLICKED(IDC_BTN_PREV, OnBnClickedBtnPrev)
END_MESSAGE_MAP()


// call for each new BrentIteration

void COptimizerDashboard::AddBrentIter(int nIter , const CVectorN<>& vParam , REAL value)
{
	m_dsIter.AddDataPoint(CVectorD<2>(R(nIter), value));
}

void COptimizerDashboard::AddCGIter(int nIter, const CVectorN<>&  vParam, REAL value)
{
	// figure out level that we are at
	CPlan *pPlan = m_pPresc->GetPlan();

	int nLevel = MAX_SCALES-1;

	CPrescription *pPrescLevel = m_pPresc->GetLevel(nLevel);
	COptimizer *pOptLevel = pPrescLevel->GetOptimizer();

	int nIterations = 0;
	while (pPlan->GetTotalBeamletCount(nLevel) < vParam.GetDim())
	{
		nIterations += pOptLevel->GetIterations();

		nLevel--;

		pPrescLevel = m_pPresc->GetLevel(nLevel);
		pOptLevel = pPrescLevel->GetOptimizer();
	}
	ASSERT(pOptLevel->GetIterations()  >= 0);
	nIterations += pOptLevel->GetIterations();

	// add to iteration series
	m_dsIter.AddDataPoint(CVectorD<2>(nIterations, value));

	// add new param, gradient, and diff grad series
	CVectorN<> vParamXform = vParam;
	pPrescLevel->Transform(&vParamXform);
	CDataSeries *pParamDS = new CDataSeries();
	ITERATE_VECTOR(vParamXform, nAt, 
		pParamDS->AddDataPoint(CVectorD<2>(R(nAt), vParamXform[nAt])));
	m_arrParamDS.Add(pParamDS);

	// set min / max of param
	ITERATE_VECTOR(vParamXform, nAt, m_minParam = __min(m_minParam, vParamXform[nAt]));
	ITERATE_VECTOR(vParamXform, nAt, m_maxParam = __max(m_maxParam, vParamXform[nAt]));

	// evaluate gradient
	CVectorN<> vGrad;
	(*pPrescLevel)(vParam, &vGrad);

	// add new gradient data series
	CDataSeries *pGradDS = new CDataSeries();
	ITERATE_VECTOR(vGrad, nAt, 
		pGradDS->AddDataPoint(CVectorD<2>(R(nAt), vGrad[nAt])));
	m_arrGradDS.Add(pGradDS);

	// set min / max
	ITERATE_VECTOR(vGrad, nAt, m_minGrad = __min(m_minGrad, vGrad[nAt]));
	ITERATE_VECTOR(vGrad, nAt, m_maxGrad = __max(m_maxGrad, vGrad[nAt]));

	// evaluate difference gradient
	CVectorN<> vDiffGrad;
	const REAL EPS = 1e-6;
	pPrescLevel->Gradient(vParam, EPS, vDiffGrad);

	// add new diff grad data series
	CDataSeries *pDiffGradDS = new CDataSeries();
	pDiffGradDS->SetColor(RGB(0, 0, 255));
	ITERATE_VECTOR(vDiffGrad, nAt, 
		pDiffGradDS->AddDataPoint(CVectorD<2>(R(nAt), vDiffGrad[nAt])));
	m_arrDiffGradDS.Add(pDiffGradDS);

	// set min / max
	ITERATE_VECTOR(vDiffGrad, nAt, m_minGrad = __min(m_minGrad, vDiffGrad[nAt]));
	ITERATE_VECTOR(vDiffGrad, nAt, m_maxGrad = __max(m_maxGrad, vDiffGrad[nAt]));
}

int COptimizerDashboard::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL COptimizerDashboard::OnInitDialog()
{
	CDialog::OnInitDialog();


	CRect rectGraphIter;
	GetDlgItem(IDC_STATIC_ITER_GRAPH)->GetWindowRect(&rectGraphIter);
	ScreenToClient(&rectGraphIter);
	if (!m_graphIterations.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, 
		rectGraphIter, this, IDC_STATIC_ITER_GRAPH))
	{
		return FALSE;
	}
	m_graphIterations.SetMargins(60, 5, 5, 30);

	CRect rectGraphParam;
	GetDlgItem(IDC_STATIC_PARAM_GRAPH)->GetWindowRect(&rectGraphParam);
	ScreenToClient(&rectGraphParam);
	if (!m_graphParam.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, 
		rectGraphParam, this, IDC_STATIC_PARAM_GRAPH))
	{
		return FALSE;
	}
	m_graphParam.SetMargins(60, 5, 5, 30);

	CRect rectGraphGrad;
	GetDlgItem(IDC_STATIC_GRAD_GRAPH)->GetWindowRect(&rectGraphGrad);
	ScreenToClient(&rectGraphGrad);
	if (!m_graphGrad.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, 
		rectGraphGrad, this, IDC_STATIC_GRAD_GRAPH))
	{
		return FALSE;
	}
	m_graphGrad.SetMargins(60, 5, 5, 30);

	// now initialize the prescription
	CVectorN<> vInit;
	m_pPresc->GetInitStateVector(vInit);

	// and run the optimization
	if (m_pPresc->Optimize(vInit, CGOptCB, this))
	{
		m_pPresc->SetStateVectorToPlan(vInit);

		m_graphIterations.AddDataSeries(&m_dsIter);

		m_graphParam.AddDataSeries(m_arrParamDS[0]);

		m_graphGrad.AddDataSeries(m_arrGradDS[0]);
		m_graphGrad.AddDataSeries(m_arrDiffGradDS[0]);


		SetAxes();

	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void COptimizerDashboard::OnBnClickedBtnNext()
{
	if (m_nAtIter < m_arrParamDS.GetSize()-1)
	{
		m_nAtIter++;

		m_graphParam.RemoveAllDataSeries();
		m_graphParam.AddDataSeries(m_arrParamDS[m_nAtIter]);

		m_graphParam.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		
		m_graphGrad.RemoveAllDataSeries();
		m_graphGrad.AddDataSeries(m_arrGradDS[m_nAtIter]);
		m_graphGrad.AddDataSeries(m_arrDiffGradDS[m_nAtIter]);

		SetAxes();

		m_graphGrad.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void COptimizerDashboard::OnBnClickedBtnPrev()
{
	if (m_nAtIter > 0)
	{
		m_nAtIter--;

		m_graphParam.RemoveAllDataSeries();
		m_graphParam.AddDataSeries(m_arrParamDS[m_nAtIter]);

		m_graphParam.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		
		m_graphGrad.RemoveAllDataSeries();
		m_graphGrad.AddDataSeries(m_arrGradDS[m_nAtIter]);
		m_graphGrad.AddDataSeries(m_arrDiffGradDS[m_nAtIter]);

		SetAxes();

		m_graphGrad.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

// sets up axes for current data
void COptimizerDashboard::SetAxes(void)
{
	m_graphIterations.AutoScale();

	CVectorD<2> vMinor = m_graphIterations.GetAxesMinor();
	vMinor[0] = 1.0;
	m_graphIterations.SetAxesMinor(vMinor);

	//////////////////////////

	m_graphParam.AutoScale();

	vMinor = m_graphParam.GetAxesMinor();
	vMinor[0] = 1.0;
	m_graphParam.SetAxesMinor(vMinor);

	CVectorD<2> vMin = m_graphParam.GetAxesMin();
	vMin[1] = m_minParam;
	m_graphParam.SetAxesMin(vMin);

	CVectorD<2> vMax = m_graphParam.GetAxesMax();
	vMax[1] = m_maxParam;
	m_graphParam.SetAxesMax(vMax);


	//////////////////////////

	m_graphGrad.AutoScale();

	vMinor = m_graphGrad.GetAxesMinor();
	vMinor[0] = 1.0;
	m_graphGrad.SetAxesMinor(vMinor);

	vMin = m_graphGrad.GetAxesMin();
	vMin[1] = m_minGrad;
	m_graphGrad.SetAxesMin(vMin);

	vMax = m_graphGrad.GetAxesMax();
	vMax[1] = m_maxGrad;
	m_graphGrad.SetAxesMax(vMax);
}
